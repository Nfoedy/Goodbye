// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoodbyeCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

#include "Goodbye.h"
#include "MovableItem.h"

AGoodbyeCharacter::AGoodbyeCharacter()
{
	// Il Character deve aggiornare ogni frame la posizione
	// dell'oggetto afferrato.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule.
	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	// Create the first person mesh that will be viewed
	// only by this character's owner.
	FirstPersonMesh =
		CreateDefaultSubobject<USkeletalMeshComponent>(
			TEXT("First Person Mesh")
		);

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType =
		EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// Create the Camera Component.
	FirstPersonCameraComponent =
		CreateDefaultSubobject<UCameraComponent>(
			TEXT("First Person Camera")
		);

	FirstPersonCameraComponent->SetupAttachment(
		FirstPersonMesh,
		TEXT("head")
	);

	FirstPersonCameraComponent->SetRelativeLocationAndRotation(
		FVector(-2.8f, 5.89f, 0.0f),
		FRotator(0.0f, 90.0f, -90.0f)
	);

	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Configure the character components.
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType =
		EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement.
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	// Componente che permette di trascinare oggetti
	// mantenendo attiva la simulazione fisica.
	PhysicsHandle =
		CreateDefaultSubobject<UPhysicsHandleComponent>(
			TEXT("Physics Handle")
		);

	// Velocità con cui l'oggetto raggiunge il punto target.
	PhysicsHandle->SetInterpolationSpeed(12.0f);
}

void AGoodbyeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGrabbedObject();

	const bool bIsHoldingObject =
		PhysicsHandle &&
		PhysicsHandle->GetGrabbedComponent() != nullptr &&
		GrabbedComponent != nullptr;

	const float TargetAlpha = bIsHoldingObject ? 1.0f : 0.0f;

	// Attiva e disattiva gradualmente l'IK,
	// evitando uno scatto immediato del braccio.
	RightHandIKAlpha = FMath::FInterpTo(
		RightHandIKAlpha,
		TargetAlpha,
		DeltaTime,
		HandIKInterpolationSpeed
	);

	if (!bIsHoldingObject || !FirstPersonCameraComponent)
	{
		return;
	}

	// Ricostruisce la posizione mondiale del punto afferrato.
	RightHandIKTarget =
		GrabbedComponent->GetComponentTransform()
		.TransformPosition(LocalGrabPoint);

	const FVector CameraLocation =
		FirstPersonCameraComponent->GetComponentLocation();

	const FVector CameraForward =
		FirstPersonCameraComponent->GetForwardVector();

	const FVector CameraRight =
		FirstPersonCameraComponent->GetRightVector();

	const FVector CameraUp =
		FirstPersonCameraComponent->GetUpVector();

	// Punto verso il quale deve orientarsi il gomito destro.
	RightElbowIKTarget =
		CameraLocation
		+ CameraForward * 35.0f
		+ CameraRight * 45.0f
		- CameraUp * 20.0f;
}

void AGoodbyeCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent
)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInputComponent)
	{
		UE_LOG(
			LogGoodbye,
			Error,
			TEXT(
				"'%s' Failed to find an Enhanced Input Component! "
				"This template is built to use the Enhanced Input system."
			),
			*GetNameSafe(this)
		);

		return;
	}

	// Jumping.
	EnhancedInputComponent->BindAction(
		JumpAction,
		ETriggerEvent::Started,
		this,
		&AGoodbyeCharacter::DoJumpStart
	);

	EnhancedInputComponent->BindAction(
		JumpAction,
		ETriggerEvent::Completed,
		this,
		&AGoodbyeCharacter::DoJumpEnd
	);

	// Moving.
	EnhancedInputComponent->BindAction(
		MoveAction,
		ETriggerEvent::Triggered,
		this,
		&AGoodbyeCharacter::MoveInput
	);

	// Looking and aiming.
	EnhancedInputComponent->BindAction(
		LookAction,
		ETriggerEvent::Triggered,
		this,
		&AGoodbyeCharacter::LookInput
	);

	EnhancedInputComponent->BindAction(
		MouseLookAction,
		ETriggerEvent::Triggered,
		this,
		&AGoodbyeCharacter::LookInput
	);

	// Grab: pressione del tasto sinistro.
	if (GrabAction)
	{
		EnhancedInputComponent->BindAction(
			GrabAction,
			ETriggerEvent::Started,
			this,
			&AGoodbyeCharacter::StartGrab
		);

		// Rilascio normale del tasto.
		EnhancedInputComponent->BindAction(
			GrabAction,
			ETriggerEvent::Completed,
			this,
			&AGoodbyeCharacter::StopGrab
		);

		// Rilascio nel caso in cui l'input venga interrotto.
		EnhancedInputComponent->BindAction(
			GrabAction,
			ETriggerEvent::Canceled,
			this,
			&AGoodbyeCharacter::StopGrab
		);
	}
}

