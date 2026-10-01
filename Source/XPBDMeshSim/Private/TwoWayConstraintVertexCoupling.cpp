#include "TwoWayConstraintVertexCoupling.h"

FTwoWayConstraintVertexCoupling::FTwoWayConstraintVertexCoupling(FSkeletonDirectPositionManager& skeletonManager, FTetMesh3& tetMesh) : tetMesh(tetMesh)
{
	jointVertexIndices.SetNum(skeletonManager.Num());
	jointParentIndices.SetNum(skeletonManager.Num());
	jointDistances.SetNum(skeletonManager.Num());

	for (int i = 0; i < skeletonManager.Num(); ++i)
	{
		const auto& fCoupl = tetMesh.GetCoupling(i);
		jointVertexIndices[i] = fCoupl.GetCoupledVertexIndices()[0];
		if (i > 0)
		{
			const auto parent = skeletonManager.GetBoneParent(i);
			jointParentIndices[i] = jointVertexIndices[parent];
			jointDistances[i] = FVector::Dist(skeletonManager.GetBoneTransform(i).GetTranslation(), skeletonManager.GetBoneTransform(parent).GetTranslation());//skeletonManager.GetInitialRelativeBoneTransform(i).GetTranslation().Length();
		}
	}

	// we don't need one way couplings from now
	tetMesh.ClearCouplings();
}

void FTwoWayConstraintVertexCoupling::EnforceConstraints()
{
	auto& vertices = tetMesh.GetMutableVerticesBuffer();
	auto& invMasses = tetMesh.GetMassesBuffer();

	for (int i = 1; i < jointVertexIndices.Num(); ++i)
	{
		auto& v = vertices[jointVertexIndices[i]];
		auto& p = vertices[jointParentIndices[i]];
		const auto& mv = invMasses[jointVertexIndices[i]],
			mp = invMasses[jointParentIndices[i]];

		const auto d = (v - p).Length();

		const auto dv = mv / (mv + mp) * (v - p) * (jointDistances[i] - d) / d;
		const auto dp = mp / (mv + mp) * (p - v) * (jointDistances[i] - d) / d;

		v += dv;
		p += dp;
	}
}
