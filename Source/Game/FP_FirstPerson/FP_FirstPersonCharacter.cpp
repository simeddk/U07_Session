#include "FP_FirstPersonCharacter.h"
#include "Global.h"
#include "FP_FirstPersonGameMode.h"
#include "CPlayerState.h"
#include "CBullet.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#define COLLISION_WEAPON		ECC_GameTraceChannel1

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

void AFP_FirstPersonCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFP_FirstPersonCharacter, CurrentTeam);
}

AFP_FirstPersonCharacter::AFP_FirstPersonCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0, 0, 64.f));
	Camera->bUsePawnControlRotation = true;
	
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetupAttachment(Camera);
	FP_Mesh->bCastDynamicShadow = false;
	FP_Mesh->CastShadow = false;

	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(FP_Mesh, TEXT("GripPoint"));

	WeaponRange = 5000.0f;
	WeaponDamage = 10.0f;

	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	GetMesh()->SetOwnerNoSee(true);
	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	TP_Gun->SetOwnerNoSee(true);

	FP_GunShotParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FP_GunShotParticle"));
	FP_GunShotParticle->bAutoActivate = false;
	FP_GunShotParticle->SetupAttachment(FP_Gun, "Muzzle");
	FP_GunShotParticle->SetOnlyOwnerSee(true);

	TP_GunShotParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TP_GunShotParticle"));
	TP_GunShotParticle->bAutoActivate = false;
	TP_GunShotParticle->SetupAttachment(TP_Gun, "Muzzle");
	TP_GunShotParticle->SetOwnerNoSee(true);
}

ACPlayerState* AFP_FirstPersonCharacter::GetSelfPlayerState()
{
	if (SelfPlayerState == nullptr)
		SelfPlayerState = Cast<ACPlayerState>(GetPlayerState());

	return SelfPlayerState;
}

void AFP_FirstPersonCharacter::SetSelfPlayerState(ACPlayerState* NewPlayerState)
{
	CheckFalse(HasAuthority());

	SetPlayerState(NewPlayerState);
	SelfPlayerState = NewPlayerState;
}


void AFP_FirstPersonCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SelfPlayerState = Cast<ACPlayerState>(GetPlayerState());

	if (GetLocalRole() == ENetRole::ROLE_Authority && SelfPlayerState != nullptr)
		SelfPlayerState->Health = 100.f;
}

void AFP_FirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() == false)
		SetTeamColor(CurrentTeam);
}

void AFP_FirstPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFP_FirstPersonCharacter::OnFire);
	PlayerInputComponent->BindAxis("MoveForward", this, &AFP_FirstPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFP_FirstPersonCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFP_FirstPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFP_FirstPersonCharacter::LookUpAtRate);
}

void AFP_FirstPersonCharacter::OnFire()
{
	CheckTrue(GetSelfPlayerState()->Health <= 0);

	if (FireAnimation != NULL)
	{
		UAnimInstance* AnimInstance = FP_Mesh->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	if (!!FP_GunShotParticle)
		FP_GunShotParticle->Activate(true);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	
	FVector ShootDir = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;

	if (PlayerController)
	{
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(StartTrace, CamRot);
		ShootDir = CamRot.Vector();

		StartTrace = StartTrace + ShootDir * ((GetActorLocation() - StartTrace) | ShootDir);
	}

	const FVector EndTrace = StartTrace + ShootDir * WeaponRange;

	OnServerFire(StartTrace, EndTrace);

}

void AFP_FirstPersonCharacter::OnServerFire_Implementation(const FVector& LineStart, const FVector& LineEnd)
{
	WeaponTrace(LineStart, LineEnd);
	MulticastFireEffect();
}

void AFP_FirstPersonCharacter::Func_Implementation()
{
	CLog::Print("HAHA");
}

void AFP_FirstPersonCharacter::MulticastFireEffect_Implementation()
{
	if (!!TP_FireAnimation)
	{
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (!!animInstance)
		{
			animInstance->Montage_Play(TP_FireAnimation);
		}
	}

	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (!!TP_GunShotParticle)
		TP_GunShotParticle->Activate(true);

	GetWorld()->SpawnActor<ACBullet>(ACBullet::StaticClass(), FP_Gun->GetSocketLocation("Muzzle"), GetControlRotation());
}

void AFP_FirstPersonCharacter::SetTeamColor_Implementation(ETeamType InTeamType)
{
	FLinearColor color;
	if (InTeamType == ETeamType::Red)
		color = FLinearColor(0.5f, 0.f, 0.f);
	else
		color = FLinearColor(0.f, 0.f, 0.5f);

	if (DynamicMaterial == nullptr)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), nullptr);
		DynamicMaterial->SetVectorParameterValue("BodyColor", color);
		GetMesh()->SetMaterial(0, DynamicMaterial);
		FP_Mesh->SetMaterial(0, DynamicMaterial);
	}
}

void AFP_FirstPersonCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFP_FirstPersonCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFP_FirstPersonCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFP_FirstPersonCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

FHitResult AFP_FirstPersonCharacter::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace)
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;
	//TraceParams.AddIgnoredActor()

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	CheckFalseResult(Hit.IsValidBlockingHit(), Hit);
	AFP_FirstPersonCharacter* other = Cast<AFP_FirstPersonCharacter>(Hit.GetActor());
	
	if (other != nullptr &&
		other->GetSelfPlayerState()->Team != GetSelfPlayerState()->Team &&
		other->GetSelfPlayerState()->Health > 0)
	{
		FDamageEvent e;
		other->TakeDamage(WeaponDamage, e, GetController(), this);
	}

	return Hit;
}

float AFP_FirstPersonCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CheckTrueResult(DamageCauser == this, DamageAmount);

	SelfPlayerState->Health -= DamageAmount;

	if (SelfPlayerState->Health <= 0)
	{
		PlayDead();

		SelfPlayerState->Death++;

		AFP_FirstPersonCharacter* other = Cast<AFP_FirstPersonCharacter>(DamageCauser);
		if (!!other)
			other->SelfPlayerState->Score += 1.f;

		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &AFP_FirstPersonCharacter::Respawn, 3.f, false);

		return DamageAmount;
	}

	PlayDamage();
	
	return DamageAmount;
}


void AFP_FirstPersonCharacter::PlayDamage_Implementation()
{
	if (!!TP_HitAnimation)
	{
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (!!animInstance)
		{
			animInstance->Montage_Play(TP_HitAnimation, 1.5f);
		}
	}
}

void AFP_FirstPersonCharacter::PlayDead_Implementation()
{
	GetMesh()->SetCollisionProfileName("Ragdoll");
	GetMesh()->SetPhysicsBlendWeight(1.f);
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFP_FirstPersonCharacter::Respawn()
{
	CheckFalse(HasAuthority());

	SelfPlayerState->Health = 100.f;
	Cast<AFP_FirstPersonGameMode>(GetWorld()->GetAuthGameMode())->Respawn(this);
	Destroy(true);
}