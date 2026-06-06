#pragma once
#include <vector>

#include "Items/BaseItem.h"

class APurgeZone;
class AHouse;
class ABaseZombie;

struct HouseMemory
{
	AHouse* house;
	double lastVisited;
	bool visited;
};

class FPerceptorMemory
{
public:
	FPerceptorMemory() = default;

	void SetOwner(AActor* owner);
	void RememberItem(ABaseItem* item);
	void RememberZombie(ABaseZombie* zombie);
	void RememberHouse(AHouse* house);
	void RememberPurgeZone(APurgeZone* purgeZone);
	void ForgetZombie(ABaseZombie* Zombie);

	void ItemPickedUp(ABaseItem* item);

	void Tick();
	
	ABaseItem* GetFood() const;
	ABaseItem* GetWeapon() const;
	ABaseItem* GetMeds() const;

	FVector GetRelZombieLoc() const { return m_RelevantAvgZombieLocation; }
	ABaseZombie* GetZombie() const;
	ABaseItem* GetClosestItem() const;
	AHouse* GetHouse() const;
	const std::vector<APurgeZone*>& GetPurgeZones() const;
	double GetClosestItemDistance() const;
	bool GetZombieCloseEnough() const;

private:
	
	bool IsItemFar(ABaseItem* item) const;
	bool IsCloseEnoughForPickup(ABaseItem* item) const;
	bool IsZombieRelevant(ABaseZombie* zombie) const;

	void UpdateZombieInfo();
	void UpdateItemInfo();
	void UpdateHouseInfo();

	bool IsWithinBounds(AHouse* house) const;

	AActor* m_Owner{nullptr};
	std::vector<ABaseItem*> m_InWorldMemoryItems{};
	std::vector<ABaseZombie*> m_SpottedZombies{};
	std::vector<HouseMemory> m_InWorldHouses{};
	std::vector<APurgeZone*> m_InWorldPurgeZones{};
	ABaseItem* m_Food;
	ABaseItem* m_Weapon;
	ABaseItem* m_Meds;
	ABaseItem* m_ClosestItem;

	AHouse* m_TargetHouse;

	FVector m_RelevantAvgZombieLocation;
	ABaseZombie* m_ClosestZombie{nullptr};
	double m_ClosestDistance{0.0};
	bool m_ZombieClose{false};

public:
	double ItemRememberRadius{1000.0};
	double ItemPickupRadius{300.0};
	double ZombieRelevanceRadius{3000.0};
	double FleeDistance{300.0};
	double HouseVisitDelay{60.0};
};
