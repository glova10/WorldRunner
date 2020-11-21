// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterWeapon.h"
#include "GameFramework/DamageType.h" // for UDamageType::StaticClass()
#include "ShooterWeapon_Melee.generated.h"

USTRUCT()
struct FMeleeWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType;
	
	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	int32 HitDamage;
	
	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float WeaponRange;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float WeaponWidth;

	/** defaults */
	FMeleeWeaponData()
	{
		WeaponRange = 200;
		HitDamage = 10.0f;
		WeaponWidth = 15;
		DamageType = UDamageType::StaticClass();
	}
};

// A weapon that fires a visible projectile
UCLASS(Abstract)
class AShooterWeapon_Melee : public AShooterWeapon
{
	GENERATED_UCLASS_BODY()

	/** apply config on projectile */
	void ApplyWeaponConfig(FMeleeWeaponData& Data);

	/** Update the character. (Running, health etc). */
	virtual void Tick(float DeltaSeconds) override;

	/** name of bone/socket for muzzle in weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName StartDmgPointName;

	// is giving damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Damage")
	bool bGivingDamage;
protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EStamina;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FMeleeWeaponData MeleeConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);

	/** get the muzzle location of the weapon */
	FVector GetStartDamageLocation() const;

	/** get direction of weapon's muzzle */
	FVector GetStartDamageDirection() const;

	/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);
	/** verify if target can be hit**/
	bool ShouldDealDamage(AActor* TestActor) const;

	FVector PrevStartDmgLocation;
	FVector PrevStartDmgDirection;

	TArray<AActor*> ActorsToIgnore;
	static bool RENDER_DEBUG;
public:
	virtual EWeaponType GetWeaponType() const override { return EWeaponType::EWT_Sword; }
};
