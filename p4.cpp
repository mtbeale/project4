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
