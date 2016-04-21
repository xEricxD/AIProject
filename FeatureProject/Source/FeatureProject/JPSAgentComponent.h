// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "JPSGridActor.h"
#include "JPSPathfinder.h"
#include "PathfindingAgentComponent.h"
#include "JPSAgentComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FEATUREPROJECT_API UJPSAgentComponent : public UPathfindingAgentComponent
{
	GENERATED_BODY()

public:
  UJPSAgentComponent();

  // Called when the game starts
  virtual void BeginPlay() override;

  // Called every frame
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

  virtual void SetPath(FPath* a_path) override;
  virtual FPath* GetPath() override;

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

  AJPSPathfinderActor* m_pathfinder;
  AJPSGridActor* m_grid;

  FJPSCell* m_startCell;
  FJPSCell* m_targetCell;

  FPath* m_path;
  bool m_waitingForPath;	
};
