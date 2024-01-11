// Fill out your copyright notice in the Description page of Project Settings.


#include "items/Item.h"
#include "Slash/DebugMacros.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Interfaces/PickUpInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"






// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = ItemMesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());

	ItemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
    ItemEffect->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	
}
float AItem::TransformedSin()
{
    return Amplitude * FMath::Sin(RunningTime * TimeCste);
}
float AItem::TransformedCosin()
{
    return Amplitude * FMath::Cos(RunningTime * TimeCste);
}
void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
   IPickUpInterface* PickupInterface = Cast<IPickUpInterface>(OtherActor);
   if(PickupInterface){
	   PickupInterface->SetOverlappingItem(this);
   }
}
void AItem::OnSphereEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
   IPickUpInterface* PickupInterface = Cast<IPickUpInterface>(OtherActor);
   if(PickupInterface){
	   PickupInterface->SetOverlappingItem(nullptr);
   }
}

void AItem::SpawnPickupSystem()
{
	if(PickupEffect){
       UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        this,
        PickupEffect,
        GetActorLocation()
       );
   }
}
void AItem::SpawnPickupSound()
{
	if(PickupSound){
		UGameplayStatics::SpawnSoundAtLocation(
		this,
		PickupSound,
		GetActorLocation()
		);
	}
}
// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	if(ItemState == EItemState::EIS_Hovering){
	AddActorWorldOffset(FVector(0.f,0.f,TransformedSin()));
    //AddActorWorldRotation(FRotator(0.f,TransformedSin(), 0.f));
	}
	
}

