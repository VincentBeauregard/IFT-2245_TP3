#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

#include <stdint.h>
#include <stdio.h>

/* Initialise la mémoire physique.  */
void pm_init (FILE *backing_store, FILE *log);

void pm_download_page (unsigned int page_number, unsigned int frame_number);
void pm_backup_frame (unsigned int frame_number, unsigned int page_number);
char pm_read (unsigned int physical_address);
void pm_write (unsigned int physical_address, char);
void pm_clean (void);

/* Fonction ajoutée */
int pm_swap(unsigned int page_number);

#endif
