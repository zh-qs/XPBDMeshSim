// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "TetMesh3.h"
#include "TwoWayConstraintVertexCoupling.h"
#include "SkeletonDirectPositionManager.h"

#include "TetrahedralMesh.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class TETMESHES_API UTetrahedralMesh : public UObject
{
	GENERATED_BODY()

private:
	TUniquePtr<FTetMesh3> m_TetMesh;
	TUniquePtr<FSkeletonDirectPositionManager> m_SkeletonManager;
	TUniquePtr<FTwoWayConstraintVertexCoupling> m_ConstraintCoupling;

public:
	UTetrahedralMesh();

	const FTetMesh3& GetMeshRef() const { return *m_TetMesh; }
	const FTetMesh3* GetMeshPtr() const { return m_TetMesh.Get(); }

	FTetMesh3& GetMeshRef() { return *m_TetMesh; }
	FTetMesh3* GetMeshPtr() { return m_TetMesh.Get(); }

	void SetCoupledSkeleton(USkeleton& skeleton, const FTransform& initialRootBoneTransform);
	bool HasSkeletonManager() const { return m_SkeletonManager.IsValid(); }
	FSkeletonDirectPositionManager& GetSkeletonManager() { return *m_SkeletonManager; }

	void InitConstraintCoupling();
	bool HasConstraintCoupling() const { return m_ConstraintCoupling.IsValid(); }
	FTwoWayConstraintVertexCoupling& GetConstraintCoupling() { return *m_ConstraintCoupling; }
	void DestroyConstraintCoupling();

	// DEBUG
	UFUNCTION(BlueprintCallable, Category="SoftBodySimulation|DebugTools")
	void PrintMesh();

	UFUNCTION(BlueprintCallable, Category="SoftBodySimulation|DebugTools")
	void PrintMeshOBJ();

	UFUNCTION(BlueprintCallable, Category="SoftBodySimulation|DebugTools")
	int GetVertexCount();

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|DebugTools")
	int GetTetrahedronCount();
};