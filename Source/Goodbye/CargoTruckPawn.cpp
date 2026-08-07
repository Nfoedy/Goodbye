#include "CargoTruckPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "CargoTruckFrontWheel.h"
#include "CargoTruckRearWheel.h"
#include "Curves/RichCurve.h"
#include "InputActionValue.h"

#include "GoodbyeCharacter.h"

// Costruttore
ACargoTruckPawn::ACargoTruckPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Il Tick viene usato solamente durante la guida automatica finale
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// AWheeledVehiclePawn possiede già la Skeletal Mesh principale.
	USkeletalMeshComponent* VehicleMesh = GetMesh();

	if (IsValid(VehicleMesh))
	{
		VehicleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
	}

	// Configuarazione delle ruote 
	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (IsValid(VehicleMovement))
	{
		VehicleMovement->WheelSetups.SetNum(4);


		// Ruota anteriore sinistra
		VehicleMovement->WheelSetups[0].WheelClass = UCargoTruckFrontWheel::StaticClass();
		VehicleMovement->WheelSetups[0].BoneName = FName(TEXT("wheelFL"));
		VehicleMovement->WheelSetups[0].AdditionalOffset = FVector::ZeroVector;


		// Ruota anteriore destra
		VehicleMovement->WheelSetups[1].WheelClass = UCargoTruckFrontWheel::StaticClass();
		VehicleMovement->WheelSetups[1].BoneName = FName(TEXT("wheelFR"));
		VehicleMovement->WheelSetups[1].AdditionalOffset = FVector::ZeroVector;


		// Ruota posteriore sinistra
		VehicleMovement->WheelSetups[2].WheelClass = UCargoTruckRearWheel::StaticClass();
		VehicleMovement->WheelSetups[2].BoneName = FName(TEXT("wheelRL"));
		VehicleMovement->WheelSetups[2].AdditionalOffset = FVector::ZeroVector;


		// Ruota posteriore destra
		VehicleMovement->WheelSetups[3].WheelClass = UCargoTruckRearWheel::StaticClass();
		VehicleMovement->WheelSetups[3].BoneName = FName(TEXT("wheelRR"));
		VehicleMovement->WheelSetups[3].AdditionalOffset = FVector::ZeroVector;


		// Simulazione meccanica
		VehicleMovement->bMechanicalSimEnabled = true;


		// Motore
		VehicleMovement->EngineSetup.MaxTorque = BaseMaxEngineTorque;
		VehicleMovement->EngineSetup.MaxRPM = 4500.0f;
		VehicleMovement->EngineSetup.EngineIdleRPM = 800.0f;
		VehicleMovement->EngineSetup.EngineBrakeEffect = 0.1f;


		// Curva della coppia del motore
		if (FRichCurve* TorqueCurve = VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve())
		{
			TorqueCurve->Reset();

			TorqueCurve->AddKey(0.0f, 0.40f);
			TorqueCurve->AddKey(800.0f, 0.60f);
			TorqueCurve->AddKey(1500.0f, 0.85f);
			TorqueCurve->AddKey(2500.0f, 1.00f);
			TorqueCurve->AddKey(3500.0f, 0.85f);
			TorqueCurve->AddKey(4500.0f, 0.55f);
		}


		// Differenziale
		// Trazione posteriore
		VehicleMovement->DifferentialSetup.DifferentialType = EVehicleDifferential::RearWheelDrive;


		// Marcia unica
		// Gestione automatica di marcia avanti e retromarcia
		VehicleMovement->TransmissionSetup.bUseAutomaticGears = true;
		VehicleMovement->TransmissionSetup.bUseAutoReverse = true;

		// Tempo necessario per passare tra avanti e retromarcia
		VehicleMovement->TransmissionSetup.GearChangeTime = 0.15f;

		// Rapporto finale
		VehicleMovement->TransmissionSetup.FinalRatio = 4.10f;

		// Perdita meccanica della trasmissione
		VehicleMovement->TransmissionSetup.TransmissionEfficiency = 0.90f;


		// Una sola marcia avanti
		VehicleMovement->TransmissionSetup.ForwardGearRatios =
		{
			2.80f
		};


		// Una sola retromarcia
		VehicleMovement->TransmissionSetup.ReverseGearRatios =
		{
			2.80f
		};

		// Massa del cargo in kg, Una massa elevata rende trascurabili le spinte provenienti dal Character o dagli oggetti piccoli.
		VehicleMovement->Mass = 2500.0f;
	}

	// CargoZone
	CargoZoneChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("CargoZoneChild"));
	CargoZoneChild->SetupAttachment(GetMesh());

	// Zona di interazione
	DriverInteractZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DriverInteractZone"));
	DriverInteractZone->SetupAttachment(GetMesh());
	DriverInteractZone->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	DriverInteractZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DriverInteractZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	DriverInteractZone->SetCollisionResponseToChannel(ECC_Pawn,	ECR_Overlap);
	DriverInteractZone->SetGenerateOverlapEvents(true);
	DriverInteractZone->OnComponentBeginOverlap.AddDynamic(this, &ACargoTruckPawn::HandleDriverZoneBeginOverlap);
	DriverInteractZone->OnComponentEndOverlap.AddDynamic(this, &ACargoTruckPawn::HandleDriverZoneEndOverlap);


	// Driver exit point
	DriverExitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DriverExitPoint"));
	DriverExitPoint->SetupAttachment(GetMesh());
	
	// Driver Mesh
	DriverSeatPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DriverSeatPoint"));
	DriverSeatPoint->SetupAttachment(GetMesh());

	DriverMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DriverMesh"));
	DriverMesh->SetupAttachment(DriverSeatPoint);
	DriverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DriverMesh->SetGenerateOverlapEvents(false);
	DriverMesh->SetSimulatePhysics(false);

	// Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bDoCollisionTest = true;
	SpringArm->bUsePawnControlRotation = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}


