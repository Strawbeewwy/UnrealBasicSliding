// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RunnerPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartSliding, class ARunnerGameCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartSprinting, class ARunnerGameCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStopSliding, class ARunnerGameCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStopSprinting, class ARunnerGameCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartDashing, class ARunnerGameCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStopDashing, class ARunnerGameCharacter*, Character);

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	IR_Walking UMETA(DisplayName = "Walking"),
	IR_Sprinting UMETA(DisplayName = "Sprinting"),
	IR_Crouching UMETA(DisplayName = "Crouching"),
	IR_Sliding UMETA(DisplayName = "Sliding")
};

UCLASS()
class RUNNERGAME_API ARunnerPlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:

	ARunnerPlayerController();

protected:
	// 
	// CAMERA CONTROL
	//
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseLookUpRate;

	//
	// MovementState 
	// 
	/*Reference of the current movement state of this controller*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|MovementState")
	EMovementState MovementState;

	//
	// CROUCHING
	//
	/*tells us if the controller is crouching*/
	UPROPERTY(VisibleAnywhere, Category = "Movement|Crouching")
	bool bCrouching;
	/*the speed at which the controller moves while crouched*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Crouching")
	float CrouchSpeed;
	/*Reference to the capsule half height: used in runtime calculations*/
	UPROPERTY(VisibleAnywhere, Category = "Movement|Crouching")
	float StandingCapsuleHalfHeight;


	// 
	// WAlKING
	//
	/*the speed at which this controller moves*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
	float WalkSpeed;
	/*the friction of the ground. higher friction means more force is needed to move this controller*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
	float WalkingGroundFriction;
	/*Deceleration factor of the controller movement higher factor means more abrupt stop*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
	float WalkingBrakingDecelerationFactor;
	/*the braking friction. higher friction means the controller will stop moving more abruptly*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
	float WalkingBrakingFrictionFactor;

	// 
	// SPRINTING
	//
	/*the speed at which this controller moves while sprinting*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sprinting")
	float SprintSpeed;
	/*tells us if the controller is sprinting*/
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprinting")
	bool bSprinting;
	/*tells us if the controller is can sprint ie: cant sprint in water higher than capsule half height*/
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprinting")
	bool bCanSprint;
	
	/*Delegate Where we can do stuff at the start of the sprint ie: change FOV like minecraft
	this delegates is to implement in blueprints*/
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStartSprinting OnStartSprinting;
	/*Delegate Where we can do stuff at the end of the sprint ie: change FOV back like minecraft
	this delegates is to implement in blueprints*/
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStopSprinting OnStopSprinting;

	//
	// SLIDING 
	//
	/*Delegate Where we can do stuff at the start of the sliding
	this delegates is to implement in blueprints*/
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStartSliding OnStartSliding;
	/*Delegate Where we can do stuff at the end of the sliding
	this delegates is to implement in blueprints*/
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStopSliding OnStopSliding;
	/*the speed at which this controller moves while sliding*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlideSpeed;
	/*the friction of the ground. higher friction means more force is needed to move this controller*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlidingGroundFriction;
	/*Deceleration factor of the controller movement higher factor means more abrupt stop*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlidingBrakingDecelerationFactor;
	/*Gain or lost of speed while sliding ie: higher would me a gain in speed therefore a sliding like Apex Legends*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlideMultiplier;

	//
	// DASHING
	//
	/*the maximum distance of the Dash*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
	float DashDistance;
	/*The cooldown of the Dash*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
	float DashCoolDown;
	/*Time it takes for the Dash to Execute*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
	float DashExecTime;
	/*braking factor of the dash higher factor means more abrupt stop*/
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
	float DashBrakingFrictionFactor;
	/*tells us if this contoller is Dashing*/
	UPROPERTY(VisibleAnywhere, Category = "Movement|Dashing")
	bool bDashing;
	/*Timer handler For The Dash Execute Time then Cooldown*/
	UPROPERTY(VisibleAnywhere, Category = "Movement|Dashing")
	FTimerHandle TimerHandle_Dashing;
	/*Delegate Where we can do stuff at the strat of the Dash
	this delegates is to implement in blueprints*/
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStartDashing OnStartDashing;
	/*Delegate Where we can do stuff at the end of the Dash
	this delegates is to implement in blueprints*/
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStopDashing OnStopDashing;
protected:


	// 
	// BASICS
	//
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Called every frames
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	// 
	// CAMERA CONTROL
	//
	/* Called via input to turn at a given rate.*/
	UFUNCTION(Category = Camera)
	void TurnRate(float Rate);

	/*Called via input to turn look up/down at a given rate.*/
	UFUNCTION(Category = Camera)
	void LookUpRate(float Rate);

	// 
	// WAlKING
	//
	/** Handles moving forward/backward */
	UFUNCTION(Category = "Movement")
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	UFUNCTION(Category = "Movement")
	void MoveRight(float Val);

	// 
	// SPRINTING
	//
	/*determine if the Player can sprint*/
	UFUNCTION(Category = "Movement|Sprinting")
	bool CanSprint();

	/*Handles the start of this controller sprinting action*/
	UFUNCTION(Category = "Movement|Sprinting")
	void StartSprinting();
	/*Handles the end of this controller sprinting action*/
	UFUNCTION(Category = "Movement|Sprinting")
	void StopSprinting();
	/*since bSprinting is protected we need this function to change its value*/
	UFUNCTION(Category = "Movement|Sprinting")
	void SetSprinting(const bool bNewSprinting);


	// 
	// CROUCHING
	//
	/*Handles the start of this controller crouching action*/
	UFUNCTION(Category = "Movement|Crouching")
	void StartCrouching();
	/*Handles the end of this controller crouching action*/
	UFUNCTION(Category = "Movement|Crouching")
	void StopCrouching();
	/*since bCrouching is protected we need this function to change its value*/
	UFUNCTION(Category = "Movement|Crouching")
	void SetCrouching(bool bNewCrouching);
	/*determines if this controller can stand of must stay crouched*/
	UFUNCTION(Category = "Movement|Crouching")
	bool CanStand();

	// 
	// JUMPING
	//
	/*Handles the start of this controller jumping action*/
	UFUNCTION(Category = "Movement|Jumping")
	void StartJumping();
	/*Handles the end of this controller jumping action*/
	UFUNCTION(Category = "Movement|Jumping")
	void StopJumping();

	// 
	// DASHING
	//
	/*Handles the start of this controller dashing action*/
	UFUNCTION(Category = "Movement|Dashing")
	void StartDashing();
	/*Handles the end of this controller dashing action*/
	UFUNCTION(Category = "Movement|Dashing")
	void StopDashing();
	/*since bDashing is protected we need this function to change its value*/
	UFUNCTION(Category = "Movement|Dashing")
	void SetDashing(bool bNewDashing);
	/*After the cooldown we reset bDashing*/
	UFUNCTION(Category = "Movement|Dashing")
	void ResetDash();
	/*determine if this controller can dash*/
	UFUNCTION(Category = "Movement|Dashing")
	bool CanDash();

	//
	// SLIDING 
	//
	/*Handles the start of this controller sliding action*/
	UFUNCTION(Category = "Movement|Sliding")
	void StartSliding();
	/*Handles the end of this controller sliding action*/
	UFUNCTION(Category = "Movement|Sliding")
	void StopSliding();
	/*calculation of the flloor influence of the sliding speed. bigger slope means more influence*/
	UFUNCTION(Category = "Movement|Sliding")
	FVector CalculateFloorInfluence(FVector FloorNormal);

	//
	// MOVEMENT RESOLVING
	//
	/*This function makes sure that no values stay true or false when they should or should not*/
	UFUNCTION(Category = "Movement|MovementState")
	void ResolveMovementState();
	/*Since this controller movement state is protected we need this function to change its value*/
	UFUNCTION(Category = "Movement|MovementState")
	void SetMovementState(EMovementState NewMovementState);
	/*When Changing States we need to reset some values*/
	UFUNCTION(Category = "Movement|MovementState")
	void OnMovementStateChange(EMovementState PreviousMovementState);

public:
	/*Helper Function that returns the current movement state*/
	UFUNCTION(BlueprintCallable, Category = "Movement|MovementState")
	FORCEINLINE EMovementState GetMovementState() {return MovementState;}
};
