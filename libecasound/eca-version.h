#ifndef INCLUDED_ECA_VERSION_H
#define INCLUDED_ECA_VERSION_H

/**
 * Ecasound library version as a formatted string.
 *
 * "vX.Y[.Z][devT]" :
 *
 * x = major version  - the overall development status
 *
 * y = minor version  - represents a set of planned features (see TODO)
 *
 * z = revision       - version number of the current development series
 *
 * devT = dev-release - development releases leading to the stable
 * 		        release X.Y[.Z]
 */
extern const char* ecasound_library_version;

/**
 * Ecasound library libtool version number (current:revision:age)
 */
extern const long int ecasound_library_version_current;
extern const long int ecasound_library_version_revision;
extern const long int ecasound_library_version_age;

#define ECASOUND_LIBRARY_VERSION             "1.9dev1"
#define ECASOUND_LIBRARY_VERSION_CURRENT     7
#define ECASOUND_LIBRARY_VERSION_REVISION    0
#define ECASOUND_LIBRARY_VERSION_AGE         0

#endif
