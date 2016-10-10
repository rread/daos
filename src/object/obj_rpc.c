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
 * DSR: RPC Protocol Serialization Functions
 */

#include <daos/event.h>
#include <daos/rpc.h>
#include "obj_rpc.h"

static struct dtp_msg_field *obj_update_in_fields[] = {
	&DMF_OID,	/* object ID */
	&DMF_UUID,	/* container handle uuid */
	&DMF_UINT64,	/* epoch */
	&DMF_UINT32,	/* count of vec_iod and sg */
	&DMF_UINT32,	/* pad */
	&DMF_IOVEC,	/* dkey */
	&DMF_VEC_IOD_ARRAY, /* daos_vector */
	&DMF_BULK_ARRAY,    /* BULK ARRAY */
};

static struct dtp_msg_field *obj_fetch_out_fields[] = {
	&DMF_INT,	/* status */
	&DMF_UINT32,	/* pad */
	&DMF_REC_SIZE_ARRAY, /* actual size of records */
};

static struct dtp_msg_field *obj_dkey_enum_in_fields[] = {
	&DMF_OID,	/* object ID */
	&DMF_UUID,	/* container handle uuid */
	&DMF_UINT64,	/* epoch */
	&DMF_UINT32,	/* number of kds */
	&DMF_UINT32,	/* pad */
	&DMF_DAOS_HASH_OUT, /* hash anchor */
	&DMF_BULK, /* BULK array for dkey */
};

static struct dtp_msg_field *obj_dkey_enum_out_fields[] = {
	&DMF_INT,		/* status of the request */
	&DMF_UINT32,		/* pad */
	&DMF_DAOS_HASH_OUT,	/* hash anchor */
	&DMF_KEY_DESC_ARRAY,	/* kds array */
};

static struct dtp_req_format DQF_OBJ_UPDATE =
	DEFINE_DTP_REQ_FMT_ARRAY("DAOS_OBJ_UPDATE",
				 obj_update_in_fields,
				 ARRAY_SIZE(obj_update_in_fields),
				 dtp_single_out_fields, 1);

static struct dtp_req_format DQF_OBJ_FETCH =
	DEFINE_DTP_REQ_FMT("DAOS_OBJ_UPDATE",
			   obj_update_in_fields,
			   obj_fetch_out_fields);

static struct dtp_req_format DQF_DKEY_ENUMERATE =
	DEFINE_DTP_REQ_FMT("DAOS_DKEY_ENUM",
			   obj_dkey_enum_in_fields,
			   obj_dkey_enum_out_fields);

struct daos_rpc daos_obj_rpcs[] = {
	{
		.dr_name	= "DAOS_OBJ_UPDATE",
		.dr_opc		= DAOS_OBJ_RPC_UPDATE,
		.dr_ver		= 1,
		.dr_flags	= 0,
		.dr_req_fmt	= &DQF_OBJ_UPDATE,
	}, {
		.dr_name	= "DAOS_OBJ_FETCH",
		.dr_opc		= DAOS_OBJ_RPC_FETCH,
		.dr_ver		= 1,
		.dr_flags	= 0,
		.dr_req_fmt	= &DQF_OBJ_FETCH,
	}, {
		.dr_name	= "DAOS_DKEY_ENUM",
		.dr_opc		= DAOS_OBJ_RPC_ENUMERATE,
		.dr_ver		= 1,
		.dr_flags	= 0,
		.dr_req_fmt	= &DQF_DKEY_ENUMERATE,
	}, {
		.dr_opc		= 0
	}
};

int
obj_req_create(dtp_context_t dtp_ctx, dtp_endpoint_t tgt_ep,
	       dtp_opcode_t opc, dtp_rpc_t **req)
{
	dtp_opcode_t opcode;

	opcode = DAOS_RPC_OPCODE(opc, DAOS_OBJ_MODULE, 1);

	return dtp_req_create(dtp_ctx, tgt_ep, opcode, req);
}

void
obj_reply_set_status(dtp_rpc_t *rpc, int status)
{
	int *ret;

	/* FIXME; The right way to do it might be find the
	 * status offset and set it, but let's put status
	 * in front of the bulk reply for now
	 */
	D_ASSERT(rpc != NULL);
	ret = dtp_reply_get(rpc);
	D_ASSERT(ret != NULL);
	*ret = status;
}

int
obj_reply_get_status(dtp_rpc_t *rpc)
{
	int *ret;

	/* FIXME; The right way to do it might be find the
	 * status offset and set it, but let's put status
	 * in front of the bulk reply for now
	 */
	ret = dtp_reply_get(rpc);
	return *ret;
}