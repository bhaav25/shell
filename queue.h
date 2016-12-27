// File: queue.h
#ifndef _queue_h
#define _queue_h

// prototypes
void addDir(char *dir);
void removeHead();
void printQ();
int getDirCount();

// variables
struct node{
  char *path;
  struct node *next;
};

struct node *head;
int dirCount;

#endif
