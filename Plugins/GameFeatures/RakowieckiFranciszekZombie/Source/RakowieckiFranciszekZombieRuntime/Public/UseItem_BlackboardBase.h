// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Items/ItemType.h"
#include "UseItem_BlackboardBase.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="Survivor Use Item"))
class RAKOWIECKIFRANCISZEKZOMBIERUNTIME_API UBTTask_SurvivorUseItem : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SurvivorUseItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Survivor")
	EItemType ItemType{EItemType::Food};
};
