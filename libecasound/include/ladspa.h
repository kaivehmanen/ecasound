/* ladspa.h

   Copyright 2000 Richard W.E. Furse, Paul Barton-Davis, Stefan
   Westerfeld. 

   This is a DEVELOPMENT VERSION and NOT FINAL. The contents of this
   file is likely to change, which will probably break any code you
   write. */

#ifndef LADSPA_INCLUDED
#define LADSPA_INCLUDED

/*****************************************************************************/

/* Overview:

   There is a large number of synthesis packages in use or development
   on the Linux platform at this time. This API (`The Linux Audio
   Developer's Simple Plugin API') attempts to give programmers the
   ability to write simple `plugin' audio processors in C and link
   them dynamically (`plug') into a range of these packages (`hosts').
   It should be possible for any host and any plugin to communicate
   completely through this interface.

   This API is deliberately short and simple. To achieve compatibility
   with a range of promising Linux sound synthesis packages it
   attempts to find the `greatest common denominator' in their logical
   behaviour. Having said this, certain limiting decisions are
   implicit, notably the use of a fixed type for all data transfer and
   absence of a parameterised `initialisation' phase. For this version
   of the API the fixed type will be `float.'

   Plugins are expected to use the `control rate' thinking implicit in
   the Music N languages (including Csound). Plugins have `ports' that
   are inputs or outputs for audio or control data and the plugin is
   `run' for a `block' corresponding to a short time interval measured
   in samples. Audio data is communicated using arrays of floats,
   allowing a block of audio to be processed by the plugin in a single
   pass. Control data is communicated using single LADSPA_Data
   values. Control data may only change once per call to the `run'
   function. This approach is not suitable for the transfer of many
   types of data, however it can provide a significant performance
   benefit. The plugin may assume that all its inputs and outputs are
   bound before it is asked to run.

   Plugins will reside in shared object files suitable for dynamic
   linking by dlopen() and family.

   This API contains very limited error-handling at this time.  */

/*****************************************************************************/

/* Fundamental data type passed around between plugins. Corresponds to
   one sample and one control value. */

typedef float LADSPA_Data;

/*****************************************************************************/

/* Special Plugin Properties: 
 
   Optional features of the plugin type are encapsulated in the
   LADSPA_Properties type. This is assembled by ORing individual
   properties together. */

typedef int LADSPA_Properties;

/* Property LADSPA_PROPERTY_REALTIME indicates that the plugin has a
   real-time dependency (e.g. listens to a MIDI device) and so its
   output should not be cached or subject to serious latency. */
#define LADSPA_PROPERTY_REALTIME        0x1

/* Property LADSPA_PROPERTY_INPLACE_BROKEN indicates that the plugin
   may cease to work correctly if the host elects to use the same
   buffer for both input and output. This should be avoided as
   enabling this flag makes it impossible for hosts to use the plugin
   to process a buffer `in-place.' */
#define LADSPA_PROPERTY_INPLACE_BROKEN  0x2

/* Property LADSPA_PROPERTY_HARD_RT_CAPABLE indicates that the plugin
   is capable of running not only in a conventional host but also in a
   `hard real-time' environment. To qualify for this the plugin must
   satisfy all of the following:

   (1) The plugin must not use malloc(), free() or other heap memory
   management within its run() function. All new memory used in run()
   must be managed via the stack. These restrictions are relaxed for
   the instantiate() and connect_port() functions.

   (2) The plugin will not attempt to make use of any library
   functions with the exceptions of functions in the ANSI standard C
   and C maths libraries, which the host is expected to provide.

   (3) The plugin will not access files, devices, pipes, sockets, IPC
   or any other mechanism that might result in process or thread
   blocking.

   (4) The plugin will take an amount of time to execute a run() call
   approximately of form (A+B*SampleCount) where A and B are machine
   and host dependent. This amount of time may not depend on input
   signals or plugin state. The host is left the responsibility to
   perform timings to estimate upper bounds for A and B. */
#define LADSPA_PROPERTY_HARD_RT_CAPABLE 0x4

#define LADSPA_IS_REALTIME(x)	     ((x) & LADSPA_PROPERTY_REALTIME)
#define LADSPA_IS_INPLACE_BROKEN(x)  ((x) & LADSPA_PROPERTY_INPLACE_BROKEN)
#define LADSPA_IS_HARD_RT_CAPABLE(x) ((x) & LADSPA_PROPERTY_HARD_RT_CAPABLE)

/*****************************************************************************/

/* Plugin Ports:

   Plugins have `ports' that are inputs or outputs for audio or
   data. Ports can communicate arrays of LADSPA_Data (for audio-rate
   inputs/outputs) or single LADSPA_Data values (for control-rate
   input/outputs). This information is encapsulated in the
   LADSPA_PortDescriptor type which is assembled by ORing individual
   properties together.

   Note that a port must be an input or an output but not both and
   that a port must be a control rate or audio rate port but not
   both. */

typedef int LADSPA_PortDescriptor;

#define LADSPA_PORT_INPUT   0x1 /* port is input */
#define LADSPA_PORT_OUTPUT  0x2 /* port is output */
#define LADSPA_PORT_CONTROL 0x4 /* port is control-rate (single LADSPA_Data) */
#define LADSPA_PORT_AUDIO   0x8 /* port is audio-rate (LADSPA_Data array) */

