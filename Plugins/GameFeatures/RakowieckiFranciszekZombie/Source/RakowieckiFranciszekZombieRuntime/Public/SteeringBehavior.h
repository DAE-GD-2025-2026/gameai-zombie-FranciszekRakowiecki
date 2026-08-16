#pragma once
#include "PerceptorMemory.h"
#include "SurvivorParams.h"

class SteeringBehavior
{
public:
	virtual ~SteeringBehavior() = default;
	virtual FVector GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner) = 0;
};