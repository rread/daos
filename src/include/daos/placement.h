/**
 * (C) Copyright 2016 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the Apache License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 */
/**
 * This file is part of daos
 *
 * include/daos/placement.h
 */

#ifndef __DAOS_PLACEMENT_H__
#define __DAOS_PLACEMENT_H__

#include <daos/common.h>
#include <daos/pool_map.h>
#include <daos/object.h>

/** types of placement maps */
typedef enum {
	PL_TYPE_UNKNOWN,
	/** only support ring map for the time being */
	PL_TYPE_RING,
	/** reserved */
	PL_TYPE_PETALS,
} pl_map_type_t;

struct pl_map_init_attr {
	pl_map_type_t		ia_type;
	uint32_t		ia_ver;
	union {
		struct pl_ring_init_attr {
			pool_comp_type_t	domain;
			unsigned int		ring_nr;
		} ia_ring;
	};
};

struct pl_map;

struct pl_target {
	uint32_t		pt_pos;
};

/** A group of targets */
struct pl_target_grp {
	/** pool map version to generate this layout */
	uint32_t		 tg_ver;
	/** number of targets */
	unsigned int		 tg_target_nr;
	/** array of targets */
	struct pl_target	*tg_targets;
};

struct pl_obj_layout {
	uint32_t		 ol_ver;
	uint32_t		 ol_nr;
	uint32_t		*ol_shards;
	uint32_t		*ol_targets;
};

int  pl_map_create(struct pool_map *poolmap, struct pl_map_init_attr *mia,
		   struct pl_map **mapp);
void pl_map_destroy(struct pl_map *map);
void pl_map_print(struct pl_map *map);
struct pl_map *pl_map_find(daos_handle_t coh, daos_obj_id_t oid);
void pl_map_addref(struct pl_map *map);
void pl_map_decref(struct pl_map *map);

void pl_obj_layout_free(struct pl_obj_layout *layout);
int  pl_obj_layout_alloc(unsigned int shard_nr,
			 struct pl_obj_layout **layout_pp);

int pl_obj_place(struct pl_map *map,
		 struct daos_obj_md *md,
		 struct daos_obj_shard_md *shard_md,
		 struct pl_obj_layout **layout_pp);

int pl_obj_find_rebuild(struct pl_map *map,
			struct daos_obj_md *md,
			struct daos_obj_shard_md *shard_md,
			struct pl_target_grp *tgp_failed,
			uint32_t *tgt_rebuild);

int pl_obj_find_reint(struct pl_map *map,
		      struct daos_obj_md *md,
		      struct daos_obj_shard_md *shard_md,
		      struct pl_target_grp *tgp_recov,
		      uint32_t *tgt_reint);

int  daos_placement_init(struct pool_map *po_map);
void daos_placement_fini(struct pool_map *po_map);
int  daos_placement_refresh(struct pool_map *old_map,
			    struct pool_map *new_map);

#endif /* __DAOS_PLACEMENT_H__ */
