#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GoodbyeCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UPhysicsHandleComponent;
class UPrimitiveComponent;
class ACargoTruckPawn;
class USceneComponent;
class USoundBase;

struct FInputActionValue;



/**
 * Character prima persona, eredita da Character
 * 
 * Definisce tutto ciò che il Goodbye Character possiede e tutte le azioni che può eseguire.
 */

UCLASS(abstract)
class AGoodbyeCharacter : public ACharacter
{
	GENERATED_BODY()


/* PUBLIC */
public:

	// Costruttore 
	AGoodbyeCharacter();

	// Aggiornamento ad ogni Tick
	virtual void Tick(float DeltaTime) override;

	// Restituisce il puntatore della Skeletal Mesh usata in prima persona
	USkeletalMeshComponent* GetFirstPersonMesh() const
	{
		return FirstPersonMesh;
	}

	// Restituisce il puntatore della Camera del Character
	UCameraComponent* GetFirstPersonCameraComponent() const
	{
		return FirstPersonCameraComponent;
	}

	// Registra il camion vicino al Character
	void SetNearbyCargoTruck(ACargoTruckPawn* CargoTruck);

	// Rimuove il camion quando il Character si allontana
	void ClearNearbyCargoTruck(ACargoTruckPawn* CargoTruck);

	// Comunica al Blueprint se il mirino deve essere visibile
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void SetAimDotVisibility(bool bVisible);


/* COMPONENTI */
private:

	// Mesh del personaggio visibile solamente al giocatore proprietario
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh = nullptr;

	// Telecamera principale utilizzata dal giocatore
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent = nullptr;

	// Punto davanti alla Camera verso cui viene trasportato l'oggetto
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab", meta = (AllowPrivateAccess = "true"))
	USceneComponent* GrabHoldPoint = nullptr;


	// Componente che permette di afferrare e trascinare oggetti 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab", meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* PhysicsHandle = nullptr;


	// Camion attualmente vicino al Character
	UPROPERTY(Transient)
	TObjectPtr<ACargoTruckPawn> NearbyCargoTruck = nullptr;


	// Gestisce la pressione del comando di interazione
	void HandleInteract();

	// Suono riprodotto quando un oggetto viene realmente afferrato.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> GrabSound = nullptr;


	// Suono riprodotto quando un oggetto viene rilasciato.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> DropSound = nullptr;



/* INPUT ACTION */
protected:

	// Jump Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	// Move Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	// Look Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// Mouse Look Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MouseLookAction;

	// Grab Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* GrabAction;


	// Interact Action (per entrare nel Cargo)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;


/* CONFIGURAZIONE DEL GRAB*/


	// Massima distanza che un oggetto può essere grabbato
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab", meta = (ClampMin = "50.0", Units = "cm"))
	float GrabDistance = 200.0f;

	// Tempo durante il quale il braccio si muove verso l'oggetto prima che il Physic Handle completi la presa
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab|Animation", meta = (ClampMin = "0.05", Units = "s"))
	float ReachDuration = 0.20f;


	// Massa massima che il Character può sollevare
	// Gli oggetti oltre questo valore possono essere spinti fisicamente, ma non afferrati
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab|Weight", meta = (ClampMin = "1.0",	Units = "kg"))
	float MaxGrabbableMass = 750.0f;


	// Velocità del Physics Handle con oggetti leggeri
	UPROPERTY(EditAnywhere,	BlueprintReadOnly,	Category = "Grab|Weight", meta = (ClampMin = "0.1"))
	float LightObjectInterpolationSpeed = 12.0f;


	// Velocità minima del Physic Handle quando viene trasportato un oggetto pesante
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab|Weight", meta = (ClampMin = "0.1"))
	float HeavyObjectInterpolationSpeed = 3.0f;


/* CONFIGURAZIONE DELL'IK */


	// Posizione mondiale che la mano destra deve raggiungere
	UPROPERTY(BlueprintReadOnly, Category = "Grab|Animation")
	FVector RightHandIKTarget = FVector::ZeroVector;

	// Posizione mondiale usata per controllare la direzione del gomito
	UPROPERTY(BlueprintReadOnly, Category = "Grab|Animation")
	FVector RightElbowIKTarget = FVector::ZeroVector;

	// Intensità con cui viene applicato il TwoBone IK
	// 0 = disattivato , 1 = applicato completamente
	UPROPERTY(BlueprintReadOnly, Category = "Grab|Animation")
	float RightHandIKAlpha = 0.0f;

	// Velocità con cui RightHandIKAlpha raggiunge il valore desiderato
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab|Animation", meta = (ClampMin = "0.1"))
	float HandIKInterpolationSpeed = 10.0f;


/* STATO INTERNO DEL GRAB */
private:

	// Componente fisico attualmente afferrato dal Physics Handle
	UPROPERTY(Transient)
	UPrimitiveComponent* GrabbedComponent = nullptr;

	//Componente individuato dal Line Trace, ma non ancora afferrato fisicamente 
	UPROPERTY(Transient)
	UPrimitiveComponent* PendingGrabComponent = nullptr;

	// Punto colpito espresso nello spazio locale dell'oggeto in attesa
	FVector PendingLocalGrabPoint = FVector::ZeroVector;


	// Punto del componente afferrato espresso nello spazio locale dell'oggetto
	FVector LocalGrabPoint = FVector::ZeroVector;


	// Indica se il braccio sta tentando di raggiungere un oggetto non ancora afferrato
	bool bIsReachingToGrab = false;

	// Indica che il pulsante del grab è ancora premuto
	bool bGrabInputHeld = false;

	// Tempo trascorso dall'inizio del movimento del braccio
	float ReachElapsedTime = 0.0f;

	// Risposta originale dell'oggetto al canale Pawn
	// Viene salvata prima del grab e ripristinata quando l'oggetto viene rilasciato
	ECollisionResponse OriginalPawnCollisionResponse = ECR_Block;




/* CONFIGURAZIONE DEGLI INPUT */
protected:

	// Collega le Input Action alle funzioni del Cahracter
	// Unreal richiama automaticamente questa funzione quando il Character viene posseduto da un Controller
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;




/* CALLBACK DELL' ENHANCED INPUT*/
private:

	// Riceve il valore dell'InputAction di movimento
	void MoveInput(const FInputActionValue& Value);

	// Riceve il valore dell'Input Action della visuale
	void LookInput(const FInputActionValue& Value);

	// Avvia un tentativo di Grab
	void StartGrab(const FInputActionValue& Value);

	// Termina il grab quando il giocatore rilascia il comando
	void StopGrab(const FInputActionValue& Value);

	// Configura il Physics Handle in base alla massa dell'oggetto afferrato
	void ApplyGrabWeight(float ObjectMassInKg);

	// Ripristina i valori normali del Physics Handle
	void ResetGrabWeight();



/* FUNZIONI INTERNE DEL SISTEMA DI GRAB */
private:

	// Aggiorna la posizione target del Physics Handle
	void UpdateGrabbedObject();

	// Completa la presa fisica dell'oggetto 
	void CompleteGrab();

	// Annulla un tentativo di presa non ancora completato
	void CancelPendingGrab();



/* AZIONI DI MOVIMENTO UTILIZZABILI ANCHE DAI BP */
protected:

	// Applica una rotazione alla visuale
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	// Muove il Character nelle direzioni indicate 
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	// Avvia il salto del Character
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	// Termina il salto del Character
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();


};