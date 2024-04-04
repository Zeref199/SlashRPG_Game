// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "Interfaces/PickUpInterface.h"
#include "SlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class ASoul;
class ATreasure;
class UAnimMontage;
class USlashOverlay;
 

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickUpInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASlashCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Jump() override;
	virtual void SetOverlappingItem(class AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;

protected:
virtual void BeginPlay() override;

/**
 * Callbacks for input
 */
void Move(const FInputActionValue &value);
void Look(const FInputActionValue &value);
void EKeyPressed();
virtual void Attack() override;
void Dodge();
void Equip();

void EquipWeapon(AWeapon *Weapon);
virtual void AttackEnd() override;
virtual void DodgeEnd() override;
virtual bool CanAttack() override;

void PlayEquipMontage(const FName &SectionName);
bool CanDisarm();
bool CanArm();
void Disarm();
void Arm();
virtual void Die() override;

UFUNCTION(BlueprintCallable)
void AttachWeaponToBack();

UFUNCTION(BlueprintCallable)
void AttachWeaponToHand();

UFUNCTION(BlueprintCallable)
void FinishEquipping();

UFUNCTION(BlueprintCallable)
void HitReactEnd();

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputMappingContext *SlashContext;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputAction *MovementAction;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputAction *LookAction;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputAction *JumpAction;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputAction *AttackAction;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputAction *EKeyAction;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputAction *DodgeAction;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
UInputAction *EquipAction;

private:

     bool IsUnoccupied();
     void InitializeSlashOverlay();
	 void SetHUDHealth();

    /** Character components*/
    
    UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;


	UPROPERTY(VisibleAnywhere, Category = "Hair")
	UGroomComponent* Eyebrows;

    UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;


	UPROPERTY(EditDefaultsOnly, Category = Montages)
    UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	
public:
   
   FORCEINLINE ECharacterState GetCharacterState() const {return CharacterState;}
   FORCEINLINE EActionState GetActionState() const {return ActionState;}
};
