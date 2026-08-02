#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "CargoTruckFrontWheel.generated.h"


/**
 * Configurazione delle ruote anteriori del camion.
 *
 * Le ruote anteriori:
 * - sterzano;
 * - frenano;
 * - non ricevono trazione;
 * - non utilizzano il freno a mano.
 */
UCLASS()
class GOODBYE_API UCargoTruckFrontWheel
	: public UChaosVehicleWheel
{
	GENERATED_BODY()


public:

	UCargoTruckFrontWheel();
};