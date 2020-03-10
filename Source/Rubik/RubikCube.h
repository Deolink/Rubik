// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "RubikCube.generated.h"


class USpringArmComponent;
class UCameraComponent;
class ARubikPiece;
class UTimelineComponent;
class UCurveFloat;

UENUM(BlueprintType)
enum class ECFace : uint8
{
	Front 		UMETA(DisplayName="Front"),
	Back  		UMETA(DisplayName="Back"),
	Right 		UMETA(DisplayName="Right"),
	Left  		UMETA(DisplayName="Left"),
	Top 		UMETA(DisplayName="Top"),
	Bottom 		UMETA(DisplayName="Bottom"),
	NotSelected UMETA(DisplayName="NotSelected")
};

UENUM(BlueprintType)
enum class ECubeState : uint8
{
	RotatingCamera UMETA(DisplayName="RotatingCamera"),
	RotatingPieces UMETA(DisplayName="RotatingPieces"),
	Animating      UMETA(DisplayName="Animation"),
	Pause 		   UMETA(DisplayName="Pause"),
	Idle 		   UMETA(DisplayName="Idle"),
	Scramble       UMETA(DisplayName="Scramble")
};



UCLASS()
class RUBIK_API ARubikCube : public APawn
{
	GENERATED_BODY()

	

public:
	// Sets default values for this pawn's properties
	ARubikCube();
	
	//Public Variables
	UPROPERTY(Category = Rubiks, EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ARubikPiece> PieceClass;

	UPROPERTY(Category = Rubiks, EditAnywhere, BlueprintReadWrite)
	TArray<ARubikPiece *> Pieces;

	UPROPERTY(Category = Rubiks, EditAnywhere, BlueprintReadWrite)
	TArray<ARubikPiece *> PiecesToRotate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	float AngleRotation = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Enum)
	ECubeState CubeState = ECubeState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Enum)
	ECFace CubeFace = ECFace::NotSelected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	float RotationPiecesPitch = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	float RotationPiecesYaw = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	float RotationPiecesRoll = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	FVector2D ScreenPointClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	FVector2D MousePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	FVector MousePosition3D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	FVector DirectionCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	FVector StartClickLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Debug)
	FVector TouchPosition;

	//Timeline setup
	UTimelineComponent* MyTimeLine;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* fCurve;

	UPROPERTY()
	FRotator StartRotation;

	UPROPERTY()
	FRotator EndRotation;

	UPROPERTY()
	float OffsetRotation = 90.f;

	/* Declare our delegate function to be binded with TimelineFloatReturn(float value)*/
	FOnTimelineFloat InterpFunction{};

	/* Declare our delegate function to be binded with OnTimelineFinished()*/
	FOnTimelineEvent TimelineFinished{};

	UFUNCTION()
	void TimelineFloatReturn(float value);

	UFUNCTION()
	void OnTimelineFinished();

	



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	//Declaring my components
	UPROPERTY(EditAnywhere)
	USceneComponent* Root;

	UPROPERTY(EditAnywhere)
	USceneComponent* RotatingRoot;

	UPROPERTY(EditAnywhere)
	USpringArmComponent* CameraSpringArm;

	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	bool bIsTouchingPressed = false;

	UPROPERTY(EditAnywhere)
	bool bIsCameraRotating = false;

	UPROPERTY(EditAnywhere)
	bool bIsFaceRotating = false;

	UPROPERTY(EditAnywhere)
	float PitchCameraValue;

	UPROPERTY(EditAnywhere)
	float YawCameraValue;

	UPROPERTY(EditAnywhere)
	float ZoomValue = 10;

	UPROPERTY(EditAnywhere)
	float SpringArmLenght = 600;

	UPROPERTY(EditAnywhere)
	APlayerController* PlayerController;

	//Declaring the functions
	UFUNCTION()
	void ControlHit();

	UFUNCTION()
	void SpawnPieces();

	UFUNCTION()
	void CameraRotateX(float Value = 1.0f);

	UFUNCTION()
	void CameraRotateY(float Value = 1.0f);

	UFUNCTION()
	void ZoomOut();

	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void BackToIdle();

	UFUNCTION()
	void AddPiecesToRotate(ECFace CubeFaceChheck, FVector PieceHittedLocation, FVector Direction);

	UFUNCTION()
	void ClickedFace(FVector NormalVector);

	/*UFUNCTION()
	void RotatePieces();*/
};
