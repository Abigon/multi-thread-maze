// Copyright Epic Games, Inc. All Rights Reserved.


#include "MultiThreadMazeGameModeBase.h"

void AMultiThreadMazeGameModeBase::BeginPlay()
{
	Super::BeginPlay();

}

void AMultiThreadMazeGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	Super::EndPlay(EndPlayReason);
}

void AMultiThreadMazeGameModeBase::GenerateMaze(int32 SideSize, int32 CountOfThreads)
{

}
