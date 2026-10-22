/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/linker/linker-defs.h>
#समावेश <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#समावेश <ksched.h>
#समावेश <wait_q.h>

पूर्णांक k_heap_array_get(काष्ठा k_heap **heap)
{
	पूर्णांक अंक;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_heap, 0, heap);
	/* Number of statically defined heaps */
	STRUCT_SECTION_गिनती(k_heap, &अंक);
	वापस अंक;
}

व्योम k_heap_init(काष्ठा k_heap *heap, व्योम *mem, माप_प्रकार bytes)
{
	z_waitq_init(&heap->wait_q);
	heap->lock = (काष्ठा k_spinlock) {};
	sys_heap_init(&heap->heap, mem, bytes);

	SYS_PORT_TRACING_OBJ_INIT(k_heap, heap);
}

अटल पूर्णांक statics_init(व्योम)
{
	STRUCT_SECTION_FOREACH(k_heap, heap) {
		k_heap_init(heap, heap->heap.init_mem, heap->heap.init_bytes);
	}
	वापस 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

प्रकार व्योम * (sys_heap_allocator_t)(काष्ठा sys_heap *heap, माप_प्रकार align, माप_प्रकार bytes);

अटल व्योम *z_heap_alloc_helper(काष्ठा k_heap *heap, माप_प्रकार align, माप_प्रकार bytes,
				 k_timeout_t timeout,
				 sys_heap_allocator_t *sys_heap_allocator)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	व्योम *ret = शून्य;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&heap->lock);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	जबतक (ret == शून्य) {
		ret = sys_heap_allocator(&heap->heap, align, bytes);

		अगर (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != शून्य) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			अवरोध;
		}

		अगर (!blocked_alloc) {
			blocked_alloc = true;

			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_heap, alloc_helper, heap, timeout);
		} अन्यथा {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(end);
		(व्योम) z_pend_curr(&heap->lock, कुंजी, &heap->wait_q, timeout);
		कुंजी = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, कुंजी);
	वापस ret;
}

व्योम *k_heap_alloc(काष्ठा k_heap *heap, माप_प्रकार bytes, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, alloc, heap, timeout);

	व्योम *ret = z_heap_alloc_helper(heap, 0, bytes, timeout,
					sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, alloc, heap, timeout, ret);

	वापस ret;
}

व्योम *k_heap_aligned_alloc(काष्ठा k_heap *heap, माप_प्रकार align, माप_प्रकार bytes,
			k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, aligned_alloc, heap, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	व्योम *ret = z_heap_alloc_helper(heap, align, bytes, timeout,
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

	वापस ret;
}

व्योम *k_heap_calloc(काष्ठा k_heap *heap, माप_प्रकार अंक, माप_प्रकार आकार, k_timeout_t timeout)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, सुस्मृति, heap, timeout);

	व्योम *ret = शून्य;
	माप_प्रकार bounds = 0U;

	अगर (!आकार_mul_overflow(अंक, आकार, &bounds)) {
		ret = k_heap_alloc(heap, bounds, timeout);
	}
	अगर (ret != शून्य) {
		(व्योम)स_रखो(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, सुस्मृति, heap, timeout, ret);

	वापस ret;
}

व्योम *k_heap_realloc(काष्ठा k_heap *heap, व्योम *ptr, माप_प्रकार bytes, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	व्योम *ret = शून्य;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&heap->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap, पुनः_स्मृति, heap, ptr, bytes, timeout);

	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	जबतक (ret == शून्य) {
		ret = sys_heap_realloc(&heap->heap, ptr, bytes);

		अगर (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != शून्य) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			अवरोध;
		}

		timeout = sys_timepoint_timeout(end);
		(व्योम) z_pend_curr(&heap->lock, कुंजी, &heap->wait_q, timeout);
		कुंजी = k_spin_lock(&heap->lock);
	}

	k_spin_unlock(&heap->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap, पुनः_स्मृति, heap, ptr, bytes, timeout, ret);

	वापस ret;
}

व्योम k_heap_free(काष्ठा k_heap *heap, व्योम *mem)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&heap->lock);

	sys_heap_free(&heap->heap, mem);

	SYS_PORT_TRACING_OBJ_FUNC(k_heap, मुक्त, heap);
	अगर (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&heap->wait_q) != 0)) {
		z_reschedule(&heap->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&heap->lock, कुंजी);
	}
}
