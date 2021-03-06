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
/*
 * client portion of the fetch operation
 *
 * dctc is the DCT part of client module/library. It exports part of the DCT
 * API defined in daos_tier.h.
 */
#define DD_SUBSYS	DD_FAC(tier)

#include <daos_types.h>
#include <daos_tier.h>
#include <daos/pool.h>
#include "dct_rpc.h"

static int
dct_fetch_cb(void *arg, daos_event_t *ev, int rc)
{
	struct daos_op_sp		*sp = arg;
	struct dc_pool			*pool = (struct dc_pool *)sp->sp_arg;
	struct tier_fetch_out		*tfo;

	if (rc) {
		D_ERROR("RPC error while fetching: %d\n", rc);
		D_GOTO(out, rc);
	}

	tfo = crt_reply_get(sp->sp_rpc);
	rc = tfo->tfo_ret;
	if (rc) {
		D_ERROR("failed to fetch: %d\n", rc);
		D_GOTO(out, rc);
	}

	sp->sp_hdl.cookie = 0;
out:
	crt_req_decref(sp->sp_rpc);
	dc_pool_put(pool);
	return rc;
}

int
dc_tier_fetch_cont(daos_handle_t poh, const uuid_t cont_id,
		   daos_epoch_t fetch_ep, daos_oid_list_t *obj_list,
		   daos_event_t *ev)
{

	struct tier_fetch_in	*in;
	crt_endpoint_t		ep;
	crt_rpc_t		*rpc;
	int			rc;
	struct daos_op_sp       *sp;
	struct dc_pool		*pool;

	D_DEBUG(DF_MISC, "Entering daos_fetch_container()\n");

	/* FIXME Harded coded enpoint stuff */
	ep.ep_grp = NULL;
	ep.ep_rank = 0;
	ep.ep_tag = 0;

	/* Create RPC and allocate memory for the various field-eybops */
	rc = dct_req_create(daos_ev2ctx(ev), ep, TIER_FETCH, &rpc);

	/* Grab the input struct of the RPC */
	in = crt_req_get(rpc);

	pool = dc_pool_lookup(poh);
	if (pool == NULL)
		return -DER_NO_HDL;

	uuid_copy(in->tfi_co_hdl, cont_id);
	uuid_copy(in->tfi_pool, pool->dp_pool);
	uuid_copy(in->tfi_pool_hdl, pool->dp_pool_hdl);
	in->tfi_ep  = fetch_ep;

	/*
	 * Get the "scratch pad" data affiliated with this RPC
	 * used to maintain per-call invocation state (I think)
	 */
	sp = daos_ev2sp(ev);
	crt_req_addref(rpc);
	sp->sp_rpc = rpc;
	sp->sp_hdl = poh;
	sp->sp_arg = pool;

	rc = daos_event_register_comp_cb(ev, dct_fetch_cb, sp);
	if (rc != 0)
		D_GOTO(out_req_put, rc);

	/* Mark the event as inflight and register our various callbacks */
	rc = daos_event_launch(ev);
	if (rc != 0)
		D_GOTO(out_req_put, rc);

	/* And now actually issue the darn RPC */
	rc = daos_rpc_send(rpc, ev);
	D_DEBUG(DF_MISC, "leaving dct_ping()\n");

	return rc;

out_req_put:
	crt_req_decref(rpc);
	crt_req_decref(rpc);
	return rc;
}
