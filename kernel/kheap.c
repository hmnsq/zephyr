/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/linker/linker-defs.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_heap_array_get(ସଂରଚନା k_heap **heap)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_ଗଣନା(k_heap, &ଅଙ୍କ);
	ଫେରନ୍ତୁ ଅଙ୍କ;
}

ଶୂନ୍ୟ k_heap_init(ସଂରଚନା k_heap *heap, ଶୂନ୍ୟ *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (ସଂରଚନା k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା statics_init(ଶୂନ୍ୟ)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	ଫେରନ୍ତୁ 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ * (sys_heap_allocator_t)(ସଂରଚନା sys_heap *heap, size_t align, size_t bytes);

ସ୍ଥିର ଶୂନ୍ୟ *z_heap_alloc_helper(ସଂରଚନା k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	ଶୂନ୍ୟ *ret = ଖାଲି;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	ଯେତେବେଳେ (ret == ଖାଲି) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		ଯଦି (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ଖାଲି) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ଭାଙ୍ଗ;
		}

		ଯଦି (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} ଅନ୍ୟଥା {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(ଶୂନ୍ୟ) z_pend_curr(&heap->lock, ଚାବି, &heap->wait_q, timeout);
		ଚାବି = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, ଚାବି);
	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *k_heap_alloc(ସଂରଚନା k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	ଶୂନ୍ୟ *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *k_heap_aligned_alloc(ସଂରଚନା k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	ଶୂନ୍ୟ *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *k_heap_calloc(ସଂରଚନା k_heap *heap, size_t ଅଙ୍କ, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	ଶୂନ୍ୟ *ret = ଖାଲି;
	size_t bounds = 0U;

	ଯଦି (!size_mul_overflow(ଅଙ୍କ, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	ଯଦି (ret != ଖାଲି) {
		(ଶୂନ୍ୟ)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *k_heap_realloc(ସଂରଚନା k_heap *heap, ଶୂନ୍ୟ *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	ଶୂନ୍ୟ *ret = ଖାଲି;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ଯେତେବେଳେ (ret == ଖାଲି) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		ଯଦି (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ଖାଲି) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ଭାଙ୍ଗ;
		}

		timeout = sys_timepoint_timeout(end);
		(ଶୂନ୍ୟ) z_pend_curr(&heap->lock, ଚାବି, &heap->wait_q, timeout);
		ଚାବି = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ k_heap_free(ସଂରଚନା k_heap *heap, ଶୂନ୍ୟ *mem)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, ମୁକ୍ତ, heap);
	ଯଦି (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&heap->lock, ଚାବି);
	}
}
