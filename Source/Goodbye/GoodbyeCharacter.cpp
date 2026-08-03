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
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "CargoTruckPawn.h"
#include "Components/SceneComponent.h"


// Costruttore 
AGoodbyeCharacter::AGoodbyeCharacter()
{
	// Il Character deve aggiornare ogni frame la posizione dell'oggetto afferrato
	PrimaryActorTick.bCanEverTick = true;

	// Configura le capsule del Character
	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	// Cra la seconda Skeletal Mesh utilizzata nella visuale in prima persona
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	// Collega la First Person mesh alla mesh principale ereditata da ACharacter
	FirstPersonMesh->SetupAttachment(GetMesh());

	// Mostra la mesh solo al giocatore proprietario
	FirstPersonMesh->SetOnlyOwnerSee(true);

	// Indica ad Unreal che questa mesh viene utilizzata come rappresentazione in prima persona
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;

	// La first person mesh è solo visiva, non ha collisioni
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));


	// Crea il componente camera utilizzata dal giocatore
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));

	// Collega la telecamera al socket "head" della First Person Mesh
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, TEXT("head"));

	// Posiziona e ruota la telecamera rispetto alla testa
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(	FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));

	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = false;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 90.0f;

	FirstPersonCameraComponent->bEnableFirstPersonScale = false;
	FirstPersonCameraComponent->FirstPersonScale = 1.0f;

	// Crea il punto verso cui viene trasportato l'oggetto afferrato
	GrabHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Grab Hold Point"));

	// Il punto segue posizione e rotazione della Camera.
	GrabHoldPoint->SetupAttachment(FirstPersonCameraComponent);

	// Posiziona l'oggetto davanti e leggermente sotto la visuale.
	GrabHoldPoint->SetRelativeLocation(FVector(140.0f, 0.0f, -30.0f));

	// Configura le mesh
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	// Configura il movimento
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;


	// Componente che permette di trascinare oggetti mantenendo attiva la simulazione fisica
	PhysicsHandle =		CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("Physics Handle"));

	// Velocità con cui l'oggetto raggiunge il punto target.
	PhysicsHandle->SetInterpolationSpeed(LightObjectInterpolationSpeed);


}


// Tick
void AGoodbyeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FirstPersonCameraComponent)
	{
		return;
	}

	// Recupera posizione e direzioni della camera nel mondo
	const FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	const FVector CameraForward = FirstPersonCameraComponent->GetForwardVector();
	const FVector CameraRight = FirstPersonCameraComponent->GetRightVector();
	const FVector CameraUp = FirstPersonCameraComponent->GetUpVector();

	

	// Il braccio sta raggiungendo l'oggetto, che rimane ancora fermo 
	if (bIsReachingToGrab)
	{
		if (!bGrabInputHeld ||!IsValid(PendingGrabComponent))
		{
			CancelPendingGrab();
			return;
		}

		// Il punto può cambiare posizione se l'oggetto viene urtato mentre la mano lo sta raggiungendo.
		const FVector PendingGrabWorldPoint =
			PendingGrabComponent
			->GetComponentTransform()
			.TransformPosition(PendingLocalGrabPoint);

		RightHandIKTarget = PendingGrabWorldPoint;

		// Stabilisce il punto verso cui deve orientarsi il gomito
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

		// Dopo una breve fase di raggiugnimento il Physics Handle completa la presa
		if (ReachElapsedTime >= ReachDuration)
		{
			CompleteGrab();
			return;
		}

		return;
	}


	// L'oggetto è stato afferrato e segue la telecamera
	UpdateGrabbedObject();

	const bool bIsHoldingObject = PhysicsHandle && PhysicsHandle->GetGrabbedComponent() && IsValid(GrabbedComponent);

	const float TargetAlpha = bIsHoldingObject ? 1.0f : 0.0f;

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

	// La mano continua a seguire lo stesso punto dell'oggetto durante il trasporto
	RightHandIKTarget = GrabbedComponent->GetComponentTransform().TransformPosition(LocalGrabPoint);

	RightElbowIKTarget =
		CameraLocation
		+ CameraForward * 35.0f
		+ CameraRight * 45.0f
		- CameraUp * 20.0f;
}


