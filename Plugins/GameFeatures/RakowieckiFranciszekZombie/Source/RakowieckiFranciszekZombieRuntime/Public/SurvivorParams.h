
#pragma once

#include "CoreMinimal.h"

class ABaseItem;

enum class ESurvivorDecision : uint8
{
	UseMedkit,
	UseFood,
	Flee,
	Fight,
	PickupItem,
	SearchHouse,
	Wander
};

struct SurvivorParams
{
	bool HasWeapon{false};
	bool HasFood{false};
	bool HasMeds{false};
	bool IsDying{false};
	bool IsHungry{false};
	bool HasInventorySpace{true};
	bool IsZombieCloseEnough{false};
	bool HasTargetLocation{false};
	bool IsCloseEnoughForPickup{false};
	bool ShouldSprint{false};

	float HealthRatio{1.0f};
	float StaminaRatio{1.0f};
	ESurvivorDecision Decision{ESurvivorDecision::Wander};

	ABaseItem* SelectedWeapon{nullptr};
	ABaseItem* SelectedFood{nullptr};
	ABaseItem* SelectedMeds{nullptr};
	ABaseItem* PickupTarget{nullptr};
};
