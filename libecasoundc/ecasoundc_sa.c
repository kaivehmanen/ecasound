/** 
 * @file ecasoundc_sa.cpp Standalone C implementation of the 
 *                        ecasound control interface
 */

/* FIXME: change to LGPL...? */

/* FIXME: add a second state machine (magicword/2crlf) for
 *        recovery */

/** ------------------------------------------------------------------------
 * ecasoundc.cpp: Standalone C implementation of the 
 *                ecasound control interface
 * Copyright (C) 2000-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
 * Copyright (C) 2001 Aymeric Jeanneau (ajeanneau@cvf.fr)
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

#include <stdio.h>        /* ANSI-C: printf(), ... */
#include <stdlib.h>       /* ANSI-C: calloc(), free() */
#include <string.h>       /* ANSI-C: strlen() */

#include <sys/types.h>    /* POSIX: fork() */
#include <sys/wait.h>     /* POSIX: wait() */
#include <unistd.h>       /* POSIX: pipe(), fork() */

#include "ecasoundc.h"

/* --------------------------------------------------------------------- 
 * Definitions and constants
 */

#define ECI_PARSE_BUF_SIZE 4096

#define ECI_STATE_INIT             0
#define ECI_STATE_LOGLEVEL         1
#define ECI_STATE_MSGSIZE          2
#define ECI_STATE_COMMON_CR_1      3
#define ECI_STATE_COMMON_LF_1      4
#define ECI_STATE_RET_TYPE         5
#define ECI_STATE_COMMON_CONTENT   6
#define ECI_STATE_COMMON_CR_2      7
#define ECI_STATE_COMMON_LF_2      8
#define ECI_STATE_COMMON_CR_3      9
#define ECI_STATE_COMMON_LF_3      10

#define ECI_STATE_MSG_GEN          0
#define ECI_STATE_MSG_RETURN       1

#define ECI_RETURN_TYPE_LOGLEVEL   256

/* --------------------------------------------------------------------- 
 * Data structures 
 */

typedef struct { 

  int pid_of_child_rep;
  int pid_of_parent_rep;
  int cmd_read_fd_rep;
  int cmd_write_fd_rep;
  int state_rep;
  int state_msg_rep;

  double last_f_rep;
  long int last_li_rep;
  int last_i_rep;
  char* last_error_repp;
  char* last_type_repp;
  char** last_los_repp;
  char* last_s_repp;

  int msg_buffer_current_rep;
  int msg_buffer_marker_rep;

  char raw_buffer_repp[4096];
  char msg_buffer_repp[4096];

} eci_internal_t;

/* --------------------------------------------------------------------- 
 * Global variables
 */

static eci_handle_t static_eci_rep = 0;

/* --------------------------------------------------------------------- 
 * Declarations of static functions
 */

void eci_impl_read_return_value(eci_internal_t* eci_rep);
void eci_impl_update_state(eci_internal_t* eci_rep);

/* ---------------------------------------------------------------------
 * Constructing and destructing                                       
 */

/**
 * Initializes session. This call clears all status info and
 * prepares ecasound for processing. Can be used to "restart"
 * the library.
 */
void eci_init(void)
{
  static_eci_rep = eci_init_r();
}

/**
 * Initializes session. This call creates a new ecasoundinstance and
 * prepares ecasound for processing. 
 */
eci_handle_t eci_init_r(void)
{
  eci_internal_t* eci_rep = (eci_internal_t*)calloc(1, sizeof(eci_internal_t));
  int cmd_send_pipe[2], cmd_receive_pipe[2];
  char* ecasound_exec = getenv("ECASOUND");
  char* ecasound_extraparams = getenv("ECASOUND_PARAMS");

  /* initialize variables */
  eci_rep->msg_buffer_current_rep = 0;
  eci_rep->msg_buffer_marker_rep = -1;

  /* read environment variables */
  if (ecasound_exec == NULL) {
    ecasound_exec = "ecasound";
  }

  if (ecasound_extraparams == NULL) {
    ecasound_extraparams = "-d:255";
  }

  /* fork ecasound and setup two-way communication */
  if (pipe(cmd_receive_pipe) == 0 && pipe(cmd_send_pipe) == 0) {
    eci_rep->pid_of_child_rep = fork();
    if (eci_rep->pid_of_child_rep == 0) { 
      /* child */

      const char* args[8] = { ecasound_exec, "-c", ecasound_extraparams, NULL };
      int res = 0;


      /* close all unused descriptors */

      close(0);
      close(1);

      dup2(cmd_send_pipe[0], 0);
      dup2(cmd_receive_pipe[1], 1);

      close(cmd_receive_pipe[0]);
      close(cmd_receive_pipe[1]);
      close(cmd_send_pipe[0]);
      close(cmd_send_pipe[1]);

      /* freopen("/dev/null", "w", stderr); */

      fprintf(stderr, "(ecasound_sa) launching ecasound!\n");

      res = execvp(args[0], (char**)args);
      exit(res);
      fprintf(stderr, "(ecasound_sa) You shouldn't see this!\n");
    }
    else if (eci_rep->pid_of_child_rep > 0) { 
      /* parent */

      /* FIXME: add better wait! */
      sleep(3);

      eci_rep->pid_of_parent_rep = getpid();

      eci_rep->cmd_read_fd_rep = cmd_receive_pipe[0];
      eci_rep->cmd_write_fd_rep = cmd_send_pipe[1];

      write(eci_rep->cmd_write_fd_rep, "int-output-mode-wellformed\n", strlen("int-output-mode-wellformed\n"));

      /* FIXME: add wait for child */
    }
  }

  return (eci_handle_t)eci_rep;
}

