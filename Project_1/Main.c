#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>


//forward declarations of functions
void print_help();

//main loop that sets up the interface and executes the other commands
int main(int argc, char** argv) {

  //then, we need to be able to split up that input string into upto 4 strings, plus a NULL to end
  char input[4][100];
  int i;

  //used to fork the program
  pid_t forkPid;
  //make sure all strings end in /0
  for(i=0; i < 4; i++) {
    input[i][99] = '\0';
  }
  //keep on cycling through the loop until reach exit command
  while(0 == 0) {
    //flush stdin, to make sure we're starting with a clean slate
    fflush(stdin);
    //print out the prompt
    printf("gis> ");
    //get user input at this point
    //read the first argument, then react accordingly, scanning more strings if necessary
    scanf("%s", input[0]);
    
    //try to run commands based on those strings
    if(strcmp(input[0], "render") == 0 || strcmp(input[0], "r") == 0) {
      //read the 1 more argument, needed for render
      scanf("%s", input[1]);
      printf("read in %s\n", input[1]); fflush(stdout);
      //run the render command
      if( (forkPid = fork()) == 0) {
	//child process - run execl
	execl("./render", input[1], NULL);
      }
      else {
	sleep(1);
      }
    }
    else if(strcmp(input[0], "flowdir") == 0 || strcmp(input[0], "fd") == 0) {
      //read in the input and output maps for flowdir
      scanf("%s %s", input[1], input[2]);
      //run the flowdir command
      if( (forkPid = fork()) == 0) {
	//child process - execute execl
	execl("./flowdir", input[1], input[2], NULL);
      }
      else {
	//parent - sleep
	sleep(1);
      }
    }
    else if(strcmp(input[0], "flowaccu") == 0 || strcmp(input[0], "fa") == 0) {
      //read in the elev, input and output maps for flowaccu
      scanf("%s %s %s", input[1], input[2], input[3]);
      //run the flowaccu command
      if( (forkPid = fork()) == 0) {
	// child process- execute flowaccu
	execl("./flowaccu", input[1], input[2], input[3], NULL);
      }
      else {
	//parent - sleep
	sleep(1);
      }
    }
    else if(strcmp(input[0], "exit") == 0 || strcmp(input[0], "quit") == 0 || strcmp(input[0], "q") == 0) {
      //quit
      exit(0);
    }
    else {
      //no valid command given - print out the help and continue
      print_help();
    }
    fpurge(stdin);
  }
  //if no input is given, print out help info and go on
  /*  else { */
  /*       //print out help info */
  /*     } */
  
  return 0;
  
}

//print out help information
void print_help() {
  printf("Invalid Command\nUsage:\nrender\t<input.asc>\nflowdir\t<elev.asc> <output.asc>\nflowaccu\t<elev.asc> <flowdir.asc> <output.asc>\nquit\n");

}
