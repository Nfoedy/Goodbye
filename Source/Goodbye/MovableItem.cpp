#include "MovableItem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"


// Costruttore 
AMovableItem::AMovableItem()
{

	// L'actor non deve eseguire operazioni ogni frame
	PrimaryActorTick.bCanEverTick = false;

	// Crea il componente della static mesh del componente
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));

	// La static mesh diventa il root component dell'actor
	SetRootComponent(ItemMesh);


	// Attiva la simulazione fisica
	ItemMesh->SetSimulatePhysics(true);


	// Attiva la gravità
	ItemMesh->SetEnableGravity(true);

	// Applica il profilo di collisione PhysicsActor
	ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	// Abilita la generazione degli eventi OnComponentHit
	ItemMesh->SetNotifyRigidBodyCollision(true);

	// Permette alla mesh di generare eventi quando entra o esce da una Cargo Zone
	ItemMesh->SetGenerateOverlapEvents(true);

	
	// Collega l'evento di collisione della mesh alla funzione HandleItemHit()
	ItemMesh->OnComponentHit.AddDynamic(this, &AMovableItem::HandleItemHit);

	// Ignora la cam
	ItemMesh->SetCollisionResponseToChannel(ECC_Camera,	ECR_Ignore);


	// Crea il componente che visualizza la barra della vita
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));

	// Collega la barra all'oggetto
	HealthWidgetComponent->SetupAttachment(ItemMesh);

	// Posiziona la barra sopra l'oggetto
	HealthWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

	// La barra rimane orientata correttamente sullo schermo
	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	// Dimensione della barra
	HealthWidgetComponent->SetDrawSize(FVector2D(160.0f, 20.0f));

	// La barra non deve avere collisioni
	HealthWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Parte nascosta
	HealthWidgetComponent->SetVisibility(false);
}


void AMovableItem::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bIsBroken = false;

	if (IsValid(HealthWidgetComponent))
	{
		HealthWidgetComponent->InitWidget();

		UpdateHealthWidget();

		HealthWidgetComponent->SetVisibility(false);
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("%s inizializzato con %.1f HP"),
		*GetName(),
		CurrentHealth
	);
}

// Massa
float AMovableItem::GetItemMassInKg() const
{
	
	// Se la mesh non è valida restituisce 0
	if (!IsValid(ItemMesh))
	{
		return 0.0f;
	}

	// Restituisce la massa effettiva 
	return ItemMesh->GetMass();
}



void AMovableItem::HandleItemHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse,	const FHitResult& Hit)
{
	// Ignora collisioni non valide o con sé stesso.
	if (!IsValid(ItemMesh) || !IsValid(OtherActor) || OtherActor == this ||	!GetWorld())
	{
		return;
	}

	// Legge la massa fisica reale della Static Mesh.
	const float ObjectMassInKg = FMath::Max(ItemMesh->GetMass(), 0.1f);

	// Normalizza l'impulso rispetto alla massa.
	const float ImpactSeverity = NormalImpulse.Size() / ObjectMassInKg;

	// Ignora piccoli urti, vibrazioni e assestamenti.
	if (ImpactSeverity < MinimumImpactSeverity)
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// Evita di registrare più volte lo stesso impatto.
	if (CurrentTime - LastImpactTime < ImpactCooldown)
	{
		return;
	}

	LastImpactTime = CurrentTime;

	// Calcola il danno solamente sulla parte dell'impatto che supera la soglia minima
	const float RawDamage = (ImpactSeverity - MinimumImpactSeverity) * ImpactDamageMultiplier;

	// Protegge il Clamp nel caso in cui i valori vengano configurati male nel Blueprint
	const float SafeMaximumDamage =	FMath::Max(	MaximumDamagePerImpact, MinimumDamagePerImpact);

	// Limita il danno base tra il minimo e il massimo configurati.
	const float BaseDamage = FMath::Clamp(RawDamage, MinimumDamagePerImpact, SafeMaximumDamage);

	// Verifica se l'impatto è avvenuto contro un altro oggetto trasportabile
	const bool bHitAnotherMovableItem = IsValid(Cast<AMovableItem>(OtherActor));

	// Gli urti tra due MovableItem producono metà del danno normale
	const float DamageMultiplier = bHitAnotherMovableItem ? ItemToItemDamageMultiplier : 1.0f;


	// Applica il moltiplicatore dopo il Clamp,  così il danno finale è realmente dimezzato.
	const float Damage = BaseDamage * DamageMultiplier;


	// Applica il danno alla salute dell'oggetto.
	ApplyImpactDamage(Damage);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"%s ha colpito %s | "
			"Massa: %.1f kg | "
			"Severita: %.2f cm/s | "
			"Moltiplicatore: %.2f | "
			"Danno: %.2f"
		),
		*GetName(),
		*GetNameSafe(OtherActor),
		ObjectMassInKg,
		ImpactSeverity,
		DamageMultiplier,
		Damage
	);
}


