// ------------------------------------------------------------------------
// eca-qtwaveform.cpp: Qt-widget for visualizing audio signals.
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

#include <qapplication.h>
#include <qwidget.h>
#include <qaccel.h>
#include <qlayout.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qevent.h>

#include <kvutils.h>

#include "audioio.h"
#include "eca-qtwavedata.h"
#include "eca-qtwaveform.h"
#include "eca-qtinte.h"
#include "eca-debug.h"

QEWaveForm::QEWaveForm(AUDIO_IO_FILE* iod, QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
  setMinimumSize(400, 200);

  string caption = "";

  wdata = new QEWaveData(iod, this, "wdata");
  if (wdata !=0 && wdata->is_valid()) {
    valid_iodevice = true;
    caption = "qtecasound - waveform view (" + iod->label() + ")";
  }
  else {
    valid_iodevice = false;
    caption = "qtecasound - waveform view (no file)";
  }

  topLayout = new QVBoxLayout( this );
  buttons = new QHBoxLayout();

  init_buttons();
  init_shortcuts();

  topLayout->addLayout(buttons,1);
  topLayout->addWidget(wdata, 2, 0);

  setCaption(caption.c_str());
}

void QEWaveForm::init_shortcuts(void) {
  QAccel *a = new QAccel(this);

  a->connectItem(a->insertItem(Key_Exclam),
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()),
		 SLOT(get_focus()));

  a->connectItem(a->insertItem(Key_Exclam),
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()),
		 SLOT(get_focus()));

  a->connectItem(a->insertItem(SHIFT+Key_Exclam),
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()),
		 SLOT(get_focus()));

  a->connectItem(a->insertItem(Key_R), this,
		 SLOT(updateWaveView()));
  a->connectItem(a->insertItem(SHIFT+Key_R), this,
		 SLOT(updateWaveView()));
  a->connectItem(a->insertItem(CTRL+Key_R), this,
		 SLOT(updateWaveView()));

  a->connectItem(a->insertItem(Key_W), this,
		 SLOT(forcedUpdateWaveData()));
  a->connectItem(a->insertItem(SHIFT+Key_W), this,
		 SLOT(forcedUpdateWaveData()));
  a->connectItem(a->insertItem(CTRL+Key_W), this,
		 SLOT(forcedUpdateWaveData()));

  a->connectItem(a->insertItem(Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(SHIFT+Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(CTRL+Key_Q), this,
		 SLOT(close()));
}

void QEWaveForm::init_buttons(void) {
  QFont butfont ("Helvetica", 12, QFont::Normal);

  QPushButton* cpanelbut = new QPushButton( "(!) Control panel", this, "cpanelbut" );
  cpanelbut->setFont(butfont);
  buttons->addWidget( cpanelbut, 1, 0);

  QObject::connect( cpanelbut, SIGNAL(clicked()), 
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()), 
		 SLOT(get_focus()));

  QPushButton* refresh = new QPushButton( "(R)efresh waveview", this, "refresh" );
  refresh->setFont(butfont);
  buttons->addWidget( refresh, 1, 0);

  QPushButton* rescan = new QPushButton( "Rescan (w)avedata", this, "refresh" );
  rescan->setFont(butfont);
  buttons->addWidget( rescan, 1, 0);

  QPushButton* quit = new QPushButton( "(Q)uit", this, "quit" );
  quit->setFont(butfont);
  buttons->addWidget( quit, 2, 0);

  QObject::connect( rescan, SIGNAL(clicked()), this,
		    SLOT(forcedUpdateWaveData()));
  QObject::connect( refresh, SIGNAL(clicked()), this,
		    SLOT(updateWaveView()));
  QObject::connect( quit, SIGNAL(clicked()), this, SLOT(close()));
  //  connect(quit, SIGNAL(clicked()), this, SLOT(emsg_quit()) );
}

void QEWaveForm::updateWaveView(void) 
{
  wdata->repaint();
}

void QEWaveForm::updateWaveData(bool force)
{
  if (!valid_iodevice) return;
  else {
    wdata->updateWaveData(force);
    wdata->repaint();
  }
}

void QEWaveForm::closeEvent( QCloseEvent *e )
{
  wdata->killTimers();
  wdata->close();
  e->accept();      // hides the widget
}
