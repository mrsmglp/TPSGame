// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	/*************************************************************************************************/
	
	CurrentActiveWeaponIndex = 0;
	bIsInitializedWeapons = false;
	
	/*************************************************************************************************/
}

// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponComponent, WeaponName);
	// Репликация только на владельце (клиент)
	DOREPLIFETIME_CONDITION(UWeaponComponent, WeaponMesh, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UWeaponComponent, WeaponAttachSocketName, COND_OwnerOnly);
	// Тут у всех остальных
	DOREPLIFETIME(UWeaponComponent, WeaponDamage);
	DOREPLIFETIME(UWeaponComponent, WeaponFireTime);
	DOREPLIFETIME(UWeaponComponent, bWeaponAutoFire);
	DOREPLIFETIME(UWeaponComponent, bIsPistol);
	DOREPLIFETIME(UWeaponComponent, bIsRifle);
	DOREPLIFETIME(UWeaponComponent, WeaponAmmoStockMax);
	DOREPLIFETIME(UWeaponComponent, WeaponAmmoClipMax);
	DOREPLIFETIME(UWeaponComponent, WeaponAmmoStock);
	DOREPLIFETIME(UWeaponComponent, WeaponAmmoClip);
	DOREPLIFETIME(UWeaponComponent, WeaponFireSound);
	DOREPLIFETIME(UWeaponComponent, WeaponFireEffect);
	DOREPLIFETIME(UWeaponComponent, WeaponMuzzleSocket);
	DOREPLIFETIME(UWeaponComponent, AllWeaponNames);
	DOREPLIFETIME(UWeaponComponent, WeaponEquipMontage);
	DOREPLIFETIME(UWeaponComponent, WeaponFireMontage);
	DOREPLIFETIME(UWeaponComponent, WeaponReloadMontage);
	DOREPLIFETIME(UWeaponComponent, CurrentActiveWeaponIndex);
	DOREPLIFETIME(UWeaponComponent, bIsInitializedWeapons);
}

void UWeaponComponent::InitializeAllWeapons(int32 ArrayIndex, FText NewName, USkeletalMesh* NewMesh, float NewDamage,
                                            float NewFireTime, bool bNewAutoFire,  bool bNewIsPistol, bool bNewIsRifle,
                                            int32 NewAmmoClip, int32 NewAmmoStock, USoundCue* NewFireSound, UParticleSystem* NewFireEffect,
                                            FName NewMuzzleSocket, UAnimMontage* NewEquipMontage, UAnimMontage* NewFireMontage, UAnimMontage* NewReloadMontage)
{
	WeaponName.Insert(NewName, ArrayIndex);
	WeaponMesh.Insert(NewMesh, ArrayIndex);
	WeaponDamage.Insert(NewDamage, ArrayIndex);
	WeaponFireTime.Insert(NewFireTime, ArrayIndex);
	bWeaponAutoFire.Insert(bNewAutoFire, ArrayIndex);
	bIsPistol.Insert(bNewIsPistol, ArrayIndex);
	bIsRifle.Insert(bNewIsRifle, ArrayIndex);
	WeaponAmmoStockMax.Insert(NewAmmoStock, ArrayIndex);
	WeaponAmmoStock.Insert(NewAmmoStock, ArrayIndex);
	WeaponAmmoClipMax.Insert(NewAmmoClip, ArrayIndex);
	WeaponAmmoClip.Insert(NewAmmoClip, ArrayIndex);
	WeaponFireSound.Insert(NewFireSound, ArrayIndex);
	WeaponFireEffect.Insert(NewFireEffect, ArrayIndex);
	WeaponMuzzleSocket.Insert(NewMuzzleSocket, ArrayIndex);
	WeaponEquipMontage.Insert(NewEquipMontage, ArrayIndex);
	WeaponFireMontage.Insert(NewFireMontage, ArrayIndex);
	WeaponReloadMontage.Insert(NewReloadMontage, ArrayIndex);
	bIsInitializedWeapons = true;
}

void UWeaponComponent::ReturnWeaponInfo(FText& NewName, USkeletalMesh*& NewMesh, float& NewDamage, float& NewFireTime,
	bool& bNewAutoFire, bool& bNewIsPistol, bool& bNewIsRifle, int32 &NewAmmoClip, int32& NewAmmoClipMax, int32 &NewAmmoStock, int32 &NewAmmoStockMax,
	USoundCue*& NewFireSound, UParticleSystem*& NewFireEffect, FName& NewMuzzleSocket, UAnimMontage*& NewEquipMontage,
	UAnimMontage*& NewFireMontage, UAnimMontage*& NewReloadMontage)
{
	NewName = WeaponName[CurrentActiveWeaponIndex];
	NewMesh = WeaponMesh[CurrentActiveWeaponIndex];
	NewDamage = WeaponDamage[CurrentActiveWeaponIndex];
	NewFireTime = WeaponFireTime[CurrentActiveWeaponIndex];
	bNewAutoFire = bWeaponAutoFire[CurrentActiveWeaponIndex];
	bNewIsPistol = bIsPistol[CurrentActiveWeaponIndex];
	bNewIsRifle = bIsRifle[CurrentActiveWeaponIndex];
	NewAmmoClip = WeaponAmmoClip[CurrentActiveWeaponIndex];
	NewAmmoClipMax = WeaponAmmoClipMax[CurrentActiveWeaponIndex];
	NewAmmoStock = WeaponAmmoStock[CurrentActiveWeaponIndex];
	NewAmmoStockMax = WeaponAmmoStockMax[CurrentActiveWeaponIndex];
	NewFireSound = WeaponFireSound[CurrentActiveWeaponIndex];
	NewFireEffect = WeaponFireEffect[CurrentActiveWeaponIndex];
	NewMuzzleSocket = WeaponMuzzleSocket[CurrentActiveWeaponIndex];
	NewEquipMontage = WeaponEquipMontage[CurrentActiveWeaponIndex];
	NewFireMontage = WeaponFireMontage[CurrentActiveWeaponIndex];
	NewReloadMontage = WeaponReloadMontage[CurrentActiveWeaponIndex];
}

