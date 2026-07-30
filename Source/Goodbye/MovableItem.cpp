#include "MovableItem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"


// Costruttore 
AMovableItem::AMovableItem()
{

	// L'actor non deve eseguire operazioni ogni frame
	PrimaryActorTick.bCanEverTick = false;

	// Crea il componente della static mesh del componente
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));

	// La static mesh diventa il root component dell'actor
	SetRootComponent(ItemMesh);


	// Attiva la simulazione fisica
	ItemMesh->SetSimulatePhysics(true);


	// Attiva la gravità
	ItemMesh->SetEnableGravity(true);

	// Applica il profilo di collisione PhysicsActor
	ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	// Abilita la generazione degli eventi OnComponentHit
	ItemMesh->SetNotifyRigidBodyCollision(true);

	
	// Collega l'evento di collisione della mesh alla funzione HandleItemHit()
	ItemMesh->OnComponentHit.AddDynamic(this, &AMovableItem::HandleItemHit);
}


// Massa
float AMovableItem::GetItemMassInKg() const
{
	
	// Se la mesh non è valida restituisce 0
	if (!IsValid(ItemMesh))
	{
		return 0.0f;
	}

	// Restituisce la massa effettiva 
	return ItemMesh->GetMass();
}



void AMovableItem::HandleItemHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse,	const FHitResult& Hit)
{
	// Ignora collisioni non valide o con sé stesso.
	if (!IsValid(ItemMesh) || !IsValid(OtherActor) || OtherActor == this ||	!GetWorld())
	{
		return;
	}

	// Legge la massa fisica reale della Static Mesh.
	const float ObjectMassInKg = FMath::Max(ItemMesh->GetMass(), 0.1f);

	// Normalizza l'impulso rispetto alla massa.
	const float ImpactSeverity = NormalImpulse.Size() / ObjectMassInKg;

	// Ignora piccoli urti, vibrazioni e assestamenti.
	if (ImpactSeverity < MinimumImpactSeverity)
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// Evita di registrare più volte lo stesso impatto.
	if (CurrentTime - LastImpactTime < ImpactCooldown)
	{
		return;
	}

	LastImpactTime = CurrentTime;

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"%s ha colpito %s | "
			"Massa: %.1f kg | "
			"Impulso: %.2f | "
			"Severita impatto: %.2f cm/s"
		),
		*GetName(),
		*GetNameSafe(OtherActor),
		ObjectMassInKg,
		NormalImpulse.Size(),
		ImpactSeverity
	);
}



// Punteggio
int32 AMovableItem::GetItemScore() const
{
	// Converte la categoria di grandezza nel relativo punteggio.
	switch (ItemSize)
	{
	case EMovableItemSize::Small:
		return 1;

	case EMovableItemSize::Medium:
		return 2;

	case EMovableItemSize::Large:
		return 3;

	case EMovableItemSize::VeryLarge:
		return 4;

	default:
		return 1;
	}
}