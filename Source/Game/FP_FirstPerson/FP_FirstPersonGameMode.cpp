#include "FP_FirstPersonGameMode.h"
#include "Global.h"
#include "CGameState.h"
#include "CPlayerState.h"
#include "CSpawnPoint.h"
#include "FP_FirstPersonHUD.h"
#include "FP_FirstPersonCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"

AFP_FirstPersonGameMode::AFP_FirstPersonGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));

	DefaultPawnClass = PlayerPawnClassFinder.Class;
	HUDClass = AFP_FirstPersonHUD::StaticClass();
	GameStateClass = ACGameState::StaticClass();
	PlayerStateClass = ACPlayerState::StaticClass();
}

void AFP_FirstPersonGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ACSpawnPoint> iter(GetWorld()); iter; ++iter)
	{
		if (iter->GetTeam() == ETeamType::Red)
			RedTeamSpawners.Add(*iter);
		else
			BlueTeamSpawners.Add(*iter);
	}

	/*APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (!!controller)
	{
		AFP_FirstPersonCharacter* player = Cast<AFP_FirstPersonCharacter>(controller->GetPawn());
		if (!!player)
		{
			player->SetTeamColor(ETeamType::Red);
			RedTeamCharacters.Add(player);

			Spawn(player);
		}
	}*/
}

void AFP_FirstPersonGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AFP_FirstPersonCharacter* player = Cast<AFP_FirstPersonCharacter>(NewPlayer->GetPawn());
	ACPlayerState* playerState = Cast<ACPlayerState>(NewPlayer->PlayerState);

	if (player != nullptr && playerState != nullptr)
	{
		player->SetPlayerState(playerState);

		if (BlueTeamCharacters.Num() >= RedTeamCharacters.Num())
		{
			RedTeamCharacters.Add(player);
			playerState->Team = ETeamType::Red;
		}
		else
		{
			BlueTeamCharacters.Add(player);
			playerState->Team = ETeamType::Blue;
		}

		player->CurrentTeam = playerState->Team;
		player->SetTeamColor(playerState->Team);

		Spawn(player);
		
	}
}

void AFP_FirstPersonGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ToBeSpawns.Num() > 0)
	{
		for (AFP_FirstPersonCharacter* player : ToBeSpawns)
			Spawn(player);
	}
}

void AFP_FirstPersonGameMode::Spawn(class AFP_FirstPersonCharacter* InPlayer)
{
	TArray<ACSpawnPoint*>* tergetTeam = nullptr;

	if (InPlayer->CurrentTeam == ETeamType::Red)
		tergetTeam = &RedTeamSpawners;
	else
		tergetTeam = &BlueTeamSpawners;


	for (ACSpawnPoint* point : *tergetTeam)
	{
		if (point->IsBlocked() == false)
		{
			InPlayer->SetActorLocation(point->GetActorLocation());
			point->UpdateOverlaps();
			
			if (ToBeSpawns.Find(InPlayer) >= 0)
				ToBeSpawns.Remove(InPlayer);

			return;
		}
	}

	if (ToBeSpawns.Find(InPlayer) < 0)
		ToBeSpawns.Add(InPlayer);
}

void AFP_FirstPersonGameMode::Respawn(class AFP_FirstPersonCharacter* InPlayer)
{
	AController* controller = InPlayer->GetController();
	InPlayer->DetachFromControllerPendingDestroy();

	AFP_FirstPersonCharacter* player = Cast<AFP_FirstPersonCharacter>(GetWorld()->SpawnActor(DefaultPawnClass));
	if (!!player)
	{
		controller->Possess(player);
		ACPlayerState* playerState = Cast<ACPlayerState>(player->GetController()->PlayerState);
		player->CurrentTeam = playerState->Team;
		player->SetSelfPlayerState(playerState);

		Spawn(player);

		player->SetTeamColor(playerState->Team);
	}
}