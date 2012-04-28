// ------------------------------------------------------------------------
// audiofx_lv2_world.cpp: Utility class for LV2 plugin loading
// Copyright (C) 2000-2004, 2011 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
//
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if ECA_USE_LIBLILV

#include "audiofx_lv2_world.h"

#define LV2PREFIX "http://lv2plug.in/ns/lv2core#"
#define IN_PLACE_BROKEN_URI LV2PREFIX "inPlaceBroken"
#define SAMPLERATE_URI LV2PREFIX "sampleRate"
#define TOGGLED_URI LV2PREFIX "toggled"
#define INTEGER_URI LV2PREFIX "integer"
#define LOGARITHMIC_URI "http://lv2plug.in/ns/dev/extportinfo#"
#define CONNECTION_OPTIONAL_URI LV2PREFIX "connectionOptional"

ECA_LV2_WORLD ECA_LV2_WORLD::i=ECA_LV2_WORLD();

ECA_LV2_WORLD::ECA_LV2_WORLD()
{
	lilvworld =0;
	audioclassnode=0;
	controlclassnode=0;
	inputclassnode=0;
	outputclassnode=0;
	inplacebrokennode=0;
	porttogglednode=0;
	portintegernode=0;
	portlogarithmicnode=0;
	portsampleratedependentnode=0;
	portconnectionoptionalnode=0;
}

ECA_LV2_WORLD::~ECA_LV2_WORLD()
{
	lilv_world_free(lilvworld);
}

#define DECLARE_ACESSOR(TYPE,METHODNAME,VARIABLENAME,VALUE) \
	TYPE* ECA_LV2_WORLD::METHODNAME() { \
		if(i.VARIABLENAME == 0){ \
			i.VARIABLENAME = VALUE; \
		} \
		return i.VARIABLENAME; \
	}

DECLARE_ACESSOR(LilvWorld, World, lilvworld, lilv_world_new())
DECLARE_ACESSOR(LilvNode, AudioClassNode,audioclassnode,lilv_new_uri(World(),LILV_URI_AUDIO_PORT))
DECLARE_ACESSOR(LilvNode, ControlClassNode,controlclassnode,lilv_new_uri(World(),LILV_URI_CONTROL_PORT))
DECLARE_ACESSOR(LilvNode, InputClassNode,inputclassnode,lilv_new_uri(World(),LILV_URI_INPUT_PORT))
DECLARE_ACESSOR(LilvNode, OutputClassNode,outputclassnode,lilv_new_uri(World(),LILV_URI_OUTPUT_PORT))
DECLARE_ACESSOR(LilvNode, InPlaceBrokenNode,inplacebrokennode,lilv_new_uri(World(),IN_PLACE_BROKEN_URI))
DECLARE_ACESSOR(LilvNode, PortToggledNode,porttogglednode,lilv_new_uri(World(),TOGGLED_URI))
DECLARE_ACESSOR(LilvNode, PortIntegerNode,portintegernode,lilv_new_uri(World(),INTEGER_URI))
DECLARE_ACESSOR(LilvNode, PortLogarithmicNode,portlogarithmicnode,lilv_new_uri(World(),LOGARITHMIC_URI))
DECLARE_ACESSOR(LilvNode, PortSamplerateDependentNode,portsampleratedependentnode,lilv_new_uri(World(),SAMPLERATE_URI))
DECLARE_ACESSOR(LilvNode, PortConnectionOptionalNode,portconnectionoptionalnode,lilv_new_uri(World(),CONNECTION_OPTIONAL_URI))

#endif /* ECA_USE_LIBLILV */
