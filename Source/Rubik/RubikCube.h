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

	UFUNCTION()
	void SpawnPieces();



};
