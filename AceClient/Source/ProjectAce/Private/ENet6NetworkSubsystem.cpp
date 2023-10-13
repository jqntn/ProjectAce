#include "ENet6NetworkSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(ENet6, Log, All);

#pragma region _Plane_

void
UENet6NetworkSubsystem::InitPlaneData(float acceleration,
                                      float maxSpeed,
                                      float minSpeed,
                                      float pitchRateMultiplier,
                                      float rollRateMultiplier,
                                      float startForwardSpeed)
{
  _planeData._accel = acceleration;
  _planeData._maxSpeed = maxSpeed;
  _planeData._minSpeed = minSpeed;
  _planeData._pitchRateMult = pitchRateMultiplier;
  _planeData._rollRateMult = rollRateMultiplier;
  _planeData._startForwardSpeed = startForwardSpeed;
}

void
UENet6NetworkSubsystem::ProcessKeyPitch(float rate)
{
  if (FMath::Abs(rate) > .2f)
    ProcessPitch(rate * 2.f);
}

void
UENet6NetworkSubsystem::ProcessKeyRoll(float rate)
{
  if (FMath::Abs(rate) > .2f)
    ProcessRoll(rate * 2.f);
}

void
UENet6NetworkSubsystem::ProcessMouseYInput(float value)
{
  ProcessPitch(value);
}

void
UENet6NetworkSubsystem::ProcessMouseXInput(float value)
{
  ProcessRoll(value);
}

void
UENet6NetworkSubsystem::SetIntentionalRoll(float value)
{
  if ((_planePawn->GetActorRotation().Roll <= 0.1f &&
       _planePawn->GetActorRotation().Roll >= -0.1f) &&
      value == 0.f)
    return;
  _planeData._bIntentionalRoll = FMath::Abs(value) > 0.f;

  float targetRollSpeed = 0.f;
  if (_planeData._bIntentionalRoll) {
    targetRollSpeed = (value * _planeData._rollRateMult);
  } else {
    targetRollSpeed = (_planePawn->GetActorRotation().Roll * -2.f);
  }

  _planeData._currRollSpeed = FMath::FInterpTo(_planeData._currRollSpeed,
                                               targetRollSpeed,
                                               GetWorld()->GetDeltaSeconds(),
                                               2.f);
}

void
UENet6NetworkSubsystem::SetIntentionalPitch(float value)
{
  _planeData._bIntentionalPitch = FMath::Abs(value) > 0.f;
}

void
UENet6NetworkSubsystem::ProcessRoll(float value)
{
  return;
  if (_planeData._bIntentionalPitch && !_planeData._bIntentionalRoll)
    return;

  const float targetRollSpeed =
    _planeData._bIntentionalRoll
      ? (value * _planeData._rollRateMult)
      : (/*_planePawn->GetActorRotation().Roll * -2.f*/ 0.0f);
  _planeData._currRollSpeed = FMath::FInterpTo(_planeData._currRollSpeed,
                                               targetRollSpeed,
                                               GetWorld()->GetDeltaSeconds(),
                                               2.f);
}

void
UENet6NetworkSubsystem::ProcessPitch(float value)
{
  gameData.Input.Throttle = value;

  const float targetPitchSpeed = value * _planeData._pitchRateMult;
  _planeData._currPitchSpeed = FMath::FInterpTo(_planeData._currPitchSpeed,
                                                targetPitchSpeed,
                                                GetWorld()->GetDeltaSeconds(),
                                                2.f);
}

