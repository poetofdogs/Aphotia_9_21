#include "PredatorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

APredatorCharacter::APredatorCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void APredatorCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

	CurrentStage = EPredatorStage::Stage1;
	TargetStage = EPredatorStage::Stage2;

	BehaviorTimer = FMath::FRandRange(
		Stage1MinBehaviorTime,
		Stage1MaxBehaviorTime
	);

	NavigationTimer = 0.0f;
	NavigationTargetLocation = FVector::ZeroVector;
}

void APredatorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePredatorStage();
	UpdateBehaviorTimer(DeltaTime);
	UpdateOrbitMovement(DeltaTime);
}

void APredatorCharacter::UpdatePredatorStage()
{
	if (!TargetActor)
	{
		return;
	}

	const float Distance = GetDistanceToTarget();

	if (Distance <= Stage3Radius)
	{
		CurrentStage = EPredatorStage::Stage3;
	}
	else if (Distance <= Stage2Radius)
	{
		CurrentStage = EPredatorStage::Stage2;
	}
	else
	{
		CurrentStage = EPredatorStage::Stage1;
	}
}

float APredatorCharacter::GetDistanceToTarget() const
{
	if (!TargetActor)
	{
		return -1.0f;
	}

	return FVector::Dist(
		GetActorLocation(),
		TargetActor->GetActorLocation()
	);
}

void APredatorCharacter::SetTargetActor(AActor* NewTarget)
{
	TargetActor = NewTarget;
}

EPredatorStage APredatorCharacter::GetCurrentStage() const
{
	return CurrentStage;
}

void APredatorCharacter::UpdateBehaviorTimer(float DeltaTime)
{
	if (!TargetActor)
	{
		return;
	}

	BehaviorTimer -= DeltaTime;

	if (BehaviorTimer <= 0.0f)
	{
		switch (CurrentStage)
		{
		case EPredatorStage::Stage1:

			TargetStage = EPredatorStage::Stage2;

			BehaviorTimer = FMath::FRandRange(
				Stage1MinBehaviorTime,
				Stage1MaxBehaviorTime
			);

			break;


		case EPredatorStage::Stage2:
		{
			const float Decision = FMath::FRand();

			if (Decision < 0.30f)
			{
				TargetStage = EPredatorStage::Stage1;
			}
			else if (Decision < 0.70f)
			{
				TargetStage = EPredatorStage::Stage2;
			}
			else
			{
				TargetStage = EPredatorStage::Stage3;
			}

			BehaviorTimer = FMath::FRandRange(
				Stage2MinBehaviorTime,
				Stage2MaxBehaviorTime
			);

			break;
		}


		case EPredatorStage::Stage3:

			TargetStage = EPredatorStage::Stage2;

			BehaviorTimer = FMath::FRandRange(
				Stage3MinBehaviorTime,
				Stage3MaxBehaviorTime
			);

			break;
		}
	}
}

float APredatorCharacter::GetDesiredOrbitRadius() const
{
	switch (CurrentStage)
	{
	case EPredatorStage::Stage1:
		return Stage1Radius;

	case EPredatorStage::Stage2:
		return Stage2Radius;

	case EPredatorStage::Stage3:
		return Stage3Radius;

	default:
		return Stage1Radius;
	}
}

float APredatorCharacter::GetTargetOrbitRadius() const
{
	switch (TargetStage)
	{
	case EPredatorStage::Stage1:
		return Stage1Radius;

	case EPredatorStage::Stage2:
		return Stage2Radius;

	case EPredatorStage::Stage3:
		return Stage3Radius;

	default:
		return Stage2Radius;
	}
}

bool APredatorCharacter::FindNavigationTarget(FVector& OutLocation)
{
	// Not used by the restored movement system.
	return false;
}

void APredatorCharacter::UpdateOrbitMovement(float DeltaTime)
{
	if (!TargetActor)
	{
		return;
	}

	const FVector PredatorLocation = GetActorLocation();
	const FVector PlayerLocation = TargetActor->GetActorLocation();

	FVector ToPredator = PredatorLocation - PlayerLocation;

	ToPredator.Z = 0.0f;

	if (ToPredator.IsNearlyZero())
	{
		ToPredator = FVector(1.0f, 0.0f, 0.0f);
	}

	ToPredator.Normalize();

	const float DesiredRadius = GetTargetOrbitRadius();
	const float CurrentDistance = GetDistanceToTarget();

	float RadialAmount = 0.0f;

	const float RadiusTolerance = 100.0f;

	if (CurrentDistance > DesiredRadius + RadiusTolerance)
	{
		RadialAmount = -1.0f;
	}
	else if (CurrentDistance < DesiredRadius - RadiusTolerance)
	{
		RadialAmount = 1.0f;
	}

	const FVector OrbitDirection(
		-ToPredator.Y,
		ToPredator.X,
		0.0f
	);

	FVector MovementDirection =
		(OrbitDirection * OrbitSpeed) +
		(ToPredator * RadialAmount * ApproachSpeed);

	if (!MovementDirection.IsNearlyZero())
	{
		MovementDirection.Normalize();

		AddMovementInput(
			MovementDirection,
			1.0f
		);
	}
}