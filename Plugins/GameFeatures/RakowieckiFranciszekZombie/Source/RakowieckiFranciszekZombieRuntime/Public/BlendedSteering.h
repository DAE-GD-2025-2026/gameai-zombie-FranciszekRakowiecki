
#pragma once
#include <memory>

#include "SteeringBehavior.h"

class BlendedSteering : public SteeringBehavior
{
	struct Blend
	{
		std::unique_ptr<SteeringBehavior> steeringBehavior;
		float weight;
	};
	
public:
	void AddSteering(std::unique_ptr<SteeringBehavior> steering, float weight);

	FVector GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner) override;

private:
	std::vector<Blend> m_Steerings{};
};
