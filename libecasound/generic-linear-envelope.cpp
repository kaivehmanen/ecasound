// ------------------------------------------------------------------------
// generic-linear-envelope.cpp: Generic linear envelope
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
// Copyright (C) 2001 Arto Hamara (artham@utu.fi)
//
// This program is fre software; you can redistribute it and/or modify
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

#include <kvutils/kvu_numtostr.h>
#include <kvutils/message_item.h>

#include "generic-linear-envelope.h"
#include "eca-debug.h"

CONTROLLER_SOURCE::parameter_type GENERIC_LINEAR_ENVELOPE::value(void) {
    curpos += step_length();

    if (curpos < pos_rep[0]) {
        curval = val_rep[0];
    } else if (curstage < static_cast<int>(pos_rep.size())-1) {
        if (curpos >= pos_rep[curstage+1]) {
            ++curstage;
            if( curstage < static_cast<int>(pos_rep.size())-1)
                curval = ( ( (curpos - pos_rep[curstage]) * val_rep[curstage+1] +
                             (pos_rep[curstage+1] - curpos) * val_rep[curstage] ) /
                           (pos_rep[curstage+1] - pos_rep[curstage]) );
            else
                curval = val_rep.back();
        } else {
            curval = ( ( (curpos - pos_rep[curstage]) * val_rep[curstage+1] +
                         (pos_rep[curstage+1] - curpos) * val_rep[curstage] ) /
                       (pos_rep[curstage+1] - pos_rep[curstage]) );
        }
    }
   
    return(curval);
}

GENERIC_LINEAR_ENVELOPE::GENERIC_LINEAR_ENVELOPE(void) { } 

void GENERIC_LINEAR_ENVELOPE::init(parameter_type step) {
    step_length(step);

    curpos = 0.0;
    curval = 0.0;
    curstage = -1;

    pos_rep.resize(1);
    val_rep.resize(1);
    pos_rep[0] = 1;
    val_rep[0] = 0;

    set_param_count(0);
    
    ecadebug->msg("(generic-linear-envelope) Envelope created.");
}

void GENERIC_LINEAR_ENVELOPE::set_param_count(int params) {
    param_names_rep = "point_count";
    if (params > 0) {
        for(int n = 0; n < params; ++n) {
            param_names_rep += ",pos";
            param_names_rep += n*2+1;
            param_names_rep += ",val";
            param_names_rep += n*2+2;
        }
    }
}

string GENERIC_LINEAR_ENVELOPE::parameter_names(void) const { 
    return(param_names_rep);
}

void GENERIC_LINEAR_ENVELOPE::set_parameter(int param, parameter_type value) {
    switch(param) {
        case 1:
            set_param_count(static_cast<int>(value));
            pos_rep.resize(static_cast<int>(value));
            val_rep.resize(static_cast<int>(value));
            break;
        default:
            int pointnum = param/2 - 1;
            if (param%2 == 0)
                pos_rep[pointnum] = value;
            else
                val_rep[pointnum] = value;
    }
}

CONTROLLER_SOURCE::parameter_type GENERIC_LINEAR_ENVELOPE::get_parameter(int param) const {
    switch(param) {
        case 1:
            return(static_cast<parameter_type>(number_of_params() - 1));
            break;
        default:
            int pointnum = param/2 - 1;
            if (pointnum >= static_cast<int>(pos_rep.size())) {
                return 0.0;
            }

            if (param&2 == 0)
                return pos_rep[pointnum];
            else
                return val_rep[pointnum];
    }
}
