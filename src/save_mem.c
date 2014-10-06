/*****************************************************
 * Copyright Grégory Mounié 2008-2013                *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mem.h"
#include <inttypes.h>


/** squelette du TP allocateur memoire */

void *zone_memoire = 0;

/* ecrire votre code ici */
int **tzl_array;

int mem_init()
{
	if (! zone_memoire)
		zone_memoire = (void *) malloc(ALLOC_MEM_SIZE);
	if (zone_memoire == 0){
		perror("mem_init:");
		return -1;
	}


	/* 
	 * Au départ, quand toute la mémoire est libre, toutes les listes sont 
	 * vide, sauf la zone de taille maximale (2^20).
	 *
	 * Les informations sur les zones libres sont stockées dans les zones 
	 * libres elle-même, en particulier le pointeur vers le suivant dans la liste.
	*/
  
	// Initialisation du Tableau de Zones Libres (tableau de listes (=pointeur))
	tzl_array = (int**)zone_memoire;
	for(int i=0; i < BUDDY_MAX_INDEX; ++i)
		tzl_array[i] = NULL;

	// Init de la première zone libre (celle de taille BUDDY_MAX_INDEX)
	tzl_array[BUDDY_MAX_INDEX] = (int*)zone_memoire;
	*tzl_array[BUDDY_MAX_INDEX] = (int)NULL;

	return 0;
}

static int get_size_id(unsigned long size){
	for(int i = 0; i <= 20; ++i)
		if(size >> i == 0)
			return i;
	return 0;
}

void * mem_alloc(unsigned long size)
{

	// Si la taille mémoire demandée n'est pas valide, on retourne null
	if(size > ALLOC_MEM_SIZE)
		return NULL;
	
	int* tmp;
	int wanted_size_id = get_size_id(size);
	int i = wanted_size_id;
	// Si il n'y a pas de zone de la taille voulue, on en créé une.
	if(tzl_array[i] == NULL){
		// On cherche une zone de taille supérieure.
		while(++i <= BUDDY_MAX_INDEX && tzl_array[i] != NULL);

		// Plus de zones libres disponibles : 
		if(tzl_array[i] == NULL)
			return NULL;
		
		// On coupe en 2 la zone trouvée, jusqu'à la taille voulue
		while(i != wanted_size_id){
			// On enlève la zone de la liste dans la TZL
			int* zone_of_bigger_size = tzl_array[i];
			tzl_array[i] = (int*)*zone_of_bigger_size;
			
			// On ajoute les deux sous-zones à la TZL.			
			// |- zone 1: (chainage en tête)
			tmp = tzl_array[i-1];
			tzl_array[i-1] = (int*)((int)zone_of_bigger_size + (1<<(i-1)));
			*tzl_array[i-1] = tmp;
			// |- zone2 :
			tmp = tzl_array[i-1];
			tzl_array[i-1] = zone_of_bigger_size;
			((int*)*tzl_array[i-1]) = tmp;
			--i;
		}
	}

	// Lorsqu'on a une zone libre de taille voulue,
	// On la dé-chaine de la TZL, puis on la retourne.
	tmp = tzl_array[i];
	tzl_array[i] = (int*)*tzl_array[i];
	return tmp;
}

int mem_free(void *ptr, unsigned long size)
{
	/* ecrire votre code ici */

	/*The "buddy" of each block can be found with an exclusive OR of the block's address and the block's size.*/
	return 0;
}


int mem_destroy()
{
	/* ecrire votre code ici */

	free(zone_memoire);
	zone_memoire = 0;
	return 0;
}

