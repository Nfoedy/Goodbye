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

	if (!FirstPersonCameraComponent)
	{
		return;
	}

	const FVector CameraLocation =
		FirstPersonCameraComponent->GetComponentLocation();

	const FVector CameraForward =
		FirstPersonCameraComponent->GetForwardVector();

	const FVector CameraRight =
		FirstPersonCameraComponent->GetRightVector();

	const FVector CameraUp =
		FirstPersonCameraComponent->GetUpVector();

	/*
	 * FASE 1:
	 * Il braccio sta raggiungendo l'oggetto,
	 * che rimane ancora fermo.
	 */
	if (bIsReachingToGrab)
	{
		if (!bGrabInputHeld ||
			!IsValid(PendingGrabComponent))
		{
			CancelPendingGrab();
			return;
		}

		// Il punto può cambiare posizione se l'oggetto
		// viene urtato mentre la mano lo sta raggiungendo.
		const FVector PendingGrabWorldPoint =
			PendingGrabComponent
			->GetComponentTransform()
			.TransformPosition(PendingLocalGrabPoint);

		RightHandIKTarget = PendingGrabWorldPoint;

		RightElbowIKTarget =
			CameraLocation
			+ CameraForward * 35.0f
			+ CameraRight * 45.0f
			- CameraUp * 20.0f;

		RightHandIKAlpha = FMath::FInterpTo(
			RightHandIKAlpha,
			1.0f,
			DeltaTime,
			HandIKInterpolationSpeed
		);

		ReachElapsedTime += DeltaTime;

		// Usiamo il socket della mesh visibile in prima persona,
		// perché rappresenta il palmo visto dal giocatore.
		if (!FirstPersonMesh ||
			!FirstPersonMesh->DoesSocketExist(GrabSocketName))
		{
			UE_LOG(
				LogGoodbye,
				Warning,
				TEXT(
					"GrabSocket '%s' non trovato sulla FirstPersonMesh."
				),
				*GrabSocketName.ToString()
			);

			CancelPendingGrab();
			return;
		}

		const FVector HandSocketLocation =
			FirstPersonMesh->GetSocketLocation(GrabSocketName);

		const float HandToObjectDistance =
			FVector::Distance(
				HandSocketLocation,
				PendingGrabWorldPoint
			);

		// La presa fisica avviene solo quando il palmo
		// è abbastanza vicino al punto dell'oggetto.
		if (HandToObjectDistance <= GrabContactDistance)
		{
			CompleteGrab();
			return;
		}

		// Evita che il personaggio rimanga bloccato
		// in una presa impossibile.
		if (ReachElapsedTime >= MaxReachDuration)
		{
			CancelPendingGrab();
		}

		return;
	}

	/*
	 * FASE 2:
	 * L'oggetto è stato afferrato e segue la telecamera.
	 */
	UpdateGrabbedObject();

	const bool bIsHoldingObject =
		PhysicsHandle &&
		PhysicsHandle->GetGrabbedComponent() &&
		GrabbedComponent;

	const float TargetAlpha =
		bIsHoldingObject ? 1.0f : 0.0f;

	RightHandIKAlpha = FMath::FInterpTo(
		RightHandIKAlpha,
		TargetAlpha,
		DeltaTime,
		HandIKInterpolationSpeed
	);

	if (!bIsHoldingObject)
	{
		return;
	}

	// La mano continua a seguire lo stesso punto
	// dell'oggetto durante il trasporto.
	RightHandIKTarget =
		GrabbedComponent
		->GetComponentTransform()
		.TransformPosition(LocalGrabPoint);

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
	bGrabInputHeld = true;

	// Non iniziare una nuova presa se ne esiste già una.
	if (!PhysicsHandle ||
		PhysicsHandle->GetGrabbedComponent() ||
		bIsReachingToGrab ||
		!FirstPersonCameraComponent ||
		!GetWorld())
	{
		return;
	}

	const FVector TraceStart =
		FirstPersonCameraComponent->GetComponentLocation();

	const FVector TraceEnd =
		TraceStart +
		FirstPersonCameraComponent->GetForwardVector() * GrabDistance;

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

	if (!MovableItem ||
		!HitComponent ||
		!HitComponent->IsSimulatingPhysics())
	{
		return;
	}

	// Per ora salviamo soltanto l'oggetto:
	// il Physics Handle non lo afferra ancora.
	PendingGrabComponent = HitComponent;

	PendingLocalGrabPoint =
		PendingGrabComponent
		->GetComponentTransform()
		.InverseTransformPosition(HitResult.ImpactPoint);

	// La mano deve raggiungere il punto preciso colpito dal trace.
	RightHandIKTarget = HitResult.ImpactPoint;

	ReachElapsedTime = 0.0f;
	bIsReachingToGrab = true;
}

void AGoodbyeCharacter::StopGrab(
	const FInputActionValue& Value
)
{
	bGrabInputHeld = false;

	// Il mouse è stato rilasciato prima che la mano
	// raggiungesse l'oggetto.
	if (bIsReachingToGrab)
	{
		CancelPendingGrab();
		return;
	}

	if (!PhysicsHandle)
	{
		return;
	}

	PhysicsHandle->ReleaseComponent();

	if (GrabbedComponent)
	{
		GrabbedComponent->SetCollisionResponseToChannel(
			ECC_Pawn,
			OriginalPawnCollisionResponse
		);

		GrabbedComponent->WakeAllRigidBodies();
		GrabbedComponent = nullptr;
	}
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


void AGoodbyeCharacter::CompleteGrab()
{
	if (!bGrabInputHeld ||
		!PhysicsHandle ||
		!PendingGrabComponent ||
		!PendingGrabComponent->IsSimulatingPhysics())
	{
		CancelPendingGrab();
		return;
	}

	// Ricostruisce il punto mondiale anche nel caso
	// in cui l'oggetto si sia mosso durante l'animazione.
	const FVector GrabWorldPoint =
		PendingGrabComponent
		->GetComponentTransform()
		.TransformPosition(PendingLocalGrabPoint);

	GrabbedComponent = PendingGrabComponent;
	LocalGrabPoint = PendingLocalGrabPoint;

	OriginalPawnCollisionResponse =
		GrabbedComponent->GetCollisionResponseToChannel(
			ECC_Pawn
		);

	// Evita che il cubo urti continuamente la capsula
	// mentre viene trasportato.
	GrabbedComponent->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Ignore
	);

	GrabbedComponent->WakeAllRigidBodies();

	// Soltanto adesso viene eseguita la presa fisica.
	PhysicsHandle->GrabComponentAtLocation(
		GrabbedComponent,
		NAME_None,
		GrabWorldPoint
	);

	PendingGrabComponent = nullptr;
	PendingLocalGrabPoint = FVector::ZeroVector;
	bIsReachingToGrab = false;
	ReachElapsedTime = 0.0f;
}


void AGoodbyeCharacter::CancelPendingGrab()
{
	PendingGrabComponent = nullptr;
	PendingLocalGrabPoint = FVector::ZeroVector;

	bIsReachingToGrab = false;
	ReachElapsedTime = 0.0f;
}