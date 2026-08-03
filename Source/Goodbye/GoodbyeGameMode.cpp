#include "GoodbyeGameMode.h"

AGoodbyeGameMode::AGoodbyeGameMode()
{
	// stub
}

// Costruttore
void AGoodbyeGameMode::BeginPlay()
{
	Super::BeginPlay();

	StartMatchTimer();
}


// Avvio del timer
void AGoodbyeGameMode::StartMatchTimer()
{
	// Imposta il tempo iniziale della partita
	RemainingTimeSeconds = MatchDurationSeconds;


	const int32 Minutes = RemainingTimeSeconds / 60;
	const int32 Seconds = RemainingTimeSeconds % 60;


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Tempo rimanente: %02d:%02d"),
		Minutes,
		Seconds
	);


	// Richiama UpdateMatchTimer ogni secondo
	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&AGoodbyeGameMode::UpdateMatchTimer,
		1.0f,
		true,
		1.0f
	);
}


// Aggiornamento del countdown
void AGoodbyeGameMode::UpdateMatchTimer()
{
	RemainingTimeSeconds = FMath::Max(RemainingTimeSeconds - 1,	0);

	const int32 Minutes = RemainingTimeSeconds / 60;
	const int32 Seconds = RemainingTimeSeconds % 60;


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Tempo rimanente: %02d:%02d"),
		Minutes,
		Seconds
	);


	if (RemainingTimeSeconds <= 0)
	{
		HandleTimeExpired();
	}
}


// Scadenza del tempo
void AGoodbyeGameMode::HandleTimeExpired()
{
	// Ferma definitivamente il countdown
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Tempo terminato!")
	);

}
