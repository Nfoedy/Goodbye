#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinishZone.generated.h"


class ALevelSequenceActor;
class UArrowComponent;
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

	// Restituisce la direzione verso cui il cargo deve proseguire durante la cinematica
	FVector GetCinematicForwardDirection() const;


protected:

	// Volume che rileva l'ingresso del cargo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Finish Zone|Components")
	TObjectPtr<UBoxComponent> FinishBox = nullptr;

	// Indica la direzione che il cargo dovrà seguire dopo aver ottenuto la vittoria
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Finish Zone|Cinematic")
	TObjectPtr<UArrowComponent> CinematicDirection = nullptr;


	// Level Sequence Actor utilizzato per riprodurre la cinematica di vittoria
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Finish Zone|Cinematic")
	TObjectPtr<ALevelSequenceActor> VictorySequenceActor = nullptr;


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

	// Avvia la Level Sequence e restituisce true se la riproduzione è partita
	bool PlayVictorySequence();


	// Gestisce la conclusione naturale della Level Sequence di vittoria
	UFUNCTION()
	void HandleVictorySequenceFinished();

};