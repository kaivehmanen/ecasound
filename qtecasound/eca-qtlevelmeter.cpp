// ------------------------------------------------------------------------
// eca-qtlevelmeter.cpp: A simple signal levelmeter widget for qt. 
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
#include <vector>
#include <cmath>

#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "eca-qtlevelmeter.h"

QELevelMeter::QELevelMeter(double max_value, QWidget *parent=0, const char *name=0 ) 
        : QWidget( parent, name )
{
  startTimer(5);
  setMaximumHeight(40);
  //  setMinimumSize(400, 40);

  maxval = max_value;
  curval = 0.0;
}

void QELevelMeter::set_value(double value) { 
  lastvalue = value;
  //  lastnew = value;
}

void QELevelMeter::timerEvent( QTimerEvent * ) {
  // display(curval);
  //  setProgress((int)ceil(100.0 * curval / maxval));
  if (lastvalue == curval) return;
  else curval = lastvalue;
  //  curval = lastnew;
  repaint(true);
}

void QELevelMeter::paintEvent( QPaintEvent* e )
{
    static QRect ur;
    ur = e->rect(); 
    //    static QPixmap pix(ur.size());          // Pixmap for double-buffering
    static QPixmap pix;
    pix.resize(ur.size());
    pix.fill( this, ur.topLeft() );     // fill with widget background
    static QPainter p;
    p.begin( &pix );
    p.setBrush(red);
    p.translate( -ur.x(), -ur.y() );    // use widget coordinate system
    //                                        // when drawing on pixmap
    //    curval += (lastnew - curval) / 1.5;
    p.drawRect(0, 0, curval / maxval * width(), height());

    p.end();

    bitBlt( this, ur.topLeft(), &pix );
}










