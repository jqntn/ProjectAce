#include "ENet6NetworkSubsystem.h"

#include "../../../../AceServer/src/include/Protocol.h"

DEFINE_LOG_CATEGORY_STATIC(ENet6, Log, All);

#pragma region _Plane_

void UENet6NetworkSubsystem::InitPlaneData(float acceleration, float minAcceleration, float maxAcceleration, float maxSpeed, float minSpeed, float pitchRateMultiplier, float rollRateMultiplier, float yawRate, float startForwardSpeed)
{
    _planeData._accel               = acceleration;
    _planeData._minAccel            = minAcceleration;
    _planeData._maxAccel            = maxAcceleration;
    _planeData._minSpeed            = minSpeed;
    _planeData._pitchRateMult       = pitchRateMultiplier;
    _planeData._rollRateMult        = rollRateMultiplier;
    _planeData._yawRate             = yawRate;
    _planeData._startForwardSpeed   = startForwardSpeed;
}

void
UENet6NetworkSubsystem::ProcessRoll()
{

    if ((_planePawn->GetActorRotation().Roll <= 0.1f && _planePawn->GetActorRotation().Roll >= -0.1f) && _planeData._currRollValue == 0.f) return;
    _planeData._bIntentionalRoll = FMath::Abs(_planeData._currRollValue) > 0.f;

    float targetRollSpeed = 0.f;
    if (_planeData._bIntentionalRoll)
    {
        targetRollSpeed = (_planeData._currRollValue * _planeData._rollRateMult);
    }
    else
    {
        targetRollSpeed = (_planePawn->GetActorRotation().Roll * -2.f);
    }

    _planeData._currRollSpeed = FMath::FInterpTo(_planeData._currRollSpeed,
        targetRollSpeed,
        GetWorld()->GetDeltaSeconds(),
        2.f);
}

void UENet6NetworkSubsystem::ProcessPitch()
{
    const float targetPitchSpeed = _planeData._currPitchValue * _planeData._pitchRateMult;
    _planeData._currPitchSpeed = FMath::FInterpTo(_planeData._currPitchSpeed,
        targetPitchSpeed,
        GetWorld()->GetDeltaSeconds(),
        2.f);
}

void UENet6NetworkSubsystem::ProcessYaw()
{
    if (_planeData._currYawValue == 0.f) _planeData._currYawSpeed = 0.f;
    else 
    {
        const float targetYawSpeed = _planeData._currYawValue * _planeData._pitchRateMult;
        _planeData._currYawSpeed = targetYawSpeed;
    }
}

void UENet6NetworkSubsystem::SetlRollInput(float value)
{
    _planeData._currRollValue = value;
}

void UENet6NetworkSubsystem::SetPitchInput(float value)
{
    _planeData._currPitchValue = value;
}

void UENet6NetworkSubsystem::SetYawInput(float value)
{
    _planeData._currYawValue = value;
}

void UENet6NetworkSubsystem::SetThrust(float thrustAdd)
{
    const float newThrust = _planeData._accel + (thrustAdd * (GetWorld()->GetDeltaSeconds() * 5000.f));
    _planeData._accel = newThrust;//FMath::Clamp(_planeData._accel, _planeData._minAccel, _planeData._maxAccel);
}

void
UENet6NetworkSubsystem::SetPlanePawn(APawn* pawn)
{
  _planePawn = pawn;

  if (!_planePawn) return;
  _planeData._currForwardSpeed = _planeData._startForwardSpeed;
  _planePawn->bUseControllerRotationPitch = false;
  _planePawn->bUseControllerRotationRoll = false;
  _planePawn->bUseControllerRotationYaw = false;
}

float UENet6NetworkSubsystem::GetCurrentPlaneAccel()
{
    return _planeData._accel;
}

APawn*
UENet6NetworkSubsystem::GetPlanePawn()
{
  return _planePawn;
}

#pragma endregion

ETickableTickType
UENet6NetworkSubsystem::GetTickableTickType() const
{
  return ETickableTickType::Always;
}

TStatId
UENet6NetworkSubsystem::GetStatId() const
{
  return TStatId();
}

bool
UENet6NetworkSubsystem::IsAllowedToTick() const
{
  return true;
}

