/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/init.h>
#অন্তর্ভাৱ <zephyr/linker/linker-defs.h>
#অন্তর্ভাৱ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <wait_q.h>

পূর্ণাঙ্ক k_heap_array_get(সংরচনা k_heap **heap)
{
	পূর্ণাঙ্ক অঙ্ক;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_গুনতি(k_heap, &অঙ্ক);
	ঘূৰাই অঙ্ক;
}

শূন্য k_heap_init(সংরচনা k_heap *heap, শূন্য *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (সংরচনা k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

স্থির পূর্ণাঙ্ক statics_init(শূন্য)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	ঘূৰাই 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

প্রকারনির্দেশ শূন্য * (sys_heap_allocator_t)(সংরচনা sys_heap *heap, size_t align, size_t bytes);

স্থির শূন্য *z_heap_alloc_helper(সংরচনা k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	শূন্য *ret = রিক্ত;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	যেতিয়ালৈ (ret == রিক্ত) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		যদি (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != রিক্ত) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ভঙ্গ;
		}

		যদি (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} অন্যথা {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(শূন্য) z_pend_curr(&heap->lock, চাবি, &heap->wait_q, timeout);
		চাবি = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, চাবি);
	ঘূৰাই ret;
}

শূন্য *k_heap_alloc(সংরচনা k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	শূন্য *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	ঘূৰাই ret;
}

শূন্য *k_heap_aligned_alloc(সংরচনা k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	শূন্য *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	ঘূৰাই ret;
}

শূন্য *k_heap_calloc(সংরচনা k_heap *heap, size_t অঙ্ক, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	শূন্য *ret = রিক্ত;
	size_t bounds = 0U;

	যদি (!size_mul_overflow(অঙ্ক, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	যদি (ret != রিক্ত) {
		(শূন্য)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	ঘূৰাই ret;
}

শূন্য *k_heap_realloc(সংরচনা k_heap *heap, শূন্য *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	শূন্য *ret = রিক্ত;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	যেতিয়ালৈ (ret == রিক্ত) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		যদি (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != রিক্ত) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ভঙ্গ;
		}

		timeout = sys_timepoint_timeout(end);
		(শূন্য) z_pend_curr(&heap->lock, চাবি, &heap->wait_q, timeout);
		চাবি = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	ঘূৰাই ret;
}

শূন্য k_heap_free(সংরচনা k_heap *heap, শূন্য *mem)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, মুক্ত, heap);
	যদি (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&heap->lock, চাবি);
	}
}
