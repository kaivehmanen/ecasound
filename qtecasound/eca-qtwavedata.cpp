// ------------------------------------------------------------------------
// eca-qtwavedata.cpp: Qt-widget for the actual wave data.
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
#include <fstream>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qprogressdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kvutils.h>

#include "audioio-types.h"
#include "samplebuffer.h"
#include "eca-qtwavedata.h"
#include "eca-debug.h"

QEWaveData::QEWaveData(AUDIO_IO_FILE* iod, QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
  startTimer(250);
  //  setMaximumHeight(100);
  setMinimumSize(400, 150);
  ioobject = iod;
  buffersize_save = ioobject->buffersize();
  channels = ioobject->channels();
  waveblocks.resize(channels);
  xposcoord = 0;

  set_wave_color(QColor("royal blue"));
  set_background_color(Qt::white);
  set_position_color(Qt::black);
  set_minmax_color(Qt::black);
  set_zeroline_color(Qt::black);

  if (!ioobject->is_open()) ioobject->open();

  if (ioobject->is_realtime() == true ||
      ioobject->length_in_samples() == 0 ||
      ioobject->io_mode() == si_write) 
    valid_ioobject = false;
  else 
    valid_ioobject = true;

}

void QEWaveData::timerEvent( QTimerEvent * ) {
  if (!valid_ioobject) return;

  newxposcoord = (int)((double)ioobject->position_in_samples() / ioobject->length_in_samples() * width());
  if (newxposcoord != xposcoord) {
    // if updateWaveData was cancelled...
    p.begin(this);
    if (xposcoord * step < static_cast<double>(waveblocks[0].size())) {
      p.setPen(background_color);
      p.drawLine(xposcoord, 0, xposcoord, height());

      p.setPen(wave_color);
      ycoord = height() / 2 / channels;

      for(char ch = 0; ch < channels; ch++) {
	ycoord += (height() / channels) * ch;      
	//	p.drawLine(xposcoord, ycoord - (int)(waveblocks[xposcoord * step].min[ch] / 32767.0 * waveheight),
	//		   xposcoord, ycoord - (int)(waveblocks[xposcoord * step].max[ch] / 32767.0 * waveheight));
	p.drawLine(xposcoord, ycoord - (int)(waveblock_minimum(ch, xposcoord * step, step) / 32767.0 * waveheight),
		   xposcoord, ycoord - (int)(waveblock_maximum(ch, xposcoord * step, step) / 32767.0 * waveheight));
      }
    }
    else {
      p.setPen(background_color);
      p.drawLine(xposcoord, 0, xposcoord, height());
    }

    ycoord = height() / 2 / ioobject->channels();
    for(char ch = 0; ch < ioobject->channels(); ch++) {
      ycoord += (height() / ioobject->channels()) * ch;
      //      p.setPen(position_color);
      p.setPen(minmax_color);
      p.drawPoint(xposcoord, ycoord - waveheight);
      p.drawPoint(xposcoord, ycoord + waveheight);
      p.setPen(zeroline_color);
      p.drawPoint(xposcoord, ycoord);
    }
    
    xposcoord = newxposcoord;
    p.setPen(position_color);
    p.drawLine(xposcoord, 0, xposcoord, height());
    p.end();
    //    repaint(true);
  }
}

void QEWaveData::updateWaveData(bool force)
{
  if (!valid_ioobject) return;
  calculate_buffersize();

  if (!force) {
    load_ews_data();
    if (waveblocks[0].size() > 0) return;
  }

  QProgressDialog progress ("Analyzing wave data...","Cancel",(int)(ioobject->length_in_seconds_exact()*10.0),0,0,true);
  progress.setProgress(0);
  progress.show();

  long save_pos = ioobject->position_in_samples();
  ioobject->seek_position_in_samples(0);
  ioobject->buffersize(buffersize, ioobject->samples_per_second());

  SAMPLE_BUFFER* t = new SAMPLE_BUFFER (buffersize, ioobject->channels());
  QEWaveBlock blocktmp;
  //  waveblocks.resize((int)(ioobject->length_in_samples() / QEWaveData::buffersize));
  vector<QEWaveBlock>::size_type bnum = 0;
  while(!ioobject->finished()) {
    ioobject->read_buffer(t);
    //    emit setProgress(ioobject->position_in_seconds());   
    progress.setProgress((int)(ioobject->position_in_seconds_exact() * 10.0));
    // progress->repaint();
    if (progress.wasCancelled()) break;

    for(char ch = 0; ch < t->number_of_channels() && ch < channels; ch++) {
      blocktmp.max = (int)(32767.0 * t->max_value(ch) / SAMPLE_BUFFER::max_amplitude);
      //      cerr << blocktmp.max[ch];
      //      cerr << "-";
      blocktmp.min = (int)(32767.0 * t->min_value(ch) / SAMPLE_BUFFER::max_amplitude);
      //      cerr << blocktmp.min[ch];
      //      cerr << "<br>\n";
      waveblocks[ch].push_back(blocktmp);
    }


    //    waveblocks[bnum] = blocktmp;
    ++bnum;
    if (bnum >= ioobject->length_in_samples() / buffersize) break;
  }

  ioobject->seek_position_in_samples(save_pos);
  ioobject->buffersize(buffersize, ioobject->samples_per_second());

  save_ews_data();
}

