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
};
