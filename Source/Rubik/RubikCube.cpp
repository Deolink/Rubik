// Fill out your copyright notice in the Description page of Project Settings.

#include "RubikCube.h"
#include "GameFramework/SpringArmComponent.h"
#include "Classes/Camera/CameraComponent.h"
#include "RubikPiece.h"
#include "Classes/Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"

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
}

// Called when the game starts or when spawneds
void ARubikCube::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = Cast<APlayerController>(GetController());
	PlayerController->bShowMouseCursor = true;
	SpawnPieces();	
}

// Called every frame
void ARubikCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Probably is better to put inside the rotation axis logic 
	if (bIsFaceRotating)
	{
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

	
	if (PlayerController != nullptr && CubeState == ECubeState::RotatingPieces)
	{
	FVector Direction;
	FVector CurrentLocation;
	FHitResult Hit;
	PlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, false, Hit);
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


	/*if ((ScreenPointClicked - MousePosition).GetSafeNormal() != FVector2D::ZeroVector)
	{
		UE_LOG(LogTemp, Display, TEXT("entered if"));
		switch (CubeState)
		{
		case ECubeState::Idle:
			
			break;
		
		default:
			break;
		}
	}
	*/
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
		FMath::Clamp(Value, -1.f, 1.f);
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
	FMath::Clamp(Value, -1.f, 1.f);
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

/*void ARubikCube::RotatePieces()
{
	
	RelativeSpeed = FMath::Clamp<float>(RelativeSpeed, -1.f, 1.f);
	float RotationChange = RelativeSpeed * MaxDegreesPerSecond * GetWorld()->DeltaTimeSeconds;
	float Rotation = YawRotation->RelativeRotation.Yaw + RotationChange;
	YawRotation->SetRelativeRotation(FRotator(0.f, Rotation, 0.f));

	if (YawRotation->RelativeRotation.Yaw > 45.f)
	{
		RelativeSpeed = -1;
	}
	else if (YawRotation->RelativeRotation.Yaw < -45.f)
	{
		RelativeSpeed = 1;
	}
}*/

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
			RotatingRoot->SetRelativeRotation(FRotator(0,90,0));
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

	CubeState = ECubeState::Idle;
	/*
	Back: 
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	/*
	Right:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	/*
	Left:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	/*
	Top:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	/*
	Bottom:
	Rotate right/left= same Z  Rotate up/down = same Y
	Z Value = Yaw -> SubtractRotation = Antiorario AddRotation = Orario
	X Value = Roll -> No Roll Rotation on front
	Y Value = Pitch -> SubtractRotation = Orario AddRotation = Antiorario
	*/
	
}

