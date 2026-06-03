// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"

#include <string>

#include "Engine/Engine.h"


UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();

	print("StudentPerceptor BeginPlay");
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
		print("StudentPerceptor bound to owner perception component");
	}
	else
	{
		print("StudentPerceptor could not find owner perception component");
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	print("Saw Something!");

	print(Actor->GetName());
}

void UStudentPerceptor::print(const char* message)
{
	print(ANSI_TO_TCHAR(message));
}

void UStudentPerceptor::print(const FString& message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, message);
	}
}
