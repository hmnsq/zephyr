/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/util.h>

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ * (sys_heap_allocator_t)(ସଂରଚନା sys_heap *heap, size_t align, size_t bytes);

ସ୍ଥିର ଶୂନ୍ୟ *z_alloc_helper(ସଂରଚନା k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	ଶୂନ୍ୟ *mem;
	ସଂରଚନା k_heap **heap_ref;
	size_t __align;
	k_spinlock_ଚାବି_t ଚାବି;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	ଯଦି (size_add_overflow(size, ଆକାର(heap_ref), &size)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}
	__align = align | ଆକାର(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	ଚାବି = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, ଚାବି);

	ଯଦି (mem == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	ଫେରନ୍ତୁ mem;
}

ଶୂନ୍ୟ k_free(ଶୂନ୍ୟ *ptr)
{
	ସଂରଚନା k_heap **heap_ref;

	ଯଦି (ptr != ଖାଲି) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		ସଂରଚନା k_heap *heap = *heap_ref;
		k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, ଚାବି);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#ଯଦି (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#ଯଦି defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

ଶୂନ୍ୟ *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	ଶୂନ୍ୟ *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	ଶୂନ୍ୟ *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *k_calloc(size_t nmemb, size_t size)
{
	ଶୂନ୍ୟ *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	ଯଦି (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ଖାଲି);

		ଫେରନ୍ତୁ ଖାଲି;
	}

	ret = k_malloc(bounds);
	ଯଦି (ret != ଖାଲି) {
		(ଶୂନ୍ୟ)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *k_realloc(ଶୂନ୍ୟ *ptr, size_t size)
{
	ସଂରଚନା k_heap *heap, **heap_ref;
	k_spinlock_ଚାବି_t ଚାବି;
	ଶୂନ୍ୟ *ret;

	ଯଦି (size == 0) {
		k_free(ptr);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ଯଦି (ptr == ଖାଲି) {
		ଫେରନ୍ତୁ k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	ଯଦି (size_add_overflow(size, ଆକାର(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ଖାଲି);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	ଚାବି = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, ଚାବି);

	ଯଦି (ret != ଖାଲି) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ k_thread_system_pool_assign(ସଂରଚନା k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#ଅନ୍ୟଥା
#define _SYSTEM_HEAP	ଖାଲି
#endif /* K_HEAP_MEM_POOL_SIZE */

ସ୍ଥିର ଶୂନ୍ୟ *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	ଶୂନ୍ୟ *ret;
	ସଂରଚନା k_heap *heap;

	ଯଦି (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} ଅନ୍ୟଥା {
		heap = _current->resource_pool;
	}

	ଯଦି (heap != ଖାଲି) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} ଅନ୍ୟଥା {
		ret = ଖାଲି;
	}

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *z_thread_aligned_alloc(size_t align, size_t size)
{
	ଫେରନ୍ତୁ z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

ଶୂନ୍ୟ *z_thread_malloc(size_t size)
{
	ଫେରନ୍ତୁ z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ସ୍ଥିର ଶୂନ୍ୟ *ଅଚଳ __used kheap_ref = k_heap_init;
