#pragma once

#include "CoreMinimal.h"
#include "CMenuWidget.h"
#include "CMainMenu.generated.h"

USTRUCT(BlueprintType)
struct FServerData
{
	GENERATED_BODY()

public:
	FString Name;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	FString HostUserName;
};

UCLASS()
class GAME_API UCMainMenu : public UCMenuWidget
{
	GENERATED_BODY()

public:
	UCMainMenu(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;

	void SetServerList(TArray<FServerData> InServerNames);
	void SetSelectedIndex(uint32 Index);

private:
	UFUNCTION()
		void HostServer();

	UFUNCTION()
		void JoinServer();
	
	UFUNCTION()
		void OpenJoinMenu();

	UFUNCTION()
		void OpenMainMenu();

	UFUNCTION()
		void OpenHostMenu();

	UFUNCTION()
		void QuitPressed();

private:
	void UpdateChildren();

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* CancelJoinMenuButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* ConfirmJoinMenuButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
		class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
		class UWidget* JoinMenu;

	UPROPERTY(meta = (BindWidget))
		class UWidget* MainMenu;

	UPROPERTY(meta = (BindWidget))
		class UPanelWidget* ServerList;

	UPROPERTY(meta = (BindWidget))
		class UWidget* HostMenu;

	UPROPERTY(meta = (BindWidget))
		class UEditableTextBox* ServerHostName;

	UPROPERTY(meta = (BindWidget))
		class UButton* CancelHostMenuButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* ConfirmHostMenuButton;

private:
	TSubclassOf<class UUserWidget> ServerRowClass;
	TOptional<uint32> SelectedIndex;
};