void AGoodbyeCharacter::MoveInput(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	DoMove(MovementVector.X, MovementVector.Y);
}

void AGoodbyeCharacter::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AGoodbyeCharacter::StartGrab(
	const FInputActionValue& Value
)
{
	// Non possiamo prendere un secondo oggetto
	// mentre ne stiamo già trasportando uno.
	if (!PhysicsHandle ||
		PhysicsHandle->GetGrabbedComponent() != nullptr ||
		!FirstPersonCameraComponent ||
		!GetWorld())
	{
		return;
	}

	// Il Line Trace parte dalla telecamera.
	const FVector TraceStart =
		FirstPersonCameraComponent->GetComponentLocation();

	const FVector TraceDirection =
		FirstPersonCameraComponent->GetForwardVector();

	const FVector TraceEnd =
		TraceStart + TraceDirection * GrabDistance;

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	AMovableItem* MovableItem =
		Cast<AMovableItem>(HitResult.GetActor());

	UPrimitiveComponent* HitComponent =
		HitResult.GetComponent();

	// Possiamo prendere solamente oggetti derivati
	// da AMovableItem che simulano la fisica.
	if (!MovableItem ||
		!HitComponent ||
		!HitComponent->IsSimulatingPhysics())
	{
		return;
	}

	GrabbedComponent = HitComponent;

	// Salva il punto colpito relativamente all'oggetto.
	// In questo modo la mano continuerà a seguire lo stesso punto
	// anche quando l'oggetto cambia posizione o ruota.
	LocalGrabPoint =
		GrabbedComponent->GetComponentTransform()
		.InverseTransformPosition(HitResult.ImpactPoint);

	RightHandIKTarget = HitResult.ImpactPoint;

	// Salviamo la risposta originale verso il Pawn,
	// così potremo ripristinarla quando lasciamo l'oggetto.
	OriginalPawnCollisionResponse =
		GrabbedComponent->GetCollisionResponseToChannel(
			ECC_Pawn
		);

	// Evita che l'oggetto trasportato urti continuamente
	// il corpo del giocatore.
	GrabbedComponent->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Ignore
	);

	GrabbedComponent->WakeAllRigidBodies();

	// L'oggetto viene afferrato nel punto preciso
	// colpito dal Line Trace.
	PhysicsHandle->GrabComponentAtLocation(
		GrabbedComponent,
		NAME_None,
		HitResult.ImpactPoint
	);
}

void AGoodbyeCharacter::StopGrab(
	const FInputActionValue& Value
)
{
	if (!PhysicsHandle)
	{
		return;
	}

	PhysicsHandle->ReleaseComponent();

	if (!GrabbedComponent)
	{
		return;
	}

	// Ripristina le collisioni con il giocatore.
	GrabbedComponent->SetCollisionResponseToChannel(
		ECC_Pawn,
		OriginalPawnCollisionResponse
	);

	GrabbedComponent->WakeAllRigidBodies();
	GrabbedComponent = nullptr;
}

void AGoodbyeCharacter::UpdateGrabbedObject()
{
	if (!PhysicsHandle ||
		!PhysicsHandle->GetGrabbedComponent() ||
		!FirstPersonCameraComponent)
	{
		return;
	}

	const FVector CameraLocation =
		FirstPersonCameraComponent->GetComponentLocation();

	const FVector CameraForward =
		FirstPersonCameraComponent->GetForwardVector();

	const FVector TargetLocation =
		CameraLocation + CameraForward * HoldDistance;

	// Il Physics Handle trascina l'oggetto verso
	// il punto davanti alla telecamera.
	PhysicsHandle->SetTargetLocation(TargetLocation);
}

void AGoodbyeCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AGoodbyeCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AGoodbyeCharacter::DoJumpStart()
{
	Jump();
}

void AGoodbyeCharacter::DoJumpEnd()
{
	StopJumping();
}