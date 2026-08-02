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

#include "GoodbyeCharacter.h"

// Costruttore
ACargoTruckPawn::ACargoTruckPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Non serve un Tick personalizzato per ingresso, uscita e rilevamento del Character
	PrimaryActorTick.bCanEverTick = false;


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

// Begin Play
void ACargoTruckPawn::BeginPlay()
{
	Super::BeginPlay();

	NearbyCharacter = nullptr;
	DriverCharacter = nullptr;

	bCanEnterVehicle = false;
	bIsDriving = false;


	// Il guidatore rimane invisibile fino all'ingresso del giocatore nel cargo.
	if (IsValid(DriverMesh))
	{
		DriverMesh->SetVisibility(false,true);
		DriverMesh->SetHiddenInGame(true,true);
	}
}


// Input
void ACargoTruckPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!IsValid(EnhancedInputComponent))
	{
		return;
	}


	if (IsValid(InteractAction))
	{
		EnhancedInputComponent->BindAction(
			InteractAction,
			ETriggerEvent::Started,
			this,
			&ACargoTruckPawn::HandleInteract
		);
	}
}


void ACargoTruckPawn::HandleInteract()
{
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


	// Restituisce il controllo al Character originale.
	PlayerController->Possess(ExitingCharacter);


	// Ripristina lo stato interno del camion.
	bIsDriving = false;
	bCanEnterVehicle = false;

	NearbyCharacter = nullptr;
	DriverCharacter = nullptr;


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Player uscito dal camion %s"),
		*GetName()
	);


	return true;
}