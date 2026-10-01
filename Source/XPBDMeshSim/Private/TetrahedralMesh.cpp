#include "TetrahedralMesh.h"


UTetrahedralMesh::UTetrahedralMesh()
{
	m_TetMesh = MakeUnique<FTetMesh3>();
}

void UTetrahedralMesh::SetCoupledSkeleton(USkeleton& skeleton, const FTransform& initialRootBoneTransform)
{
	m_SkeletonManager = MakeUnique<FSkeletonDirectPositionManager>(skeleton, initialRootBoneTransform);
}

void UTetrahedralMesh::InitConstraintCoupling()
{
	check(HasSkeletonManager());
	m_ConstraintCoupling = MakeUnique<FTwoWayConstraintVertexCoupling>(GetSkeletonManager(), GetMeshRef());
}

void UTetrahedralMesh::DestroyConstraintCoupling()
{
	if (HasConstraintCoupling())
		m_ConstraintCoupling.Reset();
}

void UTetrahedralMesh::PrintMesh()
{
	FString text{ "TETRAHEDRON MESH:\n" };
	text.Append("  Vertices:\n    [");
	for (const auto& v : m_TetMesh->GetVerticesBuffer())
		text.Appendf(TEXT("(%.2f, %.2f, %.2f), "), v[0], v[1], v[2]);
	text.Append("]\n  Initial Vertices:\n    [");
	for (const auto& v : m_TetMesh->GetInitialVerticesBuffer())
		text.Appendf(TEXT("(%.2f, %.2f, %.2f), "), v[0], v[1], v[2]);
	text.Append("]\n  Tetrahedra:\n    [");
	for (const auto& tet : m_TetMesh->GetTetrahedraBuffer())
		text.Appendf(TEXT("(%d, %d, %d, %d), "), tet[0], tet[1], tet[2], tet[3]);
	text.Append("]");
	UE_LOG(LogTemp, Log, TEXT("%s"), *text);
}

void UTetrahedralMesh::PrintMeshOBJ()
{
	FString text{ "TETRAHEDRON MESH:\n" };
	for (const auto& v : m_TetMesh->GetVerticesBuffer())
		text.Appendf(TEXT("v %.6f %.6f %.6f\n"), v[0], v[1], v[2]);
	for (const auto& tet : m_TetMesh->GetTetrahedraBuffer())
	{
		text.Appendf(TEXT("f %d %d %d\n"), tet[0] + 1, tet[1] + 1, tet[2] + 1);
		text.Appendf(TEXT("f %d %d %d\n"), tet[1] + 1, tet[3] + 1, tet[2] + 1);
		text.Appendf(TEXT("f %d %d %d\n"), tet[0] + 1, tet[2] + 1, tet[3] + 1);
		text.Appendf(TEXT("f %d %d %d\n"), tet[0] + 1, tet[3] + 1, tet[1] + 1);
	}
	UE_LOG(LogTemp, Log, TEXT("%s"), *text);
}

int UTetrahedralMesh::GetVertexCount()
{
	if (!m_TetMesh.IsValid())
		return -1;

	return m_TetMesh->VertexCount();
}

int UTetrahedralMesh::GetTetrahedronCount()
{
	if (!m_TetMesh.IsValid())
		return -1;

	return m_TetMesh->TetrahedronCount();
}
