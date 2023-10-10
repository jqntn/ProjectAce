#pragma once

#include <cstdint>

#include "enet/enet.h"
#include "Inputs.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <cassert>

constexpr auto TICK_RATE = 60;
constexpr auto NET_PORT = 8888;
constexpr auto NET_TICK = 1000 / TICK_RATE;
constexpr auto NET_MAX_WAIT = 2000;
constexpr auto NET_RETRY_TIME = 100;
constexpr auto NET_MAX_CLIENTS = 64;
constexpr auto NET_CHANNELS = 2;
constexpr auto NET_TIMEOUT = 1;

enum class Opcode : uint8_t
{
  SRV_MSG,
  MOVE,
  C_INPUTS,
};

// Un joueur envoie ses inputs au serveur
struct PlayerInputPacket
{
	static constexpr Opcode opcode = Opcode::C_INPUTS;

	PlayerInputs inputs;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerInputPacket Unserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

void PlayerInputPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_f32(byteArray, inputs.pitch);
	Serialize_f32(byteArray, inputs.yaw);
	Serialize_f32(byteArray, inputs.roll);
	Serialize_f32(byteArray, inputs.throttle);
}

PlayerInputPacket PlayerInputPacket::Unserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerInputPacket packet;
	packet.inputs.pitch			= Unserialize_f32(byteArray, offset);
	packet.inputs.yaw			= Unserialize_f32(byteArray, offset);
	packet.inputs.roll			= Unserialize_f32(byteArray, offset);
	packet.inputs.throttle		= Unserialize_f32(byteArray, offset);

	return packet;
}

// Le serveur envoie à un client la liste de tous les joueurs connectés
struct PlayerListPacket
{
	//static constexpr Opcode opcode = Opcode::S_PlayerList;

	struct Player
	{
		std::string name;
		std::uint16_t index;
	};

	std::vector<Player> players;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerListPacket Unserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

void PlayerListPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, players.size());

	for (const auto& player : players)
	{
		Serialize_str(byteArray, player.name);
		Serialize_u16(byteArray, player.index);
	}
}

PlayerListPacket PlayerListPacket::Unserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerListPacket packet;
	packet.players.resize(Unserialize_u16(byteArray, offset));

	for (auto& player : packet.players)
	{
		player.name = Unserialize_str(byteArray, offset);
		player.index = Unserialize_u16(byteArray, offset);
	}

	return packet;
}

// Le serveur informe qu'il vient de passer en phase de course (avec une liste des joueurs présents)
struct PlayersPositionPacket
{
	//static constexpr Opcode opcode = Opcode::S_PlayersPosition;

	struct PlayerData
	{
		std::uint8_t playerIndex;
		//sf::Vector2f position;
	};

	struct CurrentPlayerData
	{
		//sf::Vector2f position;
		//sf::Vector2f velocity;
		bool isOnGround;
	};

	std::optional<CurrentPlayerData> currentPlayerData;
	std::uint32_t lastInputIndex;
	std::uint32_t tickIndex;
	std::vector<PlayerData> players;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayersPositionPacket Unserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

void PlayersPositionPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, lastInputIndex);
	Serialize_u32(byteArray, tickIndex);

	Serialize_u16(byteArray, players.size());
// 	for (const auto& playerData : players)
// 	{
// 		Serialize_u8(byteArray, playerData.playerIndex);
// 		Serialize_f32(byteArray, playerData.position.x);
// 		Serialize_f32(byteArray, playerData.position.y);
// 	}
// 
// 	Serialize_u8(byteArray, currentPlayerData.has_value());
// 	if (currentPlayerData)
// 	{
// 		Serialize_f32(byteArray, currentPlayerData->position.x);
// 		Serialize_f32(byteArray, currentPlayerData->position.y);
// 		Serialize_f32(byteArray, currentPlayerData->velocity.x);
// 		Serialize_f32(byteArray, currentPlayerData->velocity.y);
// 		Serialize_u8(byteArray, currentPlayerData->isOnGround);
// 	}
}

PlayersPositionPacket PlayersPositionPacket::Unserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayersPositionPacket packet;
	packet.lastInputIndex = Unserialize_u32(byteArray, offset);
	packet.tickIndex = Unserialize_u32(byteArray, offset);

	packet.players.resize(Unserialize_u16(byteArray, offset));
// 	for (auto& playerData : packet.players)
// 	{
// 		playerData.playerIndex = Unserialize_u8(byteArray, offset);
// 		playerData.position.x = Unserialize_f32(byteArray, offset);
// 		playerData.position.y = Unserialize_f32(byteArray, offset);
// 	}
// 
// 	bool hasCurrentPlayerData = Unserialize_u8(byteArray, offset);
// 	if (hasCurrentPlayerData)
// 	{
// 		auto& currentPlayerData = packet.currentPlayerData.emplace();
// 		currentPlayerData.position.x = Unserialize_f32(byteArray, offset);
// 		currentPlayerData.position.y = Unserialize_f32(byteArray, offset);
// 		currentPlayerData.velocity.x = Unserialize_f32(byteArray, offset);
// 		currentPlayerData.velocity.y = Unserialize_f32(byteArray, offset);
// 		currentPlayerData.isOnGround = Unserialize_u8(byteArray, offset);
// 	}

	return packet;
}

