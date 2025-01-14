/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prototype_v2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mykle <mykle@42angouleme.fr>               +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 12:02:52 by mykle             #+#    #+#             */
/*   Updated: 2025/01/14 20:52:25 by mrouves          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define ECS_ENTITY_CAP 1024

#define ECS_ALIVE 0x8000000000000000


//
//	DOUBLE LINKED LIST TYPE
//
typedef struct s_double_linked_list
{
	struct s_double_linked_list	*prev;
	struct s_double_linked_list	*next;
	void						*data;
}	t_dl_list;

t_dl_list	*dl_list_node(void *data)
{
	t_dl_list	*node;

	node = malloc(sizeof(t_dl_list));
	if (!node)
		return (NULL);
	node->data = data;
	node->next = NULL;
	node->prev = NULL;
	return (node);
}

void	dl_list_insert(t_dl_list **after, t_dl_list *node)
{
	if (__builtin_expect(!after, 0))
		return ;
	if (!(*after))
	{
		*after = node;
		return ;
	}
	node->next = (*after)->next;
	node->prev = (*after);
	(*after)->next = node;
	if (node->next)
		node->next->prev = node;
}

void	dl_list_delone(t_dl_list *node, void (*del)(void *))
{
	if (__builtin_expect(!node, 0))
		return ;
	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;
	if (del)
		del(node->data);
	free(node);
}

void	dl_list_iter(t_dl_list *head, void (*f)(t_dl_list *, void *), void *arg)
{
	while (head)
	{
		f(head, arg);
		head = head->next;
	}
}

void	dl_list_clear(t_dl_list **head, void (*del)(void *))
{
	t_dl_list	*tmp;

	while (head && *head)
	{
		tmp = (*head)->next;
		dl_list_delone(*head, del);
		*head = tmp;
	}
}

//
//	UNORDERED MAP
//
typedef struct s_uint64_entry
{
	uint64_t	key;
	void		*value;
}	t_uint64_entry;

typedef struct s_uint64_map
{
	t_dl_list		**entries;
	uint32_t		len;
	uint32_t		cap;
}	t_uint64_map;

bool	uint64_map_create(t_uint64_map *map, uint32_t cap)
{
	cap--;
	cap |= cap >> 1;
	cap |= cap >> 2;
	cap |= cap >> 4;
	cap |= cap >> 8;
	cap |= cap >> 16;
	cap++;
	if (__builtin_expect(!map || !cap, 0))
		return (false);
	map->cap = cap;
	map->len = 0;
	map->entries = calloc(sizeof(t_dl_list *), cap);
	return (map->entries != NULL);
}

void	uint64_map_destroy(t_uint64_map *map)
{
	if (__builtin_expect(!map, 0))
		return ;
	while (map->cap--)
		dl_list_clear(map->entries + map->cap, NULL);
	free(map->entries);
	map->entries = NULL;
	map->len = 0;
}

void	*uint64_map_get(t_uint64_map *map, uint64_t key)
{
	t_dl_list		*entries;
	t_uint64_entry	*entry;

	if (__builtin_expect(!map, 0))
		return (NULL);
	entries = map->entries[key & (map->cap - 1)];
	while (entries)
	{
		entry = (t_uint64_entry *)entries->data;
		if (entry->key == key)
			return (entry->value);
		entries = entries->next;
	}
	return (NULL);
}

typedef struct s_free_list
{
	struct s_free_list	*next;
}	t_free_list;

typedef struct s_ecs_archetype
{
	void				*data;
	uint32_t			*ids;
	uint64_t			mask;
	uint32_t			len;
	uint32_t			cap;
	uint16_t			mem_size;
}	t_ecs_archetype;

typedef struct s_ecs
{
	uint64_t		masks[ECS_ENTITY_CAP];
	t_free_list		*free_list;
	t_uint64_map	archetypes;
	t_uint64_map	comps;
	uint32_t		len;
	uint16_t		comp_size[63];
	uint8_t			comp_nb;
}	t_ecs;

t_ecs	*ecs_create(uint8_t nb, uint16_t *comps)
{
	t_ecs	*ecs;

	if (__builtin_expect(nb > 63 || !comps, 0))
		return (NULL);
	ecs = malloc(sizeof(t_ecs));
	if (__builtin_expect(!ecs, 0))
		return (NULL);
	ecs->count = 0;
	ecs->comp_nb = nb;
	ecs->free_list = NULL;
	memset(ecs->ids, 0, sizeof(uint32_t) * ECS_ENTITY_CAP);
	memset(ecs->masks, 0, sizeof(uint64_t) * ECS_ENTITY_CAP);
	memset(ecs->comp_size, 0, sizeof(uint16_t) * 63);
	ecs->archetypes = uint64_map_create();
	ecs->comps = uint64_map_create();
	if (__builtin_expect(ecs->archetypes != NULL && ecs->comps != NULL, 1))
		return (ecs);
	free(ecs);
	free(ecs->comps);
	free(ecs->archetypes);
	return (NULL);
}

static void	__ecs_destroy_archetype(void	*data)
{
	t_ecs_archetype	*arch;

	arch = (t_ecs_archetype *)data;
	free(arch->data);
	free(arch->ids);
	free(arch);
}

void	ecs_destroy(t_ecs *ecs)
{
	if (!ecs)
		return ;
	uint64_map_destroy(ecs->archetypes, __ecs_destroy_archetype);
	uint64_map_destroy(ecs->comps, NULL);
	free(ecs);
}

void	ecs_iter_query(t_ecs *ecs, uint64_t signature, void (*sys)(t_ecs *, void *))
{
	t_ecs_archetype	*type;
	uint32_t		i;

	type = (t_ecs_archetype *)uint64_map_get(ecs->archetypes, signature);
	if (__builtin_expect(!type, 0))
		return ;
	i = -1;
	while (++i < type->count)
		sys(ecs, (type->data + i * type->mem_size));
}


void	ecs_iter_queries(t_ecs *ecs, uint64_t signature, void (*sys)(t_ecs *, void *));

uint32_t	*ecs_query(t_ecs *ecs, uint64_t signature, uint32_t *nb)
{
	t_ecs_archetype	*type;

	if (__builtin_expect(!ecs || !nb || !signature, 0))
		return (NULL);
	type = (t_ecs_archetype *)uint64_map_get(ecs->archetypes, signature);
	if (__builtin_expect(!type, 0))
		return (NULL);
	*nb	= type->count;
	return (type->ids);
}

uint32_t	ecs_entity_create(t_ecs  *ecs);
uint32_t	ecs_entity_clone(t_ecs *ecs, uint32_t id);
void		ecs_entity_kill(t_ecs *ecs, uint32_t id);
void		*ecs_entity_get(t_ecs *ecs, uint32_t id, uint8_t comp);
void		ecs_entity_remove(t_ecs *ecs, uint32_t id, uint8_t comp);
void		ecs_entity_add(t_ecs *ecs, uint32_t id, uint8_t comp, void *data);
