// ------------------------------------------------------------------------
// eca-qtdebug.cpp: qt debug widget
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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

  tview = new QTextView(this, "tview");
}

QSize QEDebug::sizeHint(void) const {
  return(QSize(600,200));
}

void QEDebug::timerEvent( QTimerEvent * ) {
  if (qtdebug_queue.cmds_available() == true) {
    string s = qtdebug_queue.front();
    qtdebug_queue.pop_front();
    QString temp (s.c_str());
    temp.replace(QRegExp("\n"), "<br>");
    t += temp;
    t += "<br>";
    unsigned int n = 0;
    for(; n < t.length(); n++) {
      if (t[n] == '<') {
	for(; t[n] != '>'; ++n) {
	  if (n == t.length()) {
	    t += "<";
	    break;
	  }
	}
      }
      if (t.length() - n < 4096) break;
    }
    if (n > 0) {
      t.remove(0, n + 1);
    }
    tview->setTextFormat(Qt::RichText);
    tview->setText("<qt>" + t + "</qt>");
    tview->verticalScrollBar()->setValue(tview->verticalScrollBar()->maxValue());
    //    tview->ensureVisible(0, tview->height(), 0, 0);
  }
}

void QEDebug::resizeEvent( QResizeEvent * ) {
  tview->resize(width(), height());
}
