// ------------------------------------------------------------------------
// qefile.cpp: Class representing a single file/waveform
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

#include <cstdio>
#include <unistd.h>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>

#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qlabel.h>

#include <ecasound/audioio.h>
#include <ecasound/eca-audio-objects.h>
#include <ecasound/samplebuffer_functions.h>
#include <ecasound/qebuttonrow.h>

#include "qefile.h"

QEFile::QEFile(const string& filename,
	       bool use_wave_cache, 
	       bool force_refresh,
	       QWidget *parent, 
	       const char *name)
        : QWidget( parent, name ),
	  filename_rep(filename),
	  refresh_toggle_rep(force_refresh), 
          wcache_toggle_rep(use_wave_cache) {
  io_object = 0;
  buffersize_rep = 0;
  length_rep = 0;
  channels_rep = 0;
  sample_rate_rep = 0;
  marking_rep = false;
  waveform_count = 0;

  open_io_object();
  calculate_buffersize();
  update_wave_form_data();
  init_layout();
  update_layout();

  // --------
  ENSURE(buffersize_rep != 0);
  ENSURE(io_object != 0);
  ENSURE(sample_rate_rep != 0);
  ENSURE(channels_rep != 0);
  ENSURE(top_layout != 0);
  ENSURE(buttonrow != 0);
  // --------
}

QEFile::QEFile(QWidget *parent, 
	       const char *name)
  : QWidget( parent, name ) {

  io_object = 0;
  init_layout();

  // --------
  ENSURE(top_layout != 0);
  ENSURE(buttonrow != 0);
  // --------
}

void QEFile::new_file(const string& name) {
  if (io_object != 0) delete io_object;
  io_object = 0;
  length_rep = 0;
  channels_rep = 0;
  sample_rate_rep = 0;
  marking_rep = false;
  waveform_count = 0;
  filename_rep = name;

  if (name.empty() != true) {
    open_io_object();
    calculate_buffersize();
    update_wave_form_data();
  }
  update_layout();
  updateGeometry();

  emit_status();
}

void QEFile::emit_status(void) {
  current_position(current_position());
  visible_area(visible_area_start(), visible_area_end());
  marked_area(marked_area_start(), marked_area_end());
}

bool QEFile::eventFilter(QObject *, QEvent *e) {
  if (e->type() == QEvent::MouseButtonPress) {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
      waveforms[n]->current_position_relative(me->x());
    }
    emit current_position_changed(ECA_AUDIO_TIME(coord_to_samples(me->x()), sample_rate_rep));
    last_mousemove_xpos = last_mouse_xpos = me->x();
    //    cerr << "MousePress\n";
    return(true);
  }
  else if (e->type() == QEvent::MouseButtonRelease) {
    if (marking_rep == true) {
      QMouseEvent* me = static_cast<QMouseEvent*>(e);
      mark_area_relative(last_mouse_xpos, me->x());
      marking_rep = false;
    }
    //    cerr << "MouseButtonRelease\n";
    return(true);
  }
  else if (e->type() == QEvent::MouseButtonDblClick) {
    marking_rep = false;
    for(int n = 0; n < static_cast<int>(waveforms.size()); n++) waveforms[n]->toggle_marking(false);
    //    cerr << "MouseButtonDblClick\n";
    return(true);
  }
  else if (e->type() == QEvent::MouseMove) {
    if (marking_rep == false) {
      if (waveforms[0]->is_marked()) unmark();
      marking_rep = true;
    }
    else {
      QMouseEvent* me = static_cast<QMouseEvent*>(e);
      if (me->x() != last_mousemove_xpos) {
	mark_area_relative(last_mouse_xpos, me->x());
      }
      last_mousemove_xpos = me->x();
    }
    //    cerr << "MouseMove\n";
    return(true);
  }
  else 
    return(false);
}

