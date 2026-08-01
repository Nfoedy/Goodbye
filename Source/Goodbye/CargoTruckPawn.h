#pragma once

#include "CoreMinimal.h"
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

struct FHitResult;


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


	// Gestisce IA_Interact quando il camion è posseduto.
	void HandleInteract();


	// Richiamata quando un Actor entra nella zona
	// di interazione della portiera.
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
};