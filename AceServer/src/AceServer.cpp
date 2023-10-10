#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define ENET_IMPLEMENTATION

#include "include/Protocol.h"
#include "include/enet/enet.h"

#include <iostream>

struct ServerData
{
  ENetHost* host;
};

void
ServerTick(ServerData& serverData);

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
          } break;
          case ENetEventType::ENET_EVENT_TYPE_DISCONNECT: {
            std::cout << enet_peer_get_id(event.peer)
                      << " - Peer Disconnected\n";
          } break;
          case ENetEventType::ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
            std::cout << enet_peer_get_id(event.peer)
                      << " - Peer Disconnected (timeout)\n";
          } break;
          case ENetEventType::ENET_EVENT_TYPE_RECEIVE: {
            std::cout << enet_peer_get_id(event.peer) << " - Packet Received: "
                      << enet_packet_get_length(event.packet) << " bytes\n";
            enet_packet_destroy(event.packet);
          } break;
        }
      while (enet_host_check_events(serverData.host, &event) > 0);

    if (now >= nextTick) {
      ServerTick(serverData);
      nextTick += NET_TICK;
    }
  }

  return EXIT_SUCCESS;
}

void
ServerTick(ServerData& serverData)
{
}