#include "TetMeshBlueprintFunctionLibrary.h"
#include "SimulationCore.h"

#include "Misc/DefaultValueHelper.h"

#include "tetgen/tetgen_wrap.h"

using namespace UE::Geometry;

int32 AddUniqueToArray(TArray<FVector3d>& array, const FVector3d& value)
{
	for (int32 i = 0; i < array.Num(); ++i)
	{
		if (FVector3d::Dist(array[i], value) < 1e-6)
		{
			return i;
		}
	}
	return array.Add(value);
}

UTetrahedralMesh* GenerateAndBindTetrahedralMeshTetgen(const UDynamicMesh* InMesh, UTetrahedralMesh* OutMesh, float Density, ETetMeshBlueprintResult& Outcome, UTetrahedronDynamicMeshBinding* Binding, const FString& Options)
{
	auto& fMesh = InMesh->GetMeshRef();
	auto& fTet = OutMesh->GetMeshRef();

	fTet.Clear();
	if (Binding != nullptr)
		Binding->Clear();

	TArray<int> NewVerticesIndices{};
	TArray<FVector3d> Vertices{};
	TArray<FIndex3i> Faces{};

	// ****** removing duplicated vertices
	auto inVertexIds = fMesh.VertexIndicesItr();
	for (const auto& idx : inVertexIds)
	{
		const auto& position = fMesh.GetVertex(idx);

		auto vertexIdx = AddUniqueToArray(Vertices, position);
		NewVerticesIndices.Add(vertexIdx);

		if (Binding != nullptr)
			Binding->AppendSingleBinding(vertexIdx, idx);
	}

	auto inTriangles = fMesh.TrianglesItr();
	for (const auto& tr : inTriangles)
	{
		Faces.Add({ NewVerticesIndices[tr[0]], NewVerticesIndices[tr[1]], NewVerticesIndices[tr[2]] });
	}
	// ******

	tetgenio_wrap in;
	in.initialize();
	in.LoadArray(Vertices.Num(), reinterpret_cast<double*>(Vertices.GetData()), Faces.Num(), reinterpret_cast<int*>(Faces.GetData()));

	tetgenio_wrap out;
	out.initialize();

	char* switches = TCHAR_TO_ANSI(*Options);
	try
	{
		tetrahedralize(switches, &in, &out, NULL);
	}
	catch (int errorCode)
	{
		UE_LOG(LogTemp, Error, TEXT("Tetgen failed with code: %d"), errorCode);
		Outcome = ETetMeshBlueprintResult::Failure;
		fTet.Clear();
		if (Binding != nullptr)
			Binding->Clear();
		return OutMesh;
	}

	int nPoints = out.numberofpoints, nTets = out.numberoftetrahedra;
	auto* points = out.pointlist;
	auto* tets = out.tetrahedronlist;

	for (int i = 0; i < nPoints; ++i)
	{
		fTet.AppendVertex(FTetMesh3::ToInternalType(FVector3d{ points[3 * i], points[3 * i + 1], points[3 * i + 2] }));
	}
	for (int i = 0; i < nTets; ++i)
	{
		fTet.AppendTetrahedron({ tets[4 * i], tets[4 * i + 1], tets[4 * i + 2], tets[4 * i + 3] });
	}

	Outcome = ETetMeshBlueprintResult::Success;

	OutMesh->GetMeshRef().CopyInitialConfiguration();
	OutMesh->GetMeshRef().InitializeMasses(Density);
	return OutMesh;
}

