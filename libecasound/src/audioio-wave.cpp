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

#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio-wave_impl.h"
#include "audioio-wave.h"

#include "eca-fileio-mmap.h"
#include "eca-fileio-stream.h"

#include "eca-error.h"
#include "eca-debug.h"

WAVEFILE::WAVEFILE (const string& name, const SIMODE mode, const
		    ECA_AUDIO_FORMAT& fmt, bool double_buffering) 
  :  AUDIO_IO_FILE(name, mode, fmt)
{
  double_buffering_rep = double_buffering;
  fio = 0;
  format_query();
}

WAVEFILE::~WAVEFILE(void) {
  ecadebug->msg(1,"(file-io) Closing file " + label());
  //  fclose(fobject);
  close();
}

void WAVEFILE::format_query(void) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(!is_open());
  // --------

  if (io_mode() == si_write) return;

  fio = new ECA_FILE_IO_STREAM();
  if (fio == 0) {
    throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
  }
  fio->open_file(label(), "rb", false);
  if (fio->file_mode() != "") {
    set_length_in_bytes();
    read_riff_fmt();     // also sets format()
    find_riff_datablock();
    fio->close_file();
  }
  delete fio;
  fio = 0;

  // -------
  // ensure:
  assert(!is_open());
  assert(fio == 0);
  // -------
}


void WAVEFILE::open(void) throw(ECA_ERROR*) {
  switch(io_mode()) {
  case si_read:
    {
      if (double_buffering_rep) fio = new ECA_FILE_IO_MMAP();
      else  fio = new ECA_FILE_IO_STREAM();
      if (fio == 0) {
	throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
      }
      fio->open_file(label(), "rb");
      //      fobject=fopen(label().c_str(),"rb");
      read_riff_header();
      read_riff_fmt();     // also sets format()
      set_length_in_bytes();
      find_riff_datablock();
      break;
    }
  case si_write:
    {
      fio = new ECA_FILE_IO_STREAM();
      if (fio == 0) {
	throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
      }
      fio->open_file(label(), "w+b");
      write_riff_header();
      write_riff_fmt();
      write_riff_datablock();
      break;
    }

  case si_readwrite:
    {
      fio = new ECA_FILE_IO_STREAM();
      if (fio == 0) {
	throw(new ECA_ERROR("AUDIOIO-WAVE", "Critical error when opening file.", ECA_ERROR::stop));
      }
      fio->open_file(label(), "r+b", false);
      if (fio->file_mode() != "") {
	set_length_in_bytes();
	read_riff_fmt();     // also sets format()
	find_riff_datablock();
      }
      else {
	fio->open_file(label(), "w+b", true);
	write_riff_header();
	write_riff_fmt();
	write_riff_datablock();
      }
    }
  }
  toggle_open_state(true);
  seek_position();
}

void WAVEFILE::close(void) {
  if (is_open() && fio != 0) {
    update();
    fio->close_file();
    delete fio;
    fio = 0;
  }
  toggle_open_state(false);
}

void WAVEFILE::update (void) {
  if (io_mode() != si_read) {
    update_riff_datablock();
    write_riff_header();
    set_length_in_bytes();
  }
}

void WAVEFILE::find_riff_datablock (void) throw(ECA_ERROR*) {
  if (find_block("data")==-1) {
    throw(new ECA_ERROR("AUDIOIO-WAVE", "no RIFF data block found", ECA_ERROR::retry));
  }
  data_start_position = fio->get_file_position();
}

void WAVEFILE::read_riff_header (void) throw(ECA_ERROR*) {
  ecadebug->msg(5, "(program flow: read_riff_header())");
   
  fio->read_to_buffer(&riff_header, sizeof(riff_header));

  //  fread(&riff_header,1,sizeof(riff_header),fobject);
  if (memcmp("RIFF",riff_header.id,4) != 0 ||
      memcmp("WAVE",riff_header.wname,4) != 0) {
    throw(new ECA_ERROR("AUDIOIO-WAVE", "invalid RIFF-header", ECA_ERROR::stop));
  }
}

void WAVEFILE::write_riff_header (void) throw(ECA_ERROR*) {
  ecadebug->msg(5, "(program flow: write_riff_header())");

  long int savetemp = fio->get_file_position();
    
  memcpy(riff_header.id,"RIFF",4);
  memcpy(riff_header.wname,"WAVE",4);
  riff_header.size = fio->get_file_length();

  fio->set_file_position(0);
  //  fseek(fobject,0,SEEK_SET);

  fio->write_from_buffer(&riff_header, sizeof(riff_header));
  //  fwrite(&riff_header,1,sizeof(riff_header),fobject);
  if (memcmp("RIFF",riff_header.id,4) != 0 || 
      memcmp("WAVE",riff_header.wname,4) != 0)
    throw(new ECA_ERROR("AUDIOIO-WAVE", "invalid RIFF-header", ECA_ERROR::stop));

  char temp[16];
  memcpy(temp, "Riff ID: ", 9);
  memcpy(&(temp[9]), riff_header.id, 4);
  temp[13] = 0;
  ecadebug->msg(5,  string(temp));

  ecadebug->msg(5, "Wave data size " + 	kvu_numtostr(riff_header.size));
  memcpy(temp, "Riff type: ", 11);
  memcpy(&(temp[11]), riff_header.wname, 4);
  temp[15] = 0;
  ecadebug->msg(5, "Riff type " + string(temp));

  //  fseek(fobject,save,SEEK_SET);
  fio->set_file_position(savetemp);
}

