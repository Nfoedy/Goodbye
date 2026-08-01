#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CargoTruckPawn.generated.h"


class AGoodbyeCharacter;

class UBoxComponent;
class UCameraComponent;
class UChildActorComponent;
class UPrimitiveComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USpringArmComponent;

struct FHitResult;


UCLASS()
class GOODBYE_API ACargoTruckPawn : public APawn
{
	GENERATED_BODY()


public:

	ACargoTruckPawn();


	// Inserisce il Character nel camion e trasferisce il PlayerController al camion
	bool EnterVehicle(AGoodbyeCharacter* RequestingCharacter);


	// Indica se il Character può entrare nel camion
	UFUNCTION(BlueprintPure, Category = "Vehicle|Interaction")
	bool CanEnterVehicle() const
	{
		return bCanEnterVehicle;
	}


	// Indica se il camion è attualmente posseduto dal giocatore
	UFUNCTION(BlueprintPure, Category = "Vehicle|Driver")
	bool IsDriving() const
	{
		return bIsDriving;
	}


protected:

	virtual void BeginPlay() override;

	// Mesh principale del camion, gli asset vengono assegnati nel BP figlio
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Components")
	TObjectPtr<USkeletalMeshComponent> TruckMesh = nullptr;


	// Child Actor che contiene BP_CargoZone
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Cargo")
	TObjectPtr<UChildActorComponent> CargoZoneChild = nullptr;


	// Zona vicino alla portiera che rileva il Character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Interaction")
	TObjectPtr<UBoxComponent> DriverInteractZone = nullptr;


	//Punto nel quale verrà posizionato il Character quando uscirà dal Cargo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Interaction")
	TObjectPtr<USceneComponent> DriverExitPoint = nullptr;


	// Punto di rif per posizionare il driver all'interno della cabina
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Driver")
	TObjectPtr<USceneComponent> DriverSeatPoint = nullptr;


	// Mesh visiva del driver seduto
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Driver")
	TObjectPtr<USkeletalMeshComponent> DriverMesh = nullptr;

	// Braccio della Camera in terza persona
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Camera")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	// Camera utilizzata durante la guida 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Camera")
	TObjectPtr<UCameraComponent> Camera = nullptr;


private:

	// Character attualmente vicino alla portiera
	UPROPERTY(Transient)
	TObjectPtr<AGoodbyeCharacter> NearbyCharacter = nullptr;


	// Character originale del giocatore, conservato durante la guida
	UPROPERTY(Transient)
	TObjectPtr<AGoodbyeCharacter> DriverCharacter = nullptr;


	// Indica che il Character è dentro la zona di interazione
	bool bCanEnterVehicle = false;


	// Indica che il giocatore sta guidando
	bool bIsDriving = false;


	// Eventi della zona di interazione
	UFUNCTION()
	void HandleDriverZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


	UFUNCTION()
	void HandleDriverZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex
	);
};