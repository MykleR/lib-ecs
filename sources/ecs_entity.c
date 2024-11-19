/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ecs_entity.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrouves <mrouves@42angouleme.fr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 22:32:05 by mrouves           #+#    #+#             */
/*   Updated: 2024/11/19 16:57:49 by mrouves          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ecs.h"

uint32_t	ecs_entity_create(t_ecs *ecs)
{
	uint64_t	*ptr;
	uint64_t	id;

	if (__builtin_expect(!ecs || ecs->entity_len >= ECS_ENTITY_CAP, 0))
		return (ECS_ENTITY_CAP - 1);
	if (!ecs->free_list)
		return (ecs->entity_len++);
	ptr = (uint64_t *)ecs->free_list;
	id = ptr - ecs->masks;
	ecs->free_list = ecs->free_list->next;
	ecs->entity_len++;
	*ptr = 0;
	return (id);
}

bool	ecs_entity_has(t_ecs *ecs, uint32_t id, uint8_t comp)
{
	if (__builtin_expect(!ecs || id >= ECS_ENTITY_CAP, 0))
		abort();
	return ((*(ecs->masks + id) & (1 << comp)));
}

void	*ecs_entity_get(t_ecs *ecs, uint32_t id, uint8_t comp)
{
	size_t	offset;
	size_t	size;

	if (__builtin_expect(!ecs || id >= ECS_ENTITY_CAP
			|| comp > ecs->nb_comps, 0))
		abort();
	offset = *(ecs->mem_offsets + comp);
	size = ecs->mem_tsize;
	return ((uint8_t *)ecs->data + (id * size + offset));
}
