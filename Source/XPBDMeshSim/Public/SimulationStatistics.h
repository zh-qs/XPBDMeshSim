#pragma once

#include "CoreMinimal.h"
#include "SimulationStatistics.generated.h"

USTRUCT(BlueprintType)
struct FSimulationStatistics
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	float MaxTetVolumeRatio = 0;

	UPROPERTY(BlueprintReadOnly)
	float MinTetVolumeRatio = 0;

	UPROPERTY(BlueprintReadOnly)
	float AverageTetVolumeRatio = 0;

	UPROPERTY(BlueprintReadOnly)
	float StdDevTetVolumeRatio = 0;

	UPROPERTY(BlueprintReadOnly)
	float TotalVolume = 0;

	UPROPERTY(BlueprintReadOnly)
	float InitialVolume = 0;

	UPROPERTY(BlueprintReadOnly)
	float MaxStress = 0;

	UPROPERTY(BlueprintReadOnly)
	float MinStress = 0;

	UPROPERTY(BlueprintReadOnly)
	double TimeElapsedInSeconds = 0;
};