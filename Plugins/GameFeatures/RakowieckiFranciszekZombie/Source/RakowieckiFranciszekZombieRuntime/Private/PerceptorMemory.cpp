
#include "PerceptorMemory.h"

#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "PurgeZones/PurgeZone.h"
#include "Zombies/BaseZombie.h"
#include "Village/House/House.h"

void FPerceptorMemory::SetOwner(AActor* owner)
{
	m_Owner = owner;
}

void FPerceptorMemory::RememberItem(ABaseItem* item)
{
	if (std::find(m_InWorldMemoryItems.begin(), m_InWorldMemoryItems.end(), item) == m_InWorldMemoryItems.end())
	{
		m_InWorldMemoryItems.push_back(item);
	}
}

void FPerceptorMemory::RememberZombie(ABaseZombie* zombie)
{
	if (!IsValid(zombie))
	{
		return;
	}

	const auto storedZombie = std::find_if(
		m_SpottedZombies.begin(), m_SpottedZombies.end(),
		[zombie](const ZombieMemory& memory) { return memory.zombie == zombie; });
	if (storedZombie == m_SpottedZombies.end())
	{
		m_SpottedZombies.push_back({zombie, ClassifyZombie(zombie)});
	}
}

void FPerceptorMemory::RememberHouse(AHouse* house)
{
	if (!house)
	{
		return;
	}
	if (std::find_if(m_InWorldHouses.begin(), m_InWorldHouses.end(), [&](const HouseMemory& memory) { return memory.house == house; }) == m_InWorldHouses.end())
	{
		m_InWorldHouses.emplace_back(house, FPlatformTime::Seconds(), false);
	}
}

void FPerceptorMemory::RememberPurgeZone(APurgeZone* purgeZone)
{
	if (std::find(m_InWorldPurgeZones.begin(), m_InWorldPurgeZones.end(), purgeZone) == m_InWorldPurgeZones.end())
	{
		m_InWorldPurgeZones.push_back(purgeZone);
	}
}

void FPerceptorMemory::ForgetZombie(ABaseZombie* Zombie)
{
	std::erase_if(
		m_SpottedZombies,
		[Zombie](const ZombieMemory& memory) { return memory.zombie == Zombie; });

	if (m_ClosestZombie == Zombie)
	{
		m_ClosestZombie = nullptr;
	}
}

void FPerceptorMemory::ForgetPurgeZone(APurgeZone* PurgeZone)
{
	std::erase(m_InWorldPurgeZones, PurgeZone);
}

void FPerceptorMemory::ItemPickedUp(ABaseItem* item)
{
	std::erase(m_InWorldMemoryItems, item);
}

void FPerceptorMemory::Tick()
{
	std::erase_if(m_InWorldMemoryItems, [](ABaseItem* item) { return !IsValid(item); });
	std::erase_if(
		m_SpottedZombies,
		[](const ZombieMemory& memory) { return !IsValid(memory.zombie); });
	std::erase_if(m_InWorldHouses, [](const HouseMemory& memory) { return !IsValid(memory.house); });
	std::erase_if(m_InWorldPurgeZones, [](APurgeZone* zone) { return !IsValid(zone); });

	UpdateZombieInfo();
	UpdateItemInfo();
	const double currentTime = FPlatformTime::Seconds();
	if (currentTime >= m_NextHouseUpdateTime)
	{
		UpdateHouseInfo();
		m_NextHouseUpdateTime = currentTime + 1.0;
	}
}

