/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/linker/linker-defs.h>
#அந்தர்பாவ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <wait_q.h>

முழு k_heap_array_get(கட்டமைப்பு k_heap **heap)
{
	முழு இலக்கம்;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_மொத்தம்(k_heap, &இலக்கம்);
	திரும்பு இலக்கம்;
}

வெறுமை k_heap_init(கட்டமைப்பு k_heap *heap, வெறுமை *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (கட்டமைப்பு k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

நிலையான முழு statics_init(வெறுமை)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	திரும்பு 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

வகைவரை வெறுமை * (sys_heap_allocator_t)(கட்டமைப்பு sys_heap *heap, size_t align, size_t bytes);

நிலையான வெறுமை *z_heap_alloc_helper(கட்டமைப்பு k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	வெறுமை *ret = வெற்று;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	போது (ret == வெற்று) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		எனில் (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != வெற்று) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			நிறுத்து;
		}

		எனில் (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} இல்லையெனில் {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(வெறுமை) z_pend_curr(&heap->lock, சாவி, &heap->wait_q, timeout);
		சாவி = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, சாவி);
	திரும்பு ret;
}

வெறுமை *k_heap_alloc(கட்டமைப்பு k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	வெறுமை *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	திரும்பு ret;
}

வெறுமை *k_heap_aligned_alloc(கட்டமைப்பு k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	வெறுமை *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	திரும்பு ret;
}

வெறுமை *k_heap_calloc(கட்டமைப்பு k_heap *heap, size_t இலக்கம், size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	வெறுமை *ret = வெற்று;
	size_t bounds = 0U;

	எனில் (!size_mul_overflow(இலக்கம், size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	எனில் (ret != வெற்று) {
		(வெறுமை)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	திரும்பு ret;
}

வெறுமை *k_heap_realloc(கட்டமைப்பு k_heap *heap, வெறுமை *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	வெறுமை *ret = வெற்று;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	போது (ret == வெற்று) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		எனில் (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != வெற்று) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			நிறுத்து;
		}

		timeout = sys_timepoint_timeout(end);
		(வெறுமை) z_pend_curr(&heap->lock, சாவி, &heap->wait_q, timeout);
		சாவி = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, சாவி);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	திரும்பு ret;
}

வெறுமை k_heap_free(கட்டமைப்பு k_heap *heap, வெறுமை *mem)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, விடுவி, heap);
	எனில் (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&heap->lock, சாவி);
	}
}
