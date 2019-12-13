// Fill out your copyright notice in the Description page of Project Settings.


#include "RubikCube.h"
#include "GameFramework/SpringArmComponent.h"
#include "Classes/Camera/CameraComponent.h"
#include "RubikPiece.h"
#include "Classes/Engine/World.h"
#include "Components/InputComponent.h"

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

	// Setting up the components tree
	RootComponent = Root;
	CameraSpringArm->SetupAttachment(RootComponent);
	RotatingRoot->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraSpringArm);

	// Setting up variables
	YawCameraValue = .0f;
	PitchCameraValue = .0f;

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

	
	PlayerInputComponent->BindAction("EnableCameraRotation", EInputEvent::IE_Pressed, this, &ARubikCube::ToggleCameraRotation);
	PlayerInputComponent->BindAction("EnableCameraRotation", EInputEvent::IE_Released, this, &ARubikCube::ToggleCameraRotation);

	// Binding the rotation of the srping arm
	PlayerInputComponent->BindAxis("CameraRotateX", this, &ARubikCube::CameraRotateX);
	PlayerInputComponent->BindAxis("CameraRotateY", this, &ARubikCube::CameraRotateY);
	PlayerInputComponent->BindAction("ZoomOut", EInputEvent::IE_Pressed, this, &ARubikCube::ZoomOut);
	PlayerInputComponent->BindAction("ZoomIn", EInputEvent::IE_Pressed, this, &ARubikCube::ZoomIn);
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
				if (i*9+j*3+k+1 != 14) //TODO function for generic size
				{
					ARubikPiece *  Piece = GetWorld()->SpawnActor<ARubikPiece>(PieceClass, (GetActorLocation() - FVector(100, +100, +100) + FVector(100 * j, 100 * i, 100 * k)), FRotator(0, 0, 0), params);

					Piece->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);

					Pieces.Add(Piece);
				}
			}
		}
	}
}

void ARubikCube::ToggleCameraRotation()
{
	bIsCameraRotating = !bIsCameraRotating;
	UE_LOG(LogTemp, Warning, TEXT("Test CameraRotating: %s"), (bIsCameraRotating ? TEXT("True") : TEXT("False")));
}

void ARubikCube::CameraRotateX(float Value)
{
	if (bIsCameraRotating)
	{
		FMath::Clamp(Value, -1.f, 1.f);
		YawCameraValue = Value;
		FRotator NewRotation = FRotator(0.f, YawCameraValue, 0.f);
		FQuat QuatRotation = FQuat(NewRotation);
		CameraSpringArm->AddLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	}
	//UE_LOG(LogTemp, Warning, TEXT("Test X %f"), Value);
}

void ARubikCube::CameraRotateY(float Value)
{
	if (bIsCameraRotating)
	{
	FMath::Clamp(Value, -1.f, 1.f);
	PitchCameraValue = Value;
	FRotator NewRotation = FRotator(PitchCameraValue, 0.f, 0.f);
	FQuat QuatRotation = FQuat(NewRotation);
	CameraSpringArm->AddLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	//UE_LOG(LogTemp, Warning, TEXT("Test Y %f"), Value);
	}
}

void ARubikCube::ZoomOut()
{	
	CameraSpringArm->TargetArmLength += ZoomValue; 
}

void ARubikCube::ZoomIn()
{
	CameraSpringArm->TargetArmLength += ZoomValue;
}
