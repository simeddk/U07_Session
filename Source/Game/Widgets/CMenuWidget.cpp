#include "CMenuWidget.h"

void UCMenuWidget::Setup()
{
	AddToViewport();
	bIsFocusable = true;

	FInputModeUIOnly inpuMode;
	inpuMode.SetWidgetToFocus(TakeWidget());
	inpuMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	UWorld* world = GetWorld();
	if (world == nullptr) return;

	APlayerController* controller = world->GetFirstPlayerController();
	if (controller == nullptr) return;
	controller->SetInputMode(inpuMode);
	controller->bShowMouseCursor = true;
}

void UCMenuWidget::Teardown()
{
	RemoveFromParent();
	bIsFocusable = false;

	FInputModeGameOnly inpuMode;

	UWorld* world = GetWorld();
	if (world == nullptr) return;

	APlayerController* controller = world->GetFirstPlayerController();
	if (controller == nullptr) return;
	controller->SetInputMode(inpuMode);
	controller->bShowMouseCursor = false;
}
