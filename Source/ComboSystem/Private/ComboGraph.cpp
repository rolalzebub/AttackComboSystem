#include "ComboGraph.h"

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
//Stat groups for performance checking for combo search
DECLARE_STATS_GROUP(TEXT("ComboGraph_Component"), STATGROUP_COMBOGRAPH, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("FastSearchForAttack"), STAT_FastSearchForAttack, STATGROUP_COMBOGRAPH);
#endif


void ComboGraph::CreateComboGraph(UDataTable* movesetTable)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	// Debugging messages
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.5f, FColor::Yellow, TEXT("Beginning combo graph creation"));
	}
#endif

	rootNode = *(new FComboGraphNode());

	TArray<FName> rowNames = movesetTable->GetRowNames(); 
	int rowCount = rowNames.Num();


	int maxTreeDepth = -1;
	// Find the max tree depth
	// We do this by checking what is the longest input sequence to activate a move
	// The number of inputs required to achieve a combo is the same as the tree depth at which the node would reside, so we simply find the longest number of inputs required to activate any of the moves
	for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
	{
		FAttackAction_Struct* attackToCheck = movesetTable->FindRow<FAttackAction_Struct>(rowNames[rowIndex], "");

		int inputsRequiredForAttack = attackToCheck->requiredSequenceToActivateAttack.Num();

		if (inputsRequiredForAttack > maxTreeDepth)
		{
			maxTreeDepth = inputsRequiredForAttack;
		}
	}
	// Make sure there is at least one attack chain in the moveset
	if (maxTreeDepth <= 0)
	{
		// TODO: Throw an exception here because the provided data table is invalid
	}

	treeDepth = maxTreeDepth;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	// Debugging messages
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Yellow, TEXT("Max Tree Depth: " + FString::FromInt(maxTreeDepth)));
	}
#endif


	// Next, fill out the graph one depth level at a time
	for (int currentTreeDepthLevel = 1; currentTreeDepthLevel <= maxTreeDepth; currentTreeDepthLevel++)
	{
		for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
		{
			FAttackAction_Struct* attackToCheck = movesetTable->FindRow<FAttackAction_Struct>(rowNames[rowIndex], "");

			int attackChainLength = attackToCheck->requiredSequenceToActivateAttack.Num();

			// Skip this move if it does not fit at the current tree depth level
			if (attackChainLength != currentTreeDepthLevel)
			{
				continue;
			}

			// If the chain length is 1, then the move is a basic move that only requires one input to execute, in which case insert it right away under the "empty" root node
			if (attackChainLength == 1)
			{
				FComboGraphNode* newAttackToInsert = new FComboGraphNode();
				newAttackToInsert->attackData = attackToCheck;
				newAttackToInsert->parentNode = &rootNode;

				rootNode.childrenNodes.Add(newAttackToInsert);

				// We can safely skip the rest of the loop for this iteration
				continue;
			}

			// Get the attack chain required to reach the parent of the current attack by removing the final input
			TArray<TEnumAsByte<AttackType_Enum>> parentMoveChain = attackToCheck->requiredSequenceToActivateAttack;
			parentMoveChain.RemoveAt(parentMoveChain.Num() - 1);

			FComboGraphNode* currentAttackParentNode = FindNodeWithAttackChain(parentMoveChain);

			// If the pointer is null, throw an exception because there is an unreachable attack in the moveset
			if (currentAttackParentNode == nullptr)
			{
				// TODO: Throw exception here for invalid attack chain in the moveset
			}

			FComboGraphNode* newAttackToInsert = new FComboGraphNode();
			newAttackToInsert->attackData = attackToCheck;
			newAttackToInsert->parentNode = currentAttackParentNode;
			currentAttackParentNode->childrenNodes.Add(newAttackToInsert);

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
			// Debugging messages
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(2, 1.5f, FColor::Yellow, TEXT("Inserted attack into moveset graph: " + attackToCheck->moveName));
			}
#endif
		}
	}


#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	// Debugging messages
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(3, 1.5f, FColor::Yellow, TEXT("Moveset graph succesfully built"));
	}
#endif
}

