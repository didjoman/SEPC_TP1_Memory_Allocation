/*****************************************************
 * Copyright Grégory Mounié 2008-2013                *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "mem.h"

/** squelette du TP allocateur memoire */

void *zone_memoire = 0;
long int *tzl_array[BUDDY_MAX_INDEX]; 

int mem_init()
{
	if (! zone_memoire)
		zone_memoire = (void *) malloc(ALLOC_MEM_SIZE);
	if (zone_memoire == 0) {
		perror("mem_init:");
		return -1;
	}

	/* Init de la TZL
	 * Au départ, la case de rang max contient la zone mémoire.
	 * Toutes les autres cases sont vides (pointeur NULL) 
	 */
	for(int i=0; i < BUDDY_MAX_INDEX; ++i)
		tzl_array[i] = NULL;
	
	tzl_array[BUDDY_MAX_INDEX] = zone_memoire;

	return 0;
}

/*
  Retourne: log2(size) ou la taille minimale allouable (= taille d'un pointeur)
*/
static int get_size_id(unsigned long size){
	if(size < 8*sizeof(void*))
		size = 8*sizeof(void*);

	for(int i = 0; i <= 20; ++i)
		if((size >> i) <= 1){
			if(((size >> i) == 1) && ((size ^ (1<<i)) == 0))
				return i;  // ex: 32 demandé = 1000 (reste 1) on donne 32
			else
				return i+1; // ex: 33 demandé, (reste 0) on donne 64
		}
	return 0;
}

void * mem_alloc(unsigned long size)
{
	if(!zone_memoire || size <= 0 || size > ALLOC_MEM_SIZE)
		return NULL;

	long int* tmp;
	long int wanted_size_id = get_size_id(size);
	long int i = wanted_size_id;
	// Si il n'y a pas de zone de la taille voulue, on en créé une.
	if(tzl_array[i] == NULL){
		// On cherche une zone de taille supérieure.
		while(++i <= BUDDY_MAX_INDEX && tzl_array[i] == NULL);
		// Plus de zones libres disponibles : 
		if(i > BUDDY_MAX_INDEX)
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

long int* min(long int* addr1, long int* addr2){
	return ((long int)addr1 > (long int)addr2) ? addr2 : addr1;
}

static long int* get_buddy(long int* addr, unsigned long size){
        return (long int *)(((((long int)addr - (long int)zone_memoire) ^ size)) + (long int)zone_memoire);
	/*
	  TODO : supprimer ça.
	  NOTE : I had a HUGE problem with the XOR : 
	  I think the XOR technic only works with addr from 0 (I did not know that -_-' ...
	  e.g. I allocated blocks of size 2^2 : 0x...08 and 0x...0C
	       Then I deallocated them. And guess what ? 
	       0x...08 XOR 8 = 0 , not 0x10 s it should in my example.
	 */
}

int mem_free(void *ptr, unsigned long size)
{
	// Vérif de la validité des paramètres : 
	if(size < 0 || size > ALLOC_MEM_SIZE || 
	   ptr < zone_memoire || ptr > zone_memoire + ALLOC_MEM_SIZE)
		return -1;

	void* area_to_free = ptr;
	int id = get_size_id(size);
	int size_allocated = 1<<id;
	// Ajout de la zone aux zones libres :
	long int *tmp = tzl_array[id];
	tzl_array[id] = (long int*)area_to_free;
	*(tzl_array[id]) = (long int)tmp;

	// Tentative de récupération de la mémoire (fusino de buddys) :
	for(int i = id; i < BUDDY_MAX_INDEX; ++i){
		// recherche du buddy dans la liste des zones de taille 2^id
		long int* buddy_pred = NULL;
		long int* buddy = tzl_array[i];
		while(buddy && get_buddy(area_to_free, size_allocated) != buddy){
			buddy_pred = buddy;
			buddy = (long int*)*buddy;
		}
		
		// Si le buddy est libre (= on l'a trouvé) on fusionne les zones
		if(get_buddy(area_to_free, size_allocated) == buddy){   		
			// remove de la zone à libérer de la TZL :
			// invariant: area_to_free est toujours en tête de la liste tzl[i]
			tzl_array[i] = (long int*)*(long int*)area_to_free;
			// remove du buddy de la TZL : 
			if(buddy_pred == NULL || buddy_pred == area_to_free)
				tzl_array[i] = (long int*)*buddy;
			else
				*buddy_pred = *buddy; 
			// Ajouter min(adr(zone1), adr(zone2)) à la liste i+1 			
			long int *tmp = tzl_array[i+1];
			tzl_array[i+1] = min(area_to_free, buddy);
			*(tzl_array[i+1]) = (long int)tmp;
			// Mise à jour de la nouvelle zone à libérer / fusinonner.
			area_to_free = tzl_array[i+1];
			size_allocated <<= 1;
		} else 	
			// Sinon, on arrête la boucle. 
			break;
	}
	return 0;
}


int mem_destroy()
{
	/* ecrire votre code ici */
	for(int i=0; i < BUDDY_MAX_INDEX; ++i)
		tzl_array[i] = NULL;

	free(zone_memoire);
	zone_memoire = 0;
	return 0;
}

