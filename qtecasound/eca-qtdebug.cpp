// ------------------------------------------------------------------------
// eca-qtdebug.cpp: Qt debug widget for ecasound
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <string>

#include <qwidget.h>
#include <qfont.h>
#include <qstring.h>
#include <qtextview.h>
#include <qregexp.h>

#include <kvutils.h>

#include "qtdebug_if.h"
#include "eca-qtdebug.h"

QEDebug::QEDebug( QWidget *parent, const char *name)
        : QWidget( parent, name )
{
  startTimer(10);

  tview_repp = new QTextView(this, "tview");
}

QSize QEDebug::sizeHint(void) const {
  return(QSize(600,200));
}

void QEDebug::timerEvent(QTimerEvent *) {
  if (qtdebug_queue.cmds_available() == true) {
    string s = qtdebug_queue.front();
    qtdebug_queue.pop_front();
    QString temp (s.c_str());
    temp.replace(QRegExp("\n"), "<br>");
    t_rep += temp;
    t_rep += "<br>";
    unsigned int n = 0;
    for(; n < t_rep.length(); n++) {
      if (t_rep[n] == '<') {
	for(; t_rep[n] != '>'; ++n) {
	  if (n == t_rep.length()) {
	    t_rep += "<";
	    break;
	  }
	}
      }
      if (t_rep.length() - n < 4096) break;
    }
    if (n > 0) {
      t_rep.remove(0, n + 1);
    }
    tview_repp->setTextFormat(Qt::RichText);
    tview_repp->setText("<qt>" + t_rep + "</qt>");
    tview_repp->verticalScrollBar()->setValue(tview_repp->verticalScrollBar()->maxValue());
  }
}

void QEDebug::resizeEvent( QResizeEvent * ) {
  tview_repp->resize(width(), height());
}
