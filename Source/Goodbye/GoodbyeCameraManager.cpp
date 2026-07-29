#include "GoodbyeCameraManager.h"

// Costruttore
AGoodbyeCameraManager::AGoodbyeCameraManager()
{
	// Evita che il giocatore ruoti la testa verticalmente
	// -70° <= Pitch <= 80°
	ViewPitchMin = -70.0f;
	ViewPitchMax = 80.0f;
}