// Tick
void ACargoTruckPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bVictoryAutoDrive)
	{
		return;
	}

	UpdateVictoryAutoDrive(DeltaTime);
}


// Attiva/ disattiva della simulazione fisica
void ACargoTruckPawn::SetVehicleSimulationEnabled(bool bEnabled)
{
	USkeletalMeshComponent* VehicleMesh = GetMesh();

	if (!IsValid(VehicleMesh))
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (bEnabled)
	{
		// Riattiva la simulazione fisica del telaio.
		VehicleMesh->SetSimulatePhysics(true);

		if (IsValid(VehicleMovement))
		{
			VehicleMovement->SetParked(false);
			VehicleMovement->SetSleeping(false);
		}

		VehicleMesh->WakeAllRigidBodies();

		return;
	}


	// Prima di parcheggiare vengono azzerati gli input eventualmente rimasti attivi
	if (IsValid(VehicleMovement))
	{
		VehicleMovement->SetThrottleInput(0.0f);
		VehicleMovement->SetBrakeInput(0.0f);
		VehicleMovement->SetSteeringInput(0.0f);

		VehicleMovement->SetParked(true);
		VehicleMovement->SetSleeping(true);
	}


	// Quando questa funzione viene chiamata dal timer il cargo è già quasi fermo
	VehicleMesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector, false);
	VehicleMesh->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector, false);
	VehicleMesh->PutAllRigidBodiesToSleep();
	VehicleMesh->SetSimulatePhysics(false);
}

// Auto Drive dopo la vittoria
void ACargoTruckPawn::StartVictoryAutoDrive(const FVector& InDirection)
{
	// Ignora l'altezza: il cargo deve allinearsi solamente sul piano orizzontale
	FVector FlatDirection(InDirection.X, InDirection.Y,	0.0f);

	FlatDirection = FlatDirection.GetSafeNormal();


	// Usa la direzione attuale del camion come fallback.
	if (FlatDirection.IsNearlyZero())
	{
		FlatDirection = GetActorForwardVector().GetSafeNormal2D();
	}


	VictoryDriveDirection = FlatDirection;

	bVictoryAutoDrive = true;
	CurrentVictorySteeringInput = 0.0f;


	// Evita che il sistema di parcheggio possa bloccare il cargo.
	StopAutoParkCheck();

	// Garantisce che la simulazione Chaos sia attiva.
	SetVehicleSimulationEnabled(true);


	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (IsValid(VehicleMovement))
	{
		VehicleMovement->SetBrakeInput(0.0f);
		VehicleMovement->SetSteeringInput(0.0f);
		VehicleMovement->SetThrottleInput(VictoryThrottleInput);
	}


	// Il Tick viene attivato solamente da questo momento.
	SetActorTickEnabled(true);


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Guida automatica di vittoria avviata. "
			"Direzione: %s"
		),
		*VictoryDriveDirection.ToString()
	);
}

