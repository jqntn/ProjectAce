// Fill out your copyright notice in the Description page of Project Settings.

#include "ENet6NetworkSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(ENet6, Log, All);

ETickableTickType
UENet6NetworkSubsystem::GetTickableTickType() const
{
  return ETickableTickType::Always;
}

bool
UENet6NetworkSubsystem::IsAllowedToTick() const
{
  return true;
}

void
UENet6NetworkSubsystem::Tick(float /*DeltaTime*/)
{
  if (!Host) {
    return;
  }

  ENetEvent Event;
  if (enet_host_service(Host, &Event, 1) > 0) {
    do {
      switch (Event.type) {
        // On est connecté au serveur (ne devrait pas arriver si géré dans la
        // méthode Connect)
        case ENetEventType::ENET_EVENT_TYPE_CONNECT:
          UE_LOG(ENet6,
                 Log,
                 TEXT("Peer %u connected!"),
                 enet_peer_get_id(Event.peer));
          break;

        // On a perdu la connexion au serveur
        case ENetEventType::ENET_EVENT_TYPE_DISCONNECT:
          UE_LOG(ENet6,
                 Log,
                 TEXT("Peer %u disconnected!"),
                 enet_peer_get_id(Event.peer));
          break;

        // On a reçu des données de la part du serveur (plus qu'à désérialiser
        // ^_^)
        case ENetEventType::ENET_EVENT_TYPE_RECEIVE:
          UE_LOG(ENet6,
                 Log,
                 TEXT("Peer %u sent data (%u bytes)"),
                 enet_peer_get_id(Event.peer),
                 enet_packet_get_length(Event.packet));
          enet_packet_dispose(Event.packet);
          break;
      }
    } while (enet_host_check_events(Host, &Event) > 0);
  }
}

TStatId
UENet6NetworkSubsystem::GetStatId() const
{
  return TStatId();
}

bool
UENet6NetworkSubsystem::Connect(FString AddressString)
{
  // Petit défaut de cette méthode : elle bloque le temps de la connexion (cinq
  // secondes de timeout) Pour éviter ça, vous pouvez le gérer en asynchrone via
  // l'événement ENET_EVENT_TYPE_CONNECT dans le tick
  Disconnect();

  ENetAddress Address;
  if (enet_address_set_host(
        &Address, ENET_ADDRESS_TYPE_ANY, TCHAR_TO_UTF8(*AddressString)) != 0) {
    UE_LOG(ENet6, Error, TEXT("Failed to resolve %s"), *AddressString);
    return false;
  }

  Address.port = 8888;
  UE_LOG(ENet6, Log, TEXT("Connecting..."));

  // On ne créé l'host qu'une fois qu'on connait le type d'adresse (IPv4/IPv6)
  Host = enet_host_create(Address.type, nullptr, 1, 0, 0, 0);
  if (!Host) {
    UE_LOG(ENet6, Error, TEXT("Failed to initialize host"));
    return false;
  }

  // On tente une connexion...
  ServerPeer = enet_host_connect(Host, &Address, 0, 0);
  check(ServerPeer);

  // On utilise la fonction enet_host_service avant d'entrer dans la boucle pour
  // valider la connexion Note : il est important d'appeler plusieurs fois la
  // fonction plutôt qu'une seule fois dans ce cas, pour autoriser plusieurs
  // tentatives de connexion (l'API ENet ayant été conçue autour du fait
  // qu'enet_host_service était appelé en boucle)

  // Nous simulons ça avec une boucle, où chaque tour de boucle va attendre
  // 100ms (pour un total de 5s)
  for (std::size_t i = 0; i < 50; ++i) {
    ENetEvent Event;
    if (enet_host_service(Host, &Event, 100) > 0) {
      // Nous avons un événement, la connexion a soit pu s'effectuer
      // (ENET_EVENT_TYPE_CONNECT) soit échoué (ENET_EVENT_TYPE_DISCONNECT)
      break; //< On sort de la boucle
    }
  }

  // On vérifie après la boucle l'état du peer, sommes-nous connectés ?
  if (ServerPeer->state != ENET_PEER_STATE_CONNECTED) {
    // On force la réinitialisation du serveur auprès d'enet pour pouvoir
    // allouer un nouveau peer
    enet_peer_reset(ServerPeer);
    UE_LOG(ENet6, Error, TEXT("Failed to connect"));
    return false;
  }

  // Si on arrive ici, on est connecté !
  UE_LOG(ENet6, Log, TEXT("Connected to %s"), *AddressString);

  return true;
}

void
UENet6NetworkSubsystem::Disconnect()
{
  if (ServerPeer) {
    check(Host);

    enet_peer_disconnect(ServerPeer, 0);
    enet_host_flush(Host);
    ServerPeer = nullptr;
  }

  if (Host) {
    enet_host_destroy(Host);
    Host = nullptr;
  }
}

void
UENet6NetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  if (enet_initialize() != 0) {
    UE_LOG(ENet6, Error, TEXT("failed to initialize ENet"));
    return;
  }
}

void
UENet6NetworkSubsystem::Deinitialize()
{
  enet_deinitialize();
}