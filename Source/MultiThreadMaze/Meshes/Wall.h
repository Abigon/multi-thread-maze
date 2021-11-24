// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wall.generated.h"


UCLASS()
class MULTITHREADMAZE_API AWall : public AActor
{
	GENERATED_BODY()
	

public:
	bool bIsBorder = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	class USceneComponent* SceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	class UStaticMeshComponent* Mesh;

public:
	AWall();
	void SetColor(const FLinearColor& Color);

protected:
	virtual void BeginPlay() override;

};

