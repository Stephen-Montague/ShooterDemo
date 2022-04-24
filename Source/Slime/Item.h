// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Worthless UMETA(DisplayName = "Worthless"),
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),
	
	EIR_MAX = 5 UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),

	EIT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
};

UCLASS()
class SLIME_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** Sets properties of item components based on current state. */
	virtual void SetItemProperties(EItemState State);

	/** Called when ItemIterpTimer is finished. */
	void FinishIterping();

	/** Handles Item interpolation when in the EquipInterping state. */
	virtual void ItemInterping(float DeltaTime);

	/** Item pickup interpolation speed (how fast it moves to the character). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float InterpolationSpeed;

	virtual void InitializeCustomDepth();

	virtual void OnConstruction(const FTransform& Transform) override;


	/** Not used FX code.  If possible, test removing this section. */
	void EnableGlowMaterial();	
	void DisableGlowMaterial();
	/** Not currently used. - Curve to drive dynamic material parameters (allowing item to pulse, etc). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveVector* PulseCurve;
	FTimerHandle PulseTimer;
	/** Not currently used. - Time interval for the pulse timer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;
	/** Not currently used. - Dynamic material glow. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;
	/** Not currently used. - Effect that increases glow on surfaces turned away from the camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;
	void ResetPulseTimer();
	void StartPulseTimer();
	/** Not currently used. - If in Item State Pickup, get curve value from PulseCurve and set dynamic material parameters. */
	void UpdatePulse();
	/** End of not used FX section. */

	
// End Protected Section.
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Line trace collides with box to show HUD widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	/** Item skeletal mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	/** Pop up widget for when player looks at item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	/** Enables item trace when overlapped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	/** Name that appears on the widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	/** Item count (ammo, etc). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	/** Determines number of stars in Pickup Widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	/** True during interpolation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bInterping;

	/** Plays when we start interpolation. */
	FTimerHandle ItemIterpTimer;

	/** Duration of the curve and timer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float IterpTimerDuration;
	
	/** Pointer to the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* Character;

	/** Sound played when item picked up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	/** Sound played when item equipped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;

	/** Enum for the type of item this is. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	/** Index to allow changing materials at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	/** Dynamic material instance - may change at runtime. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	/** Material to use for the dynamic material. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	/** Background for this item in the inventory AND widget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;

	/** Icon for this item in the inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconImage;

	/** Ammo icon for this item in the inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIcon;

	/** Slot occupied in the inventory array of a character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/** Toggles whether the Item's widget text displays as "Pick up" or "Swap" - depending on the character tracing the item. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	bool bWidgetTextIsSwap;

	/** Item rarity row base. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDataTable;

	/** Pickup Widget rarity properties based on DataTable set in Editor.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	// Probably redundant - remove after testing.
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	// UTexture2D* WidgetIconBackground;
	
// End Private Section.

// Getters and Setters.
public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox;  }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(const EItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	void BeginEquip(AShooterCharacter* InstigatingCharacter);
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE void SetPickupSound(USoundCue* Sound) { PickupSound = Sound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquipSound = Sound; }
	FORCEINLINE bool IsInterping() const { return bInterping; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount;  }
	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(const int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetWidgetTextIsSwap(const bool bMustSwap) { bWidgetTextIsSwap = bMustSwap; }
	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }
	FORCEINLINE void SetIconImage(UTexture2D* Icon) { IconImage = Icon; }
	FORCEINLINE void SetAmmoIcon(UTexture2D* Icon) { AmmoIcon = Icon; }
};


