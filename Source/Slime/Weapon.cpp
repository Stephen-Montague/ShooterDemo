// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
	FallingWeaponDuration(1.2f),
	bFalling(false),
	AmmoCount(36),
	WeaponType(EWeaponType::EWT_SubmachineGun),
	MagazineCapacity(36),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(FName(TEXT("Reload SMG"))),
	ClipBoneName(TEXT("smg_clip"))  // Todo: change when default weapon changes.

{
	// Empty constructor.
}

//void AWeapon::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//	if (bFalling && GetItemState() == EItemState::EIS_Falling)
//	{
//		const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
//		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
//	}
//	
//}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
	const FVector MeshRight{ GetItemMesh()->GetRightVector() };
	
	// Direction in which we throw the weapon (forward, right, and slightly down).
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	const float RandomRotation{ FMath::FRandRange(20.f, 40.f) };
	// const float RandomRotation{ 30.f };

	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 5'500.f;
	GetItemMesh()->AddImpulse(ImpulseDirection);

	bFalling = true;
	GetWorldTimerManager().SetTimer(FallingWeaponTimer, this, &AWeapon::StopFalling, FallingWeaponDuration);

	EnableGlowMaterial();
}

void AWeapon::DecrementAmmo()
{
	if ((AmmoCount - 1) < 0)
	{
		AmmoCount = 0;
	}
	else
	{
		--AmmoCount;
	}
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(AmmoCount + Amount <= MagazineCapacity, TEXT("Attempted to reload with more than mag capacity."));
	AmmoCount += Amount;
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	const FString WeaponTablePath(TEXT("DataTable'/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable'"));
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));
	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;
		switch (WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubMachineGun"), TEXT(""));
			break;
		case EWeaponType::EWT_AssaultRifle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
			break;
		}
		if (WeaponDataRow)
		{
				AmmoType = WeaponDataRow->AmmoType;
				AmmoCount = WeaponDataRow->AmmoCount;
				MagazineCapacity = WeaponDataRow->MagazineCapacity;
				SetPickupSound(WeaponDataRow->PickupSound);
				SetEquipSound(WeaponDataRow->EquipSound);
				GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
				SetItemName(WeaponDataRow->ItemName);
				SetIconImage(WeaponDataRow->InventoryIcon);
				SetAmmoIcon(WeaponDataRow->AmmoIcon);
		}
	}	
}

