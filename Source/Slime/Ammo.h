// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class SLIME_API AAmmo : public AItem
{
	GENERATED_BODY()
public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;

	/** Override of SetItemProperties to set properties for an Ammo Static Mesh Component, not a Skeletal Mesh.  */
	virtual void SetItemProperties(EItemState State) override;

	/** Used to interpolate item scale on pickup, and calls super to interpolate location and rotation. */
	virtual void ItemInterping(float DeltaTime) override;

	
	void SphereCollisionOverlap();

	UFUNCTION()
	void AmmoSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
		
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIconTexture;

	/** Overlap sphere for picking up ammo by character collision. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AmmoCollisionSphere;
	
public:

	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType;  }
	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;
};
