#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define ENET_IMPLEMENTATION

#include "include/Protocol.h"

#include <iostream>

struct GameData
{
  uint32_t TickIndex = 0;
  std::vector<Player> Players;
};

struct ServerData
{
  ENetHost* host;
};

void
ServerTick(ServerData& serverData, GameData& gameData);

void
HandleMessage(Player& player,
              GameData& gameData,
              const std::vector<uint8_t>& message);

ENetPacket*
BuidPlayerListPacket(const GameData& gameData);

ENetPacket*
BuildPlayerPositionPacket(GameData& gameData, const Player& player);

int
main()
{
  if (enet_initialize() < 0) {
    std::cerr << "An error occurred while initializing ENet.\n";
    return EXIT_FAILURE;
  }
  atexit(enet_deinitialize);

  auto address = ENetAddress{ ENET_HOST_ANY, NET_PORT };
  auto server = enet_host_create(&address, NET_MAX_CLIENTS, NET_CHANNELS, 0, 0);
  if (server == nullptr) {
    std::cerr
      << "An error occurred while trying to create an ENet server host.\n";
    return EXIT_FAILURE;
  }

  auto gameData = GameData();
  auto serverData = ServerData{ server };
  auto nextTick = enet_time_get();

  while (true) {
    auto now = enet_time_get();
    auto event = ENetEvent();
    while (enet_host_service(serverData.host, &event, NET_TIMEOUT) > 0)
      do
        switch (event.type) {
          case ENetEventType::ENET_EVENT_TYPE_CONNECT: {
            std::cout << enet_peer_get_id(event.peer) << " - Peer Connected\n";
            {
              auto it =
                find_if(gameData.Players.begin(),
                        gameData.Players.end(),
                        [&](const auto& p) { return p.Peer == nullptr; });
              if (it == gameData.Players.end()) {
                auto& player = gameData.Players.emplace_back();
                player.Index = gameData.Players.size() - 1;
                it = gameData.Players.end() - 1;
              }
              auto& player = *it;
              player.Peer = event.peer;
              player.Name.clear();
              player.Position = StartPos;
              {
                auto gameDataPacket = GameDataPacket();
                gameDataPacket.PlayerIndex = player.Index;
                enet_peer_send(
                  player.Peer,
                  0,
                  BuildPacket(gameDataPacket, ENET_PACKET_FLAG_RELIABLE));
              }
            }
            {
              auto playerListPacket = BuidPlayerListPacket(gameData);
              for (const auto& player : gameData.Players)
                if (player.Peer != nullptr)
                  enet_peer_send(player.Peer, 0, playerListPacket);
            }
          } break;

          case ENetEventType::ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
            std::cout << enet_peer_get_id(event.peer)
                      << " - Peer Disconnected (timeout)\n";
          }

          case ENetEventType::ENET_EVENT_TYPE_DISCONNECT: {
            std::cout << enet_peer_get_id(event.peer)
                      << " - Peer Disconnected\n";

            auto it =
              find_if(gameData.Players.begin(),
                      gameData.Players.end(),
                      [&](const auto& p) { return p.Peer == event.peer; });

            auto& player = *it;
            player.Peer = nullptr;

            auto playerListPacket = BuidPlayerListPacket(gameData);
            for (const auto& player : gameData.Players)
              if (player.Peer != nullptr)
                enet_peer_send(player.Peer, 0, playerListPacket);
          } break;

          case ENetEventType::ENET_EVENT_TYPE_RECEIVE: {
            std::cout << enet_peer_get_id(event.peer) << " - Packet Received: "
                      << enet_packet_get_length(event.packet) << " bytes\n";

            auto it =
              find_if(gameData.Players.begin(),
                      gameData.Players.end(),
                      [&](const auto& p) { return p.Peer == event.peer; });

            auto& player = *it;

            auto len = event.packet->dataLength;
            auto bytes = std::vector<uint8_t>(len);
            memcpy(bytes.data(), event.packet->data, len);

            HandleMessage(player, gameData, bytes);

            enet_packet_destroy(event.packet);
          } break;
        }
      while (enet_host_check_events(serverData.host, &event) > 0);

    if (now >= nextTick) {
      ServerTick(serverData, gameData);
      nextTick += NET_TICK;
    }
  }

  return EXIT_SUCCESS;
}

void
ServerTick(ServerData& serverData, GameData& gameData)
{
  gameData.TickIndex++;

  for (auto& player : gameData.Players) {
    if (!player.InputBuffer.empty()) {
      auto inc = 1.f;
      if (player.InputBuffer.size() < TargetInputBufferSize)
        inc *= 1.f - (TargetInputBufferSize - player.InputBuffer.size()) * .05f;
      else if (player.InputBuffer.size() > TargetInputBufferSize)
        inc *= 1.f + (player.InputBuffer.size() - TargetInputBufferSize) * .05f;
      player.InputBufferAdvancement += inc;
      if (player.InputBufferAdvancement >= 1.f)
        while (player.InputBufferAdvancement >= 1.f) {
          auto& currentInput = player.InputBuffer.front();
          player.LastInput.Index = currentInput.Index;
          player.LastInput.Yaw = currentInput.Yaw;
          player.LastInput.Pitch = currentInput.Pitch;
          player.LastInput.Roll = currentInput.Roll;
          player.LastInput.Throttle = currentInput.Throttle;
          player.InputBuffer.erase(player.InputBuffer.begin());
          player.InputBufferAdvancement -= 1.f;
        }
    }
    ComputePhysics(player, player.LastInput, NET_TICK);
  }

  for (const auto& player : gameData.Players)
    if (player.Peer != nullptr) {
      auto packet = BuildPlayerPositionPacket(gameData, player);
      enet_peer_send(player.Peer, 0, packet);
    }
}

void
HandleMessage(Player& player,
              GameData& gameData,
              const std::vector<uint8_t>& message)
{
  auto offset = size_t();
  auto opcode = static_cast<Opcode>(Unserialize_u8(message, offset));

  switch (opcode) {
    case Opcode::C_PLAYERINPUT: {
      auto input = PlayerInputPacket::Unserialize(message, offset);
      player.InputBuffer.push_back(input.Input);
    } break;
  }
}

ENetPacket*
BuidPlayerListPacket(const GameData& gameData)
{
  auto packet = PlayerListPacket();
  for (const auto& player : gameData.Players)
    if (player.Peer != nullptr) {
      auto& packetPlayer = packet.Players.emplace_back();
      packetPlayer.Index = player.Index;
      packetPlayer.Name = player.Name;
    }
  return BuildPacket(packet, ENET_PACKET_FLAG_RELIABLE);
}

ENetPacket*
BuildPlayerPositionPacket(GameData& gameData, const Player& player)
{
  auto packet = PlayersPositionPacket();
  packet.LastInputIndex = player.LastInput.Index;
  packet.TickIndex = gameData.TickIndex;

  auto& currentPlayerData = packet.CurrentPlayerData.emplace();
  currentPlayerData.Position = player.Position;

  for (const auto& other : gameData.Players) {
    if (player.Index == other.Index)
      continue;
    if (other.Peer != nullptr) {
      auto& packetPlayer = packet.Players.emplace_back();
      packetPlayer.PlayerIndex = other.Index;
      packetPlayer.Position = other.Position;
    }
  }

  return BuildPacket(packet, 0);
}