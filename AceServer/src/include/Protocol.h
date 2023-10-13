#pragma once

#ifdef SRV
#include "enet/enet.h"
#else
#define htonf
#define ntohf
#include <enet6/enet.h>
#endif

#include "math/vector3.hpp"
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

constexpr auto TICK_RATE = 60;
constexpr auto NET_PORT = 8888;
constexpr auto NET_TICK = 1000 / TICK_RATE;
constexpr auto NET_MAX_WAIT = 1000;
constexpr auto NET_RETRY_TIME = 100;
constexpr auto NET_MAX_CLIENTS = 64;
constexpr auto NET_CHANNELS = 2;
constexpr auto NET_TIMEOUT = 1;

constexpr auto TargetInputBufferSize = 5;
constexpr auto TargetInterpolationBufferSize = 5;

enum class Opcode : uint8_t
{
  C_PLAYERINPUT,
  S_GAMEDATA,
  S_PLAYERLIST,
  S_PLAYERPOSITION
};

struct PlayerInput
{
  uint32_t Index = 0;
  float Pitch = 0;
  float Yaw = 0;
  float Roll = 0;
  float Throttle = 0;
};

void
Serialize_f32(std::vector<uint8_t>& byteArray, float value);
void
Serialize_f32(std::vector<uint8_t>& byteArray, size_t offset, float value);
void
Serialize_i8(std::vector<uint8_t>& byteArray, int8_t value);
void
Serialize_i8(std::vector<uint8_t>& byteArray, size_t offset, int8_t value);
void
Serialize_i16(std::vector<uint8_t>& byteArray, int16_t value);
void
Serialize_i16(std::vector<uint8_t>& byteArray, size_t offset, int16_t value);
void
Serialize_i32(std::vector<uint8_t>& byteArray, int32_t value);
void
Serialize_i32(std::vector<uint8_t>& byteArray, size_t offset, int32_t value);
void
Serialize_u8(std::vector<uint8_t>& byteArray, uint8_t value);
void
Serialize_u8(std::vector<uint8_t>& byteArray, size_t offset, uint8_t value);
void
Serialize_u16(std::vector<uint8_t>& byteArray, uint16_t value);
void
Serialize_u16(std::vector<uint8_t>& byteArray, size_t offset, uint16_t value);
void
Serialize_u32(std::vector<uint8_t>& byteArray, uint32_t value);
void
Serialize_u32(std::vector<uint8_t>& byteArray, size_t offset, uint32_t value);
void
Serialize_str(std::vector<uint8_t>& byteArray, const std::string& value);
void
Serialize_str(std::vector<uint8_t>& byteArray,
              size_t offset,
              const std::string& value);

float
Unserialize_f32(const std::vector<uint8_t>& byteArray, size_t& offset);
int8_t
Unserialize_i8(const std::vector<uint8_t>& byteArray, size_t& offset);
int16_t
Unserialize_i16(const std::vector<uint8_t>& byteArray, size_t& offset);
int32_t
Unserialize_i32(const std::vector<uint8_t>& byteArray, size_t& offset);
uint8_t
Unserialize_u8(const std::vector<uint8_t>& byteArray, size_t& offset);
uint16_t
Unserialize_u16(const std::vector<uint8_t>& byteArray, size_t& offset);
uint32_t
Unserialize_u32(const std::vector<uint8_t>& byteArray, size_t& offset);
std::string
Unserialize_str(const std::vector<uint8_t>& byteArray, size_t& offset);

struct PlayerInputPacket
{
  PlayerInput Input;

