#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "CargoTruckRearWheel.generated.h"


/**
 * Configurazione delle ruote posteriori del camion.
 *
 * Le ruote posteriori:
 * - non sterzano;
 * - ricevono la trazione del motore;
 * - frenano;
 * - utilizzano il freno a mano.
 */
UCLASS()
class GOODBYE_API UCargoTruckRearWheel
	: public UChaosVehicleWheel
{
	GENERATED_BODY()


public:

	UCargoTruckRearWheel();
};