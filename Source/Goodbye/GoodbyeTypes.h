#pragma once

#include "CoreMinimal.h"
#include "GoodbyeTypes.generated.h"


// Difficoltà selezionabile dal giocatore.
UENUM(BlueprintType)
enum class EGameDifficulty : uint8
{
	Easy	UMETA(DisplayName = "Easy"),
	Normal	UMETA(DisplayName = "Normal"),
	Hard	UMETA(DisplayName = "Hard")
};