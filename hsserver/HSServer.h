#pragma once

class HSThread;
class HSSocketManager;
class HSClientConnection;

#include "HSSocket.h"
#include "HSIface.h"
#include "HSMutex.h"
#include <list>

/**
 * \ingroup HS_SERVER
 * \brief Main server that interacts with external clients.
 */
class HSServer
  : HSSocketMessageReceiver
{
public:
  HSServer(void);
  ~HSServer(void);

  /**
   * Start the server.
   */
  void Start();

  /**
   * Stop the server.
   */
  void Stop();

  /**
   * This function starts the server.
   */
  void ServerStart();

  /**
   * This updates the server's clients.
   */
  void UpdateClients();

  /**
   * This function is called by a client when it's done.
   */
  void ClientDisconnected(HSClientConnection *aConnection);

  // HSSocketMessageReceiver
  void ConnectionAccepted(HSSocket *aSocket);

  void ConsoleUpdate(DBRef aUser);

private:
  HSSocketManager *mSocketManager;
  HSSocket *mListeningSocket;
  HSMutex mConnectionMutex;
  std::list<HSSocket*> mClientSockets;
  std::list<HSClientConnection*> mClientConnections;
};

extern HSServer *sServer;
