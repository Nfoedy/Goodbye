#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CargoZone.generated.h"


class UBoxComponent;
class UPrimitiveComponent;
class AMovableItem;

struct FHitResult;


/**
 * Zona che rileva gli oggetti caricati nel furgone
 *
 * Gestisce:
 * - ingresso degli oggetti;
 * - uscita degli oggetti;
 * - conteggio degli oggetti caricati;
 * - calcolo del punteggio totale;
 * - rimozione degli oggetti distrutti.
 */

UCLASS()
class GOODBYE_API ACargoZone : public AActor
{
	GENERATED_BODY()


/* PUBLIC */
public:

	// Costruttore
	ACargoZone();


	// Restituisce il punteggio totale degli oggetti presenti nella Cargo Zone
	UFUNCTION(BlueprintPure, Category = "Cargo Zone")
	int32 GetCurrentScore() const
	{
		return CurrentScore;
	}


	// Restituisce il numero di oggetti attualmente presenti nella Cargo Zone
	UFUNCTION(BlueprintPure, Category = "Cargo Zone")
	int32 GetLoadedItemCount() const
	{
		return LoadedItems.Num();
	}


/* COMPONENTI */
protected:

	// Volume che rappresenta la zona interna del cassone del furgone
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cargo Zone|Components")
	TObjectPtr<UBoxComponent> CargoBox = nullptr;


/* STATO INTERNO */
private:

	// Insieme degli oggetti presenti nella Cargo Zone
	// TSet impedisce di inserire due volte lo stesso oggetto
	TSet<AMovableItem*> LoadedItems;


	// Punteggio totale degli oggetti caricati
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Cargo Zone", meta = (AllowPrivateAccess = "true"))
	int32 CurrentScore = 0;


/* EVENTI DI OVERLAP */
private:

	// Richiamata quando un componente entra nella Cargo Zone
	UFUNCTION()
	void HandleCargoBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


	// Richiamata quando un componente esce dalla Cargo Zone
	UFUNCTION()
	void HandleCargoEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex
	);


	// Richiamata quando un oggetto caricato viene distrutto
	UFUNCTION()
	void HandleLoadedItemDestroyed(AActor* DestroyedActor);


/* FUNZIONI INTERNE */
private:

	// Ricalcola il punteggio degli oggetti
	// attualmente presenti nella Cargo Zone
	void RecalculateScore();
};