/**
 * Frees all resources.
 */
void eci_cleanup(void)
{
  eci_cleanup_r(static_eci_rep);
}

/**
 * Frees all resources.
 */
void eci_cleanup_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  write(eci_rep->cmd_write_fd_rep, "quit\n", strlen("quit\n"));

  fprintf(stderr, "\n(ecasound_sa) cleaning up. waiting for children.\n");

  waitpid((pid_t)eci_rep->pid_of_child_rep, NULL, 0);

  fprintf(stderr, "(ecasound_sa) child exit signalled\n");

  if(eci_rep != 0) {
    /* if (eci_rep->eci != 0) {} */
    /* delete eci_rep->eci; */
    free(eci_rep);
  }
}

/* ---------------------------------------------------------------------
 * Issuing EIAM commands 
 */

/**
 * Sends a command to the ecasound engine. See ecasound-iam(5) for
 * more info.
 */
void eci_command(const char* command) { eci_command_r(static_eci_rep, command); }

/**
 * Sends a command to the ecasound engine. See ecasound-iam(5) for
 * more info.
 */
void eci_command_r(eci_handle_t ptr, const char* command)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  int res = 0;
  char buf[8192] = { 0 };

  fprintf(stderr, "\n(ecasound_sa) writing command '%s'.\n", command);

  sleep(1);

  write(eci_rep->cmd_write_fd_rep, command, strlen(command));
  write(eci_rep->cmd_write_fd_rep, "\n", 1);

  eci_impl_read_return_value(eci_rep);
}

/** 
 * A specialized version of 'eci_command()' taking a double value
 * as the 2nd argument.
 */
void eci_command_float_arg(const char* command, double arg) { eci_command_float_arg_r(static_eci_rep, command, arg); }

/** 
 * A specialized version of 'eci_command()' taking a double value
 * as the 2nd argument.
 */
void eci_command_float_arg_r(eci_handle_t ptr, const char* command, double arg)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  /* FIXME: 
   *  - send command to the ecasound instance
   *  - read the return value and recognize its type
   */
}

/* ---------------------------------------------------------------------
 * Getting return values 
 */

/**
 * Returns the number of strings returned by the 
 * last ECI command.
 */
int eci_last_string_list_count(void) { return(eci_last_string_list_count_r(static_eci_rep)); }

/**
 * Returns the number of strings returned by the 
 * last ECI command.
 */
int eci_last_string_list_count_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  /* FIXME: the last number of string */

  return 0;
}

/**
 * Returns the nth item of the list containing 
 * strings returned by the last ECI command.
 *
 * require:
 *  n >= 0 && n < eci_last_string_list_count()
 */
const char* eci_last_string_list_item(int n) { return(eci_last_string_list_item_r(static_eci_rep, n)); }

/**
 * Returns the nth item of the list containing 
 * strings returned by the last ECI command.
 *
 * require:
 *  n >= 0 && n < eci_last_string_list_count()
 */
const char* eci_last_string_list_item_r(eci_handle_t ptr, int n)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return NULL;
}

const char* eci_last_string(void) { return(eci_last_string_r(static_eci_rep)); }

const char* eci_last_string_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return NULL;
}

double eci_last_float(void) { return(eci_last_float_r(static_eci_rep)); }

double eci_last_float_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return 0.0f;
}

int eci_last_integer(void) { return(eci_last_integer_r(static_eci_rep)); }

