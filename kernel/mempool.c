/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/sys/util.h>

തരനിർവചനം ശൂന്യം * (sys_heap_allocator_t)(ഘടന sys_heap *heap, size_t align, size_t bytes);

സ്ഥിതം ശൂന്യം *z_alloc_helper(ഘടന k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	ശൂന്യം *mem;
	ഘടന k_heap **heap_ref;
	size_t __align;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	എങ്കിൽ (size_add_overflow(size, വലുപ്പം(heap_ref), &size)) {
		തിരികെ ശൂന്യ;
	}
	__align = align | വലുപ്പം(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	താക്കോൽ = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, താക്കോൽ);

	എങ്കിൽ (mem == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	തിരികെ mem;
}

ശൂന്യം k_free(ശൂന്യം *ptr)
{
	ഘടന k_heap **heap_ref;

	എങ്കിൽ (ptr != ശൂന്യ) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		ഘടന k_heap *heap = *heap_ref;
		k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, താക്കോൽ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#എങ്കിൽ (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#എങ്കിൽ defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#അന്തര്ഭാവ <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

ശൂന്യം *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	ശൂന്യം *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	തിരികെ ret;
}

ശൂന്യം *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	ശൂന്യം *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	തിരികെ ret;
}

ശൂന്യം *k_calloc(size_t nmemb, size_t size)
{
	ശൂന്യം *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	എങ്കിൽ (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ശൂന്യ);

		തിരികെ ശൂന്യ;
	}

	ret = k_malloc(bounds);
	എങ്കിൽ (ret != ശൂന്യ) {
		(ശൂന്യം)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	തിരികെ ret;
}

ശൂന്യം *k_realloc(ശൂന്യം *ptr, size_t size)
{
	ഘടന k_heap *heap, **heap_ref;
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	ശൂന്യം *ret;

	എങ്കിൽ (size == 0) {
		k_free(ptr);
		തിരികെ ശൂന്യ;
	}
	എങ്കിൽ (ptr == ശൂന്യ) {
		തിരികെ k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	എങ്കിൽ (size_add_overflow(size, വലുപ്പം(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ശൂന്യ);
		തിരികെ ശൂന്യ;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	താക്കോൽ = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, താക്കോൽ);

	എങ്കിൽ (ret != ശൂന്യ) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	തിരികെ ret;
}

ശൂന്യം k_thread_system_pool_assign(ഘടന k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#അല്ലെങ്കിൽ
#define _SYSTEM_HEAP	ശൂന്യ
#endif /* K_HEAP_MEM_POOL_SIZE */

സ്ഥിതം ശൂന്യം *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	ശൂന്യം *ret;
	ഘടന k_heap *heap;

	എങ്കിൽ (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} അല്ലെങ്കിൽ {
		heap = _current->resource_pool;
	}

	എങ്കിൽ (heap != ശൂന്യ) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} അല്ലെങ്കിൽ {
		ret = ശൂന്യ;
	}

	തിരികെ ret;
}

ശൂന്യം *z_thread_aligned_alloc(size_t align, size_t size)
{
	തിരികെ z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

ശൂന്യം *z_thread_malloc(size_t size)
{
	തിരികെ z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
സ്ഥിതം ശൂന്യം *സ്ഥിരം __used kheap_ref = k_heap_init;
