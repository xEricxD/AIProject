//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "AStarGrid.h"
#include "AStarPathfinderActor.h"
#include "PathfindingAgentComponent.h"
#include "AStarAgentComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEATUREPROJECT_API UAStarAgentComponent : public UPathfindingAgentComponent
{
	GENERATED_BODY()

public:
  UAStarAgentComponent();

  //Called when the game starts
  virtual void BeginPlay() override;

  //Called every frame
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

  virtual void SetPath(FPath* a_path) override;
  virtual FPath* GetPath() override;

  //if the pathfinder cant find a valid path, it will call this function
  void PathRequestFailed();

  UFUNCTION(BlueprintCallable, Category = "PathFinding")
    void CancelCurrentPath();

  UFUNCTION(BlueprintCallable, Category = "PathFinding")
    bool IsTraversingPath();

  UFUNCTION(BlueprintCallable, Category = "PathFinding")
    void CalculatePointsAndRequestPath(FVector a_targetLocation);

private:
  //debug variables
  void DrawDebugInformation();

  UPROPERTY(EditAnywhere)
    bool m_drawDebug;
  UPROPERTY(EditAnywhere)
    bool m_calculatePointsAndRequestPath;
  UPROPERTY(EditAnywhere)
    FVector m_targetPosDebug;

  AAStarPathfinderActor* m_pathfinder;
  AAStarGrid* m_grid;

  FAStarCell* m_startCell;
  FAStarCell* m_targetCell;

  FPath* m_path;
  bool m_waitingForPath;
};
