// ------------------------------------------------------------------------
// audioio-mp3.cpp: Interface for mp3 decoders and encoders that support 
//                  input/output using standard streams. Defaults to
//                  mpg123 and lame.
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
// Note! Routines for parsing mp3 header information were taken from XMMS
//       1.2.5's mpg123 plugin.
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
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <inttypes.h> /* for ANSI/ISO type defs */

#include <unistd.h>

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "audioio-mp3.h"
#include "audioio-mp3_impl.h"
#include "samplebuffer.h"
#include "audioio.h"

#include "eca-debug.h"

string MP3FILE::default_mp3_input_cmd = "mpg123 --stereo -r %s -b 0 -q -s -k %o %f";
string MP3FILE::default_mp3_output_cmd = "lame -b 128 -x -S - %f";

void MP3FILE::set_mp3_input_cmd(const std::string& value) { MP3FILE::default_mp3_input_cmd = value; }
void MP3FILE::set_mp3_output_cmd(const std::string& value) { MP3FILE::default_mp3_output_cmd = value; }

/***************************************************************
 * Routines for parsing mp3 header information. Taken from XMMS
 * 1.2.5's mpg123 plugin.
 **************************************************************/

#define         MAXFRAMESIZE            1792
#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

int tabsel_123[2][3][16] =
{
	{
    {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448,},
       {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384,},
       {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320,}},

	{
       {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256,},
	    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160,},
	    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160,}}
};

long mpg123_freqs[9] =
{44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000};

struct frame
{
	int stereo;
	int jsbound;
	int single;
	int II_sblimit;
	int down_sample_sblimit;
	int lsf;
	int mpeg25;
	int down_sample;
	int header_change;
	int lay;
	int error_protection;
	int bitrate_index;
	int sampling_frequency;
	int padding;
	int extension;
	int mode;
	int mode_ext;
	int copyright;
	int original;
	int emphasis;
	int framesize;		/* computed framesize */
};

static bool mpg123_head_check(unsigned long head)
{
	if ((head & 0xffe00000) != 0xffe00000)
		return false;
	if (!((head >> 17) & 3))
		return false;
	if (((head >> 12) & 0xf) == 0xf)
		return false;
	if (!((head >> 12) & 0xf))
		return false;
	if (((head >> 10) & 0x3) == 0x3)
		return false;
	if (((head >> 19) & 1) == 1 && ((head >> 17) & 3) == 3 && ((head >> 16) & 1) == 1)
		return false;
	if ((head & 0xffff0000) == 0xfffe0000)
		return false;
	
	return true;
}

static double mpg123_compute_bpf(struct frame *fr)
{
	double bpf;

	switch (fr->lay)
	{
		case 1:
			bpf = tabsel_123[fr->lsf][0][fr->bitrate_index];
			bpf *= 12000.0 * 4.0;
			bpf /= mpg123_freqs[fr->sampling_frequency] << (fr->lsf);
			break;
		case 2:
		case 3:
			bpf = tabsel_123[fr->lsf][fr->lay - 1][fr->bitrate_index];
			bpf *= 144000;
			bpf /= mpg123_freqs[fr->sampling_frequency] << (fr->lsf);
			break;
		default:
			bpf = 1.0;
	}

	return bpf;
}

static double mpg123_compute_tpf(struct frame *fr)
{
	static int bs[4] =
	{0, 384, 1152, 1152};
	double tpf;

	tpf = (double) bs[fr->lay];
	tpf /= mpg123_freqs[fr->sampling_frequency] << (fr->lsf);
	return tpf;
}

/*
 * the code a header and write the information
 * into the frame structure
 */
