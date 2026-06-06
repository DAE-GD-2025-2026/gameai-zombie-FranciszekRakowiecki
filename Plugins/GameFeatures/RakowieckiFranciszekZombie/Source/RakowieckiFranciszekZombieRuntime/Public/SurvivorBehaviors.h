#pragma once
#include "SteeringBehavior.h"

class FleeZombies : public SteeringBehavior
{
public:
	virtual FVector GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner) override;
};

class AvoidPurgeZones : public SteeringBehavior
{
public:
	double AvoidanceRadius{200.0};
	virtual FVector GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner) override;
private:
	bool IsWithinPurgeZoneRange(AActor* actor, APurgeZone* zone);
};

class FindHouse : public SteeringBehavior
{
public:
	virtual FVector GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner) override;
};