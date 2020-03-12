// Fill out your copyright notice in the Description page of Project Settings.

#include "RubikCube.h"
#include "GameFramework/SpringArmComponent.h"
#include "Classes/Camera/CameraComponent.h"
#include "RubikPiece.h"
#include "Classes/Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>


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

	// Setting up the components hierarchy
	RootComponent = Root;	
	CameraSpringArm->SetupAttachment(RootComponent);
	RotatingRoot->SetupAttachment(RootComponent);
	Camera->SetupAttachment(CameraSpringArm);

	// Setting up variables
	YawCameraValue = .0f;
	PitchCameraValue = .0f;
	CameraSpringArm->TargetArmLength = SpringArmLenght;

	//Timeline for rotation animation
	MyTimeLine = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));	
	InterpFunction.BindUFunction(this, FName("TimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));	
}

// Called when the game starts or when spawneds
void ARubikCube::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = Cast<APlayerController>(GetController());
	PlayerController->bShowMouseCursor = true;

	//TimeLine Logic

	// Check is curve asset reference is valid, for test
	if(fCurve)
	{
		MyTimeLine->AddInterpFloat(fCurve, InterpFunction, FName("Alpha"));
		MyTimeLine->SetTimelineFinishedFunc(TimelineFinished);

		//Setting Starting Vectors
		StartRotation = RotatingRoot->RelativeRotation;
		MyTimeLine->SetLooping(false);
		MyTimeLine->SetIgnoreTimeDilation(true);
	}
	SpawnPieces();
}

// Called every frame
void ARubikCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (PlayerController == nullptr)
	{
		return;
	}
	// touch logic

	if (bIsCameraRotating)
	{
		
		CameraRotateX(DirectionCamera.X);
		CameraRotateY(DirectionCamera.Y);
		
		PlayerController->GetInputTouchState(ETouchIndex::Touch1, TouchPosition.X, TouchPosition.Y, bIsTouchingPressed);
		
		if (bIsTouchingPressed)
		{	
			
			DirectionCamera.X = TouchPosition.X - StartClickLocation.X;
			DirectionCamera.Y = StartClickLocation.Y - TouchPosition.Y;
			DirectionCamera /= 5.f;

			if (DirectionCamera.Size() > 15 )
			{
				StartClickLocation = TouchPosition;
				DirectionCamera = FVector::ZeroVector;
			}
			StartClickLocation = TouchPosition;
		}
	}

	// Probably is better to put inside the rotation axis logic 
	if (bIsFaceRotating && CubeState != ECubeState::Animating)
	{	
		PlayerController->GetInputTouchState(ETouchIndex::Touch1, TouchPosition.X, TouchPosition.Y, bIsTouchingPressed);

		// Logic to understand which way rotate the pieces
		if (PlayerController != nullptr && CubeState == ECubeState::RotatingPieces)
		{
			FVector Direction;
			FVector CurrentLocation;
			FHitResult Hit;

			if (bIsTouchingPressed)
			{
				PlayerController->GetHitResultUnderFingerByChannel(ETouchIndex::Touch1, ETraceTypeQuery::TraceTypeQuery1, false, Hit);
			}
			else
			{
				PlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, false, Hit);
			}	
		
			//TODO controllare che ho hittato un RubikPiece
			if (Hit.Actor!=nullptr){
				CurrentLocation = Hit.Location;
				Direction = StartClickLocation - CurrentLocation;
				if (Direction.Size() > 10 )
				{
					FVector NormalizedDirection = Direction.GetSafeNormal();
					AddPiecesToRotate(CubeFace, Hit.Actor->GetActorLocation(), NormalizedDirection);
				}
			}
		}
	}
}

// Called to bind functionality to input
void ARubikCube::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Binding Actions
	PlayerInputComponent->BindAction("EnableCameraRotation", EInputEvent::IE_Pressed, this, &ARubikCube::ControlHit);
	PlayerInputComponent->BindAction("EnableCameraRotation", EInputEvent::IE_Released, this, &ARubikCube::BackToIdle);
	PlayerInputComponent->BindAction("ZoomOut", EInputEvent::IE_Pressed, this, &ARubikCube::ZoomOut);
	PlayerInputComponent->BindAction("ZoomIn", EInputEvent::IE_Pressed, this, &ARubikCube::ZoomIn);
	PlayerInputComponent->BindAction("CubeFaceRotation", EInputEvent::IE_Pressed, this, &ARubikCube::ControlHit);
	PlayerInputComponent->BindAction("CubeFaceRotation", EInputEvent::IE_Released, this, &ARubikCube::BackToIdle);
	
	// Binding Axis for camera spring
	PlayerInputComponent->BindAxis("CameraRotateX", this, &ARubikCube::CameraRotateX);
	PlayerInputComponent->BindAxis("CameraRotateY", this, &ARubikCube::CameraRotateY);
	
}

