// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "HealthComponent.h"
#include "WeaponComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


//////////////////////////////////////////////////////////////////////////
// ATPSGameCharacter

ATPSGameCharacter::ATPSGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	/*************************************************************************************************/

	SetReplicates(true);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComponent->SetupAttachment(GetMesh(), "weapon_r");
	WeaponMeshComponent->SetVisibility(true);
	WeaponMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	SprintSpeedMultiplier = 2.0f;
	bSprintEnabled = false;
	bADSEnabled = false;
	bFireEnabled = false;
	bSwitchWeaponEnabled = false;
	
	/*************************************************************************************************/
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATPSGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	/*************************************************************************************************/
	
	SetReplicateMovement(true);
	HealthComponent->SetIsReplicated(true);
	WeaponComponent->SetIsReplicated(true);

	/*************************************************************************************************/
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATPSGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATPSGameCharacter,SprintSpeedMultiplier);
	DOREPLIFETIME(ATPSGameCharacter,bSprintEnabled);
	DOREPLIFETIME(ATPSGameCharacter,bADSEnabled);
	DOREPLIFETIME(ATPSGameCharacter,bFireEnabled);
	DOREPLIFETIME(ATPSGameCharacter,bSwitchWeaponEnabled);
	DOREPLIFETIME(ATPSGameCharacter,bReloadEnabled);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATPSGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPSGameCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPSGameCharacter::Look);

		//Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATPSGameCharacter::CrouchBegin);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::CrouchEnd);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATPSGameCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::StopSprint);

		//ADS
		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Started, this, &ATPSGameCharacter::StartADS);
		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::StopADS);

		//Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATPSGameCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::StopFire);

		//Switch Weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &ATPSGameCharacter::SwitchWeapon);

		//Reload Weapon
		EnhancedInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this, &ATPSGameCharacter::ReloadWeapon);
	}

}

void ATPSGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATPSGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATPSGameCharacter::StopSprint_Implementation()
{
	if (GetCharacterMovement()->MaxWalkSpeed >400)
	{
		GetCharacterMovement()->MaxWalkSpeed /= SprintSpeedMultiplier;
		bSprintEnabled = false;
	}
}

void ATPSGameCharacter::StartSprint_Implementation()
{
	if (GetCharacterMovement()->MaxWalkSpeed <800)
	{
		if (bIsCrouched == false && bFireEnabled == false && bADSEnabled == false)
		{
			GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
			bSprintEnabled = true;
		}
	}
}

void ATPSGameCharacter::StartADS_Implementation()
{
	if(bSprintEnabled == true)
	{
		StopSprint();
		bADSEnabled = true;
	}
	else
	{
		bADSEnabled = true;
	}
}

void ATPSGameCharacter::StopADS_Implementation()
{
	bADSEnabled = false;
}

void ATPSGameCharacter::StartFire_Implementation()
{
	if(bFireEnabled == false && bSwitchWeaponEnabled == false && bReloadEnabled == false && WeaponComponent->bIsInitializedWeapons == true)
	{
		bool bCanReload, bCanFire;
		WeaponComponent->WeaponCheckAmmo(bReloadEnabled, bSwitchWeaponEnabled, bCanReload, bCanFire);
		if(bCanFire == true)
		{
			bFireEnabled = true;
			WeaponFire();
		}
	}
}

void ATPSGameCharacter::StopFire_Implementation()
{
	bFireEnabled = false;
}

void ATPSGameCharacter::WeaponFire_Implementation()
{
	if(bFireEnabled)
	{
		WeaponFireMulticast();
		FVector Start = FollowCamera->GetComponentLocation() + CameraBoom->TargetArmLength * FollowCamera->GetForwardVector() + 50 * FollowCamera->GetForwardVector();
		FVector End = FollowCamera->GetForwardVector() * 10000 + FollowCamera->GetComponentLocation();
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel1);
		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(GetWorld(), Start, HitResult.Location, FColor::Yellow, true, 5, 0, 1);
			DrawDebugPoint(GetWorld(), HitResult.Location, 5, FColor::Red, false, 5);
			UGameplayStatics::ApplyDamage(HitResult.GetActor(), WeaponComponent->WeaponDamage[WeaponComponent->CurrentActiveWeaponIndex], nullptr, this, UDamageType::StaticClass());
			FireMontageSoundEffect(Start,HitResult.Location);
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, HitResult.TraceEnd, FColor::Yellow, true, 5, 0, 1);
			FireMontageSoundEffect(Start,HitResult.TraceEnd);
		}
	}
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
		bool bCanReload, bCanFire;
		WeaponComponent->WeaponCheckAmmo(bReloadEnabled, bSwitchWeaponEnabled, bCanReload, bCanFire);
		if(bCanFire == true && WeaponComponent->bWeaponAutoFire[WeaponComponent->CurrentActiveWeaponIndex] == true)
		{
			WeaponFire();
		}
		}, WeaponComponent->WeaponFireTime[WeaponComponent->CurrentActiveWeaponIndex], false);
}

