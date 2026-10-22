/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/util.h>

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ * (sys_ਢੇਰ_allocator_t)(ਬਣਤਰ sys_ਢੇਰ *ਢੇਰ_2, size_t align, size_t ਬਾਈਟਾਂ);

ਸਥਿਰ ਖਾਲੀ *z_alloc_helper(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, size_t align, size_t ਮਾਪ,
			    sys_ਢੇਰ_allocator_t sys_ਢੇਰ_allocator)
{
	ਖਾਲੀ *mem;
	ਬਣਤਰ k_ਢੇਰ **ਢੇਰ_ref;
	size_t __align;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	ਜੇ (ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ(ਮਾਪ, ਆਕਾਰ(ਢੇਰ_ref), &ਮਾਪ)) {
		ਵਾਪਸ ਨਿਰਾ;
	}
	__align = align | ਆਕਾਰ(ਢੇਰ_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);
	mem = sys_ਢੇਰ_allocator(&ਢੇਰ_2->ਢੇਰ_2, __align, ਮਾਪ);
	k_spin_unlock(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ);

	ਜੇ (mem == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਢੇਰ_ref = mem;
	*ਢੇਰ_ref = ਢੇਰ_2;
	mem = ++ਢੇਰ_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	ਵਾਪਸ mem;
}

ਖਾਲੀ k_free(ਖਾਲੀ *ptr)
{
	ਬਣਤਰ k_ਢੇਰ **ਢੇਰ_ref;

	ਜੇ (ptr != ਨਿਰਾ) {
		ਢੇਰ_ref = ptr;
		--ਢੇਰ_ref;
		ptr = ਢੇਰ_ref;

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_sys, k_free, *ਢੇਰ_ref, ਢੇਰ_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2 = *ਢੇਰ_ref;
		k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);

		sys_ਢੇਰ_free(&ਢੇਰ_2->ਢੇਰ_2, ptr);
		k_spin_unlock(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_sys, k_free, ਢੇਰ_2, ptr);
	}
}

#ਜੇ (K_ਢੇਰ_MEM_POOL_ਮਾਪ > 0)

K_ਢੇਰ_DEFINE_2(system_ਢੇਰ, Z_ਢੇਰ_MIN_ਮਾਪ_FOR(K_ਢੇਰ_MEM_POOL_ਮਾਪ));
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYSTEM_ਢੇਰ (&system_ਢੇਰ)

#ਜੇ defined(CONFIG_SYS_ਢੇਰ_KASAN_SYSTEM)
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/heap_kasan.h>
K_ਢੇਰ_KASAN_ENABLE(system_ਢੇਰ, Z_ਢੇਰ_MIN_ਮਾਪ_FOR(K_ਢੇਰ_MEM_POOL_ਮਾਪ));
#ਜੇ_ਅੰਤ

ਖਾਲੀ *k_aligned_alloc(size_t align, size_t ਮਾਪ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_sys, k_aligned_alloc, SYSTEM_ਢੇਰ);

	ਖਾਲੀ *ret = z_alloc_helper(SYSTEM_ਢੇਰ, align, ਮਾਪ, sys_ਢੇਰ_aligned_alloc);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_sys, k_aligned_alloc, SYSTEM_ਢੇਰ, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ *k_malloc(size_t ਮਾਪ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_sys, k_malloc, SYSTEM_ਢੇਰ);

	ਖਾਲੀ *ret = z_alloc_helper(SYSTEM_ਢੇਰ, 0, ਮਾਪ, sys_ਢੇਰ_noalign_alloc);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_sys, k_malloc, SYSTEM_ਢੇਰ, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ *k_calloc(size_t nmemb, size_t ਮਾਪ)
{
	ਖਾਲੀ *ret;
	size_t bounds;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_sys, k_calloc, SYSTEM_ਢੇਰ);

	ਜੇ (ਮਾਪ_mul_ਉਛਾਲ(nmemb, ਮਾਪ, &bounds)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_sys, k_calloc, SYSTEM_ਢੇਰ, ਨਿਰਾ);

		ਵਾਪਸ ਨਿਰਾ;
	}

	ret = k_malloc(bounds);
	ਜੇ (ret != ਨਿਰਾ) {
		(ਖਾਲੀ)memset(ret, 0, bounds);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_sys, k_calloc, SYSTEM_ਢੇਰ, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ *k_realloc(ਖਾਲੀ *ptr, size_t ਮਾਪ)
{
	ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, **ਢੇਰ_ref;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਖਾਲੀ *ret;

	ਜੇ (ਮਾਪ == 0) {
		k_free(ptr);
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਜੇ (ptr == ਨਿਰਾ) {
		ਵਾਪਸ k_malloc(ਮਾਪ);
	}
	ਢੇਰ_ref = ptr;
	ptr = --ਢੇਰ_ref;
	ਢੇਰ_2 = *ਢੇਰ_ref;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_sys, k_realloc, ਢੇਰ_2, ptr);

	ਜੇ (ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ(ਮਾਪ, ਆਕਾਰ(ਢੇਰ_ref), &ਮਾਪ)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_sys, k_realloc, ਢੇਰ_2, ptr, ਨਿਰਾ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);
	ret = sys_ਢੇਰ_realloc(&ਢੇਰ_2->ਢੇਰ_2, ptr, ਮਾਪ);
	k_spin_unlock(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ);

	ਜੇ (ret != ਨਿਰਾ) {
		ਢੇਰ_ref = ret;
		ret = ++ਢੇਰ_ref;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_sys, k_realloc, ਢੇਰ_2, ptr, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ k_thread_system_pool_ਸੌਂਪੋ(ਬਣਤਰ k_thread *thread)
{
	thread->resource_pool = SYSTEM_ਢੇਰ;
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYSTEM_ਢੇਰ	ਨਿਰਾ
#ਜੇ_ਅੰਤ /* K_HEAP_MEM_POOL_SIZE */

ਸਥਿਰ ਖਾਲੀ *z_thread_alloc_helper(size_t align, size_t ਮਾਪ,
				   sys_ਢੇਰ_allocator_t sys_ਢੇਰ_allocator)
{
	ਖਾਲੀ *ret;
	ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2;

	ਜੇ (k_is_ਵਿੱਚ_isr()) {
		ਢੇਰ_2 = SYSTEM_ਢੇਰ;
	} ਨਹੀਂਤਾਂ {
		ਢੇਰ_2 = _current->resource_pool;
	}

	ਜੇ (ਢੇਰ_2 != ਨਿਰਾ) {
		ret = z_alloc_helper(ਢੇਰ_2, align, ਮਾਪ, sys_ਢੇਰ_allocator);
	} ਨਹੀਂਤਾਂ {
		ret = ਨਿਰਾ;
	}

	ਵਾਪਸ ret;
}

ਖਾਲੀ *z_thread_aligned_alloc(size_t align, size_t ਮਾਪ)
{
	ਵਾਪਸ z_thread_alloc_helper(align, ਮਾਪ, sys_ਢੇਰ_aligned_alloc);
}

ਖਾਲੀ *z_thread_malloc(size_t ਮਾਪ)
{
	ਵਾਪਸ z_thread_alloc_helper(0, ਮਾਪ, sys_ਢੇਰ_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ਸਥਿਰ ਖਾਲੀ *ਅਚਲ __used kheap_ref = k_ਢੇਰ_init;