static bool mpg123_decode_header(struct frame *fr, unsigned long newhead)
{
	if (newhead & (1 << 20))
	{
		fr->lsf = (newhead & (1 << 19)) ? 0x0 : 0x1;
		fr->mpeg25 = 0;
	}
	else
	{
		fr->lsf = 1;
		fr->mpeg25 = 1;
	}
	fr->lay = 4 - ((newhead >> 17) & 3);
	if (fr->mpeg25)
	{
		fr->sampling_frequency = 6 + ((newhead >> 10) & 0x3);
	}
	else
		fr->sampling_frequency = ((newhead >> 10) & 0x3) + (fr->lsf * 3);
	fr->error_protection = ((newhead >> 16) & 0x1) ^ 0x1;

	if (fr->mpeg25)		/* allow Bitrate change for 2.5 ... */
		fr->bitrate_index = ((newhead >> 12) & 0xf);

	fr->bitrate_index = ((newhead >> 12) & 0xf);
	fr->padding = ((newhead >> 9) & 0x1);
	fr->extension = ((newhead >> 8) & 0x1);
	fr->mode = ((newhead >> 6) & 0x3);
	fr->mode_ext = ((newhead >> 4) & 0x3);
	fr->copyright = ((newhead >> 3) & 0x1);
	fr->original = ((newhead >> 2) & 0x1);
	fr->emphasis = newhead & 0x3;

	fr->stereo = (fr->mode == MPG_MD_MONO) ? 1 : 2;

	if (!fr->bitrate_index) {
	        ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Invalid bitrate!");
		return (false);
	}

	int ssize = 0;
	switch (fr->lay)
	  {
	  case 1:
//  	    fr->do_layer = mpg123_do_layer1;
//  	    mpg123_init_layer2();
	    fr->framesize = (long) tabsel_123[fr->lsf][0][fr->bitrate_index] * 12000;
	    fr->framesize /= mpg123_freqs[fr->sampling_frequency];
	    fr->framesize = ((fr->framesize + fr->padding) << 2) - 4;
	    break;
	  case 2:
//  	    fr->do_layer = mpg123_do_layer2;
//  	    mpg123_init_layer2();
	    fr->framesize = (long) tabsel_123[fr->lsf][1][fr->bitrate_index] * 144000;
	    fr->framesize /= mpg123_freqs[fr->sampling_frequency];
	    fr->framesize += fr->padding - 4;
	    break;
	  case 3:
//  	    fr->do_layer = mpg123_do_layer3;
	    if (fr->lsf)
	      ssize = (fr->stereo == 1) ? 9 : 17;
	    else
	      ssize = (fr->stereo == 1) ? 17 : 32;
	    if (fr->error_protection)
	      ssize += 2;
	    fr->framesize = (long) tabsel_123[fr->lsf][2][fr->bitrate_index] * 144000;
	    fr->framesize /= mpg123_freqs[fr->sampling_frequency] << (fr->lsf);
	    fr->framesize = fr->framesize + fr->padding - 4;
	    break;
	  default:
	    return (false);
	}

	if(fr->framesize > MAXFRAMESIZE) {
	        ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Invalid framesize!");
		return false;
	}

	return true;
}

