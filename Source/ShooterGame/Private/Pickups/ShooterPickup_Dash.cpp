// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "ShooterPickup_Dash.h"

bool AShooterPickup_Dash::CanBePickedUp(AShooterCharacter* TestPawn) const
{
    return TestPawn;// && !TestPawn->CanDash();
}

void AShooterPickup_Dash::GivePickupTo(AShooterCharacter* Pawn)
{
    if (Pawn)
    {
        Pawn->ResetCanDash(fGravityDisableDuration);
    }
}
