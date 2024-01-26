// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HealthComponent.h"
#include "WeaponComponent.h"
#include "InputActionValue.h"
#include "TPSGameCharacter.generated.h"


UCLASS(config=Game)
class ATPSGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	
	/****************************New InputActions For Character*****************************************/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ADSAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadWeaponAction;

	/*************************************************************************************************/
	
public:
	ATPSGameCharacter();

	/********************************New Components for Character*************************************/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMeshComponent;

	FTimerHandle TimerHandle;
	
	/*************************************************************************************************/
	
	/***************************New Variables For Character*******************************************/

	UPROPERTY(Replicated,EditAnywhere,BlueprintReadWrite,Category="Sprint")
	float SprintSpeedMultiplier;

	UPROPERTY(Replicated,BlueprintReadWrite,Category="Sprint")
	bool bSprintEnabled;

	UPROPERTY(Replicated,BlueprintReadWrite,Category="ADS")
	bool bADSEnabled;

	UPROPERTY(Replicated,BlueprintReadWrite,Category="Fire")
	bool bFireEnabled;

	UPROPERTY(Replicated,BlueprintReadWrite,Category="Fire")
	bool bSwitchWeaponEnabled;

	UPROPERTY(Replicated,BlueprintReadWrite,Category="Fire")
	bool bReloadEnabled;
	
	/*************************************************************************************************/
	
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	/*******************************New Functions For Character***************************************/
	
	UFUNCTION(Server,Reliable)
	void StartSprint();

	UFUNCTION(Server,Reliable)
	void StopSprint();

	UFUNCTION(Server,Reliable)
	void StartADS();

	UFUNCTION(Server,Reliable)
	void StopADS();

	UFUNCTION(Server,Reliable)
	void StartFire();

	UFUNCTION(Server,Reliable)
	void StopFire();

	UFUNCTION(NetMulticast,Reliable)
	void FireMontageSoundEffect(FVector SoundLocation, FVector EffectLocation);

	UFUNCTION(Server,Reliable)
	void WeaponFire();

	UFUNCTION(NetMulticast,Reliable)
	void WeaponFireMulticast();
	
	UFUNCTION(Server,Reliable)
	void SwitchWeapon();

	UFUNCTION(NetMulticast,Reliable)
	void SwitchWeaponMulticast();

	UFUNCTION(Server,Reliable)
	void ReloadWeapon();

	UFUNCTION(NetMulticast,Reliable)
	void ReloadWeaponMulticast();

	UFUNCTION(NetMulticast,Reliable)
	void ReloadWeaponMontage();

	UFUNCTION(NetMulticast,Reliable)
	void SwitchWeaponMeshMontage(USkeletalMesh* NewWeaponMeshComponent);
	
	void CrouchBegin();
	
	void CrouchEnd();
	
	/*************************************************************************************************/

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Replication Function **/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

