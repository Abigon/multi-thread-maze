// Copyright Epic Games, Inc. All Rights Reserved.


#include "MultiThreadMazeGameModeBase.h"
#include "../Meshes/Wall.h"
#include "Kismet/GameplayStatics.h"
#include <random>

void AMultiThreadMazeGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	OnMazeSegmentWorkDone.BindUFunction(this, FName("OnMazeSegmentDone"));
	MazeWallShow.BindUFunction(this, FName("OnMazeWallShow"));
}

void AMultiThreadMazeGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	Super::EndPlay(EndPlayReason);
}

void AMultiThreadMazeGameModeBase::GenerateMazes(int32 BlockSize, int32 BlocksPerSide, bool bIsShow)
{
	ClearMazes();
	const auto StartLocation = GetMazeStartLocation(BlockSize * BlocksPerSide);

	//Build Maze's Border around all Blocks with Enter & Exit
	DrawMazeBorder(BlockSize * BlocksPerSide, StartLocation);

	FVector2D ShiftLocation;
	for (int32 y = 0; y < BlocksPerSide; y++)
	{
		ShiftLocation.Y = StartLocation.Y + WallSize * y * BlockSize;
		for (int32 x = 0; x < BlocksPerSide; x++)
		{
			// Флаг отображения внутренних стен между сегментами
			// 1 - правая стена
			// 2 - нижняя стена
			// 3 - комбинация 1 и 2. Можно проверить побитно
			int32 SidesFlag = 0;
			if (x < BlocksPerSide - 1) SidesFlag += 1;
			if (y < BlocksPerSide - 1) SidesFlag += 2;

			// Расчет начального положения отображения сегмента
			// Расчитывается в этом цикле, чтобы делать меньше расчетов
			// Иначе можно вынести в функцию и там расчитывать для каждого сегмената на основе X и Y
			ShiftLocation.X = StartLocation.X + WallSize * x * BlockSize;

			// Создаем / заполняем структуру информации о сегменте цветом сегмента и начальным положением 
			FMazeSegmentInfo NewMazeBlockInfo;
			NewMazeBlockInfo.BlockColor = GetRandomColor();
			NewMazeBlockInfo.StartLocation = ShiftLocation;

			// Создаем задачу в новом потоке для генерации сегмента лабиринта
			TGraphTask<FMazeGraphThread>::CreateTask(nullptr,
				ENamedThreads::AnyThread).ConstructAndDispatchWhenReady(OnMazeSegmentWorkDone, MazeWallShow, NewMazeBlockInfo, BlockSize, SidesFlag, bIsShow);

			SegmentsAmount++;
		}
	}
}

// Удаляем все стены со сцены
void AMultiThreadMazeGameModeBase::ClearMazes()
{
	SegmentsAmount = 0;
	TArray<AActor*> TempWalls;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), WallClass, TempWalls);
	for (auto Wall : TempWalls)
	{
		if (Wall) Wall->Destroy();
	}
}

// Получаем стартовое положение верхнего левого угла
FVector2D AMultiThreadMazeGameModeBase::GetMazeStartLocation(const int32 SideSize)
{
	float X = -SideSize * WallSize / 2 + 64.f;
	float Y = -SideSize * WallSize / 2 + 64.f;
	return FVector2D(X, Y);
}

// Отрисовка общих границ лабиринта с входом и выходом
void AMultiThreadMazeGameModeBase::DrawMazeBorder(const int32 SideSize, FVector2D StartLocation)
{
	const auto World = GetWorld();
	if (!World || !WallClass) return;

	TArray<AWall*> MazeWalls;

	// Вертикальные стены
	for (int32 x = 0; x < SideSize; x++)
	{
		FVector SpawnLocLeft = FVector(StartLocation.X + x * 128, StartLocation.Y - 64.f, 6.4f);
		FVector SpawnLocRight = FVector(StartLocation.X + x * 128, StartLocation.Y + SideSize * 128 - 64.f, 6.4f);
		FRotator SpawnRot = FRotator(0);
		for (int32 a = 0; a < 2; a++)
		{
			if ((a == 0) && (x == 1)) continue; // skip for Maze's Enter
			if ((a == 1) && (x == (SideSize - 2))) continue; // skip for Maze's Exit
			auto NewWallMesh = World->SpawnActor<AWall>(WallClass, (a == 0) ? SpawnLocLeft : SpawnLocRight, SpawnRot, FActorSpawnParameters());
			if (NewWallMesh)
			{
				NewWallMesh->SetColor(FLinearColor::Blue);
				MazeWalls.Add(NewWallMesh);
			}
		}
	}

	// Горизонтальные стены
	for (int32 y = 0; y < SideSize; y++)
	{
		FVector SpawnLocLeft = FVector(StartLocation.X - 64.f, StartLocation.X + y * 128, 6.4f);
		FVector SpawnLocRight = FVector(StartLocation.X + SideSize * 128 - 64.f, StartLocation.X + y * 128, 6.4f);
		FRotator SpawnRot = FRotator(0.f, 90.f, 0);
		for (int32 a = 0; a < 2; a++)
		{
			auto NewWallMesh = World->SpawnActor<AWall>(WallClass, (a == 0) ? SpawnLocLeft : SpawnLocRight, SpawnRot, FActorSpawnParameters());
			if (NewWallMesh)
			{
				NewWallMesh->SetColor(FLinearColor::Blue);
				MazeWalls.Add(NewWallMesh);
			}
		}
	}
}

// Окончание генерации сегмента
// Если был не демо-режим, то спауним стены
// Проверяем на окончание генерации всех сегментов
void AMultiThreadMazeGameModeBase::OnMazeSegmentDone(FMazeSegmentInfo Result)
{
	for (auto a : Result.WallsInfo)
	{
		SpawnWall(a.WallX, a.WallY, a.bIsVertical, Result.StartLocation, Result.BlockColor);
	}
	if (--SegmentsAmount <= 0)
	{
		OnAllSegmentsDone.Broadcast();
	}
}

// Отображение стены в демо режиме
void AMultiThreadMazeGameModeBase::OnMazeWallShow(FMazeWallDrawInfo Result)
{
	SpawnWall(Result.x, Result.y, Result.bIsVertical, Result.StartLocation, Result.Color);
}

// Спауним стену 
AWall* AMultiThreadMazeGameModeBase::SpawnWall(int32 x, int32 y, bool bIsVertical, FVector2D StartLocation, FLinearColor Color)
{
	const auto World = GetWorld();
	if (!World || !WallClass) return nullptr;
	
	FVector SpawnLoc = bIsVertical ? FVector(StartLocation.X + x * 128, StartLocation.Y + y * 128 + 64, 6.4f) : FVector(StartLocation.X + x * 128 + 64, StartLocation.Y + y * 128, 6.4f);
	FRotator SpawnRot = bIsVertical ? FRotator(0) : FRotator(0.f, 90.f, 0);

	auto NewWallMesh = World->SpawnActor<AWall>(WallClass, SpawnLoc, SpawnRot, FActorSpawnParameters());
	if (NewWallMesh)
	{
		NewWallMesh->SetColor(Color);
		return NewWallMesh;
	}
	return nullptr;
}

int AMultiThreadMazeGameModeBase::GetRandomInt(int32 min, int32 max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(min, max);
	return distr(gen);
}

FLinearColor AMultiThreadMazeGameModeBase::GetRandomColor()
{
	float R = GetRandomInt(0, 255) / 1000.f;
	float G = GetRandomInt(0, 255) / 1000.f;
	float B = GetRandomInt(0, 255) / 1000.f;
	return FLinearColor::FLinearColor(R, G, B, 1.f);
}
