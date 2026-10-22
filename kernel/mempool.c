/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/sys/util.h>

typedef void * (sys_heap_allocator_t)(sa_nracanaa sys_heap *heap, size_t align, size_t bytes);

static void *z_alloc_helper(sa_nracanaa k_heap *heap, size_t align, size_t size,
			    sys_heap_allocator_t sys_heap_allocator)
{
	void *mem;
	sa_nracanaa k_heap **heap_ref;
	size_t __align;
	k_spinlock_key_t key;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	ပေႃး (size_add_overflow(size, parimaa_na(heap_ref), &size)) {
		ပွၵ်ႈ NULL;
	}
	__align = align | parimaa_na(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	key = k_spin_lock(&heap->lock);
	mem = sys_heap_allocator(&heap->heap, __align, size);
	k_spin_unlock(&heap->lock, key);

	ပေႃး (mem == NULL) {
		ပွၵ်ႈ NULL;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	ပွၵ်ႈ mem;
}

void k_free(void *ptr)
{
	sa_nracanaa k_heap **heap_ref;

	ပေႃး (ptr != NULL) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		sa_nracanaa k_heap *heap = *heap_ref;
		k_spinlock_key_t key = k_spin_lock(&heap->lock);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#ပေႃး (K_HEAP_MEM_POOL_SIZE > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#define _SYSTEM_HEAP (&_system_heap)

#ပေႃး defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#include <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_SIZE_FOR(K_HEAP_MEM_POOL_SIZE));
#endif

void *k_aligned_alloc(size_t align, size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	void *ret = z_alloc_helper(_SYSTEM_HEAP, align, size, sys_heap_aligned_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	ပွၵ်ႈ ret;
}

void *k_malloc(size_t size)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	void *ret = z_alloc_helper(_SYSTEM_HEAP, 0, size, sys_heap_noalign_alloc);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	ပွၵ်ႈ ret;
}

void *k_calloc(size_t nmemb, size_t size)
{
	void *ret;
	size_t bounds;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	ပေႃး (size_mul_overflow(nmemb, size, &bounds)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, NULL);

		ပွၵ်ႈ NULL;
	}

	ret = k_malloc(bounds);
	ပေႃး (ret != NULL) {
		(void)memset(ret, 0, bounds);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	ပွၵ်ႈ ret;
}

void *k_realloc(void *ptr, size_t size)
{
	sa_nracanaa k_heap *heap, **heap_ref;
	k_spinlock_key_t key;
	void *ret;

	ပေႃး (size == 0) {
		k_free(ptr);
		ပွၵ်ႈ NULL;
	}
	ပေႃး (ptr == NULL) {
		ပွၵ်ႈ k_malloc(size);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	ပေႃး (size_add_overflow(size, parimaa_na(heap_ref), &size)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, NULL);
		ပွၵ်ႈ NULL;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	key = k_spin_lock(&heap->lock);
	ret = sys_heap_realloc(&heap->heap, ptr, size);
	k_spin_unlock(&heap->lock, key);

	ပေႃး (ret != NULL) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	ပွၵ်ႈ ret;
}

void k_thread_system_pool_assign(sa_nracanaa k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#ပေႃးဢမ်ႇ
#define _SYSTEM_HEAP	NULL
#endif /* K_HEAP_MEM_POOL_SIZE */

static void *z_thread_alloc_helper(size_t align, size_t size,
				   sys_heap_allocator_t sys_heap_allocator)
{
	void *ret;
	sa_nracanaa k_heap *heap;

	ပေႃး (k_is_in_isr()) {
		heap = _SYSTEM_HEAP;
	} ပေႃးဢမ်ႇ {
		heap = _current->resource_pool;
	}

	ပေႃး (heap != NULL) {
		ret = z_alloc_helper(heap, align, size, sys_heap_allocator);
	} ပေႃးဢမ်ႇ {
		ret = NULL;
	}

	ပွၵ်ႈ ret;
}

void *z_thread_aligned_alloc(size_t align, size_t size)
{
	ပွၵ်ႈ z_thread_alloc_helper(align, size, sys_heap_aligned_alloc);
}

void *z_thread_malloc(size_t size)
{
	ပွၵ်ႈ z_thread_alloc_helper(0, size, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
static void *const __used kheap_ref = k_heap_init;
