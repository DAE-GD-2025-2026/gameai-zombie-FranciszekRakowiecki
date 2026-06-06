// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "BlendedSteering.h"
#include "PerceptorMemory.h"
#include "SurvivorParams.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptor.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RAKOWIECKIFRANCISZEKZOMBIERUNTIME_API UStudentPerceptor : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStudentPerceptor();
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable, Category="Student Perceptor")
	virtual void OnPickupItem(ABaseItem* Item);
	UFUNCTION(BlueprintCallable, Category="Student Perceptor")
	virtual void OnUseItem(EItemType ItemType);

	UFUNCTION()
	void OnZombieActorDestroyed(AActor* DestroyedActor);

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	APawn* Self{nullptr};
	AAIController* Controller{nullptr};
	UBlackboardComponent* Blackboard{nullptr};
	UInventoryComponent* Inventory{nullptr};
	UHealthComponent* Health{nullptr};
	UStaminaComponent* Stamina{nullptr};

	FPerceptorMemory Memory{};
	SurvivorParams Parameters{};

	FVector MovementDirection{};

	std::unique_ptr<BlendedSteering> Steering{};
	
	void print(const char* message);
	void print(const FString& message);

	void AddItemToInventory(ABaseItem* Item);

	void UseItem(ABaseItem* Item);

	void UpdateBlackboardValues();
	void UpdateInventoryStoredInfo();
	void UpdateHealthInfo();
};