void			Serialize_f32			(std::vector<std::uint8_t>& byteArray, float value);
void			Serialize_f32			(std::vector<std::uint8_t>& byteArray, std::size_t offset, float value);
void			Serialize_i8			(std::vector<std::uint8_t>& byteArray, std::int8_t value);
void			Serialize_i8			(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int8_t value);
void			Serialize_i16			(std::vector<std::uint8_t>& byteArray, std::int16_t value);
void			Serialize_i16			(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int16_t value);
void			Serialize_i32			(std::vector<std::uint8_t>& byteArray, std::int32_t value);
void			Serialize_i32			(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int32_t value);
void			Serialize_u8			(std::vector<std::uint8_t>& byteArray, std::uint8_t value);
void			Serialize_u8			(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint8_t value);
void			Serialize_u16			(std::vector<std::uint8_t>& byteArray, std::uint16_t value);
void			Serialize_u16			(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint16_t value);
void			Serialize_u32			(std::vector<std::uint8_t>& byteArray, std::uint32_t value);
void			Serialize_u32			(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint32_t value);
void			Serialize_str			(std::vector<std::uint8_t>& byteArray, const std::string& value);
void			Serialize_str			(std::vector<std::uint8_t>& byteArray, std::size_t offset, const std::string& value);

float			Unserialize_f32			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int8_t		Unserialize_i8			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int16_t	Unserialize_i16			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int32_t	Unserialize_i32			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint8_t	Unserialize_u8			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint16_t	Unserialize_u16			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint32_t	Unserialize_u32			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::string		Unserialize_str			(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);

void Serialize_f32(std::vector<std::uint8_t>& byteArray, float value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_f32(byteArray, offset, value);
}

void Serialize_f32(std::vector<std::uint8_t>& byteArray, std::size_t offset, float value)
{
	std::uint32_t v = htonf(value);

	assert(offset + sizeof(v) <= byteArray.size());
	std::memcpy(&byteArray[offset], &v, sizeof(v));
}

void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::int8_t value)
{
	return Serialize_u8(byteArray, static_cast<std::uint8_t>(value));
}

void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int8_t value)
{
	return Serialize_u8(byteArray, offset, static_cast<std::uint8_t>(value));
}

void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::int16_t value)
{
	return Serialize_u16(byteArray, static_cast<std::uint16_t>(value));
}

void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int16_t value)
{
	return Serialize_u16(byteArray, offset, static_cast<std::uint16_t>(value));
}

void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::int32_t value)
{
	return Serialize_u32(byteArray, static_cast<std::uint32_t>(value));
}

void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int32_t value)
{
	return Serialize_u32(byteArray, offset, static_cast<std::uint32_t>(value));
}

void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::uint8_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u8(byteArray, offset, value);
}

void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint8_t value)
{
	assert(offset < byteArray.size());
	byteArray[offset] = value;
}

void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::uint16_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u16(byteArray, offset, value);
}

void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint16_t value)
{
	value = htons(value);

	assert(offset + sizeof(value) <= byteArray.size());
	std::memcpy(&byteArray[offset], &value, sizeof(value));
}

void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::uint32_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u32(byteArray, offset, value);
}

void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint32_t value)
{
	value = htonl(value);

	assert(offset + sizeof(value) <= byteArray.size());
	std::memcpy(&byteArray[offset], &value, sizeof(value));
}

void Serialize_str(std::vector<std::uint8_t>& byteArray, const std::string& value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(std::uint32_t) + value.size());
	return Serialize_str(byteArray, offset, value);
}

void Serialize_str(std::vector<std::uint8_t>& byteArray, std::size_t offset, const std::string& value)
{
	Serialize_u32(byteArray, offset, static_cast<std::uint32_t>(value.size()));
	offset += sizeof(std::uint32_t);

	if (!value.empty())
		std::memcpy(&byteArray[offset], value.data(), value.size());
}

float Unserialize_f32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));

	float v = ntohf(value);

	offset += sizeof(value);

	return v;
}

std::int8_t Unserialize_i8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int8_t>(Unserialize_u8(byteArray, offset));
}

std::int16_t Unserialize_i16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int16_t>(Unserialize_u16(byteArray, offset));
}

std::int32_t Unserialize_i32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int32_t>(Unserialize_u32(byteArray, offset));
}

std::uint8_t Unserialize_u8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint8_t value = byteArray[offset];
	offset += sizeof(value);

	return value;
}

std::uint16_t Unserialize_u16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint16_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));
	value = ntohs(value);

	offset += sizeof(value);

	return value;
}

std::uint32_t Unserialize_u32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));
	value = ntohl(value);

	offset += sizeof(value);

	return value;
}

std::string Unserialize_str(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t length = Unserialize_u32(byteArray, offset);
	std::string str(length, ' ');
	std::memcpy(&str[0], &byteArray[offset], length);

	offset += length;

	return str;
}

// Petite fonction d'aide pour construire un packet ENet à partir d'une de nos structures de packet, insère automatiquement l'opcode au début des données
template<typename T> ENetPacket* build_packet(const T& packet, enet_uint32 flags)
{
	// On sérialise l'opcode puis le contenu du packet dans un std::vector<std::uint8_t>
	std::vector<std::uint8_t> byteArray;

	Serialize_u8(byteArray, static_cast<std::uint8_t>(T::opcode));
	packet.Serialize(byteArray);

	// On copie le contenu de ce vector dans un packet enet, et on l'envoie au peer
	return enet_packet_create(byteArray.data(), byteArray.size(), flags);
}