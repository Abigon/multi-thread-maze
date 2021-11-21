// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiThreadMazeGameModeBase.generated.h"


UCLASS()
class MULTITHREADMAZE_API AMultiThreadMazeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable, Category = "Maze")
	void GenerateMaze(int32 SideSize, int32 CountOfThreads);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
