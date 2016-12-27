// File: shell.c
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // dir functions
#include <string.h> // strcpy and strcat
#include <stdbool.h> // bool
#include <sys/stat.h> // stat
#include <unistd.h> // getcwd
#include <termios.h>
#include "shell.h"
#include "queue.h"

int main(int argc, char *argv[]){
  
  // set to NON-CANONICAL MODE
  setInputMode();

  // each keystroke
  char symbol;
  int symbolCount;

  // user command
  char command[100];

  // DELETE key
  char del = 0x7F;

  int up;
  int up_done = 0;
  int up_count = 0;

  int down;
  int down_done = 0;
  int down_count = 0;

  printf("/.../%s> ", currFolder());
  fflush(stdout);
  while(1){
  
    read(STDIN_FILENO, &symbol, 1);
    if(symbol == 0x1b){
      read(STDIN_FILENO, &symbol, 1);
      if(symbol == 0x5b){
        read(STDIN_FILENO, &symbol, 1);

        // UP arrow
        if(symbol == 0x41){
         
          // UP arrow clears line if not on last UP arrow
          if(symbolCount != 0 && up_done != 1 && up_count != 10){
            for(int i = 1; i <= symbolCount; i++)
              write(STDOUT_FILENO, &del, 1);
            symbolCount = 0;
          }

          // BELL sound if no history or on last UP arrow
          if(histCount == 0 || up_done == 1 || up_count == 10)
            write(STDOUT_FILENO, "\a", 1);

          // get 
          // UP item
          else {

            if(up_count == 0)
              up = lastCommand;

            write(STDOUT_FILENO, history[up], strlen(history[up]));
            symbolCount += strlen(history[up]);
            strcpy(command, history[up]);

            up_count++;
            down_count--;

            if(histFull != 1){
              up--;
              if(up == -1)
                up_done = 1;
            } else {
              up--;
              if(up_count != 10){
                if(up == -1)
                  up = 9;
              }
            }
          }
          continue;
        }
        
        // DOWN arrow
        if(symbol == 0x42){
          
          // clear line
          if(symbolCount != 0){
            for(int i = 1; i <= symbolCount; i++)
              write(STDOUT_FILENO, &del, 1);
            symbolCount = 0;

          // BELL sound if
          // nothing to clear
          } else {
            write(STDOUT_FILENO, "\a", 1);
          }

          // get DOWN item
          if(histCount != 0){
            if(up_count != 0 && down_count != 10){
              if(histFull != 1){

                up++;
                if(up == 0){
                  up_done = 0;
                }
                up_count--;

                write(STDOUT_FILENO, history[up], strlen(history[up]));
                symbolCount += strlen(history[up]);
                strcpy(command, history[up]);
                down_count++;

              } else {

                up++;
                if(down_count != 10){
                  if(up == 9 + 1){
                    up = 0;
                  }
                  up_count--;

                  write(STDOUT_FILENO, history[up], strlen(history[up]));
                  symbolCount += strlen(history[up]);
                  strcpy(command, history[up]);
                  down_count++;
                }
              }
            }
          }
          continue;
        }
      }
    }

    // CTRL-D
    if(symbol == 0x04)
      break;
    else {
      // char isn't newline or backspace
      if(symbol != 0x0A && symbol != 0x7F){
        write(STDOUT_FILENO, &symbol, 1);
        command[symbolCount] = symbol;
        symbolCount++;
      }
      // char is backspace
      if(symbol == 0x7F){

        // BELL sound if
        // nothing on the line
        if(symbolCount == 0)
          write(STDOUT_FILENO, "\a", 1);

        // delete char
        else {
          write(STDOUT_FILENO, &symbol, 1);
          symbolCount--;
        }
      }

      // char is newline    
      if(symbol == 0x0A){
        
        // input is not empty
        if(symbolCount != 0){

          command[symbolCount] = '\0';
          lastCommand = histCount % 10;
          history[lastCommand] = (char *) malloc(sizeof(char)*40);
          strcpy(history[histCount % 10], (char *) command);
          histCount++;
          if(histCount > 10)
            histFull = 1;
          symbolCount = 0;
          evaluate(command);
        }
        
        up_count = 0;
        down_count = 0;

        printf("/.../%s> ", currFolder());
        fflush(stdout);
      }
    }
  }
  return 0;
}

// print last ten commands
void histList(){

  printf("\n");
  int start;
  int end;

  if(histFull == 0){

    start = histCount;
    end = 0;
    for(int i = start - 1; i >= end; i--)
      printf("%s\n", history[i]);

  } else {

    end = lastCommand;
    int count = 0;
    while(count != 10){
      printf("%s\n", history[end]);
      end--;
      count++;
      if(end == -1)
        end = 9;   
    }
  }
}


