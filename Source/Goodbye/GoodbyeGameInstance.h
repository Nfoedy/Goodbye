#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GoodbyeTypes.h"
#include "GoodbyeGameInstance.generated.h"


/**
 * Conserva i dati che devono rimanere disponibili
 * durante il passaggio tra Main Menu e livello di gioco.
 */
UCLASS()
class GOODBYE_API UGoodbyeGameInstance : public UGameInstance
{
	GENERATED_BODY()


public:

	// Imposta la difficoltà selezionata nel Main Menu.
	UFUNCTION(BlueprintCallable, Category = "Goodbye|Difficulty")
	void SetSelectedDifficulty(EGameDifficulty NewDifficulty);


	// Restituisce la difficoltà attualmente selezionata.
	UFUNCTION(BlueprintPure, Category = "Goodbye|Difficulty")
	EGameDifficulty GetSelectedDifficulty() const
	{
		return SelectedDifficulty;
	}


private:

	// Difficoltà conservata durante i cambi di livello.
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Goodbye|Difficulty",
		meta = (AllowPrivateAccess = "true")
	)
	EGameDifficulty SelectedDifficulty = EGameDifficulty::Normal;
};