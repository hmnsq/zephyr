/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/util.h>

ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ * (sys_heap_allocator_t)(ಸಂರಚನಾ sys_heap *heap, size_t align, size_t bytes);

ಸ್ಥಿರ ಶೂನ್ಯ *z_alloc_helper(ಸಂರಚನಾ k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	ಶೂನ್ಯ *mem;
	ಸಂರಚನಾ k_heap **heap_ref;
	size_t __align;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	ಯದಿ (size_add_overflow(size, ಪರಿಮಾಣ(heap_ref), &size)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	__align = align | ಪರಿಮಾಣ(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	ಕೀಲಿ = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, ಕೀಲಿ);

	ಯದಿ (mem == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	ಪ್ರತ್ಯಾಗಮ mem;
}

ಶೂನ್ಯ k_free(ಶೂನ್ಯ *ptr)
{
	ಸಂರಚನಾ k_heap **heap_ref;

	ಯದಿ (ptr != ರಿಕ್ತ) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		ಸಂರಚನಾ k_heap *heap = *heap_ref;
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, ಕೀಲಿ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#ಯದಿ (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#ಯದಿ defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#ಅನ್ತರ್ಭಾವ <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

ಶೂನ್ಯ *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	ಶೂನ್ಯ *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	ಶೂನ್ಯ *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *k_calloc(size_t nmemb, size_t size)
{
	ಶೂನ್ಯ *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	ಯದಿ (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ರಿಕ್ತ);

		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ret = k_malloc(bounds);
	ಯದಿ (ret != ರಿಕ್ತ) {
		(ಶೂನ್ಯ)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *k_realloc(ಶೂನ್ಯ *ptr, size_t size)
{
	ಸಂರಚನಾ k_heap *heap, **heap_ref;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಶೂನ್ಯ *ret;

	ಯದಿ (size == 0) {
		k_free(ptr);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ಯದಿ (ptr == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	ಯದಿ (size_add_overflow(size, ಪರಿಮಾಣ(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	ಕೀಲಿ = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, ಕೀಲಿ);

	ಯದಿ (ret != ರಿಕ್ತ) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ k_thread_system_pool_assign(ಸಂರಚನಾ k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#ಅನ್ಯಥಾ
#define _SYSTEM_HEAP	ರಿಕ್ತ
#endif /* K_HEAP_MEM_POOL_SIZE */

ಸ್ಥಿರ ಶೂನ್ಯ *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	ಶೂನ್ಯ *ret;
	ಸಂರಚನಾ k_heap *heap;

	ಯದಿ (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} ಅನ್ಯಥಾ {
		heap = _current->resource_pool;
	}

	ಯದಿ (heap != ರಿಕ್ತ) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} ಅನ್ಯಥಾ {
		ret = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *z_thread_aligned_alloc(size_t align, size_t size)
{
	ಪ್ರತ್ಯಾಗಮ z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

ಶೂನ್ಯ *z_thread_malloc(size_t size)
{
	ಪ್ರತ್ಯಾಗಮ z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ಸ್ಥಿರ ಶೂನ್ಯ *ಧ್ರುವ __used kheap_ref = k_heap_init;
