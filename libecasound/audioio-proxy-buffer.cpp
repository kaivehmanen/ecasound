// ------------------------------------------------------------------------
// audioio-proxy-buffer.cpp: Buffer used between proxy server and client
// Copyright (C) 2000,2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include "audioio-proxy-buffer.h"

/**
 * Constructor.
 */
AUDIO_IO_PROXY_BUFFER::AUDIO_IO_PROXY_BUFFER(int number_of_buffers,
					     long int buffersize,
					     int channels,
					     long int sample_rate)
  :  readptr_rep(0),
     writeptr_rep(0),
     finished_rep(0),
     sbufs_rep(number_of_buffers, SAMPLE_BUFFER(buffersize,
						channels,
						sample_rate))
{
//    cerr << "Created a new client buffer with " 
//         << sbufs_rep.size()
//         << " buffers." 
//         << endl;
}

/**
 * Resets all pointers to their initial state. 
 */
void AUDIO_IO_PROXY_BUFFER::reset(void) {
  readptr_rep.set(0);
  writeptr_rep.set(0);
  finished_rep.set(0);
}

/**
 * Number of sample buffer available for reading.
 */
int AUDIO_IO_PROXY_BUFFER::read_space(void) {
  int write = writeptr_rep.get();
  int read = readptr_rep.get();
  
  if (write >= read)
    return(write - read);
  else
    return((write - read + sbufs_rep.size()) % sbufs_rep.size());
}

/**
 * Number of sample buffers available for writing.
 */
int AUDIO_IO_PROXY_BUFFER::write_space(void) {
  int write = writeptr_rep.get();
  int read = readptr_rep.get();
  
  if (write > read)
    return(((read - write + sbufs_rep.size()) % sbufs_rep.size()) - 1);
  else if (write < read)
    return(read - write - 1);
  else 
    return(sbufs_rep.size() - 1);
}

void AUDIO_IO_PROXY_BUFFER::advance_read_pointer(void) {
  readptr_rep.set((readptr_rep.get() + 1) % sbufs_rep.size());
}

void AUDIO_IO_PROXY_BUFFER::advance_write_pointer(void) {
  writeptr_rep.set((writeptr_rep.get() + 1) % sbufs_rep.size());
}