// Aggiorna l'Auto Drive
void ACargoTruckPawn::UpdateVictoryAutoDrive(float DeltaTime)
{
	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!IsValid(VehicleMovement))
	{
		return;
	}


	// Rotazione attuale del cargo
	const float CurrentYaw = GetActorRotation().Yaw;


	// Rotazione indicata dalla freccia della FinishZone
	const float TargetYaw = VictoryDriveDirection.Rotation().Yaw;


	// Differenza più breve tra i due angoli
	const float YawDifference =	FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw);


	/*
	 * Esempio:
	 *
	 * differenza  0°  → sterzo 0
	 * differenza 17°  → circa mezzo sterzo
	 * differenza 35°  → sterzo completo
	 */
	const float DesiredSteeringInput = FMath::Clamp(YawDifference / VictoryFullSteeringAngle, -1.0f, 1.0f);


	// Evita cambi bruschi dello sterzo.
	CurrentVictorySteeringInput = 
		FMath::FInterpTo(
			CurrentVictorySteeringInput,
			DesiredSteeringInput,
			DeltaTime,
			VictorySteeringInterpolationSpeed
		);


	VehicleMovement->SetBrakeInput(0.0f);

	VehicleMovement->SetThrottleInput(VictoryThrottleInput);

	VehicleMovement->SetSteeringInput(CurrentVictorySteeringInput);
}



// Begin Play
void ACargoTruckPawn::BeginPlay()
{
	Super::BeginPlay();

	AttachComponentsToVehicleBody();

	// Il cargo parte parcheggiato e non può essere spinto
	SetVehicleSimulationEnabled(false);

	NearbyCharacter = nullptr;
	DriverCharacter = nullptr;

	bCanEnterVehicle = false;
	bIsDriving = false;


	// Il guidatore rimane invisibile fino all'ingresso del giocatore nel cargo.
	if (IsValid(DriverMesh))
	{
		DriverMesh->SetVisibility(false, true);
		DriverMesh->SetHiddenInGame(true, true);
	}

	// Applica i valori iniziali della configurazione del motore.
	RefreshEnginePower();

}


// Aggiorna la coppia del motore utilizzando il moltiplicatore corrente.
void ACargoTruckPawn::RefreshEnginePower()
{
	UChaosWheeledVehicleMovementComponent* VehicleMovement =
		Cast<UChaosWheeledVehicleMovementComponent>(
			GetVehicleMovementComponent()
		);

	if (!IsValid(VehicleMovement))
	{
		return;
	}

	const float UpdatedMaxTorque =
		BaseMaxEngineTorque * EnginePowerMultiplier;

	// Applica la nuova coppia massima al motore Chaos.
	VehicleMovement->SetMaxEngineTorque(UpdatedMaxTorque);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Cargo Power | Multiplier: %.2f | Max Torque: %.2f Nm"
		),
		EnginePowerMultiplier,
		UpdatedMaxTorque
	);
}



// Input
void ACargoTruckPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!IsValid(EnhancedInputComponent)) return;

	// Entrata e uscita dal camion.
	if (IsValid(InteractAction))
	{
		EnhancedInputComponent->BindAction(
			InteractAction,
			ETriggerEvent::Started,
			this,
			&ACargoTruckPawn::HandleInteract
		);
	}


	// Accelerazione.
	if (IsValid(ThrottleAction))
	{
		EnhancedInputComponent->BindAction(
			ThrottleAction,
			ETriggerEvent::Triggered,
			this,
			&ACargoTruckPawn::HandleThrottle
		);

		EnhancedInputComponent->BindAction(
			ThrottleAction,
			ETriggerEvent::Completed,
			this,
			&ACargoTruckPawn::StopThrottle
		);

		EnhancedInputComponent->BindAction(
			ThrottleAction,
			ETriggerEvent::Canceled,
			this,
			&ACargoTruckPawn::StopThrottle
		);
	}


	// Freno.
	if (IsValid(BrakeAction))
	{
		EnhancedInputComponent->BindAction(
			BrakeAction,
			ETriggerEvent::Triggered,
			this,
			&ACargoTruckPawn::HandleBrake
		);

		EnhancedInputComponent->BindAction(
			BrakeAction,
			ETriggerEvent::Completed,
			this,
			&ACargoTruckPawn::StopBrake
		);

		EnhancedInputComponent->BindAction(
			BrakeAction,
			ETriggerEvent::Canceled,
			this,
			&ACargoTruckPawn::StopBrake
		);
	}


	// Sterzo.
	if (IsValid(SteeringAction))
	{
		EnhancedInputComponent->BindAction(
			SteeringAction,
			ETriggerEvent::Triggered,
			this,
			&ACargoTruckPawn::HandleSteering
		);

		EnhancedInputComponent->BindAction(
			SteeringAction,
			ETriggerEvent::Completed,
			this,
			&ACargoTruckPawn::StopSteering
		);

		EnhancedInputComponent->BindAction(
			SteeringAction,
			ETriggerEvent::Canceled,
			this,
			&ACargoTruckPawn::StopSteering
		);
	}
}


