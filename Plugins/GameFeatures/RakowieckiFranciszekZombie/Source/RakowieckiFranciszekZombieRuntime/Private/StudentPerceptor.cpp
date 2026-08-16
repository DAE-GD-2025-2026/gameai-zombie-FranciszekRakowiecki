// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"

#include "AIController.h"
#include "SurvivorBehaviors.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Engine/Engine.h"
#include "Items/BaseItem.h"
#include "PurgeZones/PurgeZone.h"
#include "Survivor/SurvivorPawn.h"
#include "Village/House/House.h"
#include "Zombies/BaseZombie.h"


UStudentPerceptor::UStudentPerceptor(): Memory()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();

	print("StudentPerceptor BeginPlay");

	Memory.SetOwner(GetOwner());

	Self = Cast<APawn>(GetOwner()); // We are not going to be putting perceptor anywhere other than survivor

	if (!Self)
	{
		print("Failed to cast to APawn");
		return;
	}
	print("Found APawn");

	Controller = Cast<AAIController>(Self->GetController());

	if (!Controller)
	{
		print("Failed to cast AAIController");
	}
	else
	{
		print("Found AAIController");
	
		Blackboard = Controller->GetBlackboardComponent();
		if (!Blackboard)
		{
			print("Blackboard not ready yet");
		}
		else
		{
			print("Found blackboard");
		}
	}
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
		print("Found perception component");
	}
	else
	{
		print("StudentPerceptor could not find owner perception component");
	}

	if (auto InventoryComponent = GetOwner()->GetComponentByClass<UInventoryComponent>())
	{
		Inventory = InventoryComponent;
		print("Found inventory component");
	}
	else
	{
		print("StudentPerceptor could not find owner inventory component");
	}

	if (auto HealthComponent = GetOwner()->GetComponentByClass<UHealthComponent>())
	{
		Health = HealthComponent;
		print("Found health component");
	}
	else
	{
		print("StudentPerceptor could not find owner health component");
	}

	if (auto StaminaComponent = GetOwner()->GetComponentByClass<UStaminaComponent>())
	{
		Stamina = StaminaComponent;
		print("Found stamina component");
	}
	else
	{
		print("StudentPerceptor could not find owner stamina component");
	}

	Steering = std::make_unique<BlendedSteering>();

	Steering->AddSteering(std::make_unique<FleeZombies>(), 1.0);
	Steering->AddSteering(std::make_unique<AvoidPurgeZones>(), 3.6);
	Steering->AddSteering(std::make_unique<SeekPickupItem>(), 1.0);
	Steering->AddSteering(std::make_unique<FindHouse>(), 1.0);
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	print("Saw Something!");

	ABaseItem* item = Cast<ABaseItem>(Actor);
	if (item)
	{
		Memory.RememberItem(item);
		return;
	}
	ABaseZombie* zombie = Cast<ABaseZombie>(Actor);
	if (zombie)
	{
		Memory.RememberZombie(zombie);

		if (!zombie->OnDestroyed.IsAlreadyBound(this, &UStudentPerceptor::OnZombieActorDestroyed))
		{
			zombie->OnDestroyed.AddDynamic(this, &UStudentPerceptor::OnZombieActorDestroyed);
		}
		return;
	}
	AHouse* house = Cast<AHouse>(Actor);
	if (house)
	{
		Memory.RememberHouse(house);
		return;
	}
	APurgeZone* zone = Cast<APurgeZone>(Actor);
	if (zone)
	{
		Memory.RememberPurgeZone(zone);

		if (!zone->OnDestroyed.IsAlreadyBound(this, &UStudentPerceptor::OnPurgeZoneActorDestroyed))
		{
			zone->OnDestroyed.AddDynamic(this, &UStudentPerceptor::OnPurgeZoneActorDestroyed);
		}
	}
}

