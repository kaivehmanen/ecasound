#ifndef INCLUDED_ECA_RESOURCES_H
#define INCLUDED_ECA_RESOURCES_H

#include <string>
#include "resource-file.h"

/**
 * Class for representing ecasound user settings stored
 * in global ({prefix}/share/ecasound/ecasoundrc) and 
 * user-specific (~/.ecasoundrc) resource files.
 */
class ECA_RESOURCES {

 public:

  std::string resource(const std::string& tag) const;
  bool boolean_resource(const std::string& tag) const;

  bool has(const std::string& tag) const;

  void resource(const std::string& tag, const std::string& value);
  void set_defaults(void);

  ECA_RESOURCES(void);
  ~ECA_RESOURCES(void);

 private:

  std::string user_resource_directory_rep;
  RESOURCE_FILE globalrc_rep;
  RESOURCE_FILE userrc_rep;
};

#endif
