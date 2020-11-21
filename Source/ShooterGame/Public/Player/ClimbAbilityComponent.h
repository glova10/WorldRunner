// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/BaseAbilityComponent.h"
#include "ClimbAbilityComponent.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UClimbAbilityComponent : public UBaseAbilityComponent
{
    GENERATED_BODY()
public:
    // Called every frame
    virtual void UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual bool IsActive() const { return IsClimbing(); }
//////////////////////////////////////////////////////////////////////////
// Climb settings
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
       float fCLimbForwardRange = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fCLimbUpRange = 160.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fMinWallThicknes = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fClimbingSpeed = 650.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fClimbingForwardSpeed = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fClimbingMaxTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fMaxVelocityToClimb = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fClimbingZEndOffset = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        bool bDrawDebugClimb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
        float fWallInFrontMinAngleToStartClimb = 2.4f;
public:
    bool IsClimbing() const { return bIsClimbing; }
private:
    
    void UpdateClimbing();
    
    // detecs clibing position in fron of the player
    FVector GetClimbPos(); 
    bool CanClimb() const;
    void DisableClimbing();

    // information about the climbing
    bool bIsClimbing = false;
    // climbing position that we are using
    FVector ClimbingPos = FVector::ZeroVector;
    // position that character started climb
    FVector ClimbingStartPos = FVector::ZeroVector;
    /**timer that enables dashing after cooldown*/
    FTimerHandle ClimbDisableTimer;

    FVector vWallInFrontPos = FVector::ZeroVector;
    FVector vWallInFrontImpactNormal = FVector::ZeroVector;
};
