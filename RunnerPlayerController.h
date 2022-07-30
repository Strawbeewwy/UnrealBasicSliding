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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVaulting, class ARunnerGameCharacter*, Character);
// 
// INTERACTION DATA STRUCTURE
//
USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	//The current interactable component we're viewing, if there is one
	UPROPERTY()
		class UInteractionComponent* ViewedInteractionComponent;

	//The time when we last checked for an interactable
	UPROPERTY()
		float LastInteractionCheckTime;

	//Whether the local player is holding the interact key
	UPROPERTY()
		bool bInteractHeld;

};

UENUM(BlueprintType)
enum class EPlayerPerspective : uint8
{
	IR_ThirdPerson UMETA(DisplayName = "ThirdPerson"),
	IR_FirstPerson UMETA(DisplayName = "FirstPerson")
};

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	EPlayerPerspective PlayerPerspective;

	//
	// MovementState 
	// 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|MovementState")
	EMovementState MovementState;

	//
	// CROUCHING
	//
	UPROPERTY(VisibleAnywhere, Category = "Movement|Crouching")
	bool bCrouching;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Crouching")
	float CrouchSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Movement|Crouching")
	float StandingCapsuleHalfHeight;


	// 
	// WAlKING
	//
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
		float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
		float WalkingGroundFriction;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
		float WalkingBrakingDecelerationWalking;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Walking")
		float WalkingBrakingFrictionFactor;

	// 
	// SPRINTING
	//
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sprinting")
		float SprintSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprinting")
		bool bSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprinting")
		bool bCanSprint;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStartSprinting OnStartSprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStopSprinting OnStopSprinting;

	//
	// SLIDING 
	//
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStartSliding OnStartSliding;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnStopSliding OnStopSliding;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlideSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlidingGroundFriction;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlidingBrakingDecelerationWalking;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlideMultiplier;

	//
	// DASHING
	//
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
		float DashDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
		float DashCoolDown;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
		float DashExecTime;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dashing")
		float DashBrakingFrictionFactor;

	UPROPERTY(VisibleAnywhere, Category = "Movement|Dashing")
		bool bDashing;

	UPROPERTY(VisibleAnywhere, Category = "Movement|Dashing")
		FTimerHandle TimerHandle_Dashing;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
		FOnStartDashing OnStartDashing;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
		FOnStopDashing OnStopDashing;
	// 
	// INTERACTION
	//
		/*How often in seconds to check for an interactable object. Set this to zero if you want to check every tick.*/
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
		float InteractionCheckFrequency;

	/*How far we'll trace when we check if the player is looking at an interactable object*/
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
		float InteractionCheckDistance;

	/*Information about the current state of the players interaction*/
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
		FInteractionData InteractionData;

	/*TimerHandler for interactions*/
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
		FTimerHandle TimerHandle_Interact;

	//Helper function to make grabbing interactable faster
	FORCEINLINE class UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	// 
	// VAULTING
	//
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
		FOnVaulting OnVaulting;


	//
	// HUD
	//
	class ARunnerHUD* RunnerHUD;

public:

	// 
	// ITEM
	//
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class APickUp> PickupClass;

protected:


	// 
	// BASICS
	//
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

	/*start and stop sprinting functions*/
	UFUNCTION(Category = "Movement|Sprinting")
		void StartSprinting();

	UFUNCTION(Category = "Movement|Sprinting")
		void StopSprinting();

	UFUNCTION(Category = "Movement|Sprinting")
		void SetSprinting(const bool bNewSprinting);


	// 
	// CROUCHING
	//
		/*start and stop crouching functions*/
	UFUNCTION(Category = "Movement|Crouching")
		void StartCrouching();

	UFUNCTION(Category = "Movement|Crouching")
		void StopCrouching();

	UFUNCTION(Category = "Movement|Crouching")
		void SetCrouching(bool bNewCrouching);

	UFUNCTION(Category = "Movement|Crouching")
		bool CanStand();

	// 
	// JUMPING
	//
		/*start and stop jumping functions*/
	UFUNCTION(Category = "Movement|Jumping")
		void StartJumping();

	UFUNCTION(Category = "Movement|Jumping")
		void StopJumping();

	// 
	// DASHING
	//
	/*Makes the character Dash in the direction it is facing*/
	UFUNCTION(Category = "Movement|Dashing")
	void StartDashing();

	UFUNCTION(Category = "Movement|Dashing")
	void StopDashing();
	
	UFUNCTION(Category = "Movement|Dashing")
	void SetDashing(bool bNewDashing);

	UFUNCTION(Category = "Movement|Dashing")
	void ResetDash();

	UFUNCTION(Category = "Movement|Dashing")
	bool CanDash();


	// 
	// CAMERA CONTROL
	//
	/*changes the player's point of view of his character*/
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SwitchPlayerPerspective();

	//
	// SLIDING 
	//
	/*start and stop sliding functions*/
	UFUNCTION(Category = "Movement|Sliding")
	void StartSliding();

	UFUNCTION(Category = "Movement|Sliding")
	void StopSliding();

	UFUNCTION(Category = "Movement|Sliding")
	FVector CalculateFloorInfluence(FVector FloorNormal);

	//
	// MOVEMENT RESOLVING
	//
	UFUNCTION(Category = "Movement|MovementState")
	void ResolveMovementState();

	UFUNCTION(Category = "Movement|MovementState")
	void SetMovementState(EMovementState NewMovementState);

	UFUNCTION(Category = "Movement|MovementState")
	void OnMovementStateChange(EMovementState PreviousMovementState);


	// 
	// VAULTING
	//
	/*Makes the character vault*/
	UFUNCTION(Category = "Movement|Dashing")
	void Vault();



public:

	UFUNCTION(BlueprintCallable, Category = "Movement|MovementState")
	FORCEINLINE EMovementState GetMovementState() {return MovementState;}
	
	// 
	// INTERACTION
	//
	//Cast a line trace and look for InteractionComponent in the actor that the Player looks at
	UFUNCTION(Category = "interaction")
		void PerformInteractionCheck();

	//resets value when the player loose visual with an InteractionComponent
	UFUNCTION(Category = "interaction")
		void CouldntFindInteractable();

	//show the widget of the interactionComponent
	UFUNCTION(Category = "interaction")
		void FoundNewInteractable(UInteractionComponent* Interactable);

	//Begining of the interaction process
	UFUNCTION(Category = "interaction")
		void BeginInteract();

	//ending of the interaction process
	UFUNCTION(Category = "interaction")
		void EndInteract();

	//Perform the interaction
	UFUNCTION(Category = "interaction")
		void Interact();

	//True if the player is interacting with an item that has an interaction time
	UFUNCTION(Category = "interaction")
		bool IsInteracting() const;

	//Get the time till we interact with the current interactable
	UFUNCTION(Category = "interaction")
		float GetRemainingInteractTime() const;

	// 
	// ITEM
	//

	UFUNCTION(BlueprintCallable, Category = "Item")
	void UseItem(class UItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void DropItem(class UItem* Item, const int32 Quantity);
	
};