void WAVEFILE::read_riff_fmt(void) throw(ECA_ERROR*)
{
  ecadebug->msg(5, "(program flow: read_riff_fmt())");

  long int savetemp = fio->get_file_position();    

  if (find_block("fmt ")==-1)
    throw(new ECA_ERROR("AUDIOIO-WAVE", "no riff fmt-block found",  ECA_ERROR::stop));
  else {
    fio->read_to_buffer(&riff_format, sizeof(riff_format));
    //    fread(&riff_format,1,sizeof(riff_format),fobject);

    if (riff_format.format != 1) {
      throw(new ECA_ERROR("AUDIOIO-WAVE", "Only WAVE_FORMAT_PCM is supported."));
      //      ecadebug->msg("(audioio-wave) WARNING: wave-format not '1'.");
    }

    set_channels(riff_format.channels);
    set_samples_per_second(riff_format.srate);
    if (riff_format.bits == 16)
      set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
    else if (riff_format.bits == 8)
      set_sample_format(ECA_AUDIO_FORMAT::sfmt_u8);
    else 
      throw(new ECA_ERROR("AUDIOIO-WAVE", "Only 8bit and 16bit sample resolution is supported."));
  }

  fio->set_file_position(savetemp);
}

void WAVEFILE::write_riff_fmt(void)
{
  RB fblock;

  fio->set_file_position_end();

  riff_format.channels = channels();
  riff_format.bits = bits();
  riff_format.srate = samples_per_second();
  riff_format.byte_second = bytes_per_second();
  riff_format.align = frame_size();
  riff_format.format = 1;     // WAVE_FORMAT_PCM (0x0001) Microsoft Pulse Code
                              //                          Modulation (PCM) format

  memcpy(fblock.sig, "fmt ", 4);
  fblock.bsize=16;

  fio->write_from_buffer(&fblock, sizeof(fblock));
  fio->write_from_buffer(&riff_format, sizeof(riff_format));
  ecadebug->msg(5, "Wrote RIFF format header.");
}

void WAVEFILE::write_riff_datablock(void) {
  RB fblock;

  ecadebug->msg(5, "(program flow: write_riff_datablock())");
    
  fio->set_file_position_end();

  memcpy(fblock.sig,"data",4);
  fblock.bsize = 0;
  fio->write_from_buffer(&fblock, sizeof(fblock));
  data_start_position = fio->get_file_position();
}

void WAVEFILE::update_riff_datablock(void) {
  RB fblock;
  ecadebug->msg(5, "(program flow: update_riff_datablock())");
    
  memcpy(fblock.sig,"data",4);

  find_block("data");
  long int savetemp = fio->get_file_position();

  fio->set_file_position_end();
  fblock.bsize = fio->get_file_position() - savetemp;

  savetemp = savetemp - sizeof(fblock);
  if (savetemp > 0) {
    fio->set_file_position(savetemp);
    fio->write_from_buffer(&fblock, sizeof(fblock));
  }
}

bool WAVEFILE::next_riff_block(RB *t, unsigned long int *offtmp)
{
  ecadebug->msg(5, "(program flow: next_riff_block())");

  fio->read_to_buffer(t, sizeof(RB));
  if (fio->file_bytes_processed() != sizeof(RB)) {
    ecadebug->msg(2, "invalid RIFF block!");
    return(false);
  }
    
  if (!fio->is_file_ready()) return(false);
  *offtmp = t->bsize + fio->get_file_position();
  return (true);
}

signed long int WAVEFILE::find_block(const char* fblock) {
  unsigned long int offset;
  RB block;

  ecadebug->msg(5, "(audioio-wave) find_block(): " + string(fblock,4));
    
  fio->set_file_position(sizeof(riff_header));
  while(next_riff_block(&block,&offset)) {
    ecadebug->msg(5, "AUDIOIO-WAVE: found RIFF-block ");
    if (memcmp(block.sig,fblock,4) == 0) {
      return(block.bsize);
    }
    fio->set_file_position(offset);
  }

  return(-1);
}

bool WAVEFILE::finished(void) const {
 if (fio->is_file_error() ||
     !fio->is_file_ready()) 
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
  fio->read_to_buffer(target_buffer, frame_size() * samples);
  return(fio->file_bytes_processed() / frame_size());
}

void WAVEFILE::write_samples(void* target_buffer, long int samples) {
  // --------
  // require:
  assert(samples >= 0);
  assert(target_buffer != 0);
  // --------
  fio->write_from_buffer(target_buffer, frame_size() * samples);
}

void WAVEFILE::seek_position(void) {
  if (is_open())
    fio->set_file_position(data_start_position + position_in_samples() * frame_size());
}

void WAVEFILE::set_length_in_bytes(void) {
  long int savetemp = fio->get_file_position();

  find_block("data");
  long int t = fio->get_file_position();

  fio->set_file_position_end();
  t = fio->get_file_position() - t;
  length_in_samples(t / frame_size());
  MESSAGE_ITEM mitem;
  mitem << "(audioio-wave) data length " << t << "bytes.";
  ecadebug->msg(5, mitem.to_string());

  fio->set_file_position(savetemp);
}
