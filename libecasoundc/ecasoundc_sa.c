/** 
 * @file ecasoundc_sa.cpp Standalone C implementation of the 
 *                        ecasound control interface
 */

/* FIXME: change to LGPL...? */
/* FIXME: implement list-of-string return type */
/* FIXME: get rid of the static string lengths (C... blaah ;)) */

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
 *
 * -------------------------------------------------------------------------
 * History: 
 *
 * 2002-10-04 Kai Vehmanen
 *     - Rewritten as a standalone implementation.
 * 2001-06-04 Aymeric Jeanneau
 *     - Added reentrant versions of all public ECI functions.
 * 2000-12-06 Kai Vehmanen
 *     - Initial version.
 *
 * -------------------------------------------------------------------------
 */

#include <assert.h>       /* ANSI-C: assert() */
#include <stdio.h>        /* ANSI-C: printf(), ... */
#include <stdlib.h>       /* ANSI-C: calloc(), free() */
#include <string.h>       /* ANSI-C: strlen() */

/* #include <fcntl.h> */        /* POSIX: fcntl() */
#include <unistd.h>       /* POSIX: pipe(), fork() */
#include <sys/types.h>    /* POSIX: fork() */
#include <sys/wait.h>     /* POSIX: wait() */

#include "ecasoundc.h"

/* --------------------------------------------------------------------- 
 * Definitions and constants
 */

#define ECI_MAX_PARSER_BUF_SIZE    4096
#define ECI_MAX_FLOAT_BUF_SIZE     32
#define ECI_MAX_RETURN_TYPE_SIZE   4
#define ECI_MAX_STRING_SIZE        4096
#define ECI_MAX_RESYNC_ATTEMPTS    9
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
#define ECI_STATE_SEEK_TO_LF       11

#define ECI_STATE_MSG_GEN          0
#define ECI_STATE_MSG_RETURN       1

#define ECI_TOKEN_PHASE_NONE       0
#define ECI_TOKEN_PHASE_READING    1
#define ECI_TOKEN_PHASE_VALIDATE   2

#define ECI_RETURN_TYPE_LOGLEVEL   256

/* --------------------------------------------------------------------- 
 * Data structures 
 */

typedef struct { 

  int state_rep;
  int state_msg_rep;

  double last_f_rep;
  long int last_li_rep;
  int last_i_rep;
  int last_counter_rep;
  char last_error_repp[ECI_MAX_STRING_SIZE];
  char last_type_repp[ECI_MAX_RETURN_TYPE_SIZE];
  char last_los_repp[ECI_MAX_STRING_SIZE][1];
  char last_s_repp[ECI_MAX_STRING_SIZE];

  int msgsize_rep;
  int loglevel_rep;

  int token_phase_rep;
  int buffer_current_rep;

  char buffer_repp[ECI_MAX_PARSER_BUF_SIZE];
} eci_parser_t;

typedef struct { 
  int pid_of_child_rep;
  int pid_of_parent_rep;
  int cmd_read_fd_rep;
  int cmd_write_fd_rep;

  int commands_counter_rep;

  eci_parser_t* parser_repp;

  char farg_buf_repp[ECI_MAX_FLOAT_BUF_SIZE];
  char raw_buffer_repp[ECI_MAX_PARSER_BUF_SIZE];
} eci_internal_t;

/* --------------------------------------------------------------------- 
 * Global variables
 */

static eci_handle_t static_eci_rep = 0;

/* --------------------------------------------------------------------- 
 * Declarations of static functions
 */

