// Fill out your copyright notice in the Description page of Project Settings.


#include "items/Soul.h"
#include "Interfaces/PickUpInterface.h"
#include "Kismet/KismetSystemLibrary.h"


void ASoul::Tick(float DeltaTime){
    Super::Tick(DeltaTime);

    const double LocationZ = GetActorLocation().Z;
    if(LocationZ > DesiredZ){
        const FVector DeltaLocation = FVector(0.f, 0.f, DriftRate * DeltaTime);
        AddActorWorldOffset(DeltaLocation);
    }
}

void ASoul::BeginPlay(){
     Super::BeginPlay();

    const FVector Start = GetActorLocation();
    const FVector End = Start - FVector(0.f, 0.f, 2000.f);
    FHitResult HitResult;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    UKismetSystemLibrary::LineTraceSingleForObjects(
        this,
        Start,
        End,
        ObjectTypes,
        false,
        ActorsToIgnore,
        EDrawDebugTrace::ForDuration,
        HitResult,
        true
    );
    DesiredZ = HitResult.ImpactPoint.Z + 50.f;

}

void ASoul::OnSphereOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
    IPickUpInterface* PickupInterface = Cast<IPickUpInterface>(OtherActor);
   if(PickupInterface){
	   PickupInterface->AddSouls(this);
       SpawnPickupSystem();
       SpawnPickupSound();

       Destroy();
   }
   
}
