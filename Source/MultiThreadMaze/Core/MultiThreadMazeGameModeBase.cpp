// Copyright Epic Games, Inc. All Rights Reserved.


#include "MultiThreadMazeGameModeBase.h"
#include "../Meshes/Wall.h"
#include <random>

void AMultiThreadMazeGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	MazeTaskOnWorkDone.BindUFunction(this, FName("OnMazeBlockDone"));
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

	DrawMazeBorder(BlockSize * BlocksPerSide, StartLocation);

	FVector2D ShiftLocation;
	for (int32 y = 0; y < BlocksPerSide; y++)
	{
		ShiftLocation.Y = StartLocation.Y + WallSize * y * BlockSize;
		for (int32 x = 0; x < BlocksPerSide; x++)
		{
			int32 SidesFlag = 0;
			if (x < BlocksPerSide - 1) SidesFlag += 1;
			if (y < BlocksPerSide - 1) SidesFlag += 2;

			ShiftLocation.X = StartLocation.X + WallSize * x * BlockSize;

			FMazeBlockInfo NewMazeBlockInfo;
			NewMazeBlockInfo.BlockColor = GetRandomColor();
			NewMazeBlockInfo.StartLocation = ShiftLocation;
			TGraphTask<FMazeGraphThread>::CreateTask(nullptr,
				ENamedThreads::AnyThread).ConstructAndDispatchWhenReady(MazeTaskOnWorkDone, MazeWallShow, NewMazeBlockInfo, BlockSize, SidesFlag, bIsShow);

			BlockCounts++;
		}
	}
}

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

void AMultiThreadMazeGameModeBase::ClearMazes()
{
	BlockCounts = 0;
	for (auto Wall : AllWalls)
	{
		if (Wall) Wall->Destroy();
	}
	AllWalls.Empty();
}

void AMultiThreadMazeGameModeBase::DrawMazeBorder(const int32 SideSize, FVector2D StartLocation)
{
	const auto World = GetWorld();
	if (!World || !WallClass) return;

	TArray<AWall*> MazeWalls;

	for (int32 x = 0; x < SideSize; x++)
	{
		FVector SpawnLocLeft = FVector(StartLocation.X + x * 128, StartLocation.Y - 64.f, 6.4f);
		FVector SpawnLocRight = FVector(StartLocation.X + x * 128, StartLocation.Y + SideSize * 128 - 64.f, 6.4f);
		FRotator SpawnRot = FRotator(0);
		for (int32 a = 0; a < 2; a++)
		{
			if ((a == 0) && (x == 1)) continue; // skip for Maze's Enter
			if ((a == 1) && (x == (SideSize-2))) continue; // skip for Maze's Exit
			auto NewWallMesh = World->SpawnActor<AWall>(WallClass, (a == 0) ? SpawnLocLeft : SpawnLocRight, SpawnRot, FActorSpawnParameters());
			if (NewWallMesh)
			{
				NewWallMesh->SetColor(FLinearColor::Blue);
				MazeWalls.Add(NewWallMesh);
			}
		}
	}

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
	AllWalls.Append(MazeWalls);
}

FVector2D AMultiThreadMazeGameModeBase::GetMazeStartLocation(const int32 SideSize)
{
	float X = -SideSize * WallSize / 2 + 64.f;
	float Y = -SideSize * WallSize / 2 + 64.f;
	return FVector2D(X, Y);
}

void AMultiThreadMazeGameModeBase::OnMazeBlockDone(FMazeBlockInfo Result)
{
	for (auto a : Result.WallsInfo)
	{
		const auto Wall = SpawnWall(a.WallX, a.WallY, a.bIsVertical, Result.StartLocation, Result.BlockColor);
		if (Wall)
		{
			AllWalls.Add(Wall);
		}
	}
	BlockCounts--;
	if (BlockCounts <= 0)
	{
		OnAllBlocksDone.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("All Blocks Complite"));
	}
}

void AMultiThreadMazeGameModeBase::OnMazeWallShow(FMazeWallDrawInfo Result)
{
	const auto Wall = SpawnWall(Result.x, Result.y, Result.bIsVertical, Result.StartLocation, Result.Color);
	if (Wall)
	{
		AllWalls.Add(Wall);
	}
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