void
UENet6NetworkSubsystem::SetPlanePawn(APawn* pawn)
{
  _planePawn = pawn;

  if (!_planePawn)
    return;
  _planeData._currForwardSpeed = _planeData._startForwardSpeed;
  _planePawn->bUseControllerRotationPitch = false;
  _planePawn->bUseControllerRotationRoll = false;
  _planePawn->bUseControllerRotationYaw = false;
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
  {
    // if (_planePawn) {
    //   // Compute Thrust
    //   const float currAccel =
    //     -_planePawn->GetActorRotation().Pitch * DeltaTime *
    //     _planeData._accel; // Tilt up > slow down, Tilt down > accelerate
    //   const float newForwardSpeed = _planeData._currForwardSpeed + currAccel;
    //   _planeData._currForwardSpeed = FMath::Clamp(
    //     newForwardSpeed, _planeData._minSpeed, _planeData._maxSpeed);

    //  const FVector localMove =
    //    FVector(_planeData._currForwardSpeed * DeltaTime, 0.f, 0.f);
    //  _planePawn->AddActorLocalOffset(localMove, true);

    //  GEngine->AddOnScreenDebugMessage(
    //    0,
    //    0.f,
    //    FColor::Green,
    //    FString::Printf(TEXT("ForwardSpeed: %f"),
    //                    _planeData._currForwardSpeed));

    //  FRotator deltaRotation(0, 0, 0);
    //  deltaRotation.Pitch = _planeData._currPitchSpeed * DeltaTime;
    //  deltaRotation.Yaw = _planeData._currYawSpeed * DeltaTime;
    //  deltaRotation.Roll = _planeData._currRollSpeed * DeltaTime;

    //  _planePawn->AddActorLocalRotation(deltaRotation);
    //}
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
          auto len = Event.packet->dataLength;
          auto bytes = std::vector<uint8_t>(len);
          memcpy(bytes.data(), Event.packet->data, len);
          HandleMessage(bytes);
          enet_packet_dispose(Event.packet);
        } break;
      }
    } while (enet_host_check_events(Host, &Event) > 0);
  }

  auto it = std::find_if(
    gameData.Players.begin(), gameData.Players.end(), [&](const auto& p) {
      return p.Index == gameData.OwnPlayerIndex;
    });
  assert(it != gameData.players.end());
  auto& player = *it;

  auto packet = PlayerInputPacket();
  packet.Input = gameData.Input;
  packet.Input.Index = gameData.InputIndex++;

  ComputePhysics(player, packet.Input, NET_TICK);

  enet_peer_send(ServerPeer, 0, BuildPacket(packet, ENET_PACKET_FLAG_RELIABLE));

  auto predictedInput = PredictedInput();
  predictedInput.Input = packet.Input;
  gameData.PredictedInputs.push_back(predictedInput);

  if (gameData.InterpolationBuffer.size() >= 2) {
    auto& from = gameData.InterpolationBuffer[0];
    auto& to = gameData.InterpolationBuffer[1];

    auto packetDiff = to.TickIndex - from.TickIndex;

    auto interpolationIncr = DeltaTime / NET_TICK;
    interpolationIncr /= packetDiff;

    if (gameData.InterpolationBuffer.size() >= TargetInterpolationBufferSize)
      interpolationIncr *= 1.f + 0.2f * (gameData.InterpolationBuffer.size() -
                                         TargetInterpolationBufferSize);
    else
      interpolationIncr *=
        std::fmaxf(1.f - 0.2f * (TargetInterpolationBufferSize -
                                 gameData.InterpolationBuffer.size()),
                   0.f);

    for (const auto& fromPlayer : from.Players) {
      auto playerIt = std::find_if(
        gameData.Players.begin(), gameData.Players.end(), [&](const auto& p) {
          return p.Index == fromPlayer.PlayerIndex;
        });
      if (playerIt == gameData.Players.end())
        continue;

      auto& pplayer = *playerIt;
      if (fromPlayer.PlayerIndex == gameData.OwnPlayerIndex)
        continue;

      auto toIt = std::find_if(
        to.Players.begin(), to.Players.end(), [&](const auto& toPlayer) {
          return toPlayer.PlayerIndex == fromPlayer.PlayerIndex;
        });
      if (toIt == to.Players.end())
        continue;

      auto& toPlayer = *toIt;

      pplayer.Position = toPlayer.Position;

      // pplayer.Position = SmoothLerp(
      //   fromPlayer.Position, toPlayer.Position, gameData.InterpolationTime,
      //   1);
    }

    gameData.InterpolationTime += interpolationIncr;
    if (gameData.InterpolationTime >= 1.f) {
      gameData.InterpolationBuffer.erase(gameData.InterpolationBuffer.begin());
      gameData.InterpolationTime -= 1.f;
    }
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

void
UENet6NetworkSubsystem::ComputePhysics(Player& player,
                                       const PlayerInput& input,
                                       float elapsedTime)
{
}

void
UENet6NetworkSubsystem::HandleMessage(const std::vector<uint8_t>& message)
{
  auto offset = size_t();
  auto opcode = static_cast<Opcode>(Unserialize_u8(message, offset));

  switch (opcode) {
    case Opcode::S_GAMEDATA: {
      auto packet = GameDataPacket::Unserialize(message, offset);
      gameData.OwnPlayerIndex = packet.PlayerIndex;
      gameData.OwnPlayerIndex = packet.PlayerIndex;
      gameData.OwnPlayerIndex = packet.PlayerIndex;
    } break;
    case Opcode::S_PLAYERLIST: {
      auto packet = PlayerListPacket::Unserialize(message, offset);
      for (auto it = gameData.Players.begin(); it != gameData.Players.end();) {
        auto playerIt =
          std::find_if(packet.Players.begin(),
                       packet.Players.end(),
                       [&](const auto& p) { return p.Index == it->Index; });
        if (playerIt == packet.Players.end())
          it = gameData.Players.erase(it);
        else
          ++it;
      }
      for (const auto& packetPlayer : packet.Players) {
        auto it = std::find_if(
          gameData.Players.begin(), gameData.Players.end(), [&](const auto& p) {
            return p.Index == packetPlayer.Index;
          });
        if (it == gameData.Players.end()) {
          auto& player = gameData.Players.emplace_back();
          player.Index = packetPlayer.Index;
          player.Name = packetPlayer.Name;
        }
      }
    } break;
    case Opcode::S_PLAYERPOSITION: {
      auto packet = PlayersPositionPacket::Unserialize(message, offset);

      auto ownPlayerItGameData = std::find_if(
        gameData.Players.begin(), gameData.Players.end(), [&](const auto& p) {
          return p.Index == gameData.OwnPlayerIndex;
        });
      assert(ownPlayerItGameData != gameData.players.end());
      auto& ownPlayerGameData = *ownPlayerItGameData;

      auto ownPlayerItPacket = std::find_if(
        packet.Players.begin(), packet.Players.end(), [&](const auto& p) {
          return p.PlayerIndex == gameData.OwnPlayerIndex;
        });
      if (ownPlayerItPacket != packet.Players.end()) {
        auto& ownPlayerPacket = *ownPlayerItPacket;

        const FVector localMove = FVector(ownPlayerPacket.Position.x,
                                          ownPlayerPacket.Position.y,
                                          ownPlayerPacket.Position.z);
        _planePawn->AddActorLocalOffset(localMove, true);
      }

      // if (gameData.InterpolationBuffer.empty()) {
      //   auto tickIndex = packet.TickIndex;
      //   for (auto i = 0; i < TargetInterpolationBufferSize; ++i) {
      //     packet.TickIndex =
      //       tickIndex - (TargetInterpolationBufferSize - i - 1);
      //     gameData.InterpolationBuffer.push_back(packet);
      //   }
      // } else
      //   gameData.InterpolationBuffer.push_back(packet);

      // while (!gameData.PredictedInputs.empty() &&
      //        gameData.PredictedInputs.front().Input.Index <=
      //          packet.LastInputIndex)
      //   gameData.PredictedInputs.erase(gameData.PredictedInputs.begin());

      // auto ownPlayerIt = std::find_if(
      //   gameData.Players.begin(), gameData.Players.end(), [&](const auto& p)
      //   {
      //     return p.Index == gameData.OwnPlayerIndex;
      //   });
      // assert(ownPlayerIt != gameData.players.end());

      // auto& ownPlayer = *ownPlayerIt;
      // if (packet.CurrentPlayerData) {
      //   auto predictedPosition = ownPlayer.Position;

      //  ownPlayer.Position = packet.CurrentPlayerData->Position;
      //  ownPlayer.Velocity = packet.CurrentPlayerData->Velocity;

      //  for (auto& predictedInput : gameData.PredictedInputs)
      //    ComputePhysics(ownPlayer, predictedInput.Input, NET_TICK);

      //  auto reconciliatedPosition = ownPlayer.Position;
      //  auto diff = reconciliatedPosition - predictedPosition;
      //}
    } break;
  }
}