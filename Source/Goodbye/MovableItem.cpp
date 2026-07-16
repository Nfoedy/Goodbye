#include "MovableItem.h"

#include "Components/StaticMeshComponent.h"

AMovableItem::AMovableItem()
{
	// L'oggetto non ha bisogno di aggiornarsi ogni frame.
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	// L'oggetto deve reagire a gravità, urti e Physics Handle.
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetEnableGravity(true);
	ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	// Ci servirà successivamente per calcolare i danni da impatto.
	ItemMesh->SetNotifyRigidBodyCollision(true);
}