  static constexpr Opcode Opcode = Opcode::C_PLAYERINPUT;
  void Serialize(std::vector<uint8_t>& byteArray) const;
  static PlayerInputPacket Unserialize(const std::vector<uint8_t>& byteArray,
                                       size_t& offset);
};
inline void
PlayerInputPacket::Serialize(std::vector<uint8_t>& byteArray) const
{
  Serialize_f32(byteArray, Input.Pitch);
  Serialize_f32(byteArray, Input.Yaw);
  Serialize_f32(byteArray, Input.Roll);
  Serialize_f32(byteArray, Input.Throttle);
}
inline PlayerInputPacket
PlayerInputPacket::Unserialize(const std::vector<uint8_t>& byteArray,
                               size_t& offset)
{
  PlayerInputPacket packet{};
  packet.Input.Pitch = Unserialize_f32(byteArray, offset);
  packet.Input.Yaw = Unserialize_f32(byteArray, offset);
  packet.Input.Roll = Unserialize_f32(byteArray, offset);
  packet.Input.Throttle = Unserialize_f32(byteArray, offset);
  return packet;
}

struct GameDataPacket
{
  uint16_t PlayerIndex = 0;

  static constexpr Opcode Opcode = Opcode::S_GAMEDATA;
  void Serialize(std::vector<uint8_t>& byteArray) const;
  static GameDataPacket Unserialize(const std::vector<uint8_t>& byteArray,
                                    size_t& offset);
};
inline void
GameDataPacket::Serialize(std::vector<uint8_t>& byteArray) const
{
  Serialize_u16(byteArray, PlayerIndex);
}
inline GameDataPacket
GameDataPacket::Unserialize(const std::vector<uint8_t>& byteArray,
                            size_t& offset)
{
  GameDataPacket packet{};
  packet.PlayerIndex = Unserialize_u16(byteArray, offset);
  return packet;
}

struct PlayerListPacket
{
  struct Player
  {
    uint16_t Index = 0;
    std::string Name;
  };
  std::vector<Player> Players;

  static constexpr Opcode Opcode = Opcode::S_PLAYERLIST;
  void Serialize(std::vector<uint8_t>& byteArray) const;
  static PlayerListPacket Unserialize(const std::vector<uint8_t>& byteArray,
                                      size_t& offset);
};
inline void
PlayerListPacket::Serialize(std::vector<uint8_t>& byteArray) const
{
  Serialize_u16(byteArray, Players.size());
  for (const auto& player : Players) {
    Serialize_str(byteArray, player.Name);
    Serialize_u16(byteArray, player.Index);
  }
}
inline PlayerListPacket
PlayerListPacket::Unserialize(const std::vector<uint8_t>& byteArray,
                              size_t& offset)
{
  PlayerListPacket packet{};
  packet.Players.resize(Unserialize_u16(byteArray, offset));
  for (auto& player : packet.Players) {
    player.Name = Unserialize_str(byteArray, offset);
    player.Index = Unserialize_u16(byteArray, offset);
  }
  return packet;
}

struct PlayersPositionPacket
{
  struct PlayerData
  {
    uint8_t PlayerIndex = 0;
    Vector3 Position;
  };
  struct CurrentPlayerData
  {
    Vector3 Position;
    Vector3 Velocity;
  };

  std::optional<CurrentPlayerData> CurrentPlayerData;
  uint32_t LastInputIndex = 0;
  uint32_t TickIndex = 0;
  std::vector<PlayerData> Players;

