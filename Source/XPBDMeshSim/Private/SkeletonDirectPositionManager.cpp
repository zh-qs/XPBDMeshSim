#include "SkeletonDirectPositionManager.h"

void FSkeletonDirectPositionManager::UpdateSkeletonTransforms(const TArray<FVector>& boneParametrization)
{
	const auto& refSkeleton = skeleton.GetReferenceSkeleton();
	const auto& bones = refSkeleton.GetRefBonePose();

	//UE_LOG(LogTemp, Log, TEXT("Base pos: (%.2f  %.2f  %.2f)"), boneParametrization[0].X, boneParametrization[0].Y, boneParametrization[0].Z);
	//transforms[0] = bones[0] * FTransform(boneParametrization[0]);
	transforms[0] = FTransform(boneParametrization[0]);

	for (int i = 1; i < transforms.Num(); ++i)
	{
		const auto& bone = bones[i];
		const auto parent = refSkeleton.GetParentIndex(i);
		const auto& q1 = boneParametrization[i][0],
			q2 = boneParametrization[i][1];
		//UE_LOG(LogTemp, Log, TEXT("%d: q1 = %f, q2 = %f"), i, q1, q2);
		transforms[i] = GetTransformedRelativeBoneTransform(i, q1, q2) * transforms[parent];
	}


	//UE_LOG(LogTemp, Log, TEXT("SKELETON POSITIONS:"));
	for (int i = 0; i < transforms.Num(); ++i)
	{
		const auto& bone = transforms[i].GetTranslation();
		//UE_LOG(LogTemp, Log, TEXT("    Bone %d: (%.2f  %.2f  %.2f)"), i, bone.X, bone.Y, bone.Z);
	}
}