void FPerceptorMemory::UpdateItemInfo()
{
	double minDistance = std::numeric_limits<double>::max();

	m_ClosestItem = nullptr;
	m_ClosestDistance = minDistance;
	
	double minFoodDistance = std::numeric_limits<double>::max();
	double minWeaponDistance = std::numeric_limits<double>::max();
	double minHealthDistance = std::numeric_limits<double>::max();

	m_Food = nullptr;
	m_Weapon = nullptr;
	m_Meds = nullptr;

	for (auto item : m_InWorldMemoryItems)
	{
		double distance = FVector::Distance(m_Owner->GetActorLocation(), item->GetActorLocation());
		if (distance > ItemRememberRadius)
			continue;
		if (distance < minDistance)
		{
			minDistance = distance;
			m_ClosestItem = item;
			m_ClosestDistance = distance;
		}
		switch (item->GetItemType())
		{
			case EItemType::Food:
				if (distance < minFoodDistance)
				{
					minFoodDistance = distance;
					m_Food = item;
				}
				break;
			case EItemType::Medkit:
				if (distance < minHealthDistance)
				{
					minHealthDistance = distance;
					m_Meds = item;
				}
				break;
			case EItemType::Shotgun:
			case EItemType::Pistol:
				if (distance < minWeaponDistance)
				{
					minWeaponDistance = distance;
					m_Weapon = item;
				}
				break;
			case EItemType::Garbage:
				break;
		}
	}
}

void FPerceptorMemory::UpdateHouseInfo()
{
	m_TargetHouse = nullptr;
	m_TargetHousePathDistance = 0.0;
	HouseMemory* storedMemory{nullptr};
	double minDistance = std::numeric_limits<double>::max();
	for (HouseMemory& memory : m_InWorldHouses)
	{
		if (memory.visited && memory.lastVisited + HouseVisitDelay > FPlatformTime::Seconds())
			continue;
		memory.visited = false;

		double distance = GetPathDistanceTo(memory.house->GetActorLocation());
		if (distance < 0.0)
		{
			continue;
		}

		if (distance < minDistance)
		{
			minDistance = distance;
			m_TargetHouse = memory.house;
			m_TargetHousePathDistance = distance;
			storedMemory = &memory;
		}
	}

	if (storedMemory && IsWithinBounds(storedMemory->house))
	{
		storedMemory->visited = true;
		storedMemory->lastVisited = FPlatformTime::Seconds();
	}
}

bool FPerceptorMemory::IsWithinBounds(AHouse* house) const
{
	FHouseBounds bounds = house->GetBounds();
	FVector min = bounds.Origin - bounds.Extent;
	FVector max = bounds.Extent + bounds.Origin;

	FVector position = m_Owner->GetActorLocation();
	
	bool x = position.X >= min.X && position.X <= max.X;
	bool y = position.Y >= min.Y && position.Y <= max.Y;
	return x && y;
}

ABaseItem* FPerceptorMemory::GetFood() const
{
	return m_Food;
}

ABaseItem* FPerceptorMemory::GetWeapon() const
{
	return m_Weapon;
}

ABaseItem* FPerceptorMemory::GetMeds() const
{
	return m_Meds;
}

void FPerceptorMemory::UpdateZombieInfo()
{
	FVector avg{};
	uint32_t count{0};
	m_ClosestZombie = nullptr;
	m_ClosestZombieType = EPerceivedZombieType::Normal;
	double minDistance = 1500.0;
	m_ZombieClose = false;
	m_ThreatLevel = 0.0f;

	for (const ZombieMemory& zombieMemory : m_SpottedZombies)
	{
		ABaseZombie* Zombie = zombieMemory.zombie;
		if (IsZombieRelevant(Zombie))
		{
			double distance = FVector::Distance(Zombie->GetActorLocation(), m_Owner->GetActorLocation());
			float maxSpeed = Zombie->GetMovementComponent()->GetMaxSpeed();
			float typeMultiplier = 1.0f;
			switch (zombieMemory.type)
			{
			case EPerceivedZombieType::Runner:
				typeMultiplier = 1.35f;
				break;
			case EPerceivedZombieType::Heavy:
				typeMultiplier = 1.6f;
				break;
			case EPerceivedZombieType::Normal:
				break;
			}
			const float proximity = 1.0f - FMath::Clamp(static_cast<float>(distance / ZombieRelevanceRadius), 0.0f, 1.0f);
			m_ThreatLevel += proximity * typeMultiplier;
			double threatDistance = distance;
			if (threatDistance < minDistance)
			{
				minDistance = threatDistance;
				m_ClosestZombie = Zombie;
				m_ClosestZombieType = zombieMemory.type;
			}
			if (distance < 400.0 + maxSpeed * 0.5)
				m_ZombieClose = true;
			avg += Zombie->GetActorLocation();
			count++;
		}
	}

	if (count == 0)
	{
		m_RelevantAvgZombieLocation = FVector::ZeroVector;
		m_RelevantAvgZombieLocation.Z = m_Owner->GetActorLocation().Z;
		return;
	}

	avg /= double(count);

	FVector position = m_Owner->GetActorLocation();

	m_RelevantAvgZombieLocation = avg;
	m_RelevantAvgZombieLocation.Z = m_Owner->GetActorLocation().Z;
}

