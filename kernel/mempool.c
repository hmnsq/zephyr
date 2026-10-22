/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <string.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/util.h>

تعريفنوع فراغ * (sys_انباشته_allocator_t)(بنية sys_انباشته *انباشته, size_t align, size_t بایتها);

ساكن فراغ *z_alloc_helper(بنية k_انباشته *انباشته, size_t align, size_t اندازه,
			    sys_انباشته_allocator_t sys_انباشته_allocator)
{
	فراغ *mem;
	بنية k_انباشته **انباشته_ref;
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
	إذا (اندازه_جمع_کن_سرریز(اندازه, حجم(انباشته_ref), &اندازه)) {
		أرجع عدم;
	}
	__align = align | حجم(انباشته_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	کلید = k_spin_قفل(&انباشته->قفل);
	mem = sys_انباشته_allocator(&انباشته->انباشته, __align, اندازه);
	k_spin_unlock(&انباشته->قفل, کلید);

	إذا (mem == عدم) {
		أرجع عدم;
	}

	انباشته_ref = mem;
	*انباشته_ref = انباشته;
	mem = ++انباشته_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	أرجع mem;
}

فراغ k_free(فراغ *ptr)
{
	بنية k_انباشته **انباشته_ref;

	إذا (ptr != عدم) {
		انباشته_ref = ptr;
		--انباشته_ref;
		ptr = انباشته_ref;

		SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته_sys, k_free, *انباشته_ref, انباشته_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		بنية k_انباشته *انباشته = *انباشته_ref;
		k_spinlock_کلید_t کلید = k_spin_قفل(&انباشته->قفل);

		sys_انباشته_free(&انباشته->انباشته, ptr);
		k_spin_unlock(&انباشته->قفل, کلید);

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته_sys, k_free, انباشته, ptr);
	}
}

#إذا (K_انباشته_MEM_POOL_اندازه > 0)

K_انباشته_DEFINE(system_انباشته, Z_انباشته_MIN_اندازه_FOR(K_انباشته_MEM_POOL_اندازه));
#تعریف_کن SYSTEM_انباشته (&system_انباشته)

#إذا defined(CONFIG_SYS_انباشته_KASAN_SYSTEM)
#ضمن <zephyr/sys/heap_kasan.h>
K_انباشته_KASAN_ENABLE(system_انباشته, Z_انباشته_MIN_اندازه_FOR(K_انباشته_MEM_POOL_اندازه));
#پایان_اگر

فراغ *k_aligned_alloc(size_t align, size_t اندازه)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته_sys, k_aligned_alloc, SYSTEM_انباشته);

	فراغ *ret = z_alloc_helper(SYSTEM_انباشته, align, اندازه, sys_انباشته_aligned_alloc);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته_sys, k_aligned_alloc, SYSTEM_انباشته, ret);

	أرجع ret;
}

فراغ *k_malloc(size_t اندازه)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته_sys, k_malloc, SYSTEM_انباشته);

	فراغ *ret = z_alloc_helper(SYSTEM_انباشته, 0, اندازه, sys_انباشته_noalign_alloc);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته_sys, k_malloc, SYSTEM_انباشته, ret);

	أرجع ret;
}

فراغ *k_calloc(size_t nmemb, size_t اندازه)
{
	فراغ *ret;
	size_t bounds;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته_sys, k_calloc, SYSTEM_انباشته);

	إذا (اندازه_mul_سرریز(nmemb, اندازه, &bounds)) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته_sys, k_calloc, SYSTEM_انباشته, عدم);

		أرجع عدم;
	}

	ret = k_malloc(bounds);
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته_sys, k_calloc, SYSTEM_انباشته, ret);

	أرجع ret;
}

فراغ *k_realloc(فراغ *ptr, size_t اندازه)
{
	بنية k_انباشته *انباشته, **انباشته_ref;
	k_spinlock_کلید_t کلید;
	فراغ *ret;

	إذا (اندازه == 0) {
		k_free(ptr);
		أرجع عدم;
	}
	إذا (ptr == عدم) {
		أرجع k_malloc(اندازه);
	}
	انباشته_ref = ptr;
	ptr = --انباشته_ref;
	انباشته = *انباشته_ref;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته_sys, k_realloc, انباشته, ptr);

	إذا (اندازه_جمع_کن_سرریز(اندازه, حجم(انباشته_ref), &اندازه)) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته_sys, k_realloc, انباشته, ptr, عدم);
		أرجع عدم;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	کلید = k_spin_قفل(&انباشته->قفل);
	ret = sys_انباشته_realloc(&انباشته->انباشته, ptr, اندازه);
	k_spin_unlock(&انباشته->قفل, کلید);

	إذا (ret != عدم) {
		انباشته_ref = ret;
		ret = ++انباشته_ref;
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته_sys, k_realloc, انباشته, ptr, ret);

	أرجع ret;
}

فراغ k_thread_system_pool_تخصیص_بده(بنية k_thread *thread)
{
	thread->resource_pool = SYSTEM_انباشته;
}
#وإلا
#تعریف_کن SYSTEM_انباشته	عدم
#پایان_اگر /* K_HEAP_MEM_POOL_SIZE */

ساكن فراغ *z_thread_alloc_helper(size_t align, size_t اندازه,
				   sys_انباشته_allocator_t sys_انباشته_allocator)
{
	فراغ *ret;
	بنية k_انباشته *انباشته;

	إذا (k_is_درون_isr()) {
		انباشته = SYSTEM_انباشته;
	} وإلا {
		انباشته = _current->resource_pool;
	}

	إذا (انباشته != عدم) {
		ret = z_alloc_helper(انباشته, align, اندازه, sys_انباشته_allocator);
	} وإلا {
		ret = عدم;
	}

	أرجع ret;
}

فراغ *z_thread_aligned_alloc(size_t align, size_t اندازه)
{
	أرجع z_thread_alloc_helper(align, اندازه, sys_انباشته_aligned_alloc);
}

فراغ *z_thread_malloc(size_t اندازه)
{
	أرجع z_thread_alloc_helper(0, اندازه, sys_انباشته_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ساكن فراغ *ثابت __used kheap_ref = k_انباشته_init;
