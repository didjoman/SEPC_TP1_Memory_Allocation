/*****************************************************
 * Copyright Grégory Mounié 2008-2013                *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mem.h"

/** squelette du TP allocateur memoire */

void *zone_memoire = 0;
long int **tzl_array;

/* ecrire votre code ici */

int mem_init()
{
  if (! zone_memoire)
    zone_memoire = (void *) malloc(ALLOC_MEM_SIZE);
  if (zone_memoire == 0)
    {
      perror("mem_init:");
      return -1;
    }

  /* Init de la TZL
   * Au départ, la case de rang max contient la zone mémoire.
   * Toutes les autres cases sont vides (pointeur NULL) 
   */
  tzl_array = (long int**)zone_memoire;
  for(int i=0; i < BUDDY_MAX_INDEX; ++i)
	  tzl_array[i] = NULL;
  tzl_array[BUDDY_MAX_INDEX] = zone_memoire;

  //*tzl_array[BUDDY_MAX_INDEX] = (long int)NULL;

  return 0;
}

static int get_size_id(unsigned long size){
	for(int i = 0; i <= 20; ++i)
		if(size >> i == 1)
			return i;
	return 0;
}

void * mem_alloc(unsigned long size)
{
	// Si la taille mémoire demandée n'est pas valide, on retourne null
	if(size > ALLOC_MEM_SIZE)
		return NULL;
	
	long int* tmp;
	long int wanted_size_id = get_size_id(size);
	long int i = wanted_size_id;
	// Si il n'y a pas de zone de la taille voulue, on en créé une.
	if(tzl_array[i] == NULL){
		// On cherche une zone de taille supérieure.
		while(++i <= BUDDY_MAX_INDEX && tzl_array[i] == NULL);
		// Plus de zones libres disponibles : 
		if(tzl_array[i] == NULL)
			return NULL;
		
		// On coupe en 2 la zone trouvée, jusqu'à la taille voulue
		while(i != wanted_size_id){
			// On enlève la zone de la liste dans la TZL
			long int* zone_of_bigger_size = tzl_array[i];
			tzl_array[i] = (long int*)*zone_of_bigger_size;
			
			// On ajoute les deux sous-zones à la TZL.
			// |- zone 1: (chainage en tête)
			// TODO : faire fonction chainage() : 
			tmp = tzl_array[i-1];
			// (begin addr + half of the size of the current area)
			tzl_array[i-1] = (long int*)((long int)zone_of_bigger_size + (1<<(i-1))); 
			*tzl_array[i-1] = (long int)tmp;
			// |- zone2 :
			tmp = tzl_array[i-1];
			tzl_array[i-1] = zone_of_bigger_size;
			*(tzl_array[i-1]) = (long int)tmp;
			--i;
		}
	}

	// Lorsqu'on a une zone libre de taille voulue,
	// On la dé-chaine de la TZL, puis on la retourne.
	tmp = tzl_array[wanted_size_id];
	tzl_array[wanted_size_id] = (long int*)*tzl_array[wanted_size_id];
	return tmp;
}

int mem_free(void *ptr, unsigned long size)
{
	/*
	  On boucle de k à "max".
	     à chaque itération on regarde si on peut fusionner le block courant avec son buddy (si le buddy est libre)
	     puis l'élément courant devient le block de taille 2^(k+1) qui vient d'être fusionné, et ainsi de suite. 
	  
	  NOTE : l'adresse du budy = addr block XOR size; 
	  ex: 100 ^ 10 = 110
	 */
  return 0;
}


int mem_destroy()
{
  /* ecrire votre code ici */

  free(zone_memoire);
  zone_memoire = 0;
  return 0;
}

