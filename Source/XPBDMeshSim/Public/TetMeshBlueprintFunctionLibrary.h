#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UDynamicMesh.h"
#include "Animation/Skeleton.h"

#include "TetMesh3.h"
#include "TetrahedralMesh.h"
#include "TetrahedronDynamicMeshBinding.h"
#include "SimulationStatistics.h"

#include "TetMeshBlueprintFunctionLibrary.generated.h"


/**
 *
 */
UENUM(BlueprintType)
enum class ETetMeshBlueprintResult : uint8 { Success, Failure };

UENUM(BlueprintType)
enum class ETetrahedronMeshingAlgorithm : uint8 { Simple, Tetgen };

UENUM(BlueprintType)
enum class EPlasticityModel : uint8 { Constraint, PolarDecomposition };

UCLASS()
class UTetMeshBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh", meta = (ExpandEnumAsExecs = "Outcome"))
	static UTetrahedralMesh* GenerateTetrahedralMesh(const UDynamicMesh* InMesh, UTetrahedralMesh* OutMesh, ETetrahedronMeshingAlgorithm Algorithm, float Density, ETetMeshBlueprintResult& Outcome, const FString& Options);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh", meta = (ExpandEnumAsExecs = "Outcome"))
	static UDynamicMesh* CopyTetrahedralMeshToDynamicMesh(const UTetrahedralMesh* InMesh, UDynamicMesh* OutMesh, ETetMeshBlueprintResult& Outcome);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh", meta = (ExpandEnumAsExecs = "Outcome"))
	static UTetrahedralMesh* GenerateAndBindTetrahedralMesh(const UDynamicMesh* InMesh, UTetrahedralMesh* OutMesh, ETetrahedronMeshingAlgorithm Algorithm, float Density, ETetMeshBlueprintResult& Outcome, const FString& Options, UTetrahedronDynamicMeshBinding* Binding = nullptr);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh", meta = (ExpandEnumAsExecs = "Outcome"))
	static UTetrahedralMesh* LoadTetrahedralMeshDataFromFile(UDynamicMesh* DynamicMesh, UTetrahedralMesh* TetMesh, UTetrahedronDynamicMeshBinding* Binding, float Density, const FString& FilePath, ETetMeshBlueprintResult& Outcome);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh", meta = (ExpandEnumAsExecs = "Outcome"))
	static UDynamicMesh* LoadDynamicMeshDataFromFile(UDynamicMesh* DynamicMesh, const FString& FilePath, ETetMeshBlueprintResult& Outcome);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh")
	static void InitializeInitialConfiguration(UTetrahedralMesh* TetMesh);
	
	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|Simulations")
	static void DoSimpleCPUSimulationStep(UDynamicMesh* DynamicMesh, UTetrahedralMesh* TetMesh, UTetrahedronDynamicMeshBinding* Binding, float dt);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|Simulations")
	static void DoSimpleGPUSimulationStep(UDynamicMesh* DynamicMesh, UTetrahedralMesh* TetMesh, UTetrahedronDynamicMeshBinding* Binding,
		float dt, float YoungModulus, float PoissonRatio, float YieldStress, float PositionalDampingStiffness, 
		const FVector& Gravity, int NumSteps, int NumPosIters, EPlasticityModel PlasticityModel, bool LogToConsole,
		FSimulationStatistics& OutStatistics);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|Simulations")
	static void ApplyExternalImpulse(UTetrahedralMesh* TetMesh, UDynamicMesh* DynamicMesh, UTetrahedronDynamicMeshBinding* Binding, const FVector& Location, const FVector& Impulse);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|Simulations")
	static void ApplyExternalImpulseDirect(UTetrahedralMesh* TetMesh, UDynamicMesh* DynamicMesh, UTetrahedronDynamicMeshBinding* Binding, const int FaceIndex, const FVector& BaryCoords, const FVector& Impulse);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh")
	static UTetrahedralMesh* ScaleTetrahedralMesh(UTetrahedralMesh* TetMesh, UDynamicMesh* DynamicMesh, UTetrahedronDynamicMeshBinding* Binding, float ScaleFactor);

	// Transforms ONLY TetMesh vertices!
	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh")
	static UTetrahedralMesh* TransformTetrahedralMeshVertices(UTetrahedralMesh* TetMesh, const FTransform& Transform);

	// DEBUG
	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|DebugTools")
	static void PrintDynamicMesh(const UDynamicMesh* DynamicMesh);


	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|DebugTools")
	static void PrintDynamicMeshOBJ(const UDynamicMesh* DynamicMesh);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh")
	static void AddOneWayCouplingOnIntersection(UTetrahedralMesh* TetMesh, const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh")
	static void AddOneWayCouplingWithSkeleton(UTetrahedralMesh* TetMesh, USkeleton* Skeleton, const FTransform& SkeletonTransform, const float DistanceThreshold);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh")
	static void AddTwoWayCouplingWithSkeleton(UTetrahedralMesh* TetMesh, USkeleton* Skeleton, const FTransform& SkeletonTransform, const float DistanceThreshold, const bool TakeNearest);

	UFUNCTION(BlueprintCallable, Category = "SoftBodySimulation|TetrahedralMesh")
	static void DropFirstOneWayCoupling(UTetrahedralMesh* TetMesh);
};
