#ifndef _ECAWAVE_RESOURCES_H
#define _ECAWAVE_RESOURCES_H

#include <ecasound/resource-file.h>

/**
 * Class for representing ecawave user settings (~/.ecawaverc)
 */
class QEResources : public RESOURCE_FILE {

 public:

  void set_defaults(void);

  QEResources(void);
};

#endif
