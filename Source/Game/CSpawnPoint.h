#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CGameState.h"
#include "CSpawnPoint.generated.h"

UCLASS()
class GAME_API ACSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ACSpawnPoint();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE bool IsBlocked() { return OverlappedActors.Num() > 0; }
	FORCEINLINE ETeamType GetTeam() { return Team; }

private:
	UFUNCTION()
		void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
		void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		ETeamType Team;

private:
	UPROPERTY(VisibleDefaultsOnly)
		class UCapsuleComponent* Capsule;

	TArray<AActor*> OverlappedActors;

	class APlayerStart* PlayerStart;
};
