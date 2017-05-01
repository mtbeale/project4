#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>

using namespace std;

int read_input(char **argv);
enum Redirect {PIPE, REDIRECT, NEITHER};
Redirect parse_input(int argc, char** argv, char** cmd1, char** cmd2);
void pipe_cmd(char**, char**);
void run_cmd(int argc, char** argv);
void handle_sig(int sig);

int main () {

  char *argv[256], *cmd1[256], *cmd2[256];
  int argc;
  Redirect pipe_redirect;

  while (true) {
    cout << "1730sh: ";
    signal(SIGINT,handle_sig);
    signal(SIGQUIT,handle_sig);
    signal(SIGTSTP,handle_sig);
    signal(SIGTTIN,handle_sig);
    signal(SIGTTOU,handle_sig);
    signal(SIGCHLD,handle_sig);

    argc = read_input(argv);
    pipe_redirect = parse_input(argc,argv,cmd1,cmd1);

    if (pipe_redirect == NEITHER) {
      run_cmd(argc, argv);
    }
      for (int i = 0; i< argc;i++)
      argv[i] = NULL;
  }

  return 0;
  }

int read_input(char **argv) {
  char *cstr;
  string arg;
  int argc = 0;
  string newDir;

  // Read in arguments till the user hits enter
  while (cin >> arg) {
    // convert string into C string
    cstr = new char[arg.size()+1];
    strcpy(cstr, arg.c_str());
 argv[argc] = cstr;
    // Increase counter of where we're at in the array of argv
    argc++;
    // If 'enter' is the input, stop reading input
    if (cin.get() == '\n')
      break;
  }
  // Have to have the last argument be NULL so that execvp works.
  argv[argc] = NULL;
  // Return the number of arguments 
  return argc;
}

Redirect parse_input(int argc, char** argv, char** cmd1, char** cmd2) {
  Redirect result = NEITHER;

  int split = -1;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i],"|") == 0) {
        result = PIPE;
        split = i;
      } else if (strcmp(argv[i], ">>") == 0) {
        result = REDIRECT;
        split = i;
		
		// Exit builtin
    } else if (strcmp(argv[i],"exit") == 0) {
      exit(0);
	  
	  // CD builtin
    } else if (strcmp(argv[i],"cd") == 0) {
      cout << "cd to " << argv[i+1] << endl;
      chdir(argv[i+1]);
	  
	  // Help builtin.
    } else if (strcmp(argv[i],"help") == 0) {
      cout << "builtins avaliable: " << endl;
      cout << "cd [PATH] - Change the current directory to PATH." << endl;
      cout << "exit [N] - Cause the shell to exit with a status of N. N can be omitted." << endl;
      cout << "help - Display helpful information about builtin commands." << endl;
    }
  }
  if (result != NEITHER) {
    // Go through the array of arguments up to the point where the
    // pipe/redirect was found and store each of those arguments in cmd1 
    for (int i=0; i<split; i++)
      cmd1[i] = argv[i];

    // Go through the array of arguments from the point where the pipe/redirect
    // was found through the end of the array of arguments and store each
    // argument in cmd2 to use for executing pipes/redirects
    int count = 0;
    for (int i=split+1; i<argc; i++) {
      cmd2[count] = argv[i];
      count++;
    }

    // Terminate cmd1 and cmd2 with NULL, so that execvp works
    cmd1[split] = NULL;
    cmd2[count] = NULL;
  }

  // Return an enum showing whether a pipe, redirect, or neither was found
  return result;
}

void run_cmd(int argc, char** argv) {
  pid_t pid, pgid;
  int pstatus;
  const char *amp;
  amp = "&";
  bool found_amp = false;

  if (strcmp(argv[argc-1],amp) == 0)
    found_amp = true;

  if ((pid = fork()) == -1)  // error
    perror("FORK ERROR");

   if (pid == 0) { // child
    if (found_amp) {
      argv[argc-1] = NULL;
      argc--;
    }
    setpgid(pid,pgid);
    execvp(argv[0],argv);
    perror("execvp");
   } // end child
   else { // parent
   waitpid(pid,NULL,0);
   setpgid(pid,pgid);
   cout << "pid: " << pid << endl;
   }
}


void handle_sig(int sig) {
  cout << "hello" << endl;
  switch(sig) {
  case SIGINT: cout << "SIGINT" << endl; signal(SIGINT,SIG_IGN); break;
  case SIGQUIT: cout << "SIGQUIT" << endl; signal(SIGQUIT,SIG_IGN); break;
  case SIGTSTP: cout << "SIGTSTP" << endl; signal(SIGTSTP,SIG_IGN); break;
  case SIGTTIN: cout << "SIGTTIN" << endl; signal(SIGTTIN,SIG_IGN); break;
  case SIGTTOU: cout << "SIGTTOU" << endl; signal(SIGTTOU,SIG_IGN); break;
  case SIGCHLD: cout << "SIGCHLD" << endl; signal(SIGCHLD,SIG_IGN); break;
  default: cout << "???" << endl; break;
  }
}
