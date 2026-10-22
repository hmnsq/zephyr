/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/sys/util.h>

வகைவரை வெறுமை * (sys_heap_allocator_t)(கட்டமைப்பு sys_heap *heap, size_t align, size_t bytes);

நிலையான வெறுமை *z_alloc_helper(கட்டமைப்பு k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	வெறுமை *mem;
	கட்டமைப்பு k_heap **heap_ref;
	size_t __align;
	k_spinlock_சாவி_t சாவி;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	எனில் (size_add_overflow(size, அளவு(heap_ref), &size)) {
		திரும்பு வெற்று;
	}
	__align = align | அளவு(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	சாவி = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, சாவி);

	எனில் (mem == வெற்று) {
		திரும்பு வெற்று;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	திரும்பு mem;
}

வெறுமை k_free(வெறுமை *ptr)
{
	கட்டமைப்பு k_heap **heap_ref;

	எனில் (ptr != வெற்று) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		கட்டமைப்பு k_heap *heap = *heap_ref;
		k_spinlock_சாவி_t சாவி = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, சாவி);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#எனில் (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#எனில் defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#அந்தர்பாவ <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

வெறுமை *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	வெறுமை *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	திரும்பு ret;
}

வெறுமை *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	வெறுமை *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	திரும்பு ret;
}

வெறுமை *k_calloc(size_t nmemb, size_t size)
{
	வெறுமை *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	எனில் (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, வெற்று);

		திரும்பு வெற்று;
	}

	ret = k_malloc(bounds);
	எனில் (ret != வெற்று) {
		(வெறுமை)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	திரும்பு ret;
}

வெறுமை *k_realloc(வெறுமை *ptr, size_t size)
{
	கட்டமைப்பு k_heap *heap, **heap_ref;
	k_spinlock_சாவி_t சாவி;
	வெறுமை *ret;

	எனில் (size == 0) {
		k_free(ptr);
		திரும்பு வெற்று;
	}
	எனில் (ptr == வெற்று) {
		திரும்பு k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	எனில் (size_add_overflow(size, அளவு(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, வெற்று);
		திரும்பு வெற்று;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	சாவி = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, சாவி);

	எனில் (ret != வெற்று) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	திரும்பு ret;
}

வெறுமை k_thread_system_pool_assign(கட்டமைப்பு k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#இல்லையெனில்
#define _SYSTEM_HEAP	வெற்று
#endif /* K_HEAP_MEM_POOL_SIZE */

நிலையான வெறுமை *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	வெறுமை *ret;
	கட்டமைப்பு k_heap *heap;

	எனில் (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} இல்லையெனில் {
		heap = _current->resource_pool;
	}

	எனில் (heap != வெற்று) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} இல்லையெனில் {
		ret = வெற்று;
	}

	திரும்பு ret;
}

வெறுமை *z_thread_aligned_alloc(size_t align, size_t size)
{
	திரும்பு z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

வெறுமை *z_thread_malloc(size_t size)
{
	திரும்பு z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
நிலையான வெறுமை *மாறிலி __used kheap_ref = k_heap_init;
