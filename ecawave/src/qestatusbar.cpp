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

#include <ecasound/eca-controller.h>

#include "qestatusbar.h"
#include "version.h"

QEStatusBar::QEStatusBar (ECA_CONTROLLER* ctrl,
			  const string& filename,
			  QWidget *parent, 
			  const char *name)
  : QStatusBar( parent, name ),
    ectrl(ctrl),
    filename_rep(filename) { 
  editing_rep = false;

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(500, false);
}

void QEStatusBar::visible_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end) {
  vstartpos = start;
  vendpos = end;
}

void QEStatusBar::marked_area(ECA_AUDIO_TIME start, ECA_AUDIO_TIME end) {
  mstartpos = start;
  mendpos = end;
}

void QEStatusBar::update(void) {
  string status;
  if (ectrl->is_running() == true) status = "running";
  else status = " - ";


  if (filename_rep.empty() == true) {
    message("ecawave ready - no file loaded"); 
  }
  else {
    string begin;
    if (editing_rep == true) 
      begin = filename_rep + " (*)";
    else 
      begin = filename_rep;
    
    message(string(begin + 
		 " - visible [ " +
		   vstartpos.to_string(ECA_AUDIO_TIME::format_seconds) + "s - " + 
		   vendpos.to_string(ECA_AUDIO_TIME::format_seconds) + "s ] - marked [ " +
		   mstartpos.to_string(ECA_AUDIO_TIME::format_seconds) + "s - " + 
		   mendpos.to_string(ECA_AUDIO_TIME::format_seconds) + "s ] - current " +
		   curpos.to_string(ECA_AUDIO_TIME::format_seconds) + "s - " +
		   " status [" +
		   status + "]").c_str()); 
  }
}
