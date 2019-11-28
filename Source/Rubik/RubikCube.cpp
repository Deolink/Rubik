// Fill out your copyright notice in the Description page of Project Settings.


#include "RubikCube.h"
#include "GameFramework/SpringArmComponent.h"
#include "Classes/Camera/CameraComponent.h"
#include "RubikPiece.h"
#include "Classes/Engine/World.h"

// Sets default values
ARubikCube::ARubikCube()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initializing the components
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>("CameraSpringArm");
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RotatingRoot = CreateDefaultSubobject<USceneComponent>("RotatingRoot");

	CameraSpringArm->bDoCollisionTest=false;

	// Seting up the components tree
	RootComponent = Root;
	CameraSpringArm->SetupAttachment(RootComponent);
	RotatingRoot->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraSpringArm);

	

}

// Called when the game starts or when spawned
void ARubikCube::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnPieces();
}

// Called every frame
void ARubikCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARubikCube::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARubikCube::SpawnPieces()
{
	// For now we use 3 but probably we should be able to choose our cube size in future
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				FActorSpawnParameters params;
				params.Owner = this;

				ARubikPiece *  Piece = GetWorld()->SpawnActor<ARubikPiece>(PieceClass, GetActorLocation() + FVector(100 * j, 100 * i,100 * k), FRotator(0, 0, 0), params);

				Piece->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);

				Pieces.Add(Piece);
			}
		}
	}
}