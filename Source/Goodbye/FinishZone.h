#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinishZone.generated.h"


class UBoxComponent;
class UPrimitiveComponent;

struct FHitResult;


/**
 * Zona di uscita che permette di completare la partita.
 *
 * La vittoria viene richiesta solamente quando:
 * - entra il cargo;
 * - il camion è guidato dal Player;
 * - il tempo non è terminato;
 * - il punteggio richiesto è stato raggiunto.
 */

UCLASS()
class GOODBYE_API AFinishZone : public AActor
{
	GENERATED_BODY()


public:

	// Costruttore.
	AFinishZone();


protected:

	// Volume che rileva l'ingresso del cargo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Finish Zone|Components")
	TObjectPtr<UBoxComponent> FinishBox = nullptr;


private:

	// Richiamata quando un Actor entra nella Finish Zone.
	UFUNCTION()
	void HandleFinishZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};