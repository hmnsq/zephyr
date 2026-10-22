/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <string.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/util.h>

تعریفنوع تهی * (sys_heap_allocator_t)(ساختار sys_heap *heap, size_t align, size_t bytes);

ایستا تهی *z_alloc_helper(ساختار k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	تهی *mem;
	ساختار k_heap **heap_ref;
	size_t __align;
	k_spinlock_کلید_t کلید;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	اگر (size_add_overflow(size, اندازه(heap_ref), &size)) {
		بازگشت پوچ;
	}
	__align = align | اندازه(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	کلید = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, کلید);

	اگر (mem == پوچ) {
		بازگشت پوچ;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	بازگشت mem;
}

تهی k_free(تهی *ptr)
{
	ساختار k_heap **heap_ref;

	اگر (ptr != پوچ) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		ساختار k_heap *heap = *heap_ref;
		k_spinlock_کلید_t کلید = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#اگر (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#اگر defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#شامل <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

تهی *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	تهی *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	بازگشت ret;
}

تهی *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	تهی *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	بازگشت ret;
}

تهی *k_calloc(size_t nmemb, size_t size)
{
	تهی *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	اگر (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, پوچ);

		بازگشت پوچ;
	}

	ret = k_malloc(bounds);
	اگر (ret != پوچ) {
		(تهی)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	بازگشت ret;
}

تهی *k_realloc(تهی *ptr, size_t size)
{
	ساختار k_heap *heap, **heap_ref;
	k_spinlock_کلید_t کلید;
	تهی *ret;

	اگر (size == 0) {
		k_free(ptr);
		بازگشت پوچ;
	}
	اگر (ptr == پوچ) {
		بازگشت k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	اگر (size_add_overflow(size, اندازه(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, پوچ);
		بازگشت پوچ;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	کلید = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, کلید);

	اگر (ret != پوچ) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	بازگشت ret;
}

تهی k_thread_system_pool_assign(ساختار k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#وگرنه
#define _SYSTEM_HEAP	پوچ
#endif /* K_HEAP_MEM_POOL_SIZE */

ایستا تهی *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	تهی *ret;
	ساختار k_heap *heap;

	اگر (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} وگرنه {
		heap = _current->resource_pool;
	}

	اگر (heap != پوچ) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} وگرنه {
		ret = پوچ;
	}

	بازگشت ret;
}

تهی *z_thread_aligned_alloc(size_t align, size_t size)
{
	بازگشت z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

تهی *z_thread_malloc(size_t size)
{
	بازگشت z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ایستا تهی *ثابت __used kheap_ref = k_heap_init;