#define LADSPA_IS_PORT_INPUT(x)   ((x) & LADSPA_PORT_INPUT)
#define LADSPA_IS_PORT_OUTPUT(x)  ((x) & LADSPA_PORT_OUTPUT)
#define LADSPA_IS_PORT_CONTROL(x) ((x) & LADSPA_PORT_CONTROL)
#define LADSPA_IS_PORT_AUDIO(x)   ((x) & LADSPA_PORT_AUDIO)

/*****************************************************************************/

/* Plugin Handles:

   This plugin handle indicates access to a particular instance of the
   plugin concerned. It is valid to compare this to NULL (0 for C++)
   but otherwise the host should not attempt to interpret this
   value. The plugin may use it to reference instance data. */

typedef void * LADSPA_Handle;

/*****************************************************************************/

/* Descriptor for a Type of Plugin:

   This structure is used to describe a type of plugin. It provides a
   number of functions to examine the type, instantiate it, link it to
   buffers and workspaces and to run it. */

struct LADSPA_Descriptor {

  /* This identifier can be used as a unique, case-sensitive
     identifier for the plugin. Plugins should be identified by file
     and unique label rather than by index or plugin name, which may
     be changed in new versions. */

  const char * UniqueLabel;

  /* This indicates a number of properties of the plugin. */

  LADSPA_Properties Properties;

  /* This member points to the null-terminated name of the plugin
     (e.g. "Sine Oscillator"). */

  const char * Name;

  /* This member points to the null-terminated string indicating the
     maker of the plugin. This can be an empty string if you're
     shy. */

  const char * Maker;

  /* This member points to the null-terminated string indicating any
     copyright applying to the plugin. If no Copyright applies the
     string "None" should be used. "GPL" is a valid string for use
     here, indicating that the GNU General Public Licence applies. */

  const char * Copyright;

  /* This indicates the number of ports (input AND output) present on
     the plugin. */

  unsigned long PortCount;

  /* This member indicates an array of port descriptor. Valid indices
     vary from 0 to PortCount-1. */

  LADSPA_PortDescriptor * PortDescriptors;

  /* This member indicates an array of null-terminated strings
     describing ports (e.g. "Frequency (Hz)"). Valid indices vary from
     0 to PortCount-1. */

  const char ** PortNames;

  /* This may be used by the plugin developer to pass any custom
     implementation data into an instantiate call. It must not be used
     or interpreted by the host. It is expected that most plugin
     writers will not use this facility as LADSPA_Handles should be
     used to hold instance data. */

  void * ImplementationData;

  /* This member is a function pointer that instantiates a plugin. A
     handle is returned indicating the new plugin. The instantiation
     function accepts a sample rate as a parameter. The plugin
     descriptor from which this instantiate function was found must
     also be passed. This function must return NULL if instantiation
     fails. */

  LADSPA_Handle (*instantiate)(const struct LADSPA_Descriptor * Descriptor,
			       unsigned long                    SampleRate);

  /* This member is a function pointer that connects a port on an
     instantiated plugin to a memory location at which a frame of data
     for the port will be read/written. The data location is expected
     to be an array of LADSPA_Data for audio rate ports or a single
     LADSPA_Data value for control rate ports. Memory issues will be
     managed by the host. The plugin must read/write the data at these
     locations every frame and the data present at the time of this
     connection call should not be considered meaningful. Connection
     functions may be called more than once for a single instance of a
     plugin to allow the host to change the buffers that the unit
     generator is reading or writing. They must be called at least
     once for each port. When working with blocks of LADSPA_Data the
     unit generator should pay careful attention to the frame size
     passed to the run function as the block allocated may only just
     be large enough to contain the block of samples.

     Plugins should be aware that the host may elect to use the same
     buffer for more than one port and even use the same buffer for
     both input and output (see LADSPA_PROPERTY_INPLACE_BROKEN).
     However, overlapped buffers may result in unexpected
     behaviour. */

   void (*connect_port)(LADSPA_Handle Instance,
	  	        unsigned long Port,
		        LADSPA_Data * DataLocation);

  /* This method is a function pointer that runs an instance of a unit
     generator for a frame. Two parameters are required: the first is
     a handle to the particular instance to be run and the second
     indicates the frame size (in samples) for which the unit
     generator may run. 

     If the plugin has the property LADSPA_PROPERTY_HARD_RT_CAPABLE
     then there are various things that the plugin should not do
     within the run() function (see above). */

  void (*run)(LADSPA_Handle Instance,
              unsigned long SampleCount);

  /* Once an instance of a plugin has been finished with it can be
     deleted using the following function. The handle passed ceases to
     be valid after this call. */

  void (*cleanup)(LADSPA_Handle Instance);

  /* Reserved area for extensions. Must be initialised to zero. */
  
  char reserved[1000];
  
};

/**********************************************************************/

/* Accessing a Plugin:

   The exact mechanism by which plugins are loaded is host-dependent,
   however all most hosts will need to know is the name of shared
   object file containing the plugins.

   A plugin programmer must include a function called "descriptor"
   with the following function prototype within the shared object
   file.

   A host will find the plugin shared object file by one means or
   another, find the "descriptor" function, call it, and proceed from
   there.

   Plugins are accessed by index using values from 0 upwards. Out of
   range indexes must result in this function returning NULL, so the
   plugin count can be determined by checking for the least index that
   results in NULL being returned. */

typedef struct LADSPA_Descriptor * 
(*LADSPA_Descriptor_Function)(unsigned long Index);

/**********************************************************************/

#endif /* LADSPA_INCLUDED */

/* EOF */
