#pragma once

#include <string>

class HSSettingsPriv;

/**
 * \ingroup HS_PORTAB
 * \brief This class stores settings. On Win32 this is implemented using the registry.
 */
class HSSettings
{
public:
  /**
   * Get an instance of the settings class. Name is used to identify
   * the application under 'Software'.
   */
  HSSettings(std::string aApplication);
  ~HSSettings(void);

  std::string GetValue(std::string aSetting, std::string aDefaultValue = std::string());
  void SetValue(std::string aSetting, std::string aValue);

private:
  HSSettingsPriv *p;
};
