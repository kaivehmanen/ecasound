// ------------------------------------------------------------------------
// qestatusbar.cpp: Statusbar that displays audio file info
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <qstatusbar.h>
#include <qtimer.h>

#include <ecasound/eca-control.h>

#include "qestatusbar.h"
#include "version.h"

QEStatusBar::QEStatusBar (ECA_CONTROL* ctrl,
			  QWidget *parent, 
			  const char *name)
  : QStatusBar( parent, name ),
    ectrl_repp(ctrl) { 
  editing_rep = false;

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(500, false);
}

void QEStatusBar::visible_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end) {
  vstartpos_rep = start;
  vendpos_rep = end;
}

void QEStatusBar::marked_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end) {
  mstartpos_rep = start;
  mendpos_rep = end;
}

void QEStatusBar::update(void) {
  string status;

  if (editing_rep == true) status += " (*) ";

  status += "status [";
  if (ectrl_repp->is_running() == true) status += "running";
  else status += " - ";
  status += "]";
  
  status += " - visible [ " +
            vstartpos_rep.to_string(ECA_AUDIO_TIME::format_seconds) + "s - " + 
            vendpos_rep.to_string(ECA_AUDIO_TIME::format_seconds) + "s ] - marked [ " +
	    mstartpos_rep.to_string(ECA_AUDIO_TIME::format_seconds) + "s - " + 
	    mendpos_rep.to_string(ECA_AUDIO_TIME::format_seconds) + "s ] - current " +
            curpos_rep.to_string(ECA_AUDIO_TIME::format_seconds) + "s";
  message(status.c_str());
}
