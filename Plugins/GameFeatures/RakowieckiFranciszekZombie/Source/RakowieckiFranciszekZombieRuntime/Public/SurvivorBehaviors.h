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
	virtual FVector GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner) override;
};

class FindHouse : public SteeringBehavior
{
public:
	virtual FVector GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner) override;
};