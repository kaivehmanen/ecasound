#include <unistd.h>
#include "audioio-buffered-proxy.h"

AUDIO_IO_BUFFERED_PROXY::AUDIO_IO_BUFFERED_PROXY (AUDIO_IO_PROXY_SERVER *pserver, 
						  AUDIO_IO* aobject) 
  : pserver_repp(pserver),
    child_repp(aobject) 
{
  cerr << "Creating a new client." << endl;
  pserver_repp->register_client(child_repp);
  pbuffer_repp = pserver_repp->get_client_buffer(child_repp);
  cerr << "Registering aobject " << child_repp << ", got " <<
    pbuffer_repp << "." << endl;
  xruns_rep = 0;

  // just in case the child object has already been configured
  fetch_child_data();
}

void AUDIO_IO_BUFFERED_PROXY::fetch_child_data(void) {
  if (child_repp->io_mode() == AUDIO_IO::io_read) 
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_read;
  else
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_write;
  
  int channels = child_repp->channels();
  int buffersize = child_repp->buffersize();
  for(unsigned int n = 0; n < pbuffer_repp->sbufs_rep.size(); n++) {
    pbuffer_repp->sbufs_rep[n].number_of_channels(channels);
    pbuffer_repp->sbufs_rep[n].length_in_samples(buffersize);
  }
}

AUDIO_IO_BUFFERED_PROXY::~AUDIO_IO_BUFFERED_PROXY(void) {
  pserver_repp->unregister_client(child_repp);
  if (xruns_rep > 0) 
    cerr << "(audioio-buffered-proxy) There were total " << xruns_rep << " xruns." << endl;
}

bool AUDIO_IO_BUFFERED_PROXY::finished(void) const { 
  if (pbuffer_repp->finished_rep.get() == 1) return(true);
  return(false);
}

long AUDIO_IO_BUFFERED_PROXY::length_in_samples(void) const { 
  return(child_repp->length_in_samples());
}

void AUDIO_IO_BUFFERED_PROXY::read_buffer(SAMPLE_BUFFER* sbuf) { 
  if (pbuffer_repp->read_space() > 0) {
    sbuf->operator=(pbuffer_repp->sbufs_rep[pbuffer_repp->readptr_rep.get()]);
    pbuffer_repp->advance_read_pointer();
    position_in_samples_advance(sbuf->length_in_samples());
    if (pbuffer_repp->read_space() < 16) {
      cerr << "Client read " << pbuffer_repp->readptr_rep.get() << ";";
      cerr << " read_space: " << pbuffer_repp->read_space() << "." << endl;
    }
  }
  else {
    xruns_rep++;
    cerr << "Underrun! Exiting!" << endl;
    exit(0);
  }
}

void AUDIO_IO_BUFFERED_PROXY::write_buffer(SAMPLE_BUFFER* sbuf) { 
  if (pbuffer_repp->write_space() > 0) {
    pbuffer_repp->sbufs_rep[pbuffer_repp->writeptr_rep.get()].operator=(*sbuf);
    pbuffer_repp->advance_write_pointer();
    position_in_samples_advance(sbuf->length_in_samples());
    extend_position();
  }
  else {
    xruns_rep++;
    cerr << "Overrun! Exiting" << endl;
    exit(0);
  }
}

void AUDIO_IO_BUFFERED_PROXY::seek_position(void) { 
  bool was_running = false;
  if (pserver_repp->is_running() == true) {
    was_running = true;
    pserver_repp->stop();
    while(pserver_repp->is_running() != true) usleep(50000);
  }
  child_repp->seek_position_in_samples(position_in_samples());
  if (was_running == true) pserver_repp->start();
}

void AUDIO_IO_BUFFERED_PROXY::open(void) throw(SETUP_ERROR&) { 
  child_repp->open();
  fetch_child_data();
}

void AUDIO_IO_BUFFERED_PROXY::close(void) { 
  child_repp->close();
}
