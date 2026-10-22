/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <string.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/util.h>

تعريفنوع فراغ * (sys_ڍير_allocator_t)(بنية sys_ڍير *ڍير_2, size_t align, size_t بائيٽون);

ساكن فراغ *z_alloc_helper(بنية k_ڍير *ڍير_2, size_t align, size_t سائيز,
			    sys_ڍير_allocator_t sys_ڍير_allocator)
{
	فراغ *mem;
	بنية k_ڍير **ڍير_ref;
	size_t __align;
	k_spinlock_ڪنجي_t ڪنجي;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	إذا (سائيز_جوڙيو_حد_کان_مٿي(سائيز, حجم(ڍير_ref), &سائيز)) {
		أرجع عدم;
	}
	__align = align | حجم(ڍير_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	ڪنجي = k_spin_تالو(&ڍير_2->تالو);
	mem = sys_ڍير_allocator(&ڍير_2->ڍير_2, __align, سائيز);
	k_spin_unlock(&ڍير_2->تالو, ڪنجي);

	إذا (mem == عدم) {
		أرجع عدم;
	}

	ڍير_ref = mem;
	*ڍير_ref = ڍير_2;
	mem = ++ڍير_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	أرجع mem;
}

فراغ k_free(فراغ *ptr)
{
	بنية k_ڍير **ڍير_ref;

	إذا (ptr != عدم) {
		ڍير_ref = ptr;
		--ڍير_ref;
		ptr = ڍير_ref;

		SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_sys, k_free, *ڍير_ref, ڍير_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		بنية k_ڍير *ڍير_2 = *ڍير_ref;
		k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ڍير_2->تالو);

		sys_ڍير_free(&ڍير_2->ڍير_2, ptr);
		k_spin_unlock(&ڍير_2->تالو, ڪنجي);

		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_sys, k_free, ڍير_2, ptr);
	}
}

#إذا (K_ڍير_MEM_POOL_سائيز > 0)

K_ڍير_DEFINE_2(system_ڍير, Z_ڍير_MIN_سائيز_FOR(K_ڍير_MEM_POOL_سائيز));
#وصف_ڪريو SYSTEM_ڍير (&system_ڍير)

#إذا defined(CONFIG_SYS_ڍير_KASAN_SYSTEM)
#ضمن <zephyr/sys/heap_kasan.h>
K_ڍير_KASAN_ENABLE(system_ڍير, Z_ڍير_MIN_سائيز_FOR(K_ڍير_MEM_POOL_سائيز));
#جيڪڏهن_پورو

فراغ *k_aligned_alloc(size_t align, size_t سائيز)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_sys, k_aligned_alloc, SYSTEM_ڍير);

	فراغ *ret = z_alloc_helper(SYSTEM_ڍير, align, سائيز, sys_ڍير_aligned_alloc);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_sys, k_aligned_alloc, SYSTEM_ڍير, ret);

	أرجع ret;
}

فراغ *k_malloc(size_t سائيز)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_sys, k_malloc, SYSTEM_ڍير);

	فراغ *ret = z_alloc_helper(SYSTEM_ڍير, 0, سائيز, sys_ڍير_noalign_alloc);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_sys, k_malloc, SYSTEM_ڍير, ret);

	أرجع ret;
}

فراغ *k_calloc(size_t nmemb, size_t سائيز)
{
	فراغ *ret;
	size_t bounds;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_sys, k_calloc, SYSTEM_ڍير);

	إذا (سائيز_mul_حد_کان_مٿي(nmemb, سائيز, &bounds)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_sys, k_calloc, SYSTEM_ڍير, عدم);

		أرجع عدم;
	}

	ret = k_malloc(bounds);
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_sys, k_calloc, SYSTEM_ڍير, ret);

	أرجع ret;
}

فراغ *k_realloc(فراغ *ptr, size_t سائيز)
{
	بنية k_ڍير *ڍير_2, **ڍير_ref;
	k_spinlock_ڪنجي_t ڪنجي;
	فراغ *ret;

	إذا (سائيز == 0) {
		k_free(ptr);
		أرجع عدم;
	}
	إذا (ptr == عدم) {
		أرجع k_malloc(سائيز);
	}
	ڍير_ref = ptr;
	ptr = --ڍير_ref;
	ڍير_2 = *ڍير_ref;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_sys, k_realloc, ڍير_2, ptr);

	إذا (سائيز_جوڙيو_حد_کان_مٿي(سائيز, حجم(ڍير_ref), &سائيز)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_sys, k_realloc, ڍير_2, ptr, عدم);
		أرجع عدم;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	ڪنجي = k_spin_تالو(&ڍير_2->تالو);
	ret = sys_ڍير_realloc(&ڍير_2->ڍير_2, ptr, سائيز);
	k_spin_unlock(&ڍير_2->تالو, ڪنجي);

	إذا (ret != عدم) {
		ڍير_ref = ret;
		ret = ++ڍير_ref;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_sys, k_realloc, ڍير_2, ptr, ret);

	أرجع ret;
}

فراغ k_thread_system_pool_سونپيو(بنية k_thread *thread)
{
	thread->resource_pool = SYSTEM_ڍير;
}
#وإلا
#وصف_ڪريو SYSTEM_ڍير	عدم
#جيڪڏهن_پورو /* K_HEAP_MEM_POOL_SIZE */

ساكن فراغ *z_thread_alloc_helper(size_t align, size_t سائيز,
				   sys_ڍير_allocator_t sys_ڍير_allocator)
{
	فراغ *ret;
	بنية k_ڍير *ڍير_2;

	إذا (k_is_اندريون_isr()) {
		ڍير_2 = SYSTEM_ڍير;
	} وإلا {
		ڍير_2 = _current->resource_pool;
	}

	إذا (ڍير_2 != عدم) {
		ret = z_alloc_helper(ڍير_2, align, سائيز, sys_ڍير_allocator);
	} وإلا {
		ret = عدم;
	}

	أرجع ret;
}

فراغ *z_thread_aligned_alloc(size_t align, size_t سائيز)
{
	أرجع z_thread_alloc_helper(align, سائيز, sys_ڍير_aligned_alloc);
}

فراغ *z_thread_malloc(size_t سائيز)
{
	أرجع z_thread_alloc_helper(0, سائيز, sys_ڍير_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ساكن فراغ *ثابت __used kheap_ref = k_ڍير_init;
