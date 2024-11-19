/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ecs.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mrouves <mrouves@42angouleme.fr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 13:52:19 by mrouves           #+#    #+#             */
/*   Updated: 2024/11/19 16:48:51 by mrouves          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ECS_H
# define ECS_H

# include "uint_list.h"
# include "utils.h"

# include <stdbool.h>
# include <stdint.h>

# define ECS_ENTITY_CAP 1024
# define QM_INIT_SIZE 16

typedef struct s_ecs_qentry
{
	t_ecs_ulist		*query;
	uint64_t		key;
}					t_ecs_qentry;

typedef struct s_ecs_qmap
{
	t_ecs_qentry	entries[QM_INIT_SIZE];
	uint16_t		capacity;
}					t_ecs_qmap;

typedef struct s_ecs_flist
{
	struct s_ecs_flist	*next;
}	t_ecs_flist;

// Struct to represent the ecs universe
typedef struct s_ecs
{
	uint64_t		masks[ECS_ENTITY_CAP];
	void			*data;
	t_ecs_qmap		*queries;
	t_ecs_flist		*free_list;
	uint16_t		mem_offsets[64];
	uint16_t		mem_sizes[64];
	uint32_t		mem_tsize;
	uint32_t		entity_len;
	uint8_t			nb_comps;
}					t_ecs;

t_ecs			*ecs_create(uint16_t comps[64], uint8_t nb_comps);
void			ecs_destroy(t_ecs *ecs);
t_ecs_ulist		*ecs_query(t_ecs *ecs, uint64_t signature);

uint32_t		ecs_entity_create(t_ecs *ecs);
uint32_t		ecs_entity_clone(t_ecs *ecs, uint32_t id);
void			ecs_entity_kill(t_ecs *ecs, uint32_t id);
void			*ecs_entity_get(t_ecs *ecs, uint32_t id, uint8_t comp);
void			ecs_entity_remove(t_ecs *ecs, uint32_t id, uint8_t comp);
void			ecs_entity_add(t_ecs *ecs, uint32_t id,
					uint8_t comp, void *data);
bool			ecs_entity_has(t_ecs *ecs, uint32_t id, uint8_t comp);

t_ecs_qmap		*qm_create(void);
void			qm_destroy(t_ecs_qmap *map);
t_ecs_qentry	*qm_get(t_ecs_qmap *map, uint64_t key, bool *res);
bool			qm_is_inquery(uint64_t key, uint64_t mask);

#endif
