#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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


	// =========================================================
	// STAGE SETTINGS
	// =========================================================

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


	// =========================================================
	// TARGET
	// =========================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Target")
	AActor* TargetActor = nullptr;


	// =========================================================
	// MOVEMENT
	// =========================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float OrbitSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float ApproachSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Movement")
	float MovementSpeed = 600.0f;


	// =========================================================
	// NAVIGATION
	// =========================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Navigation")
	float NavigationUpdateInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Navigation")
	float NavigationAcceptanceRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Navigation")
	float NavigationSampleRadius = 300.0f;


	// =========================================================
	// BEHAVIOR TIMER
	// =========================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage1MinBehaviorTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage1MaxBehaviorTime = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage2MinBehaviorTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage2MaxBehaviorTime = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage3MinBehaviorTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator|Behavior")
	float Stage3MaxBehaviorTime = 10.0f;


	// =========================================================
	// FUNCTIONS
	// =========================================================

	UFUNCTION(BlueprintCallable, Category = "Predator")
	void UpdatePredatorStage();

	UFUNCTION(BlueprintCallable, Category = "Predator")
	float GetDistanceToTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Predator")
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Predator")
	EPredatorStage GetCurrentStage() const;


private:

	float BehaviorTimer = 0.0f;

	float NavigationTimer = 0.0f;

	FVector NavigationTargetLocation = FVector::ZeroVector;


	void UpdateBehaviorTimer(float DeltaTime);

	void UpdateOrbitMovement(float DeltaTime);

	bool FindNavigationTarget(FVector& OutLocation);

	float GetDesiredOrbitRadius() const;

	float GetTargetOrbitRadius() const;
};