#pragma once

#include "HSSocket.h"
#include "HSByteArray.h"
#include "HSIface.h"
#include "HSComputer.h"

class HSNetworkPacket;
class HSNetworkPacketContact;
class HSShip;

class HSClientConnection
  : public HSSocketMessageReceiver
{
public:
  HSClientConnection(HSSocket *aSocket);
  ~HSClientConnection(void);

  void DataRead(const HSByteArray &aData);
  void ProcessBuffer();
  void HandlePacket(HSNetworkPacket *aPacket);
  void HandleMessage(int aMsg);
  void SendMessage(int aMsg);
  void UpdateClient();
  void Disconnected();

  void ConsoleUpdate();
private:
  friend class HSServer;

  HSSocket *mSocket;
  HSByteArray mBuffer;

  DBRef mUser;

  HSNetworkPacket *mLastNavStat;

  HSShip *mShip;

  std::vector<HSNetworkPacketContact*> mCurrentContacts;
};