void eci_impl_clean_last_values(eci_parser_t* parser);
void eci_impl_read_return_value(eci_internal_t* eci_rep);
void eci_impl_set_last_values(eci_parser_t* parser);
void eci_impl_update_state(eci_parser_t* eci_rep, char c);

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
  eci_rep->parser_repp = (eci_parser_t*)calloc(1, sizeof(eci_parser_t));

  /* initialize variables */
  eci_rep->commands_counter_rep;
  eci_rep->parser_repp->last_counter_rep = 0;
  eci_rep->parser_repp->token_phase_rep = ECI_TOKEN_PHASE_NONE;
  eci_rep->parser_repp->buffer_current_rep = 0;
  eci_impl_clean_last_values(eci_rep->parser_repp);

  /* read environment variables */
  if (ecasound_exec == NULL) {
    ecasound_exec = "ecasound";
  }

  if (ecasound_extraparams == NULL) {
    /* default to only printing return-value messages */
    ecasound_extraparams = "-d:256";
  }

  /* fork ecasound and setup two-way communication */
  if (pipe(cmd_receive_pipe) == 0 && pipe(cmd_send_pipe) == 0) {
    eci_rep->pid_of_child_rep = fork();
    if (eci_rep->pid_of_child_rep == 0) { 
      /* child */

      /* -c = interactive mode, -D = direct prompts and banners to stderr */
      const char* args[8] = { ecasound_exec, "-c", "-D", ecasound_extraparams, NULL };
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

      /* fprintf(stderr, "(ecasound_sa) launching ecasound!\n"); */
      freopen("/dev/null", "w", stderr);

      res = execvp(args[0], (char**)args);
      exit(res);
      fprintf(stderr, "(ecasound_sa) You shouldn't see this!\n");
    }
    else if (eci_rep->pid_of_child_rep > 0) { 
      /* parent */

      /* FIXME: add better wait! */
      sleep(2);

      eci_rep->pid_of_parent_rep = getpid();

      eci_rep->cmd_read_fd_rep = cmd_receive_pipe[0];
      eci_rep->cmd_write_fd_rep = cmd_send_pipe[1];

      /* switch to non-blocking mode for read */
      /* fcntl(eci_rep->cmd_read_fd_rep, F_SETFL, O_NONBLOCK); */

      write(eci_rep->cmd_write_fd_rep, "int-output-mode-wellformed\n", strlen("int-output-mode-wellformed\n"));
      eci_rep->commands_counter_rep++;
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
  eci_rep->commands_counter_rep++;

  fprintf(stderr, "\n(ecasound_sa) cleaning up. waiting for children.\n");

  waitpid((pid_t)eci_rep->pid_of_child_rep, NULL, 0);

  fprintf(stderr, "(ecasound_sa) child exit signalled\n");

  if(eci_rep != 0) {
    /* if (eci_rep->eci != 0) {} */
    /* delete eci_rep->eci; */
    free(eci_rep->parser_repp);
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
  /* sleep(1); */

  eci_impl_clean_last_values(eci_rep->parser_repp);

  write(eci_rep->cmd_write_fd_rep, command, strlen(command));
  write(eci_rep->cmd_write_fd_rep, "\n", 1);
  eci_rep->commands_counter_rep++;

  if (eci_rep->commands_counter_rep - 1 !=
      eci_rep->parser_repp->last_counter_rep) {
    fprintf(stderr, 
	    "(ecasound_sa) sync error; cmd=%d lastv=%d.\n", 
	    eci_rep->commands_counter_rep,
	    eci_rep->parser_repp->last_counter_rep);
  }

  if (eci_rep->commands_counter_rep >=
      eci_rep->parser_repp->last_counter_rep) {
    eci_impl_read_return_value(eci_rep);
  }
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
  snprintf(eci_rep->farg_buf_repp, ECI_MAX_FLOAT_BUF_SIZE-1, "%s %.32f", command, arg);
  eci_command_r(ptr, eci_rep->farg_buf_repp);
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

  return 1;
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

  return eci_rep->parser_repp->last_los_repp[0];
}

const char* eci_last_string(void) { return(eci_last_string_r(static_eci_rep)); }

const char* eci_last_string_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return eci_rep->parser_repp->last_s_repp;
}

double eci_last_float(void) { return(eci_last_float_r(static_eci_rep)); }

double eci_last_float_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return eci_rep->parser_repp->last_f_rep;
}

int eci_last_integer(void) { return(eci_last_integer_r(static_eci_rep)); }

int eci_last_integer_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return eci_rep->parser_repp->last_i_rep;
}

long int eci_last_long_integer(void) { return(eci_last_long_integer_r(static_eci_rep)); }

