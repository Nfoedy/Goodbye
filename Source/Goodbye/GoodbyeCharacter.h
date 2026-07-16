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
	float GrabDistance = 180.0f;

	/** Distance at which the grabbed object is held in front of the camera. */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Grab",
		meta = (ClampMin = "50.0", Units = "cm")
	)
	float HoldDistance = 110.0f;

	/** Physical component currently held by the Physics Handle. */
	UPROPERTY(Transient)
	UPrimitiveComponent* GrabbedComponent = nullptr;

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