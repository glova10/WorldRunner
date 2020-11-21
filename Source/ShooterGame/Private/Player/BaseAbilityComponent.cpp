// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAbilityComponent.h"

int UBaseAbilityComponent::AbilitiComponentsCount = 0;
int UBaseAbilityComponent::AbilitiComponentsTickCount = 0;

// Sets default values for this component's properties
UBaseAbilityComponent::UBaseAbilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBaseAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
    OwnerCharacter = Cast<AShooterCharacter>(GetOwner());
    AbilitComponentCurIdx = AbilitiComponentsCount;
    AbilitiComponentsCount++;
	// ...	
}


// Called every frame
void UBaseAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    DebugInfo = "";
    UpdateComponent(DeltaTime, TickType, ThisTickFunction);

    bool first_component = AbilitiComponentsCount <= AbilitiComponentsTickCount;
    if (first_component)
    {
        AbilitiComponentsTickCount = 0;
    }
    if (GEngine)
    {
        if (first_component)
        {
            GEngine->ClearOnScreenDebugMessages();
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 200, FColor::White, DebugInfo);
    }
    AbilitiComponentsTickCount++;
	// ...
}


void UBaseAbilityComponent::UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

}

FORCEINLINE class UCapsuleComponent* UBaseAbilityComponent::GetCapsuleComponent() const
{
    return OwnerCharacter ? OwnerCharacter->GetCapsuleComponent() : nullptr;
}

FORCEINLINE class UCharacterMovementComponent* UBaseAbilityComponent::GetCharacterMovement() const
{
    return OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
}

FORCEINLINE FVector UBaseAbilityComponent::GetActorLocation() const
{
    return GetCharacter()->GetActorLocation();
}

FORCEINLINE FVector UBaseAbilityComponent::GetVelocity() const
{
    return GetCharacter()->GetVelocity();
}

FORCEINLINE FVector UBaseAbilityComponent::GetActorForwardVector() const
{
    return GetCharacter()->GetActorForwardVector();
}

FORCEINLINE FVector UBaseAbilityComponent::GetActorUpVector() const
{
    return GetCharacter()->GetActorUpVector();
}

FORCEINLINE FVector UBaseAbilityComponent::GetActorRightVector() const
{
    return GetCharacter()->GetActorRightVector();
}

FORCEINLINE FRotator UBaseAbilityComponent::GetBaseAimRotation() const
{
    return GetCharacter()->GetBaseAimRotation();
}

void UBaseAbilityComponent::AddDebugInfo(FString info)
{
    DebugInfo += "\n" + info;
}

