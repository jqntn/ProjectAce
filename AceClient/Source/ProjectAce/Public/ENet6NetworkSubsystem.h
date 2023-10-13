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

struct Player
{
  size_t Index = 0;
  std::string Name;

  Vector3 Position;
  Vector3 Velocity;
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
    float _maxSpeed{ 4000.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _minSpeed{ 500.f };

    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _pitchRateMult{ 200.f };
    UPROPERTY(EditAnywhere, Category = "FlightPhysics")
    float _rollRateMult{ 200.f };

    UPROPERTY(VisibleAnywhere, Category = "FlightPhysics")
    float _startForwardSpeed{ 500.f };
    //--------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "FlightPhysics")
    float _currForwardSpeed;

    float _currYawSpeed;
    float _currPitchSpeed;
    float _currRollSpeed;

    bool _bIntentionalPitch{ false };
    bool _bIntentionalRoll{ false };
  };
  PlaneData _planeData;

  UFUNCTION(BlueprintCallable)
  void InitPlaneData(float acceleration,
                     float maxSpeed,
                     float minSpeed,
                     float pitchRateMultiplier,
                     float rollRateMultiplier,
                     float startForwardSpeed);

  UFUNCTION(BlueprintCallable)
  void ProcessKeyPitch(float rate);
  UFUNCTION(BlueprintCallable)
  void ProcessKeyRoll(float rate);

  UFUNCTION(BlueprintCallable)
  void ProcessMouseYInput(float value);
  UFUNCTION(BlueprintCallable)
  void ProcessMouseXInput(float value);

  UFUNCTION(BlueprintCallable)
  void SetIntentionalRoll(float value);
  UFUNCTION(BlueprintCallable)
  void SetIntentionalPitch(float value);

  // Compute rotation
  void ProcessRoll(float value);
  void ProcessPitch(float value);

  UFUNCTION(BlueprintCallable)
  void SetPlanePawn(APawn* pawn);

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

  void ComputePhysics(Player& player,
                      const PlayerInput& input,
                      float elapsedTime);

  void HandleMessage(const std::vector<uint8_t>& message);

private:
  ENetHost* Host = nullptr;
  ENetPeer* ServerPeer = nullptr;

  APawn* _planePawn = nullptr;

  GameData gameData;
};