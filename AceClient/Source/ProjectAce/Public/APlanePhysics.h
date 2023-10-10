// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "APlanePhysics.generated.h"

UCLASS()
class PROJECTACE_API AAPlanePhysics : public ACharacter
{
	GENERATED_BODY()

	
public:
	// Sets default values for this character's properties
	AAPlanePhysics();

	UPROPERTY(EditAnywhere, Category="FlightPhysics")
	float _accel				{ 30.f };
	UPROPERTY(EditAnywhere, Category="FlightPhysics")
	float _maxSpeed				{ 4000.f };
	UPROPERTY(EditAnywhere, Category="FlightPhysics")
	float _minSpeed				{ 500.f };

	UPROPERTY(EditAnywhere, Category="FlightPhysics")
	float _pitchRateMult		{ 200.f };
	UPROPERTY(EditAnywhere, Category="FlightPhysics")
	float _rollRateMult			{ 200.f };

	UPROPERTY(VisibleAnywhere, Category="FlightPhysics")
	float _currForwardSpeed		{ 500.f };

	float _currYawSpeed;
	float _currPitchSpeed;
	float _currRollSpeed;


protected:

	void ProcessKeyPitch		(float rate);
	void ProcessKeyRoll			(float rate);

	void ProcessMouseYInput		(float value);
	void ProcessMouseXInput		(float value);

	//Compute rotation
	void ProcessRoll			(float value);
	void ProcessPitch			(float value);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
