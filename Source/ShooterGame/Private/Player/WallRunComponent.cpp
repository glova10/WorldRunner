// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunComponent.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "ShooterCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/InputComponent.h"

// Sets default values for this component's properties
UWallRunComponent::UWallRunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWallRunComponent::BeginPlay()
{
	Super::BeginPlay();
    if (GetCharacter()->GetCapsuleComponent())
    {
        GetCharacter()->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UWallRunComponent::OnHitCallback);
    }
    GetCharacterMovement()->SetPlaneConstraintEnabled(true);
    fDefaultAirControll = GetCharacterMovement()->AirControl;

    if (CameraRollCurve)
    {
        FOnTimelineFloat TimeLineCallBack;

        TimeLineCallBack.BindUFunction(this, FName("CameraRollOnTimeLine"));

        CameraTimeLine = NewObject<UTimelineComponent>(this, FName("CamRollAnimation"));
        CameraTimeLine->AddInterpFloat(CameraRollCurve, TimeLineCallBack);
        CameraTimeLine->RegisterComponent();
    }
	// ...	
}

// Called every frame
void UWallRunComponent::UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::UpdateComponent(DeltaTime, TickType, ThisTickFunction);    
    
    if (!GetCharacter()->IsAlive())
        return;
    
    if (CameraTimeLine)
    {
        CameraTimeLine->TickComponent(DeltaTime, ELevelTick::LEVELTICK_TimeOnly, NULL);
    }

    UpdateWallRun();
	// ...
}

void UWallRunComponent::OnJumpStart()
{
    if (IsWallRunning())
    {
        FVector launch_vector = FindLaunchVelocity();
        GetCharacter()->LaunchCharacter(launch_vector, false, true);
        EndWallRun(EWallRunEndReason::Jumped);
    }
}

void UWallRunComponent::BeginWallrun()
{
    bWallRunning = true;
        
    GetCharacterMovement()->AirControl = 1.f;
    GetCharacterMovement()->GravityScale = 0.f;        
    //GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.f, 0.f, 1.f));
    FVector launch_vector = (vWallrunDirection * GetCharacterMovement()->GetMaxSpeed()).GetSafeNormal2D();
    GetCharacter()->LaunchCharacter(launch_vector, true, true);
    fWallRunStartTime = GetWorld()->GetTimeSeconds();
    BeginCameraTilt();
}

void UWallRunComponent::EndWallRun(EWallRunEndReason::Type reason)
{
    bWallRunning = false;
    if (reason == EWallRunEndReason::Jumped)
    {
        // modify jump count
    }
    else
    {

    }
    EndCameraTilt();
    GetCharacterMovement()->AirControl = fDefaultAirControll;
    GetCharacterMovement()->GravityScale = 1.f;
    GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.f, 0.f, 0.f));
}

void UWallRunComponent::BeginCameraTilt()
{
    if (CameraTimeLine)
        CameraTimeLine->PlayFromStart();
}

void UWallRunComponent::EndCameraTilt()
{
    if (CameraTimeLine)
        CameraTimeLine->Reverse();
}

void UWallRunComponent::FindWallRunDirAndSide(FVector surface_normal)
{
    FVector right = GetActorRightVector().GetSafeNormal2D();
    FVector surf_2d = surface_normal.GetSafeNormal2D();
    vWallSurfaceNormal = surface_normal;

    if (FVector::DotProduct(right, surf_2d) > 0.f)
    {
        eWallRunSide = EWallRunSide::Left;
        vWallrunDirection = FVector::CrossProduct(surface_normal, FVector::UpVector);
    }
    else
    {
        eWallRunSide = EWallRunSide::Right;
        vWallrunDirection = FVector::CrossProduct(surface_normal, FVector::DownVector);
    }
    if (bDrawDebugWallRun)
    {
        FColor color = eWallRunSide == EWallRunSide::Left ? FColor::Green : FColor::Yellow;
        UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + vWallrunDirection * 50.f, 10.0f, color, 5.f, 2.f);
    }
}

bool UWallRunComponent::CanSurfaceBeWallran(FVector surface_normal) const
{
    if (surface_normal.Z < -0.05f)
        return false;

    float dot = FVector::DotProduct(surface_normal, surface_normal.GetSafeNormal2D());
    float angle = FMath::Acos(dot);

    return angle < GetCharacterMovement()->GetWalkableFloorAngle();
}

