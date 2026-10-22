/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/util.h>

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ * (sys_ರಾಶಿ_allocator_t)(ರಚನೆ sys_ರಾಶಿ *ರಾಶಿ_2, size_t align, size_t ಬೈಟುಗಳು);

ಸ್ಥಿರ ಶೂನ್ಯ *z_alloc_helper(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, size_t align, size_t ಗಾತ್ರ_2,
			    sys_ರಾಶಿ_allocator_t sys_ರಾಶಿ_allocator)
{
	ಶೂನ್ಯ *mem;
	ರಚನೆ k_ರಾಶಿ **ರಾಶಿ_ref;
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
	ಆದರೆ (ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ(ಗಾತ್ರ_2, ಗಾತ್ರ(ರಾಶಿ_ref), &ಗಾತ್ರ_2)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	__align = align | ಗಾತ್ರ(ರಾಶಿ_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);
	mem = sys_ರಾಶಿ_allocator(&ರಾಶಿ_2->ರಾಶಿ_2, __align, ಗಾತ್ರ_2);
	k_spin_unlock(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ);

	ಆದರೆ (mem == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ರಾಶಿ_ref = mem;
	*ರಾಶಿ_ref = ರಾಶಿ_2;
	mem = ++ರಾಶಿ_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	ಹಿಂತಿರುಗಿ mem;
}

ಶೂನ್ಯ k_free(ಶೂನ್ಯ *ptr)
{
	ರಚನೆ k_ರಾಶಿ **ರಾಶಿ_ref;

	ಆದರೆ (ptr != ಖಾಲಿ) {
		ರಾಶಿ_ref = ptr;
		--ರಾಶಿ_ref;
		ptr = ರಾಶಿ_ref;

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_sys, k_free, *ರಾಶಿ_ref, ರಾಶಿ_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2 = *ರಾಶಿ_ref;
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);

		sys_ರಾಶಿ_free(&ರಾಶಿ_2->ರಾಶಿ_2, ptr);
		k_spin_unlock(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_sys, k_free, ರಾಶಿ_2, ptr);
	}
}

#ಆದರೆ (K_ರಾಶಿ_MEM_POOL_ಗಾತ್ರ > 0)

K_ರಾಶಿ_DEFINE_2(system_ರಾಶಿ, Z_ರಾಶಿ_MIN_ಗಾತ್ರ_FOR(K_ರಾಶಿ_MEM_POOL_ಗಾತ್ರ));
#ವ್ಯಾಖ್ಯಾನಿಸು SYSTEM_ರಾಶಿ (&system_ರಾಶಿ)

#ಆದರೆ defined(CONFIG_SYS_ರಾಶಿ_KASAN_SYSTEM)
#ಅನ್ತರ್ಭಾವ <zephyr/sys/heap_kasan.h>
K_ರಾಶಿ_KASAN_ENABLE(system_ರಾಶಿ, Z_ರಾಶಿ_MIN_ಗಾತ್ರ_FOR(K_ರಾಶಿ_MEM_POOL_ಗಾತ್ರ));
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ *k_aligned_alloc(size_t align, size_t ಗಾತ್ರ_2)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_sys, k_aligned_alloc, SYSTEM_ರಾಶಿ);

	ಶೂನ್ಯ *ret = z_alloc_helper(SYSTEM_ರಾಶಿ, align, ಗಾತ್ರ_2, sys_ರಾಶಿ_aligned_alloc);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_sys, k_aligned_alloc, SYSTEM_ರಾಶಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *k_malloc(size_t ಗಾತ್ರ_2)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_sys, k_malloc, SYSTEM_ರಾಶಿ);

	ಶೂನ್ಯ *ret = z_alloc_helper(SYSTEM_ರಾಶಿ, 0, ಗಾತ್ರ_2, sys_ರಾಶಿ_noalign_alloc);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_sys, k_malloc, SYSTEM_ರಾಶಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *k_calloc(size_t nmemb, size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *ret;
	size_t bounds;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_sys, k_calloc, SYSTEM_ರಾಶಿ);

	ಆದರೆ (ಗಾತ್ರ_mul_ಅತಿಪ್ರವಾಹ(nmemb, ಗಾತ್ರ_2, &bounds)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_sys, k_calloc, SYSTEM_ರಾಶಿ, ಖಾಲಿ);

		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ret = k_malloc(bounds);
	ಆದರೆ (ret != ಖಾಲಿ) {
		(ಶೂನ್ಯ)memset(ret, 0, bounds);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_sys, k_calloc, SYSTEM_ರಾಶಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *k_realloc(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2)
{
	ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, **ರಾಶಿ_ref;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಶೂನ್ಯ *ret;

	ಆದರೆ (ಗಾತ್ರ_2 == 0) {
		k_free(ptr);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಆದರೆ (ptr == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ k_malloc(ಗಾತ್ರ_2);
	}
	ರಾಶಿ_ref = ptr;
	ptr = --ರಾಶಿ_ref;
	ರಾಶಿ_2 = *ರಾಶಿ_ref;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_sys, k_realloc, ರಾಶಿ_2, ptr);

	ಆದರೆ (ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ(ಗಾತ್ರ_2, ಗಾತ್ರ(ರಾಶಿ_ref), &ಗಾತ್ರ_2)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_sys, k_realloc, ರಾಶಿ_2, ptr, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);
	ret = sys_ರಾಶಿ_realloc(&ರಾಶಿ_2->ರಾಶಿ_2, ptr, ಗಾತ್ರ_2);
	k_spin_unlock(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ);

	ಆದರೆ (ret != ಖಾಲಿ) {
		ರಾಶಿ_ref = ret;
		ret = ++ರಾಶಿ_ref;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_sys, k_realloc, ರಾಶಿ_2, ptr, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ k_thread_system_pool_ನಿಯೋಜಿಸು(ರಚನೆ k_thread *thread)
{
	thread->resource_pool = SYSTEM_ರಾಶಿ;
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು SYSTEM_ರಾಶಿ	ಖಾಲಿ
#ಆದರೆ_ಅಂತ್ಯ /* K_HEAP_MEM_POOL_SIZE */

ಸ್ಥಿರ ಶೂನ್ಯ *z_thread_alloc_helper(size_t align, size_t ಗಾತ್ರ_2,
				   sys_ರಾಶಿ_allocator_t sys_ರಾಶಿ_allocator)
{
	ಶೂನ್ಯ *ret;
	ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2;

	ಆದರೆ (k_is_ಒಳ_isr()) {
		ರಾಶಿ_2 = SYSTEM_ರಾಶಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ರಾಶಿ_2 = _current->resource_pool;
	}

	ಆದರೆ (ರಾಶಿ_2 != ಖಾಲಿ) {
		ret = z_alloc_helper(ರಾಶಿ_2, align, ಗಾತ್ರ_2, sys_ರಾಶಿ_allocator);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *z_thread_aligned_alloc(size_t align, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ z_thread_alloc_helper(align, ಗಾತ್ರ_2, sys_ರಾಶಿ_aligned_alloc);
}

ಶೂನ್ಯ *z_thread_malloc(size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ z_thread_alloc_helper(0, ಗಾತ್ರ_2, sys_ರಾಶಿ_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
ಸ್ಥಿರ ಶೂನ್ಯ *ಸ್ಥಿರಾಂಕ __used kheap_ref = k_ರಾಶಿ_init;
