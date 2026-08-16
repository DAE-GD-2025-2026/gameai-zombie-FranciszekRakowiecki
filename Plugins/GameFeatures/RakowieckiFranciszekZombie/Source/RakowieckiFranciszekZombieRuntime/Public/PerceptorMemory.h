#pragma once
#include <vector>

#include "Items/BaseItem.h"

class APurgeZone;
class AHouse;
class ABaseZombie;

enum class EPerceivedZombieType : uint8
{
	Normal,
	Runner,
	Heavy
};

struct ZombieMemory
{
	ABaseZombie* zombie{nullptr};
	EPerceivedZombieType type{EPerceivedZombieType::Normal};
};

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
	void ForgetPurgeZone(APurgeZone* PurgeZone);

	void ItemPickedUp(ABaseItem* item);

	void Tick();
	
	ABaseItem* GetFood() const;
	ABaseItem* GetWeapon() const;
	ABaseItem* GetMeds() const;

	FVector GetRelZombieLoc() const { return m_RelevantAvgZombieLocation; }
	ABaseZombie* GetZombie() const;
	EPerceivedZombieType GetClosestZombieType() const { return m_ClosestZombieType; }
	ABaseItem* GetClosestItem() const;
	AHouse* GetHouse() const;
	const std::vector<APurgeZone*>& GetPurgeZones() const;
	double GetClosestItemDistance() const;
	double GetTargetHousePathDistance() const { return m_TargetHousePathDistance; }
	double GetDistanceTo(const AActor* actor) const;
	double GetPathDistanceTo(const FVector& destination) const;
	bool GetZombieCloseEnough() const;
	float GetThreatLevel() const { return m_ThreatLevel; }

	bool IsCloseEnoughForPickup(ABaseItem* item) const;

private:
	
	bool IsItemFar(ABaseItem* item) const;
	bool IsZombieRelevant(ABaseZombie* zombie) const;
	EPerceivedZombieType ClassifyZombie(const ABaseZombie* zombie) const;

	void UpdateZombieInfo();
	void UpdateItemInfo();
	void UpdateHouseInfo();

	bool IsWithinBounds(AHouse* house) const;

	AActor* m_Owner{nullptr};
	std::vector<ABaseItem*> m_InWorldMemoryItems{};
	std::vector<ZombieMemory> m_SpottedZombies{};
	std::vector<HouseMemory> m_InWorldHouses{};
	std::vector<APurgeZone*> m_InWorldPurgeZones{};
	ABaseItem* m_Food{nullptr};
	ABaseItem* m_Weapon{nullptr};
	ABaseItem* m_Meds{nullptr};
	ABaseItem* m_ClosestItem{nullptr};

	AHouse* m_TargetHouse{nullptr};
	double m_TargetHousePathDistance{0.0};
	double m_NextHouseUpdateTime{0.0};

	FVector m_RelevantAvgZombieLocation{FVector::ZeroVector};
	ABaseZombie* m_ClosestZombie{nullptr};
	EPerceivedZombieType m_ClosestZombieType{EPerceivedZombieType::Normal};
	double m_ClosestDistance{0.0};
	bool m_ZombieClose{false};
	float m_ThreatLevel{0.0f};

public:
	double ItemRememberRadius{1000.0};
	double ItemPickupRadius{300.0};
	double ZombieRelevanceRadius{3000.0};
	double FleeDistance{300.0};
	double HouseVisitDelay{60.0};
};
