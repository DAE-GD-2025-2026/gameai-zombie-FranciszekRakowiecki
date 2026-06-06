#include "SurvivorBehaviors.h"

FVector FleeZombies::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	return FVector::ZeroVector;
}

FVector AvoidPurgeZones::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	return FVector::ZeroVector;
}

FVector FindHouse::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	return FVector::ZeroVector;
}
