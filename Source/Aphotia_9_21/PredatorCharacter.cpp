#include "PredatorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

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
	AttackCooldownTimer = 0.0f;
	RetreatTimer = 0.0f;

	NavigationTargetLocation = FVector::ZeroVector;
	RetreatDirection = FVector::ZeroVector;

	bIsRetreating = false;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Green,
			TEXT("PREDATOR C++ BEGIN PLAY")
		);
	}
}

void APredatorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TargetActor)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				1.0f,
				FColor::Red,
				TEXT("ATTACK DEBUG: TargetActor is NULL")
			);
		}

		return;
	}

	UpdatePredatorStage();

	UpdateAttackCooldown(DeltaTime);

	UpdateRetreat(DeltaTime);

	if (bIsRetreating)
	{
		return;
	}

	const float Distance = GetDistanceToTarget();

	// ---------------------------------------------------------
	// DEBUG DISTANCE / STAGE
	// ---------------------------------------------------------

	if (GEngine)
	{
		FString DebugText = FString::Printf(
			TEXT("PREDATOR | Distance: %.0f | Stage: %d | AttackDist: %.0f"),
			Distance,
			static_cast<int32>(CurrentStage) + 1,
			AttackDistance
		);

		GEngine->AddOnScreenDebugMessage(
			2,
			0.2f,
			FColor::Yellow,
			DebugText
		);
	}

	// ---------------------------------------------------------
	// ATTACK CHECK
	// ---------------------------------------------------------

	if (CurrentStage == EPredatorStage::Stage3)
	{
		if (Distance <= AttackDistance)
		{
			if (AttackCooldownTimer <= 0.0f)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						3,
						0.5f,
						FColor::Cyan,
						TEXT("ATTACK CHECK REACHED")
					);
				}

				const float Decision = FMath::FRand();

				if (Decision <= AttackChance)
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(
							4,
							2.0f,
							FColor::Green,
							TEXT("ATTACK CHANCE PASSED - FIRING ATTACK")
						);
					}

					TryAttack();
					return;
				}
				else
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(
							5,
							1.0f,
							FColor::Red,
							TEXT("ATTACK CHANCE FAILED")
						);
					}
				}
			}
			else
			{
				if (GEngine)
				{
					FString CooldownText = FString::Printf(
						TEXT("ATTACK COOLDOWN: %.1f"),
						AttackCooldownTimer
					);

					GEngine->AddOnScreenDebugMessage(
						6,
						0.5f,
						FColor::Orange,
						CooldownText
					);
				}
			}
		}
	}

	UpdateBehaviorTimer(DeltaTime);

	UpdateOrbitMovement(DeltaTime);
}

void APredatorCharacter::UpdatePredatorStage()
{
	if (!TargetActor || bIsRetreating)
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

	if (GEngine)
	{
		if (NewTarget)
		{
			GEngine->AddOnScreenDebugMessage(
				7,
				3.0f,
				FColor::Green,
				TEXT("PREDATOR TARGET SUCCESSFULLY SET")
			);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(
				7,
				3.0f,
				FColor::Red,
				TEXT("PREDATOR TARGET SET TO NULL")
			);
		}
	}
}

EPredatorStage APredatorCharacter::GetCurrentStage() const
{
	return CurrentStage;
}

void APredatorCharacter::UpdateBehaviorTimer(float DeltaTime)
{
	if (!TargetActor || bIsRetreating)
	{
		return;
	}

	BehaviorTimer -= DeltaTime;

	if (BehaviorTimer > 0.0f)
	{
		return;
	}

	switch (CurrentStage)
	{
	case EPredatorStage::Stage1:
	{
		TargetStage = EPredatorStage::Stage2;

		BehaviorTimer = FMath::FRandRange(
			Stage1MinBehaviorTime,
			Stage1MaxBehaviorTime
		);

		break;
	}

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
	{
		TargetStage = EPredatorStage::Stage2;

		BehaviorTimer = FMath::FRandRange(
			Stage3MinBehaviorTime,
			Stage3MaxBehaviorTime
		);

		break;
	}
	}
}

void APredatorCharacter::UpdateAttackCooldown(float DeltaTime)
{
	if (AttackCooldownTimer > 0.0f)
	{
		AttackCooldownTimer -= DeltaTime;

		if (AttackCooldownTimer < 0.0f)
		{
			AttackCooldownTimer = 0.0f;
		}
	}
}

void APredatorCharacter::TryAttack()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			8,
			3.0f,
			FColor::Green,
			TEXT("TRY ATTACK() CALLED")
		);
	}

	if (!TargetActor)
	{
		return;
	}

	if (bIsRetreating)
	{
		return;
	}

	if (AttackCooldownTimer > 0.0f)
	{
		return;
	}

	const float Distance = GetDistanceToTarget();

	if (Distance > AttackDistance)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			9,
			3.0f,
			FColor::Green,
			TEXT("ON PREDATOR ATTACK EVENT FIRING")
		);
	}

	OnPredatorAttack(AttackDamage);

	AttackCooldownTimer = AttackCooldown;

	StartRetreat();
}

void APredatorCharacter::StartRetreat()
{
	if (!TargetActor)
	{
		return;
	}

	FVector AwayFromPlayer =
		GetActorLocation() - TargetActor->GetActorLocation();

	AwayFromPlayer.Z = 0.0f;

	if (AwayFromPlayer.IsNearlyZero())
	{
		AwayFromPlayer = FVector(1.0f, 0.0f, 0.0f);
	}

	AwayFromPlayer.Normalize();

	RetreatDirection = AwayFromPlayer;

	bIsRetreating = true;

	RetreatTimer = RetreatDuration;

	TargetStage = EPredatorStage::Stage1;

	BehaviorTimer = FMath::FRandRange(
		Stage1MinBehaviorTime,
		Stage1MaxBehaviorTime
	);
}

void APredatorCharacter::UpdateRetreat(float DeltaTime)
{
	if (!bIsRetreating)
	{
		return;
	}

	if (!TargetActor)
	{
		bIsRetreating = false;
		return;
	}

	RetreatTimer -= DeltaTime;

	FVector AwayFromPlayer =
		GetActorLocation() - TargetActor->GetActorLocation();

	AwayFromPlayer.Z = 0.0f;

	if (!AwayFromPlayer.IsNearlyZero())
	{
		AwayFromPlayer.Normalize();

		RetreatDirection = AwayFromPlayer;
	}

	FVector SideDirection(
		-RetreatDirection.Y,
		RetreatDirection.X,
		0.0f
	);

	FVector RetreatMovement =
		(RetreatDirection * RetreatSpeed) +
		(SideDirection * RetreatOrbitSpeed);

	if (!RetreatMovement.IsNearlyZero())
	{
		RetreatMovement.Normalize();

		AddMovementInput(
			RetreatMovement,
			1.0f
		);
	}

	if (RetreatTimer <= 0.0f)
	{
		bIsRetreating = false;

		CurrentStage = EPredatorStage::Stage1;
		TargetStage = EPredatorStage::Stage1;

		BehaviorTimer = FMath::FRandRange(
			Stage1MinBehaviorTime,
			Stage1MaxBehaviorTime
		);
	}
}

bool APredatorCharacter::IsRetreating() const
{
	return bIsRetreating;
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
	return false;
}

void APredatorCharacter::UpdateOrbitMovement(float DeltaTime)
{
	if (!TargetActor || bIsRetreating)
	{
		return;
	}

	const FVector PredatorLocation = GetActorLocation();
	const FVector PlayerLocation = TargetActor->GetActorLocation();

	FVector ToPredator =
		PredatorLocation - PlayerLocation;

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