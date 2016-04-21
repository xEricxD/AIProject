//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CapturePointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FEATUREPROJECT_API UCapturePointComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	//Sets default values for this component's properties
	UCapturePointComponent();

	//Called when the game starts
	virtual void BeginPlay() override;
	
	//Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

		
	
};
