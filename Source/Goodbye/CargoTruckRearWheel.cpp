#include "CargoTruckRearWheel.h"


UCargoTruckRearWheel::UCargoTruckRearWheel()
{
	// Manteniamo inizialmente le stesse dimensioni delle ruote anteriori.
	WheelRadius = 55.0f;
	WheelWidth = 30.0f;
	WheelMass = 30.0f;


	// Le ruote posteriori non sterzano.
	bAffectedBySteering = false;
	MaxSteerAngle = 0.0f;


	// Trazione posteriore.
	bAffectedByEngine = true;


	// Freno normale.
	bAffectedByBrake = true;
	MaxBrakeTorque = 3000.0f;


	// Freno a mano sulle ruote posteriori.
	bAffectedByHandbrake = true;
	MaxHandBrakeTorque = 5000.0f;


	bABSEnabled = true;
	bTractionControlEnabled = true;
}