/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/linker-defs.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>

ಪೂರ್ಣಾಙ್ಕ k_heap_array_get(ಸಂರಚನಾ k_heap **heap)
{
	ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_ಎಣಿಕೆ(k_heap, &ಅಂಕೆ);
	ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
}

ಶೂನ್ಯ k_heap_init(ಸಂರಚನಾ k_heap *heap, ಶೂನ್ಯ *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (ಸಂರಚನಾ k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ statics_init(ಶೂನ್ಯ)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ * (sys_heap_allocator_t)(ಸಂರಚನಾ sys_heap *heap, size_t align, size_t bytes);

ಸ್ಥಿರ ಶೂನ್ಯ *z_heap_alloc_helper(ಸಂರಚನಾ k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	ಶೂನ್ಯ *ret = ರಿಕ್ತ;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	ಯಾವತ್ (ret == ರಿಕ್ತ) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		ಯದಿ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ರಿಕ್ತ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ಭಙ್ಗ;
		}

		ಯದಿ (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} ಅನ್ಯಥಾ {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(ಶೂನ್ಯ) z_pend_curr(&heap->lock, ಕೀಲಿ, &heap->wait_q, timeout);
		ಕೀಲಿ = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, ಕೀಲಿ);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *k_heap_alloc(ಸಂರಚನಾ k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	ಶೂನ್ಯ *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *k_heap_aligned_alloc(ಸಂರಚನಾ k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	ಶೂನ್ಯ *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *k_heap_calloc(ಸಂರಚನಾ k_heap *heap, size_t ಅಂಕೆ, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	ಶೂನ್ಯ *ret = ರಿಕ್ತ;
	size_t bounds = 0U;

	ಯದಿ (!size_mul_overflow(ಅಂಕೆ, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	ಯದಿ (ret != ರಿಕ್ತ) {
		(ಶೂನ್ಯ)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *k_heap_realloc(ಸಂರಚನಾ k_heap *heap, ಶೂನ್ಯ *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	ಶೂನ್ಯ *ret = ರಿಕ್ತ;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ಯಾವತ್ (ret == ರಿಕ್ತ) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		ಯದಿ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ರಿಕ್ತ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ಭಙ್ಗ;
		}

		timeout = sys_timepoint_timeout(end);
		(ಶೂನ್ಯ) z_pend_curr(&heap->lock, ಕೀಲಿ, &heap->wait_q, timeout);
		ಕೀಲಿ = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, ಕೀಲಿ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ k_heap_free(ಸಂರಚನಾ k_heap *heap, ಶೂನ್ಯ *mem)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, ಮುಕ್ತ, heap);
	ಯದಿ (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&heap->lock, ಕೀಲಿ);
	}
}
