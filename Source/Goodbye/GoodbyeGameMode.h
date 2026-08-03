#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "GoodbyeGameMode.generated.h"

/**
 *  Goodbye Game Mode
 */

UCLASS(abstract)
class AGoodbyeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	// Costruttore
	AGoodbyeGameMode();


protected:

	// Avvia la logica della partita
	virtual void BeginPlay() override;

	// Durata complessiva della partita, espressa in secondi
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Timer", meta = (ClampMin = "1"))
	int32 MatchDurationSeconds = 30;


	// Tempo ancora disponibile prima della fine della partita
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game|Timer")
	int32 RemainingTimeSeconds = 0;

private:

	// Avvia il countdown della partita
	void StartMatchTimer();


	// Riduce di un secondo il tempo rimanente
	void UpdateMatchTimer();


	// Gestisce la scadenza del tempo
	void HandleTimeExpired();


	// Timer che aggiorna il countdown ogni secondo.
	FTimerHandle MatchTimerHandle;
};



