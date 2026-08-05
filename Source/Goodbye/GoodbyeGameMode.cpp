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

	// Comunica alla UI il valore del timer
	OnMatchTimeChanged.Broadcast(RemainingTimeSeconds);


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

	// Comunica alla UI il nuovo tempo rimanente
	OnMatchTimeChanged.Broadcast(RemainingTimeSeconds);

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
	FinishMatch(
		EMatchResult::Defeat
	);
}



// Termine della partita
bool AGoodbyeGameMode::TryCompleteMatch()
{
	// La partita è già terminata.
	if (bMatchFinished)
	{
		return false;
	}


	// Il timer è già arrivato a zero.
	if (RemainingTimeSeconds <= 0)
	{
		return false;
	}


	const bool bHasRequiredScore = CurrentScore >= RequiredScore;


	const EMatchResult Result =	bHasRequiredScore ? EMatchResult::Victory :	EMatchResult::Defeat;


	FinishMatch(Result);

	// True significa che la FinishZone ha concluso la partita,
	// indipendentemente dal fatto che sia vittoria o sconfitta.
	return true;
}


// Fine del gioco
void AGoodbyeGameMode::FinishMatch(EMatchResult Result)
{
	// Protezione contro chiamate multiple
	if (bMatchFinished)
	{
		return;
	}

	bMatchFinished = true;

	// Salva il risultato definitivo della partita
	FinalMatchResult = Result;

	// Il countdown non deve continuare dopo la conclusione della partita
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);


	if (Result == EMatchResult::Victory)
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"VITTORIA | Punteggio: %d/%d | Tempo rimasto: %d"
			),
			CurrentScore,
			RequiredScore,
			RemainingTimeSeconds
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"SCONFITTA | Punteggio: %d/%d | Tempo rimasto: %d"
			),
			CurrentScore,
			RequiredScore,
			RemainingTimeSeconds
		);
	}


	// La sconfitta non possiede una cinematica finale, quindi il risultato viene mostrato immediatamente
	if (Result == EMatchResult::Defeat)
	{
		PresentMatchResult();
	}
}


// Comunica una sola volta alla UI il risultato definitivo della partita
void AGoodbyeGameMode::PresentMatchResult()
{
	// Non può essere mostrato un risultato se la partita non è ancora terminata
	if (!bMatchFinished)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("GoodbyeGameMode: tentativo di mostrare il risultato prima della fine della partita")
		);

		return;
	}


	// Impedisce che la UI finale venga attivata più volte
	if (bMatchResultPresented)
	{
		return;
	}


	bMatchResultPresented = true;

	// Comunica il risultato definitivo ai Blueprint e alla UI
	OnMatchFinished.Broadcast(FinalMatchResult);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("GoodbyeGameMode: risultato finale comunicato alla UI")
	);
}