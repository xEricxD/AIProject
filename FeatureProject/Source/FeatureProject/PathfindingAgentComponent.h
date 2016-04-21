// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PathfindingAgentComponent.generated.h"

USTRUCT()
struct FPath
{
  GENERATED_USTRUCT_BODY()

    TArray<FVector> path;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FEATUREPROJECT_API UPathfindingAgentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPathfindingAgentComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

  virtual void SetPath(FPath* a_path) {}
  virtual FPath* GetPath() { return nullptr; }
};