void ARubikCube::SpawnPieces()
{
	// For now we use 3 but probably we should be able to choose our cube size in future
	for (int32 i = 0; i < 3; i++)
	{
		for (int32 j = 0; j < 3; j++)
		{
			for (int32 k = 0; k < 3; k++)
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

void ARubikCube::ControlHit()
{
	if (PlayerController != nullptr)
	{
		FHitResult Hit;
		PlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, false, Hit);
		if(Hit.bBlockingHit)
		{
			// get the normal of the place i hitted
			bIsFaceRotating = true;
			CubeState = ECubeState::RotatingPieces;
			StartClickLocation = Hit.Location;
			
			ClickedFace(Hit.ImpactNormal);
		}
		else{

			bIsCameraRotating = true;
			PlayerController->GetInputTouchState(ETouchIndex::Touch1, StartClickLocation.X, StartClickLocation.Y, bIsTouchingPressed);
			CubeState = ECubeState::RotatingCamera;			
		}
	}
}

void ARubikCube::CameraRotateX(float Value)
{	
	if (bIsCameraRotating && CubeState == ECubeState::RotatingCamera)
	{		
		YawCameraValue = Value * GetWorld()->GetDeltaSeconds() * AngleRotation;
		FRotator NewRotation = FRotator(0.f, YawCameraValue, 0.f);
		FQuat QuatRotation = FQuat(NewRotation);
		CameraSpringArm->AddLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	}	
}

void ARubikCube::CameraRotateY(float Value)
{
	if (bIsCameraRotating && CubeState == ECubeState::RotatingCamera)
	{
		//FMath::Clamp(Value, -1.f, 1.f);
		PitchCameraValue = Value * GetWorld()->GetDeltaSeconds() * AngleRotation;
		FRotator NewRotation = FRotator(PitchCameraValue, 0.f, 0.f);
		FQuat QuatRotation = FQuat(NewRotation);
		CameraSpringArm->AddLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	}
}

void ARubikCube::ZoomOut()
{	
	CameraSpringArm->TargetArmLength += ZoomValue; 
}

void ARubikCube::ZoomIn()
{
	CameraSpringArm->TargetArmLength-= ZoomValue;
}

void ARubikCube::BackToIdle()
{
	bIsCameraRotating = false;
	bIsFaceRotating = false;
	CubeState = ECubeState::Idle;
	CubeFace = ECFace::NotSelected;
	StartClickLocation = FVector::ZeroVector;
}

void ARubikCube::ClickedFace(FVector NormalVector)
{
	if(NormalVector.X == 1.0f)
	{
		CubeFace = ECFace::Back;
	}
	else if(NormalVector.X == -1.0f)
	{
		CubeFace = ECFace::Front;
	}

	if(NormalVector.Y == 1.0f)
	{
		CubeFace = ECFace::Right;
	}
	else if(NormalVector.Y == -1.0f)
	{
		CubeFace = ECFace::Left;
	}

	if(NormalVector.Z == 1.0f)
	{
		CubeFace = ECFace::Top;
	}
	else if(NormalVector.Z == -1.0f)
	{
		CubeFace = ECFace::Bottom;
	}
}

void ARubikCube::AddPiecesToRotate(ECFace CubeFaceCheck, FVector PieceHittedLocation, FVector Direction)
{
	
	TArray<USceneComponent *> PiecesDelta;
	RotatingRoot->GetChildrenComponents(false, PiecesDelta);
	for (ARubikPiece * RubikPiecesDelta : PiecesToRotate)
	{
		RubikPiecesDelta->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		RubikPiecesDelta->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	}

	PiecesToRotate.Empty();
	PiecesDelta.Empty();
	RotatingRoot->SetRelativeRotation(FRotator::ZeroRotator);
	
	if (CubeFaceCheck == ECFace::Front)
	{		
		if (Direction.Y > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,90,0), PieceHittedLocation);			
		}
		else if (Direction.Y < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,-90,0), PieceHittedLocation);
		}
		else if (Direction.Z > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(90,0,0), PieceHittedLocation);
		}
		else if (Direction.Z < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(-90,0,0), PieceHittedLocation);
		}
	}

	if (CubeFaceCheck == ECFace::Back)
	{
		
		if (Direction.Y < -0.5f){
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,90,0), PieceHittedLocation);
		}
		else if (Direction.Y > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,-90,0), PieceHittedLocation);
		}
		else if (Direction.Z < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(90,0,0), PieceHittedLocation);
		}
		else if (Direction.Z > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(-90,0,0), PieceHittedLocation);
		}
	}
	
	if (CubeFaceCheck == ECFace::Right)
	{		
		if (Direction.X > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,90,0), PieceHittedLocation);
		}
		else if (Direction.X < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,-90,0), PieceHittedLocation);
		}
		else if (Direction.Z > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,90), PieceHittedLocation);
		}
		else if (Direction.Z < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,-90), PieceHittedLocation);
		}
	}
	
	if (CubeFaceCheck == ECFace::Left)
	{
		
		if (Direction.X < -0.5f)
		{		
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,90,0), PieceHittedLocation);
		}
		else if (Direction.X > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupZ, FRotator(0,-90,0), PieceHittedLocation);
		}
		else if (Direction.Z < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,90), PieceHittedLocation);
		}
		else if (Direction.Z > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,-90), PieceHittedLocation);
		}
	}
	
	if (CubeFaceCheck == ECFace::Top)
	{
		if (Direction.Y > 0.5f)
		{			
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,-90), PieceHittedLocation);
		}
		else if (Direction.Y < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,90), PieceHittedLocation);
		}
		else if (Direction.X > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(90,0,0), PieceHittedLocation);
		}
		else if (Direction.X < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(-90,0,0), PieceHittedLocation);
		}
	}

	if (CubeFaceCheck == ECFace::Bottom)
	{
		
		if (Direction.Y < -0.5f){
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,-90), PieceHittedLocation);
		}
		else if (Direction.Y > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupX, FRotator(0,0,90), PieceHittedLocation);
		}
		else if (Direction.X < -0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(90,0,0), PieceHittedLocation);
		}
		else if (Direction.X > 0.5f)
		{
			PiecesGroup(EPiecesGroup::GroupY, FRotator(-90,0,0), PieceHittedLocation);
		}
	}
	
	CubeState = ECubeState::Idle;
}

