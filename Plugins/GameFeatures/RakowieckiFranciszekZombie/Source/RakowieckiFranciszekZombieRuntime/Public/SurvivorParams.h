
#pragma once

struct SurvivorParams
{
	bool HasWeapon{false};
	bool HasFood{false};
	bool HasMeds{false};
	bool IsDying{false};
	bool IsHungry{false};
	bool HasInventorySpace{true};

	ABaseItem* SelectedWeapon{nullptr};
	ABaseItem* SelectedFood{nullptr};
	ABaseItem* SelectedMeds{nullptr};
};