int eci_last_integer_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return 0;
}

long int eci_last_long_integer(void) { return(eci_last_long_integer_r(static_eci_rep)); }

long int eci_last_long_integer_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return 0;
}

/**
 * Returns pointer to a null-terminated string containing 
 * information about the last occured error.
 */
const char* eci_last_error(void) { return(eci_last_error_r(static_eci_rep)); }

/**
 * Returns pointer to a null-terminated string containing 
 * information about the last occured error.
 */
const char* eci_last_error_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  
  /* FIXME: syntax to parse is "(eca-control) ERROR: <msg>" */

  return NULL;
}


const char* eci_last_type(void) { return(eci_last_type_r(static_eci_rep)); }

const char* eci_last_type_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return NULL;
}

/**
 * Whether an error has occured?
 */
int eci_error(void) { return(eci_error_r(static_eci_rep)); }

/**
 * Whether an error has occured?
 */
int eci_error_r(eci_handle_t ptr)
{ 
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  /* if (eci_rep->eci->error()) return(1); */
  /*   return(0); */
  return 0;
}
 
/* --------------------------------------------------------------------- 
 * Events 
 */

int eci_events_available(void) { return(eci_events_available_r(static_eci_rep)); }
int eci_events_available_r(eci_handle_t ptr) { return(0); }
void eci_next_event(void) { eci_next_event_r(static_eci_rep); }
void eci_next_event_r(eci_handle_t ptr) { }
const char* eci_current_event(void) { return(eci_current_event_r(static_eci_rep)); }
const char* eci_current_event_r(eci_handle_t ptr) { return(0); }

/* --------------------------------------------------------------------- 
 * Implementation of static functions
 */

void eci_impl_read_return_value(eci_internal_t* eci_rep)
{
  char* raw_buffer = eci_rep->raw_buffer_repp;
  char* msg_buffer = eci_rep->msg_buffer_repp;
  int* msgcur = &eci_rep->msg_buffer_current_rep;
  int res;

  res = read(eci_rep->cmd_read_fd_rep, raw_buffer, ECI_PARSE_BUF_SIZE-1);
  if (res > 0) {
    int n;

    raw_buffer[res] = 0;
    fprintf(stderr, "\n(ecasound_sa) read %u bytes:\n--cut--\n%s\n--cut--\n", res, raw_buffer);

    for(n = 0; n < res; n++) {
      int old = eci_rep->state_rep;
      msg_buffer[*msgcur] = raw_buffer[n];

      eci_impl_update_state(eci_rep);

      /* if (old != eci_rep->state_rep) fprintf(stderr, "state change %d-%d, c=[%02X].\n", old, eci_rep->state_rep, msg_buffer[*msgcur]); */

      *msgcur = (((*msgcur + 1) == ECI_PARSE_BUF_SIZE) ? 0 : *msgcur + 1);
    }

    fprintf(stderr, "(ecasound_sa) parse end\n");


  }
  else {
    fprintf(stderr, "\n(ecasound_sa) read error!\n");
  }
}

/**
 * Finds the start of next well-formed message.
 *
 * @param buffer pointer to the raw buffer
 * @param amount of data in the buffer
 *
 * @return NULL if no message found
 */
char* eci_impl_find_next_return_value(const char* buffer, int len)
{
}

