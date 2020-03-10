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

		//Setting tutorial vectors
		StartRotation = RotatingRoot->RelativeRotation;

		//EndRotation = FRotator(StartRotation.Pitch, StartRotation.Yaw, StartRotation.Roll + OffsetRotation);
		MyTimeLine->SetLooping(false);
		MyTimeLine->SetIgnoreTimeDilation(true);
		//MyTimeLine->Play();
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
	//


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


	/*GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
	FString::Printf(TEXT("Is Touching Pressed? : %s"), (bIsTouchingPressed ? TEXT("True") : TEXT("False"))));*/

	// Probably is better to put inside the rotation axis logic 
	if (bIsFaceRotating && CubeState != ECubeState::Animating)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
		FString::Printf(TEXT("Rotating Pieces")));
	
	//Rotate RotatingRoot based if i'm moving right/left top/right
	/* Appunti per rotazione:  
 
 	- Migliorare gli stati del gioco e farli funzionanti
 	- Con la normale cerchiamo di capire in quale faccia siamo in modo da capire quali pezzi prendere
 	- Capire se il movimento quando premiamo è in su o in giù,  probabile da aggiungere alla funzione di quando premiamo
 	- Meglio creare una funzione solo per girare e una che ci ritorna un array di pezzi da ruotare
 	Oppure
 	- Fare tutto insieme nella stessa funzione e dopo fare un refactoring del codice (soluzione migliore)
 	- Analizzare se si sono inseriti troppi dati ridondanti
	*/
	/*
	------ First make it work for the front face, then we make the same logic for the others faces -------------
	Front: 
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	//PlayerController->GetMousePosition(MousePosition.X, MousePosition.Y);
	//PlayerController->DeprojectMousePositionToWorld(MousePosition3D, MouseDirection);
	//PlayerController->GetInputMouseDelta(MousePosition.X, MousePosition.Y);
	PlayerController->GetInputTouchState(ETouchIndex::Touch1, TouchPosition.X, TouchPosition.Y, bIsTouchingPressed);
	//UE_LOG(LogTemp, Warning, TEXT("Is Touching Pressed? : %s"), (bIsTouchingPressed ? TEXT("True") : TEXT("False")));

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
					//UE_LOG(LogTemp, Warning, TEXT("Normalized Direction %s"), *NormalizedDirection.ToString());
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
					//Piece->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
					
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
			//UE_LOG(LogTemp, Warning, TEXT("Hitted something %s"), *Hit.Actor->GetName() );
			//UE_LOG(LogTemp, Warning, TEXT("Normal: %s"), *Hit.ImpactNormal.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Location %s"), *Hit.Actor->GetActorLocation().ToString());
			//UE_LOG(LogTemp, Warning, TEXT("Location %s"), *Hit.Location.ToString());
			//PlayerController->ProjectWorldLocationToScreen(Hit.Location, ScreenPointClicked, true);
			//UE_LOG(LogTemp, Warning, TEXT("Location %s"), *ScreenPointClicked.ToString());
			

			StartClickLocation = Hit.Location;
			
			ClickedFace(Hit.ImpactNormal);
			//AddPiecesToRotate(CubeFace, Hit.Actor->GetActorLocation(), Hit.ImpactNormal); // spostare in update
		}
		else
		{

			bIsCameraRotating = true;
			PlayerController->GetInputTouchState(ETouchIndex::Touch1, StartClickLocation.X, StartClickLocation.Y, bIsTouchingPressed);
			CubeState = ECubeState::RotatingCamera;
			UE_LOG(LogTemp, Warning, TEXT("Hitted nothing") );
			
		}

	}

	
	
	//UE_LOG(LogTemp, Warning, TEXT("PlayerState: %s");

}

void ARubikCube::CameraRotateX(float Value)
{	
	if (bIsCameraRotating && CubeState == ECubeState::RotatingCamera)
	{
		//FMath::Clamp(Value, -1.f, 1.f);
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
		RubikPiecesDelta->DetachRootComponentFromParent(true);
		RubikPiecesDelta->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	}
	PiecesToRotate.Empty();
	PiecesDelta.Empty();
	RotatingRoot->SetRelativeRotation(FRotator::ZeroRotator);
	/*
	------ First make it work for the front face, then we make the same logic for the others faces -------------
	Front: 
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	if (CubeFaceCheck == ECFace::Front)
	{
		// y = 1 90°z ;;;; y = -1 -90°y
		// z = 1 90°y ;;;; z = -1 -90°z
		/*for (ARubikPiece * RubikPiece : Pieces)
		{
			if (RubikPiece->GetActorLocation().Z == PieceHittedLocation.Z)
			{
				PiecesToRotate.Add(RubikPiece);
				RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
				// rotate RotatingRoot
				
			}
		}*/
		if (Direction.Y > 0.5f){
			for (ARubikPiece * RubikPiece : Pieces)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s %s"), *RubikPiece->GetActorLocation().ToString(), *PieceHittedLocation.ToString());
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{

					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			//TODO Rotate Animation Start, use a flag for it
			//RotatingRoot->SetRelativeRotation(FRotator(0,90,0));

			//Start Animation

			StartRotation = RotatingRoot->RelativeRotation;

			EndRotation = FRotator(StartRotation.Pitch, StartRotation.Yaw + OffsetRotation, StartRotation.Roll);
			MyTimeLine->PlayFromStart();
		}
		else if (Direction.Y < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,-90,0));
		}
		else if (Direction.Z > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(90,0,0));
		}
		else if (Direction.Z < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(-90,0,0));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Hitted something %d"), PiecesToRotate.Num());

	}

	if (CubeFaceCheck == ECFace::Back)
	{
		// y = 1 90°z ;;;; y = -1 -90°y
		// z = 1 90°y ;;;; z = -1 -90°z
		/*for (ARubikPiece * RubikPiece : Pieces)
		{
			if (RubikPiece->GetActorLocation().Z == PieceHittedLocation.Z)
			{
				PiecesToRotate.Add(RubikPiece);
				RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
				// rotate RotatingRoot
				
			}
		}*/
		if (Direction.Y < -0.5f){
			for (ARubikPiece * RubikPiece : Pieces)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s %s"), *RubikPiece->GetActorLocation().ToString(), *PieceHittedLocation.ToString());
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{

					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			//TODO Rotate Animation Start, use a flag for it
			RotatingRoot->SetRelativeRotation(FRotator(0,90,0));
		}
		else if (Direction.Y > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,-90,0));
		}
		else if (Direction.Z < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(90,0,0));
		}
		else if (Direction.Z > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(-90,0,0));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Hitted something %d"), PiecesToRotate.Num());

	}

	/*
	Right:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	if (CubeFaceCheck == ECFace::Right)
	{
		// y = 1 90°z ;;;; y = -1 -90°y
		// z = 1 90°y ;;;; z = -1 -90°z
		if (Direction.X > 0.5f){
			for (ARubikPiece * RubikPiece : Pieces)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s %s"), *RubikPiece->GetActorLocation().ToString(), *PieceHittedLocation.ToString());
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{

					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			//TODO Rotate Animation Start, use a flag for it
			RotatingRoot->SetRelativeRotation(FRotator(0,90,0));
		}
		else if (Direction.X < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,-90,0));
		}
		else if (Direction.Z > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,0,90));
		}
		else if (Direction.Z < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,0,-90));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Hitted something %d"), PiecesToRotate.Num());

	}
	/*
	Left:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	if (CubeFaceCheck == ECFace::Left)
	{
		// y = 1 90°z ;;;; y = -1 -90°y
		// z = 1 90°y ;;;; z = -1 -90°z
		if (Direction.X < -0.5f){
			for (ARubikPiece * RubikPiece : Pieces)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s %s"), *RubikPiece->GetActorLocation().ToString(), *PieceHittedLocation.ToString());
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{

					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			//TODO Rotate Animation Start, use a flag for it
			RotatingRoot->SetRelativeRotation(FRotator(0,90,0));
		}
		else if (Direction.X > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Z  - PieceHittedLocation.Z)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,-90,0));
		}
		else if (Direction.Z < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,0,90));
		}
		else if (Direction.Z > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,0,-90));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Hitted something %d"), PiecesToRotate.Num());

	}

	/*
	Top:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	if (CubeFaceCheck == ECFace::Top)
	{
		// y = 1 90°z ;;;; y = -1 -90°y
		// z = 1 90°y ;;;; z = -1 -90°z
		if (Direction.Y > 0.5f){
			for (ARubikPiece * RubikPiece : Pieces)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s %s"), *RubikPiece->GetActorLocation().ToString(), *PieceHittedLocation.ToString());
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{

					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			//TODO Rotate Animation Start, use a flag for it
			RotatingRoot->SetRelativeRotation(FRotator(0,0,90));
		}
		else if (Direction.Y < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,0,-90));
		}
		else if (Direction.X > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(90,0,0));
		}
		else if (Direction.X < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(-90,0,0));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Hitted something %d"), PiecesToRotate.Num());

	}

	/*
	Bottom:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	if (CubeFaceCheck == ECFace::Bottom)
	{
		// y = 1 90°z ;;;; y = -1 -90°y
		// z = 1 90°y ;;;; z = -1 -90°z
		if (Direction.Y < -0.5f){
			for (ARubikPiece * RubikPiece : Pieces)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s %s"), *RubikPiece->GetActorLocation().ToString(), *PieceHittedLocation.ToString());
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{

					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			//TODO Rotate Animation Start, use a flag for it
			RotatingRoot->SetRelativeRotation(FRotator(0,0,90));
		}
		else if (Direction.Y > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().X  - PieceHittedLocation.X)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(0,0,-90));
		}
		else if (Direction.X < -0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(90,0,0));
		}
		else if (Direction.X > 0.5f)
		{
			for (ARubikPiece * RubikPiece : Pieces)
			{
				if ((FMath::Abs(RubikPiece->GetActorLocation().Y  - PieceHittedLocation.Y)) < 0.2f)
				{
					PiecesToRotate.Add(RubikPiece);
					RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
					// rotate RotatingRoot
				
				}
			}
			RotatingRoot->SetRelativeRotation(FRotator(-90,0,0));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Hitted something %d"), PiecesToRotate.Num());

	}
	
	CubeState = ECubeState::Idle;
	
	
}

void ARubikCube::TimelineFloatReturn(float value)
{	
	CubeState = ECubeState::Animating;
	RotatingRoot->SetRelativeRotation(FMath::Lerp(StartRotation, EndRotation, value));	
	UE_LOG(LogTemp, Warning, TEXT("%f"), value);
}

void ARubikCube::OnTimelineFinished()
{
	CubeState = ECubeState::Idle;
	//RotatingRoot->SetRelativeRotation(FRotator::ZeroRotator);
	//UE_LOG(LogTemp, Warning, TEXT("Finished"));
}

