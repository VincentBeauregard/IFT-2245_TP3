#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "conf.h"
#include "common.h"
#include "vmm.h"
#include "tlb.h"
#include "pt.h"
#include "pm.h"

/* Include ajouté */
#include <math.h>

static unsigned int read_count = 0;
static unsigned int write_count = 0;
static FILE* vmm_log;

void vmm_init (FILE *log)
{
  // Initialise le fichier de journal.
  vmm_log = log;
}


// NE PAS MODIFIER CETTE FONCTION
static void vmm_log_command (FILE *out, const char *command,
                             unsigned int laddress, /* Logical address. */
		             unsigned int page,
                             unsigned int frame,
                             unsigned int offset,
                             unsigned int paddress, /* Physical address.  */
		             char c) /* Caractère lu ou écrit.  */
{
  if (out)
    fprintf (out, "%s[%c]@%05d: p=%d, o=%d, f=%d pa=%d\n", command, c, laddress,
	     page, offset, frame, paddress);
}

/* Fonction ajoutée */
// Gère l'accès aux frame
int page_lookup(unsigned int page_number, bool write)
{

  int frame_number = tlb_lookup(page_number, write);

  // TLB-miss -- Va chercher dans la table et met le TLB à jour
  if (frame_number < 0) {
    frame_number = pt_lookup(page_number);

    // page fault
    if (frame_number < 0)
      frame_number = pm_swap(page_number);
    
    tlb_add_entry(page_number, frame_number, write);
  }

  return frame_number;
}

/* Effectue une lecture à l'adresse logique `laddress`.  */
char vmm_read (unsigned int laddress)
{
  char c;
  read_count++;

  // Récupère les information encodées dans laddress
  unsigned int page_bits   = 2 * NUM_PAGES - 1;
  unsigned int offset_bits = 2 * PAGE_FRAME_SIZE - 1;
  unsigned int page_number = (laddress >> ((int) log(offset_bits) + 2)) & page_bits;
  unsigned int offset      =  laddress                                  & offset_bits;
  
  unsigned int frame_number = page_lookup(page_number, false);
  
  unsigned int physical_address = frame_number * PAGE_FRAME_SIZE + offset;

  c = pm_read(physical_address);
  
  vmm_log_command (stdout, "READING",
		   laddress,
		   page_number,
		   frame_number, offset,
		   physical_address,
		   c);
  return c;
}

/* Effectue une écriture à l'adresse logique `laddress`.  */
void vmm_write (unsigned int laddress, char c)
{
  write_count++;

  // Récupère les information encodées dans laddress
  unsigned int page_bits   = 2 * NUM_PAGES - 1;
  unsigned int offset_bits = 2 * PAGE_FRAME_SIZE - 1;
  unsigned int page_number = (laddress >> ((int) log(offset_bits) + 2)) & page_bits;
  unsigned int offset      =  laddress                                  & offset_bits;
  
  unsigned int frame_number = page_lookup(page_number, true);
  
  unsigned int physical_address = frame_number * PAGE_FRAME_SIZE + offset;

  pm_write(physical_address, c);
  
  vmm_log_command (stdout, "WRITING",
		   laddress,
		   page_number,
		   frame_number, offset,
		   physical_address,
		   c);
}


// NE PAS MODIFIER CETTE FONCTION
void vmm_clean (void)
{
  fprintf (stdout, "VM reads : %4u\n", read_count);
  fprintf (stdout, "VM writes: %4u\n", write_count);
}
