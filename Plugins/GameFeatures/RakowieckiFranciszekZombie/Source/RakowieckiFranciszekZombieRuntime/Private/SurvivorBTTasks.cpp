#include "SurvivorBTTasks.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "NavigationSystem.h"
#include "StudentPerceptor.h"
#include "Items/ItemType.h"

UStudentPerceptor* UBTTask_SurvivorBase::FindPerceptor(UBehaviorTreeComponent& OwnerComp) const
{
	const AAIController* controller = OwnerComp.GetAIOwner();
	APawn* pawn = controller ? controller->GetPawn() : nullptr;
	return pawn ? pawn->FindComponentByClass<UStudentPerceptor>() : nullptr;
}

UBTTask_SurvivorShootZombie::UBTTask_SurvivorShootZombie()
{
	NodeName = TEXT("Survivor Shoot Zombie");
}

EBTNodeResult::Type UBTTask_SurvivorShootZombie::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UStudentPerceptor* perceptor = FindPerceptor(OwnerComp);
	if (!perceptor)
	{
		return EBTNodeResult::Failed;
	}

	// Both weapon enum cases use the currently selected weapon in the perceptor.
	perceptor->OnUseItem(EItemType::Pistol);
	return EBTNodeResult::Succeeded;
}

UBTTask_SurvivorPickupItem::UBTTask_SurvivorPickupItem()
{
	NodeName = TEXT("Survivor Pickup Item");
}

EBTNodeResult::Type UBTTask_SurvivorPickupItem::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UStudentPerceptor* perceptor = FindPerceptor(OwnerComp);
	if (!perceptor)
	{
		return EBTNodeResult::Failed;
	}

	return perceptor->TryPickupCurrentTarget() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

UBTTask_SurvivorWander::UBTTask_SurvivorWander()
{
	NodeName = TEXT("Survivor Find Wander Location");
	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SurvivorWander, TargetLocationKey));
	TargetLocationKey.SelectedKeyName = TEXT("RandomLocation");
}

EBTNodeResult::Type UBTTask_SurvivorWander::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* controller = OwnerComp.GetAIOwner();
	const APawn* pawn = controller ? controller->GetPawn() : nullptr;
	UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();
	if (!pawn || !blackboard)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(pawn->GetWorld());
	FNavLocation randomLocation;
	if (!navSystem || !navSystem->GetRandomReachablePointInRadius(pawn->GetActorLocation(), Radius, randomLocation))
	{
		return EBTNodeResult::Failed;
	}

	blackboard->SetValueAsVector(TargetLocationKey.SelectedKeyName, randomLocation.Location);
	return EBTNodeResult::Succeeded;
}
