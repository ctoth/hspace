#include "HSClientConnection.h"
#include "HSNetworkPacketMessage.h"
#include "HSNetworkPacketLogin.h"
#include "HSNetworkPacketShipInfo.h"
#include "HSNetworkPacketNavStat.h"
#include "HSNetworkPacketEngStat.h"
#include "HSNetworkPacketContact.h"
#include "HSThrusters.h"
#include "HSCargoBay.h"
#include "HSEngine.h"
#include "HSTools.h"
#include "HSDB.h"
#include "HSShip.h"
#include "HSHullClass.h"
#include "HSConsole.h"
#include "HSComputer.h"
#include "HSServer.h"

HSClientConnection::HSClientConnection(HSSocket *aSocket)
  : mSocket(aSocket)
  , mUser(DBRefNothing)
  , mLastNavStat(NULL)
  , mShip(NULL)
{
  mSocket->RegisterMessageReceiver(this);
  HSNetworkPacketMessage packet(HSPM_WELCOME);
  mSocket->Send(packet.ByteStream());
  mSocket->Read();
}

HSClientConnection::~HSClientConnection(void)
{
  delete mSocket;
}

void
HSClientConnection::DataRead(const HSByteArray &aData)
{
  mBuffer += aData;
  ProcessBuffer();
  mSocket->Read();
}

void
HSClientConnection::ProcessBuffer()
{
  while (mBuffer.size() > 0) {
    size_t readSize;
    HSNetworkPacket *incomingPacket = HSNetworkPacket::ReadPacket(mBuffer, &readSize);
    if (!readSize) {
      break;
    }
    HandlePacket(incomingPacket);
    delete incomingPacket;
    mBuffer.Remove(0, readSize);
  }
}

void
HSClientConnection::HandlePacket(HSNetworkPacket *aPacket)
{
  if (aPacket->mType == HSPT_LOGIN) {
    if (mUser != DBRefNothing) {
      return;
    }
    HSNetworkPacketLogin *loginPacket =
      static_cast<HSNetworkPacketLogin*>(aPacket);
    mUser = HSIFLogin(loginPacket->mUserName, loginPacket->mPassword);
    if (mUser == DBRefNothing) {
      SendMessage(HSPM_LOGINDENIED);
    } else {
      SendMessage(HSPM_LOGINACCEPTED);
      HSConsole *consoleManned = sHSDB.FindConsoleMannedBy(mUser);
      if (consoleManned) {
        mShip = consoleManned->GetShip();
      }
    }
  } else if (aPacket->mType == HSPT_MESSAGE) {
    HSNetworkPacketMessage *msgPacket =
      static_cast<HSNetworkPacketMessage*>(aPacket);
    HandleMessage(msgPacket->mMessage);
  }
}

void
HSClientConnection::HandleMessage(int aMsg)
{
  if (aMsg == HSPM_REQUESTSHIPINFO) {
    if (mUser == DBRefNothing || !mShip) {
      SendMessage(HSPM_NOCONSOLEMANNED);
      return;
    }
    HSNetworkPacketShipInfo infoPacket;
    infoPacket.mShipName = mShip->GetName();
    infoPacket.mClassName = mShip->GetHull()->GetName();
    mSocket->Send(infoPacket.ByteStream());
  }
}

void
HSClientConnection::SendMessage(int aMsg)
{
  HSNetworkPacketMessage msg((HSPacketMessage)aMsg);
  mSocket->Send(msg.ByteStream());
}

