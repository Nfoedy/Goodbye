#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GoodbyeReflectionFunctionLibrary.generated.h"


class FProperty;


/**
 * Function Library che utilizza la Reflection di Unreal per cercare e modificare proprietà a runtime tramite il loro nome.
 * 
 * Permette di cercare una UPROPERTY attraverso il suo nome durante l'esecuzione, senza conoscere direttamente il membro a compile-time.
 * 
 * In questo progetto viene utilizzata dal CargoSpeedPowerUp per individuare e modificare un proprietà del CargoTruckPawn in modo data-driven.
 */

UCLASS()
class GOODBYE_API UGoodbyeReflectionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:

	// Cerca una proprietà sull'oggetto utilizzando il suo nome a runtime
	static FProperty* RetrieveProperty(UObject* TargetObject, FName PropertyName);


	// Cerca una proprietà float tramite Reflection e ne moltiplica il valore
	UFUNCTION(BlueprintCallable, Category = "Goodbye|Reflection")
	static bool MultiplyFloatProperty(UObject* TargetObject, FName PropertyName, float Multiplier);
};