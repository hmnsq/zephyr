/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <string.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/util.h>

تعريفنوع فراغ * (sys_ډېرۍ_allocator_t)(بنية sys_ډېرۍ *ډېرۍ, size_t align, size_t بایټونه);

ساكن فراغ *z_alloc_helper(بنية k_ډېرۍ *ډېرۍ, size_t align, size_t اندازه,
			    sys_ډېرۍ_allocator_t sys_ډېرۍ_allocator)
{
	فراغ *mem;
	بنية k_ډېرۍ **ډېرۍ_ref;
	size_t __align;
	k_spinlock_کلی_t کلی;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	إذا (اندازه_جمع_کړه_ډکېدنه(اندازه, حجم(ډېرۍ_ref), &اندازه)) {
		أرجع عدم;
	}
	__align = align | حجم(ډېرۍ_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	کلی = k_spin_قلف(&ډېرۍ->قلف);
	mem = sys_ډېرۍ_allocator(&ډېرۍ->ډېرۍ, __align, اندازه);
	k_spin_unlock(&ډېرۍ->قلف, کلی);

	إذا (mem == عدم) {
		أرجع عدم;
	}

	ډېرۍ_ref = mem;
	*ډېرۍ_ref = ډېرۍ;
	mem = ++ډېرۍ_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	أرجع mem;
}

فراغ k_free(فراغ *ptr)
{
	بنية k_ډېرۍ **ډېرۍ_ref;

	إذا (ptr != عدم) {
		ډېرۍ_ref = ptr;
		--ډېرۍ_ref;
		ptr = ډېرۍ_ref;

		SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ_sys, k_free, *ډېرۍ_ref, ډېرۍ_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		بنية k_ډېرۍ *ډېرۍ = *ډېرۍ_ref;
		k_spinlock_کلی_t کلی = k_spin_قلف(&ډېرۍ->قلف);

		sys_ډېرۍ_free(&ډېرۍ->ډېرۍ, ptr);
		k_spin_unlock(&ډېرۍ->قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ_sys, k_free, ډېرۍ, ptr);
	}
}

#إذا (K_ډېرۍ_MEM_POOL_اندازه > 0)

K_ډېرۍ_DEFINE(system_ډېرۍ, Z_ډېرۍ_MIN_اندازه_FOR(K_ډېرۍ_MEM_POOL_اندازه));
#تعریف_کړه SYSTEM_ډېرۍ (&system_ډېرۍ)

#إذا defined(CONFIG_SYS_ډېرۍ_KASAN_SYSTEM)
#ضمن <zephyr/sys/heap_kasan.h>
K_ډېرۍ_KASAN_ENABLE(system_ډېرۍ, Z_ډېرۍ_MIN_اندازه_FOR(K_ډېرۍ_MEM_POOL_اندازه));
#د_که_پای

فراغ *k_aligned_alloc(size_t align, size_t اندازه)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ_sys, k_aligned_alloc, SYSTEM_ډېرۍ);

	فراغ *ret = z_alloc_helper(SYSTEM_ډېرۍ, align, اندازه, sys_ډېرۍ_aligned_alloc);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ_sys, k_aligned_alloc, SYSTEM_ډېرۍ, ret);

	أرجع ret;
}

فراغ *k_malloc(size_t اندازه)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ_sys, k_malloc, SYSTEM_ډېرۍ);

	فراغ *ret = z_alloc_helper(SYSTEM_ډېرۍ, 0, اندازه, sys_ډېرۍ_noalign_alloc);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ_sys, k_malloc, SYSTEM_ډېرۍ, ret);

	أرجع ret;
}

فراغ *k_calloc(size_t nmemb, size_t اندازه)
{
	فراغ *ret;
	size_t bounds;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ_sys, k_calloc, SYSTEM_ډېرۍ);

	إذا (اندازه_mul_ډکېدنه(nmemb, اندازه, &bounds)) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ_sys, k_calloc, SYSTEM_ډېرۍ, عدم);

		أرجع عدم;
	}

	ret = k_malloc(bounds);
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ_sys, k_calloc, SYSTEM_ډېرۍ, ret);

	أرجع ret;
}

فراغ *k_realloc(فراغ *ptr, size_t اندازه)
{
	بنية k_ډېرۍ *ډېرۍ, **ډېرۍ_ref;
	k_spinlock_کلی_t کلی;
	فراغ *ret;

	إذا (اندازه == 0) {
		k_free(ptr);
		أرجع عدم;
	}
	إذا (ptr == عدم) {
		أرجع k_malloc(اندازه);
	}
	ډېرۍ_ref = ptr;
	ptr = --ډېرۍ_ref;
	ډېرۍ = *ډېرۍ_ref;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ_sys, k_realloc, ډېرۍ, ptr);

	إذا (اندازه_جمع_کړه_ډکېدنه(اندازه, حجم(ډېرۍ_ref), &اندازه)) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ_sys, k_realloc, ډېرۍ, ptr, عدم);
		أرجع عدم;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	کلی = k_spin_قلف(&ډېرۍ->قلف);
	ret = sys_ډېرۍ_realloc(&ډېرۍ->ډېرۍ, ptr, اندازه);
	k_spin_unlock(&ډېرۍ->قلف, کلی);

	إذا (ret != عدم) {
		ډېرۍ_ref = ret;
		ret = ++ډېرۍ_ref;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ_sys, k_realloc, ډېرۍ, ptr, ret);

	أرجع ret;
}

فراغ k_thread_system_pool_ټاکل(بنية k_thread *thread)
{
	thread->resource_pool = SYSTEM_ډېرۍ;
}
#وإلا
#تعریف_کړه SYSTEM_ډېرۍ	عدم
#د_که_پای /* K_HEAP_MEM_POOL_SIZE */

ساكن فراغ *z_thread_alloc_helper(size_t align, size_t اندازه,
				   sys_ډېرۍ_allocator_t sys_ډېرۍ_allocator)
{
	فراغ *ret;
	بنية k_ډېرۍ *ډېرۍ;

	إذا (k_is_دننه_isr()) {
		ډېرۍ = SYSTEM_ډېرۍ;
	} وإلا {
		ډېرۍ = _current->resource_pool;
	}

	إذا (ډېرۍ != عدم) {
		ret = z_alloc_helper(ډېرۍ, align, اندازه, sys_ډېرۍ_allocator);
	} وإلا {
		ret = عدم;
	}

	أرجع ret;
}

فراغ *z_thread_aligned_alloc(size_t align, size_t اندازه)
{
	أرجع z_thread_alloc_helper(align, اندازه, sys_ډېرۍ_aligned_alloc);
}

فراغ *z_thread_malloc(size_t اندازه)
{
	أرجع z_thread_alloc_helper(0, اندازه, sys_ډېرۍ_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ساكن فراغ *ثابت __used kheap_ref = k_ډېرۍ_init;
