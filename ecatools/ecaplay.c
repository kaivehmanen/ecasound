/**
 * ecaplay.c: A simple command-line tool for playing audio files
 *            using the default output device specified in 
 *            "~/.ecasoundrc".
 * Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h> /* POSIX: sigaction() */
#include <stdlib.h> /* ANSI-C: malloc(), free() */
#include <string.h> /* ANSI-C: strlen(), strncmp() */

#include <ecasoundc.h>

/**
 * Function declarations
 */
int main(int argc, char *argv[]);

static void add_track_to_chainsetup(eci_handle_t eci, const char* nextrack);
static void set_track_to_chainsetup(eci_handle_t* eci, const char* nexttrack);
static const char* get_track(int n, int argc, char *argv[]);
static void print_usage(FILE* stream);
static int process_option(const char* option);
static void signal_handler(int signum);
static void setup_signal_handling(void);

/**
 * Definitions and options 
 */

#define ECAPLAY_TIMEOUT 3
#define ECAPLAY_EIAM_LOGLEVEL 256

/** 
 * Global variables
 */
static const char* ecaplay_version = "20021107-36";
static int ecaplay_debuglevel = ECAPLAY_EIAM_LOGLEVEL;
static int ecaplay_skip = 0;
static const char* ecaplay_output = NULL;
static sig_atomic_t ecaplay_skip_flag = 0;

/**
 * Function definitions
 */

int main(int argc, char *argv[])
{
  eci_handle_t eci = NULL;
  int i, res = 0, tracknum = 1;
  const char* nexttrack = NULL;

  for(i = 1; i < argc; i++) { res += process_option(argv[i]); }

  tracknum += ecaplay_skip;
  if (tracknum <= argc) {
    nexttrack = get_track(tracknum, argc, argv);
  }

  if (res == 0 && nexttrack != NULL) {
    setup_signal_handling();

    set_track_to_chainsetup(&eci, nexttrack);

    while(nexttrack != NULL) {
      unsigned int timeleft = ECAPLAY_TIMEOUT;

      while(timeleft > 0) {
	timeleft = sleep(timeleft);

	if (timeleft > 0 && ecaplay_skip_flag > 1) {
	  fprintf(stderr, "\n(ecaplay) Caught an exception. Exiting...\n");
	  eci_cleanup_r(eci);
	  return 0;
	}
      }

      if (ecaplay_skip_flag == 0) {
	eci_command_r(eci, "engine-status");
      }

      if (ecaplay_skip_flag != 0 || strcmp(eci_last_string_r(eci), "running") != 0) {
	ecaplay_skip_flag = 0;
	nexttrack = get_track(++tracknum, argc, argv);
	if (nexttrack != NULL) {
	  eci_cleanup_r(eci);
	  set_track_to_chainsetup(&eci, nexttrack);
	}
	else {
	  break;
	}
      }
    }
  
    fprintf(stderr, "exiting...\n");
  
    eci_cleanup_r(eci);
  }

  return res;
}

static void add_track_to_chainsetup(eci_handle_t eci, const char* nexttrack)
{
  size_t len = strlen("ai-add '") + strlen(nexttrack) + strlen("'") + 1;
  char* tmpbuf = malloc(len);
  assert(tmpbuf != NULL);
  snprintf(tmpbuf, len, "ai-add \"%s\"", nexttrack);
  eci_command_r(eci, tmpbuf);

  /* check that add succeeded */
  eci_command_r(eci, "ai-list");
  if (eci_last_string_list_count_r(eci) != 1) {
    fprintf(stderr, "(ecaplay) Warning! Failed to add input '%s'.\n", nexttrack);
  }

  free(tmpbuf);
}