void QEWaveData::paintEvent( QPaintEvent* e )
{
  if (!valid_ioobject) return;
  
  ur = e->rect(); 
  pix.resize(ur.size());
  //  pix.fill( this, ur.topLeft() );     // fill with widget background
  pix.fill(background_color);
  p.begin( &pix );

  //  samples_per_pixel = ioobject->length_in_samples() / width();
  //  ecadebug->msg(2,"eca-qtwaveform::drawWaveform(), samples_per_pixel " + kvu_numtostr(samples_per_pixel));

  //  step = samples_per_pixel / buffersize;
  step = static_cast<double>(waveblocks[0].size()) / width();
  if (step == 0.0) step = 1.0;

  waveheight = height() / ioobject->channels() / 2 - 5;  

  // draw the actual waveform
  //  p.setPen(QColor("steel blue"));
  p.setPen(wave_color);
  bufindex = 0;
  for(xcoord = 0; xcoord < width(); xcoord++) {
    ycoord = height() / 2 / channels;
    for(char ch = 0; ch < channels; ch++) {
      ycoord += (height() / channels) * ch;
      p.drawLine(xcoord, ycoord - (int)(waveblock_minimum(ch, bufindex, step) / 32767.0 * waveheight),
	       xcoord, ycoord - (int)(waveblock_maximum(ch, bufindex, step) / 32767.0 * waveheight));
    }
    bufindex += step;
  }

  // draw max and mix limit lines
  p.setPen(minmax_color);
  ycoord = height() / 2 / ioobject->channels();
  for(char ch = 0; ch < ioobject->channels(); ch++) {
    ycoord += (height() / ioobject->channels()) * ch;
    p.drawLine(0, ycoord - waveheight,
	       width(), ycoord - waveheight);
    p.drawLine(0, ycoord + waveheight,
	       width(), ycoord + waveheight);
    p.drawLine(0, ycoord,
	       width(), ycoord);
  }

  p.setPen(zeroline_color);
  p.drawLine(xposcoord, 0, xposcoord, height());
  
  p.end();
  bitBlt( this, ur.topLeft(), &pix );
}

int QEWaveData::waveblock_minimum(int channel, double from, double step) {
  int f = static_cast<int>(floor(from));
  int s = static_cast<int>(ceil(step));
  if (f >= waveblocks[channel].size()) return(0);
  int minimum = waveblocks[channel][f].min;

  int n = f;
  for(++n; n < f + s; n++) {
    if (n >= waveblocks[channel].size()) break;
    if (waveblocks[channel][n].min < minimum) 
      minimum = waveblocks[channel][n].min;
  }

  return(minimum);
}

int QEWaveData::waveblock_maximum(int channel, double from, double step) {
  int f = static_cast<int>(floor(from));
  int s = static_cast<int>(ceil(step));
  if (f >= waveblocks[channel].size()) return(0);
  int maximum = waveblocks[channel][f].max;

  int n = f;
  for(++n; n < f + s; n++) {
    if (n >= waveblocks[channel].size()) break;
    if (waveblocks[channel][n].max > maximum) 
      maximum = waveblocks[channel][n].max;
  }
  
  return(maximum);
}

