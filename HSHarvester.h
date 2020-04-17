#pragma once
#include "HSSystem.h"

class HSCommodity;
class HSResource;

class HSHarvester :
  public HSSystem
{
  /** Speed at which we can harvest resources */
  ATTRIBUTE(HarvestSpeed, double)
  /** Resources this system can harvest */
  ATTRIBUTE(HarvestCommods, std::vector<int>)
public:
  HSHarvester(void);
  ~HSHarvester(void);
};

class HSHarvesterInstance :
  public HSSystemInstance
{
  INHERITED_SYSTEM(HSHarvester *)
  ATTRIBUTE_INHERIT_ADJUSTED(HarvestSpeed, double)
  ATTRIBUTE_INHERIT(HarvestCommods, std::vector<int>)
  /** Object this is currently mining */
  ATTRIBUTE(Target, int)
  /** Commodity this is currently mining */
  ATTRIBUTE(Commod, int)
  /** Stored for optimization */
  ATTRIBUTE(TargetObj, HSResource*)
  ATTRIBUTE(CommodObj, HSCommodity*)
  /** 
   * Amount of a commod built up, when reaching a round value
   * this can be transferred to the ship cargo bay.
   */
  ATTRIBUTE(BuiltUp, double)
public:
  HSHarvesterInstance(void);
  ~HSHarvesterInstance(void);

  void Cycle(void);
  void AttributeChanged(std::string aName);

  void CancelHarvesting();
};
