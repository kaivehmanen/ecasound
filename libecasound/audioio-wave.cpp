// ------------------------------------------------------------------------
// audioio-wave.cpp: RIFF WAVE audio file input/output.
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
#include <cstring>
#include <cmath>

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "samplebuffer.h"
#include "audioio-wave.h"

#include "eca-fileio-mmap.h"
#include "eca-fileio-stream.h"

#include "eca-error.h"
#include "eca-debug.h"

WAVEFILE::WAVEFILE (const string& name, bool double_buffering) {
  label(name);
  double_buffering_rep = double_buffering;
  fio_repp = 0;
}

WAVEFILE::~WAVEFILE(void) {
  ecadebug->msg(ECA_DEBUG::user_objects,"(file-io) Closing file " + label());
  //  fclose(fobject);
  close();
}

void WAVEFILE::format_query(void) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(!is_open());
  // --------

  if (io_mode() == io_write) return;

  fio_repp = new ECA_FILE_IO_STREAM();
  if (fio_repp == 0) {
    throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
  }
  fio_repp->open_file(label(), "rb", false);
  if (fio_repp->file_mode() != "") {
    set_length_in_bytes();
    read_riff_fmt();     // also sets format()
    find_riff_datablock();
    fio_repp->close_file();
  }
  delete fio_repp;
  fio_repp = 0;

  // -------
  // ensure:
  assert(!is_open());
  assert(fio_repp == 0);
  // -------
}


void WAVEFILE::open(void) throw(ECA_ERROR*) {
  switch(io_mode()) {
  case io_read:
    {
      if (double_buffering_rep) fio_repp = new ECA_FILE_IO_MMAP();
      else  fio_repp = new ECA_FILE_IO_STREAM();
      if (fio_repp == 0) {
	throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
      }
      fio_repp->open_file(label(), "rb");
      //      fobject=fopen(label().c_str(),"rb");
      read_riff_header();
      read_riff_fmt();     // also sets format()
      set_length_in_bytes();
      find_riff_datablock();
      break;
    }
  case io_write:
    {
      fio_repp = new ECA_FILE_IO_STREAM();
      if (fio_repp == 0) {
	throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
      }
      fio_repp->open_file(label(), "w+b");
      write_riff_header();
      write_riff_fmt();
      write_riff_datablock();
      break;
    }

  case io_readwrite:
    {
      fio_repp = new ECA_FILE_IO_STREAM();
      if (fio_repp == 0) {
	throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
      }
      fio_repp->open_file(label(), "r+b", false);
      if (fio_repp->file_mode() != "") {
	set_length_in_bytes();
	read_riff_fmt();     // also sets format()
	find_riff_datablock();
      }
      else {
	fio_repp->open_file(label(), "w+b", true);
	write_riff_header();
	write_riff_fmt();
	write_riff_datablock();
      }
    }
  }

  if (riff_format_rep.bits > 8 && 
      format_string()[0] == 'u')
    throw(new ECA_ERROR("AUDIOIO-WAVE", "unsigned sample format accepted only with 8bit."));

  if (riff_format_rep.bits > 8 && 
      format_string().size() > 4 &&
      format_string()[4] == 'b')
    throw(new ECA_ERROR("AUDIOIO-WAVE", "bigendian byte-order not supported by RIFF wave files."));

  toggle_open_state(true);
  seek_position();
}

void WAVEFILE::close(void) {
  if (is_open() && fio_repp != 0) {
    update();
    fio_repp->close_file();
    delete fio_repp;
    fio_repp = 0;
  }
  toggle_open_state(false);
}

void WAVEFILE::update (void) {
  if (io_mode() != io_read) {
    update_riff_datablock();
    write_riff_header();
    set_length_in_bytes();
  }
}

void WAVEFILE::find_riff_datablock (void) throw(ECA_ERROR*) {
  if (find_block("data")==-1) {
    throw(new ECA_ERROR("AUDIOIO-WAVE", "no RIFF data block found", ECA_ERROR::retry));
  }
  data_start_position_rep = fio_repp->get_file_position();
}

void WAVEFILE::read_riff_header (void) throw(ECA_ERROR*) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(program flow: read_riff_header())");
   
  fio_repp->read_to_buffer(&riff_header_rep, sizeof(riff_header_rep));

  //  fread(&riff_header_rep,1,sizeof(riff_header_rep),fobject);
  if (memcmp("RIFF",riff_header_rep.id,4) != 0 ||
      memcmp("WAVE",riff_header_rep.wname,4) != 0) {
    throw(new ECA_ERROR("AUDIOIO-WAVE", "invalid RIFF-header", ECA_ERROR::stop));
  }
}

