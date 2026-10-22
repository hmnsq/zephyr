/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <zephyr/sys/math_extras.h>
#অন্তর্ভাৱ <zephyr/sys/util.h>

প্রকারনির্দেশ শূন্য * (sys_heap_allocator_t)(সংরচনা sys_heap *heap, size_t align, size_t bytes);

স্থির শূন্য *z_alloc_helper(সংরচনা k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	শূন্য *mem;
	সংরচনা k_heap **heap_ref;
	size_t __align;
	k_spinlock_চাবি_t চাবি;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	যদি (size_add_overflow(size, পরিমাণ(heap_ref), &size)) {
		ঘূৰাই রিক্ত;
	}
	__align = align | পরিমাণ(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	চাবি = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, চাবি);

	যদি (mem == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	ঘূৰাই mem;
}

শূন্য k_free(শূন্য *ptr)
{
	সংরচনা k_heap **heap_ref;

	যদি (ptr != রিক্ত) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		সংরচনা k_heap *heap = *heap_ref;
		k_spinlock_চাবি_t চাবি = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#যদি (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#যদি defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#অন্তর্ভাৱ <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

শূন্য *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	শূন্য *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	ঘূৰাই ret;
}

শূন্য *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	শূন্য *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	ঘূৰাই ret;
}

শূন্য *k_calloc(size_t nmemb, size_t size)
{
	শূন্য *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	যদি (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, রিক্ত);

		ঘূৰাই রিক্ত;
	}

	ret = k_malloc(bounds);
	যদি (ret != রিক্ত) {
		(শূন্য)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	ঘূৰাই ret;
}

শূন্য *k_realloc(শূন্য *ptr, size_t size)
{
	সংরচনা k_heap *heap, **heap_ref;
	k_spinlock_চাবি_t চাবি;
	শূন্য *ret;

	যদি (size == 0) {
		k_free(ptr);
		ঘূৰাই রিক্ত;
	}
	যদি (ptr == রিক্ত) {
		ঘূৰাই k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	যদি (size_add_overflow(size, পরিমাণ(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, রিক্ত);
		ঘূৰাই রিক্ত;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	চাবি = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, চাবি);

	যদি (ret != রিক্ত) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	ঘূৰাই ret;
}

শূন্য k_thread_system_pool_assign(সংরচনা k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#অন্যথা
#define _SYSTEM_HEAP	রিক্ত
#endif /* K_HEAP_MEM_POOL_SIZE */

স্থির শূন্য *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	শূন্য *ret;
	সংরচনা k_heap *heap;

	যদি (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} অন্যথা {
		heap = _current->resource_pool;
	}

	যদি (heap != রিক্ত) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} অন্যথা {
		ret = রিক্ত;
	}

	ঘূৰাই ret;
}

শূন্য *z_thread_aligned_alloc(size_t align, size_t size)
{
	ঘূৰাই z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

শূন্য *z_thread_malloc(size_t size)
{
	ঘূৰাই z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
স্থির শূন্য *ধ্রুৱ __used kheap_ref = k_heap_init;
