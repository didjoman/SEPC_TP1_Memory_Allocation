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

static long int* min(long int* addr1, long int* addr2);
static long int* get_buddy(long int* addr, unsigned long size);
static int get_size_id(unsigned long size);
static void ls_add_in_head(long int** list, long int* element);
static long int* ls_remove_head(long int** list);

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

void * mem_alloc(unsigned long size)
{
	if(!zone_memoire || size <= 0 || size > ALLOC_MEM_SIZE)
		return NULL;

	long int wanted_size_id = get_size_id(size);
	long int i = wanted_size_id;
	// Si il n'y a pas de zone de la taille voulue, on en créé une.
	if(tzl_array[i] == NULL){
		// On cherche une zone de taille supérieure.
		while(++i <= BUDDY_MAX_INDEX && tzl_array[i] == NULL);
		// Cas où il n'y a plus de zones libres disponibles : 
		if(i > BUDDY_MAX_INDEX)
			return NULL;
		// Cas normal (zone libre trouvée) :
		// On coupe en 2 la zone trouvée, jusqu'à la taille voulue
		while(i != wanted_size_id){
			// On enlève la zone de la liste dans la TZL
			long int* zone_of_bigger_size = 
				ls_remove_head(&tzl_array[i]);
			
			// On ajoute les deux sous-zones à la TZL.
			// |- zone 1 (2eme moitié):
			ls_add_in_head(&tzl_array[i-1], 
						  (long int*)((long int)zone_of_bigger_size + (1<<(i-1))));
			// |- zone2 (1ere moitié):
			ls_add_in_head(&tzl_array[i-1], 
						  zone_of_bigger_size);
			--i;
		}
	}

	// Lorsqu'on a une zone libre de taille voulue,
	// On la dé-chaine de la TZL, puis on la retourne.
	return ls_remove_head(&tzl_array[wanted_size_id]);
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
	// Ajout à la TZL, de la zone à libérer :
	ls_add_in_head(&tzl_array[id], (long int*)area_to_free);

	// Tentative de récupération de plus de mémoire (par fusion de buddys):
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
			// On retire la zone à libérer de la TZL :
			// invariant: elle se trouve toujours en tête de tzl[i]
			ls_remove_head(&tzl_array[i]);
			// On retire le -buddy-de-la-zone-à-libérer- de la TZL : 
			if(buddy_pred == NULL || buddy_pred == area_to_free)
				tzl_array[i] = (long int*)*buddy;
			else
				*buddy_pred = *buddy; 

			// Ajout à la TZL de l'adresse de début de la zone crée
			// (issue de la fusion de la zone à libérer et du buddy)
			ls_add_in_head(&tzl_array[i+1], min(area_to_free, buddy));

			// Mise à jour de la nouvelle zone à fusionner (ou pas)
			area_to_free = tzl_array[i+1];
			size_allocated <<= 1;
		} else 	
			// Sinon, il n'y a plus rien d'autre à récupérer
			break;
	}
	return 0;
}


int mem_destroy()
{
	for(int i=0; i < BUDDY_MAX_INDEX; ++i)
		tzl_array[i] = NULL;

	free(zone_memoire);
	zone_memoire = 0;
	return 0;
}



/* ************************************************************************** */
/* *************************** Utility Functions **************************** */
/* ************************************************************************** */

/*
  Retourne: le minimum des deux adresses.
*/
static long int* min(long int* addr1, long int* addr2){
	return ((long int)addr1 > (long int)addr2) ? addr2 : addr1;
}

/*
  Retoune : L'adresse du Buddy de la zone mémoire donnée.
  Attention: L'adresse et la taille donnée en paramêtre doivent être valides.
 */
static long int* get_buddy(long int* addr, unsigned long size){
        return (long int *)(((((long int)addr - (long int)zone_memoire) ^ size)) + (long int)zone_memoire);
	/*
	  NOTE : Notre mémoire peut commencer n'importe où,
	  On la translate donc vers la gauche, pour qu'elle commence à 0, 
	  avant de faire le XOR.
	*/
}

/*
  Retourne: log2(size) ou la taille minimale allouable (= taille d'un pointeur)
*/
static int get_size_id(unsigned long size){
	// La taille minimum est 2^sizeof(pointeur).
	if(size < 8*sizeof(void*))
		size = 8*sizeof(void*);
	// Dans tous les autres cas, on renvoit borne_sup(log2(size))
	for(int i = 0; i <= BUDDY_MAX_INDEX; ++i)
		if((size >> i) <= 1){
			// Si size est une puissance de 2 on retourne i.
			if(size == (1<<i))
				return i;  // ex: 32 demandé = 1000 (reste 1) on donne 32
			// Sinon, on retourne la puissance de deux supérieure.
			else
				return i+1; // ex: 33 demandé, (reste 0) on donne 64
		}
	return 0;
}

/* ************************************************************************** */
/* ************ Quelques fonctions de manipulation des listes : ************* */
/* ************************************************************************** */

/*
  Garantie: Ajoute un élément en tête de liste.
*/
static void ls_add_in_head(long int** list, long int* element){
	long int* tmp = *list;
	*list = element;
	**list = (long int)tmp;
}

/*
  Garantie: Supprime et retourne l'élément en tête de liste.
*/
static long int* ls_remove_head(long int** list){
	long int* tmp = *list;
	*list = (long int*)**list;
	return tmp;
}