// Configurazione degli input
void AGoodbyeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInputComponent)
	{
		UE_LOG(	LogGoodbye,	Error,	TEXT("'%s' Failed to find an Enhanced Input Component! "
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

		if (IsValid(InteractAction))
		{
			EnhancedInputComponent->BindAction(
				InteractAction,
				ETriggerEvent::Started,
				this,
				&AGoodbyeCharacter::HandleInteract
			);
		}
	}
}

// Callback  dell'enhanced input
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


// Inizio del Grab
void AGoodbyeCharacter::StartGrab(const FInputActionValue& /*Value*/)
{
	bGrabInputHeld = true;

	if (!PhysicsHandle || PhysicsHandle->GetGrabbedComponent() || bIsReachingToGrab || !FirstPersonCameraComponent || !GetWorld())
	{
		return;
	}

	// Il Line Trace parte dalla telecamera
	const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();

	// Il Line Trace termina davanti alla telecamera alla distanza configurata da GrabDistance
	const FVector TraceEnd = TraceStart + FirstPersonCameraComponent->GetForwardVector() * GrabDistance;

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// Cerca un oggetto sul canale Visibility
	const bool bHit = GetWorld()->LineTraceSingleByChannel( HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (!bHit)
	{
		return;
	}

	// Recupera l'Actor e il componente colpiti
	AMovableItem* MovableItem = Cast<AMovableItem>(HitResult.GetActor());

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	// Verifica che l'oggetto sia afferrabile e che stia simulando la fisica
	if (!MovableItem || !IsValid(HitComponent) || !HitComponent->IsSimulatingPhysics())
	{
		return;
	}

	// Ora HitComponent esiste ed è valido, quindi possiamo leggere la sua massa
	const float ObjectMassInKg = HitComponent->GetMass();

	// Gli oggetti oltre il limite massimo non possono essere sollevati
	if (ObjectMassInKg > MaxGrabbableMass)
	{
		UE_LOG(
			LogGoodbye,
			Display,
			TEXT(
				"Oggetto troppo pesante: %.1f kg. "
				"Peso massimo sollevabile: %.1f kg."
			),
			ObjectMassInKg,
			MaxGrabbableMass
		);

		return;
	}

	// Salva il componente trovato: il Physics Handle non lo afferra ancora
	PendingGrabComponent = HitComponent;

	// Salva il punto colpito nello spazio locale dell'oggetto.
	PendingLocalGrabPoint = PendingGrabComponent->GetComponentTransform().InverseTransformPosition(HitResult.ImpactPoint);

	// Imposta il punto che la mano deve raggiungere
	RightHandIKTarget = HitResult.ImpactPoint;

	// Avvia la fase di raggiungimento.
	ReachElapsedTime = 0.0f;
	bIsReachingToGrab = true;
}


// Fine grab
void AGoodbyeCharacter::StopGrab(const FInputActionValue& Value)
{
	bGrabInputHeld = false;

	// Il mouse è stato rilasciato prima che la mano raggiungesse l'oggetto.
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

	ResetGrabWeight();

	if (IsValid(GrabbedComponent))
	{
		GrabbedComponent->SetCollisionResponseToChannel(
			ECC_Pawn,
			OriginalPawnCollisionResponse
		);

		GrabbedComponent->WakeAllRigidBodies();
		GrabbedComponent = nullptr;
	}
}


// Oggetto raccolto
void AGoodbyeCharacter::UpdateGrabbedObject()
{
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent() || !IsValid(GrabHoldPoint))
	{
		return;
	}
	
	const FVector TargetLocation = GrabHoldPoint->GetComponentLocation();

	const FRotator TargetRotation = GrabHoldPoint->GetComponentRotation();

	// Aggiorna sia la posizione sia la rotazione
	PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, TargetRotation);
}


