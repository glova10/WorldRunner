// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/ShooterPickup.h"
#include "ShooterPickup_Dash.generated.h"

class AShooterCharacter;

// A pickup object that replenishes dash mid air
UCLASS(Abstract, Blueprintable)
class SHOOTERGAME_API AShooterPickup_Dash : public AShooterPickup
{
	GENERATED_BODY()
	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(AShooterCharacter* TestPawn) const override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float fGravityDisableDuration = 1.f;
protected:

	/** give pickup */
	virtual void GivePickupTo(AShooterCharacter* Pawn) override;
};