void ACargoTruckPawn::HandleInteract()
{
	if (bVictoryAutoDrive)
	{
		return;
	}

	if (!bIsDriving)
	{
		return;
	}

	ExitVehicle();
}


// Char entrato nella driver zone
void ACargoTruckPawn::HandleDriverZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	AGoodbyeCharacter* Character = Cast<AGoodbyeCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}

	if (bIsDriving)
	{
		return;
	}


	NearbyCharacter = Character;
	bCanEnterVehicle = true;

	Character->SetNearbyCargoTruck(this);


	UE_LOG(
		LogTemp,
		Display,
		TEXT("%s vicino al camion %s"),
		*GetNameSafe(Character),
		*GetName()
	);
}


// Char uscito dalla driver zone
void ACargoTruckPawn::HandleDriverZoneEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex
)
{
	AGoodbyeCharacter* Character = Cast<AGoodbyeCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}


	// Disabilitando la collisione del Char durante l'ingresso potrebbe essere generato un EndOverlap
	// Se il giocatore sta già guidando, non devo cancellare i riferimenti
	if (bIsDriving)
	{
		return;
	}


	if (Character != NearbyCharacter)
	{
		return;
	}


	Character->ClearNearbyCargoTruck(this);

	NearbyCharacter = nullptr;
	bCanEnterVehicle = false;


	UE_LOG(
		LogTemp,
		Display,
		TEXT("%s si e allontanato dal camion %s"),
		*GetNameSafe(Character),
		*GetName()
	);
}


// Ingresso nel cargo
bool ACargoTruckPawn::EnterVehicle(AGoodbyeCharacter* RequestingCharacter)
{
	if (bIsDriving)
	{
		return false;
	}


	if (!bCanEnterVehicle)
	{
		return false;
	}


	if (!IsValid(RequestingCharacter))
	{
		return false;
	}


	// Può entrare soltanto il Character realmente presente nella zona di interazione.
	if (RequestingCharacter != NearbyCharacter)
	{
		return false;
	}


	APlayerController* PlayerController = Cast<APlayerController>(RequestingCharacter->GetController());

	if (!IsValid(PlayerController))
	{
		return false;
	}


	// Conserva il Character originale
	DriverCharacter = RequestingCharacter;



	// Lo stato viene impostato prima di disabilitare la collisione del Character
	bIsDriving = true;
	bCanEnterVehicle = false;

	// Interrompe un eventuale controllo di parcheggi.
	StopAutoParkCheck();

	// Riattiva la simulazione Chaos
	SetVehicleSimulationEnabled(true);


	// Il Character non deve più considerare il cargo come una normale interazione vicina.
	DriverCharacter->ClearNearbyCargoTruck(this);


	// Ferma e disabilita il movimento del Character
	if (UCharacterMovementComponent* CharacterMovement = DriverCharacter->GetCharacterMovement())
	{
		CharacterMovement->StopMovementImmediately();
		CharacterMovement->DisableMovement();
	}


	// Il Character rimane nel mondo ma diventa invisibile e privo di collisione.
	DriverCharacter->SetActorHiddenInGame(true);
	DriverCharacter->SetActorEnableCollision(false);


	// Mostra la rappresentazione seduta nella cabina.
	if (IsValid(DriverMesh))
	{
		DriverMesh->SetVisibility(true, true);
		DriverMesh->SetHiddenInGame(false, true);
	}


	// Il PlayerController passa dal Character al cargo
	PlayerController->Possess(this);

	// Disattiva il mirino durante la guida
	DriverCharacter->SetAimDotVisibility(false);


	NearbyCharacter = nullptr;


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Player entrato nel camion %s"),
		*GetName()
	);


	return true;
}


