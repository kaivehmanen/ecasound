/** 
 * @file ecasoundc_sa.cpp Standalone C implementation of the 
 *                        ecasound control interface
 */

/* FIXME: add check for big sync-error -> ecasound probably 
 *        died so better to give an error */
/* FIXME: add check for msgsize errors */
/* FIXME: get rid of the static string lengths (C... blaah ;)) */
/* FIXME: add documentation for ECASOUND envvar */
/* FIXME: add proper signal handling */

/** ------------------------------------------------------------------------
 * ecasoundc.cpp: Standalone C implementation of the 
 *                ecasound control interface
 * Copyright (C) 2000-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
 * Copyright (C) 2001 Aymeric Jeanneau (ajeanneau@cvf.fr)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include <fcntl.h>        /* POSIX: fcntl() */
#include <sys/poll.h>     /* XPG4-UNIX: poll() */
#include <unistd.h>       /* POSIX: pipe(), fork() */
#include <sys/stat.h>     /* POSIX: stat() */
#include <sys/types.h>    /* POSIX: fork() */
#include <sys/wait.h>     /* POSIX: wait() */

#include "ecasoundc.h"

/* --------------------------------------------------------------------- 
 * Options
 */

/* #define ECI_ENABLE_DEBUG */

/* --------------------------------------------------------------------- 
 * Definitions and constants
 */

#define ECI_MAX_PARSER_BUF_SIZE    4096
#define ECI_MAX_FLOAT_BUF_SIZE     32
#define ECI_MAX_RETURN_TYPE_SIZE   4
#define ECI_MAX_STRING_SIZE        ECI_MAX_PARSER_BUF_SIZE
#define ECI_MAX_RESYNC_ATTEMPTS    9

#define ECI_READ_TIMEOUT_MS        5000 /* 2000 */

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

#ifdef ECI_ENABLE_DEBUG
#define ECI_DEBUG(x) printf(x)
#define ECI_DEBUG_1(x,y) printf(x,y)
#define ECI_DEBUG_2(x,y,z) printf(x,y,z)
#define ECI_DEBUG_3(x,y,z,t) printf(x,y,z,t)
#else
#define ECI_DEBUG(x) ((void) 0)
#define ECI_DEBUG_1(x,y) ((void) 0)
#define ECI_DEBUG_2(x,y,z) ((void) 0)
#define ECI_DEBUG_3(x,y,z,t) ((void) 0)
#endif

/* --------------------------------------------------------------------- 
 * Data structures 
 */

struct eci_los_list {
  char* data_repp;
  struct eci_los_list* prev_repp;
  struct eci_los_list* next_repp;
};

struct eci_parser { 

  int state_rep;
  int state_msg_rep;

  double last_f_rep;
  long int last_li_rep;
  int last_i_rep;
  int last_counter_rep;
  char last_error_repp[ECI_MAX_STRING_SIZE];
  char last_type_repp[ECI_MAX_RETURN_TYPE_SIZE];
  struct eci_los_list* last_los_repp;
  /* char* last_los_repp[ECI_MAX_STRING_SIZE]; */
  char last_s_repp[ECI_MAX_STRING_SIZE];

  int msgsize_rep;
  int loglevel_rep;

  int token_phase_rep;
  int buffer_current_rep;

  char buffer_repp[ECI_MAX_PARSER_BUF_SIZE];
};

struct eci_internal { 
  int pid_of_child_rep;
  int pid_of_parent_rep;
  int cmd_read_fd_rep;
  int cmd_write_fd_rep;

  int commands_counter_rep;

  struct eci_parser* parser_repp;

  char farg_buf_repp[ECI_MAX_FLOAT_BUF_SIZE];
  char raw_buffer_repp[ECI_MAX_PARSER_BUF_SIZE];
};

/* --------------------------------------------------------------------- 
 * Global variables
 */

static eci_handle_t static_eci_rep = 0;

/**
 * Message shown if ECASOUND is not defined.
 */
const char* eci_str_no_ecasound_env = 
    "\n"
    "***********************************************************************\n"
    "* Message from libecasoundc:\n"
    "* \n"
    "* 'ECASOUND' environment variable not set. Using the default value \n"
    "* value 'ECASOUND=ecasound'.\n"
    "***********************************************************************\n"
    "\n";

