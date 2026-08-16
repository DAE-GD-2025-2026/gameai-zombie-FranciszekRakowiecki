// Fill out your copyright notice in the Description page of Project Settings.


#include "UseItem_BlackboardBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "StudentPerceptor.h"

UBTTask_SurvivorUseItem::UBTTask_SurvivorUseItem()
{
	NodeName = TEXT("Survivor Use Item");
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SurvivorUseItem, BlackboardKey), AActor::StaticClass());
	BlackboardKey.SelectedKeyName = TEXT("Survivor");
}

EBTNodeResult::Type UBTTask_SurvivorUseItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();
	AActor* survivor = blackboard ? Cast<AActor>(blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName)) : nullptr;

	if (!survivor)
	{
		const AAIController* controller = OwnerComp.GetAIOwner();
		survivor = controller ? controller->GetPawn() : nullptr;
	}

	UStudentPerceptor* perceptor = survivor ? survivor->FindComponentByClass<UStudentPerceptor>() : nullptr;
	if (!perceptor)
	{
		return EBTNodeResult::Failed;
	}

	perceptor->OnUseItem(ItemType);
	return EBTNodeResult::Succeeded;
}
