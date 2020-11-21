// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbAbilityComponent.h"
#include "BaseAbilityComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UClimbAbilityComponent::UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::UpdateComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateClimbing();
}

void UClimbAbilityComponent::UpdateClimbing()
{
    if (bIsClimbing)
    {
        if (GetActorLocation().Z + fClimbingZEndOffset > ClimbingPos.Z + GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
        {
            DisableClimbing();
            UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::UpdateClimbing - CANCEL OFFSET REACHED"));
            return;
        }
        FVector climbing_dir = (ClimbingPos - ClimbingStartPos).GetSafeNormal2D();
        FVector climbing_impuls = climbing_dir * fClimbingForwardSpeed;
        climbing_impuls.Z = fClimbingSpeed;
        GetCharacter()->LaunchCharacter(climbing_impuls, true, true);
    }
    if (!CanClimb())
        return;

    FVector climb_pos = GetClimbPos();

    if (climb_pos.IsZero())
        return;

    FVector input_world = GetCharacterMovement()->GetLastInputVector();
    float lenght = input_world.Size();
    if (!input_world.Normalize() || lenght < 0.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::UpdateClimbing - NO INPUT"));
        return;
    }
    FVector climbing_dir = (climb_pos - GetActorLocation()).GetSafeNormal2D();

    if (FVector::DotProduct(input_world, climbing_dir) < 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::UpdateClimbing - BAD INPUT DOT"));
        return;
    }
    GetCharacter()->OnClimbStart();
    ClimbingPos = climb_pos;
    ClimbingStartPos = GetActorLocation();
    bIsClimbing = true;
    GetCharacter()->GetWorldTimerManager().SetTimer(ClimbDisableTimer, this, &UClimbAbilityComponent::DisableClimbing, fClimbingMaxTime);
}

FVector UClimbAbilityComponent::GetClimbPos()
{
    static float UP_TOLLERANCE = 3.f;
    TArray<AActor*> actors_to_ignore;

    // create tarray for hit results
    FHitResult OutHits;

    // start and end locations
    FVector SweepStart = GetActorLocation();
    FVector forward = GetBaseAimRotation().Vector();
    forward.Z = 0.f;
    forward.Normalize();
    FVector SweepEnd = GetActorLocation() + forward * fCLimbForwardRange;

    /*if (bDrawDebugClimb)
        UKismetSystemLibrary::DrawDebugArrow(GetWorld(), SweepStart, SweepEnd, 5.f, FLinearColor::Green);*/
        // create a collision sphere

    const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    //FCollisionShape MyColCapsule = FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(), CapsuleHalfHeight);
    // draw collision sphere		

    // check if something got hit in the sweep
    //bool isHit = GetWorld()->SweepMultiByChannel(OutHits, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, MyColSphere);

    if (UKismetSystemLibrary::CapsuleTraceSingle(this, SweepStart, SweepEnd, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
        false, actors_to_ignore, /*bDrawDebugClimb ? EDrawDebugTrace::ForOneFrame :*/ EDrawDebugTrace::None, OutHits, true, FLinearColor::Yellow, FLinearColor(FColor::Orange), -1.f)
        /*GetWorld()->SweepSingleByChannel(OutHits, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, MyColCapsule)*/)
    {
        vWallInFrontImpactNormal = OutHits.ImpactNormal;
        vWallInFrontPos = OutHits.ImpactPoint;
        if (bDrawDebugClimb)
        {
            UKismetSystemLibrary::DrawDebugArrow(this, vWallInFrontPos, vWallInFrontPos + vWallInFrontImpactNormal * 50.f, 10.0f, FColor::Purple, 5.f, 2.f);            
        }
        
        float angle = FMath::Acos(FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(), vWallInFrontImpactNormal.GetSafeNormal2D()));
        // angle is to small don't climb
        if (angle < fWallInFrontMinAngleToStartClimb)
        {
            AddDebugInfo(FString::Printf(TEXT("Can't CLIMB angle %f is smaller then %f"), angle, fWallInFrontMinAngleToStartClimb));
            return FVector::ZeroVector;
        }
            

        const FVector forward_hit_point = OutHits.ImpactPoint + forward * fMinWallThicknes;
        FVector vertical_trace_end = FVector(forward_hit_point.X, forward_hit_point.Y, GetActorLocation().Z);
        FVector vertical_trace_begin = vertical_trace_end + FVector::UpVector * fCLimbUpRange;

        if (bDrawDebugClimb)
            UKismetSystemLibrary::DrawDebugArrow(this, SweepStart, OutHits.ImpactPoint, 15.f, FLinearColor::Blue);

        TArray<FHitResult> VerticalOutHits;
        if (UKismetSystemLibrary::LineTraceMulti(this, vertical_trace_begin, vertical_trace_end, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
            false, actors_to_ignore, bDrawDebugClimb ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, VerticalOutHits, true, FLinearColor::Blue, FLinearColor(FColor::Purple), -1.f))
        {
            for (int i = VerticalOutHits.Num() - 1; i >= 0; i--) // go backward because we wan't to find firs lowest Y to climb
            {// we need to validate path
                FHitResult& curr_restult = VerticalOutHits[i];
                FVector climbing_pos = curr_restult.ImpactPoint;
                // lets switch it into 2 moves veritcal end horizontal
                FVector vertical_start = GetActorLocation();
                float up_move = (climbing_pos.Z - GetActorLocation().Z) + CapsuleHalfHeight + UP_TOLLERANCE;
                FVector vertical_end = vertical_start + FVector::UpVector * up_move;

                FHitResult vertical_result;
                if (!UKismetSystemLibrary::CapsuleTraceSingle(this, vertical_start, vertical_end, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
                    false, actors_to_ignore, bDrawDebugClimb ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, vertical_result, true, FLinearColor::White, FLinearColor::Gray, -1.f))
                { // no vertical final check if we have forward empty
                    FVector horizonal_start = vertical_end;
                    float distance2D = FVector::Dist2D(GetActorLocation(), climbing_pos);
                    FVector horizonal_end = horizonal_start + forward * distance2D;

                    FHitResult horizonal_result;
                    if (!UKismetSystemLibrary::CapsuleTraceSingle(this, horizonal_start, horizonal_end, CapsuleRadius, CapsuleHalfHeight, UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
                        false, actors_to_ignore, bDrawDebugClimb ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, horizonal_result, true, FLinearColor::Green, FLinearColor::Red, -1.f))
                    { // everything is cool  - return!
                        if (bDrawDebugClimb)
                        {
                            UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), climbing_pos, 10.0f, FColor::Green, 5.f, 2.f);
                        }
                        return horizonal_end;
                    }
                }
            }
        }
    }
    return FVector::ZeroVector;
}

bool UClimbAbilityComponent::CanClimb() const
{
    if (bIsClimbing)// no climb if he is already climbing
        return false;
    if (GetCharacter()->IsFiring())
        return false;
    if (!GetCharacterMovement()->IsFalling())
        return false;    
    if (GetCharacterMovement()->Velocity.Z > fMaxVelocityToClimb)
    {
        UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::CanClimb - Z Velocity to big"));
        return false;
    }

    return true;
}

void UClimbAbilityComponent::DisableClimbing()
{
    if (!bIsClimbing)
        return;
    bIsClimbing = false;
    ClimbingPos = FVector::ZeroVector;
}
