// ------------------------------------------------------------------------
// qesignallevel.cpp: Qt-based class for signallevel monitors.
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

#include <string>
#include <vector>

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qrect.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kvutils.h>

#include "samplebuffer_functions.h"
#include "eca-chain.h"

#include "qesignallevel.h"

QESignalLevel::QESignalLevel(int buffer_latency, QWidget *parent, const char *name) 
  : QEChainOperator(parent, name),
    buffer_latency_rep(buffer_latency) {
  channels_rep = 0;
  samples_shown = samples_processed = sample_processing_step = 0;
  startTimer(30);
  update_geometry_request = false;
}

QSize QESignalLevel::sizeHint(void) const {
  return(QSize(300, 5 * channels_rep));
}

void QESignalLevel::timerEvent( QTimerEvent * ) {
  if (try_lock_object() == true) {
    //    cerr << "C1-1" << endl;
    if (samples_shown + buffer_latency_rep * sample_processing_step < samples_processed) {
      if (update_geometry_request == true) {
	update_geometry_request = false;
	resize(sizeHint());
	updateGeometry();
      }
      while(samples_shown + buffer_latency_rep *
	    sample_processing_step < samples_processed) {
	samples_shown += sample_processing_step;
	repaint(false);
      }
    }
    unlock_object();
    //    cerr << "C1-2"  << ",samples-shown: " << samples_shown << endl;
  }
}

void QESignalLevel::paintEvent(QPaintEvent* e) {
  QRect ur (e->rect());
  QPixmap pix(ur.size());
  pix.fill(this, ur.topLeft());     // fill with widget background
  QPainter p;
  p.begin(&pix);
  p.setBrush(red);
  p.translate(-ur.x(), -ur.y());    // use widget coordinate system
                                    // when drawing on pixmap
  int meter_size = height() / channels_rep;

  for(int n = 0; n < channels_rep && rms_volume[n].size() > 0; n++) {
    p.drawRect(0, n * meter_size, rms_volume[n].front() * width(), (n + 1) * meter_size);
    rms_volume[n].pop_front();
  }
  p.end();
  bitBlt(this, ur.topLeft(), &pix);
}

void QESignalLevel::init(SAMPLE_BUFFER *insample) { 
  buffer_rep = insample;
  channels_rep = insample->number_of_channels();
  sample_processing_step = insample->length_in_samples();
    
  rms_volume.resize(channels_rep,
		    deque<CHAIN_OPERATOR::parameter_type>
		    (0));
  update_geometry_request = true;
}

void QESignalLevel::process(void) {
  lock_object();
  //  cerr << "C2-1" << endl;
  for(int n = 0; n < channels_rep; n++) {
    rms_volume[n].push_back(SAMPLE_BUFFER_FUNCTIONS::RMS_volume(*buffer_rep,
								n,
								buffer_rep->length_in_samples()));
  }
  samples_processed += buffer_rep->length_in_samples();
  unlock_object();
  //  cerr << "C2-2, rmsvolume_size:" << rms_volume[0].size() << ", samples: " << samples_processed << endl;
}
