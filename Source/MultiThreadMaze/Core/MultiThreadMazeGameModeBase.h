// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../MazeThreads/MazeGraphThread.h"
#include "MultiThreadMazeGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSegmentsDoneSignature);

UCLASS()
class MULTITHREADMAZE_API AMultiThreadMazeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	FOnMazeSegmentWorkDoneSignature OnMazeSegmentWorkDone;
	FMazeWallShowSignature MazeWallShow;

	UPROPERTY(BlueprintAssignable, Category = "Maze")
	FOnAllSegmentsDoneSignature OnAllSegmentsDone;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
	TSubclassOf<class AWall> WallClass;

private:
	const float WallSize = 128.f;

	// Счетчик сегментов лабиринта
	// Используется для определения, что все сегменты закончены
	int32 SegmentsAmount = 0;

public:
	UFUNCTION(BlueprintCallable, Category = "Maze")
	void GenerateMazes(int32 BlockSize, int32 BlocksPerSide, bool bIsShow);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ClearMazes();

	void DrawMazeBorder(const int32 SideSize, FVector2D StartLocation);
	AWall* SpawnWall(int32 x, int32 y, bool bIsVertical, FVector2D StartLocation, FLinearColor Color);
	FVector2D GetMazeStartLocation(const int32 SideSize);

	UFUNCTION()
	void OnMazeSegmentDone(FMazeSegmentInfo Result);
	UFUNCTION()
	void OnMazeWallShow(FMazeWallDrawInfo Result);

	int GetRandomInt(int32 min, int32 max);
	FLinearColor GetRandomColor();
};
