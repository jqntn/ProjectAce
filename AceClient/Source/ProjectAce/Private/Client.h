#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Client.generated.h"

UCLASS()
class UClient
  : public UGameInstanceSubsystem
  , public FTickableObjectBase
{
  GENERATED_BODY()

  void Initialize(FSubsystemCollectionBase& Collection) override;
  void Deinitialize() override;

  void Tick(float DeltaTime) override;
  TStatId GetStatId() const override;

  UFUNCTION(BlueprintCallable)
  bool Connect(FString Address);
  UFUNCTION(BlueprintCallable)
  void Disconnect();
};