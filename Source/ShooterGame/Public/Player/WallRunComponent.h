// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseAbilityComponent.h"
#include "WallRunComponent.generated.h"

class UTimelineComponent;

namespace EWallRunSide
{
    enum Type
    {
        Left,
        Right,
    };
};

namespace EWallRunEndReason
{
    enum Type
    {
        FallOff,
        Jumped,
    };
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTERGAME_API UWallRunComponent : public UBaseAbilityComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWallRunComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
public:
    // Called every frame
    virtual void UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual bool IsActive() const { return IsWallRunning(); }
 //////////////////////////////////////////////////////////////////////////
//  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Runing")
        float fWallRunAngleToStartWallRun = 130.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Runing")
        float fWallRunDetectLength = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Runing")
        float fWallFrontDetectLength = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Runing")
        float fWallRunMinTimeInAirToStart = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Runing")
        float fWallRunMinZVelToStart = -50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Runing")
        bool bDrawDebugWallRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Runing")
        float fWallRunTimeToNulifyZ = 1.f;

    UPROPERTY(EditAnywhere)
    UCurveFloat* CameraRollCurve;



    bool IsWallRunning() const { return bWallRunning;}
    void OnJumpStart();
private:

    void BeginWallrun();
    void EndWallRun(EWallRunEndReason::Type reason);
    void BeginCameraTilt();
    void EndCameraTilt();
    void OnWallrunInputPressed() { bWallInputPressed = true; }
    void OnWallrunInputReleased() { bWallInputPressed = false; }

    void FindWallRunDirAndSide(FVector surface_normal);
    bool CanSurfaceBeWallran(FVector surface_normal) const;
    FVector FindLaunchVelocity() const;
    bool AreRequiredKeysDown() const;
    bool CanContinueWallRun() const;
    bool IsWallInFront() const;
    FVector2D GetHorizontalVelocity() const;
    void SetHorizontalVelocity(FVector2D velocity);
    void UpdateWallRun();
    void UpdateWallInFront();
    float GetWallRunDuration() const;

    UFUNCTION()
        void OnHitCallback(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    FVector vWallrunDirection = FVector::ForwardVector;
    FVector vWallSurfaceNormal = FVector::LeftVector;
    bool bWallRunning = false;
    bool bWallInputPressed = false;
    int iJumpLeft = 0;
    float fRightAxis = 0.f;
    float fForwardAxis = 0.f;
    float fDefaultAirControll = 0.f;    
    float fWallRunStartTime = 0.f;
    EWallRunSide::Type eWallRunSide = EWallRunSide::Left;
    FVector vWallInFrontPos = FVector::ZeroVector;
    FVector vWallInFrontImpactNormal = FVector::ZeroVector;

    UFUNCTION()
    void CameraRollOnTimeLine();

    float CamRollValue = 0.f;
    float CamRollCurveFloatValue = 0.f;
    float CamRollTimelineValue = 0.f;
    UTimelineComponent* CameraTimeLine;
public:
    void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
};