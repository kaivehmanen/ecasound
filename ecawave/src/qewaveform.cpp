// ------------------------------------------------------------------------
// qewaveform.cpp: Class representing a one channel waveform widget
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

#include <cmath>

#include "qewaveform.h"

QEWaveForm::QEWaveForm (int channel,
			QWidget *parent, 
			const char *name) 
  : QWidget(parent,name),
    channel_rep(channel) {

  startTimer(250);
  waveblock = 0;
  marked_rep = false;

  update_wave_blocks(&empty_blocks);

  current_position_rep = 0;
  marked_area_begin_rep = 0;
  marked_area_end_rep = 0;
  visible_area_begin_rep = 0;
  visible_area_end_rep = 0;

  prev_xpos = 0;
  xpos = 0;

  set_wave_color(QColor("royal blue"));
  set_background_color(Qt::white);
  set_position_color(Qt::black);
  set_minmax_color(Qt::gray);
  set_marked_color(Qt::red);
  set_marked_background_color(Qt::black);
  set_marked_position_color(Qt::white);
  set_zeroline_color(QColor("royal blue"));
}

void QEWaveForm::current_position(long int blocks) { 
  // --------
  REQUIRE(waveblock != 0);
  REQUIRE(blocks >= 0);
  // --------

  current_position_rep = blocks;
  if (current_position_rep >= visible_area_begin_rep &&
      current_position_rep <= visible_area_end_rep) {
    double pos = current_position_rep - visible_area_begin_rep;
    pos /= (visible_area_end_rep - visible_area_begin_rep);
    pos *= width();
    xpos = pos;
  }
  else {
    xpos = 0;
  }
  //  cerr << "cur_pos_rep[1]: " << current_position_rep << ", xpos: " <<   xpos << ".\n";

  repaint_current_position();
}

void QEWaveForm::current_position_relative(int xpos_coord) { 
  // --------
  REQUIRE(waveblock != 0);
  REQUIRE(xpos_coord >= 0);
  // --------

  xpos = xpos_coord;
  double pos = visible_area_begin_rep;
  pos += (static_cast<double>(xpos) / width()) * (visible_area_end_rep - visible_area_begin_rep);
  current_position_rep = pos;

  //  cerr << "cur_pos_rep[2]: " << current_position_rep << ", xpos: " <<  xpos << ".\n";

  repaint_current_position();
}

void QEWaveForm::repaint_current_position(void) { 
  // --------
  REQUIRE(waveblock != 0);
  // --------

  int waveheight = height() / 2 - 5;
  int half_height = height() / 2;

  QPainter p (this);
  if (xpos != prev_xpos) {

    p.setPen(background_color);
    p.drawLine(prev_xpos, 0, prev_xpos, height());
  
  
    if (prev_inside_marked == true &&
	marked_rep == true) {
      p.setPen(marked_position_color);
      p.drawLine(xpos, 0, xpos, height());

      p.setPen(marked_background_color);
      p.drawLine(prev_xpos, 0,
		 prev_xpos, height());
      p.setPen(marked_color);
    }
    else {
      p.setPen(position_color);
      p.drawLine(xpos, 0, xpos, height());

      p.setPen(wave_color);
    }

    p.drawLine(prev_xpos, half_height - prev_xpos_minimum,
              prev_xpos, half_height - prev_xpos_maximum);

    p.setPen(minmax_color);
    p.drawLine(0, half_height - waveheight,
	       width(), half_height - waveheight);
    p.drawLine(0, half_height + waveheight,
	       width(), half_height + waveheight);
    
    p.setPen(zeroline_color);
    p.drawLine(0, half_height, width(), half_height);
  }
  p.end();

  if (current_position_rep >= marked_area_begin_rep &&
      current_position_rep <= marked_area_end_rep)
    prev_inside_marked = true;
  else
    prev_inside_marked = false;

  prev_xpos = xpos;
  prev_xpos_minimum = waveblock_minimum(current_position_rep, step) / 32767.0 * waveheight;
  prev_xpos_maximum = waveblock_maximum(current_position_rep, step) / 32767.0 * waveheight;
}

void QEWaveForm::marked_area_begin(long int blocks) { 
  marked_area_begin_rep = blocks; 
}

void QEWaveForm::marked_area_end(long int blocks) { 
  marked_area_end_rep = blocks; 
}

void QEWaveForm::mark_area_relative(int from, int to) {
  double pos1 = visible_area_begin_rep;
  pos1 += (static_cast<double>(from) / width()) * (visible_area_end_rep - visible_area_begin_rep);
  marked_area_begin_rep = pos1; 

  pos1 = visible_area_begin_rep;
  pos1 += (static_cast<double>(to) / width()) * (visible_area_end_rep - visible_area_begin_rep);
  marked_area_end_rep = pos1;
}

void QEWaveForm::visible_area(long int start_blocks, long int end_blocks) { 
  visible_area_begin_rep = start_blocks; 
  visible_area_end_rep = end_blocks; 
  repaint(true);
}

