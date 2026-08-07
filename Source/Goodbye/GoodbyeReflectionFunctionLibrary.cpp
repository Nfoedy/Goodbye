#include "GoodbyeReflectionFunctionLibrary.h"

#include "UObject/UnrealType.h"


// Cerca una proprietà sull'oggetto utilizzando il suo nome a runtime
FProperty* UGoodbyeReflectionFunctionLibrary::RetrieveProperty(UObject* TargetObject, FName PropertyName)
{
	// Senza un oggetto valido non esiste una classe nella quale cercare
	if (!IsValid(TargetObject))
	{
		return nullptr;
	}


	// Recupera la classe runtime dell'oggetto
	UClass* TargetClass = TargetObject->GetClass();

	if (!IsValid(TargetClass))
	{
		return nullptr;
	}


	// Cerca nei metadata della classe una proprietà con il nome richiesto
	FProperty* Property = TargetClass->FindPropertyByName(PropertyName);


	if (Property == nullptr)
	{
		return nullptr;
	}

	return Property;
}


// Cerca una proprietà float tramite Reflection e ne moltiplica il valore
bool UGoodbyeReflectionFunctionLibrary::MultiplyFloatProperty(UObject* TargetObject, FName PropertyName, float Multiplier)
{
	// Recupera genericamente la proprietà tramite Reflection
	FProperty* Property = RetrieveProperty(TargetObject, PropertyName);


	if (Property == nullptr)
	{
		return false;
	}


	// Verifica che la proprietà trovata sia realmente un float
	FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);


	if (FloatProperty == nullptr)
	{
		return false;
	}


	// Recupera l'indirizzo del valore della proprietà appartenente alla specifica istanza ricevuta
	float* PropertyValue = FloatProperty->ContainerPtrToValuePtr<float>(TargetObject);


	if (PropertyValue == nullptr)
	{
		return false;
	}


	const float OldValue = *PropertyValue;

	const float NewValue = OldValue * Multiplier;


	// Modifica direttamente il valore trovato tramite Reflection
	*PropertyValue = NewValue;

	return true;
}