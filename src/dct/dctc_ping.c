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
 * dctc_ping: Client portion of ping test
 */

#include <daos_ct.h>
#include "dct_rpc.h"

static int
dct_ping_cb(struct daos_op_sp *sp, daos_event_t *ev, int rc)
{

	struct dct_ping_out *out;

	D_DEBUG(DF_MISC, "Entering dct_ping_cb\n");

	/* extract the RPC reply */
	out = dtp_reply_get(sp->sp_rpc);

	D_DEBUG(DF_MISC, "DCT Ping Return Val %d\n", out->ping_out);

	D_DEBUG(DF_MISC, "Leaving dct_ping_cb()");

	return rc;
}

int
dct_ping(uint32_t ping_val, daos_event_t *ev)
{

	D_DEBUG(DF_MISC, "Entering dct_ping()\n");

	struct dct_ping_in	*in;
	dtp_endpoint_t		ep;
	dtp_rpc_t		*rpc;
	int			rc;
	struct daos_op_sp      *sp;


	D_DEBUG(DF_MISC, "Ping Val to Issue: %d\n", ping_val);

	/* Harded coded enpoint stuff */
	uuid_clear(ep.ep_grp_id);
	ep.ep_rank = 0;
	ep.ep_tag = 0;

	/*
	 * if we dont have an event provided, we're running synchronously with
	 * the UNTESTED  private event thingamabob
	 */
	if (ev == NULL) {
		rc = daos_event_priv_get(&ev);
		if (rc)
			return rc;
	}

	/* Create RPC and allocate memory for the various field-eybops */
	rc = dct_req_create(daos_ev2ctx(ev), ep, DCT_PING, &rpc);

	/* Grab the input struct of the RPC */
	in = dtp_req_get(rpc);

	/* set the value we want to send out */
	in->ping_in = ping_val;

	/*
	 * Get the "scratch pad" data affiliated with this RPC
	 * used to maintain per-call invocation state (I think)
	 */
	sp = daos_ev2sp(ev);
	dtp_req_addref(rpc);
	sp->sp_rpc = rpc;

	/* Mark the event as inflight and register our various callbacks */
	rc = daos_event_launch(ev, NULL /*No abort CB*/, dct_ping_cb);

	/*
	 * If we fail, decrement the ref count....twice? Mimicking pattern seen
	 * elsewhere
	 */
	if (rc != 0) {
		dtp_req_decref(rpc);
		dtp_req_decref(rpc);
		return rc;
	}

	/* And now actually issue the darn RPC */
	rc = daos_rpc_send(rpc, ev);
	D_DEBUG(DF_MISC, "leaving dct_ping()\n");

	return rc;
}