void WAVEFILE::write_riff_header (void) throw(ECA_ERROR*) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(program flow: write_riff_header())");

  long int savetemp = fio_repp->get_file_position();
    
  memcpy(riff_header_rep.id,"RIFF",4);
  memcpy(riff_header_rep.wname,"WAVE",4);
  riff_header_rep.size = fio_repp->get_file_length();

  fio_repp->set_file_position(0);
  //  fseek(fobject,0,SEEK_SET);

  fio_repp->write_from_buffer(&riff_header_rep, sizeof(riff_header_rep));
  //  fwrite(&riff_header_rep,1,sizeof(riff_header_rep),fobject);
  if (memcmp("RIFF",riff_header_rep.id,4) != 0 || 
      memcmp("WAVE",riff_header_rep.wname,4) != 0)
    throw(new ECA_ERROR("AUDIOIO-WAVE", "invalid RIFF-header", ECA_ERROR::stop));

  char temp[16];
  memcpy(temp, "Riff ID: ", 9);
  memcpy(&(temp[9]), riff_header_rep.id, 4);
  temp[13] = 0;
  ecadebug->msg(ECA_DEBUG::user_objects,  string(temp));

  ecadebug->msg(ECA_DEBUG::user_objects, "Wave data size " + 	kvu_numtostr(riff_header_rep.size));
  memcpy(temp, "Riff type: ", 11);
  memcpy(&(temp[11]), riff_header_rep.wname, 4);
  temp[15] = 0;
  ecadebug->msg(ECA_DEBUG::user_objects, "Riff type " + string(temp));

  //  fseek(fobject,save,SEEK_SET);
  fio_repp->set_file_position(savetemp);
}

