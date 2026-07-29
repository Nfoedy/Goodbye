#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "GoodbyeCameraManager.generated.h"

/*
* Eredita da PlayerCameraManager
* 
*/

UCLASS()
class AGoodbyeCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	// Costruttore
	AGoodbyeCameraManager();
};
