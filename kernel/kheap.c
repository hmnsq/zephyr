/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/linker/linker-defs.h>
#അന്തര്ഭാവ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <wait_q.h>

പൂർണ്ണസംഖ്യ k_heap_array_get(ഘടന k_heap **heap)
{
	പൂർണ്ണസംഖ്യ അക്കം;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_എണ്ണം(k_heap, &അക്കം);
	തിരികെ അക്കം;
}

ശൂന്യം k_heap_init(ഘടന k_heap *heap, ശൂന്യം *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (ഘടന k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ statics_init(ശൂന്യം)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	തിരികെ 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

തരനിർവചനം ശൂന്യം * (sys_heap_allocator_t)(ഘടന sys_heap *heap, size_t align, size_t bytes);

സ്ഥിതം ശൂന്യം *z_heap_alloc_helper(ഘടന k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	ശൂന്യം *ret = ശൂന്യ;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	ആകുമ്പോൾ (ret == ശൂന്യ) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		എങ്കിൽ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ശൂന്യ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			നിർത്തുക;
		}

		എങ്കിൽ (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} അല്ലെങ്കിൽ {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(ശൂന്യം) z_pend_curr(&heap->lock, താക്കോൽ, &heap->wait_q, timeout);
		താക്കോൽ = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, താക്കോൽ);
	തിരികെ ret;
}

ശൂന്യം *k_heap_alloc(ഘടന k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	ശൂന്യം *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	തിരികെ ret;
}

ശൂന്യം *k_heap_aligned_alloc(ഘടന k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	ശൂന്യം *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	തിരികെ ret;
}

ശൂന്യം *k_heap_calloc(ഘടന k_heap *heap, size_t അക്കം, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	ശൂന്യം *ret = ശൂന്യ;
	size_t bounds = 0U;

	എങ്കിൽ (!size_mul_overflow(അക്കം, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	എങ്കിൽ (ret != ശൂന്യ) {
		(ശൂന്യം)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	തിരികെ ret;
}

ശൂന്യം *k_heap_realloc(ഘടന k_heap *heap, ശൂന്യം *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	ശൂന്യം *ret = ശൂന്യ;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ആകുമ്പോൾ (ret == ശൂന്യ) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		എങ്കിൽ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ശൂന്യ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			നിർത്തുക;
		}

		timeout = sys_timepoint_timeout(end);
		(ശൂന്യം) z_pend_curr(&heap->lock, താക്കോൽ, &heap->wait_q, timeout);
		താക്കോൽ = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, താക്കോൽ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	തിരികെ ret;
}

ശൂന്യം k_heap_free(ഘടന k_heap *heap, ശൂന്യം *mem)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, സ്വതന്ത്രം, heap);
	എങ്കിൽ (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&heap->lock, താക്കോൽ);
	}
}
