// Fill out your copyright notice in the Description page of Project Settings.


#include "RunnerPlayerController.h"
#include "RunnerGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include "Item.h"
#include "Components/CapsuleComponent.h"
#include "PickUp.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "RunnerHUD.h"
#include "VaultingComponent.h"


ARunnerPlayerController::ARunnerPlayerController()
{
	// 
	// CAMERA CONTROL
	//
	/*set our turn rates for input*/
	BaseTurnRate = 25.f;
	BaseLookUpRate = 25.f;
	PlayerPerspective = EPlayerPerspective::IR_ThirdPerson;

	// 
	// WALKING
	//
	/*setup the walk speed*/
	WalkSpeed = 600.0f;
	WalkingGroundFriction = 8.0f;
	WalkingBrakingDecelerationWalking = 2048.0f;
	WalkingBrakingFrictionFactor = 2.0f;

	// 
	// CROUCH
	//
	CrouchSpeed = WalkSpeed / 2;

	// 
	// SPRINTING
	//
	/*setup the sprint speed*/
	SprintSpeed = WalkSpeed * 2.0f;
	/*allow the player to sprint*/
	bCanSprint = true;

	// 
	// INTERACTION
	//
	/*setup the interaction properties*/
	InteractionCheckFrequency = 0.f;
	InteractionCheckDistance = 1000.f;

	//
	// MOVEMENT STATE
	//
	MovementState = EMovementState::IR_Walking;

	//
	// SLIDING
	//
	SlidingGroundFriction = 0.0f;
	SlidingBrakingDecelerationWalking = 1024.0f;
	SlideSpeed = SprintSpeed * 2.0f;
	SlideMultiplier = 150000;

	//
	// DASHING
	//
	DashDistance = 6000.0f;
	DashCoolDown = 1.0f;
	DashExecTime = 0.1f;
	DashBrakingFrictionFactor = 0.f;
}

void ARunnerPlayerController::BeginPlay()
{
	Super::BeginPlay();


	// 
	// WALKING
	//
	/*Setting up Max Walk Speed of movement component based on the controller*/
	GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// 
	// CROUCH
	//
	/*enable crouching*/
	GetCharacter()->GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	/*Setting up Max Walk Speed while crouched of movement component based on the controller*/
	GetCharacter()->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	/*Getting the capsule half height*/
	StandingCapsuleHalfHeight = GetCharacter()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	//
	// HUD
	//
	/*Getting a reference of the heads up display*/
	RunnerHUD = Cast<ARunnerHUD>(GetHUD());
}

void ARunnerPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}

	if ((MovementState == EMovementState::IR_Crouching || MovementState == EMovementState::IR_Sliding) && CanStand() && !bCrouching)
	{
		ResolveMovementState();
	}
}

void ARunnerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		// 
		// MOVEMENT
		//
		InputComponent->BindAxis("MoveForward", this, &ARunnerPlayerController::MoveForward);
		InputComponent->BindAxis("MoveRight", this, &ARunnerPlayerController::MoveRight);

		// 
		// CAMERA CONTROL
		//
		InputComponent->BindAxis("TurnRate", this, &ARunnerPlayerController::TurnRate);
		InputComponent->BindAxis("LookUpRate", this, &ARunnerPlayerController::LookUpRate);
		InputComponent->BindAction("SwitchPlayerPerspective", IE_Pressed, this, &ARunnerPlayerController::SwitchPlayerPerspective);

		// 
		// JUMP
		//
		InputComponent->BindAction("Jump", IE_Pressed, this, &ARunnerPlayerController::StartJumping);
		InputComponent->BindAction("Jump", IE_Released, this, &ARunnerPlayerController::StopJumping);

		// 
		// CROUCH
		// 
		InputComponent->BindAction("Crouch", IE_Pressed, this, &ARunnerPlayerController::StartCrouching);
		InputComponent->BindAction("Crouch", IE_Released, this, &ARunnerPlayerController::StopCrouching);

		// 
		// SPRINTING
		// 
		InputComponent->BindAction("Sprint", IE_Pressed, this, &ARunnerPlayerController::StartSprinting);
		InputComponent->BindAction("Sprint", IE_Released, this, &ARunnerPlayerController::StopSprinting);

		// 
		// DASHING
		// 
		InputComponent->BindAction("Dash", IE_Pressed, this, &ARunnerPlayerController::StartDashing);

		// 
		// INTERACTION
		// 
		InputComponent->BindAction("Interact", IE_Pressed, this, &ARunnerPlayerController::BeginInteract);
		InputComponent->BindAction("Interact", IE_Released, this, &ARunnerPlayerController::EndInteract);

	}
}

