//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AStarGrid.h"
#include "AStarPathfinderActor.generated.h"

struct FPath;

UCLASS()
class FEATUREPROJECT_API AAStarPathfinderActor : public AActor
{
	GENERATED_BODY()
	
public:	
	//Sets default values for this actor's properties
	AAStarPathfinderActor();

	//Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Called every frame
	virtual void Tick( float DeltaSeconds ) override;

  //Agents call this when they need a new path
  void RequestPath(UActorComponent* a_requester, FAStarCell* a_startCell, FAStarCell* a_targetCell);

private:
  void DrawDebug();

  void AddNeighbors(FAStarCell* a_cell);
  void Relax(FAStarCell* a_targetCell, FAStarCell* a_startCell, int a_cost);
  
  int GetHeuristic(FAStarCell* a_cell);
  void CreatePath(FAStarCell* a_cell, FPath* a_path);
  void Clear();

  UPROPERTY(EditAnywhere)
    uint16 m_maxSpinCount;

  UPROPERTY(EditAnywhere)
    bool m_autoClear;
  UPROPERTY(EditAnywhere)
    bool m_clear;
  UPROPERTY(EditAnywhere)
    bool m_debugdraw;

  AAStarGrid* m_grid;

  TQueue<FAStarPathfindingPacket> m_pathQueue;

  TArray<FAStarCell*> m_openList;
  TArray<FAStarCell*> m_closedList;
};
