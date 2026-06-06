
#include "BlendedSteering.h"

void BlendedSteering::AddSteering(std::unique_ptr<SteeringBehavior> steering, float weight)
{
	m_Steerings.emplace_back(std::move(steering), weight);
}

FVector BlendedSteering::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	FVector output = FVector::ZeroVector;
	for (auto& steering : m_Steerings)
	{
		output += steering.steeringBehavior->GetOutput(params, memory, owner) * steering.weight;
	}

	return output.GetSafeNormal2D();
}