void ARunnerPlayerController::TurnRate(float Rate)
{
	if (Rate != 0.0f && GetCharacter() != nullptr)
	{
		/*turn camera at rate on yaw X axis*/
		GetCharacter()->AddControllerYawInput(Rate);
	}
}

void ARunnerPlayerController::LookUpRate(float Rate)
{
	if (Rate != 0.0f && GetCharacter() != nullptr)
	{
		/*turn camera at rate on pitch Y axis*/
		GetCharacter()->AddControllerPitchInput(Rate);
	}
}

void ARunnerPlayerController::MoveForward(float Value)
{
	if (Value != 0.0f && GetCharacter() != nullptr)
	{
		/*add movement in that direction*/
		GetCharacter()->AddMovementInput(GetCharacter()->GetActorForwardVector(), Value);
	}
}

void ARunnerPlayerController::MoveRight(float Value)
{
	if (Value != 0.0f && GetCharacter() != nullptr)
	{
		/*add movement in that direction*/
		GetCharacter()->AddMovementInput(GetCharacter()->GetActorRightVector(), Value);
	}
}

bool ARunnerPlayerController::CanSprint()
{
	if (!bSprinting)
	{
		return false;
	}

	if (!CanStand() || GetCharacter()->GetCharacterMovement()->IsFalling())
	{
		return false;
	}

	return true;
}

void ARunnerPlayerController::StartSprinting()
{
	SetSprinting(true);
	OnStartSprinting.Broadcast(Cast<ARunnerGameCharacter>(GetCharacter()));
}

void ARunnerPlayerController::StopSprinting()
{
	SetSprinting(false);
	OnStopSprinting.Broadcast(Cast<ARunnerGameCharacter>(GetCharacter()));
}

void ARunnerPlayerController::SetSprinting(const bool bNewSprinting)
{
	if (bNewSprinting == bSprinting)
	{
		return;
	}

	bSprinting = bNewSprinting;

	if (MovementState == EMovementState::IR_Sprinting && !bSprinting)
	{
		ResolveMovementState();
	}
	
	if ((MovementState == EMovementState::IR_Walking || MovementState == EMovementState::IR_Crouching) && bSprinting)
	{
		SetMovementState(EMovementState::IR_Sprinting);
	}
}

void ARunnerPlayerController::StartCrouching()
{
	SetCrouching(true);
}

void ARunnerPlayerController::StopCrouching()
{
	SetCrouching(false);
}

void ARunnerPlayerController::SetCrouching(bool bNewCrouching)
{
	if (bNewCrouching == bCrouching)
	{
		return;
	}
	bCrouching = bNewCrouching;

	if (MovementState == EMovementState::IR_Crouching && !bCrouching)
	{
		ResolveMovementState();
	}

	if (MovementState == EMovementState::IR_Walking && bCrouching)
	{
		SetMovementState(EMovementState::IR_Crouching);
	}

	if (MovementState == EMovementState::IR_Sprinting && bCrouching)
	{
		SetMovementState(EMovementState::IR_Sliding);
	}
}

