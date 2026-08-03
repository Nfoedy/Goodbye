#include "FinishZone.h"

#include "CargoTruckPawn.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GoodbyeGameMode.h"
#include "Kismet/GameplayStatics.h"


// Costruttore
AFinishZone::AFinishZone()
{
	// La Finish Zone lavora tramite overlap e non necessita di un Tick.
	PrimaryActorTick.bCanEverTick = false;


	// Crea il volume della zona di uscita
	FinishBox =	CreateDefaultSubobject<UBoxComponent>(TEXT("FinishBox"));

	SetRootComponent(FinishBox);


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
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Finish Zone: camion rilevato senza guidatore"
			)
		);

		return;
	}


	// Recupera il GameMode attualmente utilizzato dalla mappa
	AGoodbyeGameMode* GoodbyeGameMode = Cast<AGoodbyeGameMode>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(GoodbyeGameMode))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Finish Zone: GoodbyeGameMode non trovato"
			)
		);

		return;
	}


	// Il GameMode verifica tempo, punteggio e stato conclusivo della partita.
	const bool bMatchCompleted = GoodbyeGameMode->TryCompleteMatch();


	if (!bMatchCompleted)
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Finish Zone raggiunta, ma la partita non può ancora terminare"
			)
		);
	}
}