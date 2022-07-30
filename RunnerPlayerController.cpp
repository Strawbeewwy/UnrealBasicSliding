// Fill out your copyright notice in the Description page of Project Settings.


#include "RunnerPlayerController.h"
#include "RunnerGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


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

}

void ARunnerPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
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