void ARubikCube::PiecesGroup(EPiecesGroup PiecesGroup, FRotator EndRotationDelta, FVector PieceHittedLocation)
{
	if(PiecesGroup == EPiecesGroup::GroupZ)
	{	
		for (ARubikPiece * RubikPiece : Pieces)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s %s"), *RubikPiece->GetActorLocation().ToString(), *PieceHittedLocation.ToString());
			if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
			{
				PiecesToRotate.Add(RubikPiece);
				RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);				
			}
		}
	}

	if(PiecesGroup == EPiecesGroup::GroupY)
	{
		for (ARubikPiece * RubikPiece : Pieces)
		{
			if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
			{
				PiecesToRotate.Add(RubikPiece);
				RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}
	

	if(PiecesGroup == EPiecesGroup::GroupX)
	{
		for (ARubikPiece * RubikPiece : Pieces)
		{
			if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
			{
				PiecesToRotate.Add(RubikPiece);
				RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);				
			}
		}
	}
	//TODO Rotate Animation Start, use a flag for it
	//RotatingRoot->SetRelativeRotation(FRotator(0,90,0));

	//Start Animation

	StartRotation = RotatingRoot->RelativeRotation;

	EndRotation = EndRotationDelta;
	MyTimeLine->PlayFromStart();
}

void ARubikCube::TimelineFloatReturn(float value)
{	
	CubeState = ECubeState::Animating;
	RotatingRoot->SetRelativeRotation(FMath::Lerp(StartRotation, EndRotation, value));
}

void ARubikCube::OnTimelineFinished()
{
	CubeState = ECubeState::Idle;
}

