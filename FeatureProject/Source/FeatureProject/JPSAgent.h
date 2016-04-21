// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "JPSGridActor.h"
#include "JPSPathfinder.h"
#include "PathfindingAgentComponent.h"
#include "JPSAgent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FEATUREPROJECT_API UJPSAgent : public UActorComponent
{
	GENERATED_BODY()
};
