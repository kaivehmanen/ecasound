#include "audioio-buffered-proxy.h"

AUDIO_IO_BUFFERED_PROXY::AUDIO_IO_BUFFERED_PROXY (AUDIO_IO_PROXY_SERVER *pserver, 
						  AUDIO_IO* aobject) 
  : pserver_repp(pserver),
    child_repp(aobject) 
{
  cerr << "Creating a new client." << endl;
  pserver_repp->register_client(child_repp);
  pbuffer_repp = pserver_repp->get_client_buffer(child_repp);
  xruns_rep = 0;
}

AUDIO_IO_BUFFERED_PROXY::~AUDIO_IO_BUFFERED_PROXY(void) {
  pserver_repp->unregister_client(child_repp);
  cerr << "There were total " << xruns_rep << " xruns." << endl;
}

bool AUDIO_IO_BUFFERED_PROXY::finished(void) const { 
  child_repp->finished();
}

long AUDIO_IO_BUFFERED_PROXY::length_in_samples(void) const { 
  return(child_repp->length_in_samples());
}

void AUDIO_IO_BUFFERED_PROXY::read_buffer(SAMPLE_BUFFER* sbuf) { 
  if (pbuffer_repp->read_space() > 0) {
    sbuf->operator=(pbuffer_repp->sbufs_rep[pbuffer_repp->readptr_rep.get()]);
    pbuffer_repp->advance_read_pointer();
    if (pbuffer_repp->read_space() < 16) {
      cerr << "Client read " << pbuffer_repp->readptr_rep.get() << ";";
      cerr << " read_space: " << pbuffer_repp->read_space() << "." << endl;
    }
  }
  else {
    xruns_rep++;
    cerr << "Underrun!" << endl;
    exit(0);
  }
}

void AUDIO_IO_BUFFERED_PROXY::write_buffer(SAMPLE_BUFFER* sbuf) { 
  if (pbuffer_repp->write_space() > 0) {
    pbuffer_repp->sbufs_rep[pbuffer_repp->writeptr_rep.get()].operator=(*sbuf);
    pbuffer_repp->advance_write_pointer();
  }
  else {
    xruns_rep++;
    cerr << "Overrun!" << endl;
  }
}

void AUDIO_IO_BUFFERED_PROXY::seek_position(void) { 
  cerr << "Not implemented!" << endl;
}

void AUDIO_IO_BUFFERED_PROXY::open(void) throw(SETUP_ERROR&) { 
  child_repp->open();
  if (child_repp->io_mode() == AUDIO_IO::io_read) 
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_read;
  else
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_write;
}

void AUDIO_IO_BUFFERED_PROXY::close(void) { 
  child_repp->close();
}