static uint32_t convert_to_header(uint8_t * buf)
{

	return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

static bool mpg123_detect_by_content(const char* filename, struct frame* frp) {
	FILE *file;
	uint8_t tmp[4];
	uint32_t head;
	unsigned char *buf;
	int in_buf, i;

	if((file = fopen(filename, "rb")) == NULL) {
	  return false;
	}
	if (fread(tmp, 1, 4, file) != 4)
		goto done;
	buf = new unsigned char [1024];
	head = convert_to_header(tmp);
	while(!mpg123_head_check(head)) {
		/*
		 * The mpeg-stream can start anywhere in the file,
		 * so we check the entire file
		 */
		/* Optimize this */
		in_buf = fread(buf, 1, 1024, file);
		if(in_buf == 0)
		{
			delete[] buf;
			ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Mp3 header not found!");
			goto done;
		}
		for (i = 0; i < in_buf; i++)
		{
			head <<= 8;
			head |= buf[i];
			if(mpg123_head_check(head))
			{
				fseek(file, i+1-in_buf, SEEK_CUR);
				break;
			}
		}
	}
	delete[] buf;
	if (mpg123_decode_header(frp, head))
	{
		/*
		 * We found something which looks like a MPEG-header.
		 * We check the next frame too, to be sure
		 */
	        if (fseek(file, frp->framesize, SEEK_CUR) != 0) {
			goto done;
		}
		if (fread(tmp, 1, 4, file) != 4) {
			goto done;
		}
		head = convert_to_header(tmp);
		if (mpg123_head_check(head) && mpg123_decode_header(frp, head))
		{
			fclose(file);
			return true;
		}
	}

 done:
	fclose(file);
	ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Valid mp3 header not found!");
	return false;
}



/***************************************************************
 * MP3FILE specific parts.
 **************************************************************/

MP3FILE::MP3FILE(const std::string& name) {
  label(name);
  finished_rep = false;
  mono_input_rep = false;
  pcm_rep = 1;
}

MP3FILE::~MP3FILE(void) { close(); }

void MP3FILE::open(void) throw(AUDIO_IO::SETUP_ERROR &) { 
  if (io_mode() == io_read) {
    get_mp3_params(label());
  }

  toggle_open_state(true);
  triggered_rep = false;
}

void MP3FILE::close(void) {
  if (pid_of_child() > 0) {
      ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Cleaning child process." + kvu_numtostr(pid_of_child()) + ".");
      clean_child();
      triggered_rep = false;
  }
  toggle_open_state(false);
}

void MP3FILE::process_mono_fix(char* target_buffer, long int bytes_rep) {
  for(long int n = 0; n < bytes_rep;) {
    target_buffer[n + 2] = target_buffer[n];
    target_buffer[n + 3] = target_buffer[n + 1];
    n += 4;
  }
}

long int MP3FILE::read_samples(void* target_buffer, long int samples) {
  if (triggered_rep != true) { 
    triggered_rep = true;
    fork_mp3_input();
  }

  bytes_rep = ::fread(target_buffer, 1, frame_size() * samples, f1_rep);
  if (bytes_rep < samples * frame_size() || bytes_rep == 0) {
    if (position_in_samples() == 0) 
      ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Can't start process \"" + MP3FILE::default_mp3_input_cmd + "\". Please check your ~/.ecasoundrc.");
    finished_rep = true;
  }
  else finished_rep = false;
  
  return(bytes_rep / frame_size());
}

void MP3FILE::write_samples(void* target_buffer, long int samples) {
  if (triggered_rep != true) {
    triggered_rep = true;
    fork_mp3_output();
  }

  if (wait_for_child() != true) {
    finished_rep = true;
  }
  else {
    bytes_rep = ::write(fd_rep, target_buffer, frame_size() * samples);
    if (bytes_rep < frame_size() * samples || bytes_rep == 0) {
      if (position_in_samples() == 0) 
	ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Can't start process \"" + MP3FILE::default_mp3_output_cmd + "\". Please check your ~/.ecasoundrc.");
      finished_rep = true;
    }
    else finished_rep = false;
  }
}

void MP3FILE::seek_position(void) {
  if (triggered_rep == true &&
      last_position_rep != position_in_samples()) {
    if (is_open() == true) {
      finished_rep = false;
      ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Cleaning child process." + kvu_numtostr(pid_of_child()) + ".");
      clean_child();
      triggered_rep = false;
    }
  }
}

/*
void MP3FILE::get_mp3_params_old(const std::string& fname) throw(AUDIO_IO::SETUP_ERROR&) {
  Layer newlayer;

  if (newlayer.get(fname.c_str()) != true) {
    throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-MP3: Can't open " + label() + " for reading."));
  }

  struct stat buf;
  ::stat(fname.c_str(), &buf);
  double fsize = (double)buf.st_size;
  
  double bitrate = newlayer.bitrate() * 1000.0;
  double sfreq = newlayer.sfreq();

  // notice! mpg123 always outputs 16bit samples, stereo
  mono_input_rep = (newlayer.mode() == Layer::MPG_MD_MONO) ? true : false;
  set_channels(2);
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);

  MESSAGE_ITEM m;
  m << "(audioio-mp3) mp3 file size: " << fsize << "\n";
  m << "(audioio-mp3) mp3 length value: " << newlayer.length() << "\n";
  m << "(audioio-mp3) sfreq: " << sfreq << "\n";
  m << "(audioio-mp3) bitrate: " << bitrate << "\n";
  if (bitrate != 0)
    length_in_samples((long)ceil(8.0 * fsize / bitrate * bytes_per_second() / frame_size()));
  
  if (bitrate == 0 ||
      length_in_samples() < 0) length_in_samples(0);
  
  m << "(audioio-mp3) setting MP3 length_value: " << length_in_seconds() << "\n";
  pcm_rep = newlayer.pcmPerFrame();
  
  m << "(audioio-mp3) MP3 pcm value: " << pcm_rep;
  ecadebug->msg(ECA_DEBUG::user_objects,m.to_string());
}
*/ 

void MP3FILE::get_mp3_params(const std::string& fname) throw(AUDIO_IO::SETUP_ERROR&) {
  struct frame fr;
  if (mpg123_detect_by_content(fname.c_str(), &fr) != true) {
    throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-MP3: Can't open " + label() + " for reading."));
  }


  /* file size */
  struct stat buf;
  ::stat(fname.c_str(), &buf);
  double fsize = (double)buf.st_size;
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Total file size (bytes): " + kvu_numtostr(fsize));

  /* bitrate */
  double bitrate = tabsel_123[fr.lsf][fr.lay - 1][fr.bitrate_index] * 1000;
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Bitrate (bits/s): " + kvu_numtostr(bitrate));

  /* sample freq */
  double sfreq = mpg123_freqs[fr.sampling_frequency];
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Sampling frequncy (Hz): " + kvu_numtostr(sfreq));

  /* channels */
  // notice! mpg123 always outputs 16bit samples, stereo
  mono_input_rep = (fr.mode == MPG_MD_MONO) ? true : false;
  set_channels(2);

  /* temporal length */
  int numframes =  static_cast<int>((fsize / mpg123_compute_bpf(&fr)));
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Total length (frames): " + kvu_numtostr(numframes));
  double tpf = mpg123_compute_tpf(&fr);
  length_in_seconds(tpf * numframes);
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Total length (seconds): " + kvu_numtostr(length_in_seconds()));

  /* sample format (this comes from mpg123) */
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);

  /* set pcm per frame value */
  static int bs[4] = {0, 384, 1152, 1152};
  pcm_rep = bs[fr.lay];
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Pcm per mp3 frames: " + kvu_numtostr(pcm_rep));
}


