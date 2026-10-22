/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/init.h>
#شامل <zephyr/linker/linker-defs.h>
#شامل <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#شامل <ksched.h>
#شامل <wait_q.h>

صحیح k_heap_array_get(ساختار k_heap **heap)
{
	صحیح رقم;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_شمار(k_heap, &رقم);
	بازگشت رقم;
}

تهی k_heap_init(ساختار k_heap *heap, تهی *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (ساختار k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

ایستا صحیح statics_init(تهی)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	بازگشت 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

تعریفنوع تهی * (sys_heap_allocator_t)(ساختار sys_heap *heap, size_t align, size_t bytes);

ایستا تهی *z_heap_alloc_helper(ساختار k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	تهی *ret = پوچ;

	k_spinlock_کلید_t کلید = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	وقتی (ret == پوچ) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		اگر (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != پوچ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			بشکن;
		}

		اگر (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} وگرنه {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(تهی) z_pend_curr(&heap->lock, کلید, &heap->wait_q, timeout);
		کلید = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, کلید);
	بازگشت ret;
}

تهی *k_heap_alloc(ساختار k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	تهی *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	بازگشت ret;
}

تهی *k_heap_aligned_alloc(ساختار k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	تهی *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	بازگشت ret;
}

تهی *k_heap_calloc(ساختار k_heap *heap, size_t رقم, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	تهی *ret = پوچ;
	size_t bounds = 0U;

	اگر (!size_mul_overflow(رقم, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	اگر (ret != پوچ) {
		(تهی)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	بازگشت ret;
}

تهی *k_heap_realloc(ساختار k_heap *heap, تهی *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	تهی *ret = پوچ;

	k_spinlock_کلید_t کلید = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	وقتی (ret == پوچ) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		اگر (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != پوچ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			بشکن;
		}

		timeout = sys_timepoint_timeout(end);
		(تهی) z_pend_curr(&heap->lock, کلید, &heap->wait_q, timeout);
		کلید = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, کلید);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	بازگشت ret;
}

تهی k_heap_free(ساختار k_heap *heap, تهی *mem)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, آزاد, heap);
	اگر (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, کلید);
	} وگرنه {
		k_spin_unlock(&heap->lock, کلید);
	}
}
