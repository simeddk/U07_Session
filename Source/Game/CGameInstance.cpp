#include "CGameInstance.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/CMainMenu.h"
#include "Interfaces/OnlineSessionInterface.h"
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
		SessionInterface->CreateSession(0, SESSION_NAME, sessionSettings);
	}
}

void UCGameInstance::Join(const FString& InAddress)
{
	if (!!MainMenu)
		MainMenu->Teardown();

	UEngine* engine = GetEngine();
	if (engine == nullptr) return;
	engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Join to %s"), *InAddress));

	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller == nullptr) return;
	controller->ClientTravel(InAddress, ETravelType::TRAVEL_Absolute);
}

void UCGameInstance::LoadMainMenuLevel()
{
	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller == nullptr) return;
	controller->ClientTravel("/Game/Maps/MainMenu", ETravelType::TRAVEL_Absolute);
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
