// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Engine/Engine.h"
#include "Items/BaseItem.h"


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

	Controller = Cast<AAIController>(Self->GetController());

	if (!Controller)
	{
		print("Failed to cast AAIController");
		return;
	}
	
	Blackboard = Controller->GetBlackboardComponent();
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
	}
	else
	{
		print("StudentPerceptor could not find owner perception component");
	}

	if (auto InventoryComponent = GetOwner()->GetComponentByClass<UInventoryComponent>())
	{
		Inventory = InventoryComponent;
	}
	else
	{
		print("StudentPerceptor could not find owner inventory component");
	}

	if (auto HealthComponent = GetOwner()->GetComponentByClass<UHealthComponent>())
	{
		Health = HealthComponent;
	}
	else
	{
		print("StudentPerceptor could not find owner health component");
	}

	if (auto StaminaComponent = GetOwner()->GetComponentByClass<UStaminaComponent>())
	{
		Stamina = StaminaComponent;
	}
	else
	{
		print("StudentPerceptor could not find owner stamina component");
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	print("Saw Something!");

	ABaseItem* item = Cast<ABaseItem>(Actor);
	if (item)
	{
		Memory.RememberItem(item);
	}
}

void UStudentPerceptor::OnPickupItem(ABaseItem* Item)
{
	AddItemToInventory(Item);
}

void UStudentPerceptor::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Memory.ItemPickupRadius = Inventory->GetPickupRange();
	Memory.Tick();

	UpdateInventoryStoredInfo();
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

void UStudentPerceptor::UpdateBlackboardValues()
{
	Blackboard->SetValueAsObject(TEXT("Survivor"), GetOwner());
	Blackboard->SetValueAsObject(TEXT("Zombie"), nullptr);
	Blackboard->SetValueAsObject(TEXT("PickupItem"), Memory.GetClosestItem());
	Blackboard->SetValueAsVector(TEXT("AvgAwayFromZombies"), {});
	Blackboard->SetValueAsBool(TEXT("hasWeapon"), hasWeapon);
	Blackboard->SetValueAsBool(TEXT("hasMeds"), hasMeds);
	Blackboard->SetValueAsBool(TEXT("hasFood"), hasFood);
	Blackboard->SetValueAsBool(TEXT("isDying"), isDying);
	Blackboard->SetValueAsBool(TEXT("isHungry"), isHungry);
	Blackboard->SetValueAsBool(TEXT("hasInventorySpace"), hasInventorySpace);
	Blackboard->SetValueAsObject(TEXT("Food"), Memory.GetFood());
	Blackboard->SetValueAsObject(TEXT("Meds"), Memory.GetMeds());
	Blackboard->SetValueAsObject(TEXT("Weapon"), Memory.GetWeapon());
}

void UStudentPerceptor::UpdateInventoryStoredInfo()
{
	hasWeapon = false;
	hasFood = false;
	hasMeds = false;
	hasInventorySpace = false;
	int32_t index{-1};
	for (auto item : Inventory->GetInventory())
	{
		index++;
		if (item == nullptr)
		{
			hasInventorySpace = true;
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
				hasFood = true;
				break;
			case EItemType::Medkit:
				hasMeds = true;
				break;
			case EItemType::Shotgun:
			case EItemType::Pistol:
				hasWeapon = true;
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
		isDying = percentageMissing < 0.5f;
	}
	{
		float percentageMissing = Stamina->GetCurrentStamina() / Stamina->GetMaxStamina();
		isHungry = percentageMissing < 0.5f;
	}
}