bool ARunnerPlayerController::CanStand()
{
	if (bCrouching)
	{
		return false;
	}

	//initialization for the Line trace
	FVector TraceStart = GetCharacter()->GetActorLocation();
	TraceStart.Z -= GetCharacter()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector TraceEnd = TraceStart;
	TraceEnd.Z +=  StandingCapsuleHalfHeight * 2;

	FHitResult TraceHit;

	//we need to tell the line trace to ignore collision with the player
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetCharacter());

	//if we hit something we cant stand up
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	return true;
}

void ARunnerPlayerController::StartJumping()
{
	if (GetCharacter() == nullptr)
	{
		return;
	}

	if (Cast<ARunnerGameCharacter>(GetCharacter())->VaultingComponent->CanVault())
	{
		Vault();
	}
	
	GetCharacter()->Jump();
}

void ARunnerPlayerController::StopJumping()
{
	if (GetCharacter() == nullptr)
	{
		return;
	}

	GetCharacter()->StopJumping();

}

void ARunnerPlayerController::StartDashing()
{
	SetDashing(true);
	OnStartDashing.Broadcast(Cast<ARunnerGameCharacter>(GetCharacter()));
}

void ARunnerPlayerController::StopDashing()
{
	GetCharacter()->GetMovementComponent()->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(TimerHandle_Dashing, this, &ARunnerPlayerController::ResetDash, DashCoolDown, false);
	GetCharacter()->GetCharacterMovement()->BrakingFrictionFactor = WalkingBrakingFrictionFactor;
	OnStopDashing.Broadcast(Cast<ARunnerGameCharacter>(GetCharacter()));
}

void ARunnerPlayerController::SetDashing(bool bNewDashing)
{
	if (GetCharacter() == nullptr)
	{
		return;
	}

	if (bNewDashing == bDashing)
	{
		return;
	}

	if (!CanDash())
	{
		return;
	}

	bDashing = bNewDashing;

	GetCharacter()->GetCharacterMovement()->BrakingFrictionFactor = DashBrakingFrictionFactor;
	FVector DashVector = GetCharacter()->GetActorForwardVector();
	DashVector.Z = 0;
	DashVector.Normalize();
	GetCharacter()->LaunchCharacter(DashVector * DashDistance, true, true);
	GetWorldTimerManager().SetTimer(TimerHandle_Dashing, this, &ARunnerPlayerController::StopDashing, DashExecTime, false);

}

void ARunnerPlayerController::ResetDash()
{
	bDashing = false;
}

bool ARunnerPlayerController::CanDash()
{
	if (bCrouching || bDashing)
	{
		return false;
	}

	return true;
}

void ARunnerPlayerController::SwitchPlayerPerspective()
{
	if (GetCharacter())
 	{
		const float SmoothTimeBlend = 0.75f;
			 
		if (PlayerPerspective == EPlayerPerspective::IR_FirstPerson)
		{
			Cast<ARunnerGameCharacter>(GetCharacter())->FirstPersonCameraComponent->Deactivate();
			Cast<ARunnerGameCharacter>(GetCharacter())->ThirdPersonCameraComponent->Activate();
			PlayerPerspective = EPlayerPerspective::IR_ThirdPerson;
			InteractionCheckDistance += Cast<ARunnerGameCharacter>(GetCharacter())->ThirdPersonSpringArmComponent->TargetArmLength;
		}
		else if (PlayerPerspective == EPlayerPerspective::IR_ThirdPerson)
		{
			Cast<ARunnerGameCharacter>(GetCharacter())->ThirdPersonCameraComponent->Deactivate();
			Cast<ARunnerGameCharacter>(GetCharacter())->FirstPersonCameraComponent->Activate();
			PlayerPerspective = EPlayerPerspective::IR_FirstPerson;
			InteractionCheckDistance -= Cast<ARunnerGameCharacter>(GetCharacter())->ThirdPersonSpringArmComponent->TargetArmLength;
		}
	}
}

