#include "EnvironmentFish.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

AEnvironmentFish::AEnvironmentFish()
{
    PrimaryActorTick.bCanEverTick = true;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(20);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_Pawn);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    Collision->SetGenerateOverlapEvents(false);
    FishMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FishMesh"));
    FishMesh->SetupAttachment(Collision);
    FishMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FishMesh->SetMobility(EComponentMobility::Movable);
    SetCanBeDamaged(false);
}

void AEnvironmentFish::BeginPlay()
{
    Super::BeginPlay();
    InitializeFish();
}

void AEnvironmentFish::InitializeFish()
{
    HomeLocation = GetActorLocation();
    Random.Initialize(RandomSeed ? RandomSeed : FMath::Rand());
    Heading = GetActorForwardVector();
    ScatterBias = Random.VRand() * 0.3f;
    SpeedVariation = Random.FRandRange(0.85f, 1.15f);
    // Mesh may have an off-centre pivot. Include the component offset in the safety envelope.
    FishMesh->UpdateBounds();
    const float MeshRadius = (FishMesh->Bounds.Origin - GetActorLocation()).Size() + FishMesh->Bounds.SphereRadius;
    BodyRadius = FMath::Max3(1.f, BodyRadius, MeshRadius);
    Collision->SetSphereRadius(BodyRadius / FMath::Max(0.001f, Collision->GetShapeScale()));
    Collision->SetCollisionEnabled(bEnableAvoidance ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    bInitialized = true;
    bFleeing = false;
    TargetAge = 0;
    FlashFleeTimeRemaining = 0;
    PickTarget();
}

FVector AEnvironmentFish::EffectiveExtent() const
{
    // Never allow a negative/inverted sampling box when a designer shrinks the area.
    return FVector(FMath::Max(10.f, float(FMath::Abs(SwimExtent.X)) - BodyRadius),
        FMath::Max(10.f, float(FMath::Abs(SwimExtent.Y)) - BodyRadius),
        FMath::Max(10.f, float(FMath::Abs(SwimExtent.Z)) - BodyRadius));
}

FVector AEnvironmentFish::ClampToArea(const FVector& P) const
{
    const FVector E = EffectiveExtent();
    return FVector(FMath::Clamp(P.X, HomeLocation.X-E.X, HomeLocation.X+E.X),
        FMath::Clamp(P.Y, HomeLocation.Y-E.Y, HomeLocation.Y+E.Y),
        FMath::Clamp(P.Z, HomeLocation.Z-E.Z, HomeLocation.Z+E.Z));
}

bool AEnvironmentFish::IsInsideArea(const FVector& P) const
{
    return (P - ClampToArea(P)).IsNearlyZero(0.01);
}

void AEnvironmentFish::PickTarget()
{
    const FVector E = EffectiveExtent();
    for (int32 Attempt=0; Attempt<16; ++Attempt)
    {
        const FVector P = HomeLocation + FVector(Random.FRandRange(-E.X,E.X),
            Random.FRandRange(-E.Y,E.Y), Random.FRandRange(-E.Z,E.Z));
        const FVector Delta = P - GetActorLocation();
        if (Delta.Size() > 60 && (!bEnableAvoidance || ClearDistance(Delta.GetSafeNormal(), Delta.Size()) >= Delta.Size()-1))
        {
            TargetLocation=P;
            TargetAge=0;
            return;
        }
    }
    TargetLocation=ClampToArea(GetActorLocation()+Random.VRand()*200);
    TargetAge=0;
}

void AEnvironmentFish::ScareFishInFlashBox(AActor* FlashHitbox)
{
    if (!IsValid(FlashHitbox) || !FlashHitbox->GetWorld())
    {
        return;
    }

    const UBoxComponent* Box = FlashHitbox->FindComponentByClass<UBoxComponent>();
    if (!Box)
    {
        return;
    }

    UWorld* World = FlashHitbox->GetWorld();
    const FVector Origin = FlashHitbox->GetActorLocation();
    const FVector Extent = Box->GetScaledBoxExtent();
    const FTransform BoxTransform = Box->GetComponentTransform();

    // Sample fish at the instant of the flash, using the same oriented box as the Boss.
    // A position test also works when fish do not generate overlap events.
    for (TActorIterator<AEnvironmentFish> It(World); It; ++It)
    {
        AEnvironmentFish* Fish = *It;
        const FVector FishLocation = Fish->GetActorLocation();
        const FVector Local = BoxTransform.InverseTransformPositionNoScale(FishLocation).GetAbs();
        if (Local.X > Extent.X || Local.Y > Extent.Y || Local.Z > Extent.Z)
        {
            continue;
        }

        FCollisionQueryParams Params(SCENE_QUERY_STAT(FishFlashVisibility), false, FlashHitbox);
        Params.AddIgnoredActor(Fish);
        if (const APawn* Instigator = FlashHitbox->GetInstigator())
        {
            Params.AddIgnoredActor(Instigator);
        }

        if (!World->LineTraceTestByChannel(Origin, FishLocation, ECC_Visibility, Params))
        {
            Fish->StartFleeingFromFlash(Origin);
        }
    }
}

void AEnvironmentFish::StartFleeingFromFlash(const FVector& Origin)
{
    if (!bInitialized)
    {
        InitializeFish();
    }

    FlashOrigin = Origin;
    FlashFleeTimeRemaining = FMath::Max(0.1f, FlashFleeDuration);
    bFleeing = true;
}

float AEnvironmentFish::ClearDistance(const FVector& Direction, float Distance) const
{
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AmbientFishAvoidance), false, this);
    // Visibility is a query, not player collision. Rocks must block this channel.
    if (GetWorld()->SweepSingleByChannel(Hit, GetActorLocation(), GetActorLocation()+Direction*Distance,
        FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(BodyRadius), Params))
        return FMath::Max(0.f, Hit.Distance-2.f);
    return Distance;
}

