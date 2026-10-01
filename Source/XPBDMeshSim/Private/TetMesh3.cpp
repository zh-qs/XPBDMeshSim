#include "TetMesh3.h"

using namespace UE::Geometry;

void FTetMesh3::Clear()
{
	Vertices.Empty();
	Tetrahedra.Empty();
}

int FTetMesh3::AppendVertex(const VectorType& vertex)
{
	Vertices.Add(vertex);

	return Vertices.Num() - 1;
}

int FTetMesh3::AppendTetrahedron(const FIndex4i& tet)
{
	if (IsVertex(tet[0]) == false || IsVertex(tet[1]) == false || IsVertex(tet[2]) == false || IsVertex(tet[3]) == false)
	{
		checkSlow(false);
		return InvalidID;
	}
	if (tet[0] == tet[1] || tet[0] == tet[2] || tet[0] == tet[3] || tet[1] == tet[2] || tet[1] == tet[3] || tet[2] == tet[3])
	{
		checkSlow(false);
		return InvalidID;
	}

	// now safe to insert triangle
	Tetrahedra.Add(tet);

	return Tetrahedra.Num() - 1;
}

FTetMesh3::FTetMesh3()
{
}

FTetMesh3::FTetMesh3(const FTetMesh3& Other) : Vertices{ Other.Vertices }, Tetrahedra{ Other.Tetrahedra }
{
}

FTetMesh3::FTetMesh3(FTetMesh3&& Other) : Vertices{ MoveTemp(Other.Vertices) }, Tetrahedra{ MoveTemp(Other.Tetrahedra) }
{
}

const FTetMesh3& FTetMesh3::operator=(const FTetMesh3& Other)
{
	Vertices = Other.Vertices;
	Tetrahedra = Other.Tetrahedra;

	return *this;
}

const FTetMesh3& FTetMesh3::operator=(FTetMesh3&& Other)
{
	Vertices = MoveTemp(Other.Vertices);
	Tetrahedra = MoveTemp(Other.Tetrahedra);

	return *this;
}

FTetMesh3::~FTetMesh3() = default;