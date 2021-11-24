// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiThreadMazeGameModeBase.generated.h"


USTRUCT()
struct FWallInfo
{
	GENERATED_USTRUCT_BODY()

	int32 OneSideID = -1;
	int32 OtherSideID = -1;
	int32 WallX = -1;
	int32 WallY = -1;
	bool bIsVertical = false;
	FWallInfo(int32 id1 = -1, int32 id2 = -1, int32 x =-1, int32 y = -1, bool bV = false)
		: OneSideID(id1), OtherSideID(id2), WallX(x), WallY(y), bIsVertical(bV) {};
};

UCLASS()
class MULTITHREADMAZE_API AMultiThreadMazeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
	TSubclassOf<class AWall> WallClass;

private:
	const float WallSize = 128.f;

	TArray<AWall*> AllWalls;

	TArray<FWallInfo> WallsInfo;
	TArray<FWallInfo> ResultWallsInfo;

public:
	UFUNCTION(BlueprintCallable, Category = "Maze")
	void GenerateMazes(int32 BlockSize, int32 BlocksPerSide, bool bIsShow);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	int GetRandomInt(int32 min, int32 max);
	FLinearColor GetRandomColor();

	void GenerateMazeBlock(const int32 SideSize, bool bIsShow, FVector2D StartLocation, int32 SidesWallsFlag);

	void ClearMazes();

	void DrawMazeBorder(const int32 SideSize, FVector2D StartLocation);

	bool IsHasDifferentZone();
	void ChangeZone(int32 OldZone, int32 NewZone);

	AWall* SpawnWall(int32 x, int32 y, bool bIsVertical, FVector2D StartLocation);

	FVector2D GetMazeStartLocation(const int32 SideSize);

};