void eci_impl_update_state(eci_internal_t* eci_rep)
{
  int* msgcur = &eci_rep->msg_buffer_current_rep;
  char c = eci_rep->msg_buffer_repp[eci_rep->msg_buffer_current_rep];

  switch(eci_rep->state_rep)
    {
    case ECI_STATE_INIT:
      if (c >= 0x30 && c <= 0x39) {
	eci_rep->state_rep = ECI_STATE_LOGLEVEL;
	eci_rep->msg_buffer_marker_rep =  eci_rep->msg_buffer_current_rep;
      }
      break;
	
    case ECI_STATE_LOGLEVEL:
      if (c == ' ') {
	int len = eci_rep->msg_buffer_current_rep - eci_rep->msg_buffer_marker_rep;
	int loglevel;
	char* buf;

	if (len < 0) len += ECI_PARSE_BUF_SIZE;
	buf = malloc(len + 1);

	memcpy(buf, &eci_rep->msg_buffer_repp[eci_rep->msg_buffer_marker_rep], len);
	buf[len] = 0;
	loglevel = atoi(buf);

	if (loglevel == ECI_RETURN_TYPE_LOGLEVEL) {
	  fprintf(stderr, "\n(ecasound_sa) found rettype loglevel '%s' (i=%d,len=%d).\n", buf, loglevel, len);
	  eci_rep->state_msg_rep = ECI_STATE_MSG_RETURN;
	}
	else 
	  eci_rep->state_msg_rep = ECI_STATE_MSG_GEN;
	  
	eci_rep->state_rep = ECI_STATE_MSGSIZE;
	eci_rep->msg_buffer_marker_rep =  -1;

	free(buf);
      }
      else if (c < 0x30 && c > 0x39) {
	eci_rep->state_rep = ECI_STATE_INIT;
      }

      break;

    case ECI_STATE_MSGSIZE:
      if (c == ' ' && eci_rep->state_msg_rep == ECI_STATE_MSG_RETURN ||
	  c == 0x0d && eci_rep->state_msg_rep == ECI_STATE_MSG_GEN) {
	int len = eci_rep->msg_buffer_current_rep - eci_rep->msg_buffer_marker_rep;
	int msgsize;
	char* buf;

	if (len < 0) len += ECI_PARSE_BUF_SIZE;
	buf = malloc(len + 1);

	memcpy(buf, &eci_rep->msg_buffer_repp[eci_rep->msg_buffer_marker_rep], len);
	buf[len] = 0;
	msgsize = atoi(buf);

	fprintf(stderr, "(ecasound_sa) found msgsize '%s' (i=%d,len=%d).\n", buf, msgsize, len);

	if (eci_rep->state_msg_rep == ECI_STATE_MSG_GEN)
	  eci_rep->state_rep = ECI_STATE_COMMON_CR_1;
	else
	  eci_rep->state_rep = ECI_STATE_RET_TYPE;

	free(buf);
      }
      else if (eci_rep->msg_buffer_marker_rep == -1) {
	eci_rep->msg_buffer_marker_rep =  eci_rep->msg_buffer_current_rep;
      }
      break;

    case ECI_STATE_COMMON_CR_1: 
      if (c == 0x0d) 
	eci_rep->state_rep = ECI_STATE_COMMON_LF_1;
      else
	eci_rep->state_rep = ECI_STATE_INIT;
      break;

    case ECI_STATE_COMMON_LF_1:
      if (c == 0x0a) {
	eci_rep->state_rep = ECI_STATE_COMMON_CONTENT;
	eci_rep->msg_buffer_marker_rep = -1;
      }
      else
	eci_rep->state_rep = ECI_STATE_INIT;
      break;

    case ECI_STATE_RET_TYPE:
      if (c == 0x0d) {
	/* parse return type */
	/* set 'eci_rep->last_type_repp' */
	eci_rep->state_rep = ECI_STATE_COMMON_LF_1;
      }
      break;

    case ECI_STATE_COMMON_CONTENT:
      if (c == 0x0d) {
	/* parse return type */
	/* set 'eci_rep->last_xxx_yyy' */
	eci_rep->state_rep = ECI_STATE_COMMON_LF_2;
      }
      else if (eci_rep->msg_buffer_marker_rep == -1) {
	eci_rep->msg_buffer_marker_rep =  eci_rep->msg_buffer_current_rep;
      }
      break;


    case ECI_STATE_COMMON_CR_2:
      if (c == 0x0d)
	eci_rep->state_rep = ECI_STATE_COMMON_LF_2; 
      else {
	eci_rep->state_rep = ECI_STATE_COMMON_CONTENT;
	eci_rep->msg_buffer_marker_rep = -1;
      }
      break;
	
    case ECI_STATE_COMMON_LF_2:
      if (c == 0x0a)
	eci_rep->state_rep = ECI_STATE_COMMON_CR_3; 
      else
	eci_rep->state_rep = ECI_STATE_COMMON_CONTENT;
      break;

    case ECI_STATE_COMMON_CR_3:
      if (c == 0x0d)
	eci_rep->state_rep = ECI_STATE_COMMON_LF_3; 
      else
	eci_rep->state_rep = ECI_STATE_COMMON_CONTENT;
      break;

    case ECI_STATE_COMMON_LF_3:
      if (c == 0x0a) {
	/* mark message as processed and start
	 * parsing the next message */
	/* eci_rep->msg_buffer_start_rep = eci_rep->msg_buffer_current_rep + 1; */
	*msgcur = (((*msgcur + 1) == ECI_PARSE_BUF_SIZE) ? 0 : *msgcur + 1);
	eci_rep->state_rep = ECI_STATE_INIT; 
      }
      else
	eci_rep->state_rep = ECI_STATE_COMMON_CONTENT;
      break;

    default: {}
    }
}