// evaluate user command
void evaluate(char command[]){

  int argCount = 0;
  char *commandList[5];
  char *word = strtok(command, " ");
  while(word != NULL){
    commandList[argCount] = word;
    argCount++;
    word = strtok(NULL, " ");
  }

  // pwd
  if((argCount == 1 && strcmp(commandList[0], "pwd") == 0)){
    printf("\n%s\n", currDir());

  // ls
  } else if((argCount == 1 && strcmp(commandList[0], "ls") == 0)){
     ls();
     printf("\n");

  // cd
  } else if((argCount == 1 && strcmp(commandList[0], "cd") == 0)){
    cd();
    printf("\n");

  // cd (path)
  } else if((argCount == 2 && strcmp(commandList[0], "cd") == 0)){
    if(!cdPath(commandList[1]))
      printf("\nCould not determine path.");
    printf("\n");

  // hist
  } else if((argCount == 1 && strcmp(commandList[0], "hist") == 0)){
     histList();

  // find
  } else if(argCount == 3 && strcmp(commandList[0], "find") == 0){
    printf("\n");
    find(commandList[1], commandList[2]);

  } else
    printf("\nCommand not found.\n");
}


// command: cd
void cd(){

  char *dir = (char *) malloc(sizeof(char)*100);
  dir = getenv("HOME");
  chdir(dir);
} 

// command: cd (path)
bool cdPath(char *path){
  if(chdir(path) != -1)
    return true;
  return false;
}

// command: ls
void ls(){

  struct dirent *d;
  DIR *dp;
  dp = opendir(".");
  while(d = readdir(dp)){

    if(strcmp(d->d_name, "..") != 0 && strcmp(d->d_name, ".") != 0)
      printf("\n%s", d->d_name);
  }
  closedir(dp);
}
 
// command: find (directory) (file)
void find(char *dir, char *file){

  char *folder = dir;
  
  // if path is ..
  if(strcmp(folder, "..") == 0){

    folder = currDir();
    char *pos = &folder[strlen(folder) - 1];
    while(*pos != '/'){
      pos--;
    }
    *pos = '\0';

  // if path is .
  } else if(strcmp(folder, ".") == 0){
    folder = currDir();

  // remove '/' if in path
  } else if(folder[strlen(folder) - 1] == '/')
    folder[strlen(folder) - 1] = '\0';

  // (file) entered by user
  fileName = file;  

  // add path to queue
  addDir(folder);

  // while queue not empty
  while(getDirCount() != 0){

    lsMod(head->path);
    removeHead();
  }

  if(pathCount == 0)
    printf("\nFile not Found.\n");
  else {

    // print all locations of file
    printf("\n%d matches of %s found:\n", pathCount, fileName);

    for(int i = 1; i <= pathCount; i++){
      char *pos = paths[i];
      while(*pos != '\0'){
        printf("%c", *pos);
        pos++;
      }
      printf("\n");
    }
  } 

  fileName = '\0';
  pathCount = 0;
  dirCount = 0;
}


// ls modified to print
// contents of a directory
void lsMod(char *path){

  struct dirent *d;
  DIR *dp;

  if((dp = opendir(path)) == NULL){
    printf("Cannot open directory... Exiting\n");
    exit(1);
  }

  bool dots = false; // check if dir is . or ..
  bool folder = false; // check if dir or file
  bool found;

  char *fullPath; // file or directory
  while(d = readdir(dp)){

    fullPath = (char *) malloc(500);
    if(d->d_ino != 0){
    
      // create full path name
      strcpy(fullPath, path);
      strcat(fullPath, "/");
      strcat(fullPath, d->d_name);

      // check if the item is . or ..
      if(strcmp(d->d_name, "..") != 0 && strcmp(d->d_name, ".") != 0)
        dots = false;
      else
        dots = true;

      // check if file or dir
      if(isDir(fullPath))
        folder = true;
      else
        folder = false;
    
      // print everything inside dir
      // except . and .. dirs
      if(!dots)
        printf("%s\n", fullPath);
    
      // if a file, then compore with users file name
      if(!folder && strcmp(d->d_name, fileName) == 0){
      
        pathCount++;
        paths[pathCount] = fullPath;  
      }

      // if dir that isn't
      // curr dir or parent dir
      if(folder && !dots){
        addDir(fullPath);    
      }
    }
  }
  closedir(dp);
}

// check if directory
bool isDir(char *path){

  struct stat path_info;
  stat(path, &path_info);
  
  if(S_ISDIR(path_info.st_mode))
    return true; 
  else
    return false;
}

// get current directory
char *currDir(){
  char *dir = (char *) malloc(500);
  return getcwd(dir, 500 + 1);
}

// get current folder
// use: "/.../(folder)>"
char *currFolder(){

  char *folder = currDir();
  char *pos = &folder[strlen(folder) - 1];
  while(1){
    if(*(pos - 1) == '/')
      break;
    else
      pos--;
  }
  return pos;
}

// set to NON-CANONICAL MODE
void setInputMode(){

  struct termios tattr;
  char *name;

  if(!isatty(STDIN_FILENO)){

    fprintf(stderr, "File descriptor is not a terminal\n");
    exit(EXIT_FAILURE);
  }

  tcgetattr(STDIN_FILENO, &savedAttributes);
  atexit(resetInputMode);

  tcgetattr(STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON | ECHO);
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);

}

void resetInputMode(){
  tcsetattr(STDIN_FILENO, TCSANOW, &savedAttributes);
}
