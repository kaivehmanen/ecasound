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
#include <qmultilinedit.h>
#include <qfont.h>
#include <qstring.h>

#include <kvutils.h>

#include "qtdebug_if.h"
#include "eca-qtdebug.h"

QEDebug::QEDebug( QWidget *parent=0, const char *name) 
        : QWidget( parent, name )
{
  startTimer(10);

  //  setMinimumSize(100, 100);
  //  setMaximumSize(600, 600);

  mle = new QMultiLineEdit(this, "mle");
  mle->setAutoUpdate(true);
  mle->setFixedVisibleLines(1000);
  mle->setMinimumSize(width(), height());

  connect(this, SIGNAL(append(const QString&)), mle, SLOT(append(const QString&)) );
  mle->setReadOnly(true);
}

QSize QEDebug::sizeHint(void) const {
  return(QSize(600,200));
}

void QEDebug::timerEvent( QTimerEvent * ) {
  if (qtdebug_queue.cmds_available() == true) {
    string s = qtdebug_queue.front();
    //    debug(s.c_str());
    qtdebug_queue.pop_front();
    string t = "";

    QFont somefont;
    for(string::const_iterator p = s.begin(); p != s.end(); p++) {
      if (*p == '\e') {
	while(*p != 'm') ++p;
      }
      else if (*p == '\n') {
	emit append(t.c_str());
	mle->setCursorPosition (mle->numLines(), 0);
	t = "";
      }
      else if ((int)*p < 33)
	t += ' ';
      else 
	t += *p;
    }
    emit append(QString(t.c_str()));
    mle->setCursorPosition (mle->numLines(), 0);
  }
}

void QEDebug::resizeEvent( QResizeEvent * ) {
  mle->resize(width(), height());
}