long int eci_last_long_integer_r(eci_handle_t ptr)
{
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;

  return eci_rep->parser_repp->last_li_rep;
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

void eci_impl_clean_last_values(eci_parser_t* parser)
{
  assert(parser != 0);

  memset(parser->last_s_repp, 0, ECI_MAX_STRING_SIZE);
  memset(parser->last_los_repp, 0, ECI_MAX_STRING_SIZE);
  parser->last_i_rep = 0;
  parser->last_li_rep = 0;
  parser->last_f_rep = 0.0f;
  memset(parser->last_error_repp, 0, ECI_MAX_STRING_SIZE);
}

void eci_impl_read_return_value(eci_internal_t* eci_rep)
{
  char* raw_buffer = eci_rep->raw_buffer_repp;
  int attempts = 0;

  while(attempts < ECI_MAX_RESYNC_ATTEMPTS) {
    int res = read(eci_rep->cmd_read_fd_rep, raw_buffer, ECI_MAX_PARSER_BUF_SIZE-1);
    if (res > 0) {
      int n;

      raw_buffer[res] = 0;
      /* fprintf(stderr, "\n(ecasound_sa) read %u bytes:\n--cut--\n%s\n--cut--\n", res, raw_buffer); */

      for(n = 0; n < res; n++) {
	/* int old = eci_rep->parser_repp->state_rep; */
	eci_impl_update_state(eci_rep->parser_repp, raw_buffer[n]);
	/* if (old != eci_rep->parser_repp->state_rep) fprintf(stderr, "state change %d-%d, c=[%02X].\n", old, eci_rep->parser_repp->state_rep, raw_buffer[n]); */
      }

      if (eci_rep->commands_counter_rep ==
	  eci_rep->parser_repp->last_counter_rep) break;

      /* read return values until the correct one is found */
      ++attempts;
    }
    else {
      fprintf(stderr, "\n(ecasound_sa) Warning! read() error!\n");
      break;
    }
  }
}

/**
 * Sets the 'last value' fields in the given 'parser'
 * object.
 *
 * @pre parser != 0
 * @pre parser->state_rep == ECI_STATE_COMMON_LF_3
 */
void eci_impl_set_last_values(eci_parser_t* parser)
{
  assert(parser != 0);
  assert(parser->state_rep == ECI_STATE_COMMON_LF_3);

  switch(parser->last_type_repp[0])
    {
    case 's':
      memcpy(parser->last_s_repp, parser->buffer_repp, parser->buffer_current_rep);
      break;

    case 'S':
      memcpy(parser->last_los_repp, parser->buffer_repp, parser->buffer_current_rep);
      break;

    case 'i':
      parser->last_i_rep = atoi(parser->buffer_repp);
      break;

    case 'l':
      parser->last_i_rep = atol(parser->buffer_repp);
      break;

    case 'f':
      parser->last_f_rep = atof(parser->buffer_repp);
      break;

    case 'e':
      memcpy(parser->last_error_repp, parser->buffer_repp, parser->buffer_current_rep);
      break;

    default: {}

    }
}

void eci_impl_update_state(eci_parser_t* parser, char c)
{
  char* msg_buffer = parser->buffer_repp;
  int* msgcur = &parser->buffer_current_rep;

  switch(parser->state_rep)
    {
    case ECI_STATE_INIT:
      if (c >= 0x30 && c <= 0x39) {
	parser->token_phase_rep = ECI_TOKEN_PHASE_READING;
	parser->buffer_current_rep = 0;
	parser->state_rep = ECI_STATE_LOGLEVEL;
      }
      else {
	parser->token_phase_rep = ECI_TOKEN_PHASE_NONE;
      }
      break;

    case ECI_STATE_LOGLEVEL:
      if (c == ' ') {
	parser->buffer_repp[parser->buffer_current_rep] = 0;
	parser->loglevel_rep = atoi(parser->buffer_repp);

	if (parser->loglevel_rep == ECI_RETURN_TYPE_LOGLEVEL) {
	  /* fprintf(stderr, "\n(ecasound_sa) found rettype loglevel '%s' (i=%d,len=%d).\n", parser->buffer_repp, parser->loglevel_rep, parser->buffer_current_rep); */
	  parser->state_msg_rep = ECI_STATE_MSG_RETURN;
	}
	else {
	  /* fprintf(stderr, "\n(ecasound_sa) found loglevel '%s' (i=%d,parser->buffer_current_rep=%d).\n", buf, parser->loglevel_rep, parser->buffer_current_rep); */
	  parser->state_msg_rep = ECI_STATE_MSG_GEN;
	}
	  
	parser->state_rep = ECI_STATE_MSGSIZE;
	parser->token_phase_rep =  ECI_TOKEN_PHASE_NONE;
      }
      else if (c < 0x30 && c > 0x39) {
	parser->state_rep = ECI_STATE_SEEK_TO_LF;
      }

      break;

    case ECI_STATE_MSGSIZE:
      if (c == ' ' && parser->state_msg_rep == ECI_STATE_MSG_RETURN ||
	  c == 0x0d && parser->state_msg_rep == ECI_STATE_MSG_GEN) {

	parser->buffer_repp[parser->buffer_current_rep] = 0;
	parser->msgsize_rep = atoi(parser->buffer_repp);

	/* fprintf(stderr, "(ecasound_sa) found msgsize '%s' (i=%d,len=%d).\n", parser->buffer_repp, parser->msgsize_rep, parser->buffer_current_rep); */
	if (parser->state_msg_rep == ECI_STATE_MSG_GEN) {
	  parser->state_rep = ECI_STATE_COMMON_LF_1;
	}
	else {
	  parser->state_rep = ECI_STATE_RET_TYPE;
	}

	parser->token_phase_rep =  ECI_TOKEN_PHASE_NONE;
  
      }
      else if (c < 0x30 && c > 0x39) {
	parser->state_rep = ECI_STATE_SEEK_TO_LF;
      }
      else if (parser->token_phase_rep == ECI_TOKEN_PHASE_NONE) {
	parser->token_phase_rep =  ECI_TOKEN_PHASE_READING;
	parser->buffer_current_rep = 0;
      }
      break;

    case ECI_STATE_COMMON_CR_1: 
      if (c == 0x0d) 
	parser->state_rep = ECI_STATE_COMMON_LF_1;
      else
	parser->state_rep = ECI_STATE_INIT;
      break;

    case ECI_STATE_COMMON_LF_1:
      if (c == 0x0a) {
	parser->state_rep = ECI_STATE_COMMON_CONTENT;
      }
      else
	parser->state_rep = ECI_STATE_INIT;
      break;

    case ECI_STATE_RET_TYPE:
      if (c == 0x0d) {
	/* parse return type */
	/* set 'parser->last_type_repp' */
	int len = (parser->buffer_current_rep < ECI_MAX_RETURN_TYPE_SIZE) ? parser->buffer_current_rep : (ECI_MAX_RETURN_TYPE_SIZE - 1);
	parser->buffer_repp[parser->buffer_current_rep] = 0;
	memcpy(parser->last_type_repp, parser->buffer_repp, len);
	parser->last_type_repp[ECI_MAX_RETURN_TYPE_SIZE - 1] = 0;
	
	/* fprintf(stderr, "(ecasound_sa) found rettype '%s' (len=%d).\n", parser->last_type_repp, parser->buffer_current_rep); */

	parser->state_rep = ECI_STATE_COMMON_LF_1;
	parser->token_phase_rep =  ECI_TOKEN_PHASE_NONE;

      }
      else if (parser->token_phase_rep == ECI_TOKEN_PHASE_NONE) {
	parser->token_phase_rep =  ECI_TOKEN_PHASE_READING;
	parser->buffer_current_rep = 0;
      }

      break;

    case ECI_STATE_COMMON_CONTENT:
      if (c == 0x0d) {
	/* parse return type */
	/* set 'parser->last_xxx_yyy' */

	parser->buffer_repp[parser->buffer_current_rep] = 0;

#if 0
	fprintf(stderr, "(ecasound_sa) found content, loglevel=%d, msgsize=%d", parser->loglevel_rep, parser->msgsize_rep);
	if (parser->state_msg_rep == ECI_STATE_MSG_GEN)
	  fprintf(stderr, ".\n");
	else
	  fprintf(stderr, " type='%s'.\n", parser->last_type_repp);
#endif

	parser->state_rep = ECI_STATE_COMMON_LF_2;
	parser->token_phase_rep =  ECI_TOKEN_PHASE_VALIDATE;

      }
      else if (parser->token_phase_rep == ECI_TOKEN_PHASE_NONE) {
	parser->token_phase_rep = ECI_TOKEN_PHASE_READING;
	parser->buffer_current_rep = 0;
      }
      break;


    case ECI_STATE_COMMON_CR_2:
      if (c == 0x0d)
	parser->state_rep = ECI_STATE_COMMON_LF_2; 
      else
	parser->state_rep = ECI_STATE_COMMON_CONTENT;
      break;
	
    case ECI_STATE_COMMON_LF_2:
      if (c == 0x0a)
	parser->state_rep = ECI_STATE_COMMON_CR_3; 
      else
	parser->state_rep = ECI_STATE_COMMON_CONTENT;
      break;

    case ECI_STATE_COMMON_CR_3:
      if (c == 0x0d)
	parser->state_rep = ECI_STATE_COMMON_LF_3; 
      else
	parser->state_rep = ECI_STATE_COMMON_CONTENT;
      break;

    case ECI_STATE_COMMON_LF_3:
      if (c == 0x0a) {
	if (parser->state_msg_rep == ECI_STATE_MSG_RETURN) {
	  fprintf(stderr, "(ecasound_sa) rettype-content validated: <<< %s >>>\n", parser->buffer_repp);
	  eci_impl_set_last_values(parser);
	  parser->last_counter_rep++;
	}
	else {
	  /* fprintf(stderr, "(ecasound_sa) gen-content validated: <<< %s >>>\n", parser->buffer_repp); */
	}
	parser->state_rep = ECI_STATE_INIT; 
      }
      else
	parser->state_rep = ECI_STATE_COMMON_CONTENT;
      break;

    case ECI_STATE_SEEK_TO_LF: 
      if (c == 0x0a) {
	parser->token_phase_rep = ECI_TOKEN_PHASE_NONE;
	parser->state_rep = ECI_STATE_INIT;
      }
      break;

    default: {}

    } /* end of switch() */

  if (parser->token_phase_rep == ECI_TOKEN_PHASE_READING) {
    msg_buffer[parser->buffer_current_rep] = c;
    if (++parser->buffer_current_rep == ECI_MAX_PARSER_BUF_SIZE) {
      fprintf(stderr, "(ecasound_sa) Warnign! Parsing buffer overflowed!\n");
    }
  }
}
