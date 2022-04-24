#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayNamme = "9mm"),
	EAT_AR UMETA(DisplayNamme = "AssaultRifle"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};
