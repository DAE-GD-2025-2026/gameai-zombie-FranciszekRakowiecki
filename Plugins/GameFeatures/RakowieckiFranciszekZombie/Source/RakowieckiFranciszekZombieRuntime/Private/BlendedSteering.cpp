
#include "BlendedSteering.h"

void BlendedSteering::AddSteering(std::unique_ptr<SteeringBehavior> steering, float weight)
{
	m_Steerings.emplace_back(std::move(steering), weight);
}

bool BlendedSteering::HasOutput() const
{
	return m_HasOutput;
}

FVector BlendedSteering::GetOutput(const SurvivorParams& params, const FPerceptorMemory& memory, AActor* owner)
{
	FVector output = FVector::ZeroVector;
	m_HasOutput = false;
	for (auto& steering : m_Steerings)
	{
		FVector value = steering.steeringBehavior->GetOutput(params, memory, owner) * steering.weight;
		if (value.X != 0 || value.Y != 0)
			m_HasOutput = true;
		output += value;
	}

	return output.GetSafeNormal2D();
}
