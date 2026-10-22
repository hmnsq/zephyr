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
	पूर्णाङ्क अङ्कः;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_गणनम्(k_heap, &अङ्कः);
	प्रत्यागम अङ्कः;
}

शून्य k_heap_init(संरचना k_heap *heap, शून्य *mem, size_t अष्टकानि)
{
	z_waitq_init(&heap->wait_q);
	heap->तालकम् = (संरचना k_spinlock) {};
	sys_heap_init(&heap->heap, mem, अष्टकानि);

	SYS_पत्तनम्_TRACING_OBJ_INIT(k_heap, heap);
}

स्थिर पूर्णाङ्क statics_init(शून्य)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_अष्टकानि);
	}
	प्रत्यागम 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_प्राथमिकता_OBJECTS);

प्रकारनिर्देश शून्य * (sys_heap_allocator_t)(संरचना sys_heap *heap, size_t align, size_t अष्टकानि);

स्थिर शून्य *z_heap_alloc_helper(संरचना k_heap *heap, size_t align, size_t अष्टकानि,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t अन्तः = sys_timepoint_calc(timeout);
	शून्य *ret = रिक्त;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);

	__ASSERT(!arch_is_अन्तर्_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	यावत् (ret == रिक्त) {
		ret = sys_heap_allocator(&heap->heap, align, अष्टकानि);

		यदि (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != रिक्त) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			भङ्ग;
		}

		यदि (!blocked_alloc) {
			blocked_alloc = true;

			SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} अन्यथा {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(अन्तः);
		(शून्य) z_pend_curr(&heap->तालकम्, कुञ्चिका, &heap->wait_q, timeout);
		कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);
	}

	k_spin_unlock(&heap->तालकम्, कुञ्चिका);
	प्रत्यागम ret;
}

शून्य *k_heap_alloc(संरचना k_heap *heap, size_t अष्टकानि, k_timeout_t timeout)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	शून्य *ret = z_heap_alloc_helper(heap, 0, अष्टकानि, timeout,
					sys_heap_noalign_alloc);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	प्रत्यागम ret;
}

शून्य *k_heap_aligned_alloc(संरचना k_heap *heap, size_t align, size_t अष्टकानि,
			k_timeout_t timeout)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	शून्य *ret = z_heap_alloc_helper(heap, align, अष्टकानि, timeout,
					sys_heap_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_अप्रयुक्तम्(blocked_alloc);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap, aligned_alloc, heap, timeout, ret);

	प्रत्यागम ret;
}

शून्य *k_heap_calloc(संरचना k_heap *heap, size_t अङ्कः, size_t आकार, k_timeout_t timeout)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap, शून्यस्मृतिं_देहि, heap, timeout);

	शून्य *ret = रिक्त;
	size_t bounds = 0U;

	यदि (!आकार_mul_अतिप्रवाहः(अङ्कः, आकार, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	यदि (ret != रिक्त) {
		(शून्य)स्मृतिं_पूरय(ret, 0, bounds);
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap, शून्यस्मृतिं_देहि, heap, timeout, ret);

	प्रत्यागम ret;
}

शून्य *k_heap_realloc(संरचना k_heap *heap, शून्य *ptr, size_t अष्टकानि, k_timeout_t timeout)
{
	k_timepoint_t अन्तः = sys_timepoint_calc(timeout);
	शून्य *ret = रिक्त;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap, realloc, heap, ptr, अष्टकानि, timeout);

	__ASSERT(!arch_is_अन्तर्_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	यावत् (ret == रिक्त) {
		ret = sys_heap_realloc(&heap->heap, ptr, अष्टकानि);

		यदि (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != रिक्त) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			भङ्ग;
		}

		timeout = sys_timepoint_timeout(अन्तः);
		(शून्य) z_pend_curr(&heap->तालकम्, कुञ्चिका, &heap->wait_q, timeout);
		कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);
	}

	k_spin_unlock(&heap->तालकम्, कुञ्चिका);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap, realloc, heap, ptr, अष्टकानि, timeout, ret);

	प्रत्यागम ret;
}

शून्य k_heap_free(संरचना k_heap *heap, शून्य *mem)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);

	sys_heap_free(&heap->heap, mem);

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_heap, मुक्त, heap);
	यदि (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&heap->तालकम्, कुञ्चिका);
	}
}
