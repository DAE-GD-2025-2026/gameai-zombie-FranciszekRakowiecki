#pragma once
#include <vector>

#include "Items/BaseItem.h"

class ABaseZombie;

class FPerceptorMemory
{
public:
	FPerceptorMemory() = default;

	void SetOwner(AActor* owner);
	void RememberItem(ABaseItem* item);

	void ItemPickedUp(ABaseItem* item);

	void Tick();

	ABaseItem* GetClosestItem();
	ABaseItem* GetFood() const;
	ABaseItem* GetWeapon() const;
	ABaseItem* GetMeds() const;

private:
	
	bool IsItemFar(ABaseItem* item);
	bool IsCloseEnoughForPickup(ABaseItem* item);

	AActor* m_Owner{nullptr};
	std::vector<ABaseItem*> m_InWorldMemoryItems{};
	std::vector<ABaseZombie*> m_SpottedZombies{};
	ABaseItem* m_Food;
	ABaseItem* m_Weapon;
	ABaseItem* m_Meds;

public:
	double ItemRememberRadius{1000.0f};
	double ItemPickupRadius{300.0f};
};
