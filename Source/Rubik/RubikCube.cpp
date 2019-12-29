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
}

// Called to bind functionality to input
void ARubikCube::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Binding Actions
	PlayerInputComponent->BindAction("EnableCameraRotation", EInputEvent::IE_Pressed, this, &ARubikCube::ToggleCameraRotation);
	PlayerInputComponent->BindAction("EnableCameraRotation", EInputEvent::IE_Released, this, &ARubikCube::ToggleCameraRotation);
	PlayerInputComponent->BindAction("ZoomOut", EInputEvent::IE_Pressed, this, &ARubikCube::ZoomOut);
	PlayerInputComponent->BindAction("ZoomIn", EInputEvent::IE_Pressed, this, &ARubikCube::ZoomIn);
	PlayerInputComponent->BindAction("CubeFaceRotation", EInputEvent::IE_Pressed, this, &ARubikCube::CubeFaceRotation);

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

void ARubikCube::ToggleCameraRotation()
{

	bIsCameraRotating = !bIsCameraRotating;
	if (CubeState == ECubeState::Idle)
	{
		CubeState = ECubeState::RotatingCamera;
	}
	else if (CubeState == ECubeState::RotatingCamera)	
	{
		CubeState = ECubeState::Idle;
	}
	//FString message = TEXT("Our enum value: ") + EnumToString(TEXT("ETeam"), static_cast<uint8>(Team));
	//UE_LOG(LogTemp, Warning, TEXT("PlayerState: %s");

}

void ARubikCube::CameraRotateX(float Value)
{
	// I don't use delta seconds cause the function is already called every frame
	if (bIsCameraRotating)
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
	// Same as Camera Rotate X
	if (bIsCameraRotating)
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

void ARubikCube::CubeFaceRotation()
{
	
	// Raycast and see if i hitted a cube
	if (PlayerController != nullptr)
	{
		FHitResult Hit;
		PlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, false, Hit);

		if(Hit.bBlockingHit)
		{
			// get the normal of the place i hitted
			UE_LOG(LogTemp, Warning, TEXT("Hitted something %s"), *Hit.Actor->GetName() );
			UE_LOG(LogTemp, Warning, TEXT("Normal: %s"), *Hit.ImpactNormal.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Location %s"), *Hit.Actor->GetActorLocation().ToString());
			AddPiecesToRotate(ECFace::Front, Hit.Actor->GetActorLocation(), Hit.ImpactNormal);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Hitted nothing") );
		}

	}
	
}
/* Appunti per rotazione: 
 - Con la normale cerchiamo di capire in quale faccia siamo
 - Meglio creare una funzione solo per girare e una che ci ritorna un array di pezzi da ruotare
 Oppure
 - Fare tutto insieme nella stessa funzione e dopo fare un refactoring del codice (soluzione migliore)
 - Analizzare se si sono inseriti troppi dati ridondanti

*/
void ARubikCube::AddPiecesToRotate(ECFace CubeFaceCheck, FVector PieceHittedLocation, FVector FaceNormal)
{
	PiecesToRotate.Empty();

	if (CubeFaceCheck == ECFace::Front)
	{
		for (ARubikPiece * RubikPiece : Pieces)
		{
			if (RubikPiece->GetActorLocation().Z == PieceHittedLocation.Z)
			{
				PiecesToRotate.Add(RubikPiece);
				RubikPiece->AttachToComponent(this->RotatingRoot, FAttachmentTransformRules::KeepWorldTransform);
			}
		}

		FRotator NewRotation = FRotator(RotationPiecesPitch, RotationPiecesYaw, RotationPiecesRoll);
		FQuat QuatRotation = FQuat(NewRotation);
		RotatingRoot->AddLocalRotation(QuatRotation,false, 0, ETeleportType::None);
		
		for (ARubikPiece * Attached : PiecesToRotate)
		{
			Attached->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		}

		//UE_LOG(LogTemp, Warning, TEXT("Hitted something %d"), PiecesToRotate.Num());
	}
}

