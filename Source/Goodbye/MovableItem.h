#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/Actor.h"
#include "MovableItem.generated.h"


class UStaticMeshComponent;
class UPrimitiveComponent;
class UWidgetComponent;
class USoundBase;

struct FHitResult;


/**
 * Categoria che rappresenta la grandezza gameplay dell'oggetto trasportabile.
 *
 * La categoria determina il punteggio dell'oggetto,
 * ma non modifica la sua massa fisica.
 */
UENUM(BlueprintType)
enum class EMovableItemSize : uint8
{
	Small		UMETA(DisplayName = "Small"),
	Medium		UMETA(DisplayName = "Medium"),
	Large		UMETA(DisplayName = "Large"),
	VeryLarge	UMETA(DisplayName = "Very Large")
};


/**
 * Classe base di tutti gli oggetti trasportabili.
 *
 * Gestisce:
 * - Static Mesh;
 * - simulazione fisica;
 * - gravità;
 * - collisioni;
 * - lettura della massa fisica;
 * - categoria di grandezza;
 * - punteggio.
 */
UCLASS()
class GOODBYE_API AMovableItem : public AActor
{
	GENERATED_BODY()


	/* PUBLIC */
public:

	// Costruttore
	AMovableItem();


	// Restituisce il punteggio dell'oggetto
	// Small = 1 pt
	// Medium = 2 pt
	// Large = 3 pt
	// Very Large = 4 pt
	UFUNCTION(BlueprintPure, Category = "Movable Item|Score")
	int32 GetItemScore() const;


	// Restituisce la massa effettiva utilizzata dalla Static Mesh
	UFUNCTION(BlueprintPure, Category = "Movable Item|Physics")
	float GetItemMassInKg() const;


	// Restituisce la categoria di grandezza dell'oggetto
	UFUNCTION(BlueprintPure, Category = "Movable Item|Score")
	EMovableItemSize GetItemSize() const
	{
		return ItemSize;
	}


	// Restituisce il componente Static Mesh dell'oggetto
	UStaticMeshComponent* GetItemMesh() const
	{
		return ItemMesh.Get();
	}

	// Nasconde la barra della vita e impedisce che ricompaia
	void SuppressHealthWidget();


	// Restituisce la salute corrente dell'oggetto
	UFUNCTION(BlueprintPure, Category = "Movable Item|Health")
	float GetCurrentHealth() const
	{
		return CurrentHealth;
	}

	// Tempo durante il quale la barra rimane visibile dopo un impatto
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movable Item|Health", meta = (ClampMin = "0.1", Units = "s"))
	float HealthWidgetVisibleDuration = 3.0f;


	// Restituisce la salute massima dell'oggetto
	UFUNCTION(BlueprintPure, Category = "Movable Item|Health")
	float GetMaxHealth() const
	{
		return MaxHealth;
	}


	// Restituisce la salute normalizzata tra 0 e 1
	// Verrà utilizzata successivamente dalla barra della vita
	UFUNCTION(BlueprintPure, Category = "Movable Item|Health")
	float GetHealthPercent() const
	{
		if (MaxHealth <= 0.0f)
		{
			return 0.0f;
		}

		return FMath::Clamp(
			CurrentHealth / MaxHealth,
			0.0f,
			1.0f
		);
	}


/* FUNZIONI DI AACTOR */
protected:

	// Inizializza la salute corrente usando la salute massima
	virtual void BeginPlay() override;


/* COMPONENTI */
protected:

	// Mesh visiva e fisica dell'oggetto
	UPROPERTY(VisibleAnywhere,	BlueprintReadOnly,	Category = "Movable Item|Components")
	TObjectPtr<UStaticMeshComponent> ItemMesh = nullptr;

	// Widget Component che mostra la barra della vita sopra l'oggetto
	UPROPERTY(VisibleAnywhere,	BlueprintReadOnly,	Category = "Movable Item|Components")
	TObjectPtr<UWidgetComponent> HealthWidgetComponent = nullptr;


/* CONFIGURAZIONE DEL PUNTEGGIO */
protected:

	// Categoria di grandezza gameplay dell'oggetto
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Score"
	)
	EMovableItemSize ItemSize = EMovableItemSize::Small;

	// Punteggio assegnato all'oggetto quando si trova nel cargo
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Score",
		meta = (ClampMin = "0")
	)
	int32 ItemScore = 1;


/* CONFIGURAZIONE DEGLI IMPATTI */
protected:

	// Severità minima dell'impatto necessaria
	// per considerare significativo un urto
	// Il valore è espresso approssimativamente in cm/s
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Impact",
		meta = (ClampMin = "0.0", Units = "cm/s")
	)
	float MinimumImpactSeverity = 250.0f;


	// Tempo minimo tra due impatti significativi. Evita che una singola caduta produca molti eventi consecutivi
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Impact",
		meta = (ClampMin = "0.0", Units = "s")
	)
	float ImpactCooldown = 0.35f;


	// Moltiplicatore applicato quando l'urto avviene contro un altro MovableItem.
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Damage",
		meta = (ClampMin = "0.0", ClampMax = "1.0")
	)
	float ItemToItemDamageMultiplier = 0.5f;


/* CONFIGURAZIONE HEALTH */
protected:

	// Salute massima dell'oggetto
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Health",
		meta = (ClampMin = "1.0")
	)
	float MaxHealth = 100.0f;


	// Salute corrente dell'oggetto
	// Viene inizializzata a MaxHealth quando inizia il gioco
	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Transient,
		Category = "Movable Item|Health"
	)
	float CurrentHealth = 100.0f;


/* CONFIGURAZIONE DEL DANNO DA IMPATTO */
protected:

	// Moltiplicatore utilizzato per convertire
	// la severità dell'impatto in danno
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Damage",
		meta = (ClampMin = "0.0")
	)
	float ImpactDamageMultiplier = 0.08f;


	// Danno minimo prodotto da un impatto significativo
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Damage",
		meta = (ClampMin = "0.0")
	)
	float MinimumDamagePerImpact = 1.0f;


	// Danno massimo che un singolo impatto può produrre
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movable Item|Damage",
		meta = (ClampMin = "1.0")
	)
	float MaximumDamagePerImpact = 60.0f;


/* STATO INTERNO DEGLI IMPATTI */
private:

	// Istante dell'ultimo impatto significativo registrato
	float LastImpactTime = -1000.0f;


	// Indica che l'oggetto ha terminato la salute
	// ed è già in fase di distruzione
	bool bIsBroken = false;

	// Timer che nasconde la barra dopo l'ultimo impatto.
	FTimerHandle HealthWidgetVisibilityTimerHandle;


	// Impedisce alla barra di ricomparire durante la cinematica finale.
	bool bHealthWidgetSuppressed = false;


	// Suono riprodotto quando l'oggetto viene distrutto.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movable Item|Audio",	meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> DestroySound = nullptr;



/* EVENTI FISICI */
private:

	// Riceve gli eventi di collisione della Static Mesh
	UFUNCTION()
	void HandleItemHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit
	);


/* FUNZIONI INTERNE DEL SISTEMA DI DANNO */
private:

	// Applica una quantità di danno alla salute dell'oggetto
	void ApplyImpactDamage(float DamageAmount);


	// Gestisce la distruzione definitiva dell'oggetto
	void DestroyItem();

	// Aggiorna il Widget Health
	void UpdateHealthWidget();

	// Mostra temporaneamente la barra e riavvia il timer di scomparsa
	void ShowHealthWidgetTemporarily();

	// Nasconde la barra della vita
	void HideHealthWidget();
};