#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bits/stdc++.h>
#include <string>
#include <signal.h>
#include <filesystem>
#include <sys/stat.h>

using namespace std;
using std::filesystem::exists;
using std::filesystem::is_directory;
using std::filesystem::is_regular_file;

// globals
#define MYSH_BUFFERSIZE 64
#define MYSH_DELIM " \t\n"
fstream file;

class History
{                          // The class
public:                    // Access specifier
  int num_lines = 0;       // Attribute (int variable)
  vector<string> historyv; // Attribute (string variable)
};

History mysh_history_obj;

// custom function declarations
int mysh_exit();
int mysh_history(vector<string> tokens);
int mysh_start(vector<string> tokens);
int mysh_start_process(vector<string> tokens);
int mysh_replay(vector<string> tokens);
int fill_history();
int clear_history();
vector<string> mysh_tokenize(string line);
string mysh_read_line(void);
int mysh_add_history(string line);
int lookup_funct(vector<string> tokens, string func_name);
int mysh_execute(vector<string> args);
void mysh_loop(void);
int mysh_background(vector<string> args);
int mysh_dwelt(vector<string> tokens);
int mysh_maik(vector<string> tokens);
int mysh_coppy(vector<string> tokens);

string byebye = "byebye";
string exit_program = "exit";
string history = "history";
string start = "start";
string replay = "replay";
string background = "background";
string dwelt = "dwelt";
string maik = "maik";
string coppy = "coppy";

// contains names of all custom shell commands implemented
string lookup_str[] = {
    byebye,
    exit_program,
    history,
    start,
    replay,
    background,
    dwelt,
    maik,
    coppy};

// holds references to all commands in lookup_str[] order of commands must match each other
int lookup_funct(vector<string> tokens, string func_name)
{
  if (func_name.compare(byebye) == 0 || func_name.compare(exit_program) == 0)
  {
    return mysh_exit();
  }
  if (func_name.compare(history) == 0)
  {
    return mysh_history(tokens);
  }
  if (func_name.compare(start) == 0)
  {
    return mysh_start(tokens);
  }
  if (func_name.compare(replay) == 0)
  {
    return mysh_replay(tokens);
  }
  if (func_name.compare(background) == 0)
  {
    return mysh_background(tokens);
  }
  if (func_name.compare(dwelt) == 0)
  {
    return mysh_dwelt(tokens);
  }
  if (func_name.compare(maik) == 0)
  {
    return mysh_maik(tokens);
  }
  if (func_name.compare(coppy) == 0)
  {
    return mysh_coppy(tokens);
  }
  return 1;
}

/* custom shell commands implementations BEGIN*/
int mysh_background(vector<string> tokens)
{
  pid_t pid;
  pid_t cpid;
  pid_t wpid;
  int state;

  std::vector<char *> pvec(tokens.size());
  std::transform(tokens.begin(), tokens.end(), pvec.begin(), [](auto &str)
                 { return &str[0]; });
  pvec.push_back(NULL);

  pid = fork();

  if (pid == 0)
  {
    cpid = fork();
    if (cpid == 0)
    {
      if (execvp(pvec.data()[0], (char *const *)pvec.data()) == -1)
      {
        // cout << "mysh: failed to execute\n\n";
      }
    }
    return 1;
  }
  // forking failed
  else if (pid < 0)
  {
    // cout << "mysh: failed to fork\n\n";
  }

  cout << pid;

  return 1;
}

int mysh_dwelt(vector<string> tokens)
{

  if (exists(tokens[1]))
  {
    if (is_directory(tokens[1]))
      cout << "Abode is.\n";
    if (is_regular_file(tokens[1]))
      cout << "Dwelt indeed.\n";
  }
  else
  {
    cout << "Dwelt not.\n";
  };

  return 1;
}

int mysh_maik(vector<string> tokens)
{
  if (exists(tokens[1]))
  {
    cout << "Error in maik: file with this name already exists.";
  }
  else
  {
    file.open(tokens[1], ios::out);
    file << "Draft";
    file.close();
  }
  return 1;
}

int mysh_coppy(vector<string> tokens)
{
  if (!exists(tokens[1]))
  {
    cout << "Error in coppy: source file with this name does not exist.";
  }
  if (exists(tokens[2]))
  {
    cout << "Error in coppy: desination file with this name already exists.";
  }
  else
  {
    string line;
    ifstream in_file{tokens[1]};
    ofstream out_file{tokens[2]};

    if (in_file && out_file)
    {

      while (getline(in_file, line))
      {
        out_file << line << "\n";
      }
    }
    else
    {
      cout << "Error in coppy: cannot read file";
    }
  }
  return 1;
}