UTetrahedralMesh* GenerateAndBindTetrahedralMeshSimple(const UDynamicMesh* InMesh, UTetrahedralMesh* OutMesh, float Density, ETetMeshBlueprintResult& Outcome, UTetrahedronDynamicMeshBinding* Binding)
{
	// dummy algorithm
	FTetMesh3::VectorType center = { 0,0,0 };
	auto& fMesh = InMesh->GetMeshRef();
	auto& fTet = OutMesh->GetMeshRef();

	fTet.Clear();
	if (Binding != nullptr)
		Binding->Clear();

	auto inVertexIds = fMesh.VertexIndicesItr();
	for (const auto& idx : inVertexIds)
	{
		const auto& position = fMesh.GetVertex(idx);
		auto internalPos = FTetMesh3::ToInternalType(position);
		auto tetIdx = fTet.AppendVertex(internalPos);

		if (Binding != nullptr)
			Binding->AppendSingleBinding(tetIdx, idx);

		center += internalPos;
	}
	center /= fMesh.VertexCount();

	int centerIdx = fTet.AppendVertex(center);

	auto inTriangles = fMesh.TrianglesItr();
	for (const auto& tr : inTriangles)
	{
		FIndex4i tet;
		if (Binding == nullptr) // unsafe
			tet = { tr[0],tr[1],tr[2],centerIdx };
		else
			tet = {
				Binding->GetTetrahedronMeshIndex(tr[0]),
				Binding->GetTetrahedronMeshIndex(tr[1]),
				Binding->GetTetrahedronMeshIndex(tr[2]),
				centerIdx };

		if (!fTet.IsVertex(tet[0]) || !fTet.IsVertex(tet[1]) || !fTet.IsVertex(tet[2]) || !fTet.IsVertex(tet[3]))
		{
			UE_LOG(LogTemp, Error, TEXT("Triangle (%d, %d, %d) not mapped properly! (to (%d, %d, %d, %d))"), tr[0], tr[1], tr[2], tet[0], tet[1], tet[2], tet[3]);
			Outcome = ETetMeshBlueprintResult::Failure;
			fTet.Clear();
			if (Binding != nullptr)
				Binding->Clear();
			return OutMesh;
		}

		fTet.AppendTetrahedron(tet);
	}

	Outcome = ETetMeshBlueprintResult::Success;

	OutMesh->GetMeshRef().CopyInitialConfiguration();
	OutMesh->GetMeshRef().InitializeMasses(Density);
	return OutMesh;
}

UTetrahedralMesh* UTetMeshBlueprintFunctionLibrary::GenerateTetrahedralMesh(const UDynamicMesh* InMesh, UTetrahedralMesh* OutMesh, ETetrahedronMeshingAlgorithm Algorithm, float Density, ETetMeshBlueprintResult& Outcome, const FString& Options)
{
	return GenerateAndBindTetrahedralMesh(InMesh, OutMesh, Algorithm, Density, Outcome, Options, nullptr);
}

UTetrahedralMesh* UTetMeshBlueprintFunctionLibrary::GenerateAndBindTetrahedralMesh(const UDynamicMesh* InMesh, UTetrahedralMesh* OutMesh, ETetrahedronMeshingAlgorithm Algorithm, float Density, ETetMeshBlueprintResult& Outcome, const FString& Options, UTetrahedronDynamicMeshBinding* Binding)
{
	if (OutMesh == nullptr || InMesh == nullptr || Binding == nullptr)
	{
		Outcome = ETetMeshBlueprintResult::Failure;
		return OutMesh;
	}

	switch (Algorithm)
	{
	case ETetrahedronMeshingAlgorithm::Simple:
		return GenerateAndBindTetrahedralMeshSimple(InMesh, OutMesh, Density, Outcome, Binding);
	case ETetrahedronMeshingAlgorithm::Tetgen:
		return GenerateAndBindTetrahedralMeshTetgen(InMesh, OutMesh, Density, Outcome, Binding, Options);
	}

	Outcome = ETetMeshBlueprintResult::Failure;
	return OutMesh;
}

