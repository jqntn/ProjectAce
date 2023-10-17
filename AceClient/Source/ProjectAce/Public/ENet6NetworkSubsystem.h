#pragma once

#include "../../../../AceServer/src/include/Protocol.h"
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <enet6/enet.h>

#include "ENet6NetworkSubsystem.generated.h"

struct PredictedInput
{
  PlayerInput Input;
};

struct GameData
{
  ENetPeer* ServerPeer = nullptr;

  std::vector<Player> Players;
  size_t OwnPlayerIndex = 0;

  PlayerInput Input;
  uint32_t InputIndex = 1;
  std::vector<PredictedInput> PredictedInputs;

  std::vector<PlayersPositionPacket> InterpolationBuffer;
  float InterpolationTime = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerJoined, int, PlayerIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerLeft, int, PlayerIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerPositionReceived,
                                             int,
                                             PlayerIndex,
                                             FVector,
                                             PlayerPosition);

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
    float _accel{ 30.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _minAccel{ 30.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _maxAccel{ 400.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _maxSpeed{ 4000.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _minSpeed{ 500.f };

    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _pitchRateMult{ 200.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _rollRateMult{ 200.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _yawRate{ 200.f };

    UPROPERTY(VisibleAnywhere, Category = "FlightPhysics")
    float _startForwardSpeed{ 500.f };
    //--------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "FlightPhysics")
    float _currForwardSpeed;

    float _currYawSpeed;
    float _currPitchSpeed;
    float _currRollSpeed;

    float _currYawValue;
    float _currPitchValue;
    float _currRollValue;

    bool _bIntentionalPitch{ false };
    bool _bIntentionalRoll{ false };
  };
  PlaneData _planeData;

  UFUNCTION(BlueprintCallable)
  void InitPlaneData(float acceleration,
                     float minAcceleration,
                     float maxAcceleration,
                     float maxSpeed,
                     float minSpeed,
                     float pitchRateMultiplier,
                     float rollRateMultiplier,
                     float yawRate,
                     float startForwardSpeed);

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

  void HandleMessage(const std::vector<uint8_t>& message);

  UPROPERTY(BlueprintAssignable)
  FPlayerJoined PlayerJoined;

  UPROPERTY(BlueprintAssignable)
  FPlayerLeft PlayerLeft;

  UPROPERTY(BlueprintAssignable)
  FPlayerPositionReceived PlayerPositionReceived;

private:
  ENetHost* Host = nullptr;
  ENetPeer* ServerPeer = nullptr;
  APawn* _planePawn;
  GameData gameData;

  void ProcessRoll();
  void ProcessPitch();
  void ProcessYaw();
};
