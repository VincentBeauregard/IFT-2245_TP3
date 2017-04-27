#include <stdio.h>
#include <string.h>


#include "conf.h"
#include "pm.h"

/* Include ajouté */
#include "pt.h"

static FILE *pm_backing_store;
static FILE *pm_log;
static char pm_memory[PHYSICAL_MEMORY_SIZE];
static unsigned int download_count = 0;
static unsigned int backup_count = 0;
static unsigned int read_count = 0;
static unsigned int write_count = 0;

/* Variables ajoutées */
static unsigned int  frame_table[NUM_FRAMES];
static unsigned char flag_table[NUM_FRAMES];
static unsigned char swap_count = 0;

static const char RATE = NUM_FRAMES - 1;
static const char OLD = 1;
static const char DIRTY = 2;

/* Fonction ajoutée */
// Effectue le swap d'une page
int pm_swap(unsigned int page_number)
{
  unsigned int frame_number;

  // Trouve une vielle page à swapper
  for (int i = 0; i < NUM_FRAMES; i++)
    if (flag_table[i] & OLD) {
      frame_number = i;
      break;
    }

  pt_unset_entry(frame_table[frame_number]);
  
  if (flag_table[frame_number] & DIRTY)
    pm_backup_frame(frame_number,  frame_table[frame_number]);

  pm_download_page(frame_number, page_number); 

  pt_set_entry(page_number, frame_number);

  /* Algorithme de remplacement des pages
   * À chaque nombre d'accès prédéfini (dans RATE) le programme va placer le
   * flag OLD de la page à true. Le programme ne swap que les vielles pages.
   */
  swap_count = (swap_count + 1) % RATE;
  if (!swap_count)
    for (int i = 0; i < NUM_FRAMES; i++)
      flag_table[i] |= OLD;
  
  return frame_number;
}

// Initialise la mémoire physique
void pm_init (FILE *backing_store, FILE *log)
{
  // Initialise les flags
  for (int i = 0; i < NUM_FRAMES; i++)
    flag_table[i] = OLD;
  pm_backing_store = backing_store;
  pm_log = log;
  memset (pm_memory, '\0', sizeof (pm_memory));
}

// Charge la page demandée du backing store
void pm_download_page (unsigned int page_number, unsigned int frame_number)
{
  download_count++;
  
  fseek(pm_backing_store, page_number * PAGE_FRAME_SIZE, SEEK_SET);
  fread(&pm_memory[page_number * PAGE_FRAME_SIZE],
	sizeof(char),
	PAGE_FRAME_SIZE,
	pm_backing_store);
  
  frame_table[frame_number] = page_number;
}

// Sauvegarde la frame spécifiée dans la page du backing store
void pm_backup_frame (unsigned int frame_number, unsigned int page_number)
{
  backup_count++;
  
  fseek(pm_backing_store, page_number * PAGE_FRAME_SIZE, SEEK_SET);
  fwrite(&pm_memory[page_number * PAGE_FRAME_SIZE],
	 sizeof(char),
	 PAGE_FRAME_SIZE,
	 pm_backing_store);
}

char pm_read (unsigned int physical_address)
{
  read_count++;
  
  // Met le flag OLD à false
  flag_table[physical_address / PAGE_FRAME_SIZE] |= ~OLD;
  
  return pm_memory[physical_address];
}

void pm_write (unsigned int physical_address, char c)
{
  write_count++;
  
  pm_memory[physical_address] = c;
  
  // Met le flag DIRTY à true et OLD à false
  flag_table[physical_address / PAGE_FRAME_SIZE] = DIRTY;
}


void pm_clean (void)
{
  // Fais l'enregistrement
  for (int i = 0; i < NUM_FRAMES; i++)
    if (flag_table[i] & DIRTY)
      pm_backup_frame(i, frame_table[i]);

  // Enregistre l'état de la mémoire physique.
  if (pm_log)
    {
      for (unsigned int i = 0; i < PHYSICAL_MEMORY_SIZE; i++)
	{
	  if (i % 80 == 0)
	    fprintf (pm_log, "%c\n", pm_memory[i]);
	  else
	    fprintf (pm_log, "%c", pm_memory[i]);
	}
    }
  fprintf (stdout, "Page downloads: %2u\n", download_count);
  fprintf (stdout, "Page backups  : %2u\n", backup_count);
  fprintf (stdout, "PM reads : %4u\n", read_count);
  fprintf (stdout, "PM writes: %4u\n", write_count);
}
