#include "TetrahedronDynamicMeshBinding.h"

UTetrahedronDynamicMeshBinding::UTetrahedronDynamicMeshBinding()
{
}

UTetrahedronDynamicMeshBinding::~UTetrahedronDynamicMeshBinding() = default;

void UTetrahedronDynamicMeshBinding::Clear()
{
	TetIdxToDynamicMeshIdx.Empty();
	DynamicMeshIdxToTetIdx.Empty();
}

void UTetrahedronDynamicMeshBinding::AppendSingleBinding(int tetIdx, int dmIdx)
{
	if (tetIdx >= TetIdxToDynamicMeshIdx.Num())
		TetIdxToDynamicMeshIdx.SetNum(tetIdx + 1);
	if (dmIdx >= DynamicMeshIdxToTetIdx.Num())
		DynamicMeshIdxToTetIdx.SetNum(dmIdx + 1);
	TetIdxToDynamicMeshIdx[tetIdx] = dmIdx + 1;
	DynamicMeshIdxToTetIdx[dmIdx] = tetIdx + 1;
}

int UTetrahedronDynamicMeshBinding::GetTetrahedronMeshIndex(int dmIdx)
{
	if (dmIdx < 0 || dmIdx >= DynamicMeshIdxToTetIdx.Num())
		return -1;

	return DynamicMeshIdxToTetIdx[dmIdx] - 1;
}

void UTetrahedronDynamicMeshBinding::PrintBinding()
{
	FString text{ "BINDING:\n" };
	text.Append("  TetIdxToDynamicMeshIdx:\n    [");
	for (const auto& idx : TetIdxToDynamicMeshIdx)
		text.Appendf(TEXT("%d, "), idx);
	text.Append("]\n  DynamicMeshIdxToTetIdx:\n    [");
	for (const auto& idx : DynamicMeshIdxToTetIdx)
		text.Appendf(TEXT("%d, "), idx);
	text.Append("]");
	UE_LOG(LogTemp, Log, TEXT("%s"), *text);
}

void UTetrahedronDynamicMeshBinding::UpdateDynamicMeshFromTetMesh(UDynamicMesh* DynamicMesh, const UTetrahedralMesh* TetMesh)
{
	const auto& vertices = TetMesh->GetMeshRef().GetVerticesBuffer();
	DynamicMesh->EditMesh([&](FDynamicMesh3& fMesh) {
		auto indices = fMesh.VertexIndicesItr();
		for (auto idx : indices)
		{
			auto tetIdx = IsActive() ? GetTetrahedronMeshIndex(idx) : idx;
			if (!TetMesh->GetMeshRef().IsVertex(tetIdx))
			{
				UE_LOG(LogTemp, Error, TEXT("Dynamic mesh vertex no. %d is not properly mapped! (to %d)"), idx, tetIdx);
			}
			else
			{
				fMesh.SetVertex(idx, FVector3d{ vertices[tetIdx] });
			}
		}
	});
}