const char* eci_str_null_handle = 
    "\n"
    "***********************************************************************\n"
    "* Message from libecasoundc:\n"
    "* \n"
    "* A null client handle detected. This is usually caused by a bug \n"
    "* in the ECI application. Please report this bug to the author of\n"
    "* the program.\n"
    "***********************************************************************\n"
    "\n";

const char* eci_str_sync_lost =
    "\n"
    "***********************************************************************\n"
    "* Message from libecasoundc:\n"
    "* \n"
    "* Connection to the processing engine was lost. Check that ecasound \n"
    "* is correctly installed. Also make sure that ecasound is either \n"
    "* in some directory listed in PATH, or the environment variable\n"
    "* 'ECASOUND' contains the path to a working ecasound executable.\n"
    "***********************************************************************\n"
    "\n";

/* --------------------------------------------------------------------- 
 * Declarations of static functions
 */

void eci_impl_check_handle(struct eci_internal* eci_rep);
void eci_impl_clean_last_values(struct eci_parser* parser);
ssize_t eci_impl_fd_read(int fd, void *buf, size_t count, int timeout);
const char* eci_impl_get_ecasound_path(void);
void eci_impl_los_list_add_item(struct eci_los_list** i, char* stmp, int len);
void eci_impl_los_list_alloc_item(struct eci_los_list **ptr);
void eci_impl_los_list_clear(struct eci_los_list **ptr);
void eci_impl_read_return_value(struct eci_internal* eci_rep, int timeout);
void eci_impl_set_last_los_value(struct eci_parser* parser);
void eci_impl_set_last_values(struct eci_parser* parser);
void eci_impl_update_state(struct eci_parser* eci_rep, char c);

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
  assert(static_eci_rep == NULL);
  static_eci_rep = eci_init_r();
}

/**
 * Initializes session. This call creates a new ecasoundinstance and
 * prepares ecasound for processing. 
 */
eci_handle_t eci_init_r(void)
{
  struct eci_internal* eci_rep = NULL;
  int cmd_send_pipe[2], cmd_receive_pipe[2];
  const char* ecasound_exec = eci_impl_get_ecasound_path();

  /* fork ecasound and setup two-way communication */
  if (ecasound_exec != NULL &&
      (pipe(cmd_receive_pipe) == 0 && pipe(cmd_send_pipe) == 0)) {
    eci_rep = (struct eci_internal*)calloc(1, sizeof(struct eci_internal));
    eci_rep->parser_repp = (struct eci_parser*)calloc(1, sizeof(struct eci_parser));

    /* initialize variables */
    eci_rep->commands_counter_rep = 0;
    eci_rep->parser_repp->last_counter_rep = 0;
    eci_rep->parser_repp->token_phase_rep = ECI_TOKEN_PHASE_NONE;
    eci_rep->parser_repp->buffer_current_rep = 0;
    eci_impl_clean_last_values(eci_rep->parser_repp);

    eci_rep->pid_of_child_rep = fork();
    if (eci_rep->pid_of_child_rep == 0) { 
      /* child */

      /* -c = interactive mode, -D = direct prompts and banners to stderr */
      const char* args[4] = { NULL, "-c", "-D", NULL };
      int res = 0;

      args[0] = ecasound_exec;

      /* close all unused descriptors */

      close(0);
      close(1);

      dup2(cmd_send_pipe[0], 0);
      dup2(cmd_receive_pipe[1], 1);

      close(cmd_receive_pipe[0]);
      close(cmd_receive_pipe[1]);
      close(cmd_send_pipe[0]);
      close(cmd_send_pipe[1]);

      freopen("/dev/null", "w", stderr);
      
      /* notify the parent that we're up */
      res = write(1, args, 1); 

      res = execvp(args[0], (char**)args);
      if (res < 0) printf("(ecasoundc_sa) launcing ecasound FAILED!\n");
      
      close(0);
      close(1);

      exit(res);
      ECI_DEBUG("(ecasoundc_sa) You shouldn't see this!\n");
    }
    else if (eci_rep->pid_of_child_rep > 0) { 
      /* parent */
      int res;
      char buf[1];

      eci_rep->pid_of_parent_rep = getpid();

      eci_rep->cmd_read_fd_rep = cmd_receive_pipe[0];
      eci_rep->cmd_write_fd_rep = cmd_send_pipe[1];

      /* switch to non-blocking mode for read */
      fcntl(eci_rep->cmd_read_fd_rep, F_SETFL, O_NONBLOCK);

      /* check that fork succeeded() */
      res = eci_impl_fd_read(eci_rep->cmd_read_fd_rep, buf, 1, ECI_READ_TIMEOUT_MS);
      if (res != 1) {
	ECI_DEBUG_1("(ecasoundc_sa) fork() of %s FAILED!\n", ecasound_exec);
	free(eci_rep->parser_repp);
	free(eci_rep);
	eci_rep = NULL;
      }
      else {
	write(eci_rep->cmd_write_fd_rep, "debug 259\n", strlen("debug 259\n"));
	write(eci_rep->cmd_write_fd_rep, "int-output-mode-wellformed\n", strlen("int-output-mode-wellformed\n"));
	eci_rep->commands_counter_rep ++;
      
	/* check that exec() succeeded */
	eci_impl_read_return_value(eci_rep, ECI_READ_TIMEOUT_MS);
	if (eci_rep->commands_counter_rep != eci_rep->parser_repp->last_counter_rep) {
	  ECI_DEBUG_3("(ecasoundc_sa) exec() of %s FAILED (%d=%d)!\n", ecasound_exec, eci_rep->commands_counter_rep, eci_rep->parser_repp->last_counter_rep);
	  free(eci_rep->parser_repp);
	  free(eci_rep);
	  eci_rep = NULL;
	}
      }
    }
  }

  return (eci_handle_t)eci_rep;
}

