#pragma once

#include <map>
#include <list>
#include <deque>
#include "HSMutex.h"
#include "HSCondVar.h"

class HSSocketPosix;
extern std::map<int,HSSocketManagerPriv*> sSocketAssociation;

class HSSocketManagerPriv
{
public:
  std::map<int,HSSocketPosix*> mActiveSockets;

  void PostActivity(int mSocket);

  bool mShuttingDown;
  std::deque<int> mSocketTasks;
  HSMutex mActivityMutex;
  HSCondVar mActivityCondvar;
};
