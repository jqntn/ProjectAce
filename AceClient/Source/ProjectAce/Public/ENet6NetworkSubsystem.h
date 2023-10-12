#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <enet6/enet.h>

#include "ENet6NetworkSubsystem.generated.h"

UCLASS()
class PROJECTACE_API UENet6NetworkSubsystem
  : public UGameInstanceSubsystem
  , public FTickableGameObject
{
  GENERATED_BODY()

public:
#pragma region _Plane_

  struct PlaneData
  {
    //-------------Constants----------------
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _accel                    { 30.f };
     UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _minAccel                 { 30.f };
     UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _maxAccel                 { 400.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _maxSpeed                 { 4000.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _minSpeed                 { 500.f };

    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _pitchRateMult            { 200.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _rollRateMult             { 200.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _yawRate                  { 200.f };

    UPROPERTY(VisibleAnywhere, Category = "FlightPhysics")
    float _startForwardSpeed        { 500.f };
    //--------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "FlightPhysics")
    float _currForwardSpeed;

    float _currYawSpeed;
    float _currPitchSpeed;
    float _currRollSpeed;

    float _currYawValue;
    float _currPitchValue;
    float _currRollValue;

    bool _bIntentionalPitch         { false };
    bool _bIntentionalRoll          { false };
  };
  PlaneData _planeData;

  UFUNCTION(BlueprintCallable)
  void InitPlaneData(float acceleration, float minAcceleration, float maxAcceleration, float maxSpeed, float minSpeed, float pitchRateMultiplier, float rollRateMultiplier, float yawRate, float startForwardSpeed);


  UFUNCTION(BlueprintCallable)
  void SetlRollInput(float value);
  UFUNCTION(BlueprintCallable)
  void SetPitchInput(float value);
  UFUNCTION(BlueprintCallable)
  void SetYawInput(float value);

  UFUNCTION(BlueprintCallable)
  void SetThrust(float thrustAdd);

  UFUNCTION(BlueprintCallable)
  void SetPlanePawn(APawn* pawn);

  UFUNCTION(BlueprintCallable)
  float GetCurrentPlaneAccel();

  UFUNCTION(BlueprintCallable)
  APawn* GetPlanePawn();
#pragma endregion

  ETickableTickType GetTickableTickType() const override;
  TStatId GetStatId() const override;
  bool IsAllowedToTick() const override final;
  void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable)
  bool Connect(FString AddressString);
  UFUNCTION(BlueprintCallable)
  void Disconnect();

  void Initialize(FSubsystemCollectionBase& Collection) override;
  void Deinitialize() override;

private:
  ENetHost* Host = nullptr;
  ENetPeer* ServerPeer = nullptr;
  APawn* _planePawn;

  void ProcessRoll();
  void ProcessPitch();
  void ProcessYaw();

};
