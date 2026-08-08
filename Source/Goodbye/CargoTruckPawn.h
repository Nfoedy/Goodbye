#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "WheeledVehiclePawn.h"
#include "CargoTruckPawn.generated.h"


class AGoodbyeCharacter;

class UBoxComponent;
class UCameraComponent;
class UChildActorComponent;
class UInputAction;
class UInputComponent;
class UPrimitiveComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class USoundBase;

struct FHitResult;
struct FInputActionValue;


UCLASS()
class GOODBYE_API ACargoTruckPawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()


public:

	// Costruttore
	ACargoTruckPawn(const FObjectInitializer& ObjectInitializer);


	// Collega gli Input Action quando il camion viene posseduto.
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;


	// Inserisce il Character nel camion e trasferisce il PlayerController al veicolo.
	bool EnterVehicle(AGoodbyeCharacter* RequestingCharacter);


	// Fa uscire il giocatore dal camion e restituisce  il controllo al Character originale.
	bool ExitVehicle();


	// Aggiorna la coppia del motore utilizzando il moltiplicatore corrente
	void RefreshEnginePower();


	// Avvia la guida automatica usata durante la cinematica di vittoria
	void StartVictoryAutoDrive(const FVector& InDirection);


	// Indica se il Character può entrare nel camion.
	UFUNCTION(BlueprintPure, Category = "Vehicle|Interaction")
	bool CanEnterVehicle() const
	{
		return bCanEnterVehicle;
	}


	// Indica se il camion è attualmente controllato dal giocatore.
	UFUNCTION(BlueprintPure, Category = "Vehicle|Driver")
	bool IsDriving() const
	{
		return bIsDriving;
	}



protected:

	virtual void BeginPlay() override;

	// Aggiorna la guida automatica durante la vittoria.
	virtual void Tick(float DeltaTime) override;


	// Child Actor che contiene BP_CargoZone.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Cargo")
	TObjectPtr<UChildActorComponent> CargoZoneChild = nullptr;


	// Zona vicino alla portiera che rileva il Character.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Interaction")
	TObjectPtr<UBoxComponent> DriverInteractZone = nullptr;


	// Punto nel quale viene posizionato il Character quando esce dal cargo.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Interaction")
	TObjectPtr<USceneComponent> DriverExitPoint = nullptr;


	// Punto di riferimento usato per posizionare il guidatore nella cabina
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Driver")
	TObjectPtr<USceneComponent> DriverSeatPoint = nullptr;


	// Mesh visiva del guidatore seduto.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Driver")
	TObjectPtr<USkeletalMeshComponent> DriverMesh = nullptr;


	// Braccio della Camera in terza persona.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Camera")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;


	// Camera utilizzata durante la guida.
	UPROPERTY( VisibleAnywhere,	BlueprintReadOnly, Category = "Vehicle|Camera")
	TObjectPtr<UCameraComponent> Camera = nullptr;


	// Azione generica di interazione associata al tasto E.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction = nullptr;


	// Accelerazione del cargo.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ThrottleAction = nullptr;


	// Freno del cargo.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BrakeAction = nullptr;


	// Sterzo del cargo.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SteeringAction = nullptr;


