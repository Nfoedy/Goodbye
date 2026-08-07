#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CargoSpeedPowerUp.generated.h"


class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

struct FHitResult;


UCLASS()
class GOODBYE_API ACargoSpeedPowerUp : public AActor
{
	GENERATED_BODY()


public:

	// Costruttore.
	ACargoSpeedPowerUp();


protected:

	// Inizializza il PowerUp e registra l'evento di overlap.
	virtual void BeginPlay() override;


	// Rimuove il collegamento all'evento di overlap.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	// Aggiorna la rotazione e la fluttuazione della moneta.
	virtual void Tick(float DeltaTime) override;


private:

	// Root del PowerUp.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power Up", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot = nullptr;


	// Mesh visiva della moneta.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power Up", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CoinMesh = nullptr;


	// Zona che rileva il passaggio del camion.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power Up", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerBox = nullptr;


	// Nome della proprietà che verrà cercata tramite Reflection.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Up|Reflection", meta = (AllowPrivateAccess = "true"))
	FName TargetProperty = TEXT("EnginePowerMultiplier");


	// Moltiplicatore applicato alla proprietà trovata.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Up|Reflection", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float Multiplier = 1.15f;


	// Velocità di rotazione della moneta in gradi al secondo.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Up|Animation", meta = (AllowPrivateAccess = "true"))
	float RotationSpeed = 120.0f;


	// Altezza massima della fluttuazione rispetto alla posizione iniziale.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Up|Animation", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FloatingAmplitude = 20.0f;


	// Velocità della fluttuazione verticale.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Up|Animation", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FloatingSpeed = 2.0f;


	// Posizione relativa iniziale della mesh.
	FVector InitialCoinRelativeLocation = FVector::ZeroVector;


	// Tempo utilizzato per calcolare il movimento sinusoidale.
	float FloatingElapsedTime = 0.0f;


	// Gestisce l'ingresso di un Actor nella zona del PowerUp.
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};