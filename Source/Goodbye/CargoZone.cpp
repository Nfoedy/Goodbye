#include "CargoZone.h"

#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "MovableItem.h"


// Costruttore
ACargoZone::ACargoZone()
{
	// La Cargo Zone non deve aggiornarsi ogni frame
	PrimaryActorTick.bCanEverTick = false;

	// Crea il volume della zona di carico
	CargoBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CargoBox"));

	// Il Box diventa il Root Component dell'Actor
	SetRootComponent(CargoBox);


	// Dimensione iniziale della zona, potrà essere modificata nel Blueprint
	CargoBox->SetBoxExtent(FVector(150.0f, 100.0f, 100.0f));


	// La Cargo Zone serve solo per rilevare overlap e non deve bloccare gli oggetti
	CargoBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);


	// La Cargo Zone ignora tutti gli altri canali
	CargoBox->SetCollisionResponseToAllChannels(ECR_Ignore);


	// Rileva gli oggetti fisici
	CargoBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);


	// Abilita la generazione degli eventi di overlap
	CargoBox->SetGenerateOverlapEvents(true);


	// Collega l'ingresso nella zona alla relativa callback
	CargoBox->OnComponentBeginOverlap.AddDynamic(this, &ACargoZone::HandleCargoBeginOverlap);


	// Collega l'uscita dalla zona alla relativa callback
	CargoBox->OnComponentEndOverlap.AddDynamic(this, &ACargoZone::HandleCargoEndOverlap);
}



// Entrata nella CargoZone
void ACargoZone::HandleCargoBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// Accetta solamente Actor di tipo AMovableItem
	AMovableItem* MovableItem = Cast<AMovableItem>(OtherActor);

	if (!IsValid(MovableItem))
	{
		return;
	}


	// Evita di contare più volte lo stesso oggetto
	if (LoadedItems.Contains(MovableItem))
	{
		return;
	}


	// Registra l'oggetto nella Cargo Zone
	LoadedItems.Add(MovableItem);


	// Se l'oggetto viene distrutto mentre si trova nel cassone, la Cargo Zone verrà avvisata
	MovableItem->OnDestroyed.AddDynamic(this, &ACargoZone::HandleLoadedItemDestroyed);


	// Aggiorna il punteggio totale
	RecalculateScore();

}



// Uscita dalla Cargo Zone
void ACargoZone::HandleCargoEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex
)
{
	AMovableItem* MovableItem = Cast<AMovableItem>(OtherActor);

	if (!IsValid(MovableItem))
	{
		return;
	}


	// Verifica che l'oggetto fosse effettivamente registrato
	if (!LoadedItems.Contains(MovableItem))
	{
		return;
	}



	// Un actor può avere più componenti
	if (CargoBox->IsOverlappingActor(MovableItem))
	{
		return;
	}


	// Rimuove il collegamento all'evento di distruzione
	MovableItem->OnDestroyed.RemoveDynamic(this, &ACargoZone::HandleLoadedItemDestroyed);


	// Rimuove l'oggetto dalla Cargo Zone
	LoadedItems.Remove(MovableItem);


	// Aggiorna il punteggio totale
	RecalculateScore();

}


// Distruzione di un oggetto caricato
void ACargoZone::HandleLoadedItemDestroyed(AActor* DestroyedActor)
{
	AMovableItem* MovableItem = Cast<AMovableItem>(DestroyedActor);

	if (!MovableItem)
	{
		return;
	}


	// Rimuove l'oggetto distrutto dall'insieme
	const int32 RemovedItems = LoadedItems.Remove(MovableItem);

	if (RemovedItems <= 0)
	{
		return;
	}


	// Aggiorna il punteggio dopo la distruzione
	RecalculateScore();

}



// CAlcolo del punteggio
void ACargoZone::RecalculateScore()
{
	CurrentScore = 0;

	for (AMovableItem* MovableItem : LoadedItems)
	{
		if (!IsValid(MovableItem))
		{
			continue;
		}

		CurrentScore += MovableItem->GetItemScore();
	}

	// Comunica ai Blueprint i nuovi valori
	OnCargoChanged.Broadcast(LoadedItems.Num(),	CurrentScore);
}