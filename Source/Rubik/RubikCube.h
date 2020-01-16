// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RubikCube.generated.h"


class USpringArmComponent;
class UCameraComponent;
class ARubikPiece;

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

	UPROPERTY()
	TArray<ARubikPiece *> Pieces;

	UPROPERTY()
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
	void CameraRotateX(float Value);

	UFUNCTION()
	void CameraRotateY(float Value);

	UFUNCTION()
	void ZoomOut();

	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void BackToIdle();

	UFUNCTION()
	void AddPiecesToRotate(ECFace CubeFaceChheck, FVector PieceHittedLocation, FVector FaceNormal);

	UFUNCTION()
	void ClickedFace(FVector NormalVector);
};