void UWeaponComponent::WeaponReloadAmmo(int32 &CurrentAmmoClipMax, int32 &CurrentAmmoStock)
{
	int32 ReloadDif = WeaponAmmoClipMax[CurrentActiveWeaponIndex]-WeaponAmmoClip[CurrentActiveWeaponIndex];
	if(WeaponAmmoStock[CurrentActiveWeaponIndex] >= ReloadDif)
	{
		WeaponAmmoClip[CurrentActiveWeaponIndex] = WeaponAmmoClipMax[CurrentActiveWeaponIndex];
		WeaponAmmoStock[CurrentActiveWeaponIndex] -= ReloadDif;
		CurrentAmmoClipMax = WeaponAmmoClipMax[CurrentActiveWeaponIndex];
		CurrentAmmoStock = WeaponAmmoStock[CurrentActiveWeaponIndex];
		OnWeaponAmmoChanged.Broadcast(WeaponAmmoClip[CurrentActiveWeaponIndex], WeaponAmmoStock[CurrentActiveWeaponIndex]);
	}
	else
	{
		WeaponAmmoClip[CurrentActiveWeaponIndex] += WeaponAmmoStock[CurrentActiveWeaponIndex];
		WeaponAmmoStock[CurrentActiveWeaponIndex] = 0;
		CurrentAmmoClipMax = WeaponAmmoClipMax[CurrentActiveWeaponIndex];
		CurrentAmmoStock = WeaponAmmoStock[CurrentActiveWeaponIndex];
		OnWeaponAmmoChanged.Broadcast(WeaponAmmoClip[CurrentActiveWeaponIndex], WeaponAmmoStock[CurrentActiveWeaponIndex]);
	}
}

void UWeaponComponent::WeaponTakeBulletFromClip(int32 &NewClipAmmo)
{
	int32 NewClip = WeaponAmmoClip[CurrentActiveWeaponIndex] - 1;
	if (WeaponAmmoClip.IsValidIndex(CurrentActiveWeaponIndex))
	{
		WeaponAmmoClip[CurrentActiveWeaponIndex] = NewClip;
		NewClipAmmo = NewClip;
		OnWeaponAmmoChanged.Broadcast(WeaponAmmoClip[CurrentActiveWeaponIndex], WeaponAmmoStock[CurrentActiveWeaponIndex]);
	}
}

void UWeaponComponent::WeaponCheckAmmo(bool ReloadEnabled, bool SwitchEnabled, bool& bCanReload, bool& bCanFire)
{
	if(WeaponAmmoClip[CurrentActiveWeaponIndex] > 0 && !ReloadEnabled && !SwitchEnabled)
	{
		bCanFire = true;
	}
	else
	{
		bCanFire = false;
	}
	if (WeaponAmmoStock[CurrentActiveWeaponIndex] > 0 && WeaponAmmoClip[CurrentActiveWeaponIndex] < WeaponAmmoClipMax[CurrentActiveWeaponIndex] && !SwitchEnabled)
	{
		bCanReload = true;
	}
	else
	{
		bCanReload = false;
	}
}

void UWeaponComponent::SwitchWeapon()
{
	if(bIsInitializedWeapons == true)
	{
		if(CurrentActiveWeaponIndex == AllWeaponNames.Num() - 1)
		{
			CurrentActiveWeaponIndex = 0;
			OnWeaponNameChanged.Broadcast(WeaponName[CurrentActiveWeaponIndex]);
			OnWeaponDamageChanged.Broadcast(WeaponDamage[CurrentActiveWeaponIndex]);
			OnWeaponFireTimeChanged.Broadcast(WeaponFireTime[CurrentActiveWeaponIndex]);
			OnWeaponAmmoChanged.Broadcast(WeaponAmmoClip[CurrentActiveWeaponIndex], WeaponAmmoStock[CurrentActiveWeaponIndex]);
		}
		else
		{
			CurrentActiveWeaponIndex++;
			OnWeaponNameChanged.Broadcast(WeaponName[CurrentActiveWeaponIndex]);
			OnWeaponDamageChanged.Broadcast(WeaponDamage[CurrentActiveWeaponIndex]);
			OnWeaponFireTimeChanged.Broadcast(WeaponFireTime[CurrentActiveWeaponIndex]);
			OnWeaponAmmoChanged.Broadcast(WeaponAmmoClip[CurrentActiveWeaponIndex], WeaponAmmoStock[CurrentActiveWeaponIndex]);
		}
	}
}

