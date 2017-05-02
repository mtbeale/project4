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
enum Redirect {PIPE, STDIN ,STDOUTA, STDOUTT, STDERR, NEITHER};
Redirect parse_input(int argc, char** argv, char** cmd1, char** cmd2);
void pipe_cmd(char**, char**);
void run_cmd(int argc, char** argv);
void handle_sig(int sig);
void redirect_stdoutt(char** cmd, char** file);
void redirect_stdouta(char** cmd, char** file);
void close_pipe(int fds [2]);

int main () {
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  char *argv[256], *cmd1[256], *cmd2[256], cwd[1024];
  int argc;
  Redirect pipe_redirect;
  getcwd(cwd,sizeof(cwd));

  while (true) {
    cout << "1730sh: " << cwd << "/$ ";
    signal(SIGINT,handle_sig);
    //  signal(SIGQUIT,handle_sig);
    //  signal(SIGTSTP,handle_sig);
    signal(SIGTTIN,handle_sig);
    signal(SIGTTOU,handle_sig);
    //    signal(SIGCHLD,handle_sig);

    argc = read_input(argv);
    pipe_redirect = parse_input(argc,argv,cmd1,cmd1);

    if (pipe_redirect == PIPE) {
      pipe_cmd(cmd1, cmd2);
    } else if (pipe_redirect == STDOUTT) {
      redirect_stdoutt(argv,cmd1);
    } else if (pipe_redirect == STDOUTA) {
redirect_stdouta(argv,cmd1);
    } else {
      run_cmd(argc,argv);
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
    // Increment our counter of where we're at in the array of arguments.
    argc++;
    // If the user hit enter, stop reading input.
    if (cin.get() == '\n')
      break;
  }
  // Have to have the last argument be NULL so that execvp works.
  argv[argc] = NULL;
  // Return the number of arguments we got.
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
      result = STDOUTA;
      split = i;
    }  else if (strcmp(argv[i],">") == 0) {
      result = STDOUTT;
      split = i;
    } else if (strcmp(argv[i],"<") == 0) {
      result = STDIN;
      split = i;
    } else if (strcmp(argv[i],"e>") == 0){
      result = STDERR;
      split = i;
    } else if (strcmp(argv[i],"exit") == 0) {
      exit(0);
    } else if (strcmp(argv[i],"cd") == 0) {
      cout << "cd to " << argv[i+1] << endl;
      chdir(argv[i+1]);
    } else if (strcmp(argv[i],"help") == 0) {
      cout << "builtins avaliable: " << endl;
      cout << "cd [PATH] - Change the current directory to PATH." << endl;
      cout << "exit [N] - Cause the shell to exit with a status of N. N can be omitted." << endl;
      cout << "help - Display helpful information about builtin commands." << endl;
    } else if (strcmp(argv[i],"export") == 0) {
      cout << "export plz" << endl;
}
}
 if (result != NEITHER) {
    // Go through the array of arguments up to the point where the
    // pipe/redirect was found and store each of those arguments in cmd1.
    for (int i=0; i<split; i++)
      cmd1[i] = argv[i];

    // Go through the array of arguments from the point where the pipe/redirect
    // was found through the end of the array of arguments and store each
    // argument in cmd2.
    int count = 0;
    for (int i=split+1; i<argc; i++) {
      cmd2[count] = argv[i];
      count++;
    }

    // Terminate cmd1 and cmd2 with NULL, so that execvp likes them.
    cmd1[split] = NULL;
    cmd2[count] = NULL;
  }

  // Return an enum showing whether a pipe, redirect, or neither was found.
  return result;
}

void run_cmd(int argc, char** argv) {
  pid_t pid, pgid = getpgrp();
  int pstatus;
  const char *amp;
  amp = "&";
  bool found_amp = false;


  if (strcmp(argv[argc-1],amp) == 0)
    found_amp = true;

  if ((pid = fork()) == -1)  // error
    perror("FORK ERROR");
 if (pid == 0) { // child
     pid = getpid();
     if (pgid == 0) pgid = pid;
     setpgid(pid,pgid);
     cout << pid << " " << pgid << endl;
    if (found_amp) {
      argv[argc-1] = NULL;
      argc--;
    }
    execvp(argv[0],argv);
    perror("execvp");
   } // end child
   else if (!found_amp)  // parent
     waitpid(pid,NULL,0);
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

void redirect_stdoutt(char** cmd, char** file) {
  int fds[2]; // file descriptors
  int count;  // used for reading from stdout
  int fd;     // single file descriptor
  char c;     // used for writing and reading a character at a time
  pid_t pid;  // will hold process ID; used with fork()
  // child process #1
  if (fork() == 0) {
    fd = open(file[1], O_RDWR | O_CREAT | O_TRUNC, 0666);
    // open() returns a -1 if an error occurred
    if (fd < 0) {
      printf("Error: %s\n", strerror(errno));
      return;
    }

    dup2(fd, 1);
 // Read from stdout...
    while ((count = read(0, &c, 1)) > 0)
      write(fd, &c, 1); // Write to file.

    execvp(cmd[1],cmd);
  } else {
    waitpid(pid, NULL, 0);
  }
}

void redirect_stdouta(char** cmd, char** file) {
  int fds[2]; // file descriptors
  int count;  // used for reading from stdout
  int fd;     // single file descriptor
  char c;     // used for writing and reading a character at a time
  pid_t pid;  // will hold process ID; used with fork()


  // child process #1
  if (fork() == 0) {

      fd = open(file[2], O_RDWR |O_APPEND | O_CREAT, 0666);

      // open() returns a -1 if an error occurred
      if (fd < 0) {
        printf("Error: %s\n", strerror(errno));
        return;
      }

      dup2(fd, 1);

      // Read from stdout...
      while ((count = read(0, &c, 1)) > 0)
        write(fd, &c, 1); // Write to file.

    } else {
      waitpid(pid, NULL, 0);
    }
}
void pipe_cmd(char** cmd1, char** cmd2) {
  int fds[2]; // file descriptors
  pipe(fds);
  pid_t pid1, pid2;
  // child process #1
  if (pid1 = fork() == 0) {
    // Reassign stdin to fds[0] end of pipe
    dup2(fds[0], 0);
    // Not going to write in this child process, so we can close this end
    // of the pipe
    close(fds[1]);
    // Execute the second command
    execvp(cmd1[0], cmd1);
    perror("execvp failed");

    // child process #2
  } else if ((pid2 = fork()) == 0) {
    // Reassign stdout to fds[1] end of pipe
    dup2(fds[1], 1);

    // Not going to read in this child process, so we can close this end
    // of the pipe
    close(fds[1]);

    // Execute the first command
    execvp(cmd2[0], cmd2);
    perror("execvp failed");

    // parent process
  } else {
    close(fds[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2,NULL,0);
  }
}

void close_pipe(int pipefd [2]) {
  if (close(pipefd[0]) == -1) {
    perror("close");
    exit(EXIT_FAILURE);
  } // if
  if (close(pipefd[1]) == -1) {
    perror("close");
    exit(EXIT_FAILURE);
  } // if
} // close_pipe
