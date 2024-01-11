// Fill out your copyright notice in the Description page of Project Settings.

#include "items/Treasure.h"
#include "Interfaces/PickUpInterface.h"
#include "SlashCharacter.h"




void ATreasure::OnSphereOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
     IPickUpInterface* PickupInterface = Cast<IPickUpInterface>(OtherActor);
     if(PickupInterface){
	   PickupInterface->AddGold(this);

       SpawnPickupSound();
       Destroy();
     }
}