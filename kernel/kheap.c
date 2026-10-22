/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#ضمن <ksched.h>
#ضمن <wait_q.h>

صحيح k_heap_array_get(بنية k_heap **heap)
{
	صحيح رقم;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_تعداد(k_heap, &رقم);
	إرجاع رقم;
}

فراغ k_heap_init(بنية k_heap *heap, فراغ *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (بنية k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

ساكن صحيح statics_init(فراغ)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	إرجاع 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

تعريفنوع فراغ * (sys_heap_allocator_t)(بنية sys_heap *heap, size_t align, size_t bytes);

ساكن فراغ *z_heap_alloc_helper(بنية k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_مفتاح_t مفتاح = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	بينما (ret == عدم) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		إذا (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} وإلا {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(فراغ) z_pend_curr(&heap->lock, مفتاح, &heap->wait_q, timeout);
		مفتاح = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, مفتاح);
	إرجاع ret;
}

فراغ *k_heap_alloc(بنية k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	فراغ *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	إرجاع ret;
}

فراغ *k_heap_aligned_alloc(بنية k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	فراغ *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
					sys_heap_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_UNUSED(blocked_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, aligned_alloc, heap, timeout, ret);

	إرجاع ret;
}

فراغ *k_heap_calloc(بنية k_heap *heap, size_t رقم, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	فراغ *ret = عدم;
	size_t bounds = 0U;

	إذا (!size_mul_overflow(رقم, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	إرجاع ret;
}

فراغ *k_heap_realloc(بنية k_heap *heap, فراغ *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_مفتاح_t مفتاح = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	بينما (ret == عدم) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		timeout = sys_timepoint_timeout(end);
		(فراغ) z_pend_curr(&heap->lock, مفتاح, &heap->wait_q, timeout);
		مفتاح = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, مفتاح);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	إرجاع ret;
}

فراغ k_heap_free(بنية k_heap *heap, فراغ *mem)
{
	k_spinlock_مفتاح_t مفتاح = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, حرر, heap);
	إذا (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, مفتاح);
	} وإلا {
		k_spin_unlock(&heap->lock, مفتاح);
	}
}
