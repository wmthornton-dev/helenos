/*
 * Copyright (c) 2017 Ondrej Hlavaty
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup drvusbxhci
 * @{
 */
/** @file
 * TRB-related structures of the xHC.
 *
 * This file contains all the types of TRB and the TRB ring handling.
 */

#ifndef XHCI_TRB_H
#define XHCI_TRB_H

#include "common.h"

/**
 * TRB types: section 6.4.6, table 139
 */
enum xhci_trb_type {
	XHCI_TRB_TYPE_RESERVED = 0,

// Transfer ring:
	XHCI_TRB_TYPE_NORMAL,
	XHCI_TRB_TYPE_SETUP_STAGE,
	XHCI_TRB_TYPE_DATA_STAGE,
	XHCI_TRB_TYPE_STATUS_STAGE,
	XHCI_TRB_TYPE_ISOCH,
	XHCI_TRB_TYPE_LINK,
	XHCI_TRB_TYPE_EVENT_DATA,
	XHCI_TRB_TYPE_NO_OP,

// Command ring:
	XHCI_TRB_TYPE_ENABLE_SLOT_CMD,
	XHCI_TRB_TYPE_DISABLE_SLOT_CMD,
	XHCI_TRB_TYPE_ADDRESS_DEVICE_CMD,
	XHCI_TRB_TYPE_CONFIGURE_ENDPOINT_CMD,
	XHCI_TRB_TYPE_EVALUATE_CONTEXT_CMD,
	XHCI_TRB_TYPE_RESET_ENDPOINT_CMD,
	XHCI_TRB_TYPE_STOP_ENDPOINT_CMD,
	XHCI_TRB_TYPE_SET_TR_DEQUEUE_POINTER_CMD,
	XHCI_TRB_TYPE_RESET_DEVICE_CMD,
	XHCI_TRB_TYPE_FORCE_EVENT_CMD,
	XHCI_TRB_TYPE_NEGOTIATE_BANDWIDTH_CMD,
	XHCI_TRB_TYPE_SET_LATENCY_TOLERANCE_VALUE_CMD,
	XHCI_TRB_TYPE_GET_PORT_BANDWIDTH_CMD,
	XHCI_TRB_TYPE_FORCE_HEADER_CMD,
	XHCI_TRB_TYPE_NO_OP_CMD,
// Reserved: 24-31

// Event ring:
	XHCI_TRB_TYPE_TRANSFER_EVENT = 32,
	XHCI_TRB_TYPE_COMMAND_COMPLETION_EVENT,
	XHCI_TRB_TYPE_PORT_STATUS_CHANGE_EVENT,
	XHCI_TRB_TYPE_BANDWIDTH_REQUEST_EVENT,
	XHCI_TRB_TYPE_DOORBELL_EVENT,
	XHCI_TRB_TYPE_HOST_CONTROLLER_EVENT,
	XHCI_TRB_TYPE_DEVICE_NOTIFICATION_EVENT,
	XHCI_TRB_TYPE_MFINDEX_WRAP_EVENT,
};

/**
 * TRB template: section 4.11.1
 */
typedef struct xhci_trb {
	xhci_qword_t parameter;
	xhci_dword_t status;
	xhci_dword_t control;
} xhci_trb_t;

#define TRB_TYPE(trb)           XHCI_DWORD_EXTRACT((trb).control, 15, 10)
#define TRB_LINK_TC(trb)        XHCI_DWORD_EXTRACT((trb).control, 1, 1)

/**
 * The Chain bit is valid only in specific TRB types.
 */
static inline bool xhci_trb_is_chained(xhci_trb_t *trb) {
	const int type = TRB_TYPE(*trb);
	const bool chain_bit = XHCI_DWORD_EXTRACT(trb->control, 4, 4);

	return chain_bit &&
	    (type == XHCI_TRB_TYPE_NORMAL
	    || type == XHCI_TRB_TYPE_DATA_STAGE
	    || type == XHCI_TRB_TYPE_STATUS_STAGE
	    || type == XHCI_TRB_TYPE_ISOCH);
}

static inline void xhci_trb_set_cycle(xhci_trb_t *trb, bool cycle)
{
	xhci_dword_set_bits(&trb->control, cycle, 1, 1);
}

static inline void xhci_trb_link_fill(xhci_trb_t *trb, uintptr_t next_phys)
{
	// TRBs require 16-byte alignment
	assert((next_phys & 0xf) == 0);

	xhci_dword_set_bits(&trb->control, XHCI_TRB_TYPE_LINK, 15, 10);
	xhci_qword_set(&trb->parameter, next_phys);
}

static inline void xhci_trb_copy(xhci_trb_t *dst, xhci_trb_t *src)
{
    /*
     * As we do not know, whether our architecture is capable of copying 16
     * bytes atomically, let's copy the fields one by one.
     */
    dst->parameter = src->parameter;
    dst->status = src->status;
    dst->control = src->control;
}


/**
 * Event Ring Segment Table: section 6.5
 */
typedef struct xhci_erst_entry {
	xhci_qword_t rs_base_ptr;       // sans bits 0-6
	xhci_dword_t size;              // only low 16 bits, the rest is reserved
} xhci_erst_entry_t;

#endif
