// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Weapon.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Ammo.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Slime.h"
#include "Components/AudioComponent.h"

// Set default values.
AShooterCharacter::AShooterCharacter() :
	// Base for turning / looking up.
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	// Turn rate while aiming / not aiming - for console controller.
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(40.f),
	AimingLookUpRate(40.f),
	// Turn rate while aiming / not aiming - for mouse controller.
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.66f),
	MouseAimingLookUpRate(0.66f),
	// True when aiming.
	bAiming(false),
	// Camera Field of View.
	CameraDefaultFOV(0.f), // Set in BeginPlay().
	CameraZoomedFOV(28.f),
	CameraCurrentFOV(0.f),  // Set in BeginPlay().
	ZoomInterpSpeed(20.f),
	// Crosshair spread, updated via Tick() in SetCrosshairSpreadMultiplier().
	CrosshairSpreadMultiplier(0.5),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// Used for bullet fire timer. 
	ShootTimeDuration(0.12f),
	bFiringBullet(false),
	// Used for automatic weapon fire.
	AutomaticFireRate(0.1f),
	bShouldFire(true),
	bFireButtonPressed(false),
	// Item trace
	bShouldTraceForItems(false),
	// For interpolating items being equipped momentarily to the front of player's view
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	// Ammo
	Starting9mmAmmo(108),
	StartingARAmmo(40),
	// Combat state
	CombatState(ECombatState::ECS_Unoccupied),
	// Movement and aiming
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	AimMovementSpeed(450.f),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),
	bAimingButtonPressed(false),
	// Underwater
	bUnderwater(false),
	SetUnderwaterTimerRate(0.2),
	// Inventory
	HighlightedSlot(-1)

{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (which pulls in towards the character if there is a collision).
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 240.f; 
	CameraBoom->bUsePawnControlRotation = true; 
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a follow camera.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate character when the controller rotates - controller only rotates camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement.
	GetCharacterMovement()->bOrientRotationToMovement = false; 
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); 
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create hand scene component, attached in GrabClip().
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComponent"));
}

// Called when the game starts or when spawned.
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	// Hide the Belica skeleton weapons.
	GetMesh()->HideBoneByName(TEXT("weapon"), EPhysBodyOp::PBO_None);
	GetMesh()->HideBoneByName(TEXT("pistol"), EPhysBodyOp::PBO_None);

	GetWorldTimerManager().SetTimer(CheckUnderwaterTimer, this, &AShooterCharacter::SetUnderwaterSFX, SetUnderwaterTimerRate, true, 0.5);
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Calculate which way is forward.
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Calculate which way is right.
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information.
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // Degrees per Second * Seconds per Frame = Deg / Frame
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Rate)
{
	float TurnScaleFactor;
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Rate * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Rate)
{
	float LookUpScaleFactor;
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Rate * LookUpScaleFactor);
}

void AShooterCharacter::PlayFireSound()
{
	// Play fire sound.
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(
			SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					BeamEnd);
			}
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					BeamParticles,
					SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	// Play hip fire montage.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	StartCrosshairBulletFireTimer();  // Animates crosshairs.
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();
		StartFireTimer();
	}
}

bool AShooterCharacter::GetBeamEndLocation(
	const FVector& MuzzleSocketLocation,
	FVector& OutBeamLocation)
{
	// Check for crosshair trace hit.
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);
	if (bCrosshairHit)
	{
		// Initial Beam End Location.  Next trace will be from gun barrel.
		OutBeamLocation = CrosshairHitResult.Location;
	}  // Else: OutBeamLocation is the End location of the line trace.

	// Perform a second trace, this time from the gun barrel.
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f};  // Lengthen second trace to ensure consistent hit result.
	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit) // Object between barrel and BeamEndPoint?
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return  false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();	
}

void AShooterCharacter::SetCameraFOV(float DeltaTime)
{
	if (bAiming && CameraCurrentFOV != CameraZoomedFOV)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
	if (!bAiming && CameraCurrentFOV != CameraDefaultFOV)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
		GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
	}
}

