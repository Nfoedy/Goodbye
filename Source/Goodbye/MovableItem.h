#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovableItem.generated.h"

class UStaticMeshComponent;

UCLASS()
class GOODBYE_API AMovableItem : public AActor
{
	GENERATED_BODY()

public:
	AMovableItem();

protected:
	// Mesh fisica dell'oggetto trasportabile.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movable Item")
	TObjectPtr<UStaticMeshComponent> ItemMesh;
};