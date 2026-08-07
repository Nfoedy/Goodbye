#include "CargoSpeedPowerUp.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

#include "CargoTruckPawn.h"
#include "GoodbyeReflectionFunctionLibrary.h"


// Costruttore.
ACargoSpeedPowerUp::ACargoSpeedPowerUp()
{
	PrimaryActorTick.bCanEverTick = true;


	// Root.
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);


	// Mesh della moneta.
	CoinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoinMesh"));
	CoinMesh->SetupAttachment(SceneRoot);

	CoinMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoinMesh->SetGenerateOverlapEvents(false);


	// Trigger utilizzato per rilevare il camion.
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);

	TriggerBox->SetBoxExtent(
		FVector(
			100.0f,
			100.0f,
			120.0f
		)
	);

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}


// Inizializza il PowerUp e registra l'evento di overlap.
void ACargoSpeedPowerUp::BeginPlay()
{
	Super::BeginPlay();


	if (IsValid(CoinMesh))
	{
		InitialCoinRelativeLocation =
			CoinMesh->GetRelativeLocation();
	}


	if (IsValid(TriggerBox))
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&ACargoSpeedPowerUp::HandleBeginOverlap
		);
	}
}


// Rimuove il collegamento all'evento di overlap.
void ACargoSpeedPowerUp::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	if (IsValid(TriggerBox))
	{
		TriggerBox->OnComponentBeginOverlap.RemoveAll(this);
	}


	Super::EndPlay(EndPlayReason);
}


// Aggiorna la rotazione e la fluttuazione della moneta.
void ACargoSpeedPowerUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (!IsValid(CoinMesh))
	{
		return;
	}


	// Rotazione continua della moneta.
	CoinMesh->AddLocalRotation(
		FRotator(
			0.0f,
			RotationSpeed * DeltaTime,
			0.0f
		)
	);


	// Movimento verticale sinusoidale.
	FloatingElapsedTime += DeltaTime;


	const float VerticalOffset =
		FMath::Sin(
			FloatingElapsedTime * FloatingSpeed
		) * FloatingAmplitude;


	CoinMesh->SetRelativeLocation(
		InitialCoinRelativeLocation +
		FVector(
			0.0f,
			0.0f,
			VerticalOffset
		)
	);
}


// Gestisce l'ingresso di un Actor nella zona del PowerUp.
void ACargoSpeedPowerUp::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	ACargoTruckPawn* CargoTruck =
		Cast<ACargoTruckPawn>(OtherActor);


	if (!IsValid(CargoTruck))
	{
		return;
	}


	// Il PowerUp può essere raccolto soltanto mentre
	// il camion è realmente guidato dal giocatore.
	if (!CargoTruck->IsDriving())
	{
		return;
	}


	// Cerca la proprietà indicata tramite Reflection
	// e ne moltiplica il valore.
	const bool bPowerUpApplied =
		UGoodbyeReflectionFunctionLibrary::MultiplyFloatProperty(
			CargoTruck,
			TargetProperty,
			Multiplier
		);


	if (!bPowerUpApplied)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Cargo Speed PowerUp: impossibile modificare '%s'"
			),
			*TargetProperty.ToString()
		);

		return;
	}


	// La proprietà è stata modificata tramite Reflection.
	// Aggiorna quindi la configurazione runtime del motore Chaos.
	CargoTruck->RefreshEnginePower();


	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Cargo Speed PowerUp raccolto | Property: %s | Multiplier: %.2f"
		),
		*TargetProperty.ToString(),
		Multiplier
	);


	// Il PowerUp può essere raccolto una sola volta.
	Destroy();
}