private:

	// Character attualmente vicino alla portiera.
	UPROPERTY(Transient)
	TObjectPtr<AGoodbyeCharacter> NearbyCharacter = nullptr;


	// Character originale conservato durante la guida.
	UPROPERTY(Transient)
	TObjectPtr<AGoodbyeCharacter> DriverCharacter = nullptr;


	// Indica che un Character valido si trova
	// dentro la zona di interazione.
	bool bCanEnterVehicle = false;


	// Indica che il giocatore sta guidando.
	bool bIsDriving = false;


	// Gestisce IA_Interact quando il cargo è posseduto.
	void HandleInteract();


	// Gestisce l'accelerazione.
	void HandleThrottle(const FInputActionValue& Value);


	// Azzera l'accelerazione quando W viene rilasciato.
	void StopThrottle();


	// Gestisce il freno.
	void HandleBrake(const FInputActionValue& Value);


	// Azzera il freno quando S viene rilasciato.
	void StopBrake();


	// Gestisce lo sterzo.
	void HandleSteering(const FInputActionValue& Value);


	// Riporta lo sterzo al centro.
	void StopSteering();

	// Collega i componenti visivi e di gameplay al bone fisico principale del cargo.
	void AttachComponentsToVehicleBody();


	// Attiva o disattiva la simulazione fisica del cargo
	void SetVehicleSimulationEnabled(bool bEnabled);


	// Avvia il controllo che parcheggerà automaticamente il cargo quando avrà smesso di muoversi
	void StartAutoParkCheck();


	// Interrompe il controllo automatico del parcheggio.
	void StopAutoParkCheck();


	// Controlla periodicamente la velocità del cargo.
	void CheckAutoPark();


	// Intervallo tra i controlli della velocità.
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Parking", meta = (ClampMin = "0.05"))
	float AutoParkCheckInterval = 0.20f;


	// Velocità lineare sotto la quale il cargo viene considerato quasi fermo, in cm/s.
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Parking", meta = (ClampMin = "0.0"))
	float AutoParkLinearSpeedThreshold = 5.0f;


	// Velocità angolare sotto la quale il cargo viene considerato quasi fermo, in gradi/s.
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Parking", meta = (ClampMin = "0.0"))
	float AutoParkAngularSpeedThreshold = 2.0f;


	// Tempo per cui il cargo deve rimanere quasi fermo prima di essere bloccato.
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Parking", meta = (ClampMin = "0.0"))
	float AutoParkStableDuration = 1.0f;


	// Tempo accumulato sotto le soglie di movimento.
	float AutoParkStableElapsed = 0.0f;


	// Timer utilizzato dal controllo automatico.
	FTimerHandle AutoParkTimerHandle;


	// Aggiorna accelerazione e sterzo automatici
	void UpdateVictoryAutoDrive(float DeltaTime);


	// Richiamata quando un Actor entra nella zona di interazione della portiera.
	UFUNCTION()
	void HandleDriverZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


	// Richiamata quando un Actor esce dalla zona
	// di interazione della portiera.
	UFUNCTION()
	void HandleDriverZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex
	);

	// Suono riprodotto quando il giocatore accende il Cargo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle|Audio",	meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> EngineStartSound = nullptr;


	// Suono riprodotto quando il giocatore spegne il Cargo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> EngineStopSound = nullptr;


	// REFLECTION
	// Moltiplicatore usato per modificare la potenza del motore.
	// Questa costituisce il target del CargoSpeedPowerUp, viene individuata a runtime tramite il nome "EnginePowerMultiplier".
	// Val iniziale: 1.0
	// Val dopo PowerUp: 1.15 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle|Power Up", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float EnginePowerMultiplier = 1.0f;


	// Coppia massima base del motore, espressa in Nm
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vehicle|Engine", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BaseMaxEngineTorque = 500.0f;


	// Accelerazione applicata durante la cinematica.
	UPROPERTY(EditDefaultsOnly,	Category = "Vehicle|Victory", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VictoryThrottleInput = 0.25f;


	// Differenza angolare necessaria per applicare sterzo completo
	UPROPERTY(EditDefaultsOnly,	Category = "Vehicle|Victory", meta = (ClampMin = "1.0"))
	float VictoryFullSteeringAngle = 35.0f;


	// Velocità con cui lo sterzo automatico cambia valore
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Victory", meta = (ClampMin = "0.1"))
	float VictorySteeringInterpolationSpeed = 2.5f;


	// Indica che il cargo sta avanzando automaticamente.
	bool bVictoryAutoDrive = false;


	// Direzione mondiale indicata dalla FinishZone
	FVector VictoryDriveDirection = FVector::ForwardVector;


	// Valore di sterzo automatico attualmente applicato
	float CurrentVictorySteeringInput = 0.0f;

};