// Uscita dal cargo
bool ACargoTruckPawn::ExitVehicle()
{
	if (!bIsDriving)
	{
		return false;
	}


	if (!IsValid(DriverCharacter))
	{
		return false;
	}


	if (!IsValid(DriverExitPoint))
	{
		return false;
	}


	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!IsValid(PlayerController))
	{
		return false;
	}


	// Conserva temporaneamente il riferimento, prima di azzerare DriverCharacter.
	AGoodbyeCharacter* ExitingCharacter = DriverCharacter;

	// Utilizziamo soltanto lo Yaw del punto di uscita, così il Character non viene inclinato insieme al camion.
	FRotator ExitRotation = DriverExitPoint->GetComponentRotation();

	ExitRotation.Pitch = 0.0f;
	ExitRotation.Roll = 0.0f;


	// La collisione del Char è ancora disabilitata, quindi non può essere spostato senza bloccarsi contro il camion
	ExitingCharacter->SetActorLocationAndRotation(
		DriverExitPoint->GetComponentLocation(),
		ExitRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);


	// Mostra nuovamente il Character.
	ExitingCharacter->SetActorHiddenInGame(false);


	// Riattiva la collisione.
	ExitingCharacter->SetActorEnableCollision(true);


	// Riattiva il movimento.
	if (UCharacterMovementComponent* CharacterMovement = ExitingCharacter->GetCharacterMovement())
	{
		CharacterMovement->SetMovementMode(MOVE_Walking);
	}


	// Nasconde la rappresentazione seduta.
	if (IsValid(DriverMesh))
	{
		DriverMesh->SetVisibility(false, true);
		DriverMesh->SetHiddenInGame(true, true);
	}


	// Azzera tutti gli input prima di lasciare il cargo
	if (UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		VehicleMovement->SetThrottleInput(0.0f);
		VehicleMovement->SetBrakeInput(0.0f);
		VehicleMovement->SetSteeringInput(0.0f);
	}


	// Restituisce il controllo al Character originale.
	PlayerController->Possess(ExitingCharacter);

	// Attiva il mirino dell'HUD
	ExitingCharacter->SetAimDotVisibility(true);


	// Ripristina lo stato interno del camion.
	bIsDriving = false;
	bCanEnterVehicle = false;

	NearbyCharacter = nullptr;
	DriverCharacter = nullptr;

	// Il cargo continua liberamente per inerzia. Verrà parcheggiato soltanto quando sarà fermo.
	StartAutoParkCheck();

	return true;
}


// Accellerazione 
void ACargoTruckPawn::HandleThrottle(const FInputActionValue& Value)
{
	if (bVictoryAutoDrive)
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!IsValid(VehicleMovement))
	{
		return;
	}

	const float ThrottleValue = FMath::Clamp(Value.Get<float>(), 0.0f,	1.0f);

	VehicleMovement->SetThrottleInput(ThrottleValue);
}

// Decellerazione quando non viene premuto l'accelleratore
void ACargoTruckPawn::StopThrottle()
{
	if (bVictoryAutoDrive)
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!IsValid(VehicleMovement))
	{
		return;
	}

	VehicleMovement->SetThrottleInput(0.0f);
}

// Freno
void ACargoTruckPawn::HandleBrake(const FInputActionValue& Value)
{
	if (bVictoryAutoDrive)
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!IsValid(VehicleMovement))
	{
		return;
	}

	const float BrakeValue = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f);

	VehicleMovement->SetBrakeInput(BrakeValue);
}

// Rilascio del freno
void ACargoTruckPawn::StopBrake()
{
	if (bVictoryAutoDrive)
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!IsValid(VehicleMovement))
	{
		return;
	}

	VehicleMovement->SetBrakeInput(0.0f);
}

// Sterzo
void ACargoTruckPawn::HandleSteering(const FInputActionValue& Value)
{
	if (bVictoryAutoDrive)
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!IsValid(VehicleMovement))
	{
		return;
	}

	const float SteeringValue = FMath::Clamp(Value.Get<float>(), -1.0f,	1.0f);

	VehicleMovement->SetSteeringInput(SteeringValue);
}