bool UStudentPerceptor::OnPickupItem(ABaseItem* Item)
{
	if (!Item || !Inventory)
	{
		return false;
	}

	if (!Memory.IsCloseEnoughForPickup(Item))
		return false;

	if (Item->GetItemType() == EItemType::Garbage)
	{
		const uint32_t lastIndex = Inventory->GetInventoryCapacity() - 1;
		if (Inventory->GetInventory()[lastIndex] == nullptr)
		{
			if (!Inventory->GrabItem(lastIndex, Item))
			{
				return false;
			}
			Memory.ItemPickedUp(Item);
			Inventory->RemoveItem(lastIndex); // Genuinely this is better than making my own function to remove garbage from the floor
			return true;
		}
		return false;
	}
	else if (Parameters.HasMeds && Item->GetItemType() == EItemType::Medkit)
		return false;
	return AddItemToInventory(Item);
}

bool UStudentPerceptor::TryPickupCurrentTarget()
{
	return OnPickupItem(CurrentPickupTarget);
}

void UStudentPerceptor::OnUseItem(EItemType ItemType)
{
	switch (ItemType)
	{
	case EItemType::Food:
		UseItem(Parameters.SelectedFood);
		break;
	case EItemType::Medkit:
		UseItem(Parameters.SelectedMeds);
		break;
	case EItemType::Shotgun:
		UseItem(Parameters.SelectedWeapon);
		break;
	case EItemType::Pistol:
		UseItem(Parameters.SelectedWeapon);
		break;
	case EItemType::Garbage:
		break;
	}
}

void UStudentPerceptor::OnZombieActorDestroyed(AActor* DestroyedActor)
{
	Memory.ForgetZombie(Cast<ABaseZombie>(DestroyedActor));
}

void UStudentPerceptor::OnPurgeZoneActorDestroyed(AActor* DestroyedActor)
{
	Memory.ForgetPurgeZone(Cast<APurgeZone>(DestroyedActor));
}

void UStudentPerceptor::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Controller && Self)
	{
		Controller = Cast<AAIController>(Self->GetController());
	}
	if (!Blackboard && Controller)
	{
		Blackboard = Controller->GetBlackboardComponent();
	}
	if (!Inventory || !Health || !Stamina || !Blackboard)
	{
		return;
	}

	Memory.ItemPickupRadius = Inventory->GetPickupRange();
	Memory.Tick();

	UpdateInventoryStoredInfo();
	UpdateHealthInfo();
	Parameters.IsZombieCloseEnough = Memory.GetZombieCloseEnough();

	CurrentPickupTarget = GetDesiredPickupItem();
	Parameters.PickupTarget = CurrentPickupTarget;
	if (CurrentPickupTarget)
	{
		Parameters.IsCloseEnoughForPickup = Memory.IsCloseEnoughForPickup(CurrentPickupTarget);
	}
	else
	{
		Parameters.IsCloseEnoughForPickup = false;
	}
	UpdateDecision();
	UpdateSprintState();

	MovementDirection = Steering->GetOutput(Parameters, Memory, GetOwner());
	Parameters.HasTargetLocation = IsMovementDecision()
		&& Steering->HasOutput()
		&& !MovementDirection.IsNearlyZero();
	
	UpdateBlackboardValues();
}

void UStudentPerceptor::print(const char* message)
{
	print(ANSI_TO_TCHAR(message));
}

void UStudentPerceptor::print(const FString& message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, message);
	}
}

bool UStudentPerceptor::AddItemToInventory(ABaseItem* Item)
{
	for (int index = 0; index < Inventory->GetInventoryCapacity(); ++index)
	{
		if (Inventory->GrabItem(index, Item))
		{
			Memory.ItemPickedUp(Item);
			return true;
		}
 	}
	return false;
}

void UStudentPerceptor::UseItem(ABaseItem* Item)
{
	if (Item)
	{
		Item->UseItem(*Cast<ASurvivorPawn>(GetOwner()));
	}
}

