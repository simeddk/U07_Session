#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FP_FirstPersonGameMode.generated.h"

UCLASS(minimalapi)
class AFP_FirstPersonGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFP_FirstPersonGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Tick(float DeltaTime) override;

	void Spawn(class AFP_FirstPersonCharacter* InPlayer);
	void Respawn(class AFP_FirstPersonCharacter* InPlayer);

private:
	TArray<class AFP_FirstPersonCharacter*> RedTeamCharacters;
	TArray<class AFP_FirstPersonCharacter*> BlueTeamCharacters;
	TArray<class AFP_FirstPersonCharacter*> ToBeSpawns;

	TArray<class ACSpawnPoint*> RedTeamSpawners;
	TArray<class ACSpawnPoint*> BlueTeamSpawners;
};