void QEWaveData::calculate_buffersize(void) {
  buffersize = 16384;
  while(ioobject->length_in_samples() / buffersize <
	QApplication::desktop()->width()) {
    buffersize /= 2;
    if (buffersize == 0) {
      buffersize = 256; 
      break;
    }
  }
  ecadebug->msg(2, "(eca-qtwavedata) Set buffersize to " + kvu_numtostr(buffersize) + ".");
}

void QEWaveData::load_ews_data(void) { 
  string newfile = ioobject->label() + ".ews";

  struct stat wfile;
  stat(ioobject->label().c_str(), &wfile);
  struct stat ewsfile;
  stat(newfile.c_str(), &ewsfile);

  if (wfile.st_ctime >= ewsfile.st_ctime) {
    for(int n = 0; n < channels; n++) {
      waveblocks[n].resize(0);
    }
    return;
  }

  FILE *f1;
  f1 = fopen(newfile.c_str(), "rb");
  if (!f1) return; 

  int c = fgetc(f1); 
  if (c == 'E') {
    fseek(f1, 8, SEEK_SET);
    int16_t buftemp;

    fread(&buftemp, 1, sizeof(buftemp), f1);
    ecadebug->msg(2,"(eca-qtwavedata) reading cache file with buffersize: " + kvu_numtostr(buffersize));
    fseek(f1, 16, SEEK_SET);

    if (buftemp != buffersize) {
      fclose(f1);
      return;
    }
    waveblocks.resize(ioobject->length_in_samples() / buffersize);
    if (!f1) return;
    for(char ch = 0; ch < channels; ch++) {
      for(int t = 0; t < static_cast<int>(waveblocks[ch].size()); t++) {
	fread(&(waveblocks[ch][t].min), 1, sizeof(waveblocks[ch][t].min), f1);
	fread(&(waveblocks[ch][t].max), 1, sizeof(waveblocks[ch][t].max), f1);
      }
    }
    fclose(f1);
  }
  else {
    fclose(f1);
    load_ews_data_ascii();
  }
}

void QEWaveData::save_ews_data(bool forced) { 

  struct stat wfile;
  stat(ioobject->label().c_str(), &wfile);
  struct stat ewsfile;
  int res = stat(string(ioobject->label() + ".ews").c_str(), &ewsfile);
  if (!forced && wfile.st_size == ewsfile.st_size) {
    return;
  }

  FILE *f1;
  f1 = fopen(string(ioobject->label() + ".ews").c_str(), "wb");
  if (!f1) return;

  char magic[8] = "EWSBIN_";
  fwrite(magic, 8, 1, f1);
  fseek(f1, 8, SEEK_SET);
  int16_t buftemp = buffersize;
  ecadebug->msg(2,"(eca-qtwavedata) writing cache file with buffersize: " + kvu_numtostr(buffersize));
  fwrite(&buftemp, 1, sizeof(buftemp), f1);
  fseek(f1, 8, SEEK_CUR);

    for(char ch = 0; ch < channels; ch++) {
      for(int t = 0; t < static_cast<int>(waveblocks[ch].size()); t++) {
	fwrite(&(waveblocks[ch][t].min), 1, sizeof(waveblocks[ch][t].min), f1);
	fwrite(&(waveblocks[ch][t].max), 1, sizeof(waveblocks[ch][t].max), f1);
    }
  }
  fclose(f1);
}

void QEWaveData::load_ews_data_ascii(void) { 
  string newfile = ioobject->label() + ".ews";

  struct stat wfile;
  stat(ioobject->label().c_str(), &wfile);
  struct stat ewsfile;
  stat(newfile.c_str(), &ewsfile);

  if (wfile.st_ctime >= ewsfile.st_ctime) {
    for(int n = 0; n < channels; n++) {
      waveblocks[n].resize(0);
    }
    return;
  }

  fstream f1;
  f1.open(newfile.c_str(), ios::in);

  int16_t buftemp;
  f1 >> buftemp;
  if (buftemp != buffersize) {
    f1.close();
    return;
  }
  if (!f1) return;

  for(char ch = 0; ch < channels; ch++) {
    for(int t = 0; t < static_cast<int>(waveblocks.size()); t++) {
      waveblocks[ch].resize(ioobject->length_in_samples() / buffersize);
      f1 >> waveblocks[ch][t].min;
      f1 >> waveblocks[ch][t].max;
    }
  }
  f1.close();
}
