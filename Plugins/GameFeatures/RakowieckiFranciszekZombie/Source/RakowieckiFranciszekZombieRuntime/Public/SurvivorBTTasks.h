#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SurvivorBTTasks.generated.h"

class UStudentPerceptor;

UCLASS(Abstract)
class RAKOWIECKIFRANCISZEKZOMBIERUNTIME_API UBTTask_SurvivorBase : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	UStudentPerceptor* FindPerceptor(UBehaviorTreeComponent& OwnerComp) const;
};

UCLASS()
class RAKOWIECKIFRANCISZEKZOMBIERUNTIME_API UBTTask_SurvivorShootZombie : public UBTTask_SurvivorBase
{
	GENERATED_BODY()

public:
	UBTTask_SurvivorShootZombie();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

UCLASS()
class RAKOWIECKIFRANCISZEKZOMBIERUNTIME_API UBTTask_SurvivorPickupItem : public UBTTask_SurvivorBase
{
	GENERATED_BODY()

public:
	UBTTask_SurvivorPickupItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

UCLASS()
class RAKOWIECKIFRANCISZEKZOMBIERUNTIME_API UBTTask_SurvivorWander : public UBTTask_SurvivorBase
{
	GENERATED_BODY()

public:
	UBTTask_SurvivorWander();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category="Wander", meta=(ClampMin="0.0"))
	float Radius{1000.0f};
};
