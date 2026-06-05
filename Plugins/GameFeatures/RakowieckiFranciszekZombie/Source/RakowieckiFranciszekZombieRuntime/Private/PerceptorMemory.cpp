
#include "PerceptorMemory.h"

#include "Zombies/BaseZombie.h"

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
		zombie->OnDestroyed.Add([&](ABaseZombie* zombie)
		{
			if (auto it = std::find(m_SpottedZombies.begin(), m_SpottedZombies.end(), zombie); it != m_SpottedZombies.end())
			{
				m_SpottedZombies.erase(it);
			}
		}); // Don't need to unbind this because the all the callbacks are getting cleared anyway
	}
}

void FPerceptorMemory::ItemPickedUp(ABaseItem* item)
{
	std::erase(m_InWorldMemoryItems, item);
}

void FPerceptorMemory::Tick()
{
	UpdateZombieInfo();
}

ABaseItem* FPerceptorMemory::GetClosestItem()
{
	double minDistance = std::numeric_limits<double>::max();
	ABaseItem* closestItem = nullptr;

	double minFoodDistance = std::numeric_limits<double>::max();
	double minWeaponDistance = std::numeric_limits<double>::max();
	double minHealthDistance = std::numeric_limits<double>::max();

	m_Food = nullptr;
	m_Weapon = nullptr;
	m_Meds = nullptr;

	for (auto item : m_InWorldMemoryItems)
	{
		double distance = FVector::DistSquared(m_Owner->GetActorLocation(), item->GetActorLocation());
		if (distance < minDistance)
		{
			minDistance = distance;
			closestItem = item;
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

	return closestItem;
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
	double minDistance = 3000.0;

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

	avg /= double(count);

	FVector position = m_Owner->GetActorLocation();

	FVector direction = position - avg;

	direction.Normalize();

	m_RelevantAvgZombieLocation = direction * 5.0 + position;
}

ABaseZombie* FPerceptorMemory::GetZombie() const
{
	return m_ClosestZombie;
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