ABaseItem* UStudentPerceptor::GetDesiredPickupItem() const
{
	if (!Parameters.HasInventorySpace)
	{
		return nullptr;
	}

	ABaseItem* bestItem = nullptr;
	float bestScore = 0.0f;
	auto considerItem = [&](ABaseItem* item, float needScore)
	{
		if (!item)
		{
			return;
		}
		const float distancePenalty = static_cast<float>(Memory.GetDistanceTo(item)) / 25.0f;
		const float score = needScore - distancePenalty;
		if (score > bestScore)
		{
			bestScore = score;
			bestItem = item;
		}
	};

	if (!Parameters.HasMeds)
	{
		considerItem(Memory.GetMeds(), 60.0f + (1.0f - Parameters.HealthRatio) * 80.0f);
	}
	if (!Parameters.HasFood)
	{
		considerItem(Memory.GetFood(), 55.0f + (1.0f - Parameters.StaminaRatio) * 70.0f);
	}
	if (!Parameters.HasWeapon)
	{
		considerItem(Memory.GetWeapon(), 100.0f);
	}
	return bestItem;
}

bool UStudentPerceptor::IsMovementDecision() const
{
	return Parameters.Decision == ESurvivorDecision::Flee
		|| Parameters.Decision == ESurvivorDecision::PickupItem
		|| Parameters.Decision == ESurvivorDecision::SearchHouse;
}

void UStudentPerceptor::UpdateDecision()
{
	if (Parameters.HealthRatio < 0.25f && Parameters.HasMeds)
	{
		Parameters.Decision = ESurvivorDecision::UseMedkit;
	}
	else if (Parameters.IsZombieCloseEnough)
	{
		const bool hasShotgun = Parameters.SelectedWeapon
			&& Parameters.SelectedWeapon->GetItemType() == EItemType::Shotgun;
		const float acceptableThreat = hasShotgun ? 2.0f : 1.25f;
		Parameters.Decision = Parameters.HasWeapon
			&& Parameters.HealthRatio > 0.4f
			&& Memory.GetThreatLevel() <= acceptableThreat
			? ESurvivorDecision::Fight
			: ESurvivorDecision::Flee;
	}
	else if (Parameters.IsDying && Parameters.HasMeds)
	{
		Parameters.Decision = ESurvivorDecision::UseMedkit;
	}
	else if (Parameters.IsHungry && Parameters.HasFood)
	{
		Parameters.Decision = ESurvivorDecision::UseFood;
	}
	else if (CurrentPickupTarget)
	{
		Parameters.Decision = ESurvivorDecision::PickupItem;
	}
	else if (Memory.GetHouse())
	{
		Parameters.Decision = ESurvivorDecision::SearchHouse;
	}
	else
	{
		Parameters.Decision = ESurvivorDecision::Wander;
	}

	Parameters.ShouldSprint = Parameters.Decision == ESurvivorDecision::Flee
		&& Parameters.StaminaRatio > 0.2f;
}

void UStudentPerceptor::UpdateSprintState()
{
	ASurvivorPawn* survivor = Cast<ASurvivorPawn>(GetOwner());
	if (!survivor)
	{
		return;
	}

	if (Parameters.ShouldSprint)
	{
		survivor->StartRunning();
	}
	else
	{
		survivor->StopRunning();
	}
}