  static constexpr Opcode Opcode = Opcode::S_PLAYERPOSITION;
  void Serialize(std::vector<uint8_t>& byteArray) const;
  static PlayersPositionPacket Unserialize(
    const std::vector<uint8_t>& byteArray,
    size_t& offset);
};
inline void
PlayersPositionPacket::Serialize(std::vector<uint8_t>& byteArray) const
{
  Serialize_u32(byteArray, LastInputIndex);
  Serialize_u32(byteArray, TickIndex);
  Serialize_u16(byteArray, Players.size());
  for (const auto& playerData : Players) {
    Serialize_u8(byteArray, playerData.PlayerIndex);
    Serialize_f32(byteArray, playerData.Position.x);
    Serialize_f32(byteArray, playerData.Position.y);
    Serialize_f32(byteArray, playerData.Position.z);
  }

  Serialize_u8(byteArray, CurrentPlayerData.has_value());
  if (CurrentPlayerData) {
    Serialize_f32(byteArray, CurrentPlayerData->Position.x);
    Serialize_f32(byteArray, CurrentPlayerData->Position.y);
    Serialize_f32(byteArray, CurrentPlayerData->Position.z);
    Serialize_f32(byteArray, CurrentPlayerData->Velocity.x);
    Serialize_f32(byteArray, CurrentPlayerData->Velocity.y);
    Serialize_f32(byteArray, CurrentPlayerData->Velocity.z);
  }
}
inline PlayersPositionPacket
PlayersPositionPacket::Unserialize(const std::vector<uint8_t>& byteArray,
                                   size_t& offset)
{
  PlayersPositionPacket packet{};
  packet.LastInputIndex = Unserialize_u32(byteArray, offset);
  packet.TickIndex = Unserialize_u32(byteArray, offset);

  packet.Players.resize(Unserialize_u16(byteArray, offset));
  for (auto& playerData : packet.Players) {
    playerData.PlayerIndex = Unserialize_u8(byteArray, offset);
    playerData.Position.x = Unserialize_f32(byteArray, offset);
    playerData.Position.y = Unserialize_f32(byteArray, offset);
    playerData.Position.z = Unserialize_f32(byteArray, offset);
  }

  bool hasCurrentPlayerData = (bool)Unserialize_u8(byteArray, offset);
  if (hasCurrentPlayerData) {
    auto& currentPlayerData = packet.CurrentPlayerData.emplace();
    currentPlayerData.Position.x = Unserialize_f32(byteArray, offset); // x
    currentPlayerData.Position.y = Unserialize_f32(byteArray, offset); // y
    currentPlayerData.Position.z = Unserialize_f32(byteArray, offset); // z
    currentPlayerData.Velocity.x = Unserialize_f32(byteArray, offset); // x
    currentPlayerData.Velocity.y = Unserialize_f32(byteArray, offset); // y
    currentPlayerData.Velocity.z = Unserialize_f32(byteArray, offset); // z
  }

  return packet;
}

