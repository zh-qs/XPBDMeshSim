#pragma once

#include "CoreMinimal.h"
#include <IndexTypes.h>
#include "OneWayVertexCoupling.h"

/**
 * 
 */
class FTetMesh3
{
public:
	using VectorType = FVector3f;
	static VectorType ToInternalType(const FVector3f& v) { return v; }
	static VectorType ToInternalType(const FVector3d& v) { return VectorType{ v }; }
protected:
	

	static constexpr inline int InvalidID = -1;

	TArray<VectorType> Vertices{};

	// array of initial coordinates of vertices (undeformed configuration)
	TArray<VectorType> InitialVertices{};

	TArray<UE::Geometry::FIndex4i> Tetrahedra{};
	TArray<FMatrix> PlasticDeformationMatrices{};
	TArray<FMatrix> PlasticVelocities{};

	TArray<VectorType> Velocities{};
	TArray<float> Masses{};

	TArray<VectorType> ExternalImpulses{};
	TArray<VectorType> InternalForces{};

	// couplings
	TArray<FOneWayVertexCoupling> Couplings{};
	TArray<FMatrix> ActorTransforms{};
	TArray<int> CoupledActorIndices{};
	TArray<VectorType> CoupleVectors{};

public:

	FTetMesh3();

	FTetMesh3(const FTetMesh3& Other);
	FTetMesh3(FTetMesh3&& Other);

	const FTetMesh3& operator=(const FTetMesh3& Other);
	const FTetMesh3& operator=(FTetMesh3&& Other);

	~FTetMesh3();

	/** @return number of vertices in the mesh */
	int VertexCount() const
	{
		return (int)Vertices.Num();
	}
	/** @return number of tetrahedra in the mesh */
	int TetrahedronCount() const
	{
		return (int)Tetrahedra.Num();
	}

	/** @return upper bound on vertex IDs used in the mesh, i.e. all vertex IDs in use are < MaxVertexID */
	int MaxVertexID() const
	{
		return (int)Vertices.Num();
	}
	/** @return upper bound on tetrahedron IDs used in the mesh, i.e. all tetrahedra IDs in use are < MaxTetrahedronID */
	int MaxTetrahedronID() const
	{
		return (int)Tetrahedra.Num();
	}

	/** @return true if VertexID is a valid vertex in this mesh */
	inline bool IsVertex(int VertexID) const
	{
		return VertexID >= 0 && VertexID < MaxVertexID();
	}
	
	/** @return true if TetrahedronID is a valid tetrahedron in this mesh */
	inline bool IsTetrahedron(int TetrahedronID) const
	{
		return TetrahedronID >= 0 && TetrahedronID < MaxTetrahedronID();
	}

	void Clear();

	int AppendVertex(const VectorType& vertex);
	int AppendTetrahedron(const UE::Geometry::FIndex4i& tet);
	int AppendTetrahedron(int v1, int v2, int v3, int v4) { return AppendTetrahedron(UE::Geometry::FIndex4i{ v1, v2, v3, v4 }); }

	TArray<VectorType>& GetMutableVerticesBuffer()
	{
		return Vertices;
	}

	const TArray<VectorType>& GetVerticesBuffer() const
	{
		return Vertices;
	}

	TArray<VectorType>& GetMutableVelocitiesBuffer()
	{
		return Velocities;
	}

	const TArray<VectorType>& GetVelocitiesBuffer() const
	{
		return Velocities;
	}

	const TArray<VectorType>& GetInternalForcesBuffer() const
	{
		return InternalForces;
	}
	
	TArray<VectorType>& GetMutableInternalForcesBuffer()
	{
		return InternalForces;
	}

	TArray<float>& GetMutableMassesBuffer()
	{
		return Masses;
	}

	TArray<float>& GetMassesBuffer()
	{
		return Masses;
	}
	
	const TArray<VectorType>& GetInitialVerticesBuffer() const
	{
		return InitialVertices;
	}

	TArray<VectorType>& GetMutableInitialVerticesBuffer()
	{
		return InitialVertices;
	}

	TArray<FMatrix>& GetMutablePlasticDeformationMatrixBuffer()
	{
		return PlasticDeformationMatrices;
	}
	
	TArray<FMatrix>& GetMutablePlasticVelocityBuffer()
	{
		return PlasticVelocities;
	}

	const TArray<UE::Geometry::FIndex4i>& GetTetrahedraBuffer() const
	{
		return Tetrahedra;
	}