FComboGraphNode* ComboGraph::FindNodeWithAttackChain(TArray<TEnumAsByte<AttackType_Enum>> attackChainToFind)
{
	TQueue<FComboGraphNode*>* nodesToSearch = new TQueue<FComboGraphNode*>();
	
	// Add children of root nodes to the queue because we don't want to search the root node which has an empty attack chain
	for (int childIndex = 0; childIndex < rootNode.childrenNodes.Num(); childIndex++)
	{
		nodesToSearch->Enqueue(rootNode.childrenNodes[childIndex]);
	}

	FComboGraphNode* currentNode;

	nodesToSearch->Dequeue(currentNode);

	while (currentNode != nullptr)
	{
		// Check if the current node is the one we are searching for
		if (currentNode->attackData->requiredSequenceToActivateAttack == attackChainToFind)
		{
			return currentNode;
		}

		// Add all children nodes to the queue for next iterations of the search
		for (int childIndex = 0; childIndex < currentNode->childrenNodes.Num(); childIndex++)
		{
			nodesToSearch->Enqueue(currentNode->childrenNodes[childIndex]);
		}

		// Either break out of the loop if the queue is empty
		if (nodesToSearch->IsEmpty())
		{
			currentNode = nullptr;
		}
		// Or proceed to search the next element
		else
		{
			nodesToSearch->Dequeue(currentNode);
		}
	}

	return nullptr;
}


FComboGraphNode* ComboGraph::FastFindNodeWithAttackChain(TArray<TEnumAsByte<AttackType_Enum>> attackChainToFind)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	SCOPE_CYCLE_COUNTER(STAT_FastSearchForAttack);
#endif

	TQueue<FComboGraphNode*> searchQueue;

	FComboGraphNode* nodeToCheck = &rootNode;

	return SearchGraphStartingFromRootNode(attackChainToFind, nodeToCheck);
}

FComboGraphNode* ComboGraph::FastFindNodeWithLastPerformedAttack(TArray<TEnumAsByte<AttackType_Enum>> currentSequence, FComboGraphNode* lastAttackNode)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	SCOPE_CYCLE_COUNTER(STAT_FastSearchForAttack);
#endif

	FComboGraphNode* nodeToCheck = lastAttackNode;

	return SearchGraphStartingFromRootNode(currentSequence, nodeToCheck);
}


// The logic behind this search pattern is:
// 1. Consider the root Node the starting point of the search
// 2. Break down the list of attacks for the chain and search one element at a time
// 3. At each element starting from the empty root node, search the node's children if they have the same attack as the attack chain at that index of the chain
// 4.1.a. If a match is found, and if the current depth level is the same as the length of the attack chain then return the match as this is our result
// 4.1.b If a match is found, but it is not our final result, set it as the new "node to search" and flag that the search can continue. Break out of the for loop as we do not need to check the rest of the children.
// 4.2 If a match is not found, we can cancel search as there is no attack chain with the given sequence thus far
// 5. If the next node to search is found, we can continue to the next iteration of the loop. Go to step 3.
// 6. Break out of the search loop and return a nullptr as the attack chain cannot be found.
//
FComboGraphNode* ComboGraph::SearchGraphStartingFromRootNode(TArray<TEnumAsByte<AttackType_Enum>> currentSequence, FComboGraphNode* searchRootNode)
{
	FDateTime startTime = FDateTime::UtcNow();

	FComboGraphNode* nodeToCheck = searchRootNode;

	bool nextNodeFound = true;
	
	//init the current depth level to 0 if we are starting from a root node search, or to the actual depth level of the attack chain before this point otherwise
	int currentDepthLevel;
	if (currentSequence.Num() < 1)
	{
		currentDepthLevel = 0;
	}
	else
	{
		currentDepthLevel = currentSequence.Num() - 1;
	}

	while (nextNodeFound == true)
	{
		nextNodeFound = false;

		for (int childIndex = 0; childIndex < nodeToCheck->childrenNodes.Num(); childIndex++)
		{
			// If the current child that is being looked at has the correct input type at the depth level to match the attack chain
			// use it as the next iteration's parent node
			if (nodeToCheck->childrenNodes[childIndex]->attackData->requiredSequenceToActivateAttack[currentDepthLevel] == currentSequence[currentDepthLevel])
			{
				// Check if we are at the end of the attack chain, return the move we have found now if so
				if (currentDepthLevel == currentSequence.Num() - 1)
				{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
					//debug statements
					GEngine->AddOnScreenDebugMessage(1, 5, FColor::Green, TEXT("Time taken to success condition: " + (FString)std::to_string((FDateTime::UtcNow() - startTime).GetTotalMilliseconds()).c_str()));
#endif

					return nodeToCheck->childrenNodes[childIndex];
				}

				// Increase depth level of search, store the next iteration's parent node, and flag that the next iteration can continue

				currentDepthLevel++;

				nodeToCheck = nodeToCheck->childrenNodes[childIndex];

				nextNodeFound = true;

				break;
			}
		}
	}

	#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	//debug statements
	GEngine->AddOnScreenDebugMessage(1, 5, FColor::Green, TEXT("Time taken to failure condition: " + (FString)std::to_string((FDateTime::UtcNow() - startTime).GetTotalMilliseconds()).c_str()));
#endif
	return nullptr;
}
