#include "PredatorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


APredatorCharacter::APredatorCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

	// 3d movement for predator is flying for now
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}


void APredatorCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	GetCharacterMovement()->MaxFlySpeed = MovementSpeed;

	CurrentStage = EPredatorStage::Stage1;

	TargetStage = EPredatorStage::Stage2;

	BehaviorTimer = FMath::FRandRange(
		Stage1MinBehaviorTime,
		Stage1MaxBehaviorTime
	);

	NavigationTimer = 0.0f;

	AttackCooldownTimer = 0.0f;

	RetreatTimer = 0.0f;

	AttackRushTimer = 0.0f;

	VerticalOrbitPhase = FMath::FRandRange(
		0.0f,
		2.0f * PI
	);

	NavigationTargetLocation = FVector::ZeroVector;

	RetreatDirection = FVector::ZeroVector;

	AttackDirection = FVector::ZeroVector;

	bIsRetreating = false;

	bReturningToStage1 = false;

	bIsAttacking = false;

	bHasDealtAttackDamage = false;
}


//tick

void APredatorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TargetActor)
	{
		return;
	}


	if (bReturningToStage1)
	{
		CurrentStage = EPredatorStage::Stage1;

		TargetStage = EPredatorStage::Stage1;

		const float Distance = GetDistanceToTarget();

		const float Stage1Tolerance = 100.0f;

		if (Distance >= Stage1Radius - Stage1Tolerance)
		{
			bReturningToStage1 = false;

			GetCharacterMovement()->MaxFlySpeed =
				MovementSpeed;

			CurrentStage = EPredatorStage::Stage1;

			TargetStage = EPredatorStage::Stage2;

			BehaviorTimer = FMath::FRandRange(
				Stage1MinBehaviorTime,
				Stage1MaxBehaviorTime
			);
		}
		else
		{
			UpdateStage1ResetMovement(DeltaTime);

			return;
		}
	}



	if (bIsAttacking)
	{
		UpdateAttackMovement(DeltaTime);

		return;
	}



	UpdatePredatorStage();



	UpdateAttackCooldown(DeltaTime);


	UpdateRetreat(DeltaTime);

	if (bIsRetreating)
	{
		return;
	}



	if (CurrentStage == EPredatorStage::Stage3)
	{
		const float Distance = GetDistanceToTarget();

		if (Distance <= AttackTriggerDistance &&
			AttackCooldownTimer <= 0.0f)
		{
			const float Decision = FMath::FRand();

			if (Decision <= AttackChance)
			{
				TryStartAttack();

				return;
			}
		}
	}



	UpdateBehaviorTimer(DeltaTime);

	UpdateOrbitMovement(DeltaTime);
}


    //stage updation

void APredatorCharacter::UpdatePredatorStage()
{
	if (!TargetActor ||
		bIsRetreating ||
		bReturningToStage1 ||
		bIsAttacking)
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


//timers for stages

void APredatorCharacter::UpdateBehaviorTimer(float DeltaTime)
{
	if (!TargetActor ||
		bIsRetreating ||
		bReturningToStage1 ||
		bIsAttacking)
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

	// 1st stage
	case EPredatorStage::Stage1:
	{
		TargetStage = EPredatorStage::Stage2;

		BehaviorTimer = FMath::FRandRange(
			Stage1MinBehaviorTime,
			Stage1MaxBehaviorTime
		);

		break;
	}

     //stage2

	case EPredatorStage::Stage2:
	{
		// Stage 2 ONLY progresses toward Stage 3.

		TargetStage = EPredatorStage::Stage3;

		BehaviorTimer = FMath::FRandRange(
			Stage2MinBehaviorTime,
			Stage2MaxBehaviorTime
		);

		break;
	}


	//3rd stage

	case EPredatorStage::Stage3:
	{
		// If no attack happens, return to Stage 2.

		TargetStage = EPredatorStage::Stage2;

		BehaviorTimer = FMath::FRandRange(
			Stage3MinBehaviorTime,
			Stage3MaxBehaviorTime
		);

		break;
	}
	}
}


//cooldown between attacks

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




