// File: shell.h
#ifndef _shell_h
#define _shell_h

// prototypes
void histList();
void evaluate();
void cd();
bool cdPath();
void ls();
void find();
void lsMod(char *path);
bool isDir(char *path);
char *currDir();
char *currFolder();
void setInputMode();
void resetInputMode();

// variables 
char *fileName; 
char *paths[100]; 
int pathCount = 0; 
struct termios savedAttributes;
char *history[10];
int histCount;
int histFull; 
int lastCommand;

#endif
