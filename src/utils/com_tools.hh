#ifndef __COM_TOOLS_H__

#define __COM_TOOLS_H__

#include "DIET_client.h"

/* Retourne la taille d'un objet d'un des types de base
 * 
 * Rq: La taille est alignee (on utilise sizeof)
 */
int base_type_size(diet_base_type_t base_type);

/* Retourne une estimation de la taille du parametre, qui va
 * transiter sur le reseau lors de l'envoi des donnees 
 * (c'est pourquoi on ne prend pas en compte la taille de
 *  toutes les structures)
 */
long parameter_size(diet_arg_t *p);

/* Retourne le nombre d'octets (appoximatif) contenu dans le profile
 * En l'occurrence, c'est la somme des tailles de tous les parametres
 */
long profile_size(diet_profile_t *p);

/* Initialise la transmission des informations concernant la
 * circulation des paquets sur le reseau 
 */
void init_communications();

/* Indique a un programme externe (typiquement la demo de visualisation
 * du reseau) qu'une communication a lieu entre deux noeuds du reseau
 */
void add_communication(char* node0, char* node1, long size);

#endif // __COM_TOOLS_H_