void AEnvironmentFish::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    StepSimulation(DeltaSeconds);
}

void AEnvironmentFish::StepSimulation(float DeltaSeconds)
{
    if (!bInitialized) InitializeFish();
    if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0) return;
    // Small substeps keep turn and swept movement stable across low frame rates.
    float Remaining=FMath::Min(DeltaSeconds, 0.25f);
    while (Remaining > UE_SMALL_NUMBER)
    {
        const float Step=FMath::Min(Remaining, 1.f/30.f);
        SimulateSubstep(Step);
        Remaining-=Step;
    }
}

void AEnvironmentFish::SimulateSubstep(float Dt)
{
    if (bFleeing)
    {
        FlashFleeTimeRemaining -= Dt;
        if (FlashFleeTimeRemaining <= 0.f)
        {
            bFleeing = false;
            PickTarget();
        }
    }
    TargetAge+=Dt;
    if (TargetAge>10.f || FVector::DistSquared(GetActorLocation(),TargetLocation)<FMath::Square(40.f)) PickTarget();
    FVector Desired=(TargetLocation-GetActorLocation()).GetSafeNormal();
    if (bFleeing)
    {
        const FVector Away = (GetActorLocation() - FlashOrigin).GetSafeNormal();
        Desired = (Away + ScatterBias).GetSafeNormal();
    }
    if (Desired.IsNearlyZero()) Desired=Heading;
    const float Speed=FMath::Max(0.f,SwimSpeed)*SpeedVariation*(bFleeing ? FMath::Max(1.f,FleeSpeedMultiplier) : 1.f);
    const float Probe=FMath::Max3(10.f,LookAhead,Speed*0.8f);
    const FVector Location=GetActorLocation();
    if (!IsInsideArea(Location+Desired*FMath::Min(Probe,100.f)))
        Desired=(ClampToArea(Location+Desired*Probe)-Location).GetSafeNormal();
    if (Desired.IsNearlyZero()) Desired=(HomeLocation-Location).GetSafeNormal();
    FVector Chosen=Desired;
    if (bEnableAvoidance && ClearDistance(Desired,Probe)<Probe)
    {
        float Best=-TNumericLimits<float>::Max();
        // Search a sphere of alternatives, including behind and above/below the fish.
        for (int32 X=-1;X<=1;++X) for (int32 Y=-1;Y<=1;++Y) for (int32 Z=-1;Z<=1;++Z)
        {
            const FVector D=FVector(X,Y,Z).GetSafeNormal();
            if (D.IsNearlyZero() || !IsInsideArea(Location+D*FMath::Min(Probe,100.f))) continue;
            const float Clear=ClearDistance(D,Probe);
            const float Score=Clear/Probe*4.f+float(FVector::DotProduct(D,Desired))+float(FVector::DotProduct(D,Heading))*0.3f;
            if (Score>Best) { Best=Score; Chosen=D; }
        }
    }
    Heading=FMath::RInterpConstantTo(Heading.Rotation(),Chosen.Rotation(),Dt,FMath::Max(1.f,TurnSpeed)*(bFleeing?1.5f:1.f)).Vector();
    SetActorRotation(Heading.Rotation());
    const FVector Next=ClampToArea(Location+Heading*Speed*Dt);
    const FVector Delta=Next-Location;
    // Sweep the actual step as well as looking ahead. A failed route stops; it never teleports.
    const float Allowed=bEnableAvoidance ? ClearDistance(Delta.GetSafeNormal(),Delta.Size()) : float(Delta.Size());
    FHitResult Hit;
    Collision->SetCollisionEnabled(bEnableAvoidance ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    AddActorWorldOffset(Delta.GetSafeNormal()*Allowed,bEnableAvoidance,&Hit);
    if ((GetActorLocation()-Location).SizeSquared()<1 && Speed>1) TargetAge=11;
}

void AEnvironmentFish::ShowSwimBounds()
{
    DrawDebugBox(GetWorld(),GetActorLocation(),SwimExtent.GetAbs(),FColor::Cyan,false,8.f,0,2.f);
}
