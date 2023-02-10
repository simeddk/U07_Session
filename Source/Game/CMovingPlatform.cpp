#include "CMovingPlatform.h"
#include "Materials/MaterialInstanceConstant.h"

ACMovingPlatform::ACMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	SetMobility(EComponentMobility::Movable);

	ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Geometry/Meshes/1M_Cube.1M_Cube'"));
	if (meshAsset.Succeeded())
		GetStaticMeshComponent()->SetStaticMesh(meshAsset.Object);

	ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> materialAsset(TEXT("MaterialInstanceConstant'/Game/Materials/MAT_Platform_Red.MAT_Platform_Red'"));
	if (materialAsset.Succeeded())
		GetStaticMeshComponent()->SetMaterial(0, materialAsset.Object);
}

void ACMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetReplicates(true);
		SetReplicateMovement(true);
	}

	GlobalStartLocation = GetActorLocation();
	GlobalTargetLocation = GetTransform().TransformPosition(TargetLocation);;
}

void ACMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ActiveTrigger < 1) return;

	if (HasAuthority())
	{
		FVector location = GetActorLocation();
		
		float totalDistance = (GlobalStartLocation - GlobalTargetLocation).Size();
		float currentDistance = (GlobalStartLocation - location).Size();

		if (currentDistance > totalDistance)
		{
			FVector temp = GlobalStartLocation;
			GlobalStartLocation = GlobalTargetLocation;
			GlobalTargetLocation = temp;
		}

		FVector direction = (GlobalTargetLocation - GlobalStartLocation).GetSafeNormal();
		location += Speed * direction * DeltaTime;
		SetActorLocation(location);
	}
}

void ACMovingPlatform::AddActiveTrigger()
{
	ActiveTrigger++;
}

void ACMovingPlatform::RemoveActiveTrigger()
{
	if (ActiveTrigger > 0)
		ActiveTrigger--;
}