void QEFile::init_layout(void) {
  top_layout = new QVBoxLayout(this);

  buttonrow = new QEButtonRow(this, "buttonrow");
  buttonrow->add_button(new QPushButton("(Z)oom in",buttonrow), 
		       ALT+Key_Z,
		       this, SLOT(zoom_to_marked()));
  buttonrow->add_button(new QPushButton("Zoo(m) out",buttonrow), 
		       ALT+Key_M,
		       this, SLOT(zoom_out()));
  buttonrow->add_button(new QPushButton("Ma(r)k all",buttonrow),
		       ALT+Key_R,
		       this, SLOT(mark_all()));
  buttonrow->add_button(new QPushButton("(U)nmark",buttonrow), 
		       ALT+Key_U,
		       this, SLOT(unmark()));
  buttonrow->add_button(new QPushButton("Redra(w)",buttonrow), 
		       ALT+Key_W,
		       this, SLOT(update_wave_form_data()));
  top_layout->addWidget(buttonrow, 0, Qt::AlignTop);
}

void QEFile::update_layout(void) {
  // --------
  REQUIRE(channels_rep >= 0);
  // --------

  QString cinfo = "Channel ";
  for(int n = 0; n < channels_rep; n++) {
    if (n == static_cast<int>(waveforms.size())) {
      QEWaveForm* w = new QEWaveForm(n, this);
      QLabel* l = new QLabel(cinfo + QString::number(n), this);
      top_layout->addWidget(l);
      top_layout->addWidget(w);
      w->installEventFilter(this);
      w->show();
      waveforms.push_back(w);
      clabels.push_back(l);
    } 
  }
  for(int n = channels_rep; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->close();
    clabels[n]->close();
  }
  waveforms.resize(channels_rep);
  clabels.resize(channels_rep);

  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->update_wave_blocks(&(waveblocks[n]));
  }
  updateGeometry();

  // --------
  ENSURE(static_cast<int>(waveforms.size()) == channels_rep);
  // --------
}

bool QEFile::is_valid(void) const {
  if (io_object == 0 ||
      waveblocks.size() == 0 ||
      waveblocks[0].size() == 0) 
    return(false);

  return(true);
}

void QEFile::current_position(long int samples) {
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->current_position(waveblocks[n].size() * (static_cast<double>(samples) / length_rep));
  }
  emit current_position_changed(ECA_AUDIO_TIME(samples, sample_rate_rep));
}

void QEFile::visible_area(long int startpos_samples, long int endpos_samples) {
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->visible_area(waveblocks[n].size() *
			       (static_cast<double>(startpos_samples) /
				length_rep),
			       waveblocks[n].size() * (static_cast<double>(endpos_samples) / length_rep));
  }
  emit visible_area_changed(ECA_AUDIO_TIME(startpos_samples, sample_rate_rep), 
			    ECA_AUDIO_TIME(endpos_samples, sample_rate_rep));
}

void QEFile::marked_area(long int startpos_samples, int endpos_samples) {
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->marked_area_begin(samples_to_blocks(startpos_samples));
    waveforms[n]->marked_area_end(samples_to_blocks(endpos_samples));
  }
  emit marked_area_changed(ECA_AUDIO_TIME(startpos_samples, sample_rate_rep), 
			   ECA_AUDIO_TIME(endpos_samples, sample_rate_rep));
}

void QEFile::mark_area_relative(int from, int to) {
  int pos1,pos2;
  if (from > to) {
    pos1 = to;
    pos2 = from;
  }
  else {
    pos1 = from;
    pos2 = to;
  }
  if (pos1 < 0) pos1 = 0;

  int rpos1 = pos1;
  int rpos2 = pos2;
  if (last_mousemove_xpos < rpos1) rpos1 = last_mousemove_xpos;
  else if (last_mousemove_xpos > rpos2) rpos2 = last_mousemove_xpos;
  
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->mark_area_relative(pos1,pos2);
    //      cerr << "Mouse area blocks: " << to << " to " << from << ".\n";
    waveforms[n]->toggle_marking(true);
    waveforms[n]->repaint(rpos1, 0, rpos2 - rpos1, waveforms[n]->height(), false);
  }

  emit marked_area_changed(ECA_AUDIO_TIME(coord_to_samples(pos1), sample_rate_rep), 
			   ECA_AUDIO_TIME(coord_to_samples(pos2), sample_rate_rep));
}

