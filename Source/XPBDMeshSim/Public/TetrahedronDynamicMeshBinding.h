#pragma once

#include "CoreMinimal.h"
#include <Util/DynamicVector.h>
#include "UDynamicMesh.h"
#include "TetrahedralMesh.h"

#include "TetrahedronDynamicMeshBinding.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class TETMESHES_API UTetrahedronDynamicMeshBinding : public UObject
{
	GENERATED_BODY()
private:
	TArray<int> TetIdxToDynamicMeshIdx{}; // 0 if no dynamic mesh vertex bound to tet vertex, else 1+dmVtxIdx
	TArray<int> DynamicMeshIdxToTetIdx{}; // 1+tetVtxIdx
	
	bool Active = true;
public:

	UTetrahedronDynamicMeshBinding();
	~UTetrahedronDynamicMeshBinding();

	void Clear();

	void AppendSingleBinding(int tetIdx, int dmIdx);
	int GetTetrahedronMeshIndex(int dmIdx);
	
	// DEBUG
	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|DebugTools")
	void PrintBinding();

	void UpdateDynamicMeshFromTetMesh(UDynamicMesh* DynamicMesh, const UTetrahedralMesh* TetMesh);
	
	void SetActive(bool active) { Active = active; }
	bool IsActive() const { return Active; }
};
