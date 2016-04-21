//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "JPSPlusGridActor.h"
#include "JPSPlusPathfinderActor.h"
#include "PathfindingAgentComponent.h"
#include "JPSPlusAgentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FEATUREPROJECT_API UJPSPlusAgentComponent : public UPathfindingAgentComponent
{
	GENERATED_BODY()

public:	
	//Sets default values for this component's properties
  UJPSPlusAgentComponent();

	//Called when the game starts
	virtual void BeginPlay() override;
	
	//Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

  virtual void SetPath(FPath* a_path) override;
  virtual FPath* GetPath() override;

  UFUNCTION(BlueprintCallable, Category = "PathFinding")
    void CancelCurrentPath();

  UFUNCTION(BlueprintCallable, Category = "PathFinding")
    bool IsTraversingPath();

  UFUNCTION(BlueprintCallable, Category = "PathFinding")
    void CalculatePointsAndRequestPath(FVector a_targetLocation);

  void PathRequestFailed();

private:
  //debug variables
  void DrawDebugInformation();

  UPROPERTY(EditAnywhere)
    bool m_drawDebug;
  UPROPERTY(EditAnywhere)
    bool m_calculatePointsAndRequestPath;
  UPROPERTY(EditAnywhere)
    FVector m_targetPosDebug;

  AJPSPlusPathfinderActor* m_pathfinder;
  AJPSPlusGridActor* m_grid;

  FPathfindingCell* m_startCell;
  FPathfindingCell* m_targetCell;

  FPath* m_path;
  bool m_waitingForPath;	
};
