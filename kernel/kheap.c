/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/linker-defs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>

ਪੂਰਨ k_ਢੇਰ_array_get(ਬਣਤਰ k_ਢੇਰ **ਢੇਰ_2)
{
	ਪੂਰਨ ਅੰਕ;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_ਢੇਰ, 0, ਢੇਰ_2);
	/* Number of statically defined heaps */
	STRUCT_SECTION_ਗਿਣਤੀ(k_ਢੇਰ, &ਅੰਕ);
	ਵਾਪਸ ਅੰਕ;
}

ਖਾਲੀ k_ਢੇਰ_init(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, ਖਾਲੀ *mem, size_t ਬਾਈਟਾਂ)
{
	z_waitq_init(&ਢੇਰ_2->wait_q);
	ਢੇਰ_2->ਤਾਲਾ = (ਬਣਤਰ k_spinlock) {};
	sys_ਢੇਰ_init(&ਢੇਰ_2->ਢੇਰ_2, mem, ਬਾਈਟਾਂ);

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_ਢੇਰ, ਢੇਰ_2);
}

ਸਥਿਰ ਪੂਰਨ statics_init(ਖਾਲੀ)
{
	STRUCT_SECTION_FOREACH(k_ਢੇਰ, ਢੇਰ_2) {
		k_ਢੇਰ_init(ਢੇਰ_2, ਢੇਰ_2->ਢੇਰ_2.init_mem, ਢੇਰ_2->ਢੇਰ_2.init_ਬਾਈਟਾਂ);
	}
	ਵਾਪਸ 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_ਤਰਜੀਹ_OBJECTS);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ * (sys_ਢੇਰ_allocator_t)(ਬਣਤਰ sys_ਢੇਰ *ਢੇਰ_2, size_t align, size_t ਬਾਈਟਾਂ);

ਸਥਿਰ ਖਾਲੀ *z_ਢੇਰ_alloc_helper(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, size_t align, size_t ਬਾਈਟਾਂ,
				 k_timeout_t timeout,
				 sys_ਢੇਰ_allocator_t *sys_ਢੇਰ_allocator)
{
	k_timepoint_t ਸਿਰਾ = sys_timepoint_calc(timeout);
	ਖਾਲੀ *ret = ਨਿਰਾ;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);

	__ASSERT(!arch_is_ਵਿੱਚ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	ਜਦੋਂ (ret == ਨਿਰਾ) {
		ret = sys_ਢੇਰ_allocator(&ਢੇਰ_2->ਢੇਰ_2, align, ਬਾਈਟਾਂ);

		ਜੇ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ਨਿਰਾ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ਰੋਕੋ;
		}

		ਜੇ (!blocked_alloc) {
			blocked_alloc = true;

			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_ਢੇਰ, alloc_helper, ਢੇਰ_2, timeout);
		} ਨਹੀਂਤਾਂ {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(ਸਿਰਾ);
		(ਖਾਲੀ) z_pend_curr(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ, &ਢੇਰ_2->wait_q, timeout);
		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);
	}

	k_spin_unlock(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ);
	ਵਾਪਸ ret;
}

ਖਾਲੀ *k_ਢੇਰ_alloc(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, size_t ਬਾਈਟਾਂ, k_timeout_t timeout)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ, alloc, ਢੇਰ_2, timeout);

	ਖਾਲੀ *ret = z_ਢੇਰ_alloc_helper(ਢੇਰ_2, 0, ਬਾਈਟਾਂ, timeout,
					sys_ਢੇਰ_noalign_alloc);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ, alloc, ਢੇਰ_2, timeout, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ *k_ਢੇਰ_aligned_alloc(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, size_t align, size_t ਬਾਈਟਾਂ,
			k_timeout_t timeout)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ, aligned_alloc, ਢੇਰ_2, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	ਖਾਲੀ *ret = z_ਢੇਰ_alloc_helper(ਢੇਰ_2, align, ਬਾਈਟਾਂ, timeout,
					sys_ਢੇਰ_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_ਅਣਵਰਤਿਆ(blocked_alloc);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ, aligned_alloc, ਢੇਰ_2, timeout, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ *k_ਢੇਰ_calloc(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, size_t ਅੰਕ, size_t ਮਾਪ, k_timeout_t timeout)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ, ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ, ਢੇਰ_2, timeout);

	ਖਾਲੀ *ret = ਨਿਰਾ;
	size_t bounds = 0U;

	ਜੇ (!ਮਾਪ_mul_ਉਛਾਲ(ਅੰਕ, ਮਾਪ, &bounds)) {
		ret = k_ਢੇਰ_alloc(ਢੇਰ_2, bounds, timeout);
	}
	ਜੇ (ret != ਨਿਰਾ) {
		(ਖਾਲੀ)memset(ret, 0, bounds);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ, ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ, ਢੇਰ_2, timeout, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ *k_ਢੇਰ_realloc(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, ਖਾਲੀ *ptr, size_t ਬਾਈਟਾਂ, k_timeout_t timeout)
{
	k_timepoint_t ਸਿਰਾ = sys_timepoint_calc(timeout);
	ਖਾਲੀ *ret = ਨਿਰਾ;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ, realloc, ਢੇਰ_2, ptr, ਬਾਈਟਾਂ, timeout);

	__ASSERT(!arch_is_ਵਿੱਚ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ਜਦੋਂ (ret == ਨਿਰਾ) {
		ret = sys_ਢੇਰ_realloc(&ਢੇਰ_2->ਢੇਰ_2, ptr, ਬਾਈਟਾਂ);

		ਜੇ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ਨਿਰਾ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ਰੋਕੋ;
		}

		timeout = sys_timepoint_timeout(ਸਿਰਾ);
		(ਖਾਲੀ) z_pend_curr(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ, &ਢੇਰ_2->wait_q, timeout);
		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);
	}

	k_spin_unlock(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ, realloc, ਢੇਰ_2, ptr, ਬਾਈਟਾਂ, timeout, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ k_ਢੇਰ_free(ਬਣਤਰ k_ਢੇਰ *ਢੇਰ_2, ਖਾਲੀ *mem)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ_2->ਤਾਲਾ);

	sys_ਢੇਰ_free(&ਢੇਰ_2->ਢੇਰ_2, mem);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_ਢੇਰ, ਮੁਕਤ, ਢੇਰ_2);
	ਜੇ (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&ਢੇਰ_2->wait_q) != 0)) {
		z_reschedule(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&ਢੇਰ_2->ਤਾਲਾ, ਕੁੰਜੀ);
	}
}
