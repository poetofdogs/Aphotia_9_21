#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentFish.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/** Ambient fish that roam, avoid obstacles, and flee from strong flashes. */
UCLASS(Blueprintable)
class APHOTIA_9_21_API AEnvironmentFish : public AActor
{
    GENERATED_BODY()
public:
    AEnvironmentFish();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fish")
    TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fish")
    TObjectPtr<UStaticMeshComponent> FishMesh;

    /** Half size, in cm, of the world-aligned swimming box around the starting position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|1 Swimming", meta=(ClampMin="50"))
    FVector SwimExtent = FVector(1000, 1000, 400);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|1 Swimming", meta=(ClampMin="0"))
    float SwimSpeed = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|1 Swimming", meta=(ClampMin="1"))
    float TurnSpeed = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|1 Swimming")
    int32 RandomSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|2 Avoidance")
    bool bEnableAvoidance = true;
    /** Covers the whole fish, including nose and tail. Automatically enlarged from mesh bounds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|2 Avoidance", meta=(ClampMin="1"))
    float BodyRadius = 20;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|2 Avoidance", meta=(ClampMin="10"))
    float LookAhead = 180;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|3 Flash", meta=(ClampMin="0.1", Units="s"))
    float FlashFleeDuration = 2.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fish|3 Flash", meta=(ClampMin="1"))
    float FleeSpeedMultiplier = 2.2f;

    /** Called when the player's existing directional flash hitbox is spawned. */
    UFUNCTION(BlueprintCallable, Category="Fish|3 Flash", meta=(DefaultToSelf="FlashHitbox"))
    static void ScareFishInFlashBox(AActor* FlashHitbox);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Fish|Status")
    bool bFleeing = false;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Fish|Status")
    FVector HomeLocation;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Fish|Status")
    FVector TargetLocation;

    UFUNCTION(CallInEditor, Category="Fish|1 Swimming")
    void ShowSwimBounds();
private:
    bool bInitialized = false;
    float TargetAge = 0;
    float FlashFleeTimeRemaining = 0.f;
    FVector FlashOrigin = FVector::ZeroVector;
    float SpeedVariation = 1;
    FVector Heading = FVector::ForwardVector;
    FVector ScatterBias;
    FRandomStream Random;
    void InitializeFish();
    void StepSimulation(float DeltaSeconds);
    FVector EffectiveExtent() const;
    FVector ClampToArea(const FVector& Point) const;
    bool IsInsideArea(const FVector& Point) const;
    void PickTarget();
    void StartFleeingFromFlash(const FVector& Origin);
    float ClearDistance(const FVector& Direction, float Distance) const;
    void SimulateSubstep(float DeltaSeconds);
};