void WAVEFILE::read_riff_fmt(void) throw(ECA_ERROR*)
{
  ecadebug->msg(ECA_DEBUG::user_objects, "(program flow: read_riff_fmt())");

  long int savetemp = fio_repp->get_file_position();    

  if (find_block("fmt ")==-1)
    throw(new ECA_ERROR("AUDIOIO-WAVE", "no riff fmt-block found",  ECA_ERROR::stop));
  else {
    fio_repp->read_to_buffer(&riff_format_rep, sizeof(riff_format_rep));
    //    fread(&riff_format_rep,1,sizeof(riff_format_rep),fobject);

    if (riff_format_rep.format != 1 &&
	riff_format_rep.format != 3) {
      throw(new ECA_ERROR("AUDIOIO-WAVE", "Only WAVE_FORMAT_PCM and
                           WAVE_FORMAT_IEEE_FLOAT are supported."));
    }

    set_samples_per_second(riff_format_rep.srate);
    set_channels(riff_format_rep.channels);
    if (riff_format_rep.bits == 32) {
      if (riff_format_rep.format == 3)
	set_sample_format(ECA_AUDIO_FORMAT::sfmt_f32_le);
      else
	set_sample_format(ECA_AUDIO_FORMAT::sfmt_s32_le);
    }
    else if (riff_format_rep.bits == 24)
      set_sample_format(ECA_AUDIO_FORMAT::sfmt_s24_le);
    else if (riff_format_rep.bits == 16)
      set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
    else if (riff_format_rep.bits == 8)
      set_sample_format(ECA_AUDIO_FORMAT::sfmt_u8);
    else 
      throw(new ECA_ERROR("AUDIOIO-WAVE", "Sample format not supported."));
  }

  fio_repp->set_file_position(savetemp);
}

void WAVEFILE::write_riff_fmt(void)
{
  RB fblock;

  fio_repp->set_file_position_end();

  riff_format_rep.channels = channels();
  riff_format_rep.bits = bits();
  riff_format_rep.srate = samples_per_second();
  riff_format_rep.byte_second = bytes_per_second();
  riff_format_rep.align = frame_size();
  if (sample_format() == sfmt_f32 ||
      sample_format() == sfmt_f32_le ||
      sample_format() == sfmt_f32_be ||
      sample_format() == sfmt_f64 ||
      sample_format() == sfmt_f64_le ||
      sample_format() == sfmt_f64_be)
    riff_format_rep.format = 3;     // WAVE_FORMAT_IEEE_FLOAT 0x0003 (Microsoft IEEE754 range [-1, +1))
  else
    riff_format_rep.format = 1;     // WAVE_FORMAT_PCM (0x0001) Microsoft Pulse Code
                                    // Modulation (PCM) format

  memcpy(fblock.sig, "fmt ", 4);
  fblock.bsize=16;

  fio_repp->write_from_buffer(&fblock, sizeof(fblock));
  fio_repp->write_from_buffer(&riff_format_rep, sizeof(riff_format_rep));
  ecadebug->msg(ECA_DEBUG::user_objects, "Wrote RIFF format header.");
}

void WAVEFILE::write_riff_datablock(void) {
  RB fblock;

  ecadebug->msg(ECA_DEBUG::user_objects, "(program flow: write_riff_datablock())");
    
  fio_repp->set_file_position_end();

  memcpy(fblock.sig,"data",4);
  fblock.bsize = 0;
  fio_repp->write_from_buffer(&fblock, sizeof(fblock));
  data_start_position_rep = fio_repp->get_file_position();
}

void WAVEFILE::update_riff_datablock(void) {
  RB fblock;
  ecadebug->msg(ECA_DEBUG::user_objects, "(program flow: update_riff_datablock())");
    
  memcpy(fblock.sig,"data",4);

  find_block("data");
  long int savetemp = fio_repp->get_file_position();

  fio_repp->set_file_position_end();
  fblock.bsize = fio_repp->get_file_position() - savetemp;

  savetemp = savetemp - sizeof(fblock);
  if (savetemp > 0) {
    fio_repp->set_file_position(savetemp);
    fio_repp->write_from_buffer(&fblock, sizeof(fblock));
  }
}

bool WAVEFILE::next_riff_block(RB *t, unsigned long int *offtmp)
{
  ecadebug->msg(ECA_DEBUG::user_objects, "(program flow: next_riff_block())");

  fio_repp->read_to_buffer(t, sizeof(RB));
  if (fio_repp->file_bytes_processed() != sizeof(RB)) {
    ecadebug->msg(ECA_DEBUG::user_objects, "invalid RIFF block!");
    return(false);
  }
    
  if (!fio_repp->is_file_ready()) return(false);
  *offtmp = t->bsize + fio_repp->get_file_position();
  return (true);
}

signed long int WAVEFILE::find_block(const char* fblock) {
  unsigned long int offset;
  RB block;

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-wave) find_block(): " + string(fblock,4));
    
  fio_repp->set_file_position(sizeof(riff_header_rep));
  while(next_riff_block(&block,&offset)) {
    ecadebug->msg(ECA_DEBUG::user_objects, "AUDIOIO-WAVE: found RIFF-block ");
    if (memcmp(block.sig,fblock,4) == 0) {
      return(block.bsize);
    }
    fio_repp->set_file_position(offset);
  }

  return(-1);
}

bool WAVEFILE::finished(void) const {
 if (fio_repp->is_file_error() ||
     !fio_repp->is_file_ready()) 
   return true;

 return false;
}

long int WAVEFILE::read_samples(void* target_buffer, long int samples)
{
  // --------
  // require:
  assert(samples > 0);
  assert(target_buffer != 0);
  // --------
  fio_repp->read_to_buffer(target_buffer, frame_size() * samples);
  return(fio_repp->file_bytes_processed() / frame_size());
}

void WAVEFILE::write_samples(void* target_buffer, long int samples) {
  // --------
  // require:
  assert(samples >= 0);
  assert(target_buffer != 0);
  // --------
  fio_repp->write_from_buffer(target_buffer, frame_size() * samples);
}

void WAVEFILE::seek_position(void) {
  if (is_open())
    fio_repp->set_file_position(data_start_position_rep + position_in_samples() * frame_size());
}

void WAVEFILE::set_length_in_bytes(void) {
  long int savetemp = fio_repp->get_file_position();

  find_block("data");
  long int t = fio_repp->get_file_position();

  fio_repp->set_file_position_end();
  t = fio_repp->get_file_position() - t;
  length_in_samples(t / frame_size());
  MESSAGE_ITEM mitem;
  mitem << "(audioio-wave) data length " << t << "bytes.";
  ecadebug->msg(ECA_DEBUG::user_objects, mitem.to_string());

  fio_repp->set_file_position(savetemp);
}

