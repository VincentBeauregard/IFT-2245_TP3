#include <stdio.h>
#include <stdlib.h>

#include "conf.h"

FILE *command1;
FILE *command2;

void create1(int size) {
  char command[50];
  // Maximise les chances d'un TLB hit
  int addr = TLB_NUM_ENTRIES * NUM_PAGES * 2;
  for (int i = 0; i < size; i++) {
	  rand()%addr;
    if (rand() % 2) 
      sprintf(command,
	      "W%d'%c';\n",
	      rand() % addr,
	      rand() % 95 + 32);
    else
      sprintf(command, "R%d;\n", rand() % addr);
    
    fprintf(command1, command);
  }
}

void create2(int size) {
  char command[50];
  // Maximise les chances d'un page swap
  int addr = PAGE_FRAME_SIZE * NUM_PAGES;
  
  for (int i = 0; i < size; i++) {
    
    if (rand() % 2) 
      sprintf(command,
	      "W%d'%c';\n",
	      rand() % addr,
	      rand() % 95 + 32);
    else
      sprintf(command, "R%d;\n", rand() % addr);
    
    fprintf(command2, command);
  }
}

int main () {
  command1 = fopen("tests/command2.in", "w+");
  command2 = fopen("tests/command1.in", "w+");

  create1(NUM_PAGES);
  create2(NUM_PAGES * 10);
  
  fclose(command1);
  fclose(command2);
  return 0;
}