void ACargoTruckPawn::StopSteering()
{
	if (bVictoryAutoDrive)
	{
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

	if (!IsValid(VehicleMovement))
	{
		return;
	}

	VehicleMovement->SetSteeringInput(0.0f);
}

// Attacca i componenti al corpo del Cargo
void ACargoTruckPawn::AttachComponentsToVehicleBody()
{
	USkeletalMeshComponent* VehicleMesh = GetMesh();

	if (!IsValid(VehicleMesh))
	{
		return;
	}


	const FName VehicleBodyBoneName(TEXT("carBody"));

	if (VehicleMesh->GetBoneIndex(VehicleBodyBoneName) == INDEX_NONE)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Impossibile trovare il bone %s nella mesh del camion"
			),
			*VehicleBodyBoneName.ToString()
		);

		return;
	}


	// Mantiene le posizioni già configurate nel BP ma cambia riferimento dal Mesh Componente al bone fisico
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld,	EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);


	if (IsValid(CargoZoneChild))
	{
		CargoZoneChild->AttachToComponent(
			VehicleMesh,
			AttachmentRules,
			VehicleBodyBoneName
		);
	}


	if (IsValid(DriverInteractZone))
	{
		DriverInteractZone->AttachToComponent(
			VehicleMesh,
			AttachmentRules,
			VehicleBodyBoneName
		);
	}


	if (IsValid(DriverExitPoint))
	{
		DriverExitPoint->AttachToComponent(
			VehicleMesh,
			AttachmentRules,
			VehicleBodyBoneName
		);
	}


	if (IsValid(DriverSeatPoint))
	{
		DriverSeatPoint->AttachToComponent(
			VehicleMesh,
			AttachmentRules,
			VehicleBodyBoneName
		);
	}


	if (IsValid(SpringArm))
	{
		SpringArm->AttachToComponent(
			VehicleMesh,
			AttachmentRules,
			VehicleBodyBoneName
		);
	}


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Componenti del camion collegati al bone %s"
		),
		*VehicleBodyBoneName.ToString()
	);
}


// Implementa il timer per il check
void ACargoTruckPawn::StartAutoParkCheck()
{
	// Il cargo non deve essere parcheggiato automaticamente mentre è guidato.
	if (bIsDriving)
	{
		return;
	}

	USkeletalMeshComponent* VehicleMesh = GetMesh();

	if (!IsValid(VehicleMesh))
	{
		return;
	}


	// Se la simulazione è già disattivata, il cargo è già parcheggiato.
	if (!VehicleMesh->IsSimulatingPhysics())
	{
		return;
	}


	StopAutoParkCheck();

	AutoParkStableElapsed = 0.0f;


	GetWorldTimerManager().SetTimer(
		AutoParkTimerHandle,
		this,
		&ACargoTruckPawn::CheckAutoPark,
		AutoParkCheckInterval,
		true,
		AutoParkCheckInterval
	);
}


void ACargoTruckPawn::StopAutoParkCheck()
{
	GetWorldTimerManager().ClearTimer(AutoParkTimerHandle);

	AutoParkStableElapsed = 0.0f;
}


// Controlla se il cargo è fermo
void ACargoTruckPawn::CheckAutoPark()
{
	// Se nel frattempo il Player è rientrato interrompiamo il controllo.
	if (bIsDriving)
	{
		StopAutoParkCheck();
		return;
	}


	USkeletalMeshComponent* VehicleMesh = GetMesh();

	if (!IsValid(VehicleMesh))
	{
		StopAutoParkCheck();
		return;
	}


	if (!VehicleMesh->IsSimulatingPhysics())
	{
		StopAutoParkCheck();
		return;
	}


	const FName VehicleBodyBoneName(TEXT("carBody"));


	const FVector LinearVelocity = VehicleMesh->GetPhysicsLinearVelocity(VehicleBodyBoneName);

	const FVector AngularVelocity = VehicleMesh->GetPhysicsAngularVelocityInDegrees(VehicleBodyBoneName);

	const float LinearSpeed = LinearVelocity.Size();

	const float AngularSpeed = AngularVelocity.Size();


	const bool bIsAlmostStopped = LinearSpeed <= AutoParkLinearSpeedThreshold && AngularSpeed <= AutoParkAngularSpeedThreshold;


	if (bIsAlmostStopped)
	{
		AutoParkStableElapsed += AutoParkCheckInterval;
	}
	else
	{
		// Il cargo si è mosso nuovamente: ricominciamo il conteggio
		AutoParkStableElapsed = 0.0f;
	}


	if (AutoParkStableElapsed <	AutoParkStableDuration)
	{
		return;
	}


	StopAutoParkCheck();
	SetVehicleSimulationEnabled(false);


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Camion parcheggiato automaticamente")
	);
}