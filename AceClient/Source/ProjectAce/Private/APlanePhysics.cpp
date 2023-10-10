#include "APlanePhysics.h"
// Fill out your copyright notice in the Description page of Project Settings.


#include "APlanePhysics.h"

// Sets default values
AAPlanePhysics::AAPlanePhysics()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AAPlanePhysics::ProcessKeyPitch(float rate)
{
	if (FMath::Abs(rate) > .2f)
		ProcessPitch(rate * 2.f);
}

void AAPlanePhysics::ProcessKeyRoll(float rate)
{
	if (FMath::Abs(rate) > .2f)
		ProcessRoll(rate * 2.f);
}

void AAPlanePhysics::ProcessMouseYInput(float value)
{
	ProcessPitch(value);
}

void AAPlanePhysics::ProcessMouseXInput(float value)
{
	ProcessRoll(value);
}

void AAPlanePhysics::ProcessRoll(float value)
{
	const float targetRollSpeed = value * _rollRateMult;
	_currRollSpeed = FMath::FInterpTo(_currRollSpeed, targetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void AAPlanePhysics::ProcessPitch(float value)
{
	const float targetPitchSpeed = value * _pitchRateMult;
	_currPitchSpeed = FMath::FInterpTo(_currPitchSpeed, targetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

// Called when the game starts or when spawned
void AAPlanePhysics::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAPlanePhysics::Tick(float DeltaTime)
{
	//Compute Thrust
	const float currAccel = -GetActorRotation().Pitch * DeltaTime * _accel; //Tilt up > slow down, Tilt down > accelerate
	const float newForwardSpeed = _currForwardSpeed + currAccel;
	_currForwardSpeed = FMath::Clamp(newForwardSpeed, _minSpeed, _maxSpeed);

	const FVector localMove = FVector(_currForwardSpeed * DeltaTime, 0.f, 0.f);
	AddActorLocalOffset(localMove, true);

	GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Green, FString::Printf(TEXT("ForwardSpeed: %f"), _currForwardSpeed));


	FRotator deltaRotation(0, 0, 0);
	deltaRotation.Pitch		= _currPitchSpeed * DeltaTime;
	deltaRotation.Yaw		= _currYawSpeed * DeltaTime;
	deltaRotation.Roll		= _currRollSpeed * DeltaTime;

	AddActorLocalRotation(deltaRotation);

	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAPlanePhysics::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Turn",			this, &AAPlanePhysics::ProcessMouseXInput	);
	PlayerInputComponent->BindAxis("TurnRate",		this, &AAPlanePhysics::ProcessKeyRoll		);
	PlayerInputComponent->BindAxis("LookUp",		this, &AAPlanePhysics::ProcessMouseYInput	);
	PlayerInputComponent->BindAxis("LookUpRate",	this, &AAPlanePhysics::ProcessKeyPitch		);

}

