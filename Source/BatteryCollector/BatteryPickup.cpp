// Fill out your copyright notice in the Description page of Project Settings.
#include "BatteryPickup.h"
#include "BatteryCollector.h"

// Sets default values for this actor's properties
ABatteryPickup::ABatteryPickup()
{
	GetMesh()->SetSimulatePhysics(true);
}