void APredatorCharacter::TryStartAttack()
{
	if (!TargetActor)
	{
		return;
	}

	if (bIsRetreating ||
		bReturningToStage1 ||
		bIsAttacking)
	{
		return;
	}

	if (AttackCooldownTimer > 0.0f)
	{
		return;
	}

	const float Distance = GetDistanceToTarget();

	if (Distance > AttackTriggerDistance)
	{
		return;
	}



	AttackDirection =
		TargetActor->GetActorLocation() -
		GetActorLocation();

	if (AttackDirection.IsNearlyZero())
	{
		AttackDirection =
			FVector(1.0f, 0.0f, 0.0f);
	}

	AttackDirection.Normalize();


	//begin attacking

	bIsAttacking = true;

	bHasDealtAttackDamage = false;

	AttackRushTimer = AttackRushDuration;


	// The attack movement is much faster than normal movement.
	GetCharacterMovement()->MaxFlySpeed =
		AttackRushSpeed;
}




void APredatorCharacter::UpdateAttackMovement(float DeltaTime)
{
	if (!TargetActor)
	{
		bIsAttacking = false;

		GetCharacterMovement()->MaxFlySpeed =
			MovementSpeed;

		return;
	}


	AttackRushTimer -= DeltaTime;


	// Recalculate the attack direction continuously so the
	// predator tracks the player during the rush.
	AttackDirection =
		TargetActor->GetActorLocation() -
		GetActorLocation();


	if (!AttackDirection.IsNearlyZero())
	{
		AttackDirection.Normalize();
	}


	// Make sure the attack speed stays active.
	GetCharacterMovement()->MaxFlySpeed =
		AttackRushSpeed;


	// Rush directly toward the player in 3D space.
	AddMovementInput(
		AttackDirection,
		1.0f
	);


	const float Distance = GetDistanceToTarget();


	//hit success

	if (!bHasDealtAttackDamage &&
		Distance <= AttackHitDistance)
	{
		bHasDealtAttackDamage = true;

		OnPredatorAttack(AttackDamage);

		AttackCooldownTimer =
			AttackCooldown;

		FinishAttack();

		return;
	}


	

	if (AttackRushTimer <= 0.0f)
	{
		// If the predator somehow didn't reach the player,
		// still finish the attack without dealing damage.
		FinishAttack();
	}
}


//finalising attack

void APredatorCharacter::FinishAttack()
{
	bIsAttacking = false;

	bHasDealtAttackDamage = false;

	// Restore normal underwater movement speed.
	GetCharacterMovement()->MaxFlySpeed =
		MovementSpeed;


	// Immediately swim away after attacking.
	StartRetreat();
}


//beginning to retreat

void APredatorCharacter::StartRetreat()
{
	if (!TargetActor)
	{
		return;
	}


	FVector AwayFromPlayer =
		GetActorLocation() -
		TargetActor->GetActorLocation();


	if (AwayFromPlayer.IsNearlyZero())
	{
		AwayFromPlayer =
			FVector(1.0f, 0.0f, 0.0f);
	}

	AwayFromPlayer.Normalize();

	RetreatDirection =
		AwayFromPlayer;

	bIsRetreating = true;

	RetreatTimer =
		RetreatDuration;

	TargetStage =
		EPredatorStage::Stage1;

	BehaviorTimer =
		FMath::FRandRange(
			Stage1MinBehaviorTime,
			Stage1MaxBehaviorTime
		);
}


//retreating

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
		GetActorLocation() -
		TargetActor->GetActorLocation();


	if (!AwayFromPlayer.IsNearlyZero())
	{
		AwayFromPlayer.Normalize();

		RetreatDirection =
			AwayFromPlayer;
	}


	// curved retreat
	FVector SideDirection =
		FVector::CrossProduct(
			FVector::UpVector,
			RetreatDirection
		);


	if (SideDirection.IsNearlyZero())
	{
		SideDirection =
			FVector::RightVector;
	}
	else
	{
		SideDirection.Normalize();
	}


	FVector RetreatMovement =
		(RetreatDirection * RetreatSpeed)
		+
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

		CurrentStage =
			EPredatorStage::Stage1;

		TargetStage =
			EPredatorStage::Stage1;

		BehaviorTimer =
			FMath::FRandRange(
				Stage1MinBehaviorTime,
				Stage1MaxBehaviorTime
			);
	}
}


//flash behaviour 

