// Fill out your copyright notice in the Description page of Project Settings.


#include "SlashCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Components/AttributeComponent.h"
#include "items/Item.h"
#include "items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"


// Sets default values
ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);


	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("Head");

}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)){
	EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
	EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
	EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
	EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Equip);
	}

}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
    HandleDamage(DamageAmount);
    SetHUDHealth();
    return DamageAmount;
}



void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter){

	Super::GetHit_Implementation(ImpactPoint, Hitter);

    SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if(Attributes && Attributes->GetHealthPercent() > 0.f){
	ActionState = EActionState::EAS_HitReaction;
	}
}



void ASlashCharacter::Jump()
{
    if (IsUnoccupied())
    {
        Super::Jump();
    }
}



void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("EngageableTarget"));

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if(PlayerController){
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if(Subsystem){
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
    InitializeSlashOverlay();
}



void ASlashCharacter::Move(const FInputActionValue &value)
{
	if(ActionState != EActionState::EAS_Unoccupied){return;}
     const FVector2D MovementVector = value.Get<FVector2D>();
	 
	 const FRotator Rotation = Controller->GetControlRotation();
	 const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	 const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	 AddMovementInput(ForwardDirection, MovementVector.Y);
	 const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	 AddMovementInput(RightDirection, MovementVector.X);
	
}

void ASlashCharacter::Look(const FInputActionValue &value)
{
	const FVector2D LookAxisValue = value.Get<FVector2D>();
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
}

void ASlashCharacter::EKeyPressed()
{
   AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
   if(OverlappingWeapon){
	  EquipWeapon(OverlappingWeapon);
   }
}

void ASlashCharacter::Attack()
{
	Super::Attack();
   if(CanAttack()){
      PlayAttackMontage();
	  ActionState = EActionState::EAS_Attacking;
   }
}

void ASlashCharacter::Dodge()
{
}

void ASlashCharacter::Equip(){
	if(CanDisarm()){
         Disarm();
	  }
	  else if(CanArm()){
		 Arm();
	  }
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon){
      Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	  CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	  OverlappingItem = nullptr;
	  EquippedWeapon = Weapon;

}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;	 
}

bool ASlashCharacter::CanAttack(){

   return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EquipMontage){
	   AnimInstance->Montage_Play(EquipMontage);
	   AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
   }
}

bool ASlashCharacter::CanDisarm()
{
   return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm(){
	return ActionState == EActionState::EAS_Unoccupied && CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}

void ASlashCharacter::Disarm(){
         PlayEquipMontage(FName("Unequip"));
		 CharacterState = ECharacterState::ECS_Unequipped;
		 ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm(){
	     PlayEquipMontage(FName("Equip"));
		 CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		 ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

void ASlashCharacter::AttachWeaponToBack()
{
	if(EquippedWeapon){
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if(EquippedWeapon){
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        ASlashHUD *SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
        if (SlashHUD)
        {
            SlashOverlay = SlashHUD->GetSlashOverlay();
            if (SlashOverlay && Attributes)
            {
                SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
                SlashOverlay->SetStaminaBarPercent(1.f);
                SlashOverlay->SetGold(0);
                SlashOverlay->SetSouls(0);
            }
        }
    }
}

void ASlashCharacter::SetHUDHealth()
{
    if (SlashOverlay && Attributes)
    {
        SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
    }
}

bool ASlashCharacter::IsUnoccupied()
{
    return ActionState == EActionState::EAS_Unoccupied;
}