/** Change turn and look sensitivity based on aiming, for game controller, but does NOT adjust mouse). */
void AShooterCharacter::SetTurnLookRate()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else 
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };  // Since called on tick, magic numbers are most performant.
	FVector2D VelocityMultiplierRange{ 0.f, 1.0f };
	FVector LateralVelocity{ GetVelocity() };
	LateralVelocity.Z = 0;

	// Calculate velocity factor.
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, LateralVelocity.Size());

	// Calculate in air factor.
	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);  // Spread crosshairs slowly while in air.
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);  // Return crosshairs quickly when on ground.
	}

	// Calculate aiming factor.
	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.4f, DeltaTime, 30.f);  // Tighten crosshairs quickly a small amount.
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);  // Return crosshairs quickly.
	}

	// Calculate shooting factor.
	if (bFiringBullet)  
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.7f, DeltaTime, 60.f);  // Widen crosshairs very quickly a small amount.
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 45.f);  // Return crosshairs quickly.
	}
	
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFireTimer()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFireTimer, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFireTimer()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;	
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult)
{
	// Get current size of the viewport.
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of cross hairs.
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of cross hairs.
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			return true;
		}
	}
	return false;
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Get current size of the viewport.
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of cross hairs.
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of cross hairs.
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if(bScreenToWorld)
	{
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceHitResult;
		TraceUnderCrosshairs(ItemTraceHitResult);
		if (ItemTraceHitResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceHitResult.GetActor());
			
			// Handle Inventory Highlights.
			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			if(TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
				{
					BeginHighlightInventorySlot();
				}
				
			}
			else
			{
				if (HighlightedSlot != -1)
				{
					EndHighlightInventorySlot();
				}
			}
			
			// Begin Item Highlight and Show Widget.
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->EnableCustomDepth();
				TraceHitItem->GetPickupWidget()->SetVisibility(true);

				// Toggle Item's widget text prompt ("Pickup" vs "Swap") based on whether our inventory is full.
				Inventory.Num() == INVENTORY_CAPACITY ? TraceHitItem->SetWidgetTextIsSwap(true) : TraceHitItem->SetWidgetTextIsSwap(false);
			}

			// End Item Highlight and Hide Widget.
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->DisableCustomDepth();
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if(TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->DisableCustomDepth();
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check the TSubclassOf variable is set in BP.
	if (DefaultWeaponClass)
	{
		// Spawn weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{
		// Play SFX
		if (WeaponToEquip->GetEquipSound())
		{
			UGameplayStatics::PlaySound2D(this, WeaponToEquip->GetEquipSound());
		}
		
		// Attach to hand socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		
		// Broadcast to HUD which inventory item was selected and which to select next.
		if (EquippedWeapon == nullptr) 
		{
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else  
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		// Set... 
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (TraceHitItem)
	{
		TraceHitItem->BeginEquip(this);
	}
}

void AShooterCharacter::SelectButtonRelease()
{

	
}

void AShooterCharacter::SwapWeapon(AWeapon* Weapon)
{
	DropWeapon();
	EquipWeapon(Weapon);
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon)
	{
		return EquippedWeapon->GetAmmo() > 0;
	}
	return false;
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon == nullptr) return;

	if (CarryingAmmo())  
	{
		if (bAiming)
		{
			StopAiming();
		}
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::FinishReloading()
{
	if (EquippedWeapon == nullptr) return;

	// Update AmmoMap.
	const auto AmmoType{ EquippedWeapon->GetAmmoType() };
	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];

		// Calculate ammo space left in the equipped weapon.
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// Dump all ammo into weapon and update.
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// Fill mag and update.
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);			
		}
	}

	// Update State.
	CombatState = ECombatState::ECS_Unoccupied;

	// Reload may override aiming, so check if still aiming after reload is finished.
	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	const auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}
	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	// Index for the clip / mag bone on the weapon.
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };

	// Store the transform of the clip / mag.
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	// Attach a scene component, that represents the mag, to the left hand, keeping a relative transform to hand, then set the world transform of the component to the mag.
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{

	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if ( ! GetCharacterMovement()->IsFalling())  // IsFalling() returns true if character is in the air, so "If (not in air...), do..."
	{
		bCrouching = ! bCrouching;  // Toggle boolean value every touch.
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight;
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight { FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f) };

	// Raise or lower mesh by the difference between the target capsule height and the current height - keeps crouching character on the ground, not under it.
	// Delta is negative if crouching, positive if standing - negate the delta to raise / lower mesh. 
	const float DeltaCapsuleHalfHeight { InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = AimMovementSpeed;
	}
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// Play SFX
	if (Ammo->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Ammo->GetEquipSound());
	}
	
	// Check if AmmoMap contains correct ammo, get the amount, and update it.
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	// For convenience, if equipped weapon is empty, and ammo pickup matches, reload.
	if (EquippedWeapon->GetAmmo() == 0 && EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		ReloadWeapon();
	}
	Ammo->Destroy();
}

