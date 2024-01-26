// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "WeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponNameChangedSignature, const FText&, NewWeaponName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponDamageChangedSignature, float, NewWeaponDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponFireTimeChangedSignature, float, NewWeaponFireTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedSignature, int32, NewClipAmmo, int32, NewStockAmmo);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSGAME_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

	/*************************************************************************************************/
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FText> WeaponName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<USkeletalMesh*> WeaponMesh;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FName> WeaponAttachSocketName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<float> WeaponDamage;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<float> WeaponFireTime;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<bool> bWeaponAutoFire;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<bool> bIsPistol;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<bool> bIsRifle;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Weapon")
	TArray<int32> WeaponAmmoStockMax;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Weapon")
	TArray<int32> WeaponAmmoClipMax;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Weapon")
	TArray<int32> WeaponAmmoStock;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Weapon")
	TArray<int32> WeaponAmmoClip;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<USoundCue*> WeaponFireSound;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UParticleSystem*> WeaponFireEffect;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FName> WeaponMuzzleSocket;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> WeaponEquipMontage;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> WeaponFireMontage;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> WeaponReloadMontage;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Weapon")
	TArray<FName> AllWeaponNames;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentActiveWeaponIndex;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Weapon")
	bool bIsInitializedWeapons;

	/*************************************************************************************************/
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

public:

	/*************************************************************************************************/
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void InitializeAllWeapons(int32 ArrayIndex, FText NewName, USkeletalMesh* NewMesh, float NewDamage,
		float NewFireTime, bool bNewAutoFire, bool bNewIsPistol, bool bNewIsRifle,
		int32 NewAmmoClip, int32 NewAmmoStock, USoundCue* NewFireSound, UParticleSystem* NewFireEffect,
		FName NewMuzzleSocket, UAnimMontage* NewEquipMontage, UAnimMontage* NewFireMontage, UAnimMontage* NewReloadMontage);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ReturnWeaponInfo(FText &NewName, USkeletalMesh* &NewMesh, float &NewDamage,
		float &NewFireTime, bool &bNewAutoFire, bool &bNewIsPistol, bool &bNewIsRifle,
		int32 &NewAmmoClip, int32& NewAmmoClipMax, int32 &NewAmmoStock, int32 &NewAmmoStockMax,
		USoundCue* &NewFireSound, UParticleSystem* &NewFireEffect,
		FName &NewMuzzleSocket, UAnimMontage* &NewEquipMontage, UAnimMontage* &NewFireMontage, UAnimMontage* &NewReloadMontage);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void WeaponReloadAmmo(int32 &CurrentAmmoClipMax, int32 &CurrentAmmoStock);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void WeaponTakeBulletFromClip(int32 &NewClipAmmo);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void WeaponCheckAmmo(bool ReloadEnabled, bool SwitchEnabled, bool &bCanReload, bool &bCanFire);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SwitchWeapon();

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FWeaponNameChangedSignature OnWeaponNameChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FWeaponDamageChangedSignature OnWeaponDamageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FWeaponFireTimeChangedSignature OnWeaponFireTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FWeaponAmmoChangedSignature OnWeaponAmmoChanged;

	/*************************************************************************************************/
};

