#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Sound/SoundBase.h"
#include "PredatorCharacter.generated.h"

UENUM(BlueprintType)
enum class EPredatorStage : uint8
{
	Stage1 UMETA(DisplayName = "Stage 1"),
	Stage2 UMETA(DisplayName = "Stage 2"),
	Stage3 UMETA(DisplayName = "Stage 3")
};

UCLASS()
class APHOTIA_9_21_API APredatorCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	APredatorCharacter();

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;


	//stage settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Stages")
	float Stage1Radius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Stages")
	float Stage2Radius = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Stages")
	float Stage3Radius = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Predator|Stages")
	EPredatorStage CurrentStage = EPredatorStage::Stage1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Stages")
	EPredatorStage TargetStage = EPredatorStage::Stage2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Target")
	AActor* TargetActor = nullptr;

 // predd move

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float OrbitSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float ApproachSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float MovementSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float VerticalOrbitStrength = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float VerticalOrbitSpeed = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float Stage1ResetSpeed = 3000.0f;


	//navv

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Navigation")
	float NavigationUpdateInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Navigation")
	float NavigationAcceptanceRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Navigation")
	float NavigationSampleRadius = 300.0f;


	//timers

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage1MinBehaviorTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage1MaxBehaviorTime = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage2MinBehaviorTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage2MaxBehaviorTime = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage3MinBehaviorTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage3MaxBehaviorTime = 5.0f;

 //attack part begins

	// Distance at which the predator begins its attack rush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float AttackTriggerDistance = 600.0f;

	// Distance from player at which the attack actually hits.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float AttackHitDistance = 100.0f;

	// Speed of the predator while rushing toward the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float AttackRushSpeed = 1800.0f;

	// Maximum amount of time the attack rush can last.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float AttackRushDuration = 1.0f;

	// Probability of starting an attack when the attack condition is met.
	// 1.0 = 100%.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float AttackChance = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float AttackDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float AttackCooldown = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float RetreatDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float RetreatSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	float RetreatOrbitSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Attack")
	USoundBase* stageTransitionSFX;


	UFUNCTION(BlueprintImplementableEvent, Category = "Predator|Attack")
	void OnPredatorAttack(float Damage);



	UFUNCTION(BlueprintCallable, Category = "Predator")
	void UpdatePredatorStage();

	UFUNCTION(BlueprintCallable, Category = "Predator")
	float GetDistanceToTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Predator")
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Predator")
	EPredatorStage GetCurrentStage() const;

	UFUNCTION(BlueprintPure, Category = "Predator|Attack")
	bool IsRetreating() const;

	UFUNCTION(BlueprintCallable, Category = "Predator|Flashlight")
	void ResetToStage1();


private:

 // internal states

	float BehaviorTimer = 0.0f;

	float NavigationTimer = 0.0f;

	float AttackCooldownTimer = 0.0f;

	float RetreatTimer = 0.0f;

	float AttackRushTimer = 0.0f;

	float VerticalOrbitPhase = 0.0f;

	FVector NavigationTargetLocation = FVector::ZeroVector;

	FVector RetreatDirection = FVector::ZeroVector;

	FVector AttackDirection = FVector::ZeroVector;

	bool bIsRetreating = false;

	bool bReturningToStage1 = false;

	bool bIsAttacking = false;

	bool bHasDealtAttackDamage = false;


 // internal methods

	void UpdateBehaviorTimer(float DeltaTime);

	void UpdateAttackCooldown(float DeltaTime);

	void UpdateRetreat(float DeltaTime);

	void UpdateStage1ResetMovement(float DeltaTime);

	void UpdateAttackMovement(float DeltaTime);

	void UpdateOrbitMovement(float DeltaTime);

	void TryStartAttack();

	void FinishAttack();

	void StartRetreat();

	bool FindNavigationTarget(FVector& OutLocation);

	float GetDesiredOrbitRadius() const;

	float GetTargetOrbitRadius() const;
};