long int QEFile::coord_to_samples(int coord) {
  if (waveforms.size() == 0) return(0);
  
  long int preceding_samples = static_cast<long int>(length_rep * (static_cast<double>(waveforms[0]->visible_area_begin()) / waveblocks[0].size()));

  long int visible_blocks = waveforms[0]->visible_area_end() - waveforms[0]->visible_area_begin();
  long int visible_position = static_cast<long int>(visible_blocks * (static_cast<double>(coord) / width()));
  return(static_cast<long int>(preceding_samples + (length_rep * static_cast<double>(visible_position) / waveblocks[0].size())));
}

long int QEFile::blocks_to_samples(long int blocks) {
  if (waveforms.size() == 0) return(0);
  return(static_cast<long int>(length_rep * static_cast<double>(blocks) / waveblocks[0].size()));
}

long int QEFile::samples_to_blocks(long int samples) {
  if (waveforms.size() == 0) return(0);
  return(static_cast<long int>(waveblocks[0].size() * static_cast<double>(samples) / length_rep));
}

void QEFile::mark_all(void) {
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->toggle_marking(true);
    waveforms[n]->marked_area_begin(0);
    waveforms[n]->marked_area_end(samples_to_blocks(length_rep));
    waveforms[n]->repaint(false);
  }
  emit marked_area_changed(ECA_AUDIO_TIME(0, sample_rate_rep), 
			   ECA_AUDIO_TIME(length_rep, sample_rate_rep));
}

void QEFile::unmark(void) {
  //  cerr << "F9\n";
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->toggle_marking(false);
    waveforms[n]->repaint(false);
  }
  emit marked_area_changed(ECA_AUDIO_TIME(0, sample_rate_rep), 
			   ECA_AUDIO_TIME(0, sample_rate_rep));
}

void QEFile::zoom_to_marked(void) { 
  //  cerr << "F10\n";
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->zoom_to_marked();
    emit visible_area_changed(ECA_AUDIO_TIME(blocks_to_samples(waveforms[n]->visible_area_begin()), sample_rate_rep),
			      ECA_AUDIO_TIME(blocks_to_samples(waveforms[n]->visible_area_end()), sample_rate_rep));
  }
}

void QEFile::zoom_out(void) {
  //  cerr << "F11\n";
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->visible_area(0, waveblocks[n].size());
  }
  emit visible_area_changed(ECA_AUDIO_TIME(0, sample_rate_rep),
			    ECA_AUDIO_TIME(length_rep, sample_rate_rep));
}

long int QEFile::current_position(void) const {
  if (waveforms.size() == 0) return(0);
  return(static_cast<long int>(length_rep * (static_cast<double>(waveforms[0]->current_position()) / waveblocks[0].size())));
}

bool QEFile::is_marked(void) const {
 if (waveforms.size() > 0 &&
     waveforms[0]->is_marked() == true) 
   return(true);
 return(false);
}  

long int QEFile::marked_area_start(void) const {
  if (waveforms.size() == 0) return(0);
  if (waveforms[0]->is_marked() == true) {
    double pos = waveforms[0]->marked_area_begin();
    return(static_cast<long int>(length_rep * (pos / waveblocks[0].size())));
  }
  else 
    return(current_position());
}

long int QEFile::marked_area_end(void) const {
  if (waveforms.size() > 0 && waveforms[0]->is_marked() == true) {
    double pos = waveforms[0]->marked_area_end();
    return(static_cast<long int>(length_rep * (pos / waveblocks[0].size())));
  }
  else 
    return(0);
}

long int QEFile::visible_area_start(void) const {
  if (waveforms.size() == 0) return(0);
  double pos = waveforms[0]->visible_area_begin();
  return(static_cast<long int>(length_rep * (pos / waveblocks[0].size())));
}

long int QEFile::visible_area_end(void) const {
  if (waveforms.size() == 0) return(0);
  double pos = waveforms[0]->visible_area_end();
  return(static_cast<long int>(length_rep * (pos / waveblocks[0].size())));
}

