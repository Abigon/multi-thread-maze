#include "MazeGraphThread.h"
#include <random>


void FMazeGraphThread::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	GenerateMazeSegment(CurrentThread);
}

void FMazeGraphThread::GenerateMazeSegment(ENamedThreads::Type CurrentThread)
{
	WallsInfo.Empty();
	ResultWallsInfo.Empty();

	// Заполняем массив стен с ID
	// Идентификаторы идут слева-направо, сверху-вниз
	int32 CellID = 0;
	for (int32 y = 0; y < SegmentSideSize; y++)
	{
		bool bIsLastRow = y == (SegmentSideSize - 1);
		for (int32 x = 0; x < SegmentSideSize; x++)
		{
			if (x != (SegmentSideSize - 1))  // Не создаем крайнуюю правую стенку
			{
				//Парвая стенка: Текущий ID и следующий ID
				WallsInfo.Add(FWallInfo(CellID, CellID + 1, x, y, false));
			}
			if (!bIsLastRow) // Не создаем крайнюю нижнюю стенку
			{
				//Нижняя стенка: Текущий ID и ID ячейки ниже
				WallsInfo.Add(FWallInfo(CellID, CellID + SegmentSideSize, x, y, true));
			}
			CellID++;
		}
	}

	// Цикл длится пока есть различные зоны в массиве стен
	while (IsHasDifferentZone())
	{
		int32 WallNum = GetRandomInt(0, WallsInfo.Num() - 1);  // Вибираем рандомную стену из массива
		if (WallsInfo[WallNum].OneSideID != WallsInfo[WallNum].OtherSideID) 
		{
			// Если у стены ID разные, то меняем OtherSideID на OneSideID у всех стен с ID = OtherSideID и удаляем стену 
			ChangeZone(WallsInfo[WallNum].OtherSideID, WallsInfo[WallNum].OneSideID);
			auto a = WallsInfo[WallNum];
			WallsInfo.RemoveAt(WallNum);
		}
		else
		{
			// Если ID у стены одинаковые, то добавляем стену в результирующий массив и удаляем из массива стен
			auto a = WallsInfo[WallNum];
			if (bIsShow)
			{
				// Если работаем в демо-режиме, то выдаем делегат, что надо спаунить стену
				ShowWall(CurrentThread, a);
			}
			ResultWallsInfo.Add(WallsInfo[WallNum]);
			WallsInfo.RemoveAt(WallNum);
		}
	}
	// Если в демо-режиме, то показваем оставшиеся в массиве стены
	// Если в нормальном режиме, то добавляем оставшиеся стены в резульирующий массив
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

	// Если флаг требует создать граничные стены сегмента то создаем
	if (SidesWallsFlag > 0)
	{
		// Получаем рандомные проходы в нижней и правой границах
		const auto HoleX = GetRandomInt(0, SegmentSideSize - 1);
		const auto HoleY = GetRandomInt(0, SegmentSideSize - 1);
		for (int32 a = 0; a < SegmentSideSize; a++)
		{
			// Создаем/показываем правые граничные стены с проходом
			if (((SidesWallsFlag % 2) > 0) && (a != HoleX))
			{
				if (bIsShow)
				{
					ShowWall(CurrentThread, FWallInfo(0, 0, SegmentSideSize - 1, a, false));
				}
				else
				{
					ResultWallsInfo.Add(FWallInfo(0, 0, SegmentSideSize - 1, a, false));
				}
			}
			// Создаем/показываем нижние граничные стены с проходом
			if ((SidesWallsFlag > 1) && (a != HoleY))
			{
				if (bIsShow)
				{
					ShowWall(CurrentThread, FWallInfo(0, 0, a, SegmentSideSize - 1, true));
				}
				else
				{
					ResultWallsInfo.Add(FWallInfo(0, 0, a, SegmentSideSize - 1, true));
				}
			}
		}
	}

	// Очищаяем массив стен для передачи
	SegmentResult.WallsInfo.Empty();

	// Если не демо-режим, то добавляем стены из результирующего массива в структуру для возврата
	if (!bIsShow)
	{
		SegmentResult.WallsInfo.Append(ResultWallsInfo);
	}

	// Сообщаем об окончании генерации сегмента
	TGraphTask<FTask_FinishMazeBlock>::CreateTask(NULL, CurrentThread).ConstructAndDispatchWhenReady(OnMazeSegmentWorkDone, SegmentResult);
}

// Проверка на наличие стен с разными ID 
bool FMazeGraphThread::IsHasDifferentZone()
{
	for (auto aaaa : WallsInfo)
	{
		if (aaaa.OneSideID != aaaa.OtherSideID) return true;
	}
	return false;
}

// Замена ID у стен 
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

// Сообщаем о необходимости показать стену в демо-режиме и делам паузу в потоке
void FMazeGraphThread::ShowWall(ENamedThreads::Type CurrentThread, FWallInfo Wall)
{
	FMazeWallDrawInfo Res = FMazeWallDrawInfo(Wall.WallX, Wall.WallY, Wall.bIsVertical, SegmentResult.StartLocation, SegmentResult.BlockColor);
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