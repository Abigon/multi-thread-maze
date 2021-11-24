#pragma once

#include "CoreMinimal.h"
#include "MazeGraphThread.generated.h"

struct FWallInfo
{
	int32 OneSideID = -1;
	int32 OtherSideID = -1;
	int32 WallX = -1;
	int32 WallY = -1;
	bool bIsVertical = false;
	FWallInfo(int32 id1 = -1, int32 id2 = -1, int32 x = -1, int32 y = -1, bool bV = false)
		: OneSideID(id1), OtherSideID(id2), WallX(x), WallY(y), bIsVertical(bV) {};
};

USTRUCT()
struct FMazeWallDrawInfo
{
	GENERATED_USTRUCT_BODY()

	int32 x;
	int32 y;
	bool bIsVertical;
	FVector2D StartLocation;
	FLinearColor Color;
	FMazeWallDrawInfo(int32 inX = -1, int32 inY = -1, bool bV = false, FVector2D inSL= FVector2D(0), FLinearColor inColor = FLinearColor::Black)
		: x(inX), y(inY), bIsVertical(bV), StartLocation(inSL), Color(inColor) {};
};

USTRUCT()
struct FMazeSegmentInfo
{
	GENERATED_USTRUCT_BODY()

	TArray<FWallInfo> WallsInfo;
	FLinearColor BlockColor;
	FVector2D StartLocation;
};

DECLARE_DELEGATE_OneParam(FOnMazeSegmentWorkDoneSignature, FMazeSegmentInfo Result)
DECLARE_DELEGATE_OneParam(FMazeWallShowSignature, FMazeWallDrawInfo Result)


// Task for broadcast delegate when the Segment is finished 
class FTask_FinishMazeBlock
{
	FOnMazeSegmentWorkDoneSignature OnMazeSegmentWorkDone;
	FMazeSegmentInfo Result;

public:
	FTask_FinishMazeBlock(FOnMazeSegmentWorkDoneSignature InTaskDelegate_OnWorkDone, FMazeSegmentInfo InResult)
		: OnMazeSegmentWorkDone(InTaskDelegate_OnWorkDone), Result(InResult) { }
	
	~FTask_FinishMazeBlock() { }
	
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FTask_FinishMazeBlock, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread()
	{
		//run task in game thread for delegate correctly work
		return ENamedThreads::GameThread;
	}

	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::FireAndForget; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		check(IsInGameThread());
		if (OnMazeSegmentWorkDone.IsBound())
		{
			OnMazeSegmentWorkDone.Execute(Result);
		}
	}
};

// Task for broadcast delegate when to show €ÛÔ¸ÛÚÂ Wall 
class FTask_MazeWallShow
{
	FMazeWallShowSignature MazeWallShow;
	FMazeWallDrawInfo Result;

public:
	FTask_MazeWallShow(FMazeWallShowSignature InTaskDelegate_OnWallShowDone, FMazeWallDrawInfo InResult)
		: MazeWallShow(InTaskDelegate_OnWallShowDone), Result(InResult) { }

	~FTask_MazeWallShow() {  }

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FTask_MazeWallShow, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread()
	{
		//run task in game thread for delegate correctly work
		return ENamedThreads::GameThread;
	}

	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::FireAndForget; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		check(IsInGameThread());
		if (MazeWallShow.IsBound())
		{
			MazeWallShow.Execute(Result);
		}
	}
};


class FMazeGraphThread
{
	FOnMazeSegmentWorkDoneSignature OnMazeSegmentWorkDone;
	FMazeWallShowSignature MazeWallShow;

	FMazeSegmentInfo SegmentResult;
	int32 SegmentSideSize;
	int32 SidesWallsFlag;
	bool bIsShow;

	TArray<FWallInfo> WallsInfo;
	TArray<FWallInfo> ResultWallsInfo;
	
public:
	FMazeGraphThread(FOnMazeSegmentWorkDoneSignature InTaskDelegate_OnWorkDone, FMazeWallShowSignature InTaskDelegate_OnWallShowDone, 
		FMazeSegmentInfo InSimpleOutput, int32 InSideSize, int32 InWallsFlag, bool InIsShow)
		: OnMazeSegmentWorkDone(InTaskDelegate_OnWorkDone),
		MazeWallShow(InTaskDelegate_OnWallShowDone),
		SegmentResult(InSimpleOutput),
		SegmentSideSize(InSideSize),
		SidesWallsFlag(InWallsFlag),
		bIsShow(InIsShow)
	{
		UE_LOG(LogTemp, Warning, TEXT("FMazeGraphThread Constructor"));
	}
	
	~FMazeGraphThread()
	{
		UE_LOG(LogTemp, Warning, TEXT("FMazeGraphThread Destructor"));
	}

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FMazeGraphThread, STATGROUP_TaskGraphTasks); }

	static ENamedThreads::Type GetDesiredThread()
	{
		//backGround run task
		FAutoConsoleTaskPriority myTaskPriority(
			TEXT("TaskGraph.TaskPriorities.LoadFileToString"),
			TEXT("Task and thread priority for file loading."),
			ENamedThreads::BackgroundThreadPriority,
			ENamedThreads::NormalTaskPriority,
			ENamedThreads::NormalTaskPriority
		);
		return myTaskPriority.Get();		
	}

	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::FireAndForget; }
	
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent);

private:

	void GenerateMazeSegment(ENamedThreads::Type CurrentThread);
	bool IsHasDifferentZone();
	void ChangeZone(int32 OldZone, int32 NewZone);
	void ShowWall(ENamedThreads::Type CurrentThread, FWallInfo Wall);
	int GetRandomInt(int32 min, int32 max);
};
