/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <string.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/sys/util.h>

प्रकार व्योम * (sys_heap_allocator_t)(काष्ठा sys_heap *heap, माप_प्रकार align, माप_प्रकार bytes);

अटल व्योम *z_alloc_helper(काष्ठा k_heap *heap, माप_प्रकार align, माप_प्रकार आकार,
			    sys_heap_allocator_t sys_heap_allocator)
{
	व्योम *mem;
	काष्ठा k_heap **heap_ref;
	माप_प्रकार __align;
	k_spinlock_कुंजी_t कुंजी;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	अगर (आकार_add_overflow(आकार, माप(heap_ref), &आकार)) {
		वापस शून्य;
	}
	__align = align | माप(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	कुंजी = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, आकार);
	k_spin_unlock(&heap->lock, कुंजी);

	अगर (mem == शून्य) {
		वापस शून्य;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	वापस mem;
}

व्योम k_free(व्योम *ptr)
{
	काष्ठा k_heap **heap_ref;

	अगर (ptr != शून्य) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		काष्ठा k_heap *heap = *heap_ref;
		k_spinlock_कुंजी_t कुंजी = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#अगर (K_HEAP_MEM_POOL_आकार > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_आकार_FOR(K_HEAP_MEM_POOL_आकार));
#घोषणा _SYSTEM_HEAP (&_system_heap)

#अगर defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#समावेश <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_आकार_FOR(K_HEAP_MEM_POOL_आकार));
#पूर्ण_अगर

व्योम *k_aligned_alloc(माप_प्रकार align, माप_प्रकार आकार)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	व्योम *ret = z_alloc_helper(_SYSTEM_HEAP, align, आकार, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	वापस ret;
}

व्योम *k_malloc(माप_प्रकार आकार)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	व्योम *ret = z_alloc_helper(_SYSTEM_HEAP, 0, आकार, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	वापस ret;
}

व्योम *k_calloc(माप_प्रकार nmemb, माप_प्रकार आकार)
{
	व्योम *ret;
	माप_प्रकार bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	अगर (आकार_mul_overflow(nmemb, आकार, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, शून्य);

		वापस शून्य;
	}

	ret = k_malloc(bounds);
	अगर (ret != शून्य) {
		(व्योम)स_रखो(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	वापस ret;
}

व्योम *k_realloc(व्योम *ptr, माप_प्रकार आकार)
{
	काष्ठा k_heap *heap, **heap_ref;
	k_spinlock_कुंजी_t कुंजी;
	व्योम *ret;

	अगर (आकार == 0) {
		k_free(ptr);
		वापस शून्य;
	}
	अगर (ptr == शून्य) {
		वापस k_malloc(आकार);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	अगर (आकार_add_overflow(आकार, माप(heap_ref), &आकार)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, शून्य);
		वापस शून्य;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	कुंजी = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, आकार);
	k_spin_unlock(&heap->lock, कुंजी);

	अगर (ret != शून्य) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	वापस ret;
}

व्योम k_thread_system_pool_assign(काष्ठा k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#अन्यथा
#घोषणा _SYSTEM_HEAP	शून्य
#पूर्ण_अगर /* K_HEAP_MEM_POOL_SIZE */

अटल व्योम *z_thread_alloc_helper(माप_प्रकार align, माप_प्रकार आकार,
				   sys_heap_allocator_t sys_heap_allocator)
{
	व्योम *ret;
	काष्ठा k_heap *heap;

	अगर (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} अन्यथा {
		heap = _current->resource_pool;
	}

	अगर (heap != शून्य) {
		ret = z_alloc_helper(heap, align, आकार, sys_heap_allocator);
	} अन्यथा {
		ret = शून्य;
	}

	वापस ret;
}

व्योम *z_thread_aligned_alloc(माप_प्रकार align, माप_प्रकार आकार)
{
	वापस z_thread_alloc_helper(align, आकार, sys_heap_aligned_alloc);
}

व्योम *z_thread_malloc(माप_प्रकार आकार)
{
	वापस z_thread_alloc_helper(0, आकार, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
अटल व्योम *स्थिर __used kheap_ref = k_heap_init;
