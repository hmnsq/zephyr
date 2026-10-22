/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/linker/linker-defs.h>
#અન્તર્ભાવ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <wait_q.h>

પૂર્ણાંક k_heap_array_get(સંરચના k_heap **heap)
{
	પૂર્ણાંક અંક;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_ગણતરી(k_heap, &અંક);
	પાછા અંક;
}

શૂન્ય k_heap_init(સંરચના k_heap *heap, શૂન્ય *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (સંરચના k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

સ્થિર પૂર્ણાંક statics_init(શૂન્ય)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	પાછા 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

પ્રકારવ્યાખ્યા શૂન્ય * (sys_heap_allocator_t)(સંરચના sys_heap *heap, size_t align, size_t bytes);

સ્થિર શૂન્ય *z_heap_alloc_helper(સંરચના k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	શૂન્ય *ret = ખાલી;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	જ્યારે (ret == ખાલી) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		જો (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ખાલી) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			તોડો;
		}

		જો (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} નહીંતર {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(શૂન્ય) z_pend_curr(&heap->lock, ચાવી, &heap->wait_q, timeout);
		ચાવી = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, ચાવી);
	પાછા ret;
}

શૂન્ય *k_heap_alloc(સંરચના k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	શૂન્ય *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	પાછા ret;
}

શૂન્ય *k_heap_aligned_alloc(સંરચના k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	શૂન્ય *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	પાછા ret;
}

શૂન્ય *k_heap_calloc(સંરચના k_heap *heap, size_t અંક, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	શૂન્ય *ret = ખાલી;
	size_t bounds = 0U;

	જો (!size_mul_overflow(અંક, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	જો (ret != ખાલી) {
		(શૂન્ય)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	પાછા ret;
}

શૂન્ય *k_heap_realloc(સંરચના k_heap *heap, શૂન્ય *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	શૂન્ય *ret = ખાલી;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	જ્યારે (ret == ખાલી) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		જો (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ખાલી) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			તોડો;
		}

		timeout = sys_timepoint_timeout(end);
		(શૂન્ય) z_pend_curr(&heap->lock, ચાવી, &heap->wait_q, timeout);
		ચાવી = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, ચાવી);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	પાછા ret;
}

શૂન્ય k_heap_free(સંરચના k_heap *heap, શૂન્ય *mem)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, મુક્ત, heap);
	જો (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&heap->lock, ચાવી);
	}
}
