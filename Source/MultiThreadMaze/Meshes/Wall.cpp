// Fill out your copyright notice in the Description page of Project Settings.


#include "Wall.h"

AWall::AWall()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
}

void AWall::BeginPlay()
{
	Super::BeginPlay();

}

void AWall::SetColor(const FLinearColor& Color)
{
	FName MaterialColorName = "BaseColor";
	const auto MaterialInst = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	if (!MaterialInst) return;

	MaterialInst->SetVectorParameterValue(MaterialColorName, Color);
}
