#include "GoodbyeGameMode.h"
#include "CargoZone.h"
#include "Kismet/GameplayStatics.h"


// Costruttore
AGoodbyeGameMode::AGoodbyeGameMode()
{
	// stub
}


// Begin Play
void AGoodbyeGameMode::BeginPlay()
{
	Super::BeginPlay();

	ConfigureDifficulty();
	StartMatchTimer();

	// Aspetta il frame successivo, così la Cargo Zone contenuta nel cargo è già stata creata
	GetWorldTimerManager().SetTimerForNextTick(this, &AGoodbyeGameMode::BindToCargoZone);
}



void AGoodbyeGameMode::BindToCargoZone()
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(this,ACargoZone::StaticClass());

	CargoZone = Cast<ACargoZone>(FoundActor);

	if (!IsValid(CargoZone))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("GoodbyeGameMode: Cargo Zone non trovata")
		);

		return;
	}

	// Collega il GameMode all'evento della Cargo Zone
	CargoZone->OnCargoChanged.AddDynamic(this, &AGoodbyeGameMode::HandleCargoChanged);

	// Recupera anche i valori iniziali, senza aspettare il primo ingresso o la prima uscita di un oggetto
	HandleCargoChanged(CargoZone->GetLoadedItemCount(),	CargoZone->GetCurrentScore());


	UE_LOG(
		LogTemp,
		Display,
		TEXT("GoodbyeGameMode collegato alla Cargo Zone")
	);
}

// Aggiorna gli oggetti caricati
void AGoodbyeGameMode::HandleCargoChanged(int32 NewLoadedItemCount, int32 NewCargoScore)
{
	LoadedItemCount = NewLoadedItemCount;
	CurrentScore = NewCargoScore;


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"GameMode | Oggetti: %d | Punteggio: %d/%d"
		),
		LoadedItemCount,
		CurrentScore,
		RequiredScore
	);
}


// Configura la difficoltà
void AGoodbyeGameMode::ConfigureDifficulty()
{
	switch (SelectedDifficulty)
	{
	case EGameDifficulty::Easy:
	{
		RequiredScore = 5;
		break;
	}


	case EGameDifficulty::Normal:
	{
		RequiredScore = 8;
		break;
	}


	case EGameDifficulty::Hard:
	{
		RequiredScore = 12;
		break;
	}


	default:
	{
		SelectedDifficulty = EGameDifficulty::Normal;
		RequiredScore = 8;
		break;
	}
	}


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Difficolta configurata. Punteggio richiesto: %d"),
		RequiredScore
	);
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
	if (bMatchFinished)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(
		MatchTimerHandle
	);

	bMatchFinished = true;

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"SCONFITTA! Tempo terminato. "
			"Punteggio finale: %d/%d"
		),
		CurrentScore,
		RequiredScore
	);
}

// Termine della partita
bool AGoodbyeGameMode::TryCompleteMatch()
{
	// Impedisce di terminare la partita più volte
	if (bMatchFinished) return false;

	// Non si può vincere dopo lo scadere del tempo
	if (RemainingTimeSeconds <= 0) return false;

	// Il cargo è arrivato all'uscita, ma non contiene ancora abbastanza punti
	if (CurrentScore < RequiredScore)
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Uscita raggiunta, ma il punteggio non basta: %d/%d"
			),
			CurrentScore,
			RequiredScore
		);

		return false;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	bMatchFinished = true;

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"VITTORIA! Uscita raggiunta con punteggio %d/%d e %d secondi rimanenti"
		),
		CurrentScore,
		RequiredScore,
		RemainingTimeSeconds
	);

	return true;
}