void ARunnerPlayerController::StartSliding()
{
	FHitResult FloorHitResult;
	FloorHitResult = GetCharacter()->GetCharacterMovement()->CurrentFloor.HitResult;
	FVector SlideForce  = CalculateFloorInfluence(FloorHitResult.Normal);

	SlideForce *= SlideMultiplier;

	GetCharacter()->GetCharacterMovement()->AddForce(SlideForce);

	if (GetCharacter()->GetVelocity().Size() > SlideSpeed)
	{
		GetCharacter()->GetCharacterMovement()->Velocity = FVector(GetCharacter()->GetVelocity().Normalize() * (SlideSpeed));
	}
	else if (GetCharacter()->GetVelocity().Size() < CrouchSpeed)
	{
		StopSliding();
	}

	OnStartSliding.Broadcast(Cast<ARunnerGameCharacter>(GetCharacter()));
}

void ARunnerPlayerController::StopSliding()
{
	ResolveMovementState();
	GetCharacter()->GetCharacterMovement()->GroundFriction = WalkingGroundFriction;
	GetCharacter()->GetCharacterMovement()->BrakingDecelerationWalking = WalkingBrakingDecelerationWalking;
	OnStopSliding.Broadcast(Cast<ARunnerGameCharacter>(GetCharacter()));
}

FVector ARunnerPlayerController::CalculateFloorInfluence(FVector FloorNormal)
{
	if (FloorNormal == FVector::UpVector)
	{
		return FVector::ZeroVector;
	}

	FVector ResultingVector = FVector::CrossProduct(FloorNormal, FVector::CrossProduct(FloorNormal, FVector::UpVector));
	ResultingVector.Normalize();

	return ResultingVector;
}

void ARunnerPlayerController::ResolveMovementState()
{
	if ((!CanStand() && !CanSprint()) || bCrouching)
	{
		SetMovementState(EMovementState::IR_Crouching);
	}
	else if (!CanSprint())
	{
		SetMovementState(EMovementState::IR_Walking);
	}
	else
	{
		SetMovementState(EMovementState::IR_Sprinting);
	}
}

void ARunnerPlayerController::SetMovementState(EMovementState NewMovementState)
{
	if (MovementState == NewMovementState)
	{
		return;
	}

	EMovementState PreviousMovementState = MovementState;
	MovementState = NewMovementState;
	OnMovementStateChange(PreviousMovementState);
}

void ARunnerPlayerController::OnMovementStateChange(EMovementState PreviousMovementState)
{
	if (PreviousMovementState == EMovementState::IR_Sliding)
	{
		StopSliding();
	}

	switch (MovementState)
	{
	case EMovementState::IR_Sprinting:
	{
		GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		StopCrouching();
		GetCharacter()->UnCrouch();
		break;
	}
	case EMovementState::IR_Crouching:
	{
		GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		GetCharacter()->Crouch();
		break;
	}
	case EMovementState::IR_Walking:
	{
		GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		StopCrouching();
		GetCharacter()->UnCrouch();
		break;
	}
	case EMovementState::IR_Sliding:
	{
		GetCharacter()->Crouch();
		GetCharacter()->GetCharacterMovement()->Velocity = GetCharacter()->GetActorForwardVector() * SprintSpeed;
		GetCharacter()->GetCharacterMovement()->GroundFriction = SlidingGroundFriction;
		GetCharacter()->GetCharacterMovement()->BrakingDecelerationWalking = SlidingBrakingDecelerationWalking;
		StartSliding();
		break;
	}
	default:
	{
		break;
	}
	}
}

void ARunnerPlayerController::Vault()
{
	ARunnerGameCharacter* CastedCharacter = Cast<ARunnerGameCharacter>(GetCharacter());
	CastedCharacter->VaultingComponent->VaultInternal();
	OnVaulting.Broadcast(CastedCharacter);
}

