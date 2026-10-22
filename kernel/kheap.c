/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/linker/linker-defs.h>
#अन्तर्भाव <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <wait_q.h>

पूर्णाङ्क k_heap_array_get(संरचना k_heap **heap)
{
	पूर्णाङ्क अंक;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_गिनती(k_heap, &अंक);
	प्रत्यागम अंक;
}

शून्य k_heap_init(संरचना k_heap *heap, शून्य *mem, size_t bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (संरचना k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

स्थिर पूर्णाङ्क statics_init(शून्य)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	प्रत्यागम 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

प्रकारनिर्देश शून्य * (sys_heap_allocator_t)(संरचना sys_heap *heap, size_t align, size_t bytes);

स्थिर शून्य *z_heap_alloc_helper(संरचना k_heap *heap, size_t align, size_t bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	शून्य *ret = रिक्त;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	यावत् (ret == रिक्त) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		यदि (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != रिक्त) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			भङ्ग;
		}

		यदि (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} अन्यथा {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(शून्य) z_pend_curr(&heap->lock, कुंजी, &heap->wait_q, timeout);
		कुंजी = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, कुंजी);
	प्रत्यागम ret;
}

शून्य *k_heap_alloc(संरचना k_heap *heap, size_t bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	शून्य *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	प्रत्यागम ret;
}

शून्य *k_heap_aligned_alloc(संरचना k_heap *heap, size_t align, size_t bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	शून्य *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	प्रत्यागम ret;
}

शून्य *k_heap_calloc(संरचना k_heap *heap, size_t अंक, size_t size, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, calloc, heap, timeout);

	शून्य *ret = रिक्त;
	size_t bounds = 0U;

	यदि (!size_mul_overflow(अंक, size, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	यदि (ret != रिक्त) {
		(शून्य)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, calloc, heap, timeout, ret);

	प्रत्यागम ret;
}

शून्य *k_heap_realloc(संरचना k_heap *heap, शून्य *ptr, size_t bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	शून्य *ret = रिक्त;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	यावत् (ret == रिक्त) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		यदि (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != रिक्त) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			भङ्ग;
		}

		timeout = sys_timepoint_timeout(end);
		(शून्य) z_pend_curr(&heap->lock, कुंजी, &heap->wait_q, timeout);
		कुंजी = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, bytes, timeout, ret);

	प्रत्यागम ret;
}

शून्य k_heap_free(संरचना k_heap *heap, शून्य *mem)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, मुक्त, heap);
	यदि (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&heap->lock, कुंजी);
	}
}
