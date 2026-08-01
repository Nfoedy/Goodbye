#include "CargoTruckPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"

#include "GoodbyeCharacter.h"


// Costruttore
ACargoTruckPawn::ACargoTruckPawn()
{
	// Il Tick non serve per ingresso, uscita e rilevamento
	PrimaryActorTick.bCanEverTick = false;


	// Truck mesh
	TruckMesh =	CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TruckMesh"));
	SetRootComponent(TruckMesh);
	TruckMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TruckMesh->SetCollisionProfileName(TEXT("BlockAll"));


	// Cargo Zone
	CargoZoneChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("CargoZoneChild"));
	CargoZoneChild->SetupAttachment(TruckMesh);

	// Zona di interact
	DriverInteractZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DriverInteractZone"));
	DriverInteractZone->SetupAttachment(TruckMesh);
	DriverInteractZone->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	DriverInteractZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DriverInteractZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	DriverInteractZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DriverInteractZone->SetGenerateOverlapEvents(true);
	DriverInteractZone->OnComponentBeginOverlap.AddDynamic(this, &ACargoTruckPawn::HandleDriverZoneBeginOverlap);
	DriverInteractZone->OnComponentEndOverlap.AddDynamic(this, &ACargoTruckPawn::HandleDriverZoneEndOverlap);


	// Punto di uscita
	DriverExitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DriverExitPoint"));
	DriverExitPoint->SetupAttachment(TruckMesh);


	// Driver
	DriverSeatPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DriverSeatPoint"));
	DriverSeatPoint->SetupAttachment(TruckMesh);

	DriverMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DriverMesh"));
	DriverMesh->SetupAttachment(DriverSeatPoint);
	DriverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DriverMesh->SetGenerateOverlapEvents(false);
	DriverMesh->SetSimulatePhysics(false);


	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(TruckMesh);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bDoCollisionTest = true;
	SpringArm->bUsePawnControlRotation = false;

	// Camera
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


	// Il guidatore deve essere invisibile fino all'ingresso nel camion
	if (IsValid(DriverMesh))
	{
		DriverMesh->SetVisibility(false, true);

		DriverMesh->SetHiddenInGame(true, true);
	}
}

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
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACargoTruckPawn::HandleInteract);
	}
}


// Character entrato nella zona
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


// Character uscito dalla zona
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


	// Disabilitando la collisione del Character durante l'ingresso potrebbe generare un EndOverlap
	// In quel caso non dobbiamo cancellare il rif al DriverCharacter
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


// Ingresso nel camion
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

	// Può entrare soltanto il Character realmente presente 
	if (RequestingCharacter != NearbyCharacter)
	{
		return false;
	}


	APlayerController* PlayerController = Cast<APlayerController>(RequestingCharacter->GetController());

	if (!IsValid(PlayerController))
	{
		return false;
	}


	// Conserviamo il Character originale
	DriverCharacter = RequestingCharacter;


	// imposto lo stato prima di disabilitare la collisione del Character
	bIsDriving = true;
	bCanEnterVehicle = false;


	// Il Character non deve più considerare il cargo come una normale interazione vicina
	DriverCharacter->ClearNearbyCargoTruck(this);


	// Ferma il movimento del Character.
	if (UCharacterMovementComponent* CharacterMovement = DriverCharacter->GetCharacterMovement())
	{
		CharacterMovement->StopMovementImmediately();
		CharacterMovement->DisableMovement();
	}


	// Il Character rimane nel mondo ma diventa invisibile e senza collisione
	DriverCharacter->SetActorHiddenInGame(true);
	DriverCharacter->SetActorEnableCollision(false);


	// Mostra la mesh seduta nella cabina
	if (IsValid(DriverMesh))
	{
		DriverMesh->SetVisibility(true, true);
		DriverMesh->SetHiddenInGame(false, true);
	}


	// Il PlayerController passa dal Character al camion
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


// Interazione per uscire dal cargo
void ACargoTruckPawn::HandleInteract()
{
	if (!bIsDriving)
	{
		return;
	}

	ExitVehicle();
}


// Uscita dal veicolo
bool ACargoTruckPawn::ExitVehicle()
{
	// Il giocatore deve trovarsi realmente nel camion.
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


	// Posiziona il Character accanto alla portiera
	// La collisione è ancora disabilitata, quindi non rischia di bloccarsi durante lo spostamento
	FRotator ExitRotation =	DriverExitPoint->GetComponentRotation();

	ExitRotation.Pitch = 0.0f;
	ExitRotation.Roll = 0.0f;

	DriverCharacter->SetActorLocationAndRotation(
		DriverExitPoint->GetComponentLocation(),
		ExitRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);


	// Mostra nuovamente il Character reale.
	DriverCharacter->SetActorHiddenInGame(false);


	// Riattiva la collisione.
	DriverCharacter->SetActorEnableCollision(true);


	// Riattiva il movimento del Character.
	if (UCharacterMovementComponent* CharacterMovement = DriverCharacter->GetCharacterMovement())
	{
		CharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	}


	// Nasconde la rappresentazione seduta.
	if (IsValid(DriverMesh))
	{
		DriverMesh->SetVisibility(false, true);
		DriverMesh->SetHiddenInGame(true, true);
	}


	// Il PlayerController torna a controllare il Character originale
	PlayerController->Possess(DriverCharacter);


	// Ripristina lo stato del camion.
	bIsDriving = false;
	bCanEnterVehicle = false;

	NearbyCharacter = nullptr;


	UE_LOG(
		LogTemp,
		Display,
		TEXT("Player uscito dal camion %s"),
		*GetName()
	);


	return true;
}