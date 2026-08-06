// Copyright Epic Games, Inc. All Rights Reserved.


#include "GoodbyePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Goodbye.h"
#include "GoodbyeCameraManager.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SVirtualJoystick.h"

AGoodbyePlayerController::AGoodbyePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AGoodbyeCameraManager::StaticClass();
}

void AGoodbyePlayerController::BeginPlay()
{
	Super::BeginPlay();

	
	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogGoodbye, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AGoodbyePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Recupera l'Enhanced Input Component utilizzato dal PlayerController.
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);


	if (!IsValid(EnhancedInputComponent))
	{
		UE_LOG(
			LogGoodbye,
			Error,
			TEXT(
				"GoodbyePlayerController: "
				"EnhancedInputComponent non disponibile"
			)
		);

		return;
	}


	// Collega IA_Pause alla gestione del menu di pausa.
	if (IsValid(PauseAction))
	{
		EnhancedInputComponent->BindAction(
			PauseAction,
			ETriggerEvent::Started,
			this,
			&AGoodbyePlayerController::HandlePauseAction
		);
	}
	else
	{
		UE_LOG(
			LogGoodbye,
			Warning,
			TEXT(
				"GoodbyePlayerController: "
				"PauseAction non assegnata"
			)
		);
	}

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}


// Gestisce la pressione dell'Input Action dedicata alla pausa.
void AGoodbyePlayerController::HandlePauseAction()
{
	if (!bPauseMenuEnabled)
	{
		return;
	}


	// Se il menu è già visibile, ESC ripristina il gameplay.
	if (
		IsValid(PauseMenuWidget) &&
		PauseMenuWidget->IsInViewport()
		)
	{
		ClosePauseMenu();
		return;
	}


	// Se il menu non è presente, ESC lo apre.
	OpenPauseMenu();
}


// Crea e apre il menu di pausa.
void AGoodbyePlayerController::OpenPauseMenu()
{
	if (!IsLocalPlayerController() || !bPauseMenuEnabled)
	{
		return;
	}


	if (!IsValid(PauseMenuWidgetClass))
	{
		UE_LOG(
			LogGoodbye,
			Error,
			TEXT(
				"GoodbyePlayerController: "
				"PauseMenuWidgetClass non assegnata"
			)
		);

		return;
	}


	// Crea il Widget soltanto la prima volta.
	if (!IsValid(PauseMenuWidget))
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(
			this,
			PauseMenuWidgetClass
		);
	}


	if (!IsValid(PauseMenuWidget))
	{
		UE_LOG(
			LogGoodbye,
			Error,
			TEXT(
				"GoodbyePlayerController: "
				"impossibile creare il menu di pausa"
			)
		);

		return;
	}


	// Aggiunge il menu davanti agli altri Widget.
	if (!PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->AddToViewport(100);
	}


	// Mette in pausa il mondo di gioco.
	const bool bPauseApplied = SetPause(true);


	if (!bPauseApplied)
	{
		UE_LOG(
			LogGoodbye,
			Warning,
			TEXT(
				"GoodbyePlayerController: "
				"impossibile mettere in pausa il gioco"
			)
		);
	}


	// Mostra il cursore.
	bShowMouseCursor = true;


	// Permette sia alla UI sia al PlayerController
	// di ricevere gli input.
	FInputModeGameAndUI InputMode;

	InputMode.SetWidgetToFocus(
		PauseMenuWidget->TakeWidget()
	);

	InputMode.SetLockMouseToViewportBehavior(
		EMouseLockMode::DoNotLock
	);

	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);
}


// Chiude il menu di pausa e ripristina il controllo del gioco.
void AGoodbyePlayerController::ClosePauseMenu()
{
	if (
		IsValid(PauseMenuWidget) &&
		PauseMenuWidget->IsInViewport()
		)
	{
		PauseMenuWidget->RemoveFromParent();
	}


	// Rimuove la pausa.
	SetPause(false);


	// Nasconde nuovamente il cursore.
	bShowMouseCursor = false;


	// Restituisce gli input esclusivamente al gameplay.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}


// Chiude il menu di pausa e ripristina il gameplay.
void AGoodbyePlayerController::ResumeGame()
{
	ClosePauseMenu();
}


// Torna al livello del Main Menu.
void AGoodbyePlayerController::ReturnToMainMenu()
{
	// Rimuove il Widget prima di cambiare livello.
	if (
		IsValid(PauseMenuWidget) &&
		PauseMenuWidget->IsInViewport()
		)
	{
		PauseMenuWidget->RemoveFromParent();
	}


	// Il nuovo livello non deve essere caricato mentre il gioco è in pausa.
	SetPause(false);


	UGameplayStatics::OpenLevel(
		this,
		FName(TEXT("Lvl_MainMenu"))
	);
}


// Abilita o disabilita l'apertura del menu di pausa.
void AGoodbyePlayerController::SetPauseMenuEnabled(bool bEnabled)
{
	bPauseMenuEnabled = bEnabled;


	// Se viene disabilitato mentre è visibile,
	// rimuove solamente il Widget.
	if (
		!bPauseMenuEnabled &&
		IsValid(PauseMenuWidget) &&
		PauseMenuWidget->IsInViewport()
		)
	{
		PauseMenuWidget->RemoveFromParent();
	}
}

bool AGoodbyePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