inline void
Serialize_f32(std::vector<uint8_t>& byteArray, float value)
{
  size_t offset = byteArray.size();
  byteArray.resize(offset + sizeof(value));
  return Serialize_f32(byteArray, offset, value);
}
inline void
Serialize_f32(std::vector<uint8_t>& byteArray, size_t offset, float value)
{
  uint32_t v = htonf(value);
  assert(offset + sizeof(v) <= byteArray.size());
  memcpy(&byteArray[offset], &v, sizeof(v));
}
inline void
Serialize_i8(std::vector<uint8_t>& byteArray, int8_t value)
{
  return Serialize_u8(byteArray, static_cast<uint8_t>(value));
}
inline void
Serialize_i8(std::vector<uint8_t>& byteArray, size_t offset, int8_t value)
{
  return Serialize_u8(byteArray, offset, static_cast<uint8_t>(value));
}
inline void
Serialize_i16(std::vector<uint8_t>& byteArray, int16_t value)
{
  return Serialize_u16(byteArray, static_cast<uint16_t>(value));
}
inline void
Serialize_i16(std::vector<uint8_t>& byteArray, size_t offset, int16_t value)
{
  return Serialize_u16(byteArray, offset, static_cast<uint16_t>(value));
}
inline void
Serialize_i32(std::vector<uint8_t>& byteArray, int32_t value)
{
  return Serialize_u32(byteArray, static_cast<uint32_t>(value));
}
inline void
Serialize_i32(std::vector<uint8_t>& byteArray, size_t offset, int32_t value)
{
  return Serialize_u32(byteArray, offset, static_cast<uint32_t>(value));
}
inline void
Serialize_u8(std::vector<uint8_t>& byteArray, uint8_t value)
{
  size_t offset = byteArray.size();
  byteArray.resize(offset + sizeof(value));
  return Serialize_u8(byteArray, offset, value);
}
inline void
Serialize_u8(std::vector<uint8_t>& byteArray, size_t offset, uint8_t value)
{
  assert(offset < byteArray.size());
  byteArray[offset] = value;
}
inline void
Serialize_u16(std::vector<uint8_t>& byteArray, uint16_t value)
{
  size_t offset = byteArray.size();
  byteArray.resize(offset + sizeof(value));
  return Serialize_u16(byteArray, offset, value);
}
inline void
Serialize_u16(std::vector<uint8_t>& byteArray, size_t offset, uint16_t value)
{
  value = ENET_HOST_TO_NET_16(value);
  assert(offset + sizeof(value) <= byteArray.size());
  memcpy(&byteArray[offset], &value, sizeof(value));
}
inline void
Serialize_u32(std::vector<uint8_t>& byteArray, uint32_t value)
{
  size_t offset = byteArray.size();
  byteArray.resize(offset + sizeof(value));
  return Serialize_u32(byteArray, offset, value);
}
inline void
Serialize_u32(std::vector<uint8_t>& byteArray, size_t offset, uint32_t value)
{
  value = ENET_HOST_TO_NET_32(value);
  assert(offset + sizeof(value) <= byteArray.size());
  memcpy(&byteArray[offset], &value, sizeof(value));
}
inline void
Serialize_str(std::vector<uint8_t>& byteArray, const std::string& value)
{
  size_t offset = byteArray.size();
  byteArray.resize(offset + sizeof(uint32_t) + value.size());
  return Serialize_str(byteArray, offset, value);
}
inline void
Serialize_str(std::vector<uint8_t>& byteArray,
              size_t offset,
              const std::string& value)
{
  Serialize_u32(byteArray, offset, static_cast<uint32_t>(value.size()));
  offset += sizeof(uint32_t);
  if (!value.empty())
    memcpy(&byteArray[offset], value.data(), value.size());
}

inline float
Unserialize_f32(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  uint32_t value;
  memcpy(&value, &byteArray[offset], sizeof(value));
  float v = ntohf(value);
  offset += sizeof(value);
  return v;
}
inline int8_t
Unserialize_i8(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  return static_cast<int8_t>(Unserialize_u8(byteArray, offset));
}
inline int16_t
Unserialize_i16(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  return static_cast<int16_t>(Unserialize_u16(byteArray, offset));
}
inline int32_t
Unserialize_i32(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  return static_cast<int32_t>(Unserialize_u32(byteArray, offset));
}
inline uint8_t
Unserialize_u8(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  uint8_t value = byteArray[offset];
  offset += sizeof(value);
  return value;
}
inline uint16_t
Unserialize_u16(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  uint16_t value;
  memcpy(&value, &byteArray[offset], sizeof(value));
  value = ntohs(value);
  offset += sizeof(value);
  return value;
}
inline uint32_t
Unserialize_u32(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  uint32_t value;
  memcpy(&value, &byteArray[offset], sizeof(value));
  value = ntohl(value);
  offset += sizeof(value);
  return value;
}
inline std::string
Unserialize_str(const std::vector<uint8_t>& byteArray, size_t& offset)
{
  uint32_t length = Unserialize_u32(byteArray, offset);
  std::string str(length, ' ');
  memcpy(&str[0], &byteArray[offset], length);
  offset += length;
  return str;
}

template<typename T>
ENetPacket*
BuildPacket(const T& packet, enet_uint32 flags)
{
  std::vector<uint8_t> byteArray;
  Serialize_u8(byteArray, static_cast<uint8_t>(T::Opcode));
  packet.Serialize(byteArray);
  return enet_packet_create(byteArray.data(), byteArray.size(), flags);
}