/**
 * Frees all resources.
 */
void eci_cleanup(void)
{
  if (static_eci_rep != NULL) {
    eci_cleanup_r(static_eci_rep);
    static_eci_rep = NULL;
  }
}

/**
 * Frees all resources.
 */
void eci_cleanup_r(eci_handle_t ptr)
{
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);

  write(eci_rep->cmd_write_fd_rep, "quit\n", strlen("quit\n"));
  eci_rep->commands_counter_rep++;
  
  /* kill((pid_t)eci_rep->pid_of_child_rep, SIGKILL); */

  ECI_DEBUG("\n(ecasoundc_sa) cleaning up. waiting for children.\n");

  waitpid((pid_t)eci_rep->pid_of_child_rep, NULL, 0);

  ECI_DEBUG("(ecasoundc_sa) child exit signalled\n");

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
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;
  int timeout = ECI_READ_TIMEOUT_MS;

  eci_impl_check_handle(eci_rep);

  ECI_DEBUG_2("\n(ecasoundc_sa) writing command '%s' (cmd-counter=%d).\n", 
	      command, eci_rep->commands_counter_rep + 1);

  eci_impl_clean_last_values(eci_rep->parser_repp);

  write(eci_rep->cmd_write_fd_rep, command, strlen(command));
  write(eci_rep->cmd_write_fd_rep, "\n", 1);

  /* 'run' is the only blocking function */
  if (strncmp(command, "run", 3) != 0) {
    timeout = -1;
  }

  eci_rep->commands_counter_rep++;
    
  if (eci_rep->commands_counter_rep - 1 !=
      eci_rep->parser_repp->last_counter_rep) {
    ECI_DEBUG_2("(ecasoundc_sa) sync error; cmd=%d lastv=%d.\n", 
		eci_rep->commands_counter_rep,
		eci_rep->parser_repp->last_counter_rep);
  }
  
  if (eci_rep->commands_counter_rep >=
      eci_rep->parser_repp->last_counter_rep) {
    eci_impl_read_return_value(eci_rep, timeout);
  }

  ECI_DEBUG_2("\n(ecasoundc_sa) set return value type='%s' (read-counter=%d).\n", 
	      eci_rep->parser_repp->last_type_repp, eci_rep->parser_repp->last_counter_rep);
  
  if (eci_rep->commands_counter_rep >
      eci_rep->parser_repp->last_counter_rep) {
    fprintf(stderr, "%s", eci_str_sync_lost);
    eci_cleanup_r(eci_rep);
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
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);

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
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;
  struct eci_los_list* i;
  int count = 0;

  eci_impl_check_handle(eci_rep);

  for(i = eci_rep->parser_repp->last_los_repp; 
      i != NULL; 
      i = i->next_repp) {
    ++count;
  }

  return count;
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
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;
  struct eci_los_list* i;
  int count = 0;

  eci_impl_check_handle(eci_rep);

  for(i = eci_rep->parser_repp->last_los_repp;  
      i != NULL; 
      i = i->next_repp) {
    if (count++ == n) {
      return i->data_repp;
    }
  }

  return NULL;
}

