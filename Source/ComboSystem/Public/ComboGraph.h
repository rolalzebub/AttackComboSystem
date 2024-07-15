#pragma once

#include "FAttackAction_Struct.h"
#include "ComboGraph.generated.h"


/// <summary>
/// This struct represents one node in the graph.
/// A node in the graph represents one attack in the moveset.
/// </summary>
USTRUCT(BlueprintType)
struct FComboGraphNode
{
	GENERATED_USTRUCT_BODY()

public:
	// This represents the previous attack chain that needs to be achieved to reach this attack.
	FComboGraphNode* parentNode;

	// This represents the next possible attacks that can be performed after this.
	TArray<FComboGraphNode*> childrenNodes;

	// The data entry in the combo table for this particular attack.
	FAttackAction_Struct* attackData;
};


/// <summary>
/// This graph represents the entire moveset contained in the data table in graph form.
/// </summary>
class ComboGraph
{
	// Private variables/properties
private:
	/// <summary>
	/// Root for the entire combo graph. The root node will always be a non-attack representing the starting point of a combo where no attack has been performed yet.
	/// </summary>
	FComboGraphNode rootNode;


	// The depth of the current tree
	int treeDepth = -1;


	// Public functions
public:

	/// <summary>
	/// This function uses the data table provided to create a combo graph contained within. Use this to initialize the graph.
	/// </summary>
	/// <param name="movesetTable">The table to create a moveset from</param>
	void CreateComboGraph(UDataTable* movesetTable);

	/// <summary>
	/// This function does a breadth-first search of the graph to find a node whose attack chain matches the one provided as the parameter
	/// </summary>
	/// <param name="attackChainToFind">The attack chain to look for</param>
	/// <returns>Pointer to the found node. Null if the node cannot be found</returns>
	FComboGraphNode* FindNodeWithAttackChain(TArray<TEnumAsByte<AttackType_Enum>> attackChainToFind);

	/// <summary>
	/// This function does a fast (single iteration depth-first) search of the graph to find the node with the matching attack sequence.
	/// </summary>
	/// <param name="attackChainToFind">The attack chain to look for</param>
	/// <returns>Pointer to the found node. Null if the node cannot be found</returns>
	FComboGraphNode* FastFindNodeWithAttackChain(TArray<TEnumAsByte<AttackType_Enum>> attackChainToFind);


	FComboGraphNode* FastFindNodeWithLastPerformedAttack(TArray<TEnumAsByte<AttackType_Enum>> currentSequence, FComboGraphNode* lastAttackNode);

	FComboGraphNode* SearchGraphStartingFromRootNode(TArray<TEnumAsByte<AttackType_Enum>> currentSequence, FComboGraphNode* searchRootNode);

};