EPhysicalSurface AShooterCharacter::GetFootstepSurface()
{
	if (bUnderwater)
	{
		return EPS_UNDERWATER;
	}
	FHitResult HitResult;
	const FVector StartLocation{ GetActorLocation() };
	const FVector EndLocation = StartLocation + FVector(0.f, 0.f, -400.f);
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, QueryParams);
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterCharacter::SetUnderwater()
{
	FHitResult HitResult;
	const FVector StartLocation{ GetActorLocation() };
	const FVector EndLocation{ StartLocation + FVector(0.f, 0.f, 3000.f) };
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	bool bTraceHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, QueryParams);
	if (bTraceHit && UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get()) == EPS_WATER && HitResult.Location.Z > GetFollowCamera()->GetComponentLocation().Z + 25)
	{
		bUnderwater = true;
	}
	else
	{
		bUnderwater = false;
	}
}

void AShooterCharacter::SetUnderwaterSFX()
{
	if  (bUnderwater)
	{
		if (UnderwaterSoundCue)
		{
			if (!UnderwaterSoundPlayer || UnderwaterSoundPlayer->GetPlayState() == EAudioComponentPlayState::Stopped)
			{
				UnderwaterSoundPlayer = UGameplayStatics::SpawnSoundAttached(UnderwaterSoundCue, GetMesh());
			}
		}
	}
	else
	{
		if (UnderwaterSoundPlayer)
		{
			if (UnderwaterSoundPlayer->GetPlayState() == EAudioComponentPlayState::Playing)
			{
				UnderwaterSoundPlayer->FadeOut(0.6f, 0.f);
			}
		}
	}
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;
	
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;

	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if (CurrentItemIndex == NewItemIndex || !Inventory.IsValidIndex(NewItemIndex)) return;

	if (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping)
	{
		CombatState = ECombatState::ECS_Equipping;

		EquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		EquipWeapon(Cast<AWeapon>(Inventory[NewItemIndex]));

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}
	}
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	int i{ 0 };
	for (const auto& Item : Inventory)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
		++i;
	}
	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}
	return -1; // Inventory is full.
}

void AShooterCharacter::BeginHighlightInventorySlot()
{
	const int32 EmptySlot{ GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

void AShooterCharacter::EndHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

// Called every frame.  To optimize, some of these could be called on timers from Begin Play.
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetCameraFOV(DeltaTime);
	SetTurnLookRate();
	CalculateCrosshairSpread(DeltaTime);
	TraceForItems();
	InterpCapsuleHalfHeight(DeltaTime);
	SetUnderwater();
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate); // For console controller
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate); // For console controller
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);  // For mouse
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp); // For mouse

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonRelease);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
	
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::UpdateOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation(FollowCamera->GetComponentLocation());
	const FVector CameraForward{ FollowCamera->GetForwardVector() };

	// Desired location = CameraLocation + (Fwd * DistA) + Up.
	return CameraWorldLocation + (CameraForward * CameraInterpDistance) +
		FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::HandlePickupItem(AItem* Item)
{
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon == EquippedWeapon) { return; }

	// Play SFX
	if (Item->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetPickupSound());
	}
	
	if (Weapon)
	{
		// If the inventory has enough capacity, add the weapon.
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());  // Set Weapon SlotIndex based on Inventory's current size, before adding it. 
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		// Otherwise replace the current weapon in the inventory, drop the old weapon, and equip the new weapon.
		else
		{
			if (Inventory.IsValidIndex(EquippedWeapon->GetSlotIndex()))  
			{
				Inventory[EquippedWeapon->GetSlotIndex()] = Weapon;
				Weapon->SetSlotIndex(EquippedWeapon->GetSlotIndex());
			}
		
			DropWeapon();
			EquipWeapon(Weapon);
			TraceHitItem = nullptr;  // Ensures the source of this pickup cannot be picked up again.
			TraceHitItemLastFrame = nullptr;
		}
	}
	
	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}
