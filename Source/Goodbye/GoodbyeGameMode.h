#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "GoodbyeGameMode.generated.h"


// Enum per la difficoltà
UENUM(BlueprintType)
enum class EGameDifficulty : uint8
{
	Easy	UMETA(DisplayName = "Easy"),
	Normal	UMETA(DisplayName = "Normal"),
	Hard	UMETA(DisplayName = "Hard")
};



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

	// Difficoltà selezionata per la partita.
	UPROPERTY(EditDefaultsOnly,	BlueprintReadOnly,Category = "Game|Difficulty")
	EGameDifficulty SelectedDifficulty = EGameDifficulty::Normal;


	// Punteggio necessario per vincere la partita.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game|Difficulty")
	int32 RequiredScore = 8;

private:

	// Avvia il countdown della partita
	void StartMatchTimer();


	// Riduce di un secondo il tempo rimanente
	void UpdateMatchTimer();


	// Gestisce la scadenza del tempo
	void HandleTimeExpired();


	// Timer che aggiorna il countdown ogni secondo.
	FTimerHandle MatchTimerHandle;


	// Configura il punteggio richiesto in base alla difficoltà
	void ConfigureDifficulty();
};



