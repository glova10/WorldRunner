// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseAbilityComponent.generated.h"

class UCharacterMovementComponent;
class AShooterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTERGAME_API UBaseAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBaseAbilityComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    FORCEINLINE class UCapsuleComponent* GetCapsuleComponent() const;
    FORCEINLINE class UCharacterMovementComponent* GetCharacterMovement() const;
    FORCEINLINE class AShooterCharacter* GetCharacter() const { return OwnerCharacter; }
    FORCEINLINE FVector GetActorLocation() const;
    FORCEINLINE FVector GetVelocity() const;
    FORCEINLINE FVector GetActorForwardVector() const;
    FORCEINLINE FVector GetActorUpVector() const;
    FORCEINLINE FVector GetActorRightVector() const;
    FORCEINLINE FRotator GetBaseAimRotation() const;
    void AddDebugInfo(FString info);
    virtual bool IsActive() const { return false; }
public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void UpdateComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
private:

    AShooterCharacter* OwnerCharacter;
    FString DebugInfo = "";    
    static int AbilitiComponentsCount;
    static int AbilitiComponentsTickCount;
    int AbilitComponentCurIdx;
    /** Returns CharacterMovement subobject **/
		
};
