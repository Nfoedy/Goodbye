#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "GoodbyeGameMode.generated.h"


class ACargoZone;


// Enum per la difficoltà
UENUM(BlueprintType)
enum class EGameDifficulty : uint8
{
	Easy	UMETA(DisplayName = "Easy"),
	Normal	UMETA(DisplayName = "Normal"),
	Hard	UMETA(DisplayName = "Hard")
};


// Risultati conclusivi della partita.
UENUM(BlueprintType)
enum class EMatchResult : uint8
{
	Victory	UMETA(DisplayName = "Victory"),
	Defeat	UMETA(DisplayName = "Defeat")
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchTimeChangedSignature, int32, NewRemainingTimeSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchFinishedSignature, EMatchResult, MatchResult);



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

	// Prova a completare la partita quando il cargo raggiunge l'uscita
	bool TryCompleteMatch();

	// Evento richiamato ogni volta che cambia il tempo rimanente
	UPROPERTY(BlueprintAssignable, Category = "Game|Timer")
	FMatchTimeChangedSignature OnMatchTimeChanged;

	// Evento richiamato una sola volta quando termina la partita
	UPROPERTY(BlueprintAssignable, Category = "Game|Result")
	FMatchFinishedSignature OnMatchFinished;

	// Restituisce il tempo rimanente in secondi
	UFUNCTION(BlueprintPure, Category = "Game|Timer")
	int32 GetRemainingTimeSeconds() const
	{
		return RemainingTimeSeconds;
	}

	// Restituisce il punteggio necessario per vincere
	UFUNCTION(BlueprintPure, Category = "Game|Difficulty")
	int32 GetRequiredScore() const
	{
		return RequiredScore;
	}


protected:

	// Avvia la logica della partita
	virtual void BeginPlay() override;

	// Durata complessiva della partita, espressa in secondi
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Timer", meta = (ClampMin = "1"))
	int32 MatchDurationSeconds = 240;


	// Tempo ancora disponibile prima della fine della partita
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game|Timer")
	int32 RemainingTimeSeconds = 0;

	// Difficoltà selezionata per la partita.
	UPROPERTY(EditDefaultsOnly,	BlueprintReadOnly,Category = "Game|Difficulty")
	EGameDifficulty SelectedDifficulty = EGameDifficulty::Normal;


	// Punteggio necessario per vincere la partita.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game|Difficulty")
	int32 RequiredScore = 8;


	// Numero di oggetti attualmente caricati nel cargo.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game|Score")
	int32 LoadedItemCount = 0;


	// Punteggio attualmente presente nel cargo.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Game|Score")
	int32 CurrentScore = 0;


private:

	// Avvia il countdown della partita
	void StartMatchTimer();


	// Riduce di un secondo il tempo rimanente
	void UpdateMatchTimer();


	// Gestisce la scadenza del tempo
	void HandleTimeExpired();

	// Conclude definitivamente la partita con il risultato indicato
	void FinishMatch(EMatchResult Result);

	// Timer che aggiorna il countdown ogni secondo.
	FTimerHandle MatchTimerHandle;

	// Indica che la partita è già terminata
	bool bMatchFinished = false;


	// Configura il punteggio richiesto in base alla difficoltà
	void ConfigureDifficulty();

	// Cargo Zone utilizzata dalla partita
	UPROPERTY(Transient)
	TObjectPtr<ACargoZone> CargoZone = nullptr;

	// Cerca la Cargo Zone e si collega al suo evento.
	void BindToCargoZone();


	// Riceve il nuovo conteggio e il nuovo punteggio dalla Cargo Zone.
	UFUNCTION()
	void HandleCargoChanged(int32 NewLoadedItemCount, int32 NewCargoScore);
};