void ARunnerPlayerController::PerformInteractionCheck()
{
	//updates the last time when tried to seek for an interactable
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	//since it's FPS we want the line trace to be done where the players looks at
	FVector EyesLocation;
	FRotator EyesRotation;
	GetPlayerViewPoint(EyesLocation, EyesRotation);

	//initialization for the Line trace
	FVector TraceStart = EyesLocation;
	FVector TraceEnd = (EyesRotation.Vector() * InteractionCheckDistance) + TraceStart;
	FHitResult TraceHit;

	//we need to tell the line trace to ignore collision with the player
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetCharacter());

	//if we hit something
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		//Check if we hit an Actor
		if (TraceHit.GetActor())
		{
			//if the actor we hit has an InteractionComponent
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size();
				//checks if the distance we are is under the maximum distance to Interact With the object and check if it is not the same actor
				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
				{
					FoundNewInteractable(InteractionComponent);
				}
				else if (GetInteractable() && Distance > InteractionComponent->InteractionDistance) //if we are too far away, or looking at the same actor
				{
					CouldntFindInteractable();
				}

				return;
			}
		}
	}
	CouldntFindInteractable();
}

void ARunnerPlayerController::CouldntFindInteractable()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(Cast<ARunnerGameCharacter>(GetCharacter()));

		if (InteractionData.bInteractHeld)
		{
			EndInteract();
		}
	}
	InteractionData.ViewedInteractionComponent = nullptr;
}

void ARunnerPlayerController::FoundNewInteractable(UInteractionComponent* Interactable)
{
	EndInteract();

	if (UInteractionComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(Cast<ARunnerGameCharacter>(GetCharacter()));
	}

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(Cast<ARunnerGameCharacter>(GetCharacter()));
}

void ARunnerPlayerController::BeginInteract()
{

	InteractionData.bInteractHeld = true;

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->BeginInteract(Cast<ARunnerGameCharacter>(GetCharacter()));

		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{
			Interact();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ARunnerPlayerController::Interact, Interactable->InteractionTime, false);
		}
	}
}

void ARunnerPlayerController::EndInteract()
{

	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndInteract(Cast<ARunnerGameCharacter>(GetCharacter()));
	}

}

void ARunnerPlayerController::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->Interact(Cast<ARunnerGameCharacter>(GetCharacter()));
	}
}

bool ARunnerPlayerController::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float ARunnerPlayerController::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void ARunnerPlayerController::UseItem(class UItem* Item)
{
	if (Cast<ARunnerGameCharacter>(GetCharacter())->PlayerInventory && Cast<ARunnerGameCharacter>(GetCharacter())->PlayerInventory->FindItem(Item))
	{//keeps the player from cheating by telling the server to use an item they don t own
		return;
	}
	
	if (Item)
	{
		Item->Use(Cast<ARunnerGameCharacter>(GetCharacter()));
	}
}

void ARunnerPlayerController::DropItem(class UItem* Item, const int32 Quantity)
{
	if (Item && Cast<ARunnerGameCharacter>(GetCharacter())->PlayerInventory && Cast<ARunnerGameCharacter>(GetCharacter())->PlayerInventory->FindItem(Item))
	{

		const int32 ItemQuantity = Item->GetQuantity();
		const int32 DroppedQuantity = Cast<ARunnerGameCharacter>(GetCharacter())->PlayerInventory->ConsumeItem(Item, Quantity);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = NULL;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector PickupSpawnLocation = GetCharacter()->GetActorLocation();
		PickupSpawnLocation.Z -= GetCharacter()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FTransform PickupSpawnTransform(GetCharacter()->GetActorRotation(), PickupSpawnLocation);

		APickUp* PickupToSpawn = GetWorld()->SpawnActor<APickUp>(PickupClass, PickupSpawnTransform, SpawnParams);
		PickupToSpawn->InitializePickup(Item->GetClass(), DroppedQuantity);
		
	}
}