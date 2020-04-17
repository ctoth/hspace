#include "HSServer.h"
#include <string>
#include <list>

#ifdef WIN32
#include <windows.h>
#endif

// Portab
#include "HSThread.h"
#include "HSSocketManager.h"
#include "HSSocket.h"

// hspace basic
#include "HSTools.h"
#include "HSVersion.h"
#include "HSClientConnection.h"

HSServer *sServer;

HSServer::HSServer(void)
  : mSocketManager(NULL)
{
}

HSServer::~HSServer(void)
{
  Stop();
}

void
HSServer::Start()
{
  if (mSocketManager) {
    // Already running.
    return;
  }
  ServerStart();
}

void
HSServer::Stop()
{
  delete mListeningSocket;
  mListeningSocket = NULL;
  HSAutoMutexLock lock(mConnectionMutex);
  for (std::list<HSClientConnection*>::iterator iter = mClientConnections.begin();
    iter != mClientConnections.end(); iter++)
  {
    delete *iter;
  }
  for (std::list<HSSocket*>::iterator iter = mClientSockets.begin();
    iter != mClientSockets.end(); iter++)
  {
    delete *iter;
  }
  mClientSockets.clear();
  mClientConnections.clear();
  delete mSocketManager;
  mSocketManager = NULL;
  HSLog() << "Server succesfully terminated";
}

void
HSServer::ServerStart()
{
  HSLog() << GetVersionString() << "server starting";
  mSocketManager = new HSSocketManager();
  
  mListeningSocket = mSocketManager->CreateSocket();
  mListeningSocket->Listen(5463);

  mListeningSocket->RegisterMessageReceiver(this);

  mClientSockets.push_back(mSocketManager->CreateSocket());
  mListeningSocket->Accept(*mClientSockets.begin());
}

void
HSServer::UpdateClients()
{
  HSAutoMutexLock lock(mConnectionMutex);
  for (std::list<HSClientConnection*>::iterator iter = mClientConnections.begin();
    iter != mClientConnections.end(); iter++)
  {
    (*iter)->UpdateClient();
  }
}

void 
HSServer::ConnectionAccepted(HSSocket *aSocket)
{
  HSClientConnection *conn = new HSClientConnection(aSocket);
  HSAutoMutexLock lock(mConnectionMutex);
  mClientSockets.remove(aSocket);
  mClientConnections.push_back(conn);
  HSSocket *newSocket = mSocketManager->CreateSocket();
  mClientSockets.push_back(newSocket);
  mListeningSocket->Accept(newSocket);
}

void
HSServer::ClientDisconnected(HSClientConnection *aConnection)
{
  HSAutoMutexLock lock(mConnectionMutex);
  mClientConnections.remove(aConnection);
  delete aConnection;
}

void
HSServer::ConsoleUpdate(DBRef aUser)
{
  HSAutoMutexLock lock(mConnectionMutex);
  for (std::list<HSClientConnection*>::iterator iter = mClientConnections.begin();
    iter != mClientConnections.end(); iter++)
  {
    if ((*iter)->mUser == aUser) {
      (*iter)->ConsoleUpdate();
    }
  }
}