// Grab compeltato
void AGoodbyeCharacter::CompleteGrab()
{
	if (!bGrabInputHeld || !PhysicsHandle || !IsValid(GrabHoldPoint) || !IsValid(PendingGrabComponent) || !PendingGrabComponent->IsSimulatingPhysics())
	{
		CancelPendingGrab();
		return;
	}

	// Ricostruisce il punto mondiale anche nel caso in cui l'oggetto si sia mosso durante l'animazione.
	const FVector GrabWorldPoint = PendingGrabComponent->GetComponentTransform().TransformPosition(PendingLocalGrabPoint);

	GrabbedComponent = PendingGrabComponent;
	LocalGrabPoint = PendingLocalGrabPoint;

	OriginalPawnCollisionResponse = GrabbedComponent->GetCollisionResponseToChannel(ECC_Pawn);

	// Evita che il cubo urti continuamente la capsula mentre viene trasportato.
	GrabbedComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	GrabbedComponent->WakeAllRigidBodies();

	// Elimina l'eventuale rotazione che l'oggetto ha dopo essere stato afferrato
	GrabbedComponent->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);

	// Il Grab Hold Point assume l'orientamento corrente dell'oggetto.
	// Essendo collegato alla Camera, manterrà questo orientamento relativo mentre il giocatore ruota la visuale.
	GrabHoldPoint->SetWorldRotation(GrabbedComponent->GetComponentRotation());

	// Configura il comportamento del Physics Handle usando la massa reale del componente
	ApplyGrabWeight(GrabbedComponent->GetMass());

	// Soltanto adesso viene eseguita la presa fisica.
	PhysicsHandle->GrabComponentAtLocationWithRotation(GrabbedComponent, NAME_None, GrabWorldPoint, GrabHoldPoint->GetComponentRotation());

	PendingGrabComponent = nullptr;
	PendingLocalGrabPoint = FVector::ZeroVector;
	bIsReachingToGrab = false;
	ReachElapsedTime = 0.0f;

}

// Annullamento del grab
void AGoodbyeCharacter::CancelPendingGrab()
{
	PendingGrabComponent = nullptr;
	PendingLocalGrabPoint = FVector::ZeroVector;

	bIsReachingToGrab = false;
	ReachElapsedTime = 0.0f;
}


// Applica il peso dell'oggetto
void AGoodbyeCharacter::ApplyGrabWeight(float ObjectMassInKg)
{
	if (!PhysicsHandle)
	{
		return;
	}

	GrabbedMassInKg = ObjectMassInKg;

	
	// Converte la massa in un valore tra 0 e 1
	// 0 = molto leggero
	// 1 = pesante
	const float WeightRatio = FMath::Clamp(ObjectMassInKg / MaxGrabbableMass, 0.0f, 1.0f);

	
	// Gli oggetti leggeri utilizzano una velocità elevata.
	// Gli oggetti pesanti utilizzano una velocità minore.
	
	const float InterpolationSpeed = FMath::Lerp(LightObjectInterpolationSpeed, HeavyObjectInterpolationSpeed, WeightRatio);

	PhysicsHandle->SetInterpolationSpeed(InterpolationSpeed);

	UE_LOG(LogGoodbye, Display,	TEXT("Grab: massa %.1f kg, interpolation speed %.2f"), ObjectMassInKg, InterpolationSpeed);
}



void AGoodbyeCharacter::ResetGrabWeight()
{
	GrabbedMassInKg = 0.0f;

	if (PhysicsHandle)
	{
		PhysicsHandle->SetInterpolationSpeed(LightObjectInterpolationSpeed);
	}
}



// Funzioni del Cargo
void AGoodbyeCharacter::SetNearbyCargoTruck(ACargoTruckPawn* CargoTruck)
{
	if (!IsValid(CargoTruck))
	{
		return;
	}

	NearbyCargoTruck = CargoTruck;
}


void AGoodbyeCharacter::ClearNearbyCargoTruck(ACargoTruckPawn* CargoTruck)
{
	if (NearbyCargoTruck != CargoTruck)
	{
		return;
	}

	NearbyCargoTruck = nullptr;
}


void AGoodbyeCharacter::HandleInteract()
{
	if (!IsValid(NearbyCargoTruck))
	{
		return;
	}

	NearbyCargoTruck->EnterVehicle(this);
}





// Azioni di movimento

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
