#include "SurvivorBehaviors.h"

#include "PurgeZones/PurgeZone.h"
#include "Village/House/House.h"

FVector FleeZombies::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	if (memory.GetZombie() != nullptr)
	{
		return (owner->GetActorLocation() - memory.GetRelZombieLoc()).GetSafeNormal();
	}
	return FVector::ZeroVector;
}

FVector AvoidPurgeZones::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	FVector position = owner->GetActorLocation();
	for (APurgeZone* zone : memory.GetPurgeZones())
	{
		if (IsWithinPurgeZoneRange(owner, zone))
		{
			FVector directionToPlayer = (position - zone->GetActorLocation()).GetSafeNormal();
			position += directionToPlayer * 400.0;
		}
	}
	return position - owner->GetActorLocation();
}

bool AvoidPurgeZones::IsWithinPurgeZoneRange(AActor* actor, APurgeZone* zone)
{
	FVector extent = { AvoidanceRadius, AvoidanceRadius, 0.0 };
	FVector min = zone->GetActorLocation() - extent;
	FVector max = zone->GetActorLocation() + extent;
	FVector position = actor->GetActorLocation();

	bool x = min.X <= position.X && position.X <= max.X;
	bool y = min.Y <= position.Y && position.Y <= max.Y;

	return x && y;
}

FVector FindHouse::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	AHouse* house = memory.GetHouse();
	if (house != nullptr)
	{
		return (owner->GetActorLocation() - memory.GetHouse()->GetActorLocation()).GetSafeNormal();
	}
	return FVector::ZeroVector;
}
