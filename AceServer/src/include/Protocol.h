#pragma once

#include <cstdint>

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
};