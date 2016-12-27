// File: queue.c
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

// add directory
void addDir(char *dir){

  struct node *new_node = (struct node *) malloc(sizeof(struct node));
  new_node->next = NULL;

  // queue is empty
  if(getDirCount() == 0){

    head = new_node;
    head->path = dir;
    head->next = NULL;
  
  // not empty
  } else {  
    struct node *temp = head;    
    while(temp->next != NULL){
      temp = temp->next;
    }
    temp->next = new_node;
    temp->next->path = dir;
  }
  dirCount++;
}

// remove head node
void removeHead(){
  head = head->next;
  dirCount--;
}

// queue item count
int getDirCount(){
  return dirCount;
}

// print contents
void printQ(){
  struct node *temp = head;
  while(temp != NULL){
    printf("%s\n", temp->path);
    temp = temp->next;
  }
}