UDynamicMesh* UTetMeshBlueprintFunctionLibrary::CopyTetrahedralMeshToDynamicMesh(const UTetrahedralMesh* InMesh, UDynamicMesh* OutMesh, ETetMeshBlueprintResult& Outcome)
{
	if (InMesh == nullptr || OutMesh == nullptr)
	{
		Outcome = ETetMeshBlueprintResult::Failure;
		return OutMesh;
	}
	
	auto& fTet = InMesh->GetMeshRef();
	OutMesh->EditMesh([&](FDynamicMesh3& fMesh)
	{
		fMesh.Clear();
		for (const auto& v : fTet.GetVerticesBuffer())
			fMesh.AppendVertex(FVector3d{ v });
		for (const auto& t : fTet.GetTetrahedraBuffer())
		{
			fMesh.AppendTriangle({ t[0], t[1], t[2] });
			fMesh.AppendTriangle({ t[1], t[2], t[3] });
			fMesh.AppendTriangle({ t[2], t[3], t[0] });
			fMesh.AppendTriangle({ t[3], t[0], t[1] });
		}
	});
	
	Outcome = ETetMeshBlueprintResult::Success;
	return OutMesh;
}

UTetrahedralMesh* UTetMeshBlueprintFunctionLibrary::LoadTetrahedralMeshDataFromFile(UDynamicMesh* DynamicMesh, UTetrahedralMesh* TetMesh, UTetrahedronDynamicMeshBinding* Binding, float Density, const FString& FilePath, ETetMeshBlueprintResult& Outcome)
{
	if (TetMesh == nullptr || DynamicMesh == nullptr || Binding == nullptr)
	{
		Outcome = ETetMeshBlueprintResult::Failure;
		return TetMesh;
	}

	FString text;
	if (!FFileHelper::LoadFileToString(text, *FilePath))
	{
		Outcome = ETetMeshBlueprintResult::Failure;
		return TetMesh;
	}
	TArray<FString> lines{};
	text.ParseIntoArray(lines, TEXT("\n"), true);

	bool success = true;
	
	auto& fTet = TetMesh->GetMeshRef();

	TArray<FVector3d> MeshVertices{};
	TArray<FIndex3i> MeshTriangles{};

	fTet.Clear();
	Binding->Clear();

	for (const auto& line : lines)
	{
		TArray<FString> tokens;
		line.ParseIntoArray(tokens, TEXT(" "), true);
		tokens[tokens.Num() - 1].RemoveFromEnd(TEXT("\r"));
		if (tokens[0] == TEXT("vd"))
		{
			FVector3d v{};
			success &= FDefaultValueHelper::ParseDouble(tokens[1], v[0]);
			success &= FDefaultValueHelper::ParseDouble(tokens[2], v[1]);
			success &= FDefaultValueHelper::ParseDouble(tokens[3], v[2]);

			MeshVertices.Add(v);
		}
		else if (tokens[0] == TEXT("vm"))
		{
			FVector3f v{};
			success &= FDefaultValueHelper::ParseFloat(tokens[1], v[0]);
			success &= FDefaultValueHelper::ParseFloat(tokens[2], v[1]);
			success &= FDefaultValueHelper::ParseFloat(tokens[3], v[2]);
			fTet.AppendVertex(v);
		}
		else if (tokens[0] == TEXT("f"))
		{
			FIndex3i f{};
			success &= FDefaultValueHelper::ParseInt(tokens[1], f[0]);
			success &= FDefaultValueHelper::ParseInt(tokens[2], f[1]);
			success &= FDefaultValueHelper::ParseInt(tokens[3], f[2]);
			--f[0];
			--f[1];
			--f[2]; // indices indexed from 1 as in OBJ format

			MeshTriangles.Add(f);
		}
		else if (tokens[0] == TEXT("t"))
		{
			FIndex4i t{};
			success &= FDefaultValueHelper::ParseInt(tokens[1], t[0]);
			success &= FDefaultValueHelper::ParseInt(tokens[2], t[1]);
			success &= FDefaultValueHelper::ParseInt(tokens[3], t[2]);
			success &= FDefaultValueHelper::ParseInt(tokens[4], t[3]);
			--t[0];
			--t[1];
			--t[2];
			--t[3]; // indices indexed from 1 as in OBJ format
			fTet.AppendTetrahedron(t);
		}
		else if (tokens[0] == TEXT("b"))
		{
			int mvIdx, tvIdx;
			success &= FDefaultValueHelper::ParseInt(tokens[1], tvIdx);
			success &= FDefaultValueHelper::ParseInt(tokens[2], mvIdx);
			--tvIdx;
			--mvIdx; // indices indexed from 1 as in OBJ format
			Binding->AppendSingleBinding(tvIdx, mvIdx);
		}
	}
	if (!success)
	{
		fTet.Clear();
		Binding->Clear();
		Outcome = ETetMeshBlueprintResult::Failure;
		return TetMesh;
	}

	DynamicMesh->EditMesh([&](FDynamicMesh3& fMesh)
	{
		fMesh.Clear();
		for (const auto& v : MeshVertices)
			fMesh.AppendVertex(v);
		for (const auto& f : MeshTriangles)
			fMesh.AppendTriangle(f);
	});

	Outcome = ETetMeshBlueprintResult::Success;

	fTet.CopyInitialConfiguration();
	fTet.InitializeMasses(Density);
	return TetMesh;
}

