#pragma once

#include "CoreMinimal.h"
#include "SkeletonDirectPositionManager.h"

class FOneWayVertexCoupling 
{
	const FTransform* additionalTransform = nullptr;
	const FTransform& movementSource;
	TArray<int> coupledVertexIndices;
	TArray<FVector3f> couplings;

public:
	FOneWayVertexCoupling(const FTransform& movementSource) : movementSource(movementSource) {}
	FOneWayVertexCoupling(const FTransform& movementSource, const FTransform& additionalTransform) : movementSource(movementSource), additionalTransform(&additionalTransform) {}
	void AddCoupledVertex(int index, const FVector3f& coupling)
	{
		coupledVertexIndices.Add(index);
		couplings.Add(coupling);
	}

	~FOneWayVertexCoupling() {}

	FMatrix GetMovementSourceMatrix() const {
		if (additionalTransform == nullptr)
			return movementSource.ToMatrixNoScale();
		return additionalTransform->ToMatrixWithScale() * movementSource.ToMatrixNoScale();
	}

	void UpdateCouplingsArrays(TArray<int>& actorIndices, int actorIndexToInsert, TArray<FVector3f>& otherCouplings) const
	{
		for (int i = 0; i < coupledVertexIndices.Num(); ++i)
		{
			actorIndices[coupledVertexIndices[i]] = actorIndexToInsert;
			otherCouplings[coupledVertexIndices[i]] = couplings[i];
		}
	}

	const TArray<int>& GetCoupledVertexIndices() const { return coupledVertexIndices; }
	const TArray<FVector3f>& GetCouplings() const { return couplings; }

	int Num() const { return coupledVertexIndices.Num(); }
	bool Empty() const { return Num() == 0; }
};