QSize QEFile::sizeHint(void) const {
  QSize t;
  if (buttonrow != 0) {
    t.setWidth(buttonrow->width());
    t.setHeight(buttonrow->height());
  }
  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    t += waveforms[n]->sizeHint();
  }

  return(t);
}

void QEFile::open_io_object(void) {
  // --------
  REQUIRE(io_object == 0);
  REQUIRE(filename_rep.empty() == false);
  // --------

  try {
    io_object = ECA_AUDIO_OBJECTS::create_audio_object(filename_rep);
    if (io_object == 0) {
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      QString errormsg ("Can't open file \"");
      errormsg += QString(filename_rep.c_str()) + "\", unknown audio format.";
      mbox->information(this, "ecawave", errormsg,0);
      filename_rep = "";
    }
    else {
      io_object->io_mode(AUDIO_IO::io_read);
      io_object->set_audio_format(aformat);
      io_object->buffersize(max_buffer_size, io_object->samples_per_second());
      io_object->open();
      channels_rep = io_object->channels();
      length_rep = io_object->length_in_samples();
      sample_rate_rep = io_object->samples_per_second();
      io_object->close();
    }
  }
  catch(ECA_ERROR* e) {
    if (e->error_action() != ECA_ERROR::stop) {
      io_object = 0;
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      QString errormsg ("Can't open file \"");
      errormsg += QString(filename_rep.c_str()) + "\": ";
      errormsg += QString(e->error_msg().c_str());
      mbox->information(this, "ecawave", errormsg,0);
    }
    else throw;
  }
}

void QEFile::update_wave_form_data(void) {
  if (io_object == 0 ||
      buffersize_rep > 0) return;

  io_object->open();
  if (length_rep != io_object->length_in_samples()) length_rep = io_object->length_in_samples();
  waveblocks.resize(0);
  if (wcache_toggle_rep == true &&
      refresh_toggle_rep == false && 
      check_ews_data() == true) {
    load_ews_data();
  }
  if (is_valid() == false) {
    waveblocks.resize(io_object->channels());

    long start_pos = 0;
    long end_pos = length_rep;

    QProgressDialog progress ("Analyzing wave data...","Cancel",(int)((end_pos - start_pos) / 1000),0,0,true);
    progress.setProgress(0);
    progress.show();
    
    io_object->buffersize(buffersize_rep, io_object->samples_per_second());

    SAMPLE_BUFFER* t = new SAMPLE_BUFFER (buffersize_rep, 
					  io_object->channels(), 
					  io_object->samples_per_second());
    QEWaveBlock blocktmp;

    io_object->seek_position_in_samples(0);
    long int bnum = 0;
    while(io_object->finished() == false &&
	  io_object->position_in_samples() <= end_pos) {
      io_object->read_buffer(t);
      progress.setProgress((int)((io_object->position_in_samples() - start_pos) / 1000));
      if (progress.wasCancelled()) {
	for(int ch = 0; ch < t->number_of_channels() && ch < io_object->channels(); ch++)
	  waveblocks[ch].resize(0);
	break;
      }

      for(int ch = 0; ch < t->number_of_channels() && ch < io_object->channels(); ch++) {
	blocktmp.max = (int)(32767.0 * SAMPLE_BUFFER_FUNCTIONS::max_value(*t, ch) / SAMPLE_SPECS::max_amplitude);
	//	cerr << blocktmp.max << "-"
	blocktmp.min = (int)(32767.0 * SAMPLE_BUFFER_FUNCTIONS::min_value(*t, ch) / SAMPLE_SPECS::max_amplitude);
	//	cerr << blocktmp.min << endl;
	waveblocks[ch].push_back(blocktmp);
      }
    
      //    waveblocks[bnum] = blocktmp;
      ++bnum;
      if (bnum >= length_rep / buffersize_rep) break;
    }

    if (wcache_toggle_rep == true) save_ews_data(true);
  }
  io_object->close();

  for(int n = 0; n < static_cast<int>(waveforms.size()); n++) {
    waveforms[n]->update_wave_blocks(&(waveblocks[n]));
  }
  if (refresh_toggle_rep == true) refresh_toggle_rep = false;

  // --------
  ENSURE(io_object->is_open() == false);
  // --------
}

