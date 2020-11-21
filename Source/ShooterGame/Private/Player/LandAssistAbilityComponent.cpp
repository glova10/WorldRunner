// Fill out your copyright notice in the Description page of Project Settings.


#include "LandAssistAbilityComponent.h"
#include "BaseAbilityComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ULandAssistAbilityComponent::ULandAssistAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;    
}

void ULandAssistAbilityComponent::UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::UpdateComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateLandAssist();
}

void ULandAssistAbilityComponent::UpdateLandAssist()
{
    if (!GetCharacterMovement()->IsFalling() && !GetCharacter()->IsDashing() ||
        GetCharacter()->IsClimbing())
    {

        if (!VVelBeforeLandAssist.IsZero())
        {
            FVector land_world_pos = LandComponent ? LandComponent->GetComponentTransform().TransformPosition(VLandAssistPos) : VLandAssistPos;
            if (abs(land_world_pos.Z - GetActorLocation().Z) < 3.f)// whe we've landed properly we can add impulse
            {
                FVector input_world = GetCharacterMovement()->GetLastInputVector();
                GetCharacter()->LaunchCharacter(input_world * VVelBeforeLandAssist.Size2D(), true, false);
            }
        }
        VVelBeforeLandAssist = FVector::ZeroVector;
        VLandAssistPos = FVector::ZeroVector;
        LandComponent = nullptr;
        bLandAssisWasSpaceValid = false;
        return;
    }
    if (!VLandAssistPos.IsZero())
    {
        UpdateLandVelocity();
        return;
    }

    TArray<AActor*> actors_to_ignore;

    // create tarray for hit results
    FHitResult OutHits;

    FVector SweepStart = GetActorLocation();
    FVector forward = GetBaseAimRotation().Vector();
    forward.Z = 0.f;
    forward.Normalize();
    FVector SweepEnd = GetActorLocation() + FVector::DownVector * fMinZSpaceToLandAssist;

    const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    FVector land_pos = FVector::ZeroVector;
    if (VLandAssistPos.IsZero())
    {
        if (UKismetSystemLibrary::CapsuleTraceSingle(this, SweepStart, SweepEnd, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
            false, actors_to_ignore, /*bDrawDebugClimb ? EDrawDebugTrace::ForOneFrame :*/ EDrawDebugTrace::None, OutHits, true, FLinearColor::Yellow, FLinearColor(FColor::Orange), -1.f))
        {

            float dist = SweepStart.Z - OutHits.ImpactPoint.Z;
            if (dist < fZSpaceToLandAssist && FVector::DotProduct(OutHits.ImpactNormal, FVector::UpVector) > 0.99f)
            {
                land_pos = OutHits.ImpactPoint;
                if (fDebugRenderLandAssistTime >= 0.f)
                    UKismetSystemLibrary::DrawDebugArrow(this, SweepStart, OutHits.ImpactPoint, 30.f, FLinearColor::Blue, fDebugRenderLandAssistTime);
            }
        }
        else
        {
            bLandAssisWasSpaceValid = true;
            if (fDebugRenderLandAssistTime >= 0.f)
                UKismetSystemLibrary::DrawDebugArrow(this, SweepStart, SweepEnd, 15.f, FLinearColor::Green, fDebugRenderLandAssistTime);
        }
    }

    if (GetCharacterMovement()->Velocity.Z > fMinZVelocityToLandAssist)
        return; // no help in ascending

    if (land_pos.Z < GetActorLocation().Z)
        return; // we are bellow land pos to late

    if ((land_pos.IsZero() || !bLandAssisWasSpaceValid) && VLandAssistPos.IsZero())
        return;

    LandComponent = OutHits.GetComponent();
    VLandAssistPos = LandComponent ? LandComponent->GetComponentTransform().InverseTransformPosition(land_pos) : land_pos;
    VVelBeforeLandAssist = GetVelocity();
    GetCharacter()->OnLandAssistStart();    
    UpdateLandVelocity();
    bLandAssisWasSpaceValid = false;
}

void ULandAssistAbilityComponent::UpdateLandVelocity()
{
    FVector land_world_pos = LandComponent ? LandComponent->GetComponentTransform().TransformPosition(VLandAssistPos) : VLandAssistPos;
    if (fDebugRenderLandAssistTime >= 0.f)
    {
        UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(),
            land_world_pos, 45.f, FLinearColor::Red, fDebugRenderLandAssistTime);
        if (LandComponent)
        {
            FVector actor_position = LandComponent->GetComponentLocation();
            UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(),
                actor_position, 15.f, FLinearColor::Yellow, fDebugRenderLandAssistTime);
        }
    }
    if (land_world_pos.Z < GetActorLocation().Z)
        return; // we are bellow land pos to late

    if (GetCharacterMovement()->Velocity.Z < 0.f)
    {// only when descending
        // compute time in Z velocity - we will use it to set 2d vel
        const float time_to_land = (GetActorLocation().Z - land_world_pos.Z) / -GetCharacterMovement()->Velocity.Z;
        // distance for calculating velocity
        const float dist_2_land2d = FVector::Dist2D(GetActorLocation(), land_world_pos);
        // dir in wolrd space to land pos
        const FVector dir_to_set = (land_world_pos - GetActorLocation()).GetSafeNormal2D();
        // dir * velocity_computed from v = s / t;
        const FVector velo2d_to_set = dir_to_set * dist_2_land2d / time_to_land;

        GetCharacter()->LaunchCharacter(FVector(velo2d_to_set.X, velo2d_to_set.Y, GetCharacterMovement()->Velocity.Z), true, true);
    }
}
