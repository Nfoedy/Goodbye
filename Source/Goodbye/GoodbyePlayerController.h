// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GoodbyePlayerController.generated.h"


class UInputMappingContext;
class UUserWidget;
class UInputAction;
class UInputMappingContext;
class UUserWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class GOODBYE_API AGoodbyePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AGoodbyePlayerController();


	// Chiude il menu di pausa e ripristina il gameplay.
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ResumeGame();


	// Torna al livello del Main Menu.
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ReturnToMainMenu();


	// Abilita o disabilita l'apertura del menu di pausa.
	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void SetPauseMenuEnabled(bool bEnabled);

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	// Input Action utilizzata per aprire e chiudere il menu di pausa.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> PauseAction = nullptr;


	// Classe del Widget utilizzato come menu di pausa.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Pause Menu")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;


	// Istanza attualmente utilizzata del menu di pausa.
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;


private:

	// Gestisce la pressione dell'Input Action dedicata alla pausa.
	void HandlePauseAction();


	// Crea e apre il menu di pausa.
	void OpenPauseMenu();


	// Chiude il menu di pausa e ripristina il controllo del gioco.
	void ClosePauseMenu();


	// Indica se il giocatore può aprire il menu di pausa.
	bool bPauseMenuEnabled = true;
};