	void InitializeMasses(float Density) {
		Velocities.SetNum(Vertices.Num());
		Masses.SetNum(Vertices.Num());
		InternalForces.SetNum(Vertices.Num());
		PlasticDeformationMatrices.Init(FMatrix::Identity, Tetrahedra.Num());
		PlasticVelocities.Init(FMatrix{EForceInit::ForceInitToZero}, Tetrahedra.Num());

		for (int i = 0; i < Vertices.Num(); ++i)
			Masses[i] = Velocities[i][0] = Velocities[i][1] = Velocities[i][2] = 0;

		for (int i = 0; i < Tetrahedra.Num(); ++i)
		{
			const UE::Geometry::FIndex4i& tet = Tetrahedra[i];
			const FVector3f& x1 = Vertices[tet[0]],
				& x2 = Vertices[tet[1]],
				& x3 = Vertices[tet[2]],
				& x4 = Vertices[tet[3]];
			const float vol = (0.25f / 6.0f) * Density * FMath::Abs(
				(x1[0] - x4[0]) * (x2[1] - x4[1]) * (x3[2] - x4[2])
				+ (x2[0] - x4[0]) * (x3[1] - x4[1]) * (x1[2] - x4[2])
				+ (x3[0] - x4[0]) * (x1[1] - x4[1]) * (x2[2] - x4[2])
				- (x3[0] - x4[0]) * (x2[1] - x4[1]) * (x1[2] - x4[2])
				- (x2[0] - x4[0]) * (x1[1] - x4[1]) * (x3[2] - x4[2])
				- (x1[0] - x4[0]) * (x3[1] - x4[1]) * (x2[2] - x4[2]));
			Masses[tet[0]] += vol;
			Masses[tet[1]] += vol;
			Masses[tet[2]] += vol;
			Masses[tet[3]] += vol;
		}
		for (int i = 0; i < Vertices.Num(); ++i)
			Masses[i] = Masses[i] == 0 ? 0 : (1 / Masses[i]);
	}

	void SetInitialConfiguration(const TArray<VectorType>& array) {
		check(array.Num() == Vertices.Num());
		InitialVertices = array;		
	}
	void SetInitialConfiguration(TArray<VectorType>&& array) {
		check(array.Num() == Vertices.Num());
		InitialVertices = MoveTemp(array);
	}
	void CopyInitialConfiguration() 
	{ 
		SetInitialConfiguration(Vertices); 
		CoupledActorIndices.SetNum(Vertices.Num());
		for (int i = 0; i < CoupledActorIndices.Num(); ++i)
			CoupledActorIndices[i] = -1;
		CoupleVectors.SetNum(Vertices.Num());
	}

protected:
	// Couplings
	void UpdateCouplings() 
	{
		for (int i = 0; i < CoupledActorIndices.Num(); i++)
			CoupledActorIndices[i] = -1;
		for (int i = 0; i < Couplings.Num(); i++)
		{
			Couplings[i].UpdateCouplingsArrays(CoupledActorIndices, i, CoupleVectors);
		}
	}

public:
	// Couplings
	void AddOneWayCoupling(const FOneWayVertexCoupling& coupling)
	{
		if (coupling.Empty())
			return;
		Couplings.Add(coupling);
		UpdateCouplings();
	}

	void AddOneWayCoupling(FOneWayVertexCoupling&& coupling)
	{
		if (coupling.Empty())
			return;
		Couplings.Add(MoveTemp(coupling));
		UpdateCouplings();
	}

	void RemoveOneWayCoupling(int idx)
	{
		if (Couplings.IsEmpty())
			return;
		Couplings.RemoveAt(idx);
		UpdateCouplings();
	}

	int CouplingsNum() const { return Couplings.Num(); }

	void ClearCouplings() 
	{ 
		Couplings.Empty(); 
		UpdateCouplings();
	}

	const TArray<FOneWayVertexCoupling>& GetCouplings() const { return Couplings; }
	const FOneWayVertexCoupling& GetCoupling(int i) const { return Couplings[i]; }

	void UpdateActorTransforms()
	{
		ActorTransforms.SetNum(Couplings.Num());
		for (int i = 0; i < Couplings.Num(); i++)
		{
			ActorTransforms[i] = Couplings[i].GetMovementSourceMatrix();
		}
	}

	const TArray<FMatrix>& GetActorTransformBuffer() const
	{
		return ActorTransforms;
	}
	const TArray<int>& GetCoupledActorIndexBuffer() const
	{
		return CoupledActorIndices;
	}
	const TArray<FVector3f>& GetCoupleVectorBuffer() const
	{
		return CoupleVectors;
	}
};