FVector UWallRunComponent::FindLaunchVelocity() const
{
    if (IsWallRunning())
    {
        float scale = GetCharacterMovement()->JumpZVelocity;
        if (EWallRunSide::Right == eWallRunSide)
        {
            return  scale * (FVector::CrossProduct(vWallrunDirection, FVector::UpVector) + FVector::UpVector);
        }
        else
        {
            return  scale * (FVector::CrossProduct(vWallrunDirection, FVector::DownVector) + FVector::UpVector);
        }
    }
    return FVector::ZeroVector;
}

bool UWallRunComponent::AreRequiredKeysDown() const
{
    return bWallInputPressed;
    /*if (GetCharacter()->GetMoveInputLocal().Y < 0.25f)
        return false;

    if (eWallRunSide == EWallRunSide::Left)
    {
        return GetCharacter()->GetMoveInputLocal().X > 0.25f;
    }
    return GetCharacter()->GetMoveInputLocal().X < -0.25f;*/
}

bool UWallRunComponent::CanContinueWallRun() const
{
    if (!AreRequiredKeysDown())
        return false;

    if (IsWallInFront())
        return false;

    return true;
}

bool UWallRunComponent::IsWallInFront() const
{
    if (FVector::Dist2D(GetActorLocation(), vWallInFrontPos) < 100.f)
    {
        if (IsWallRunning() && FVector::DotProduct(vWallrunDirection, vWallInFrontImpactNormal) < 0.f)
            return true;
        if (!IsWallRunning() && !vWallInFrontImpactNormal.IsZero())
        {
            float angle = FMath::Acos(FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(), vWallInFrontImpactNormal.GetSafeNormal2D()));

            return fWallRunAngleToStartWallRun < abs(angle);
        }
    }
    return false;
}

FVector2D UWallRunComponent::GetHorizontalVelocity() const
{
    return FVector2D(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y);
}

void UWallRunComponent::SetHorizontalVelocity(FVector2D velocity)
{
    GetCharacterMovement()->Velocity = FVector(velocity.X, velocity.Y, GetCharacterMovement()->Velocity.Z);
    GetCharacterMovement()->UpdateComponentVelocity();
}

void UWallRunComponent::UpdateWallInFront()
{
    TArray<AActor*> actors_to_ignore;

    // create tarray for hit results
    FHitResult OutHits;

    // start and end locations
    FVector SweepStart = GetActorLocation();
    FVector forward = GetBaseAimRotation().Vector();
    forward.Z = 0.f;
    forward.Normalize();
    FVector SweepEnd = GetActorLocation() + forward * fWallFrontDetectLength;

    const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius() / 2.f;
    const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() / 2.f;

    if (UKismetSystemLibrary::CapsuleTraceSingle(this, SweepStart, SweepEnd, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
        false, actors_to_ignore, /*bDrawDebugClimb ? EDrawDebugTrace::ForOneFrame :*/ EDrawDebugTrace::None, OutHits, true, FLinearColor::Yellow, FLinearColor(FColor::Orange), -1.f)
        /*GetWorld()->SweepSingleByChannel(OutHits, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, MyColCapsule)*/)
    {
        vWallInFrontImpactNormal = OutHits.ImpactNormal;
        vWallInFrontPos = OutHits.ImpactPoint;
        if (bDrawDebugWallRun)
        {
            UKismetSystemLibrary::DrawDebugArrow(this, vWallInFrontPos, vWallInFrontPos + vWallInFrontImpactNormal * 50.f, 10.0f, FColor::Purple, 5.f, 2.f);
            float angle = FMath::Acos(FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(), vWallInFrontImpactNormal.GetSafeNormal2D()));
            AddDebugInfo(FString::Printf(TEXT("Angle To Wall: %f"), angle));
        }
    }
    else
    {
        vWallInFrontImpactNormal = FVector::ZeroVector;
        vWallInFrontPos = FVector::ZeroVector;
    }
}

float UWallRunComponent::GetWallRunDuration() const
{
    if (!IsWallRunning())
        return 0.f;
    return GetWorld()->GetTimeSeconds() - fWallRunStartTime;
}