void QEFile::calculate_buffersize(void) {
  //  cerr << "F17\n";
  // --------
  REQUIRE(io_object != 0);
  REQUIRE(io_object->is_open() == false);
  // --------

  buffersize_rep = max_buffer_size;
  io_object->open();
  while(length_rep / buffersize_rep < QApplication::desktop()->width()) {
    buffersize_rep /= 2;
    if (buffersize_rep < 2) {
      buffersize_rep = 2; 
      break;
    }
  }
  io_object->close();

  // --------
  ENSURE(buffersize_rep != 0);
  ENSURE(io_object->is_open() == false);
  // --------
}

bool QEFile::check_ews_data(void) { 
  //  cerr << "F18\n";
  string nEWWaveForm = io_object->label() + ".ews";

  struct stat wfile;
  stat(io_object->label().c_str(), &wfile);
  struct stat ewsfile;
  stat(nEWWaveForm.c_str(), &ewsfile);
  
  if (wfile.st_mtime > ewsfile.st_mtime) {
   for(int n = 0; n < static_cast<int>(waveblocks.size()); n++) {
      waveblocks[n].resize(0);
    }
    return(false);
  }

  return(true);
}

void QEFile::load_ews_data(void) { 
  //  cerr << "F19\n";
  string nEWWaveForm = io_object->label() + ".ews";

  FILE *f1;
  f1 = fopen(nEWWaveForm.c_str(), "rb");
  if (!f1) return;

  int c = fgetc(f1);
  if (c == 'E') {
    fseek(f1, 8, SEEK_SET);
    int16_t buftemp;
    
    fread(&buftemp, 1, sizeof(buftemp), f1);
    fseek(f1, 16, SEEK_SET);

    if (buftemp != buffersize_rep) {
      fclose(f1);
      return;
    }

    waveblocks.resize(io_object->channels(), vector<QEWaveBlock>
		      (length_rep / buffersize_rep));

    if (!f1) return;
    for(char ch = 0; ch < static_cast<int>(waveblocks.size()); ch++) {
      for(int t = 0; t < static_cast<int>(waveblocks[ch].size()); t++) {
	fread(&(waveblocks[ch][t].min), 1, sizeof(waveblocks[ch][t].min), f1);
	fread(&(waveblocks[ch][t].max), 1, sizeof(waveblocks[ch][t].max), f1);
      }
    }
    fclose(f1);
  }
  else {
    fclose(f1);
  }
  //  cerr << "Read " << waveblocks[0].size() << " samples.\n";
}

void QEFile::save_ews_data(bool forced) { 
  //  cerr << "F20\n";
  struct stat wfile;
  stat(io_object->label().c_str(), &wfile);
  struct stat ewsfile;
  int res = stat(string(io_object->label() + ".ews").c_str(), &ewsfile);
  if (!forced && wfile.st_size == ewsfile.st_size) {
    return;
  }
  
  FILE *f1;
  f1 = fopen(string(io_object->label() + ".ews").c_str(), "wb");
  if (!f1) return;
  
  char magic[8] = "EWSBIN_";
  fwrite(magic, 8, 1, f1);
  fseek(f1, 8, SEEK_SET);
  int16_t buftemp = buffersize_rep;
  fwrite(&buftemp, 1, sizeof(buftemp), f1);
  fseek(f1, 16, SEEK_SET);
  
  for(char ch = 0; ch < static_cast<int>(waveblocks.size()); ch++) {
    for(int t = 0; t < static_cast<int>(waveblocks[ch].size()); t++) {
      fwrite(&(waveblocks[ch][t].min), 1, sizeof(waveblocks[ch][t].min), f1);
      fwrite(&(waveblocks[ch][t].max), 1, sizeof(waveblocks[ch][t].max), f1);
    }
  }
  fclose(f1);
}