void
HSClientConnection::UpdateClient()
{
  if (!mShip) {
    return;
  }
  HSNetworkPacketNavStat *navStatPacket = new HSNetworkPacketNavStat();

  navStatPacket->mPosition = mShip->GetLocation();
  navStatPacket->mVelocity = mShip->GetVelocity();
  navStatPacket->mHeading = mShip->GetHeading();
  navStatPacket->mDesiredHeading = mShip->GetDesiredHeading();
  navStatPacket->mDesiredSpeed = (int)mShip->GetDesiredVelocity();
  navStatPacket->mHullMax = mShip->GetHull()->GetHullPoints();
  navStatPacket->mHullCurrent = mShip->GetCurrentHullPoints();

  HSByteArray navStatBytes = navStatPacket->ByteStream(mLastNavStat);
  if (navStatBytes.size()) {
    mSocket->Send(navStatBytes);
    delete mLastNavStat;
    mLastNavStat = navStatPacket;
  } else {
    delete navStatPacket;
  }

  HSNetworkPacketEngStat engStatPacket;

  // Calculate turning rate and stop distance:
  double totalThrust = 0;
  double totalMass = mShip->GetHull()->GetMass();
  double totalAngular = 0;
  foreach(HSSystemInstance*, system, mShip->GetSystems()) {
    totalMass += system->GetMass();
    if (system->GetType() == HSST_ENGINE) {
      HSEngineInstance *engine = static_cast<HSEngineInstance*>(system);
      totalThrust += engine->GetThrust(true);
    }
    if (system->GetType() == HSST_CARGOBAY) {
      HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(system);
      totalMass += bay->GetCargoMass();
    }
    if (system->GetType() == HSST_THRUSTERS) {
      HSThrustersInstance *thrusters = static_cast<HSThrustersInstance*>(system);
      totalAngular = thrusters->GetAngularVelocity(true);
    }
  }
  double angularRate;
  if (!totalMass) {
    angularRate = totalAngular;
  } else {
    angularRate = totalAngular / sqrt(totalMass);  
  }
  if (angularRate > 360) {
    angularRate = 360;
  }
  double accel;
  if (!totalMass) {
    accel = totalThrust;
  } else {
    accel = totalThrust / totalMass;
  }
  engStatPacket.mTurnRate = angularRate;
  engStatPacket.mAcceleration = accel;
  engStatPacket.mTotalMass = (int)totalMass;

  mSocket->Send(engStatPacket.ByteStream());

  // Now lets check for contacts.
  HSComputerInstance *computer = mShip->FindComputer();
  if (!computer) {
    return;
  }

  foreach(HSNetworkPacketContact*, contactPacket, mCurrentContacts) {
    contactPacket->mRemoved = true;
  }
  foreach(HSSensorContact, contact, computer->GetSensorContacts()) {
    bool found = false;
    char type[64];
    if (contact.Object->GetType() == HSOT_SHIP) {
      HSShip *ship = static_cast<HSShip*>(contact.Object);
      if (ship->FindSystemsByType(HSST_ENGINE).empty()) {
        sprintf(type, "Base");
      } else {
        sprintf(type, "Ship");
      }
    } else {
      sprintf(type, "%s", 
        HSObject::GetNameForType((HSObjectType)contact.Object->GetType()).c_str());
    }
    for (size_t i = 0; i < mCurrentContacts.size(); i++) {
      if (mCurrentContacts[i]->mContactID == contact.ID) {
        mCurrentContacts[i]->mRemoved = false;
        HSNetworkPacketContact *contactPacket =
          new HSNetworkPacketContact();
        contactPacket->mContactID = contact.ID;
        contactPacket->mPosition = contact.Object->GetLocation();
        contactPacket->mVelocity = contact.Object->GetVelocity();
        contactPacket->mObjectType = contact.Object->GetType();
        contactPacket->mName = contact.Detail > 50 ? 
          contact.Object->GetName() : type;
        contactPacket->mRemoved = false;
        contactPacket->mObjectID = contact.Object->GetID();
        HSByteArray contactBytes = contactPacket->ByteStream(mCurrentContacts[i]);
        if (contactBytes.size() <= 16) {
          delete contactPacket;
        } else {
          delete mCurrentContacts[i];
          mCurrentContacts[i] = contactPacket;
          mSocket->Send(contactBytes);
        }
        found = true;
      }
    }
    if (!found) {
      HSNetworkPacketContact *contactPacket =
        new HSNetworkPacketContact();
      contactPacket->mContactID = contact.ID;
      contactPacket->mPosition = contact.Object->GetLocation();
      contactPacket->mVelocity = contact.Object->GetVelocity();
      contactPacket->mObjectType = contact.Object->GetType();
      contactPacket->mName = contact.Detail > 50 ? 
          contact.Object->GetName() : type;
      contactPacket->mRemoved = false;
      contactPacket->mObjectID = contact.Object->GetID();
      mSocket->Send(contactPacket->ByteStream());
      mCurrentContacts.push_back(contactPacket);
    }
  }
  for (size_t i = 0; i < mCurrentContacts.size(); i++) {
      if (mCurrentContacts[i]->mRemoved) {
        HSNetworkPacketContact *oldPacket =
          new HSNetworkPacketContact();
        oldPacket->mContactID = mCurrentContacts[i]->mContactID;
        oldPacket->mPosition = mCurrentContacts[i]->mPosition;
        oldPacket->mVelocity = mCurrentContacts[i]->mVelocity;
        oldPacket->mName = mCurrentContacts[i]->mName;
        oldPacket->mType = mCurrentContacts[i]->mType;
        oldPacket->mObjectID = mCurrentContacts[i]->mObjectID;
        oldPacket->mRemoved = false;
        mSocket->Send(mCurrentContacts[i]->ByteStream(oldPacket));
        delete oldPacket;
        delete mCurrentContacts[i];
        for (std::vector<HSNetworkPacketContact*>::iterator iter = mCurrentContacts.begin();
          iter != mCurrentContacts.end(); iter++) {
            if (mCurrentContacts[i] == *iter) {
              mCurrentContacts.erase(iter);
              break;
            }
        }
        --i;
      }
  }
}

void
HSClientConnection::Disconnected()
{
  sServer->ClientDisconnected(this);
}

void
HSClientConnection::ConsoleUpdate()
{
  HSConsole *consoleManned = sHSDB.FindConsoleMannedBy(mUser);
  if (consoleManned) {
    mShip = consoleManned->GetShip();
  } else {
    mShip = NULL;
  }
  if (!mShip) {
    SendMessage(HSPM_NOCONSOLEMANNED);
    return;
  }
  HSNetworkPacketShipInfo infoPacket;
  infoPacket.mShipName = mShip->GetName();
  infoPacket.mClassName = mShip->GetHull()->GetName();
  mSocket->Send(infoPacket.ByteStream());
}