void UWallRunComponent::UpdateWallRun()
{
    UpdateWallInFront();
    if (IsWallRunning())
    {
        if (eWallRunSide == EWallRunSide::Left)
        {
            AddDebugInfo(FString::Printf(TEXT("Left")));
        }
        else
        {
            AddDebugInfo(FString::Printf(TEXT("Right")));
        }
        AddDebugInfo(FString::Printf(TEXT("Time In Wallrun: %f"), GetWallRunDuration()));

        if (!CanContinueWallRun())
        {
            EndWallRun(EWallRunEndReason::FallOff);
            return;
        }
        TArray<AActor*> actors_to_ignore;

        // create tarray for hit results
        FHitResult OutHits;

        // start and end locations
        FVector SweepStart = GetActorLocation();
        FVector dir = FVector::CrossProduct(vWallrunDirection, FVector::UpVector);
        if (eWallRunSide == EWallRunSide::Right)
            dir = FVector::CrossProduct(vWallrunDirection, FVector::DownVector);
        FVector SweepEnd = GetActorLocation() + dir * fWallRunDetectLength;

        const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
        const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

        if (UKismetSystemLibrary::CapsuleTraceSingle(this, SweepStart, SweepEnd, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
            false, actors_to_ignore, /*bDrawDebugClimb ? EDrawDebugTrace::ForOneFrame :*/ EDrawDebugTrace::None, OutHits, true, FLinearColor::Yellow, FLinearColor(FColor::Orange), -1.f)
            /*GetWorld()->SweepSingleByChannel(OutHits, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, MyColCapsule)*/)
        {
            EWallRunSide::Type prev_side = eWallRunSide;
            FindWallRunDirAndSide(OutHits.ImpactNormal);
            if (prev_side != eWallRunSide)
            {
                EndWallRun(EWallRunEndReason::FallOff);
                return;
            }
            FVector wall_run_velocit = vWallrunDirection.GetSafeNormal2D() * GetCharacterMovement()->GetMaxSpeed();
            if (GetWallRunDuration() > fWallRunTimeToNulifyZ) ///free conttraint so we can move up / down on wallrun
            {
                AddDebugInfo("Look Rotation: " + GetBaseAimRotation().ToString());
                AddDebugInfo("Wall Surface Normal: " + vWallSurfaceNormal.ToString());                
                wall_run_velocit = wall_run_velocit.RotateAngleAxis(GetBaseAimRotation().Pitch, 
                    eWallRunSide == EWallRunSide::Left ? -vWallSurfaceNormal : vWallSurfaceNormal);
            }
            AddDebugInfo("Curr Vel: " + GetCharacterMovement()->Velocity.ToString());
            GetCharacterMovement()->Velocity = wall_run_velocit;            
            AddDebugInfo("Wall Vel: " + wall_run_velocit.ToString());
            //GEngine->AddOnScreenDebugMessage(-1, 200, FColor::Red, FString::Printf(TEXT("Angle To Wall: %f"), angle));
        }
        else
        {
            EndWallRun(EWallRunEndReason::FallOff);
            return;
        }
//         if (GetWallRunDuration() > fWallRunTimeToNulifyZ && 
//             !GetCharacterMovement()->GetPlaneConstraintNormal().IsZero()) ///free conttraint so we can move up / down on wallrun
//         {
//             GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.f, 0.f, 0.f));
//         }
            
    }
}

void UWallRunComponent::OnHitCallback(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (IsWallRunning())
        return; // do nothing we are on wall

    if (!CanSurfaceBeWallran(Hit.ImpactNormal))
        return;

    if (!GetCharacterMovement()->IsFalling())
        return;

    if (GetCharacter()->IsClimbing())
        return;

    if (GetCharacter()->GetTimeInAir() < fWallRunMinTimeInAirToStart)
        return;

    if (GetCharacterMovement()->Velocity.Z > fWallRunMinZVelToStart)
        return;

    if (IsWallInFront())
        return;

    FindWallRunDirAndSide(Hit.ImpactNormal);

    if (!AreRequiredKeysDown())
        return;

    BeginWallrun();
}

void UWallRunComponent::CameraRollOnTimeLine()
{
    CamRollTimelineValue = CameraTimeLine->GetPlaybackPosition();
    CamRollCurveFloatValue = CameraRollCurve->GetFloatValue(CamRollTimelineValue);
    if (eWallRunSide == EWallRunSide::Right)
        CamRollCurveFloatValue *= -1.f;
    // set cam roll
    if (!GetCharacter() || !GetCharacter()->GetController())
        return;

    FRotator rotation = GetCharacter()->GetController()->GetControlRotation();
    rotation.Roll = CamRollCurveFloatValue;
    GetCharacter()->GetController()->SetControlRotation(rotation);
}

void UWallRunComponent::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction("WallRun", IE_Pressed, this, &UWallRunComponent::OnWallrunInputPressed);
    PlayerInputComponent->BindAction("WallRun", IE_Released, this, &UWallRunComponent::OnWallrunInputReleased);
}
