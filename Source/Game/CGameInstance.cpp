#include "CGameInstance.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"

UCGameInstance::UCGameInstance(const FObjectInitializer& ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> mainMenuClass(TEXT("/Game/Widgets/WB_MainMenu"));

	if (mainMenuClass.Succeeded())
		MainMenuClass = mainMenuClass.Class;
}

void UCGameInstance::Init()
{

}

void UCGameInstance::LoadMainMenu()
{
	if (MainMenuClass == nullptr) return;

	UUserWidget* mainMenu = CreateWidget<UUserWidget>(this, MainMenuClass);
	if (mainMenu == nullptr) return;
	mainMenu->AddToViewport();

	mainMenu->bIsFocusable = true;

	FInputModeUIOnly inpuMode;
	inpuMode.SetWidgetToFocus(mainMenu->TakeWidget());
	inpuMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller == nullptr) return;
	controller->SetInputMode(inpuMode);
	controller->bShowMouseCursor = true;
}

void UCGameInstance::Host()
{
	UEngine* engine = GetEngine();
	if (engine == nullptr) return;
	engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Host"));

	UWorld* world = GetWorld();
	if (world == nullptr) return;
	world->ServerTravel("/Game/Maps/Play?listen");
}

void UCGameInstance::Join(const FString& InAddress)
{
	UEngine* engine = GetEngine();
	if (engine == nullptr) return;
	engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Join to %s"), *InAddress));

	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller == nullptr) return;
	controller->ClientTravel(InAddress, ETravelType::TRAVEL_Absolute);
}
