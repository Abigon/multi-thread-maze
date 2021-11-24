#include "MazeGraphThread.h"
#include <random>


void FMazeGraphThread::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	GenerateMazeBlock(CurrentThread);
}

void FMazeGraphThread::GenerateMazeBlock(ENamedThreads::Type CurrentThread)
{
	WallsInfo.Empty();
	ResultWallsInfo.Empty();

	int32 CellID = 0;
	for (int32 y = 0; y < SideSize; y++)
	{
		bool bIsLastRow = y == (SideSize - 1);
		for (int32 x = 0; x < SideSize; x++)
		{
			if (x != (SideSize - 1))
			{
				auto NewWallX = FWallInfo(CellID, CellID + 1, x, y, false);
				WallsInfo.Add(NewWallX);
			}
			if (!bIsLastRow)
			{
				auto NewWallY = FWallInfo(CellID, CellID + SideSize, x, y, true);
				WallsInfo.Add(NewWallY);
			}
			CellID++;
		}
	}

	while (IsHasDifferentZone())
	{
		int32 WallNum = GetRandomInt(0, WallsInfo.Num() - 1);
		if (WallsInfo[WallNum].OneSideID != WallsInfo[WallNum].OtherSideID)
		{
			ChangeZone(WallsInfo[WallNum].OtherSideID, WallsInfo[WallNum].OneSideID);
			auto a = WallsInfo[WallNum];
			WallsInfo.RemoveAt(WallNum);
		}
		else
		{
			auto a = WallsInfo[WallNum];
			if (bIsShow)
			{
				ShowWall(CurrentThread, a);
			}
			ResultWallsInfo.Add(WallsInfo[WallNum]);
			WallsInfo.RemoveAt(WallNum);
		}
	}
	if (bIsShow)
	{
		for (auto Wall : WallsInfo)
		{
			ShowWall(CurrentThread, Wall);
		}
	}
	else
	{
		ResultWallsInfo.Append(WallsInfo);
	}

	if (SidesWallsFlag > 0)
	{
		const auto HoleX = GetRandomInt(0, SideSize - 1);
		const auto HoleY = GetRandomInt(0, SideSize - 1);
		for (int32 a = 0; a < SideSize; a++)
		{
			if (((SidesWallsFlag % 2) > 0) && (a != HoleX))
			{
				if (bIsShow)
				{
					ShowWall(CurrentThread, FWallInfo(0, 0, SideSize - 1, a, false));
				}
				else
				{
					ResultWallsInfo.Add(FWallInfo(0, 0, SideSize - 1, a, false));
				}
			}
			if ((SidesWallsFlag > 1) && (a != HoleY))
			{
				if (bIsShow)
				{
					ShowWall(CurrentThread, FWallInfo(0, 0, a, SideSize - 1, true));
				}
				else
				{
					ResultWallsInfo.Add(FWallInfo(0, 0, a, SideSize - 1, true));
				}
			}
		}
	}
	if (!bIsShow)
	{
		SimpleOutput.WallsInfo.Empty();
		SimpleOutput.WallsInfo.Append(ResultWallsInfo);
	}
	TGraphTask<FTask_FinishMazeBlock>::CreateTask(NULL, CurrentThread).ConstructAndDispatchWhenReady(MazeTaskOnWorkDone, SimpleOutput);
}

bool FMazeGraphThread::IsHasDifferentZone()
{
	for (auto aaaa : WallsInfo)
	{
		if (aaaa.OneSideID != aaaa.OtherSideID) return true;
	}
	return false;
}

void FMazeGraphThread::ChangeZone(int32 OldZone, int32 NewZone)
{
	for (int32 a = 0; a < WallsInfo.Num(); a++)
	{
		if (WallsInfo[a].OneSideID == OldZone) WallsInfo[a].OneSideID = NewZone;
		if (WallsInfo[a].OtherSideID == OldZone) WallsInfo[a].OtherSideID = NewZone;
	}
	for (int32 a = 0; a < ResultWallsInfo.Num(); a++)
	{
		if (ResultWallsInfo[a].OneSideID == OldZone) ResultWallsInfo[a].OneSideID = NewZone;
		if (ResultWallsInfo[a].OtherSideID == OldZone) ResultWallsInfo[a].OtherSideID = NewZone;
	}
}

void FMazeGraphThread::ShowWall(ENamedThreads::Type CurrentThread, FWallInfo Wall)
{
	FMazeWallDrawInfo Res;
	Res.x = Wall.WallX;
	Res.y = Wall.WallY;
	Res.bIsVertical = Wall.bIsVertical;
	Res.StartLocation = SimpleOutput.StartLocation;
	Res.Color = SimpleOutput.BlockColor;
	TGraphTask<FTask_MazeWallShow>::CreateTask(NULL, CurrentThread).ConstructAndDispatchWhenReady(MazeWallShow, Res);
	FPlatformProcess::Sleep(0.5f);
}

int FMazeGraphThread::GetRandomInt(int32 min, int32 max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(min, max);
	return distr(gen);
}