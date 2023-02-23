#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CGameState.h"
#include "FP_FirstPersonCharacter.generated.h"

class UInputComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class USoundBase;
class UAnimMontage;

UCLASS(config=Game)
class AFP_FirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//FP
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* FP_Mesh;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	UPROPERTY(VisibleDefaultsOnly, Category = Gameplay)
		class UParticleSystemComponent* FP_GunShotParticle;

	//TP
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* TP_Gun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UAnimMontage* TP_FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UAnimMontage* TP_HitAnimation;

	UPROPERTY(VisibleDefaultsOnly, Category = Gameplay)
		class UParticleSystemComponent* TP_GunShotParticle;

public:
	AFP_FirstPersonCharacter();

	class ACPlayerState* GetSelfPlayerState();
	void SetSelfPlayerState(class ACPlayerState* NewPlayerState);


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float WeaponRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float WeaponDamage;

protected:
	virtual void PossessedBy(AController* NewController) override;
	void BeginPlay() override;

	void OnFire();
	
	UFUNCTION(Server, Reliable)
		void OnServerFire(const FVector& LineStart, const FVector& LineEnd);
	void OnServerFire_Implementation(const FVector& LineStart, const FVector& LineEnd);

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastFireEffect();
	void MulticastFireEffect_Implementation();

	UFUNCTION(NetMulticast, Unreliable)
		void PlayDamage();
	void PlayDamage_Implementation();
	
	UFUNCTION(NetMulticast, Unreliable)
		void PlayDead();
	void PlayDead_Implementation();

public:
	UFUNCTION(NetMulticast, Reliable)
		void SetTeamColor(ETeamType InTeamType);
	void SetTeamColor_Implementation(ETeamType InTeamType);

protected:
	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace);
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

private:
	UFUNCTION()
		void Respawn();

public:
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return FP_Mesh; }
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return Camera; }

public:
	UPROPERTY(Replicated)
		ETeamType CurrentTeam;

private:
	class UMaterialInstanceDynamic* DynamicMaterial;
	class ACPlayerState* SelfPlayerState;

	UFUNCTION(Client, Reliable) void Func();
};

