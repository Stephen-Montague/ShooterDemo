// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"


UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class SLIME_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for forwards/backwards input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	* Called via input to turn at a given rate.
	* @param Rate  This is a normalized rate, as in, 1.0 means 100% of turn rate.
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to look up/down at a given rate.
	* @param Rate  This is a normalized rate, as in,  1.0 means 100% of rate.
	*/
	void LookUpAtRate(float Rate);

	/** Rotate player controller based on mouse X movement.	*/
	void Turn(float Rate);

	/** Rotate player controller based on mouse Y movement.	*/
	void LookUp(float Rate);
	
	/** Called when the Fire Button is pressed */
	void FireWeapon();
	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();	
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	/** Sets bAiming to true or false. */
	void AimingButtonPressed();
	void AimingButtonReleased();
	
	void SetCameraFOV(float DeltaTime);
	void SetTurnLookRate();

	// Crosshairs
	void CalculateCrosshairSpread(float DeltaTime);
	void StartCrosshairBulletFireTimer();
	UFUNCTION()
	void FinishCrosshairBulletFireTimer();

// Protected: Continued.
	
	// Automatic fire
	void FireButtonPressed();
	void FireButtonReleased();
	void StartFireTimer();
	UFUNCTION()
	void AutoFireReset();

	// Look at items
	bool TraceUnderCrosshairs(FHitResult& OutHitResult);
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);
	void TraceForItems();

	/** Spawns default weapon and attaches to mesh. */
	class AWeapon* SpawnDefaultWeapon();

	/** Takes a Weapon and attaches to mesh. */
	void EquipWeapon(AWeapon* WeaponToEquip);

	/** Currently equipped weapon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	/** Set this in BP for the default weapon class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/** Detach Weapon and let it fall to the ground. */
	void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonRelease();

	/** Drops currently equipped weapon and equips weapon hit by trace. */
	void SwapWeapon(AWeapon* WeaponToSwap);

	/** Initialize Ammo Map with ammo values.  */
	void InitializeAmmoMap();
	bool WeaponHasAmmo();

	void ReloadWeapon();
	void ReloadButtonPressed();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	
	/** Checks if carrying ammo for the equipped weapon. */
	bool CarryingAmmo();

	/** Called from Animation BP with Grab Clip Notify. */
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	/** Called from Animation BP with Release Clip Notify. */
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	/** Transform of clip / mag when first reloading. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	/** Scene component to attach to character's hand during reload. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;
	
	void CrouchButtonPressed();

	/** Interps capsule half height when changing from standing to crouching. */
	void InterpCapsuleHalfHeight(float DeltaTime);

	void Aim();

	void StopAiming();

	void PickupAmmo(class AAmmo* Ammo);

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetFootstepSurface();

	void SetUnderwater();
	void SetUnderwaterSFX();

	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();
	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);
	int32 GetEmptyInventorySlot();
	void BeginHighlightInventorySlot();
	
	
// End Protected.
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/** Camera boom positioning the camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Camera that follows the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** Turn rate while NOT aiming. Console controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Look up/down rate while NOT aiming. Console controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate while aiming. Console controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;
	
	/** Look up/down rate while aiming. Console controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/** Turn rate while NOT aiming. Mouse controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

// Private: Continued.

	/** Look up/down rate while aiming. Mouse controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/** Turn rate while aiming. Mouse controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/** Look up/down rate while aiming. Mouse controller only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	/** Gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	/** Flash spawned at BarrelSocket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	/** Montage for firing the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	/** True when aiming. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/** Underwater sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundCue* UnderwaterSoundCue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAudioComponent* UnderwaterSoundPlayer;

	
// Private: Continued.

	/** Default camera field of view. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	/** Field of view when zoomed in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraZoomedFOV;

	/** Used for camera interpolation: field of view this frame. */
	float CameraCurrentFOV;

	/** Speed to change camera from Default to Zoomed. Higher is faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/** Determines spread of cross hairs. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	/** Velocity factor for cross hairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/** In air factor for cross hairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/** Aim factor for cross hairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/** Shooting factor for cross hairs spread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;
	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

// Private: Continued.
	
	/** For automatic weapon fire, timer based. */
	bool bFireButtonPressed;
	bool bShouldFire;
	float AutomaticFireRate;
	FTimerHandle AutoFireTimer;

	/** True if character should trace for items. */
	bool bShouldTraceForItems;

	/** Number of overlapped AItems. */
	int8 OverlappedItemCount;

	/** The AItem we hit last frame. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	/** AItem hit by trace in TraceForItems() - could be null. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	/** Distance outward from the camera for an interpolation destination, from item world location to in front of viewer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	/** Distance upward from the camera for an interpolation destination, from item world location to in front of viewer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	/** Map to track different ammo types. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

// Private continued.
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	/** Montage for reloading the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	/** Montage for equipping the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** Regular movement speed, based on initial Character Movement Component Max Walk Speed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	/** Crouch walking movement speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	/** Aim walking movement speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AimMovementSpeed;

	/** Current Half Height of the Collision Capsule. */
	float CurrentCapsuleHalfHeight;

	/** Half height of capsule when not crouching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	/** Half height of capsule when crouching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	bool bAimingButtonPressed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUnderwater;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float SetUnderwaterTimerRate;
	FTimerHandle CheckUnderwaterTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 6 };

	/** Delegate for sending slot info to InventoryBar widget when equipping weapons. */
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;

	/** Delegate for sending slot info to InventoryBar widget for highlighting a slot while tracing an item. */
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	/** Inventory index of currently highlighted slot, where -1 is None, 0 is Default Weapon, 1 is Slot 1, 2 is Slot 2, etc. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlot;
	
	// End Private Section.
	
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetAiming() const { return bAiming ; }
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	/** Adds to or subtracts from OverlappedItemCount and updates bShouldTraceForItems. */
	void UpdateOverlappedItemCount(int8 Amount);
	FVector GetCameraInterpLocation();
	void HandlePickupItem(AItem* Item);
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE bool GetCrouching() const { return bCrouching; }
	void EndHighlightInventorySlot();
};
