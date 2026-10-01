#pragma once

#include "OneWayVertexCoupling.h"
#include "TetMesh3.h"

class FTwoWayConstraintVertexCoupling
{
	FTetMesh3& tetMesh;

	TArray<int> jointVertexIndices;
	TArray<int> jointParentIndices;
	TArray<float> jointDistances;
	
public:
	FTwoWayConstraintVertexCoupling(FSkeletonDirectPositionManager& skeletonManager, FTetMesh3& tetMesh);

	void EnforceConstraints();
};