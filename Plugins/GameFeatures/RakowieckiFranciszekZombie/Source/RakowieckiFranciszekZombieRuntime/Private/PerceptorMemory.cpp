
#include "PerceptorMemory.h"

void FPerceptorMemory::SetOwner(AActor* owner)
{
	m_Owner = owner;
}

void FPerceptorMemory::RememberItem(ABaseItem* item)
{
	if (IsItemFar(item))
		return;
	if (std::find(m_InWorldMemoryItems.begin(), m_InWorldMemoryItems.end(), item) == m_InWorldMemoryItems.end())
	{
		m_InWorldMemoryItems.push_back(item);
	}
}

void FPerceptorMemory::ItemPickedUp(ABaseItem* item)
{
	std::erase(m_InWorldMemoryItems, item);
}

void FPerceptorMemory::Tick()
{
	std::erase_if(m_InWorldMemoryItems, [&](ABaseItem* item) { return IsItemFar(item); });
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

bool FPerceptorMemory::IsItemFar(ABaseItem* item)
{
	double distance = FVector::DistSquared(m_Owner->GetActorLocation(), item->GetActorLocation());

	return distance > ItemRememberRadius * ItemRememberRadius;
}

bool FPerceptorMemory::IsCloseEnoughForPickup(ABaseItem* item)
{
	double distance = FVector::DistSquared(m_Owner->GetActorLocation(), item->GetActorLocation());

	return distance < ItemPickupRadius * ItemPickupRadius;
}
