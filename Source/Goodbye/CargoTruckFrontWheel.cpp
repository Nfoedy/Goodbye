#include "CargoTruckFrontWheel.h"


UCargoTruckFrontWheel::UCargoTruckFrontWheel()
{
	// Configura dimensioni e massa della ruota anteriore.
	WheelRadius = 55.0f;
	WheelWidth = 30.0f;
	WheelMass = 30.0f;


	// Le ruote anteriori controllano lo sterzo.
	bAffectedBySteering = true;
	MaxSteerAngle = 35.0f;


	// Le ruote anteriori non ricevono trazione.
	bAffectedByEngine = false;


	// Il freno normale agisce anche sulle ruote anteriori.
	bAffectedByBrake = true;
	MaxBrakeTorque = 3000.0f;


	// Il freno a mano non agisce sulle ruote anteriori.
	bAffectedByHandbrake = false;
	MaxHandBrakeTorque = 0.0f;


	// Configura i sistemi di assistenza alla guida.
	bABSEnabled = true;
	bTractionControlEnabled = false;
}