UDynamicMesh* UTetMeshBlueprintFunctionLibrary::LoadDynamicMeshDataFromFile(UDynamicMesh* DynamicMesh, const FString& FilePath, ETetMeshBlueprintResult& Outcome)
{
	if (DynamicMesh == nullptr)
	{
		Outcome = ETetMeshBlueprintResult::Failure;
		return DynamicMesh;
	}

	FString text;
	if (!FFileHelper::LoadFileToString(text, *FilePath))
	{
		Outcome = ETetMeshBlueprintResult::Failure;
		return DynamicMesh;
	}
	TArray<FString> lines{};
	text.ParseIntoArray(lines, TEXT("\n"), true);

	bool success = true;
	DynamicMesh->EditMesh([&](FDynamicMesh3& fMesh)
	{
		fMesh.Clear();

		for (const auto& line : lines)
		{
			TArray<FString> tokens;
			line.ParseIntoArray(tokens, TEXT(" "), true);
			tokens[tokens.Num() - 1].RemoveFromEnd(TEXT("\r"));
			if (tokens[0] == TEXT("vd"))
			{
				FVector3d v{};
				success &= FDefaultValueHelper::ParseDouble(tokens[1], v[0]);
				success &= FDefaultValueHelper::ParseDouble(tokens[2], v[1]);
				success &= FDefaultValueHelper::ParseDouble(tokens[3], v[2]);
				fMesh.AppendVertex(v);
			}
			else if (tokens[0] == TEXT("f"))
			{
				FIndex3i f{};
				success &= FDefaultValueHelper::ParseInt(tokens[1], f[0]);
				success &= FDefaultValueHelper::ParseInt(tokens[2], f[1]);
				success &= FDefaultValueHelper::ParseInt(tokens[3], f[2]);
				--f[0];
				--f[1];
				--f[2]; // indices indexed from 1 as in OBJ format
				fMesh.AppendTriangle(f);
			}
		}
		if (!success)
		{
			fMesh.Clear();
		}
	});

	if (!success)
	{
		Outcome = ETetMeshBlueprintResult::Failure;
		return DynamicMesh;
	}

	Outcome = ETetMeshBlueprintResult::Success;
	return DynamicMesh;
}

void UTetMeshBlueprintFunctionLibrary::InitializeInitialConfiguration(UTetrahedralMesh* TetMesh)
{
	TetMesh->GetMeshRef().CopyInitialConfiguration();
}

