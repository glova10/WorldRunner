// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/BaseAbilityComponent.h"
#include "LandAssistAbilityComponent.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API ULandAssistAbilityComponent : public UBaseAbilityComponent
{
    GENERATED_BODY()

    ULandAssistAbilityComponent();
public:
    // Called every frame
    virtual void UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual bool IsActive() const { return !VLandAssistPos.IsZero(); }
    //////////////////////////////////////////////////////////////////////////
    // LandAssist
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandAssist")
        float fMinZVelocityToLandAssist = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandAssist")
        float fMinZSpaceToLandAssist = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandAssist")
        float fZSpaceToLandAssist = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandAssist")
        float fDebugRenderLandAssistTime = 10.f;
private:
    void UpdateLandAssist();
    void UpdateLandVelocity();

    // land assist component (used for moving objects)    
    class UPrimitiveComponent* LandComponent = nullptr;
    /* land assist pos in local space of land actor*/
    FVector VLandAssistPos = FVector::ZeroVector;
    /* velocity to regain after land to avoid stop on landing*/
    FVector VVelBeforeLandAssist = FVector::ZeroVector;
    /** current dash state */
    bool bLandAssisWasSpaceValid = false;
};
