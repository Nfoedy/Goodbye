#include "FinishZone.h"

#include "CargoTruckPawn.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ArrowComponent.h"
#include "GoodbyeGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovableItem.h"


// Costruttore
AFinishZone::AFinishZone()
{
	// La Finish Zone lavora tramite overlap e non necessita di un Tick.
	PrimaryActorTick.bCanEverTick = false;


	// Crea il volume della zona di uscita
	FinishBox =	CreateDefaultSubobject<UBoxComponent>(TEXT("FinishBox"));

	SetRootComponent(FinishBox);

	// Crea il riferimento visivo della direzione cinematografica
	CinematicDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("CinematicDirection"));

	CinematicDirection->SetupAttachment(FinishBox);

	// Inizialmente punta nella stessa direzione della Finish Zone
	CinematicDirection->SetRelativeLocation(FVector::ZeroVector);
	CinematicDirection->SetRelativeRotation(FRotator::ZeroRotator);


	// Dimensione iniziale modificabile nel Blueprint
	FinishBox->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));


	// La zona deve solamente rilevare overlap
	FinishBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);


	// Ignora inizialmente tutti i canali
	FinishBox->SetCollisionResponseToAllChannels(ECR_Ignore);


	// Rileva il canale utilizzato dai veicoli
	FinishBox->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);


	FinishBox->SetGenerateOverlapEvents(true);


	FinishBox->OnComponentBeginOverlap.AddDynamic(this, &AFinishZone::HandleFinishZoneBeginOverlap);
}

// Getter della direzione della cinematica
FVector AFinishZone::GetCinematicForwardDirection() const
{
	if (!IsValid(CinematicDirection))
	{
		return GetActorForwardVector();
	}

	return CinematicDirection->GetForwardVector();
}



// Avvia la Level Sequence e restituisce true se la riproduzione è partita
bool AFinishZone::PlayVictorySequence()
{
	// Verifica che sia stato assegnato un Level Sequence Actor
	if (!IsValid(VictorySequenceActor))
	{
		return false;
	}


	// Recupera il Player che controlla la riproduzione della Sequence
	ULevelSequencePlayer* SequencePlayer = VictorySequenceActor->GetSequencePlayer();


	if (!IsValid(SequencePlayer))
	{
		return false;
	}


	// Rimuove un eventuale collegamento precedente per evitare duplicazioni
	SequencePlayer->OnFinished.RemoveDynamic(
		this,
		&AFinishZone::HandleVictorySequenceFinished
	);


	// Collega la fine naturale della Sequence alla gestione del risultato
	SequencePlayer->OnFinished.AddDynamic(
		this,
		&AFinishZone::HandleVictorySequenceFinished
	);


	// Avvia la Sequence dalla posizione corrente
	SequencePlayer->Play();

	return true;
}



// Gestisce la conclusione naturale della Level Sequence di vittoria.
void AFinishZone::HandleVictorySequenceFinished()
{

	// Recupera il GameMode utilizzato dalla partita
	AGoodbyeGameMode* GoodbyeGameMode = Cast<AGoodbyeGameMode>(UGameplayStatics::GetGameMode(this));


	if (!IsValid(GoodbyeGameMode))
	{
		return;
	}

	// Mette in pausa il gioco quando lo schermo è completamente nero
	const bool bGamePaused = UGameplayStatics::SetGamePaused(this, true);


	if (!bGamePaused)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Finish Zone: impossibile mettere in pausa il gioco")
		);
	}


	// Comunica il risultato alla UI e mostra la schermata conclusiva
	GoodbyeGameMode->PresentMatchResult();
}



// Nasconde definitivamente le barre vita degli oggetti trasportabili.
void AFinishZone::SuppressMovableItemHealthWidgets()
{
	TArray<AActor*> MovableItemActors;


	// Recupera tutti gli oggetti trasportabili presenti nel livello.
	UGameplayStatics::GetAllActorsOfClass(
		this,
		AMovableItem::StaticClass(),
		MovableItemActors
	);


	for (AActor* Actor : MovableItemActors)
	{
		AMovableItem* MovableItem =
			Cast<AMovableItem>(Actor);


		if (!IsValid(MovableItem))
		{
			continue;
		}


		MovableItem->SuppressHealthWidget();
	}
}



// Ingresso nella Finish Zone
void AFinishZone::HandleFinishZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// Accetta solamente il cargo del gioco
	ACargoTruckPawn* CargoTruck = Cast<ACargoTruckPawn>(OtherActor);

	if (!IsValid(CargoTruck))
	{
		return;
	}


	// Un cargo vuoto arrivato per inerzia non può completare la partita.
	if (!CargoTruck->IsDriving())
	{
		return;
	}


	// Recupera il GameMode attualmente utilizzato dalla mappa
	AGoodbyeGameMode* GoodbyeGameMode = Cast<AGoodbyeGameMode>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(GoodbyeGameMode))
	{
		return;
	}

	// Salvo la condizione che il player ha vinto
	const bool bWillBeVictory = GoodbyeGameMode->HasReachedRequiredScore();


	// Il GameMode verifica tempo, punteggio e stato conclusivo della partita.
	const bool bMatchCompleted = GoodbyeGameMode->TryCompleteMatch();


	if (!bMatchCompleted)
	{
		return;
	}


	// Se il punteggio richiesto non è stato raggiunto,
	// la sconfitta è già stata gestita dal GameMode.
	if (!bWillBeVictory)
	{
		return;
	}


	// Nasconde le barre e impedisce che ricompaiano durante la cinematica finale.
	SuppressMovableItemHealthWidgets();


	// Avvia il movimento automatico del camion.
	CargoTruck->StartVictoryAutoDrive(
		GetCinematicForwardDirection()
	);


	// Avvia contemporaneamente il movimento della CineCamera.
	const bool bSequenceStarted = PlayVictorySequence();


	// Se la Sequence non può partire, mostra comunque il risultato
	// per evitare che la partita rimanga bloccata senza EndPanel.
	if (!bSequenceStarted)
	{
		GoodbyeGameMode->PresentMatchResult();
	}
}