void MP3FILE::fork_mp3_input(void) {
  string cmd = MP3FILE::default_mp3_input_cmd;
  if (cmd.find("%o") != string::npos) {
    cmd.replace(cmd.find("%o"), 2, kvu_numtostr((long)(position_in_samples() / pcm_rep)));
  }
  last_position_rep = position_in_samples();
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) " + cmd);
  set_fork_command(cmd);
  set_fork_file_name(label());
  set_fork_sample_rate(samples_per_second());
// for ecawave mp3 bug debugging:
//    cerr << "About to fork! (5s)" << endl;
//    sleep(5);
//    cerr << "About to fork! (now)" << endl;
  fork_child_for_read();
  if (child_fork_succeeded() == true) {
// for ecawave mp3 bug debugging:
//      cerr << "Child fork succeeded!" << endl;
    fd_rep = file_descriptor();
    f1_rep = fdopen(fd_rep, "r");
    if (f1_rep == 0) finished_rep = true;
  }
// for ecawave mp3 bug debugging:
//    cerr << "My pid: " << ::getpid() << endl;
//    sleep(5);
//    cerr << "Fork exit()" << endl;
}

void MP3FILE::fork_mp3_output(void) {
  ecadebug->msg("(audioio-mp3) Starting to encode " + label() + " with lame.");
  last_position_rep = position_in_samples();
  set_fork_command(MP3FILE::default_mp3_output_cmd);
  set_fork_file_name(label());
  fork_child_for_write();
  if (child_fork_succeeded() == true) {
    fd_rep = file_descriptor();
  }
}
