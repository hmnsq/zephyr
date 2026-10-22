/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/sys/util.h>

પ્રકારવ્યાખ્યા શૂન્ય * (sys_heap_allocator_t)(સંરચના sys_heap *heap, size_t align, size_t bytes);

સ્થિર શૂન્ય *z_alloc_helper(સંરચના k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	શૂન્ય *mem;
	સંરચના k_heap **heap_ref;
	size_t __align;
	k_spinlock_ચાવી_t ચાવી;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	જો (size_add_overflow(size, કદ(heap_ref), &size)) {
		પાછા ખાલી;
	}
	__align = align | કદ(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	ચાવી = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, ચાવી);

	જો (mem == ખાલી) {
		પાછા ખાલી;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	પાછા mem;
}

શૂન્ય k_free(શૂન્ય *ptr)
{
	સંરચના k_heap **heap_ref;

	જો (ptr != ખાલી) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		સંરચના k_heap *heap = *heap_ref;
		k_spinlock_ચાવી_t ચાવી = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, ચાવી);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#જો (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#જો defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#અન્તર્ભાવ <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

શૂન્ય *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	શૂન્ય *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	પાછા ret;
}

શૂન્ય *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	શૂન્ય *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	પાછા ret;
}

શૂન્ય *k_calloc(size_t nmemb, size_t size)
{
	શૂન્ય *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	જો (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ખાલી);

		પાછા ખાલી;
	}

	ret = k_malloc(bounds);
	જો (ret != ખાલી) {
		(શૂન્ય)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	પાછા ret;
}

શૂન્ય *k_realloc(શૂન્ય *ptr, size_t size)
{
	સંરચના k_heap *heap, **heap_ref;
	k_spinlock_ચાવી_t ચાવી;
	શૂન્ય *ret;

	જો (size == 0) {
		k_free(ptr);
		પાછા ખાલી;
	}
	જો (ptr == ખાલી) {
		પાછા k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	જો (size_add_overflow(size, કદ(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ખાલી);
		પાછા ખાલી;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	ચાવી = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, ચાવી);

	જો (ret != ખાલી) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	પાછા ret;
}

શૂન્ય k_thread_system_pool_assign(સંરચના k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#નહીંતર
#define _SYSTEM_HEAP	ખાલી
#endif /* K_HEAP_MEM_POOL_SIZE */

સ્થિર શૂન્ય *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	શૂન્ય *ret;
	સંરચના k_heap *heap;

	જો (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} નહીંતર {
		heap = _current->resource_pool;
	}

	જો (heap != ખાલી) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} નહીંતર {
		ret = ખાલી;
	}

	પાછા ret;
}

શૂન્ય *z_thread_aligned_alloc(size_t align, size_t size)
{
	પાછા z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

શૂન્ય *z_thread_malloc(size_t size)
{
	પાછા z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
સ્થિર શૂન્ય *અચલ __used kheap_ref = k_heap_init;