void
UENet6NetworkSubsystem::Tick(float DeltaTime)
{
  if (Host == nullptr)
    return;

  // Plane physics
  if (_planePawn) {
    // Compute Thrust
    const float currAccel =
      -_planePawn->GetActorRotation().Pitch * DeltaTime/* * _planeData._accel*/; // Tilt up > slow down, Tilt down > accelerate
    const float newForwardSpeed = (_planeData._currForwardSpeed + currAccel) * _planeData._accel;
    _planeData._currForwardSpeed =
      FMath::Clamp(newForwardSpeed, _planeData._minSpeed, _planeData._maxSpeed);

    const FVector localMove =
      FVector(_planeData._currForwardSpeed * DeltaTime, 0.f, 0.f);
    _planePawn->AddActorLocalOffset(localMove, true);

     GEngine->AddOnScreenDebugMessage(
       0,
       0.f,
       FColor::Green,
       FString::Printf(TEXT("ForwardSpeed: %f"), localMove.X));

     ProcessPitch();
     ProcessRoll();
     ProcessYaw();

    FRotator deltaRotation(0, 0, 0);
    deltaRotation.Pitch = _planeData._currPitchSpeed * DeltaTime;
    deltaRotation.Yaw = _planeData._currYawSpeed * DeltaTime;
    deltaRotation.Roll = _planeData._currRollSpeed * DeltaTime;

    _planePawn->AddActorLocalRotation(deltaRotation);
  }

  auto Event = ENetEvent();
  if (enet_host_service(Host, &Event, NET_TIMEOUT) > 0) {
    do {
      switch (Event.type) {
        case ENetEventType::ENET_EVENT_TYPE_CONNECT: {
          UE_LOG(ENet6,
                 Log,
                 TEXT("Peer %u connected!"),
                 enet_peer_get_id(Event.peer));
        } break;
        case ENetEventType::ENET_EVENT_TYPE_DISCONNECT: {
          UE_LOG(ENet6,
                 Log,
                 TEXT("Peer %u disconnected!"),
                 enet_peer_get_id(Event.peer));
        } break;
        case ENetEventType::ENET_EVENT_TYPE_RECEIVE: {
          UE_LOG(ENet6,
                 Log,
                 TEXT("Peer %u sent data (%u bytes)"),
                 enet_peer_get_id(Event.peer),
                 enet_packet_get_length(Event.packet));
          enet_packet_dispose(Event.packet);
        } break;
      }
    } while (enet_host_check_events(Host, &Event) > 0);
  }
}

bool
UENet6NetworkSubsystem::Connect(FString AddrStr)
{
  Disconnect();

  auto Address = ENetAddress();
  if (enet_address_set_host(
        &Address, ENET_ADDRESS_TYPE_ANY, TCHAR_TO_UTF8(*AddrStr)) < 0) {
    UE_LOG(ENet6, Error, TEXT("Failed to resolve %s"), *AddrStr);
    return false;
  }
  Address.port = NET_PORT;
  UE_LOG(ENet6, Log, TEXT("Connecting..."));
  Host = enet_host_create(Address.type, nullptr, 1, 0, 0, 0);
  if (Host == nullptr) {
    UE_LOG(ENet6, Error, TEXT("Failed to initialize host"));
    return false;
  }

  ServerPeer = enet_host_connect(Host, &Address, 0, 0);
  check(ServerPeer);

  for (auto i = 0; i < NET_MAX_WAIT / NET_RETRY_TIME; i++) {
    auto Event = ENetEvent();
    if (enet_host_service(Host, &Event, NET_RETRY_TIME) > 0)
      break;
  }

  if (ServerPeer->state != ENET_PEER_STATE_CONNECTED) {
    enet_peer_reset(ServerPeer);
    UE_LOG(ENet6, Error, TEXT("Failed to connect"));
    return false;
  }

  UE_LOG(ENet6, Log, TEXT("Connected to %s"), *AddrStr);
  return true;
}

void
UENet6NetworkSubsystem::Disconnect()
{
  if (ServerPeer != nullptr) {
    check(Host);
    enet_peer_disconnect(ServerPeer, 0);
    enet_host_flush(Host);
    ServerPeer = nullptr;
  }

  if (Host != nullptr) {
    enet_host_destroy(Host);
    Host = nullptr;
  }
}

void
UENet6NetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  if (enet_initialize() < 0) {
    UE_LOG(ENet6, Error, TEXT("An error occurred while initializing ENet."));
    return;
  }

}

void
UENet6NetworkSubsystem::Deinitialize()
{
  enet_deinitialize();
}