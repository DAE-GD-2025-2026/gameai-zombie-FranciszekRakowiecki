
#pragma once

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

	ABaseItem* SelectedWeapon{nullptr};
	ABaseItem* SelectedFood{nullptr};
	ABaseItem* SelectedMeds{nullptr};
};