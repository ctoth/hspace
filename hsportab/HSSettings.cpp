#include "HSSettings.h"
#include <windows.h>

class HSSettingsPriv
{
public:
  HKEY mKey;
};

HSSettings::HSSettings(std::string aApplication)
{
  p = new HSSettingsPriv();
  int rv;

  rv = ::RegCreateKeyEx(HKEY_CURRENT_USER,
    std::string("Software\\").append(aApplication).c_str(),
    0,
    NULL,
    0,
    KEY_READ | KEY_WRITE,
    NULL,
    &p->mKey,
    NULL);

}

HSSettings::~HSSettings(void)
{
  ::RegCloseKey(p->mKey);
  delete p;
}

std::string
HSSettings::GetValue(std::string aSetting, std::string aDefaultValue)
{
  char tbuf[512];
  DWORD buflen = 512;

  int rv = ::RegQueryValueEx(p->mKey,
    aSetting.c_str(),
    NULL,
    NULL,
    (BYTE*)tbuf,
    &buflen);

  if (rv == ERROR_SUCCESS) {
    return tbuf;
  } else {
    return aDefaultValue;
  }
}

void
HSSettings::SetValue(std::string aSetting, std::string aValue)
{
  ::RegSetValueEx(p->mKey,
    aSetting.c_str(),
    0,
    REG_SZ,
    (const BYTE *)aValue.c_str(),
    (DWORD)aValue.size() + 1);
}
