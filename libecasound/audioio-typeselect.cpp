// ------------------------------------------------------------------------
// audioio-typeselect.cpp: A proxy class for overriding default keyword
//                         and filename associations.
// Copyright (C) 2001,2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include "eca-debug.h"
#include "eca-object-factory.h"
#include "audioio-null.h"
#include "audioio-typeselect.h"

/**
 * Constructor.
 */
AUDIO_IO_TYPESELECT::AUDIO_IO_TYPESELECT (void) { 

  //  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-typeselect) constructor " + label() + ".");  

  child_repp = new NULLFILE("uninitialized");
  init_rep = false;
}

/**
 * Destructor.
 */
AUDIO_IO_TYPESELECT::~AUDIO_IO_TYPESELECT (void)
{
  //  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-typeselect) destructor " + label() + ".");  
  delete child_repp; // either null or the actual child object
}

void AUDIO_IO_TYPESELECT::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-typeselect) open " + label() + ".");  

  if (init_rep != true) {
    AUDIO_IO* tmp = 0;
    if (params_rep.size() > 1) {
      string& type = params_rep[1];
      if (type.size() > 0) {
	//  cerr << "typeselect: " << type << endl;
	tmp = ECA_OBJECT_FACTORY::create_audio_object(type);
      }
    }

    if (tmp != 0) {
      delete child_repp; /* the placeholder null object */
      child_repp = tmp;

      int numparams = child_repp->number_of_params();
      for(int n = 0; n < numparams; n++) {
	child_repp->set_parameter(n + 1, get_parameter(n + 3));
	numparams = child_repp->number_of_params(); // in case 'n_o_p()' varies
      }

      init_rep = true; /* must be set after dyn. parameters */
    }
  }
  
  child_repp->buffersize(buffersize(), samples_per_second());
  child_repp->io_mode(io_mode());
  child_repp->set_audio_format(audio_format());
  child_repp->open();
  if (child_repp->locked_audio_format() == true) {
    set_audio_format(child_repp->audio_format());
  }
  label(child_repp->label());
  toggle_open_state(true);
}

void AUDIO_IO_TYPESELECT::close(void)
{ 
  child_repp->close();
  toggle_open_state(false);
}

string AUDIO_IO_TYPESELECT::parameter_names(void) const
{ 
  return(string("typeselect,format,") + child_repp->parameter_names()); 
}

void AUDIO_IO_TYPESELECT::set_parameter(int param, string value)
{ 
  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-typeselect) set_parameter "
		+ label() + ".");  

  if (param > static_cast<int>(params_rep.size())) params_rep.resize(param);

  if (param > 0) {
    params_rep[param - 1] = value;
  }
  
  if (param > 2 && 
      init_rep == true) {
    child_repp->set_parameter(param - 2, value);
  }
}

string AUDIO_IO_TYPESELECT::get_parameter(int param) const
{
  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-typeselect) get_parameter "
		+ label() + ".");  

  if (param > 0 && param < static_cast<int>(params_rep.size()) + 1) {
    if (param > 2 &&
	init_rep == true) {
      params_rep[param - 1] = child_repp->get_parameter(param - 2);
    }
    return(params_rep[param - 1]);
  }

  return(""); 
}