// ApplyDamage
void AMovableItem::ApplyImpactDamage(float DamageAmount)
{
	if (bIsBroken || DamageAmount <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	UpdateHealthWidget();


	// Mostra la barra per tre secondi dall'ultimo impatto
	ShowHealthWidgetTemporarily();

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"%s | Danno ricevuto: %.2f | "
			"Salute: %.2f / %.2f"
		),
		*GetName(),
		DamageAmount,
		CurrentHealth,
		MaxHealth
	);

	if (CurrentHealth <= 0.0f)
	{
		DestroyItem();
	}
}


// Mostra temporaneamente la barra e riavvia il timer di scomparsa
void AMovableItem::ShowHealthWidgetTemporarily()
{
	if (bHealthWidgetSuppressed || bIsBroken ||	CurrentHealth <= 0.0f ||
		!IsValid(HealthWidgetComponent) ||
		!GetWorld()
		)
	{
		return;
	}


	// Mostra la barra aggiornata.
	HealthWidgetComponent->SetVisibility(true);


	// Annulla il timer precedente, così ogni nuovo colpo
	// fa ripartire il conteggio da zero.
	GetWorldTimerManager().ClearTimer(
		HealthWidgetVisibilityTimerHandle
	);


	// Nasconde la barra dopo il tempo configurato.
	GetWorldTimerManager().SetTimer(
		HealthWidgetVisibilityTimerHandle,
		this,
		&AMovableItem::HideHealthWidget,
		HealthWidgetVisibleDuration,
		false
	);
}


// Nasconde la barra della vita.
void AMovableItem::HideHealthWidget()
{
	if (!IsValid(HealthWidgetComponent))
	{
		return;
	}


	HealthWidgetComponent->SetVisibility(false);
}


// Nasconde la barra della vita e impedisce che ricompaia.
void AMovableItem::SuppressHealthWidget()
{
	bHealthWidgetSuppressed = true;


	// Il timer non deve più provare a modificare la barra.
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(
			HealthWidgetVisibilityTimerHandle
		);
	}


	HideHealthWidget();
}





// Punteggio
// Restituisce il punteggio configurato nel Blueprint
int32 AMovableItem::GetItemScore() const
{
	return FMath::Max(ItemScore, 0);
}


// Destroy Item
void AMovableItem::DestroyItem()
{
	if (bIsBroken)
	{
		return;
	}

	bIsBroken = true;
	CurrentHealth = 0.0f;

	// Ferma il timer e nasconde la barra prima di distruggere l'oggetto.
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(
			HealthWidgetVisibilityTimerHandle
		);
	}

	HideHealthWidget();

	if (IsValid(ItemMesh))
	{
		// Impedisce nuovi eventi di impatto mentre l'Actor attende di essere rimosso
		ItemMesh->SetNotifyRigidBodyCollision(false);

		ItemMesh->SetSimulatePhysics(false);

		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Destroy();
}



void AMovableItem::UpdateHealthWidget()
{
	if (!IsValid(HealthWidgetComponent))
	{
		return;
	}

	// Recupera il Widget Blueprint visualizzato dal Widget Component
	UUserWidget* HealthWidget = HealthWidgetComponent->GetUserWidgetObject();

	if (!IsValid(HealthWidget))
	{
		return;
	}

	// Cerca nel WBP la Progress Bar chiamata esattamente "HealthProgressBar"
	UProgressBar* HealthProgressBar =Cast<UProgressBar>(HealthWidget->GetWidgetFromName(TEXT("HealthProgressBar"))
		);

	if (!IsValid(HealthProgressBar))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"%s: HealthProgressBar non trovata nel Widget"
			),
			*GetName()
		);

		return;
	}

	// Aggiorna la barra con un valore tra 0 e 1
	HealthProgressBar->SetPercent(GetHealthPercent());
}