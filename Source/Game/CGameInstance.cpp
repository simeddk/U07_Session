#include "CGameInstance.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/CMainMenu.h"
#include "OnlineSessionSettings.h"

const static FName SESSION_NAME = TEXT("GameSession");

UCGameInstance::UCGameInstance(const FObjectInitializer& ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> mainMenuClass(TEXT("/Game/Widgets/WB_MainMenu"));
	if (mainMenuClass.Succeeded())
		MainMenuClass = mainMenuClass.Class;

	ConstructorHelpers::FClassFinder<UUserWidget> inGameMenuClass(TEXT("/Game/Widgets/WB_InGameMenu"));
	if (inGameMenuClass.Succeeded())
		InGameMenuClass = inGameMenuClass.Class;
}

void UCGameInstance::Init()
{
	IOnlineSubsystem* oss = IOnlineSubsystem::Get();
	if (!!oss)
	{
		UE_LOG(LogTemp, Error, TEXT("OSS Pointer Found. Name : %s"), *oss->GetSubsystemName().ToString());

		SessionInterface = oss->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UCGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Not Found OSS"));
	}
}

void UCGameInstance::LoadMainMenu()
{
	if (MainMenuClass == nullptr) return;

	MainMenu = CreateWidget<UCMainMenu>(this, MainMenuClass);
	if (MainMenu == nullptr) return;
	
	MainMenu->SetMenuInterface(this);
	MainMenu->Setup();
}

void UCGameInstance::LoadInGameMenu()
{
	if (InGameMenuClass == nullptr) return;

	UCMenuWidget* ingameMenu = CreateWidget<UCMenuWidget>(this, InGameMenuClass);
	if (ingameMenu == nullptr) return;

	ingameMenu->SetMenuInterface(this);
	ingameMenu->Setup();
}

void UCGameInstance::Host()
{
	if (SessionInterface.IsValid())
	{
		auto exsistingSession = SessionInterface->GetNamedSession(SESSION_NAME);

		if (!!exsistingSession)
		{
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession();
		}
	}
}

void UCGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings sessionSettings;
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			sessionSettings.bIsLANMatch = true;
		}
		else
		{
			sessionSettings.bIsLANMatch = false;
		}
		sessionSettings.NumPublicConnections = 2;
		sessionSettings.bShouldAdvertise = true;
		sessionSettings.bUsesPresence = true;

		SessionInterface->CreateSession(0, SESSION_NAME, sessionSettings);
	}
}

void UCGameInstance::Join(uint32 Index)
{
	if (SessionInterface.IsValid() == false) return;
	if (SessionSearch.IsValid() == false) return;

	if (!!MainMenu)
		MainMenu->Teardown();
	
	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UCGameInstance::LoadMainMenuLevel()
{
	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller == nullptr) return;
	controller->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UCGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		//SessionSearch->bIsLanQuery = true;
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		UE_LOG(LogTemp, Error, TEXT("Start Find Sessions"));
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UCGameInstance::OnCreateSessionComplete(FName InSessionName, bool InSuccess)
{
	if (InSuccess == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Could Not Create Session!!"));
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("SessionName : %s"), *InSessionName.ToString());

	if (!!MainMenu)
		MainMenu->Teardown();

	UEngine* engine = GetEngine();
	if (engine == nullptr) return;
	engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Host"));

	UWorld* world = GetWorld();
	if (world == nullptr) return;
	world->ServerTravel("/Game/Maps/Play?listen");
}

void UCGameInstance::OnDestroySessionComplete(FName InSessionName, bool InSuccess)
{
	if (InSuccess == true)
		CreateSession();
}

void UCGameInstance::OnFindSessionsComplete(bool InSuccess)
{
	if (InSuccess == true && SessionSearch.IsValid() && MainMenu != nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Finished Find Sessions"));

		TArray<FServerData> serverNames;
		for (const FOnlineSessionSearchResult& searchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found Session ID : %s"), *searchResult.GetSessionIdStr());
			UE_LOG(LogTemp, Warning, TEXT("Ping : %d"), searchResult.PingInMs);

			FServerData data;
			data.Name = searchResult.GetSessionIdStr();
			data.MaxPlayers = searchResult.Session.SessionSettings.NumPublicConnections;
			data.CurrentPlayers = data.MaxPlayers - searchResult.Session.NumOpenPublicConnections;
			data.HostUserName = searchResult.Session.OwningUserName;
			serverNames.Add(data);
		}

		MainMenu->SetServerList(serverNames);
	}
}

void UCGameInstance::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	if (SessionInterface.IsValid() == false) return;

	FString address;
	if (SessionInterface->GetResolvedConnectString(InSessionName, address) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not get IP address"));
		return;
	}

	UEngine* engine = GetEngine();
	if (engine == nullptr) return;
	engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Join to %s"), *address));

	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller == nullptr) return;
	controller->ClientTravel(address, ETravelType::TRAVEL_Absolute);
}
