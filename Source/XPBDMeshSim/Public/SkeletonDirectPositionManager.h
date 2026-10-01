#pragma once

#include "CoreMinimal.h"
#include "Animation/Skeleton.h"

class FSkeletonDirectPositionManager
{
	USkeleton& skeleton;
	TArray<FTransform> transforms;
	TArray<TArray<int>> children;
	
	void PrepareSkeletonTransforms(const FTransform& initialRootBoneTransform)
	{
		const auto& refSkeleton = skeleton.GetReferenceSkeleton();
		const auto& bones = refSkeleton.GetRefBonePose();
		transforms.SetNum(bones.Num());

		for (int i = 0; i < transforms.Num(); ++i)
		{
			const auto& bone = bones[i];
			const auto parent = refSkeleton.GetParentIndex(i);
			if (parent == INDEX_NONE)
				transforms[i] = bone * initialRootBoneTransform;
			else
				transforms[i] = bone * transforms[parent];
		}
	}

	void FillChildrenArray()
	{
		children.SetNum(Num());
		for (int i = 0; i < Num(); ++i)
		{
			skeleton.GetChildBones(i, children[i]);
		}
	}
	
public:
	FSkeletonDirectPositionManager(USkeleton& skeleton, const FTransform& initialRootBoneTransform) : skeleton(skeleton)
	{
		PrepareSkeletonTransforms(initialRootBoneTransform);
		FillChildrenArray();
	}

	const FTransform& GetBoneTransform(int idx) const
	{
		return transforms[idx];
	}

	void SetBoneTransform(int idx, const FTransform& transform)
	{
		transforms[idx] = transform;
	}

	const FTransform& GetInitialRelativeBoneTransform(int idx) const
	{
		return skeleton.GetReferenceSkeleton().GetRefBonePose()[idx];
	}

	int GetBoneParent(int idx) const
	{
		return skeleton.GetReferenceSkeleton().GetParentIndex(idx);
	}

	const TArray<int>& GetBoneChildren(int idx) const
	{
		return children[idx];
	}

	bool IsLeaf(int idx) const
	{
		return GetBoneChildren(idx).IsEmpty();
	}

	int Num() const { return transforms.Num(); }

	FTransform GetTransformedRelativeBoneTransform(int idx, float q1, float q2) const
	{
		return GetInitialRelativeBoneTransform(idx) * FTransform(FQuat::MakeFromRotationVector({ q1, 0, 0 })) * FTransform(FQuat::MakeFromRotationVector({ 0, 0, q2 }));
	}

	void UpdateSkeletonTransforms(const TArray<FVector>& boneParametrization);
};