void UStudentPerceptor::UpdateBlackboardValues()
{
	Blackboard->SetValueAsObject(TEXT("Survivor"), GetOwner());
	Blackboard->SetValueAsObject(TEXT("SelfActor"), GetOwner());
	Blackboard->SetValueAsObject(TEXT("Zombie"), Memory.GetZombie());
	Blackboard->SetValueAsObject(TEXT("PickupItem"), CurrentPickupTarget);
	Blackboard->SetValueAsVector(TEXT("TargetLocation"), Parameters.HasTargetLocation ? MovementDirection * Memory.FleeDistance + GetOwner()->GetActorLocation() : GetOwner()->GetActorLocation());
	Blackboard->SetValueAsBool(TEXT("hasTargetLocation"), Parameters.HasTargetLocation);
	Blackboard->SetValueAsBool(TEXT("hasWeapon"), Parameters.HasWeapon);
	Blackboard->SetValueAsBool(TEXT("hasMeds"), Parameters.HasMeds);
	Blackboard->SetValueAsBool(TEXT("hasFood"), Parameters.HasFood);
	Blackboard->SetValueAsBool(TEXT("isDying"), Parameters.IsDying);
	Blackboard->SetValueAsBool(TEXT("isHungry"), Parameters.IsHungry);
	Blackboard->SetValueAsBool(TEXT("isZombieCloseEnough"), Parameters.IsZombieCloseEnough);
	Blackboard->SetValueAsBool(TEXT("hasInventorySpace"), Parameters.HasInventorySpace);
	Blackboard->SetValueAsBool(TEXT("isCloseEnoughForPickup"), Parameters.IsCloseEnoughForPickup);
	Blackboard->SetValueAsObject(TEXT("Food"), Memory.GetFood());
	Blackboard->SetValueAsObject(TEXT("Medical"), Memory.GetMeds());
	Blackboard->SetValueAsObject(TEXT("Weapon"), Memory.GetWeapon());
	Blackboard->SetValueAsVector(TEXT("ClosestZombieLocation"), Memory.GetZombie() ? Memory.GetZombie()->GetActorLocation() : FVector{});
	Blackboard->SetValueAsVector(TEXT("FoodLocation"), Memory.GetFood() ? Memory.GetFood()->GetActorLocation() : FVector{});
	Blackboard->SetValueAsVector(TEXT("MedsLocation"), Memory.GetMeds() ? Memory.GetMeds()->GetActorLocation() : FVector{});
	Blackboard->SetValueAsVector(TEXT("WeaponLocation"), Memory.GetWeapon() ? Memory.GetWeapon()->GetActorLocation() : FVector{});
}

void UStudentPerceptor::UpdateInventoryStoredInfo()
{
	Parameters.HasWeapon = false;
	Parameters.HasFood = false;
	Parameters.HasMeds = false;
	Parameters.HasInventorySpace = false;
	Parameters.SelectedWeapon = nullptr;
	Parameters.SelectedFood = nullptr;
	Parameters.SelectedMeds = nullptr;
	int32_t index{-1};
	for (auto item : Inventory->GetInventory())
	{
		index++;
		if (item == nullptr)
		{
			Parameters.HasInventorySpace = true;
			continue;
		}
		if (item->GetValue() == 0)
		{
			Inventory->RemoveItem(index);
			continue;
		}
		switch (item->GetItemType())
		{
			case EItemType::Food:
				Parameters.HasFood = true;
				if (!Parameters.SelectedFood)
					Parameters.SelectedFood = item;
				break;
			case EItemType::Medkit:
				Parameters.HasMeds = true;
				if (!Parameters.SelectedMeds)
					Parameters.SelectedMeds = item;
				break;
			case EItemType::Shotgun:
				Parameters.HasWeapon = true;
				Parameters.SelectedWeapon = item;
				break;
			case EItemType::Pistol:
				Parameters.HasWeapon = true;
				if (!Parameters.SelectedWeapon)
					Parameters.SelectedWeapon = item;
				break;
			case EItemType::Garbage:
				break;
		}
	}
}

void UStudentPerceptor::UpdateHealthInfo()
{
	{
		Parameters.HealthRatio = float(Health->GetHealth()) / float(Health->GetMaxHealth());
		Parameters.IsDying = Parameters.HealthRatio < 0.5f;
	}
	{
		Parameters.StaminaRatio = Stamina->GetCurrentStamina() / Stamina->GetMaxStamina();
		Parameters.IsHungry = Parameters.StaminaRatio < 0.5f;
	}
}
