/*
 * Copyright (c) 2016 - 2018 Cadence Design Systems Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _XRP_HOST_COMMON_H
#define _XRP_HOST_COMMON_H

#include <stdlib.h>
#include "xrp_atomic.h"
#include "xrp_thread_impl.h"
#include "xrp_host_impl.h"
#include <stdatomic.h>
#include "xrp_kernel_defs.h"

struct xrp_refcounted {
	atomic_ulong count;
};

enum device_type{
		XRP_DEVICE_NORMAL,
		XRP_DEVICE_NEWMODE,
};

struct xrp_device {
	struct xrp_refcounted ref;
	struct xrp_device_impl impl;
	enum device_type type;
};
enum buf_type{
		XRP_BUFFER_TYPE_HOST,
		XRP_BUFFER_TYPE_DEVICE,
	};
struct xrp_buffer {
	struct xrp_refcounted ref;
	struct xrp_device *device;
	enum buf_type type;
	void *ptr;
	void *apptr;
	size_t size;
	atomic_ulong map_count;
	enum xrp_access_flags map_flags;
	/*struct is zero size , remove for some strange byte offset issue, in some compile option it seems occpuied some bytes*/
	/*struct xrp_buffer_impl impl;*/
	int32_t ion_fd;
	void *ionhandle;
};

struct xrp_buffer_group_record {
	struct xrp_buffer *buffer;
	enum xrp_access_flags access_flags;
};

struct xrp_buffer_group {
	struct xrp_refcounted ref;
	xrp_mutex mutex;
	size_t n_buffers;
	size_t capacity;
	struct xrp_buffer_group_record *buffer;
};

struct xrp_queue {
	struct xrp_refcounted ref;
	struct xrp_device *device;
	int use_nsid;
	int priority;
	char nsid[XRP_NAMESPACE_ID_SIZE];
	struct xrp_queue_impl impl;
};

struct xrp_event {
	struct xrp_refcounted ref;
	struct xrp_queue *queue;
	atomic_int status;
	struct xrp_event_impl impl;
};

/* Helpers */

static inline void set_status(enum xrp_status *status, enum xrp_status v)
{
	if (status)
		*status = v;
}

static inline void *alloc_refcounted(size_t sz)
{
	void *buf = calloc(1, sz);
	struct xrp_refcounted *ref = (struct xrp_refcounted *)buf;

	if (ref) {
//		ref->count = 1;
		atomic_store_explicit(&ref->count , (unsigned long)1 , memory_order_seq_cst);
	}
	return buf;
}

static inline void retain_refcounted(void *buf)
{
	struct xrp_refcounted *ref = (struct xrp_refcounted *)buf;
//	(void)++ref->count;
	atomic_fetch_add_explicit(&ref->count , (unsigned long)1 , memory_order_seq_cst);
}

static inline int last_release_refcounted(void *buf)
{
	struct xrp_refcounted *ref = (struct xrp_refcounted *)buf;
//	return --ref->count == 0;
	return (atomic_fetch_sub_explicit(&ref->count , (unsigned long)1 , memory_order_seq_cst) == 1);
}

static inline int get_ref_count(void *buf)
{
	struct xrp_refcounted *ref = (struct xrp_refcounted *)buf;
	return atomic_load_explicit(&ref->count , memory_order_seq_cst);
}
#endif
