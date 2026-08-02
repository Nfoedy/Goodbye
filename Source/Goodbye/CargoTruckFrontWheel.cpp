#include "CargoTruckFrontWheel.h"


UCargoTruckFrontWheel::UCargoTruckFrontWheel()
{
	// Dimensioni iniziali della ruota.
	// Verranno calibrate dopo il primo test Chaos.
	WheelRadius = 55.0f;
	WheelWidth = 30.0f;
	WheelMass = 30.0f;


	// Le ruote anteriori controllano lo sterzo.
	bAffectedBySteering = true;
	MaxSteerAngle = 35.0f;


	// Per ora scegliamo una trazione posteriore.
	bAffectedByEngine = false;


	// Il freno normale agisce anche sulle ruote anteriori.
	bAffectedByBrake = true;
	MaxBrakeTorque = 3000.0f;


	// Il freno a mano agirà soltanto sulle ruote posteriori.
	bAffectedByHandbrake = false;
	MaxHandBrakeTorque = 0.0f;


	// Aiuti iniziali per rendere il veicolo più stabile.
	bABSEnabled = true;
	bTractionControlEnabled = false;
}