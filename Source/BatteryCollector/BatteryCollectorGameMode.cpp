// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BatteryCollectorGameMode.h"
#include "BatteryCollectorCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "SpawnVolume.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"

ABatteryCollectorGameMode::ABatteryCollectorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PrimaryActorTick.bCanEverTick = true;

	//Base decay rate
	DecayRate = 0.01f;
}

void ABatteryCollectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	//find all spawn volume Actors
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		ASpawnVolume* SpawnVolumeActor = Cast<ASpawnVolume>(Actor);
		if (SpawnVolumeActor)
		{
			SpawnVolumeActors.AddUnique(SpawnVolumeActor);
		}
	}

	SetCurrentState(EBatteryPlayState::EPlaying);

	// Set the score to beat
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		PowerToWin = (MyCharacter->GetInitialPower() * 1.25f);
	}

	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

float ABatteryCollectorGameMode::GetPowerToWin() const
{
	return PowerToWin;
}

void ABatteryCollectorGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Check that we are using the battery collector character
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		//If our power is greater than needed to win, set the game's state to won
		if (MyCharacter->GetCurrentPower() > PowerToWin)
		{
			SetCurrentState(EBatteryPlayState::EWon);
		}
		//If the character power is positive
		else if (MyCharacter->GetCurrentPower() > 0)
		{
			//Decrease the character's power using the decay rate
			MyCharacter->UpdatePower(-DeltaTime * DecayRate * (MyCharacter->GetInitialPower()));
		}
		else {
			SetCurrentState(EBatteryPlayState::EGameOver);
		}
	}
}

EBatteryPlayState ABatteryCollectorGameMode::GetCurrentState() const
{
	return CurrentState;
}

void ABatteryCollectorGameMode::SetCurrentState(EBatteryPlayState NewState)
{
	CurrentState = NewState;
	HandleNewState(CurrentState);
}

void ABatteryCollectorGameMode::HandleNewState(EBatteryPlayState NewState)
{
	switch (NewState)
	{
		//If the game is playing
		case EBatteryPlayState::EPlaying:
		{
			for (ASpawnVolume* Volume : SpawnVolumeActors)
			{
				Volume->SetSpawningActive(true);
			}
		}
		break;
		//If the game is over
		case EBatteryPlayState::EGameOver:
		{
			for (ASpawnVolume* Volume : SpawnVolumeActors)
			{
				Volume->SetSpawningActive(true);
			}

			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
			if (PlayerController)
			{
				PlayerController->SetCinematicMode(true, false, false, true, true);
			}

			ACharacter* MyCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
			if (MyCharacter)
			{
				MyCharacter->GetMesh()->SetSimulatePhysics(true);
				MyCharacter->GetMovementComponent()->MovementState.bCanJump = false;
			}
		}
		break;
		//If we win the game
		case EBatteryPlayState::EWon:
		{
			for (ASpawnVolume* Volume : SpawnVolumeActors)
			{
				Volume->SetSpawningActive(false);
			}
		}
		break;
		//Unkown
		default:
		case EBatteryPlayState::EUnknown:
		{
			
		}
		break;
	}
}