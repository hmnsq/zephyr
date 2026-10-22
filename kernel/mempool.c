/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <string.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/util.h>

تعريفنوع فراغ * (sys_ڈھیر_allocator_t)(بنية sys_ڈھیر *ڈھیر_2, size_t align, size_t بائٹاں);

ساكن فراغ *z_alloc_helper(بنية k_ڈھیر *ڈھیر_2, size_t align, size_t ناپ,
			    sys_ڈھیر_allocator_t sys_ڈھیر_allocator)
{
	فراغ *mem;
	بنية k_ڈھیر **ڈھیر_ref;
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
	إذا (ناپ_جوڑو_حد_توں_ودھ(ناپ, حجم(ڈھیر_ref), &ناپ)) {
		أرجع عدم;
	}
	__align = align | حجم(ڈھیر_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	کلید = k_spin_جندرا(&ڈھیر_2->جندرا);
	mem = sys_ڈھیر_allocator(&ڈھیر_2->ڈھیر_2, __align, ناپ);
	k_spin_unlock(&ڈھیر_2->جندرا, کلید);

	إذا (mem == عدم) {
		أرجع عدم;
	}

	ڈھیر_ref = mem;
	*ڈھیر_ref = ڈھیر_2;
	mem = ++ڈھیر_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	أرجع mem;
}

فراغ k_free(فراغ *ptr)
{
	بنية k_ڈھیر **ڈھیر_ref;

	إذا (ptr != عدم) {
		ڈھیر_ref = ptr;
		--ڈھیر_ref;
		ptr = ڈھیر_ref;

		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_free, *ڈھیر_ref, ڈھیر_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		بنية k_ڈھیر *ڈھیر_2 = *ڈھیر_ref;
		k_spinlock_کلید_t کلید = k_spin_جندرا(&ڈھیر_2->جندرا);

		sys_ڈھیر_free(&ڈھیر_2->ڈھیر_2, ptr);
		k_spin_unlock(&ڈھیر_2->جندرا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_free, ڈھیر_2, ptr);
	}
}

#إذا (K_ڈھیر_MEM_POOL_ناپ > 0)

K_ڈھیر_DEFINE_2(system_ڈھیر, Z_ڈھیر_MIN_ناپ_FOR(K_ڈھیر_MEM_POOL_ناپ));
#تعریف_کرو SYSTEM_ڈھیر (&system_ڈھیر)

#إذا defined(CONFIG_SYS_ڈھیر_KASAN_SYSTEM)
#ضمن <zephyr/sys/heap_kasan.h>
K_ڈھیر_KASAN_ENABLE(system_ڈھیر, Z_ڈھیر_MIN_ناپ_FOR(K_ڈھیر_MEM_POOL_ناپ));
#جے_انت

فراغ *k_aligned_alloc(size_t align, size_t ناپ)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_aligned_alloc, SYSTEM_ڈھیر);

	فراغ *ret = z_alloc_helper(SYSTEM_ڈھیر, align, ناپ, sys_ڈھیر_aligned_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_aligned_alloc, SYSTEM_ڈھیر, ret);

	أرجع ret;
}

فراغ *k_malloc(size_t ناپ)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_malloc, SYSTEM_ڈھیر);

	فراغ *ret = z_alloc_helper(SYSTEM_ڈھیر, 0, ناپ, sys_ڈھیر_noalign_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_malloc, SYSTEM_ڈھیر, ret);

	أرجع ret;
}

فراغ *k_calloc(size_t nmemb, size_t ناپ)
{
	فراغ *ret;
	size_t bounds;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_calloc, SYSTEM_ڈھیر);

	إذا (ناپ_mul_حد_توں_ودھ(nmemb, ناپ, &bounds)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_calloc, SYSTEM_ڈھیر, عدم);

		أرجع عدم;
	}

	ret = k_malloc(bounds);
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_calloc, SYSTEM_ڈھیر, ret);

	أرجع ret;
}

فراغ *k_realloc(فراغ *ptr, size_t ناپ)
{
	بنية k_ڈھیر *ڈھیر_2, **ڈھیر_ref;
	k_spinlock_کلید_t کلید;
	فراغ *ret;

	إذا (ناپ == 0) {
		k_free(ptr);
		أرجع عدم;
	}
	إذا (ptr == عدم) {
		أرجع k_malloc(ناپ);
	}
	ڈھیر_ref = ptr;
	ptr = --ڈھیر_ref;
	ڈھیر_2 = *ڈھیر_ref;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_realloc, ڈھیر_2, ptr);

	إذا (ناپ_جوڑو_حد_توں_ودھ(ناپ, حجم(ڈھیر_ref), &ناپ)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_realloc, ڈھیر_2, ptr, عدم);
		أرجع عدم;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	کلید = k_spin_جندرا(&ڈھیر_2->جندرا);
	ret = sys_ڈھیر_realloc(&ڈھیر_2->ڈھیر_2, ptr, ناپ);
	k_spin_unlock(&ڈھیر_2->جندرا, کلید);

	إذا (ret != عدم) {
		ڈھیر_ref = ret;
		ret = ++ڈھیر_ref;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_realloc, ڈھیر_2, ptr, ret);

	أرجع ret;
}

فراغ k_thread_system_pool_سونپو(بنية k_thread *thread)
{
	thread->resource_pool = SYSTEM_ڈھیر;
}
#وإلا
#تعریف_کرو SYSTEM_ڈھیر	عدم
#جے_انت /* K_HEAP_MEM_POOL_SIZE */

ساكن فراغ *z_thread_alloc_helper(size_t align, size_t ناپ,
				   sys_ڈھیر_allocator_t sys_ڈھیر_allocator)
{
	فراغ *ret;
	بنية k_ڈھیر *ڈھیر_2;

	إذا (k_is_وچ_isr()) {
		ڈھیر_2 = SYSTEM_ڈھیر;
	} وإلا {
		ڈھیر_2 = _current->resource_pool;
	}

	إذا (ڈھیر_2 != عدم) {
		ret = z_alloc_helper(ڈھیر_2, align, ناپ, sys_ڈھیر_allocator);
	} وإلا {
		ret = عدم;
	}

	أرجع ret;
}

فراغ *z_thread_aligned_alloc(size_t align, size_t ناپ)
{
	أرجع z_thread_alloc_helper(align, ناپ, sys_ڈھیر_aligned_alloc);
}

فراغ *z_thread_malloc(size_t ناپ)
{
	أرجع z_thread_alloc_helper(0, ناپ, sys_ڈھیر_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ساكن فراغ *ثابت __used kheap_ref = k_ڈھیر_init;
