// Fill out your copyright notice in the Description page of Project Settings.


#include "DashAbilityComponent.h"
#include "Components/InputComponent.h"

UDashAbilityComponent::UDashAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsToDash = false;
    bCanDash = false;
    fDashBlockRunDuration = -1.f;
    fDashBlockRunDelay = -1.f;
    fDashBlockMoveDuration = -1.f;
    fDashBlockMoveDelay = -1.f;
    fDashCooldown = 1.f;
    fDashImpulseVelocity = 800.f;
    fDashImpulseVelocityInAirFactor = 0.2f;
    fDashImpulseDuration = 0.3f;
    DashVelocity = FVector::ZeroVector;
    fRadialBlurDownSpeed = 2.f;

}

void UDashAbilityComponent::UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::UpdateComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateDashing(DeltaTime);
    
    if (RadialBlurValue > 0.f)
    {
        RadialBlurValue = FMath::FInterpTo(RadialBlurValue, 0.f, DeltaTime, fRadialBlurDownSpeed);
        if (PostProcessCollectionInstance)
        {
            PostProcessCollectionInstance->SetScalarParameterValue(FName("RadialBlur"), RadialBlurValue);
        }
    }
}

void UDashAbilityComponent::UpdateDashing(float dt)
{
    if (GetCharacter()->IsAlive() && bWantsToDash)
    {
        if (GetCharacterMovement()->IsFalling())
        {
            GetCharacterMovement()->Velocity += DashVelocity * fDashImpulseVelocityInAirFactor * dt;
        }
        else
        {
            GetCharacterMovement()->Velocity += DashVelocity * dt;
        }
        GetCharacterMovement()->UpdateComponentVelocity();

    }
    else if (!GetCharacterMovement()->IsFalling())
    {
        bCanDash = true;
    }
}

void UDashAbilityComponent::OnStartDash()
{
    if (CanDash())
    {
        EnableMovementFromDash();
        EnableGravity();
        UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::OnStartDash("));
        bWantsToDash = true;
        bCanDash = false;
        // this will block dash for period of dash cooldown
        GetCharacter()->GetWorldTimerManager().SetTimer(DashEnableTimer, this, &UDashAbilityComponent::OnStopDash, fDashCooldown);
        GetCharacter()->GetWorldTimerManager().SetTimer(DashVelocityDisableTimer, this, &UDashAbilityComponent::DisableDashVeloctity, fDashImpulseDuration);
        FVector input_world = GetCharacterMovement()->GetLastInputVector();
        FVector forward = GetBaseAimRotation().Vector();
        if (!input_world.Normalize() || FVector::DotProduct(forward.GetSafeNormal2D(), input_world) > 0.5f)
            input_world = forward; //GetActorRotation().Vector().GetUnsafeNormal(); // uncomment this to get 2d dash 
        if (PostProcessCollectionInstance)
        {
            PostProcessCollectionInstance->SetScalarParameterValue(FName("RadialBlur"), 1.f);
        }
        //UKismetSystemLibrary::DrawDebugArrow(GetWorld(), GetActorLocation(), GetActorLocation() + input_world * fDashImpulseVelocity, 1.0f, FColor::Red, 5.f);
        /*if (GetCharacterMovement()->IsFalling())
            LaunchCharacter(input_world * fDashImpulseVelocityInAirFactor * fDashImpulseVelocity, true, false);
        else*/
        DashVelocity = input_world * fDashImpulseVelocity;


        if (fDashBlockMoveDuration > 0.f)
        {
            if (fDashBlockMoveDelay > 0.f)
            {
                GetCharacter()->GetWorldTimerManager().SetTimer(MoveDisableTimer, this, &UDashAbilityComponent::DisableMovementFromDash, fDashBlockMoveDelay);
            }
            else
            {
                DisableMovementFromDash();
            }
        }
        if (fDashBlockRunDuration > 0.f)
        {
            if (fDashBlockMoveDelay > 0.f)
            {
                GetCharacter()->GetWorldTimerManager().SetTimer(RunDisableTimer, this, &UDashAbilityComponent::DisableRunFromDash, fDashBlockMoveDelay);
            }
            else
            {
                DisableRunFromDash();
            }
        }
    }
}

void UDashAbilityComponent::OnStopDash()
{
    bWantsToDash = false;
    DashVelocity = FVector::ZeroVector;
}

void UDashAbilityComponent::DisableMovementFromDash()
{
    GetCharacterMovement()->MaxWalkSpeed = 0.f;
    //bBlockMovement = true;
    GetCharacter()->GetWorldTimerManager().SetTimer(MoveEnableTimer, this, &UDashAbilityComponent::EnableMovementFromDash, fDashBlockMoveDuration);
}

void UDashAbilityComponent::EnableMovementFromDash()
{
    GetCharacterMovement()->MaxWalkSpeed = GetCharacter()->GetMaxWalkSpeed();
}

void UDashAbilityComponent::DisableRunFromDash()
{
    GetCharacter()->GetWorldTimerManager().SetTimer(RunEnableTimer, this, &UDashAbilityComponent::EnableRunFromDash, fDashBlockRunDuration);
}

void UDashAbilityComponent::EnableRunFromDash()
{
    GetCharacterMovement()->MaxWalkSpeed = GetCharacter()->GetMaxWalkSpeed();
    //bBlockMovement = false;
}

void UDashAbilityComponent::DisableDashVeloctity()
{
    DashVelocity = FVector::ZeroVector;
    if (PostProcessCollectionInstance)
    {
        PostProcessCollectionInstance->SetScalarParameterValue(FName("RadialBlur"), 1.f);
    }
    RadialBlurValue = 1.f;
    //LaunchCharacter(FVector(0.f, 0.f, GetCharacterMovement()->Velocity.Z), true, true);	
    GetCharacterMovement()->StopMovementImmediately();
}

void UDashAbilityComponent::EnableGravity()
{
    GetCharacterMovement()->GravityScale = 1.f;
}

void UDashAbilityComponent::CancelDash()
{
    if (bWantsToDash)
    {
        DisableDashVeloctity();
        GetCharacter()->GetWorldTimerManager().TimerExists(MoveDisableTimer);
        EnableMovementFromDash();
        OnStopDash();
    }
}

void UDashAbilityComponent::ResetCanDash(float no_gravity_duration)
{
    bCanDash = true;
    bWantsToDash = false;
    //JumpMaxCount++;
    //GetCharacterMovement()->StopMovementImmediately();
    CancelDash();
    GetCharacterMovement()->GravityScale = 0.f;
    GetCharacterMovement()->MaxWalkSpeed = 150.f;
    //DisableMovementFromDash();
    GetCharacter()->GetWorldTimerManager().SetTimer(GravityEnableTimer, this, &UDashAbilityComponent::EnableGravity, no_gravity_duration);
    GetCharacter()->GetWorldTimerManager().SetTimer(MoveEnableTimer, this, &UDashAbilityComponent::EnableMovementFromDash, no_gravity_duration);
}

void UDashAbilityComponent::PostInitializeComponents()
{
    if (PostProcessCollection)
    {
        PostProcessCollectionInstance = GetWorld()->GetParameterCollectionInstance(PostProcessCollection);
    }
}

void UDashAbilityComponent::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &UDashAbilityComponent::OnStartDash);
}