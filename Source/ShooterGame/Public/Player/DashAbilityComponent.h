// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/BaseAbilityComponent.h"
#include "DashAbilityComponent.generated.h"

class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;

UCLASS()
class SHOOTERGAME_API UDashAbilityComponent : public UBaseAbilityComponent
{
	GENERATED_BODY()
    UDashAbilityComponent();
public:
    // Called every frame
    virtual void UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual bool IsActive() const { return IsDashing(); }
    void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
    /** player pressed dash action */
    void OnStartDash();

    bool IsDashing() const { return bWantsToDash; }
    void CancelDash();
    void ResetCanDash(float no_gravity_duration);
    void PostInitializeComponents();    
    void DisableDashVeloctity();
    bool CanDash() const { return bCanDash && !bWantsToDash; }
public:
        /**how much time (in seconds ) between dash*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashCooldown = 0.55f;

    /**how much time (in seconds ) to apply dash impulse*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashImpulseDuration = 0.1f;

    /**how much time (in seconds ) between dash*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashImpulseVelocity = 250000.f;

    /**how much time (in seconds ) between dash*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashImpulseVelocityInAirFactor;

    /**when (in sec) to block movement of the player after dash)*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashBlockMoveDelay;
    /**for how long (in sec) the movement is blocked*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashBlockMoveDuration;

    /**when (in sec) to block running of the player after dash)*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashBlockRunDelay;
    /**for how long (in sec) running is blocked*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fDashBlockRunDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
        float fRadialBlurDownSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
        UMaterialParameterCollection* PostProcessCollection;

private:    
    void UpdateDashing(float dt);    

    /** player pressed dash action */
    void OnStopDash();

    void DisableMovementFromDash();
    void EnableMovementFromDash();

    void DisableRunFromDash();
    void EnableRunFromDash();    
    void EnableGravity();

    /** current dash state */
    uint8 bWantsToDash : 1;

    /** cand dash ie has landed on ground*/
    uint8 bCanDash : 1;

    /**timer that enables dashing after cooldown*/
    FTimerHandle DashEnableTimer;

    /**timer that enables dashing after cooldown*/
    FTimerHandle DashVelocityDisableTimer;

    /**timer that enables dashing after cooldown*/
    FTimerHandle MoveDisableTimer;
    /**timer that enables dashing after cooldown*/
    FTimerHandle MoveEnableTimer;

    /**timer that enables dashing after cooldown*/
    FTimerHandle RunDisableTimer;
    /**timer that enables dashing after cooldown*/
    FTimerHandle RunEnableTimer;

    /*veloctiy that is applied every frame*/
    FVector DashVelocity;

    FTimerHandle GravityEnableTimer;

    UMaterialParameterCollectionInstance* PostProcessCollectionInstance;

    float RadialBlurValue = 0.f;

};
