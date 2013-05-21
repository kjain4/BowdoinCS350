
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef __APPLE__
#  include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "misc.h"
#include "vector.h"
#include "datagrid.h"
#include "flow.h"


const int CONTINUE = 0;
const int QUIT = 1;
const int ERROR = 2;
const char *PRGM = "FishGIS";
const char *PREFIX = "./fishgis-";
const char *USAGE =
  "Usage: fishgis-shell [command] [args]\n"
  "  If a command is specified, equivalent to calling\n"
  "    $> fishgis-command args\n"
  "  Otherwise, opens a readline-enabled shell.";

const char* commands[] = { "flowdir", "flowaccu", "trials", NULL };


// helper functions
void parse_args (char *buf, Vector *argv);
int  run_command(Vector *argv);
void print_usage(void);
void initialize_readline(void);
char *dupstr __P((const char *));
char *command_generator __P((const char *, int));
char **fishgis_completion __P((const char *, int, int));

extern char *xmalloc (size_t bytes);


/**
 * Run a readline-enabled shell interface to the FishGIS command library.
 *
 * Includes command history, full emacs-style interaction, and completion for
 * available FishGIS command bases.
 */
int main(int argc, const char **argv)
{
  const char *PRMPT = "fishgis> ";

  char *line;
  Vector *line_argv;
  int i, loop;

  // bind readline completer
  initialize_readline();

  // initialize vector to hold arugments for each call
  line_argv = vinit(sizeof(char*));
  line = NULL;
  loop = TRUE;

  // read initial line, or use command-line arguments
  if (argc == 1) {
    // read the first line
    line = readline(PRMPT);
    if (line) {
      // parse arguments into Vector
      parse_args(line, line_argv);
    }
  }else {
    // transfer arguments pointers into Vector
    for (i = 1; i < argc; i++)
      vappend(line_argv, &argv[i]);
    // exit immediately after running command
    loop = FALSE;
  }

  // read lines and execute commands
  do {
    // execute sub-command with arguments
    if (run_command(line_argv) & (QUIT | ERROR))
      loop = FALSE;
    // clear the command Vector
    vclear(line_argv);
    // free old inpnut line
    if (line) {
      // save it in the history first, if it contains text
      if (*line)
        add_history(line);
      free(line);
    }
    // read another input line
    if (loop) {
      line = readline(PRMPT);
      if (line)
        // parse arguments into Vector
        parse_args(line, line_argv);
      else
        loop = FALSE;
    }
  } while (loop);

  // EOF caused exit, or direct argument singleton call
  //   print a new line to make terminal nice
  if (!line)
    printf("\n");
  vfree(line_argv);

  return 0;
}

void parse_args(char *buf, Vector *argv)
{
  const char *delim = " \t";
  char *tok;
  
  tok = strtok(buf, delim);
  while (tok) {
    vappend(argv, &tok);
    tok = strtok(NULL, delim);
  }
}

int run_command(Vector *argv)
{
  const int BUFSIZE = 1024;
  static char cmd[1024];
  const char *arg;
  int i, argi, len;

  if (argv->length >= 1 && strcmp(*(const char**)vget(argv, 0), "exit") == 0)
    return QUIT;

  // prepend the "fishgis" prefix
  i = 0;
  len = strlen(PREFIX);
  memcpy(&cmd[i], PREFIX, len);
  i += len;

  for (argi = 0; argi < argv->length; argi++) {
    arg = *(const char**)vget(argv, argi);
    len = strlen(arg);

    if (i + len >= BUFSIZE) {
      fprintf(stderr, "Command length exceeds buffer size (%i)\n", BUFSIZE);
      return ERROR;
    }
    memcpy(&cmd[i], arg, len);
    i += len;

    if (i + 1 >= BUFSIZE) {
      fprintf(stderr, "Command length exceeds buffer size (%i)\n", BUFSIZE);
      return ERROR;
    }else if (argi < argv->length - 1)
      cmd[i++] = ' ';
    else
      cmd[i++] = '\0';
  }

  if (system(cmd) != 0)
    ; // pass

  return CONTINUE;
}

void print_usage()
{
  const char **cmd;

  fprintf(stderr, "%s\n", USAGE);
  fprintf(stderr, "  Available commands:\n");

  cmd = &commands[0];
  while (*cmd)
    fprintf(stderr, "    %s\n", *cmd++);
}

void initialize_readline()
{
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = PRGM;

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = fishgis_completion;

  /* Have readline process signals */
  rl_catch_signals = TRUE;
  rl_set_signals();
}

/* Duplicate a string.  Returns a new reference. */
char *
dupstr (s)
       const char *s;
{
    char *r;

      r = xmalloc (strlen (s) + 1);
        strcpy (r, s);
          return (r);
}

/* Attempt to complete on the contents of TEXT.  START and END
   bound the region of rl_line_buffer that contains the word to
   complete.  TEXT is the word to complete.  We can use the entire
   contents of rl_line_buffer in case we want to do some simple
   parsing.  Returnthe array of matches, or NULL if there aren't any. */
char **
fishgis_completion (text, start, end)
     const char *text;
     int start, end;
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = rl_completion_matches (text, command_generator);

  return (matches);
}

/* Generator function for command completion.  STATE lets us
   know whether to start from scratch; without any state
   (i.e. STATE == 0), then we start at the top of the list. */
char *
command_generator (text, state)
     const char *text;
     int state;
{
  static int list_index, len;
  const char *name;

  /* If this is a new word to complete, initialize now.  This
     includes saving the length of TEXT for efficiency, and
     initializing the index variable to 0. */
  if (!state)
    {
      list_index = 0;
      len = strlen (text);
    }

  /* Return the next name which partially matches from the
     command list. */
  while ((name = commands[list_index]))
    {
      list_index++;

      if (strncmp (name, text, len) == 0)
        return (dupstr(name));
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}