static void set_track_to_chainsetup(eci_handle_t* eci, const char* nexttrack)
{
  const char* ret = NULL;

  *eci = eci_init_r();

  if (ecaplay_debuglevel != -1) {
    char tmpbuf[32];
    snprintf(tmpbuf, 32, "debug %d", ecaplay_debuglevel);
    eci_command_r(*eci, tmpbuf);
  }

  eci_command_r(*eci, "cs-add ecaplay_chainsetup");
  /* check that add succeeded */
  eci_command_r(*eci, "cs-list");
  if (eci_last_string_list_count_r(*eci) != 2) {
    fprintf(stderr, "(ecaplay) Warning! Failed to add a new chainsetup.\n");
  }

  /* as this is a new chainsetup, we can assume that 
   * adding chains succeeds */
  eci_command_r(*eci, "c-add ecaplay_chain");
  
  add_track_to_chainsetup(*eci, nexttrack);

  /* FIXME: add support for fetcing input audio format, see ecanormalize.cpp */

  if (ecaplay_output == NULL) {
    eci_command_r(*eci, "ao-add-default");

    /* check that add succeeded */
    eci_command_r(*eci, "ao-list");
    if (eci_last_string_list_count_r(*eci) != 1) {
      fprintf(stderr, "(ecaplay) Warning! Failed to add default output.\n");
    }
  }
  else {
    int len = strlen("ao-add ") + strlen(ecaplay_output) + 1;
    char* tmpbuf = (char*)malloc(len);
    snprintf(tmpbuf, len, "ao-add %s", ecaplay_output);
    eci_command_r(*eci, tmpbuf);
    free(tmpbuf);
  }

  /* FIXME: add detection of consecutive errors */
  eci_command_r(*eci, "cs-connect");
  /* FIXME: fetch last_error and print it; see other ecatools */
  eci_command_r(*eci, "cs-connected");
  ret = eci_last_string_r(*eci);
  if (strncmp(ret, "ecaplay_chainsetup", strlen("ecaplay_chainsetup")) != 0) {
    fprintf(stderr, "(ecaplay) Error while playing file '%s' . Skipping...\n", nexttrack);
  }
  else {
    printf("(ecaplay) Playing file '%s'.\n", nexttrack);
    eci_command_r(*eci, "start");
  }
}

/**
 * Returns the track number from the list 
 * given in argc and argv.
 * 
 * @return track name or NULL on error
 */
static const char* get_track(int n, int argc, char *argv[])
{
  int i, c = 0;

  assert(n > 0 && n <= argc);

  for(i = 1; i < argc; i++) { 
    /* FIXME: add support for '-- -foo.wav' */
    if (argv[i][0] != '-') {
      if (++c == n) {
	return argv[i];
      }
    }
  }
 
  return NULL;
}

static void print_usage(FILE* stream)
{
  fprintf(stream, "ecaplay v%s\n", ecaplay_version);
  fprintf(stream, "Copyright (C) 1997-2002 Kai Vehmanen, released under GPL licence \n");
  fprintf(stream, "Ecaplay comes with ABSOLUTELY NO WARRANTY.\n");
  fprintf(stream, "You may redistribute copies of ecasound under the terms of the GNU\n");
  fprintf(stream, "General Public License. For more information about these matters, see\n"); 
  fprintf(stream, "the file named COPYING.\n");

  fprintf(stream, "\nUSAGE: ecaplay [-dhk] file1 [ file2, ... fileN ]\n\n");
}

static int process_option(const char* option)
{
  if (option[0] == '-') {
    if (strncmp("--help", option, sizeof("--help")) == 0 ||
	strncmp("--version", option, sizeof("--version")) == 0) {
      print_usage(stdout);
      return(1);
    }

    switch(option[1]) 
      {
      case 'd': 
	{
	  const char* level = &option[3];
	  if (option[2] != 0 && option[3] != 0) {
	    ecaplay_debuglevel |= atoi(level);
	    printf("(ecaplay) Setting log level to %d.\n", ecaplay_debuglevel);
	  }
	  break;
	}
      
      case 'k': 
	{
	  const char* skip = &option[3];
	  if (option[2] != 0 && option[3] != 0) {
	    ecaplay_skip = atoi(skip);
	    printf("(ecaplay) Skipping the first %d files..\n", ecaplay_skip);
	  }
	  break;
	}
	
      case 'h': 
	{
	  print_usage(stdout);
	  return 1;
	}

      case 'o': 
	{
	  const char* output = &option[3];
	  if (option[2] != 0 && option[3] != 0) {
	    ecaplay_output = output;
	    printf("(ecaplay) Using device '%s' for output.\n", ecaplay_output);
	  }
	  break;
	}

      
      default:
	{
	  fprintf(stderr, "(ecaplay) Error! Unknown option '%s'.\n", option);
	  print_usage(stderr);
	  return 2;
	}
      }
  }

  return 0;
}

static void setup_signal_handling(void)
{
  struct sigaction es_handler_int;
  struct sigaction ign_handler;

  es_handler_int.sa_handler = signal_handler;
  sigemptyset(&es_handler_int.sa_mask);
  es_handler_int.sa_flags = 0;

  ign_handler.sa_handler = SIG_IGN;
  sigemptyset(&ign_handler.sa_mask);
  ign_handler.sa_flags = 0;

  /* handle the follwing signals explicitly */
  sigaction(SIGINT, &es_handler_int, 0);

  /* ignore the following signals */
  sigaction(SIGPIPE, &ign_handler, 0);
  sigaction(SIGFPE, &ign_handler, 0);
}

static void signal_handler(int signum)
{
  ++ecaplay_skip_flag;
}