void APredatorCharacter::ResetToStage1()
{
	if (!TargetActor)
	{
		return;
	}


	
	bIsAttacking = false;

	bIsRetreating = false;

	bHasDealtAttackDamage = false;

	AttackRushTimer = 0.0f;

	RetreatTimer = 0.0f;


	
	CurrentStage =
		EPredatorStage::Stage1;

	TargetStage =
		EPredatorStage::Stage1;


	
	bReturningToStage1 = true;


	
	BehaviorTimer =
		FMath::FRandRange(
			Stage1MinBehaviorTime,
			Stage1MaxBehaviorTime
		);


	
	VerticalOrbitPhase =
		FMath::FRandRange(
			0.0f,
			2.0f * PI
		);


	// fast movement activation
	GetCharacterMovement()->MaxFlySpeed =
		Stage1ResetSpeed;
}




void APredatorCharacter::UpdateStage1ResetMovement(float DeltaTime)
{
	if (!TargetActor)
	{
		return;
	}


	GetCharacterMovement()->MaxFlySpeed =
		Stage1ResetSpeed;


	// Full 3D direction away from player.
	FVector AwayFromPlayer =
		GetActorLocation() -
		TargetActor->GetActorLocation();


	if (AwayFromPlayer.IsNearlyZero())
	{
		AwayFromPlayer =
			FVector(1.0f, 0.0f, 0.0f);
	}


	AwayFromPlayer.Normalize();


	// Rush away from player.
	AddMovementInput(
		AwayFromPlayer,
		1.0f
	);
}


//retreat stage

bool APredatorCharacter::IsRetreating() const
{
	return bIsRetreating;
}


//curr stage radii

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


//target stage radii

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


//nav

bool APredatorCharacter::FindNavigationTarget(
	FVector& OutLocation)
{
	return false;
}




void APredatorCharacter::UpdateOrbitMovement(
	float DeltaTime)
{
	if (!TargetActor ||
		bIsRetreating ||
		bIsAttacking)
	{
		return;
	}


	

	VerticalOrbitPhase +=
		VerticalOrbitSpeed *
		DeltaTime;


	if (VerticalOrbitPhase >
		2.0f * PI)
	{
		VerticalOrbitPhase -=
			2.0f * PI;
	}


	

	const FVector PredatorLocation =
		GetActorLocation();

	const FVector PlayerLocation =
		TargetActor->GetActorLocation();


	FVector ToPredator =
		PredatorLocation -
		PlayerLocation;


	if (ToPredator.IsNearlyZero())
	{
		ToPredator =
			FVector(1.0f, 0.0f, 0.0f);
	}

	ToPredator.Normalize();


	//horizontally orbitiing

	FVector HorizontalOrbitDirection =
		FVector::CrossProduct(
			FVector::UpVector,
			ToPredator
		);


	if (HorizontalOrbitDirection.IsNearlyZero())
	{
		HorizontalOrbitDirection =
			FVector::RightVector;
	}
	else
	{
		HorizontalOrbitDirection.Normalize();
	}


	//vertically orbittin

	FVector VerticalOrbitDirection =
		FVector::CrossProduct(
			ToPredator,
			HorizontalOrbitDirection
		);


	if (!VerticalOrbitDirection.IsNearlyZero())
	{
		VerticalOrbitDirection.Normalize();
	}


	const float VerticalAmount =
		FMath::Sin(
			VerticalOrbitPhase
		) *
		VerticalOrbitStrength;




	const float DesiredRadius =
		GetTargetOrbitRadius();

	const float CurrentDistance =
		GetDistanceToTarget();


	//radial correction

	float RadialAmount = 0.0f;

	const float RadiusTolerance =
		100.0f;


	if (CurrentDistance >
		DesiredRadius +
		RadiusTolerance)
	{
		RadialAmount = -1.0f;
	}
	else if (CurrentDistance <
		DesiredRadius -
		RadiusTolerance)
	{
		RadialAmount = 1.0f;
	}


	//3d moevment

	FVector MovementDirection =
		(HorizontalOrbitDirection *
			OrbitSpeed)
		+
		(
			VerticalOrbitDirection *
			VerticalAmount *
			OrbitSpeed
		)
		+
		(
			ToPredator *
			RadialAmount *
			ApproachSpeed
		);


	if (!MovementDirection.IsNearlyZero())
	{
		MovementDirection.Normalize();

		AddMovementInput(
			MovementDirection,
			1.0f
		);
	}
}