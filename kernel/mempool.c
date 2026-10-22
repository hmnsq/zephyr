/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <string.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/util.h>

تعريفنوع فراغ * (sys_heap_allocator_t)(بنية sys_heap *heap, size_t align, size_t bytes);

ساكن فراغ *z_alloc_helper(بنية k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	فراغ *mem;
	بنية k_heap **heap_ref;
	size_t __align;
	k_spinlock_مبودي_t مبودي;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	إذا (size_add_overflow(size, حجم(heap_ref), &size)) {
		أرجع عدم;
	}
	__align = align | حجم(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	مبودي = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, مبودي);

	إذا (mem == عدم) {
		أرجع عدم;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	أرجع mem;
}

فراغ k_free(فراغ *ptr)
{
	بنية k_heap **heap_ref;

	إذا (ptr != عدم) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		بنية k_heap *heap = *heap_ref;
		k_spinlock_مبودي_t مبودي = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, مبودي);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#إذا (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#إذا defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#ضمن <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

فراغ *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	فراغ *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	أرجع ret;
}

فراغ *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	فراغ *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	أرجع ret;
}

فراغ *k_calloc(size_t nmemb, size_t size)
{
	فراغ *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	إذا (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, عدم);

		أرجع عدم;
	}

	ret = k_malloc(bounds);
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	أرجع ret;
}

فراغ *k_realloc(فراغ *ptr, size_t size)
{
	بنية k_heap *heap, **heap_ref;
	k_spinlock_مبودي_t مبودي;
	فراغ *ret;

	إذا (size == 0) {
		k_free(ptr);
		أرجع عدم;
	}
	إذا (ptr == عدم) {
		أرجع k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	إذا (size_add_overflow(size, حجم(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, عدم);
		أرجع عدم;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	مبودي = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, مبودي);

	إذا (ret != عدم) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	أرجع ret;
}

فراغ k_thread_system_pool_assign(بنية k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#وإلا
#define _SYSTEM_HEAP	عدم
#endif /* K_HEAP_MEM_POOL_SIZE */

ساكن فراغ *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	فراغ *ret;
	بنية k_heap *heap;

	إذا (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} وإلا {
		heap = _current->resource_pool;
	}

	إذا (heap != عدم) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} وإلا {
		ret = عدم;
	}

	أرجع ret;
}

فراغ *z_thread_aligned_alloc(size_t align, size_t size)
{
	أرجع z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

فراغ *z_thread_malloc(size_t size)
{
	أرجع z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ساكن فراغ *ثابت __used kheap_ref = k_heap_init;
