/**
 *
 * Hemlock Space 5 (HSpace 5)
 * Copyright (c) 2009, Bas Schouten and Shawn Sagady
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 
 *    * Redistribution in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the HSpace 5 Development Team nor the names
 *      of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS  SOFTWARE IS PROVIDED BY THE HSPACE DEVELOPMENT TEAM AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FINTESS FOR A PARTICULAR
 * PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  COPYRIGHT  OWNERS OR
 * CONTRIBUTORS  BE  LIABLE  FOR  ANY  DIRECT,  INDIRECT, INCIDENTAL, SPECIAL
 * EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED TO,
 * PRODUCEMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR  BUSINESS  INTERUPTION)  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER  IN  CONTRACT,  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)  ARISING  IN  ANY  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Original author(s):
 *   Bas Schouten
 *
 */
#include "HSLog.h"
#include <sstream>

#ifdef HS_LOG
#ifdef WIN32
#include <windows.h>
#endif
#include <time.h>

class HSLogger
{
public:
  HSLogger()
  {
  }
  ~HSLogger()
  {
#ifdef WIN32
    ::CloseHandle(file);
#else
    if (fp) {
      fclose(fp);
    }
#endif
  }

  void LogMessage(std::string aMessage)
  {
    time_t rawtime;
    time(&rawtime);
#ifdef WIN32
    ::OutputDebugStringA(aMessage.append("\n").c_str());
    char time[64];
    if (ctime_s(time, 64, &rawtime) != 0) {
      // Eeep! Weird!
      return;
    }
    std::string strTime = time;
    strTime = strTime.substr(0,strTime.length() - 1);
    aMessage = strTime.append(": ").append(aMessage);
    if (!file) {
      file = ::CreateFile("log\\hspace.log", 
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
      if (file == INVALID_HANDLE_VALUE) {
        return;
      }
      SetFilePointer (file, 0, NULL, FILE_END);
    }
    DWORD written;
    ::WriteFile(file, aMessage.append("\r\n").c_str(),
      (DWORD)aMessage.append("\r\n").size(), &written, NULL);
#else
    std::string strTime = ctime(&rawtime);
    strTime = strTime.substr(0,strTime.length() - 1);
    aMessage = strTime.append(": ").append(aMessage);
    if (!fp) {
      fp = fopen("log/hspace.log", "a");
    }

    if (fp) {
      fputs(aMessage.append("\n").c_str(), fp);
      fflush(fp);
    }
#endif
  }

private:
#ifdef WIN32
  HANDLE file;
#else
  FILE *fp;
#endif

} sHSLogger;

#endif
 
HSLogInst::HSLogInst(void)
{
}

HSLogInst::~HSLogInst(void)
{
#ifdef HS_LOG
  sHSLogger.LogMessage(stream.str());
#endif
}

HSLogInst&
HSLogInst::operator <<(std::string aString)
{
#ifdef HS_LOG
  stream << aString;
#endif
  return *this;
}

HSLogInst&
HSLogInst::operator <<(HSAttributed *aObject)
{
#ifdef HS_LOG
  stream << "HSATTRIBUTED(ID:";
    stream << "UNKNOWN";
  stream << ")";
#endif
  return *this;
}

HSLogInst&
HSLogInst::operator <<(int aInt)
{
#ifdef HS_LOG
  stream << aInt;
#endif
  return *this;
}

HSLogInst&
HSLogInst::operator <<(double aDouble)
{
#ifdef HS_LOG
  stream << aDouble;
#endif
  return *this;
}

HSLogInst&
HSLogInst::operator <<(unsigned long aULong)
{
#ifdef HS_LOG
  stream << aULong;
#endif
  return *this;
}
