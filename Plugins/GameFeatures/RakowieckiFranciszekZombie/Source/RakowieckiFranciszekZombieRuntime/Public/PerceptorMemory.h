#pragma once
#include <vector>

#include "Items/BaseItem.h"

class ABaseZombie;

struct ZombieMemory
{
	float lastSeen;
	ABaseZombie* zombie{nullptr};
};

class FPerceptorMemory
{
public:
	FPerceptorMemory() = default;

	void SetOwner(AActor* owner);
	void RememberItem(ABaseItem* item);
	void RememberZombie(ABaseZombie* zombie);

	void ItemPickedUp(ABaseItem* item);

	void Tick();

	ABaseItem* GetClosestItem();
	ABaseItem* GetFood() const;
	ABaseItem* GetWeapon() const;
	ABaseItem* GetMeds() const;

	FVector GetRelZombieLoc() const { return m_RelevantAvgZombieLocation; }
	ABaseZombie* GetZombie() const;

private:
	
	bool IsItemFar(ABaseItem* item) const;
	bool IsCloseEnoughForPickup(ABaseItem* item) const;
	bool IsZombieRelevant(ABaseZombie* zombie) const;

	void UpdateZombieInfo();

	AActor* m_Owner{nullptr};
	std::vector<ABaseItem*> m_InWorldMemoryItems{};
	std::vector<ABaseZombie*> m_SpottedZombies{};
	ABaseItem* m_Food;
	ABaseItem* m_Weapon;
	ABaseItem* m_Meds;

	FVector m_RelevantAvgZombieLocation;
	ABaseZombie* m_ClosestZombie{nullptr};

public:
	double ItemRememberRadius{1000.0};
	double ItemPickupRadius{300.0};
	double ZombieRelevanceRadius{3000.0};
};
