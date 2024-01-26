// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	/*************************************************************************************************/
	
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	bIsDead = false;

	/*************************************************************************************************/
}

void UHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth-Damage,0.0f,MaxHealth);
	if(Damage<=0)
	{
		return;
	}
	if (bIsDead)
	{
		return;
	}
	if (CurrentHealth <= 0)
	{
		bIsDead = true;
		OnPlayerSpawn.Broadcast();
		OnPlayerDeath.Broadcast(DamageCauser);
	}
}

void UHealthComponent::OnRep_CurrentHealth(float OldHealth)
{
	OnHealthChanged.Broadcast(CurrentHealth);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	/*************************************************************************************************/
	
	AActor* MyOwner = GetOwner();
	if (MyOwner) {
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeDamage);
	}

	/*************************************************************************************************/
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, MaxHealth);
	DOREPLIFETIME(UHealthComponent, bIsDead);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, DeathMontages);
}