ABaseZombie* FPerceptorMemory::GetZombie() const
{
	return m_ClosestZombie;
}

ABaseItem* FPerceptorMemory::GetClosestItem() const
{
	return m_ClosestItem;
}

AHouse* FPerceptorMemory::GetHouse() const
{
	return m_TargetHouse;
}

const std::vector<APurgeZone*>& FPerceptorMemory::GetPurgeZones() const
{
	return m_InWorldPurgeZones;
}

double FPerceptorMemory::GetClosestItemDistance() const
{
	return m_ClosestDistance;
}

double FPerceptorMemory::GetDistanceTo(const AActor* actor) const
{
	return actor && m_Owner
		? FVector::Distance(m_Owner->GetActorLocation(), actor->GetActorLocation())
		: std::numeric_limits<double>::max();
}

double FPerceptorMemory::GetPathDistanceTo(const FVector& destination) const
{
	if (!m_Owner)
	{
		return -1.0;
	}

	UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(m_Owner->GetWorld());
	if (!navSystem)
	{
		return -1.0;
	}

	UNavigationPath* path = navSystem->FindPathToLocationSynchronously(
		m_Owner->GetWorld(), m_Owner->GetActorLocation(), destination, m_Owner);
	if (!path || !path->IsValid() || path->PathPoints.Num() < 2)
	{
		return -1.0;
	}

	double distance = 0.0;
	for (int32 index = 1; index < path->PathPoints.Num(); ++index)
	{
		distance += FVector::Distance(path->PathPoints[index - 1], path->PathPoints[index]);
	}
	return distance;
}

bool FPerceptorMemory::GetZombieCloseEnough() const
{
	return m_ZombieClose;
}

bool FPerceptorMemory::IsItemFar(ABaseItem* item) const
{
	double distance = FVector::DistSquared(m_Owner->GetActorLocation(), item->GetActorLocation());

	return distance > ItemRememberRadius * ItemRememberRadius;
}

bool FPerceptorMemory::IsCloseEnoughForPickup(ABaseItem* item) const
{
	if (!IsValid(item) || !m_Owner)
	{
		return false;
	}

	const double distance = FVector::Dist2D(m_Owner->GetActorLocation(), item->GetActorLocation());
	return distance <= ItemPickupRadius;
}

bool FPerceptorMemory::IsZombieRelevant(ABaseZombie* zombie) const
{
	double distance = FVector::DistSquared(m_Owner->GetActorLocation(), zombie->GetActorLocation());

	return distance < ZombieRelevanceRadius * ZombieRelevanceRadius;
}

EPerceivedZombieType FPerceptorMemory::ClassifyZombie(const ABaseZombie* zombie) const
{
	if (!IsValid(zombie))
	{
		return EPerceivedZombieType::Normal;
	}

	const FString className = zombie->GetClass()->GetName();
	if (className.Contains(TEXT("Runner"), ESearchCase::IgnoreCase))
	{
		return EPerceivedZombieType::Runner;
	}
	if (className.Contains(TEXT("Heavy"), ESearchCase::IgnoreCase))
	{
		return EPerceivedZombieType::Heavy;
	}
	return EPerceivedZombieType::Normal;
}
