// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPSGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"


//////////////////////////////////////////////////////////////////////////
// ATPSGameCharacter

ATPSGameCharacter::ATPSGameCharacter()
{
	SetReplicates(true);
	SetReplicateMovement(true);
	
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

	SprintSpeedMultiplier = 2.0f;
	bSprintEnabled = false;
	bADSEnabled = false;
	bFireEnabled = false;
	bSwitchWeaponEnabled = false;
	MaxHealth = 100.0f;
	Health=MaxHealth;
	
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
	DOREPLIFETIME(ATPSGameCharacter,MaxHealth);
	DOREPLIFETIME(ATPSGameCharacter,Health);
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
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ATPSGameCharacter::CrouchBegin);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::CrouchEnd);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ATPSGameCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::StopSprint);

		//ADS
		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Triggered, this, &ATPSGameCharacter::StartADS);
		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::StopADS);

		//Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ATPSGameCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATPSGameCharacter::StopFire);
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
		if (bIsCrouched !=true && bFireEnabled !=true && bADSEnabled !=true)
		{
			GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
			bSprintEnabled = true;
		}
	}
}

void ATPSGameCharacter::StartADS_Implementation()
{
	//GEngine->AddOnScreenDebugMessage(-1, 25.0f, FColor::Yellow, TEXT("ADS_Implementation Has Been Called"));
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
	if(bSprintEnabled == true)
	{
		StopSprint();
		bFireEnabled = true;
	}
	else
	{
		bFireEnabled = true;
	}
}

void ATPSGameCharacter::StopFire_Implementation()
{
	bFireEnabled = false;
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

