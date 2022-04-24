// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"


UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_NotAiming UMETA(DisplayName = "Not Aiming"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "In Air"),

	EOS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SLIME_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	
	UShooterAnimInstance();

	virtual void NativeInitializeAnimation() override;
	void SetRecoilWeight();


protected:
	void TurnInPlace();

	/** Handle calculations for leaning while running. */
	void Lean(float DeltaTime);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	/** The speed of the character.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** Whether the character is in the air.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/** Whether the character is accelerating.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/** Offset yaw used for strafing.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/** Offset yaw the frame before character stopped moving.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	/** True if character is aiming.*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"));
	bool bAiming;

	/** Variables to support Turn in Place (TIP), only updated when standing still and not in air. */
	float TIPCharacterYaw;
	float TIPCharacterYawLastFrame;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	/** Rotation Curve value this frame. */
	float RotationCurve;

	/** Rotation Curve value last frame. */
	float RotationCurveLastFrame;

	/** Pitch of the aim rotation used for Aim Offset. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	/** True when reloading, true prevents Aim Offset. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn in Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim Offset State", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	/** Lean Variables to lean while running and turning. */
	FRotator CharacterRotation;
	FRotator CharacterRotationLastFrame;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	/** Is crouching?. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	/** Is equipping? */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	bool bEquipping;
	
	/** Change the weapon recoil (affects firing and reloading) based on turn-in-place and crouching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;

	/** True when turning in place. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;
};
