#ifndef _ECA_RESOURCES_H
#define _ECA_RESOURCES_H

#include "resource-file.h"

/**
 * Class for representing ecasound user settings (~/.ecasoundrc)
 */
class ECA_RESOURCES : public RESOURCE_FILE {

 public:

  void set_defaults(void);

  ECA_RESOURCES(void);
};

#endif





