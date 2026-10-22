/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <string.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/util.h>

قسمتعریف خالی * (sys_ڈھیر_allocator_t)(ساخت sys_ڈھیر *ڈھیر_2, size_t align, size_t بائٹیں);

ساکن خالی *z_alloc_helper(ساخت k_ڈھیر *ڈھیر_2, size_t align, size_t سائز,
			    sys_ڈھیر_allocator_t sys_ڈھیر_allocator)
{
	خالی *mem;
	ساخت k_ڈھیر **ڈھیر_ref;
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
	اگر (سائز_جوڑو_حد_سے_تجاوز(سائز, حجم(ڈھیر_ref), &سائز)) {
		واپس تہی;
	}
	__align = align | حجم(ڈھیر_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	کلید = k_spin_تالا(&ڈھیر_2->تالا);
	mem = sys_ڈھیر_allocator(&ڈھیر_2->ڈھیر_2, __align, سائز);
	k_spin_unlock(&ڈھیر_2->تالا, کلید);

	اگر (mem == تہی) {
		واپس تہی;
	}

	ڈھیر_ref = mem;
	*ڈھیر_ref = ڈھیر_2;
	mem = ++ڈھیر_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	واپس mem;
}

خالی k_free(خالی *ptr)
{
	ساخت k_ڈھیر **ڈھیر_ref;

	اگر (ptr != تہی) {
		ڈھیر_ref = ptr;
		--ڈھیر_ref;
		ptr = ڈھیر_ref;

		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_free, *ڈھیر_ref, ڈھیر_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		ساخت k_ڈھیر *ڈھیر_2 = *ڈھیر_ref;
		k_spinlock_کلید_t کلید = k_spin_تالا(&ڈھیر_2->تالا);

		sys_ڈھیر_free(&ڈھیر_2->ڈھیر_2, ptr);
		k_spin_unlock(&ڈھیر_2->تالا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_free, ڈھیر_2, ptr);
	}
}

#اگر (K_ڈھیر_MEM_POOL_سائز > 0)

K_ڈھیر_DEFINE_2(system_ڈھیر, Z_ڈھیر_MIN_سائز_FOR(K_ڈھیر_MEM_POOL_سائز));
#تعریف_کرو SYSTEM_ڈھیر (&system_ڈھیر)

#اگر defined(CONFIG_SYS_ڈھیر_KASAN_SYSTEM)
#شامل <zephyr/sys/heap_kasan.h>
K_ڈھیر_KASAN_ENABLE(system_ڈھیر, Z_ڈھیر_MIN_سائز_FOR(K_ڈھیر_MEM_POOL_سائز));
#اگر_ختم

خالی *k_aligned_alloc(size_t align, size_t سائز)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_aligned_alloc, SYSTEM_ڈھیر);

	خالی *ret = z_alloc_helper(SYSTEM_ڈھیر, align, سائز, sys_ڈھیر_aligned_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_aligned_alloc, SYSTEM_ڈھیر, ret);

	واپس ret;
}

خالی *k_malloc(size_t سائز)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_malloc, SYSTEM_ڈھیر);

	خالی *ret = z_alloc_helper(SYSTEM_ڈھیر, 0, سائز, sys_ڈھیر_noalign_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_malloc, SYSTEM_ڈھیر, ret);

	واپس ret;
}

خالی *k_calloc(size_t nmemb, size_t سائز)
{
	خالی *ret;
	size_t bounds;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_calloc, SYSTEM_ڈھیر);

	اگر (سائز_mul_حد_سے_تجاوز(nmemb, سائز, &bounds)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_calloc, SYSTEM_ڈھیر, تہی);

		واپس تہی;
	}

	ret = k_malloc(bounds);
	اگر (ret != تہی) {
		(خالی)memset(ret, 0, bounds);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_calloc, SYSTEM_ڈھیر, ret);

	واپس ret;
}

خالی *k_realloc(خالی *ptr, size_t سائز)
{
	ساخت k_ڈھیر *ڈھیر_2, **ڈھیر_ref;
	k_spinlock_کلید_t کلید;
	خالی *ret;

	اگر (سائز == 0) {
		k_free(ptr);
		واپس تہی;
	}
	اگر (ptr == تہی) {
		واپس k_malloc(سائز);
	}
	ڈھیر_ref = ptr;
	ptr = --ڈھیر_ref;
	ڈھیر_2 = *ڈھیر_ref;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_sys, k_realloc, ڈھیر_2, ptr);

	اگر (سائز_جوڑو_حد_سے_تجاوز(سائز, حجم(ڈھیر_ref), &سائز)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_realloc, ڈھیر_2, ptr, تہی);
		واپس تہی;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	کلید = k_spin_تالا(&ڈھیر_2->تالا);
	ret = sys_ڈھیر_realloc(&ڈھیر_2->ڈھیر_2, ptr, سائز);
	k_spin_unlock(&ڈھیر_2->تالا, کلید);

	اگر (ret != تہی) {
		ڈھیر_ref = ret;
		ret = ++ڈھیر_ref;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_sys, k_realloc, ڈھیر_2, ptr, ret);

	واپس ret;
}

خالی k_thread_system_pool_تفویض(ساخت k_thread *thread)
{
	thread->resource_pool = SYSTEM_ڈھیر;
}
#ورنہ
#تعریف_کرو SYSTEM_ڈھیر	تہی
#اگر_ختم /* K_HEAP_MEM_POOL_SIZE */

ساکن خالی *z_thread_alloc_helper(size_t align, size_t سائز,
				   sys_ڈھیر_allocator_t sys_ڈھیر_allocator)
{
	خالی *ret;
	ساخت k_ڈھیر *ڈھیر_2;

	اگر (k_is_اندر_isr()) {
		ڈھیر_2 = SYSTEM_ڈھیر;
	} ورنہ {
		ڈھیر_2 = _current->resource_pool;
	}

	اگر (ڈھیر_2 != تہی) {
		ret = z_alloc_helper(ڈھیر_2, align, سائز, sys_ڈھیر_allocator);
	} ورنہ {
		ret = تہی;
	}

	واپس ret;
}

خالی *z_thread_aligned_alloc(size_t align, size_t سائز)
{
	واپس z_thread_alloc_helper(align, سائز, sys_ڈھیر_aligned_alloc);
}

خالی *z_thread_malloc(size_t سائز)
{
	واپس z_thread_alloc_helper(0, سائز, sys_ڈھیر_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ساکن خالی *ثابت __used kheap_ref = k_ڈھیر_init;
