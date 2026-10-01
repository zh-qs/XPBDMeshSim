#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"

#include "TetrahedralMesh.h"
#include "TetrahedronDynamicMeshBinding.h"

#include "DeformableDynamicMeshComponent.generated.h"

UENUM(BlueprintType)
enum class EDeformableDisplayMode : uint8 { TriangleMesh, Wireframe };

/**
 * 
 */
UCLASS()
class TETMESHES_API UDeformableDynamicMeshComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()
private:
	UPROPERTY(Instanced)
	TObjectPtr<UTetrahedralMesh> TetMeshObject;

	UPROPERTY(Instanced)
	TObjectPtr<UTetrahedronDynamicMeshBinding> Binding;
	
	UPROPERTY(Instanced)
	TObjectPtr<UDynamicMesh> SecondMeshObject;

	EDeformableDisplayMode Mode = EDeformableDisplayMode::TriangleMesh;
	
	void InitSubobjectsUnsafe();

public:
	UDeformableDynamicMeshComponent();

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMeshComponent")
	void InitSubobjects();

	virtual void PostLoad() override;

	UFUNCTION(BlueprintCallable, Category="SoftBodySimulation|TetrahedralMeshComponent")
	UTetrahedralMesh* GetTetrahedralMesh() { 
		ensure(TetMeshObject != nullptr);
		return TetMeshObject; 
	}

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMeshComponent")
	UTetrahedronDynamicMeshBinding* GetBinding() { 
		ensure(Binding != nullptr);
		return Binding; 
	}
	
	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMeshComponent")
	UDynamicMesh* GetSecondDynamicMesh() { 
		ensure(SecondMeshObject != nullptr);
		return SecondMeshObject; 
	}

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|Display")
	void SetDisplayMode(EDeformableDisplayMode DisplayMode);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|Display")
	EDeformableDisplayMode GetDisplayMode() { return Mode; }
};
