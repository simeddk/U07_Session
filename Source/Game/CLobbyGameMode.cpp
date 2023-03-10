#include "CLobbyGameMode.h"
#include "CGameInstance.h"

void ACLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	++NumberOfPlayers;

	if (NumberOfPlayers >= 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Reached 2 Players!"));

		GetWorldTimerManager().SetTimer(GameStartTimer, this, &ACLobbyGameMode::StartGame, 10);			
	}
}

void ACLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	--NumberOfPlayers;
}

void ACLobbyGameMode::StartGame()
{
	UCGameInstance* gameInstance = Cast<UCGameInstance>(GetGameInstance());
	if (gameInstance == nullptr) return;
	gameInstance->StartSession();

	UWorld* world = GetWorld();
	if (world == nullptr) return;

	//bUseSeamlessTravel = true;
	world->ServerTravel("/Game/Maps/Combat?listen");
}
