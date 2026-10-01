#include "DeformableDynamicMeshComponent.h"

UDeformableDynamicMeshComponent::UDeformableDynamicMeshComponent() : Super()
{
	TetMeshObject = CreateDefaultSubobject<UTetrahedralMesh>(TEXT("TetMesh"));
	Binding = CreateDefaultSubobject<UTetrahedronDynamicMeshBinding>(TEXT("Binding"));
	SecondMeshObject = CreateDefaultSubobject<UDynamicMesh>(TEXT("DynamicMesh2"));
}

void UDeformableDynamicMeshComponent::PostLoad()
{
	Super::PostLoad();
	InitSubobjects();
}

void UDeformableDynamicMeshComponent::InitSubobjectsUnsafe()
{
	TetMeshObject = CreateDefaultSubobject<UTetrahedralMesh>(TEXT("TetMesh"));
	Binding = CreateDefaultSubobject<UTetrahedronDynamicMeshBinding>(TEXT("Binding"));
	SecondMeshObject = CreateDefaultSubobject<UDynamicMesh>(TEXT("DynamicMesh2"));
}

void UDeformableDynamicMeshComponent::InitSubobjects()
{
	if (ensure(TetMeshObject != nullptr) == false)
		TetMeshObject = NewObject<UTetrahedralMesh>(this, TEXT("TetMesh"));
	if (ensure(Binding != nullptr) == false)
		Binding = NewObject<UTetrahedronDynamicMeshBinding>(this, TEXT("Binding"));
	if (ensure(SecondMeshObject != nullptr) == false)
		SecondMeshObject = NewObject<UDynamicMesh>(this, TEXT("DynamicMesh2"));
}

void UDeformableDynamicMeshComponent::SetDisplayMode(EDeformableDisplayMode DisplayMode)
{
	if (Mode == DisplayMode)
		return;

	Mode = DisplayMode;
	
	// swap DynamicMeshes
	auto* swap = GetDynamicMesh();
	SetDynamicMesh(SecondMeshObject);
	SecondMeshObject = swap;

	switch (Mode)
	{
	case EDeformableDisplayMode::TriangleMesh:
		Binding->SetActive(true);
		break;
	case EDeformableDisplayMode::Wireframe:
		Binding->SetActive(false);
		break;
	}
}