void QEWaveForm::zoom_to_marked(void) {
  visible_area_begin_rep = marked_area_begin_rep;
  visible_area_end_rep = marked_area_end_rep;
  repaint(true);
}

QSize QEWaveForm::sizeHint(void) const {
  return(QSize(600,200));
}

void QEWaveForm::update_wave_blocks(const vector<QEWaveBlock>* block) {
  // --------
  REQUIRE(block != 0);
  // --------

  waveblock = block;
  visible_area_begin_rep = 0;
  visible_area_end_rep = waveblock->size();
  repaint(0, 0, width(), height(), true);
}

void QEWaveForm::paintEvent(QPaintEvent* e) {
  // --------
  REQUIRE(waveblock != 0);
  // --------

  QRect ur = e->rect(); 
  
  //  cerr << "RP-event: " << ur.left() << "," << ur.right() << "," << ur.width() << ".\n";

  QPixmap pix(ur.size());
  pix.fill(this, ur.topLeft());     // fill with widget background
  QPainter p (&pix);
  p.translate( -ur.x(), -ur.y());

  // pix.fill(background_color);
  //  p.begin( &pix );

  //  step = static_cast<double>(waveblock->size()) / width();
  step = static_cast<double>(visible_area_end_rep - visible_area_begin_rep) / width();
  if (step == 0.0) step = 1.0;

  long pos = static_cast<double>(width()) * current_position_rep / waveblock->size();
  //  cerr << "New position (pixels): " << pos << ".\n";

  int waveheight = height() / 2 - 5;
  int half_height = height() / 2;

  p.setPen(wave_color);
  //  double bufindex = visible_area_begin_rep;
  double bufindex = visible_area_begin_rep;
  bufindex += (static_cast<double>(ur.left()) / width()) * (visible_area_end_rep - visible_area_begin_rep);

  for(int xcoord = ur.left(); xcoord <= ur.right(); xcoord++) {
    //    cerr << "D" << xcoord << ".";
    if (xcoord == pos) {
      // --
      // draw the current pointer
      // --
      if (current_position_rep >= marked_area_begin_rep &&
	  current_position_rep <= marked_area_end_rep)
	prev_inside_marked = true;
      else
	prev_inside_marked = false;

      if (prev_inside_marked == true) 
	p.setPen(marked_position_color);
      else
	p.setPen(position_color);
      p.drawLine(xcoord, 0, xcoord, height());
      prev_xpos = xcoord;
      prev_xpos_minimum = waveblock_minimum(bufindex, step) / 32767.0 * waveheight;
      prev_xpos_maximum = waveblock_maximum(bufindex, step) / 32767.0 * waveheight;
    }
    else {
      // --
      // otherwise draw the normal minmax line
      // --
      if (bufindex >= marked_area_begin_rep &&
	  bufindex <= marked_area_end_rep &&
	  marked_rep == true) {
	p.setPen(marked_background_color);
	p.drawLine(xcoord, 0,
		   xcoord, height());
	p.setPen(marked_color);
      }
      else {
	p.setPen(background_color);
	p.drawLine(xcoord, 0,
		   xcoord, height());
	p.setPen(wave_color);
      }
      p.drawLine(xcoord, half_height - (int)(waveblock_minimum(bufindex, step) / 32767.0 * waveheight),
		 xcoord, half_height - (int)(waveblock_maximum(bufindex, step) / 32767.0 * waveheight));
    }
    bufindex += step;
    if (bufindex > visible_area_end_rep) break;
  }

  p.setPen(minmax_color);
  p.drawLine(0, half_height - waveheight,
	     width(), half_height - waveheight);
  p.drawLine(0, half_height + waveheight,
	     width(), half_height + waveheight);
  p.drawLine(0, half_height,
	     width(), half_height);

  p.setPen(zeroline_color);
  p.drawLine(0, half_height, width(), half_height);

  p.end();
  //  p.drawPixmap(ur.topLeft(), pix);
  bitBlt( this, ur.topLeft(), &pix );
}

int QEWaveForm::waveblock_minimum(double from, double step) {
  int f = static_cast<int>(floor(from));
  int s = static_cast<int>(ceil(step));
  if (f >= waveblock->size()) return(0);
  int minimum = (*waveblock)[f].min;

  int n = f;
  for(++n; n < f + s; n++) {
    if (n >= (*waveblock).size()) break;
    if ((*waveblock)[n].min < minimum) 
      minimum = (*waveblock)[n].min;
  }

  return(minimum);
}

int QEWaveForm::waveblock_maximum(double from, double step) {
  int f = static_cast<int>(floor(from));
  int s = static_cast<int>(ceil(step));
  if (f >= (*waveblock).size()) return(0);
  int maximum = (*waveblock)[f].max;
  
  int n = f;
  for(++n; n < f + s; n++) {
    if (n >= (*waveblock).size()) break;
    if ((*waveblock)[n].max > maximum) 
      maximum = (*waveblock)[n].max;
  }
  
  return(maximum);
}