int mysh_replay(vector<string> tokens)
{
  int number = stoi(tokens[1]);
  int counter = 0;
  for (int i = mysh_history_obj.historyv.size() - 1; i >= 0; i--)
  {
    if (counter == number)
    {
      string line = mysh_history_obj.historyv[i - 1];
      vector<string> args = mysh_tokenize(line);
      if (args[0].compare(replay) == 0)
      {
        // cout << "mysh: you cannot replay 'replay' commands\n\n";
        return 1;
      }
      return mysh_execute(args);
    }
    counter++;
  }
  return 1;
}

int mysh_start(vector<string> tokens)
{
  tokens.erase(tokens.begin());
  mysh_start_process(tokens);
  return 1;
}

int clear_history()
{
  file.close();
  file.open("mysh.history", ios::out | ios::trunc);
  file.close();
  mysh_history_obj.historyv.clear();

  printf("mysh history cleared\n");
  return 1;
}

int mysh_add_history(string line)
{
  // if exists then append to the history
  if (access("mysh.history", F_OK) == 0)
  {
    file.open("mysh.history", ios::app);
  }
  // otherwise create mysh.history and start writing
  else
  {
    file.open("mysh.history", ios::out);
  }

  file << line << "\n";
  mysh_history_obj.historyv.push_back(line);
  file.close();

  return 1;
}

int mysh_history(vector<string> tokens)
{
  if (mysh_history_obj.num_lines == 0)
  {
    // cout << "mysh: history is empty\n\n";
  }
  else
  {
    int counter = 0;
    for (int i = mysh_history_obj.historyv.size() - 1; i >= 0; i--)
    {
      cout << counter << ": " << mysh_history_obj.historyv[i] << "\n";
      counter++;
    }
  }
  return 1;
}

//  fills the history object if the file exists
int fill_history()
{
  file.open("mysh.history", ios::in);

  if (file.is_open())
  {
    string line;
    int count = 0;
    while (getline(file, line))
    {
      mysh_history_obj.historyv.push_back(line);
      count++;
    }
    mysh_history_obj.num_lines = count;
  }
  file.close();

  return 1;
}

int mysh_exit()
{
  return 0;
}

int num_commands()
{
  return sizeof(lookup_str) / sizeof(string);
}
/* custom shell functions END*/

/* main shell processes BEGIN*/
// returns the tokens (arguments) array after tokenizing line from mysh_read_line()
vector<string> mysh_tokenize(string line)
{
  vector<string> tokens;
  stringstream check1(line);
  string temp;

  while (getline(check1, temp, ' '))
  {
    tokens.push_back(temp);
  }

  return tokens;
}

// mysh_read_line allocates MYSH_BUFFER_SIZE of memory to the intial buffer
// it reallocates memory as needed with getLine() function
// returns line to be processed and tokenized by mysh_tokenize()
string mysh_read_line(void)
{
  string line;
  getline(cin, line);
  return line;
}

// args passed comes from mysh_tokenize()
int mysh_start_process(vector<string> tokens)
{
  pid_t pid;
  pid_t wpid;
  int state;

  std::vector<char *> pvec(tokens.size());
  std::transform(tokens.begin(), tokens.end(), pvec.begin(), [](auto &str)
                 { return &str[0]; });
  pvec.push_back(NULL);

  pid = fork();

  // if we enter child process
  if (pid == 0)
  {
    if (execvp(pvec.data()[0], (char *const *)pvec.data()) == -1)
    {
      // cout << "mysh: failed to execute\n\n";
    }
    exit(EXIT_FAILURE);
  }
  // forking failed
  else if (pid < 0)
  {
    // cout << "mysh: failed to fork\n\n";
  }
  else
  {
    // if we enter parent process
    do
    {
      wpid = waitpid(pid, &state, WUNTRACED);
    } while (!WIFEXITED(state) && !WIFSIGNALED(state));
  }

  return 1;
}

// calls mysh_start_process() and handles programs being called
int mysh_execute(vector<string> args)
{
  int i;
  for (i = 0; i < num_commands(); i++)
  {
    if (args[0].compare(lookup_str[i]) == 0)
    {
      if (args[0].compare("history") == 0 && args.size() == 2)
      {
        string str1 = args[1];
        string str2 = "-c";
        if (str1.compare(str2) == 0)
        {
          return clear_history();
        }
      }
      return lookup_funct(args, lookup_str[i]);
    }
  }

  return mysh_start_process(args);
}

void mysh_loop(void)
{
  string line;
  vector<string> args;
  int state;

  do
  {
    cout << "# ";
    line = mysh_read_line();
    args = mysh_tokenize(line);
    mysh_add_history(line);
    mysh_history_obj.num_lines++;
    state = mysh_execute(args);

  } while (state);
}

int main(int argc, char **argv)
{
  // run main program loop
  fill_history();
  mysh_loop();
  file.close();

  return EXIT_SUCCESS;
}
/* main shell processes END*/