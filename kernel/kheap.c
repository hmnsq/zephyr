/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/linker-defs.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>

ಪೂರ್ಣಾಂಕ k_ರಾಶಿ_array_get(ರಚನೆ k_ರಾಶಿ **ರಾಶಿ_2)
{
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_ರಾಶಿ, 0, ರಾಶಿ_2);
	/* Number of statically defined heaps */
	STRUCT_SECTION_ಎಣಿಕೆ(k_ರಾಶಿ, &ಅಂಕೆ);
	ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}

ಶೂನ್ಯ k_ರಾಶಿ_init(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, ಶೂನ್ಯ *mem, size_t ಬೈಟುಗಳು)
{
	z_waitq_init(&ರಾಶಿ_2->wait_q);
	ರಾಶಿ_2->ಬೀಗ = (ರಚನೆ k_spinlock) {};
	sys_ರಾಶಿ_init(&ರಾಶಿ_2->ರಾಶಿ_2, mem, ಬೈಟುಗಳು);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_ರಾಶಿ, ರಾಶಿ_2);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statics_init(ಶೂನ್ಯ)
{
	STRUCT_SECTION_FOREACH(k_ರಾಶಿ, ರಾಶಿ_2) {
		k_ರಾಶಿ_init(ರಾಶಿ_2, ರಾಶಿ_2->ರಾಶಿ_2.init_mem, ರಾಶಿ_2->ರಾಶಿ_2.init_ಬೈಟುಗಳು);
	}
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_ಆದ್ಯತೆ_OBJECTS);

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ * (sys_ರಾಶಿ_allocator_t)(ರಚನೆ sys_ರಾಶಿ *ರಾಶಿ_2, size_t align, size_t ಬೈಟುಗಳು);

ಸ್ಥಿರ ಶೂನ್ಯ *z_ರಾಶಿ_alloc_helper(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, size_t align, size_t ಬೈಟುಗಳು,
				 k_timeout_t timeout,
				 sys_ರಾಶಿ_allocator_t *sys_ರಾಶಿ_allocator)
{
	k_timepoint_t ಕೊನೆ = sys_timepoint_calc(timeout);
	ಶೂನ್ಯ *ret = ಖಾಲಿ;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);

	__ASSERT(!arch_is_ಒಳ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	ಆಗ (ret == ಖಾಲಿ) {
		ret = sys_ರಾಶಿ_allocator(&ರಾಶಿ_2->ರಾಶಿ_2, align, ಬೈಟುಗಳು);

		ಆದರೆ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ಖಾಲಿ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (!blocked_alloc) {
			blocked_alloc = true;

			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_ರಾಶಿ, alloc_helper, ರಾಶಿ_2, timeout);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(ಕೊನೆ);
		(ಶೂನ್ಯ) z_pend_curr(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ, &ರಾಶಿ_2->wait_q, timeout);
		ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);
	}

	k_spin_unlock(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *k_ರಾಶಿ_alloc(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, size_t ಬೈಟುಗಳು, k_timeout_t timeout)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, alloc, ರಾಶಿ_2, timeout);

	ಶೂನ್ಯ *ret = z_ರಾಶಿ_alloc_helper(ರಾಶಿ_2, 0, ಬೈಟುಗಳು, timeout,
					sys_ರಾಶಿ_noalign_alloc);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, alloc, ರಾಶಿ_2, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *k_ರಾಶಿ_aligned_alloc(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, size_t align, size_t ಬೈಟುಗಳು,
			k_timeout_t timeout)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, aligned_alloc, ರಾಶಿ_2, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	ಶೂನ್ಯ *ret = z_ರಾಶಿ_alloc_helper(ರಾಶಿ_2, align, ಬೈಟುಗಳು, timeout,
					sys_ರಾಶಿ_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_ಬಳಸದ(blocked_alloc);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, aligned_alloc, ರಾಶಿ_2, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *k_ರಾಶಿ_calloc(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, size_t ಅಂಕೆ, size_t ಗಾತ್ರ_2, k_timeout_t timeout)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು, ರಾಶಿ_2, timeout);

	ಶೂನ್ಯ *ret = ಖಾಲಿ;
	size_t bounds = 0U;

	ಆದರೆ (!ಗಾತ್ರ_mul_ಅತಿಪ್ರವಾಹ(ಅಂಕೆ, ಗಾತ್ರ_2, &bounds)) {
		ret = k_ರಾಶಿ_alloc(ರಾಶಿ_2, bounds, timeout);
	}
	ಆದರೆ (ret != ಖಾಲಿ) {
		(ಶೂನ್ಯ)memset(ret, 0, bounds);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು, ರಾಶಿ_2, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ *k_ರಾಶಿ_realloc(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, ಶೂನ್ಯ *ptr, size_t ಬೈಟುಗಳು, k_timeout_t timeout)
{
	k_timepoint_t ಕೊನೆ = sys_timepoint_calc(timeout);
	ಶೂನ್ಯ *ret = ಖಾಲಿ;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, realloc, ರಾಶಿ_2, ptr, ಬೈಟುಗಳು, timeout);

	__ASSERT(!arch_is_ಒಳ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ಆಗ (ret == ಖಾಲಿ) {
		ret = sys_ರಾಶಿ_realloc(&ರಾಶಿ_2->ರಾಶಿ_2, ptr, ಬೈಟುಗಳು);

		ಆದರೆ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ಖಾಲಿ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ನಿಲ್ಲಿಸು;
		}

		timeout = sys_timepoint_timeout(ಕೊನೆ);
		(ಶೂನ್ಯ) z_pend_curr(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ, &ರಾಶಿ_2->wait_q, timeout);
		ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);
	}

	k_spin_unlock(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, realloc, ರಾಶಿ_2, ptr, ಬೈಟುಗಳು, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ k_ರಾಶಿ_free(ರಚನೆ k_ರಾಶಿ *ರಾಶಿ_2, ಶೂನ್ಯ *mem)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ_2->ಬೀಗ);

	sys_ರಾಶಿ_free(&ರಾಶಿ_2->ರಾಶಿ_2, mem);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_ರಾಶಿ, ಬಿಡುಗಡೆ, ರಾಶಿ_2);
	ಆದರೆ (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&ರಾಶಿ_2->wait_q) != 0)) {
		z_reschedule(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&ರಾಶಿ_2->ಬೀಗ, ಕೀಲಿ);
	}
}
