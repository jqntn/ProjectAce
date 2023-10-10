#include "ENet6NetworkSubsystem.h"

#include "../../../../AceServer/src/include/Protocol.h"

DEFINE_LOG_CATEGORY_STATIC(ENet6, Log, All);

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