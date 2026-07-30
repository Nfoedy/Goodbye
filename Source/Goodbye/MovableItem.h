#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovableItem.generated.h"


class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;


/**
 * Categoria che rappresenta la grandezza gameplay dell'oggetto trasportabile *
 * La categoria determina il punteggio dell'oggetto, ma non modifica la sua massa fisica.
 */

UENUM(BlueprintType)
enum class EMovableItemSize : uint8
{
	Small		UMETA(DisplayName = "Small"),
	Medium		UMETA(DisplayName = "Medium"),
	Large		UMETA(DisplayName = "Large"),
	VeryLarge	UMETA(DisplayName = "Very Large")
};


/**
 * Classe base di tutti gli oggetti trasportabili.
 *
 * Gestisce:
 * - Static Mesh;
 * - simulazione fisica;
 * - gravità;
 * - collisioni;
 * - lettura della massa fisica;
 * - categoria di grandezza;
 * - punteggio.
 */
UCLASS()
class GOODBYE_API AMovableItem : public AActor
{
	GENERATED_BODY()


public:

	// Costruttore
	AMovableItem();


	// Restituisce il punteggio dell'oggetto 
	// Small = 1 pt
	// Medium = 2 pt
	// Large = 3 pt
	// Very Large = 4 pt
	UFUNCTION(BlueprintPure, Category = "Movable Item|Score")
	int32 GetItemScore() const;


	// Restituisce la massa effettiva utilizzata dalla static Mesh
	UFUNCTION(BlueprintPure, Category = "Movable Item|Physics")
	float GetItemMassInKg() const;


	// Restituisce la categoria di grandezza dell'oggetto
	UFUNCTION(BlueprintPure, Category = "Movable Item|Score")
	EMovableItemSize GetItemSize() const
	{
		return ItemSize;
	}


	// restituisce il componente static mesh dell'oggetto
	UStaticMeshComponent* GetItemMesh() const
	{
		return ItemMesh.Get();
	}

	// Istante dell'ultimo impatto significativo registrato.
	float LastImpactTime = -1000.0f;


/* COMPONENTI */
protected:

	// Mesh visiva e fisicamente dell'oggetto
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movable Item|Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh = nullptr;


/* CONFIGURAZIONE DEL PUNTEGGIO */
protected:

	// Categoria di grandezza gameplay dell'oggetto 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movable Item|Score")
	EMovableItemSize ItemSize = EMovableItemSize::Small;


/* CONFIGURAZIONE DEGLI IMPATTI */
protected:

	// Gravità minima dell'impatto necessaria per considerare significativo un urto.
    // Il valore è espresso approssimativamente in cm/s.
	UPROPERTY(EditDefaultsOnly,	BlueprintReadOnly,	Category = "Movable Item|Impact", meta = (ClampMin = "0.0", Units = "cm/s"))
	float MinimumImpactSeverity = 250.0f;


	// Tempo minimo tra due impatti significativi.
	// Evita che una singola caduta produca molti eventi consecutivi.
	UPROPERTY(EditDefaultsOnly,	BlueprintReadOnly,	Category = "Movable Item|Impact", meta = (ClampMin = "0.0", Units = "s")	)
	float ImpactCooldown = 0.35f;





/* EVENTI FISICI */
private:

	// Riceve gli eventi di collisione della Stati Mesh
	UFUNCTION()
	void HandleItemHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

};