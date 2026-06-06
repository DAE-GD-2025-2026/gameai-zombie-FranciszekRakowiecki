
#include "PerceptorMemory.h"

#include "Kismet/GameplayStatics.h"
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
	if (std::find(m_SpottedZombies.begin(), m_SpottedZombies.end(), zombie) == m_SpottedZombies.end())
	{
		m_SpottedZombies.push_back(zombie);
	}
}

void FPerceptorMemory::RememberHouse(AHouse* house)
{
	if (std::find_if(m_InWorldHouses.begin(), m_InWorldHouses.end(), [&](const HouseMemory& memory) { return memory.house == house; }) == m_InWorldHouses.end())
	{
		m_InWorldHouses.emplace_back(house, FPlatformTime::Seconds(), false);
	}
}

void FPerceptorMemory::ForgetZombie(ABaseZombie* Zombie)
{
	std::erase(m_SpottedZombies, Zombie);

	if (m_ClosestZombie == Zombie)
	{
		m_ClosestZombie = nullptr;
	}
}

void FPerceptorMemory::ItemPickedUp(ABaseItem* item)
{
	std::erase(m_InWorldMemoryItems, item);
}

void FPerceptorMemory::Tick()
{
	UpdateZombieInfo();
	UpdateItemInfo();
	UpdateHouseInfo();
}

void FPerceptorMemory::UpdateItemInfo()
{
	double minDistance = std::numeric_limits<double>::max();

	m_ClosestItem = nullptr;
	
	double minFoodDistance = std::numeric_limits<double>::max();
	double minWeaponDistance = std::numeric_limits<double>::max();
	double minHealthDistance = std::numeric_limits<double>::max();

	m_Food = nullptr;
	m_Weapon = nullptr;
	m_Meds = nullptr;

	for (auto item : m_InWorldMemoryItems)
	{
		double distance = FVector::Distance(m_Owner->GetActorLocation(), item->GetActorLocation());
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
	HouseMemory* storedMemory{nullptr};
	double minDistance = std::numeric_limits<double>::max();
	for (HouseMemory& memory : m_InWorldHouses)
	{
		if (memory.visited && memory.lastVisited + HouseVisitDelay > FPlatformTime::Seconds())
			continue;
		memory.visited = false;

		double distance = FVector::Distance(m_Owner->GetActorLocation(), memory.house->GetActorLocation());

		if (distance < minDistance)
		{
			minDistance = distance;
			m_TargetHouse = memory.house;
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
	double minDistance = 1500.0;

	for (auto Zombie : m_SpottedZombies)
	{
		if (IsZombieRelevant(Zombie))
		{
			double distance = FVector::Distance(Zombie->GetActorLocation(), m_Owner->GetActorLocation());
			if (distance < minDistance)
			{
				minDistance = distance;
				m_ClosestZombie = Zombie;
			}
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

	FVector direction = position - avg;

	direction.Normalize();

	m_RelevantAvgZombieLocation = position;
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

double FPerceptorMemory::GetClosestItemDistance() const
{
	return m_ClosestDistance;
}

bool FPerceptorMemory::IsItemFar(ABaseItem* item) const
{
	double distance = FVector::DistSquared(m_Owner->GetActorLocation(), item->GetActorLocation());

	return distance > ItemRememberRadius * ItemRememberRadius;
}

bool FPerceptorMemory::IsCloseEnoughForPickup(ABaseItem* item) const
{
	double distance = FVector::DistSquared(m_Owner->GetActorLocation(), item->GetActorLocation());

	return distance < ItemPickupRadius * ItemPickupRadius;
}

bool FPerceptorMemory::IsZombieRelevant(ABaseZombie* zombie) const
{
	double distance = FVector::DistSquared(m_Owner->GetActorLocation(), zombie->GetActorLocation());

	return distance < ZombieRelevanceRadius * ZombieRelevanceRadius;
}
