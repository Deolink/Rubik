// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RubikCube.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ARubikPiece;

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
	float PitchCameraValue;

	UPROPERTY(EditAnywhere)
	float YawCameraValue;

	UPROPERTY(EditAnywhere)
	float ZoomValue = 10;

	UPROPERTY(EditAnywhere)
	float SpringArmLenght = 600;

	UPROPERTY(EditAnywhere)
	APlayerController* PlayerController;

	UFUNCTION()
	void ToggleCameraRotation();

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
	void CubeFaceRotation();

};
