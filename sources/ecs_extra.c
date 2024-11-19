/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ecs_extra.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrouves <mrouves@42angouleme.fr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/19 15:08:20 by mrouves           #+#    #+#             */
/*   Updated: 2024/11/19 17:02:36 by mrouves          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ecs_extra.h"

static t_ecs_queue	*new_queue_node(t_ecs_alteration type, t_ecs_queue *next,
		t_ecs_queue_payload p)
{
	t_ecs_queue	*node;

	node = malloc(sizeof(t_ecs_queue));
	if (__builtin_expect(!node, 0))
		return (NULL);
	node->type = type;
	node->next = next;
	node->payload = p;
	return (node);
}

void	ecs_queue_add(t_ecs_queue **queue, t_ecs_alteration type,
		t_ecs_queue_payload p)
{
	if (__builtin_expect(!queue, 0))
		return ;
	*queue = new_queue_node(type, *queue, p);
}

void	ecs_queue_process(t_ecs *ecs, t_ecs_queue **queue)
{
	t_ecs_queue_payload	payload;
	t_ecs_queue			*tmp;

	if (__builtin_expect(!queue || !ecs, 0))
		return ;
	while (*queue)
	{
		payload = (*queue)->payload;
		if ((*queue)->type == ADD)
			ecs_entity_add(ecs, payload.id, payload.comp, payload.data);
		else if ((*queue)->type == CLONE)
			ecs_entity_clone(ecs, payload.id);
		else if ((*queue)->type == KILL)
			ecs_entity_kill(ecs, payload.id);
		else if ((*queue)->type == REMOVE)
			ecs_entity_remove(ecs, payload.id, payload.comp);
		tmp = (*queue)->next;
		free(*queue);
		*queue = tmp;
	}
}