void ATPSGameCharacter::WeaponFireMulticast_Implementation()
{
	int32 NewClip;
	WeaponComponent->WeaponTakeBulletFromClip(NewClip);
}

void ATPSGameCharacter::FireMontageSoundEffect_Implementation(FVector SoundLocation, FVector EffectLocation)
{
	const USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	AnimInstance->Montage_Play(WeaponComponent->WeaponFireMontage[WeaponComponent->CurrentActiveWeaponIndex],1,EMontagePlayReturnType::MontageLength,0,false);
	if (WeaponComponent->WeaponFireSound[WeaponComponent->CurrentActiveWeaponIndex])
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponComponent->WeaponFireSound[WeaponComponent->CurrentActiveWeaponIndex], SoundLocation);
	}
	if (WeaponComponent->WeaponFireEffect[WeaponComponent->CurrentActiveWeaponIndex])
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponComponent->WeaponFireEffect[WeaponComponent->CurrentActiveWeaponIndex], EffectLocation);
	}
}


void ATPSGameCharacter::SwitchWeapon_Implementation()
{
	if(bFireEnabled == false && bSwitchWeaponEnabled == false && bReloadEnabled == false && WeaponComponent->bIsInitializedWeapons == true)
	{
		bSwitchWeaponEnabled = true;
		SwitchWeaponMulticast();
		SwitchWeaponMeshMontage(WeaponComponent->WeaponMesh[WeaponComponent->CurrentActiveWeaponIndex]);
	}
}

void ATPSGameCharacter::SwitchWeaponMulticast_Implementation()
{
	WeaponComponent->SwitchWeapon();
}

void ATPSGameCharacter::SwitchWeaponMeshMontage_Implementation(USkeletalMesh* NewWeaponMeshComponent)
{
	const USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	AnimInstance->Montage_Play(WeaponComponent->WeaponEquipMontage[WeaponComponent->CurrentActiveWeaponIndex],1,EMontagePlayReturnType::MontageLength,0,false);
	GetWorldTimerManager().SetTimer(TimerHandle, [this, NewWeaponMeshComponent]()
	{
		// Вызываем функцию установки меша оружия через 0.5 секунд
		WeaponMeshComponent->SetSkeletalMesh(NewWeaponMeshComponent);
		bSwitchWeaponEnabled = false;
	}, 0.5f, false);
}

void ATPSGameCharacter::ReloadWeapon_Implementation()
{
	if(bFireEnabled == false && bSwitchWeaponEnabled == false && bReloadEnabled == false && WeaponComponent->bIsInitializedWeapons == true)
	{
		bool bCanReload, bCanFire;
		WeaponComponent->WeaponCheckAmmo(bReloadEnabled, bSwitchWeaponEnabled, bCanReload, bCanFire);
		if (bCanReload == true)
		{
			bReloadEnabled = true;
			ReloadWeaponMontage();
			GetWorldTimerManager().SetTimer(TimerHandle, [this]()
				{
				// Вызываем функцию WeaponReloadAmmo через 1 секунду
				ReloadWeaponMulticast();
				bReloadEnabled = false;
				}, 2.0f, false);
		}
	}
}

void ATPSGameCharacter::ReloadWeaponMontage_Implementation()
{
	const USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	AnimInstance->Montage_Play(WeaponComponent->WeaponReloadMontage[WeaponComponent->CurrentActiveWeaponIndex],1,EMontagePlayReturnType::MontageLength,0,false);
}

void ATPSGameCharacter::ReloadWeaponMulticast_Implementation()
{
	int32 CurrentAmmoClipMax, CurrentAmmoStock;
	WeaponComponent->WeaponReloadAmmo(CurrentAmmoClipMax, CurrentAmmoStock);
}


void ATPSGameCharacter::CrouchBegin()
{
	if (bSprintEnabled == true)
	{
		StopSprint();
		if(!GetCharacterMovement()->IsCrouching())
		{
			GetCharacterMovement()->bWantsToCrouch = true;
			GetCharacterMovement()->Crouch();
		}
	}
	else
	{
		if(!GetCharacterMovement()->IsCrouching())
		{
			GetCharacterMovement()->bWantsToCrouch = true;
			GetCharacterMovement()->Crouch();
		}
	}
}

void ATPSGameCharacter::CrouchEnd()
{
	if(GetCharacterMovement()->IsCrouching())
	{
		GetCharacterMovement()->bWantsToCrouch = false;
		GetCharacterMovement()->UnCrouch();
	}
}

