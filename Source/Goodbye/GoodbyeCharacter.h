// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GoodbyeCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UPhysicsHandleComponent;
class UPrimitiveComponent;

struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * A basic first person character
 */
UCLASS(abstract)
class AGoodbyeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view, seen only by the owning player. */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Components",
		meta = (AllowPrivateAccess = "true")
	)
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera. */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Components",
		meta = (AllowPrivateAccess = "true")
	)
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MouseLookAction;

	/** Left mouse input used to grab and release physical objects. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* GrabAction;

	/** Component used to grab and move objects that simulate physics. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab")
	UPhysicsHandleComponent* PhysicsHandle;

	/** Maximum distance at which an object can be grabbed. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Grab",
		meta = (ClampMin = "50.0", Units = "cm")
	)
	float GrabDistance = 100.0f;

	/** Distance at which the grabbed object is held in front of the camera. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Grab",
		meta = (ClampMin = "50.0", Units = "cm")
	)
	float HoldDistance = 75.0f;

	/** Physical component currently held by the Physics Handle. */
	UPROPERTY(Transient)
	UPrimitiveComponent* GrabbedComponent = nullptr;

	/** Componente individuato dal Line Trace, ma non ancora afferrato. */
	UPROPERTY(Transient)
	UPrimitiveComponent* PendingGrabComponent = nullptr;

	/** Punto colpito espresso nello spazio locale dell'oggetto in attesa. */
	FVector PendingLocalGrabPoint = FVector::ZeroVector;

	/** Nome del socket posizionato nel palmo destro. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Grab|Animation"
	)
	FName GrabSocketName = TEXT("GrabSocket");

	/** Distanza massima tra il palmo e l'oggetto per completare la presa. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Grab|Animation",
		meta = (ClampMin = "1.0", Units = "cm")
	)
	float GrabContactDistance = 8.0f;

	/** Tempo massimo concesso al braccio per raggiungere l'oggetto. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Grab|Animation",
		meta = (ClampMin = "0.1", Units = "s")
	)
	float MaxReachDuration = 1.0f;

	/** Indica che il braccio sta raggiungendo un oggetto. */
	bool bIsReachingToGrab = false;

	/** Indica che il pulsante del grab è ancora premuto. */
	bool bGrabInputHeld = false;

	/** Tempo trascorso dall'inizio del movimento del braccio. */
	float ReachElapsedTime = 0.0f;

	/** Original collision response of the object toward the Pawn channel. */
	ECollisionResponse OriginalPawnCollisionResponse = ECR_Block;

public:

	AGoodbyeCharacter();

	/** Updates the position of the held object every frame. */
	virtual void Tick(float DeltaTime) override;

protected:

	/** Called from Input Actions for movement input. */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input. */
	void LookInput(const FInputActionValue& Value);

	/** Starts grabbing the physical object in front of the camera. */
	void StartGrab(const FInputActionValue& Value);

	/** Releases the currently grabbed object. */
	void StopGrab(const FInputActionValue& Value);

	/** Moves the grabbed object toward the holding point. */
	void UpdateGrabbedObject();

	/** Afferra fisicamente l'oggetto quando il palmo lo raggiunge. */
	void CompleteGrab();

	/** Annulla un tentativo di presa non ancora completato. */
	void CancelPendingGrab();

	/** Punto del componente afferrato espresso nello spazio locale dell'oggetto. */
	FVector LocalGrabPoint = FVector::ZeroVector;

	/** Posizione mondiale che la mano destra deve raggiungere. */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|Animation")
	FVector RightHandIKTarget = FVector::ZeroVector;

	/** Posizione mondiale usata per controllare la direzione del gomito. */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|Animation")
	FVector RightElbowIKTarget = FVector::ZeroVector;

	/** Intensità dell'IK: 0 disattivato, 1 completamente applicato. */
	UPROPERTY(BlueprintReadOnly, Category = "Grab|Animation")
	float RightHandIKAlpha = 0.0f;

	/** Velocità con cui il braccio entra ed esce dalla posa di presa. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Grab|Animation",
		meta = (ClampMin = "0.1")
	)
	float HandIKInterpolationSpeed = 10.0f;

	/** Handles aim inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	/** Sets up input action bindings. */
	virtual void SetupPlayerInputComponent(
		UInputComponent* InputComponent
	) override;

public:

	/** Returns the first person mesh. */
	USkeletalMeshComponent* GetFirstPersonMesh() const
	{
		return FirstPersonMesh;
	}

	/** Returns the first person camera component. */
	UCameraComponent* GetFirstPersonCameraComponent() const
	{
		return FirstPersonCameraComponent;
	}
};