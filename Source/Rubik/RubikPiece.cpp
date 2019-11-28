// Fill out your copyright notice in the Description page of Project Settings.


#include "RubikPiece.h"

// Sets default values
ARubikPiece::ARubikPiece()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARubikPiece::BeginPlay()
{
	Super::BeginPlay();
	StartPosition = GetActorLocation();
}

// Called every frame
void ARubikPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ARubikPiece::IsAtStartPosition()
{
	if (StartPosition.Equals(GetActorLocation(), 0.01f))
	{
		return true;
	}
	return false;
}