const char* eci_last_string(void) { return(eci_last_string_r(static_eci_rep)); }

const char* eci_last_string_r(eci_handle_t ptr)
{
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);

  return eci_rep->parser_repp->last_s_repp;
}

double eci_last_float(void) { return(eci_last_float_r(static_eci_rep)); }

double eci_last_float_r(eci_handle_t ptr)
{
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);

  return eci_rep->parser_repp->last_f_rep;
}

int eci_last_integer(void) { return(eci_last_integer_r(static_eci_rep)); }

int eci_last_integer_r(eci_handle_t ptr)
{
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);

  return eci_rep->parser_repp->last_i_rep;
}

long int eci_last_long_integer(void) { return(eci_last_long_integer_r(static_eci_rep)); }

long int eci_last_long_integer_r(eci_handle_t ptr)
{
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);

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
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);
  
  return eci_rep->parser_repp->last_error_repp;
}


const char* eci_last_type(void) { return(eci_last_type_r(static_eci_rep)); }

const char* eci_last_type_r(eci_handle_t ptr)
{
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;

  eci_impl_check_handle(eci_rep);

  return eci_rep->parser_repp->last_type_repp;
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
  struct eci_internal* eci_rep = (struct eci_internal*)ptr;
  int res;

  eci_impl_check_handle(eci_rep);

  res = (eci_rep->parser_repp->last_type_repp[0] == 'e') ? 1 : 0;

  return res;
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

void eci_impl_check_handle(struct eci_internal* eci_rep)
{
  if (eci_rep == NULL) {
    fprintf(stderr, "%s", eci_str_null_handle);
    assert(eci_rep != NULL);
    exit(-1);
  }
}

void eci_impl_clean_last_values(struct eci_parser* parser)
{
  assert(parser != 0);

  memset(parser->last_s_repp, 0, ECI_MAX_STRING_SIZE);
  parser->last_los_repp = NULL;
  parser->last_i_rep = 0;
  parser->last_li_rep = 0;
  parser->last_f_rep = 0.0f;
  memset(parser->last_error_repp, 0, ECI_MAX_STRING_SIZE);
}

/**
 * Attempts to read up to 'count' bytes from file descriptor 'fd' 
 * into the buffer starting at 'buf'. If no data is available
 * for reading, up to 'timeout' milliseconds will be waited. 
 * A negative value means infinite timeout.
 */
ssize_t eci_impl_fd_read(int fd, void *buf, size_t count, int timeout)
{
  int nfds = 1;
  struct pollfd ufds;
  ssize_t rescount = 0;
  int ret;

  ufds.fd = fd;
  ufds.events = POLLIN | POLLPRI;
  ufds.revents = 0;
  
  ret = poll(&ufds, nfds, timeout);
  if (ret > 0) {
    if (ufds.revents & POLLIN ||
	ufds.revents & POLLPRI) {
      rescount = read(fd, buf, count);
    }
  }
  else if (ret == 0) {
    /* timeout */
    rescount = -1;
  }
  return rescount;
}

const char* eci_impl_get_ecasound_path(void)
{
  const char* result = getenv("ECASOUND");

  if (result == NULL) {
    fprintf(stderr, "%s", eci_str_no_ecasound_env);
    result = "ecasound";
  }

  return result;
}


void eci_impl_los_list_add_item(struct eci_los_list** headptr, char* stmp, int len)
{
  struct eci_los_list* i = *headptr;
  struct eci_los_list* prev = NULL;
  
  if (len >= ECI_MAX_STRING_SIZE) {
    fprintf(stderr, "(ecasoundc_sa) WARNING! String list buffer overflowed!\n\n");
    len = ECI_MAX_STRING_SIZE - 1;
  }

  stmp[len] = 0;
  /* ECI_DEBUG_1("(ecasoundc_sa) adding item '%s' to los list\n", stmp); */

  while(1) {
    if (i == NULL) {
      eci_impl_los_list_alloc_item(&i);
      if (prev != NULL) prev->next_repp = i;
      if (*headptr == NULL) *headptr = i;

      memcpy(i->data_repp, stmp, len + 1);
      /* ECI_DEBUG_1("(ecasoundc_sa) copied %d bytes to %p.\n", len, i->data_repp); */

      break;
    }

    prev = i;
    i = i->next_repp;
  }
}

void eci_impl_los_list_alloc_item(struct eci_los_list **ptr)
{
  *ptr = (struct eci_los_list*)malloc(sizeof(struct eci_los_list*));
  (*ptr)->data_repp = (char*)malloc(ECI_MAX_STRING_SIZE);
  (*ptr)->next_repp = NULL;
}

void eci_impl_los_list_clear(struct eci_los_list **ptr)
{
  struct eci_los_list *i = *ptr;

  while(i != NULL) {
    struct eci_los_list* next = i->next_repp;

    if (i->data_repp) {
      {
	if (next->data_repp != NULL) {
	  /* ECI_DEBUG_1("(ecasoundc_sa) removing item '%s' from los list\n", i->data_repp); */
	}
      }
      free(i->data_repp);
    }
    free(i);

    i = next;
  }

  *ptr = NULL;
}

void eci_impl_read_return_value(struct eci_internal* eci_rep, int timeout)
{
  char* raw_buffer = eci_rep->raw_buffer_repp;
  int attempts = 0;

  assert(eci_rep->commands_counter_rep >=
	 eci_rep->parser_repp->last_counter_rep);

  while(attempts < ECI_MAX_RESYNC_ATTEMPTS) {
    int res = eci_impl_fd_read(eci_rep->cmd_read_fd_rep, raw_buffer, ECI_MAX_PARSER_BUF_SIZE-1, timeout);
    if (res > 0) {
      int n;

      raw_buffer[res] = 0;
      /* ECI_DEBUG_2("\n(ecasoundc_sa) read %u bytes:\n--cut--\n%s\n--cut--\n", res, raw_buffer); */

      for(n = 0; n < res; n++) {
	/* int old = eci_rep->parser_repp->state_rep; */
	eci_impl_update_state(eci_rep->parser_repp, raw_buffer[n]);
	/* if (old != eci_rep->parser_repp->state_rep) ECI_DEBUG_3("state change %d-%d, c=[%02X].\n", old, eci_rep->parser_repp->state_rep, raw_buffer[n]); */
      }

      if (eci_rep->commands_counter_rep ==
	  eci_rep->parser_repp->last_counter_rep) break;

      /* read return values until the correct one is found */
    }
    else {
      if (res < 0) {
	ECI_DEBUG_1("(ecasoundc_sa) timeout when reading return values (attempts=%d)!\n", attempts);
	break;
      }
    }
    ++attempts;
  }

  if (eci_rep->commands_counter_rep !=
      eci_rep->parser_repp->last_counter_rep) {
    ECI_DEBUG("\n(ecasoundc_sa) Warning! read() error!\n");
  }
}

/**
 * Sets the last 'list of strings' values.
 *
 * @pre parser != 0
 * @pre parser->state_rep == ECI_STATE_COMMON_LF_3
 */
void eci_impl_set_last_los_value(struct eci_parser* parser)
{
  struct eci_los_list** i = &parser->last_los_repp;
  int quoteflag = 0, m = 0, n;
  char* stmp = malloc(ECI_MAX_STRING_SIZE);

  assert(parser != 0);
  assert(parser->state_rep == ECI_STATE_COMMON_LF_3);

  /* ECI_DEBUG_2("(ecasoundc_sa) parsing a list '%s' (count=%d)\n", parser->buffer_repp, parser->buffer_current_rep); */

  eci_impl_los_list_clear(i);

  for(n = 0; n < parser->buffer_current_rep && n < parser->msgsize_rep; n++) {
    char c = parser->buffer_repp[n];

    if (c == '\"') {
      quoteflag = !quoteflag;
    }
    else if (c == '\\') {
      n++;
      c = parser->buffer_repp[n];
      stmp[m++] = c;
    }
    else if (c != ',' || quoteflag == 1) {
      stmp[m++] = c;
    }
    else {
      if (m == 0) continue;
      eci_impl_los_list_add_item(i, stmp, m);
      m = 0;
    }
  }
  if (m > 0) {
    eci_impl_los_list_add_item(i, stmp, m);
  }

  /* delete tmp char-buffer */
  free(stmp);
}

/**
 * Sets the 'last value' fields in the given 'parser'
 * object.
 *
 * @pre parser != 0
 * @pre parser->state_rep == ECI_STATE_COMMON_LF_3
 */
void eci_impl_set_last_values(struct eci_parser* parser)
{
  assert(parser != 0);
  assert(parser->state_rep == ECI_STATE_COMMON_LF_3);

  switch(parser->last_type_repp[0])
    {
    case 's':
      memcpy(parser->last_s_repp, parser->buffer_repp, parser->buffer_current_rep);
      break;

    case 'S': {
      eci_impl_set_last_los_value(parser);
      break;
    }

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

void eci_impl_update_state(struct eci_parser* parser, char c)
{
  char* msg_buffer = parser->buffer_repp;

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
	  /* ECI_DEBUG_3("\n(ecasoundc_sa) found rettype loglevel '%s' (i=%d,len=%d).\n", parser->buffer_repp, parser->loglevel_rep, parser->buffer_current_rep); */
	  parser->state_msg_rep = ECI_STATE_MSG_RETURN;
	}
	else {
	  /* ECI_DEBUG_3("\n(ecasoundc_sa) found loglevel '%s' (i=%d,parser->buffer_current_rep=%d).\n", buf, parser->loglevel_rep, parser->buffer_current_rep); */
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
      if ((c == ' ' && parser->state_msg_rep == ECI_STATE_MSG_RETURN) ||
	  (c == 0x0d && parser->state_msg_rep == ECI_STATE_MSG_GEN)) {

	parser->buffer_repp[parser->buffer_current_rep] = 0;
	parser->msgsize_rep = atoi(parser->buffer_repp);

	/* ECI_DEBUG_3("(ecasoundc_sa) found msgsize '%s' (i=%d,len=%d).\n", parser->buffer_repp, parser->msgsize_rep, parser->buffer_current_rep); */
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
	
	/* ECI_DEBUG_2("(ecasoundc_sa) found rettype '%s' (len=%d).\n", parser->last_type_repp, parser->buffer_current_rep); */

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

	/* handle empty content */
	if (parser->msgsize_rep == 0) parser->buffer_repp[0] = 0;

#if 1
	ECI_DEBUG_2("(ecasoundc_sa) found content, loglevel=%d, msgsize=%d", parser->loglevel_rep, parser->msgsize_rep);
	if (parser->state_msg_rep == ECI_STATE_MSG_GEN)
	  ECI_DEBUG(".\n");
	else
	  ECI_DEBUG_1(" type='%s'.\n", parser->last_type_repp);
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
	  ECI_DEBUG_1("(ecasoundc_sa) rettype-content validated: <<< %s >>>\n", parser->buffer_repp);
	  eci_impl_set_last_values(parser);
	  parser->last_counter_rep++;
	}
	else {
	  ECI_DEBUG_1("(ecasoundc_sa) gen-content validated: <<< %s >>>\n", parser->buffer_repp);
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
    if (parser->buffer_current_rep >= parser->msgsize_rep) {
      /* ECI_DEBUG_2("\n(ecasoundc_sa) WARNING! Content size exceeded; cur=%d, msgsize=%d.\n\n", parser->buffer_current_rep, parser->msgsize_rep); */
    }
    msg_buffer[parser->buffer_current_rep] = c;
    if (++parser->buffer_current_rep == ECI_MAX_PARSER_BUF_SIZE) {
      fprintf(stderr, "\n(ecasoundc_sa) WARNING! Parsing buffer overflowed!\n\n");
    }
  }
}
