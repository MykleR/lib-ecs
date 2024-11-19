/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   uint_list.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrouves <mrouves@42angouleme.fr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 19:06:16 by mrouves           #+#    #+#             */
/*   Updated: 2024/11/19 17:01:24 by mrouves          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "uint_list.h"

t_ecs_ulist	*list_create(uint32_t val, t_ecs_ulist *prev, t_ecs_ulist *next)
{
	t_ecs_ulist	*node;

	node = malloc(sizeof(t_ecs_ulist));
	if (__builtin_expect(node == NULL, 0))
		return (NULL);
	node->start = val;
	node->end = val;
	if (next)
		next->prev = node;
	node->next = next;
	if (prev)
		prev->next = node;
	node->prev = prev;
	return (node);
}

void	list_clear(t_ecs_ulist **lst)
{
	t_ecs_ulist	*next;

	if (__builtin_expect(lst == NULL, 0))
		return ;
	while (*lst)
	{
		next = (*lst)->next;
		free(*lst);
		*lst = next;
	}
}

t_ecs_ulist	*list_iter(t_ecs_ulist *lst, uint32_t *prev)
{
	static uint32_t	index = UINT32_MAX;

	if (__builtin_expect(prev == NULL, 0))
		return (NULL);
	if (!lst)
	{
		index = UINT32_MAX;
		*prev = index;
		return (NULL);
	}
	if (index == UINT32_MAX)
		index = lst->start;
	else
		index++;
	if (index > lst->end)
	{
		lst = lst->next;
		if (lst)
			index = lst->start;
		else
			index = UINT32_MAX;
	}
	*prev = index;
	return (lst);
}

uint32_t	list_popfront(t_ecs_ulist **lst)
{
	uint32_t	val;

	if (__builtin_expect(!lst || !(*lst), 0))
		return (-1);
	val = (*lst)->start;
	list_remove(lst, val);
	return (val);
}
