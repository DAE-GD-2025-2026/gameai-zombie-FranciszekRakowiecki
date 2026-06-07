// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"

#include "AIController.h"
#include "SurvivorBehaviors.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Engine/Engine.h"
#include "Items/BaseItem.h"
#include "PurgeZones/PurgeZone.h"
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
			Inventory->GrabItem(lastIndex, Item);
			Memory.ItemPickedUp(Item);
			Inventory->RemoveItem(lastIndex); // Genuinely this is better than making my own function to remove garbage from the floor
			return false;
		}
		return false;
	}
	else if (Parameters.HasMeds && Item->GetItemType() == EItemType::Medkit)
		return false;
	AddItemToInventory(Item);
	return true;
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
	if (!Inventory || !Blackboard)
	{
		return;
	}

	Memory.ItemPickupRadius = Inventory->GetPickupRange();
	Memory.Tick();

	UpdateInventoryStoredInfo();
	UpdateHealthInfo();

	CurrentPickupTarget = GetDesiredPickupItem();
	if (CurrentPickupTarget)
	{
		Parameters.IsCloseEnoughForPickup = Memory.IsCloseEnoughForPickup(CurrentPickupTarget);
	}
	else
	{
		Parameters.IsCloseEnoughForPickup = false;
	}

	MovementDirection = Steering->GetOutput(Parameters, Memory, GetOwner());
	Parameters.HasTargetLocation = Steering->HasOutput() && !MovementDirection.IsNearlyZero();
	Parameters.IsZombieCloseEnough = Memory.GetZombieCloseEnough();
	
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

void UStudentPerceptor::AddItemToInventory(ABaseItem* Item)
{
	for (int index = 0; index < Inventory->GetInventoryCapacity(); ++index)
	{
		if (Inventory->GrabItem(index, Item))
		{
			Memory.ItemPickedUp(Item);
			return;
		}
 	}
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
	if (Parameters.IsDying && !Parameters.HasMeds && Memory.GetMeds())
	{
		return Memory.GetMeds();
	}
	if (Parameters.IsHungry && !Parameters.HasFood && Memory.GetFood())
	{
		return Memory.GetFood();
	}
	if (!Parameters.HasWeapon && Memory.GetWeapon())
	{
		return Memory.GetWeapon();
	}
	return Memory.GetClosestItem();
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
		float percentageMissing = float(Health->GetHealth()) / float(Health->GetMaxHealth());
		Parameters.IsDying = percentageMissing < 0.5f;
	}
	{
		float percentageMissing = Stamina->GetCurrentStamina() / Stamina->GetMaxStamina();
		Parameters.IsHungry = percentageMissing < 0.5f;
	}
}