void UTetMeshBlueprintFunctionLibrary::DoSimpleCPUSimulationStep(UDynamicMesh* DynamicMesh, UTetrahedralMesh* TetMesh, UTetrahedronDynamicMeshBinding* Binding, float dt)
{
	// 'simulate' tet mesh
	auto& vertices = TetMesh->GetMeshRef().GetMutableVerticesBuffer();
	for (auto& vertex : vertices)
	{
		FTetMesh3::VectorType delta = FTetMesh3::ToInternalType(FVector3f{ FMath::RandRange(-1.0f,1.0f),FMath::RandRange(-1.0f,1.0f),FMath::RandRange(-1.0f,1.0f) }); // = FTetMesh3::VectorType::UpVector;
		delta.Normalize();
		vertex += dt * delta;
	}

	// now update dynamic mesh via binding
	DynamicMesh->EditMesh([&](FDynamicMesh3& fMesh) {
		auto indices = fMesh.VertexIndicesItr();
		for (auto idx : indices)
		{
			auto tetIdx = Binding->GetTetrahedronMeshIndex(idx);
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

void UTetMeshBlueprintFunctionLibrary::DoSimpleGPUSimulationStep(UDynamicMesh* DynamicMesh, UTetrahedralMesh* TetMesh, UTetrahedronDynamicMeshBinding* Binding,
	float dt, float YoungModulus, float PoissonRatio, float YieldStress, float PositionalDampingStiffness, const FVector& Gravity, 
	int NumSteps, int NumPosIters, EPlasticityModel PlasticityModel, bool LogToConsole,
	FSimulationStatistics& OutStatistics
)
{
	if (DynamicMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DoSimpleGPUSimulationStep: DynamicMesh is null!"));
		return;
	}
	if (TetMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DoSimpleGPUSimulationStep: TetMesh is null!"));
		return;
	}
	if (Binding == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DoSimpleGPUSimulationStep: Binding is null!"));
		return;
	}

	auto& fTet = TetMesh->GetMeshRef();

	const double start = FPlatformTime::Seconds();

	fTet.UpdateActorTransforms();

	const auto numPlasticityModel = static_cast<int>(PlasticityModel);

	simpleXPBDSimulationStep(
		fTet.GetMutableVerticesBuffer(), fTet.GetMutableVelocitiesBuffer(), fTet.GetInitialVerticesBuffer(), fTet.GetMassesBuffer(), fTet.VertexCount(),
		fTet.GetTetrahedraBuffer(), fTet.TetrahedronCount(), fTet.GetMutableInternalForcesBuffer(),
		fTet.GetActorTransformBuffer(), fTet.GetCoupledActorIndexBuffer(), fTet.GetCoupleVectorBuffer(),
		dt, YoungModulus, PoissonRatio, YieldStress, PositionalDampingStiffness, Gravity, NumSteps, NumPosIters, LogToConsole, 
		fTet.GetMutablePlasticDeformationMatrixBuffer(), fTet.GetMutablePlasticVelocityBuffer(), TetMesh->HasConstraintCoupling() ? &TetMesh->GetConstraintCoupling() : nullptr,
		numPlasticityModel);

	// now update dynamic mesh via binding
	Binding->UpdateDynamicMeshFromTetMesh(DynamicMesh, TetMesh);

	const double end = FPlatformTime::Seconds();

	// statistics 
	OutStatistics.TimeElapsedInSeconds = end - start;

	getTotalVolumes(fTet.GetVerticesBuffer(), fTet.GetInitialVerticesBuffer(), fTet.GetTetrahedraBuffer(),
		OutStatistics.InitialVolume, OutStatistics.TotalVolume);

	auto volStats = getVolumeRatioStatistics(fTet.GetVerticesBuffer(), fTet.GetInitialVerticesBuffer(), fTet.GetTetrahedraBuffer());
	OutStatistics.MaxTetVolumeRatio = volStats.max;
	OutStatistics.MinTetVolumeRatio = volStats.min;
	OutStatistics.AverageTetVolumeRatio = volStats.average;
	OutStatistics.StdDevTetVolumeRatio = volStats.stddev;

	auto stressStats = getStressStatistics(fTet.GetVerticesBuffer(), fTet.GetInitialVerticesBuffer(), fTet.GetTetrahedraBuffer(), fTet.GetMutablePlasticDeformationMatrixBuffer(), YoungModulus, PoissonRatio, numPlasticityModel);
	OutStatistics.MaxStress = stressStats.max;
	OutStatistics.MinStress = stressStats.min;
}

FTetMesh3::VectorType CenterOfMass(const TArray<FTetMesh3::VectorType>& vertices)
{
	FTetMesh3::VectorType center{ 0,0,0 };
	for (const auto& v : vertices)
		center += v;

	center /= vertices.Num();
	return center;
}

UTetrahedralMesh* UTetMeshBlueprintFunctionLibrary::ScaleTetrahedralMesh(UTetrahedralMesh* TetMesh, UDynamicMesh* DynamicMesh, UTetrahedronDynamicMeshBinding* Binding, float ScaleFactor)
{
	if (ensure(TetMesh != nullptr) == false || ensure(DynamicMesh != nullptr) == false || ensure(Binding != nullptr) == false)
		return TetMesh;

	auto& vertices = TetMesh->GetMeshRef().GetMutableVerticesBuffer();

	auto center = CenterOfMass(vertices);

	for (auto& v : vertices)
		v = (v - center) * ScaleFactor + center;

	Binding->UpdateDynamicMeshFromTetMesh(DynamicMesh, TetMesh);

	return TetMesh;
}

UTetrahedralMesh* UTetMeshBlueprintFunctionLibrary::TransformTetrahedralMeshVertices(UTetrahedralMesh* TetMesh, const FTransform& Transform)
{
	if (ensure(TetMesh != nullptr) == false)
		return TetMesh;

	auto& vertices = TetMesh->GetMeshRef().GetMutableVerticesBuffer();

	for (auto& v : vertices) {
		v = FTetMesh3::ToInternalType(Transform.TransformPosition(FVector{ v }));
	}

	return TetMesh;
}

void UTetMeshBlueprintFunctionLibrary::ApplyExternalImpulse(UTetrahedralMesh* TetMesh, UDynamicMesh* DynamicMesh, UTetrahedronDynamicMeshBinding* Binding, const FVector& Location, const FVector& Impulse)
{
	auto faces = DynamicMesh->GetMeshRef().TrianglesItr();
	const auto& fMesh = DynamicMesh->GetMeshRef();

	double minDistSq = INFINITY;
	FIndex3i minFace;
	FVector minPointOnFace;
	for (const auto& face : faces)
	{
		const auto pointOnFace = FMath::ClosestPointOnTriangleToPoint(Location, fMesh.GetVertexRef(face[0]), fMesh.GetVertexRef(face[1]), fMesh.GetVertexRef(face[2]));
		const auto distSq = FVector::DistSquared(pointOnFace, Location);
		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			minFace = face;
			minPointOnFace = pointOnFace;
		}
	}

	const auto baryCoords = FMath::ComputeBaryCentric2D(minPointOnFace, fMesh.GetVertexRef(minFace[0]), fMesh.GetVertexRef(minFace[1]), fMesh.GetVertexRef(minFace[2]));
	auto& velocities = TetMesh->GetMeshRef().GetMutableVelocitiesBuffer();
	for (int i = 0; i < 3; ++i)
		velocities[Binding->GetTetrahedronMeshIndex(minFace[i])] += FTetMesh3::ToInternalType(baryCoords[i] * Impulse);

	UE_LOG(LogTemp, Log, TEXT("Added impulse at face (%d,%d,%d), bary (%f,%f,%f) of strength (%f,%f,%f)"), minFace[0], minFace[1], minFace[2], baryCoords[0], baryCoords[1], baryCoords[2], Impulse[0], Impulse[1], Impulse[2]);
}

void UTetMeshBlueprintFunctionLibrary::ApplyExternalImpulseDirect(UTetrahedralMesh* TetMesh, UDynamicMesh* DynamicMesh, UTetrahedronDynamicMeshBinding* Binding, const int FaceIndex, const FVector& BaryCoords, const FVector& Impulse)
{
	const auto& face = DynamicMesh->GetMeshRef().GetTriangleRef(FaceIndex);
	auto& velocities = TetMesh->GetMeshRef().GetMutableVelocitiesBuffer();

	for (int i=0;i<3;++i)
		velocities[Binding->GetTetrahedronMeshIndex(face[i])] += FTetMesh3::ToInternalType(BaryCoords[i] * Impulse);

	UE_LOG(LogTemp, Log, TEXT("Added impulse at face (%d,%d,%d), bary (%f,%f,%f) of strength (%f,%f,%f)"), face[0], face[1], face[2], BaryCoords[0], BaryCoords[1], BaryCoords[2], Impulse[0], Impulse[1], Impulse[2]);
}

void UTetMeshBlueprintFunctionLibrary::PrintDynamicMesh(const UDynamicMesh* DynamicMesh)
{
	if (ensure(DynamicMesh != nullptr) == false)
		return;

	const auto& fMesh = DynamicMesh->GetMeshRef();

	FString text{ "DYNAMIC MESH:\n" };

	text.Append("  Vertices:\n    [");
	auto inVertices = fMesh.VerticesItr();
	for (const auto& v : inVertices)
		text.Appendf(TEXT("(%.2f, %.2f, %.2f), "), v[0], v[1], v[2]);

	text.Append("]\n  Triangles:\n    [");
	auto inTriangles = fMesh.TrianglesItr();
	for (const auto& tr : inTriangles)
		text.Appendf(TEXT("(%d, %d, %d), "), tr[0], tr[1], tr[2]);

	text.Append("]");
	UE_LOG(LogTemp, Log, TEXT("%s"), *text);
}

void UTetMeshBlueprintFunctionLibrary::PrintDynamicMeshOBJ(const UDynamicMesh* DynamicMesh)
{
	if (ensure(DynamicMesh != nullptr) == false)
		return;

	const auto& fMesh = DynamicMesh->GetMeshRef();

	FString text{ "DYNAMIC MESH:\n" };

	auto inVertices = fMesh.VerticesItr();
	for (const auto& v : inVertices)
		text.Appendf(TEXT("v %.6f %.6f %.6f\n"), v[0], v[1], v[2]);

	auto inTriangles = fMesh.TrianglesItr();
	for (const auto& tr : inTriangles)
		text.Appendf(TEXT("f %d %d %d\n"), tr[0] + 1, tr[1] + 1, tr[2] + 1);

	UE_LOG(LogTemp, Log, TEXT("%s"), *text);
}

void UTetMeshBlueprintFunctionLibrary::AddOneWayCouplingOnIntersection(UTetrahedralMesh* TetMesh, const AActor* Actor)
{
	if (ensure(TetMesh != nullptr) == false || ensure(Actor != nullptr) == false)
		return;

	auto& fTetMesh = TetMesh->GetMeshRef();
	const auto& vertices = fTetMesh.GetVerticesBuffer();

	// primitive box solution: couple all points inside actor's bounding box
	auto bbox = Actor->GetComponentsBoundingBox();
	auto center = Actor->GetTransform().GetTranslation();

	FOneWayVertexCoupling coupling{ Actor->GetTransform() };
	for (int i = 0; i < vertices.Num(); ++i)
	{
		FVector vtx_vec{ vertices[i] };
		if (bbox.IsInsideOrOn(vtx_vec))
		{
			coupling.AddCoupledVertex(i, FTetMesh3::ToInternalType(vtx_vec - center));
		}
	}

	if (!coupling.Empty())
		fTetMesh.AddOneWayCoupling(MoveTemp(coupling));
}

void UTetMeshBlueprintFunctionLibrary::AddOneWayCouplingWithSkeleton(UTetrahedralMesh* TetMesh, USkeleton* Skeleton, const FTransform& SkeletonTransform, const float DistanceThreshold)
{
	if (ensure(TetMesh != nullptr) == false || ensure(Skeleton != nullptr) == false)
		return;

	auto& fTetMesh = TetMesh->GetMeshRef();
	const auto& vertices = fTetMesh.GetVerticesBuffer();

	TetMesh->SetCoupledSkeleton(*Skeleton, SkeletonTransform);

	const auto& skeletonManager = TetMesh->GetSkeletonManager();

	for (int i=0;i<skeletonManager.Num();++i)
	{
		const auto& bone = skeletonManager.GetBoneTransform(i);
		auto t = bone.GetTranslation();
		FOneWayVertexCoupling coupling{ bone };

		for (int j = 0; j < vertices.Num(); ++j)
		{
			auto diff = FVector{ vertices[j] } - t;
			if (diff.Length() <= DistanceThreshold)
			{
				coupling.AddCoupledVertex(j, FTetMesh3::ToInternalType(diff));
			}
		}

		if (!coupling.Empty())
			fTetMesh.AddOneWayCoupling(MoveTemp(coupling));
	}
}

void UTetMeshBlueprintFunctionLibrary::AddTwoWayCouplingWithSkeleton(UTetrahedralMesh* TetMesh, USkeleton* Skeleton, const FTransform& SkeletonTransform, const float DistanceThreshold, const bool TakeNearest)
{
	if (ensure(TetMesh != nullptr) == false || ensure(Skeleton != nullptr) == false)
		return;

	auto& fTetMesh = TetMesh->GetMeshRef();
	const auto& vertices = fTetMesh.GetVerticesBuffer();
	TArray<FOneWayVertexCoupling> tempCouplings = { fTetMesh.GetCouplings() };
	fTetMesh.ClearCouplings();

	TetMesh->SetCoupledSkeleton(*Skeleton, SkeletonTransform);

	const auto& skeletonManager = TetMesh->GetSkeletonManager();
	bool couplingValid = true;

	for (int i = 0; i < skeletonManager.Num(); ++i)
	{
		const auto& bone = skeletonManager.GetBoneTransform(i);
		auto t1 = bone.GetTranslation();
		FOneWayVertexCoupling coupling{ bone };

		float min_dist = INFINITY;
		int minj = -1;
		FVector min_diff;

		for (int j = 0; j < vertices.Num(); ++j)
		{
			const auto v = FVector{ vertices[j] };
			auto diff = FVector{ vertices[j] } - t1;
			const auto dist = diff.Length();

			if (dist <= DistanceThreshold)
			{
				if (TakeNearest)
				{
					if (dist < min_dist)
					{
						min_dist = dist;
						minj = j;
						min_diff = diff;
					}
				}
				else
				{
					coupling.AddCoupledVertex(j, FTetMesh3::ToInternalType(diff));
				}
			}
		}

		if (TakeNearest && minj >= 0)
		{
			fTetMesh.GetMutableInitialVerticesBuffer()[minj] = FTetMesh3::ToInternalType(t1);
			fTetMesh.GetMutableVerticesBuffer()[minj] = FTetMesh3::ToInternalType(t1);
			coupling.AddCoupledVertex(minj, FTetMesh3::ToInternalType(min_diff));
		}

		if (!coupling.Empty())
		{
			fTetMesh.AddOneWayCoupling(MoveTemp(coupling));
		}
		else
		{
			if (couplingValid)
			{
				UE_LOG(LogTemp, Error, TEXT("No vertex was coupled with bone no. %d. This will make the inertia matrix equal 0. Switching to one-way coupling."), i);
				couplingValid = false;
			}
		}
	}

	if (couplingValid)
	{
		TetMesh->InitConstraintCoupling();
	}
	for (auto&& c : tempCouplings)
		fTetMesh.AddOneWayCoupling(MoveTemp(c));
}

void UTetMeshBlueprintFunctionLibrary::DropFirstOneWayCoupling(UTetrahedralMesh* TetMesh)
{
	TetMesh->GetMeshRef().RemoveOneWayCoupling(0);
}
