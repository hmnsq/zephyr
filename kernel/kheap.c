/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#ضمن <ksched.h>
#ضمن <wait_q.h>

صحيح k_ڍير_array_get(بنية k_ڍير **ڍير_2)
{
	صحيح انگ;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_ڍير, 0, ڍير_2);
	/* Number of statically defined heaps */
	STRUCT_SECTION_ڳڻپ(k_ڍير, &انگ);
	أرجع انگ;
}

فراغ k_ڍير_init(بنية k_ڍير *ڍير_2, فراغ *mem, size_t بائيٽون)
{
	z_waitq_init(&ڍير_2->wait_q);
	ڍير_2->تالو = (بنية k_spinlock) {};
	sys_ڍير_init(&ڍير_2->ڍير_2, mem, بائيٽون);

	SYS_پورٽ_TRACING_OBJ_INIT(k_ڍير, ڍير_2);
}

ساكن صحيح statics_init(فراغ)
{
	STRUCT_SECTION_FOREACH(k_ڍير, ڍير_2) {
		k_ڍير_init(ڍير_2, ڍير_2->ڍير_2.init_mem, ڍير_2->ڍير_2.init_بائيٽون);
	}
	أرجع 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_ترجيح_OBJECTS);

تعريفنوع فراغ * (sys_ڍير_allocator_t)(بنية sys_ڍير *ڍير_2, size_t align, size_t بائيٽون);

ساكن فراغ *z_ڍير_alloc_helper(بنية k_ڍير *ڍير_2, size_t align, size_t بائيٽون,
				 k_timeout_t timeout,
				 sys_ڍير_allocator_t *sys_ڍير_allocator)
{
	k_timepoint_t پڇاڙي = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ڍير_2->تالو);

	__ASSERT(!arch_is_اندريون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	طالما (ret == عدم) {
		ret = sys_ڍير_allocator(&ڍير_2->ڍير_2, align, بائيٽون);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		إذا (!blocked_alloc) {
			blocked_alloc = true;

			SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_ڍير, alloc_helper, ڍير_2, timeout);
		} وإلا {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(پڇاڙي);
		(فراغ) z_pend_curr(&ڍير_2->تالو, ڪنجي, &ڍير_2->wait_q, timeout);
		ڪنجي = k_spin_تالو(&ڍير_2->تالو);
	}

	k_spin_unlock(&ڍير_2->تالو, ڪنجي);
	أرجع ret;
}

فراغ *k_ڍير_alloc(بنية k_ڍير *ڍير_2, size_t بائيٽون, k_timeout_t timeout)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير, alloc, ڍير_2, timeout);

	فراغ *ret = z_ڍير_alloc_helper(ڍير_2, 0, بائيٽون, timeout,
					sys_ڍير_noalign_alloc);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير, alloc, ڍير_2, timeout, ret);

	أرجع ret;
}

فراغ *k_ڍير_aligned_alloc(بنية k_ڍير *ڍير_2, size_t align, size_t بائيٽون,
			k_timeout_t timeout)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير, aligned_alloc, ڍير_2, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	فراغ *ret = z_ڍير_alloc_helper(ڍير_2, align, بائيٽون, timeout,
					sys_ڍير_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_اڻ_استعمال_ٿيل(blocked_alloc);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير, aligned_alloc, ڍير_2, timeout, ret);

	أرجع ret;
}

فراغ *k_ڍير_calloc(بنية k_ڍير *ڍير_2, size_t انگ, size_t سائيز, k_timeout_t timeout)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير, صفر_يادگيري_ڏيو, ڍير_2, timeout);

	فراغ *ret = عدم;
	size_t bounds = 0U;

	إذا (!سائيز_mul_حد_کان_مٿي(انگ, سائيز, &bounds)) {
		ret = k_ڍير_alloc(ڍير_2, bounds, timeout);
	}
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير, صفر_يادگيري_ڏيو, ڍير_2, timeout, ret);

	أرجع ret;
}

فراغ *k_ڍير_realloc(بنية k_ڍير *ڍير_2, فراغ *ptr, size_t بائيٽون, k_timeout_t timeout)
{
	k_timepoint_t پڇاڙي = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ڍير_2->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير, realloc, ڍير_2, ptr, بائيٽون, timeout);

	__ASSERT(!arch_is_اندريون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	طالما (ret == عدم) {
		ret = sys_ڍير_realloc(&ڍير_2->ڍير_2, ptr, بائيٽون);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		timeout = sys_timepoint_timeout(پڇاڙي);
		(فراغ) z_pend_curr(&ڍير_2->تالو, ڪنجي, &ڍير_2->wait_q, timeout);
		ڪنجي = k_spin_تالو(&ڍير_2->تالو);
	}

	k_spin_unlock(&ڍير_2->تالو, ڪنجي);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير, realloc, ڍير_2, ptr, بائيٽون, timeout, ret);

	أرجع ret;
}

فراغ k_ڍير_free(بنية k_ڍير *ڍير_2, فراغ *mem)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ڍير_2->تالو);

	sys_ڍير_free(&ڍير_2->ڍير_2, mem);

	SYS_پورٽ_TRACING_OBJ_FUNC(k_ڍير, حرر, ڍير_2);
	إذا (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&ڍير_2->wait_q) != 0)) {
		z_reschedule(&ڍير_2->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&ڍير_2->تالو, ڪنجي);
	}
}
