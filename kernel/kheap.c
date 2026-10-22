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

صحيح k_ډېرۍ_array_get(بنية k_ډېرۍ **ډېرۍ)
{
	صحيح عدد;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_ډېرۍ, 0, ډېرۍ);
	/* Number of statically defined heaps */
	STRUCT_SECTION_شمېر(k_ډېرۍ, &عدد);
	أرجع عدد;
}

فراغ k_ډېرۍ_init(بنية k_ډېرۍ *ډېرۍ, فراغ *mem, size_t بایټونه)
{
	z_waitq_init(&ډېرۍ->wait_q);
	ډېرۍ->قلف = (بنية k_spinlock) {};
	sys_ډېرۍ_init(&ډېرۍ->ډېرۍ, mem, بایټونه);

	SYS_پورټ_TRACING_OBJ_INIT(k_ډېرۍ, ډېرۍ);
}

ساكن صحيح statics_init(فراغ)
{
	STRUCT_SECTION_FOREACH(k_ډېرۍ, ډېرۍ) {
		k_ډېرۍ_init(ډېرۍ, ډېرۍ->ډېرۍ.init_mem, ډېرۍ->ډېرۍ.init_بایټونه);
	}
	أرجع 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_لومړیتوب_OBJECTS);

تعريفنوع فراغ * (sys_ډېرۍ_allocator_t)(بنية sys_ډېرۍ *ډېرۍ, size_t align, size_t بایټونه);

ساكن فراغ *z_ډېرۍ_alloc_helper(بنية k_ډېرۍ *ډېرۍ, size_t align, size_t بایټونه,
				 k_timeout_t timeout,
				 sys_ډېرۍ_allocator_t *sys_ډېرۍ_allocator)
{
	k_timepoint_t پای_ټکی = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_کلی_t کلی = k_spin_قلف(&ډېرۍ->قلف);

	__ASSERT(!arch_is_دننه_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	طالما (ret == عدم) {
		ret = sys_ډېرۍ_allocator(&ډېرۍ->ډېرۍ, align, بایټونه);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		إذا (!blocked_alloc) {
			blocked_alloc = true;

			SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_ډېرۍ, alloc_helper, ډېرۍ, timeout);
		} وإلا {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(پای_ټکی);
		(فراغ) z_pend_curr(&ډېرۍ->قلف, کلی, &ډېرۍ->wait_q, timeout);
		کلی = k_spin_قلف(&ډېرۍ->قلف);
	}

	k_spin_unlock(&ډېرۍ->قلف, کلی);
	أرجع ret;
}

فراغ *k_ډېرۍ_alloc(بنية k_ډېرۍ *ډېرۍ, size_t بایټونه, k_timeout_t timeout)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ, alloc, ډېرۍ, timeout);

	فراغ *ret = z_ډېرۍ_alloc_helper(ډېرۍ, 0, بایټونه, timeout,
					sys_ډېرۍ_noalign_alloc);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ, alloc, ډېرۍ, timeout, ret);

	أرجع ret;
}

فراغ *k_ډېرۍ_aligned_alloc(بنية k_ډېرۍ *ډېرۍ, size_t align, size_t بایټونه,
			k_timeout_t timeout)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ, aligned_alloc, ډېرۍ, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	فراغ *ret = z_ډېرۍ_alloc_helper(ډېرۍ, align, بایټونه, timeout,
					sys_ډېرۍ_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_ناکارول_شوی(blocked_alloc);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ, aligned_alloc, ډېرۍ, timeout, ret);

	أرجع ret;
}

فراغ *k_ډېرۍ_calloc(بنية k_ډېرۍ *ډېرۍ, size_t عدد, size_t اندازه, k_timeout_t timeout)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ, حافظه_صفر_ورکړه, ډېرۍ, timeout);

	فراغ *ret = عدم;
	size_t bounds = 0U;

	إذا (!اندازه_mul_ډکېدنه(عدد, اندازه, &bounds)) {
		ret = k_ډېرۍ_alloc(ډېرۍ, bounds, timeout);
	}
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ, حافظه_صفر_ورکړه, ډېرۍ, timeout, ret);

	أرجع ret;
}

فراغ *k_ډېرۍ_realloc(بنية k_ډېرۍ *ډېرۍ, فراغ *ptr, size_t بایټونه, k_timeout_t timeout)
{
	k_timepoint_t پای_ټکی = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_کلی_t کلی = k_spin_قلف(&ډېرۍ->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېرۍ, realloc, ډېرۍ, ptr, بایټونه, timeout);

	__ASSERT(!arch_is_دننه_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	طالما (ret == عدم) {
		ret = sys_ډېرۍ_realloc(&ډېرۍ->ډېرۍ, ptr, بایټونه);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		timeout = sys_timepoint_timeout(پای_ټکی);
		(فراغ) z_pend_curr(&ډېرۍ->قلف, کلی, &ډېرۍ->wait_q, timeout);
		کلی = k_spin_قلف(&ډېرۍ->قلف);
	}

	k_spin_unlock(&ډېرۍ->قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېرۍ, realloc, ډېرۍ, ptr, بایټونه, timeout, ret);

	أرجع ret;
}

فراغ k_ډېرۍ_free(بنية k_ډېرۍ *ډېرۍ, فراغ *mem)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&ډېرۍ->قلف);

	sys_ډېرۍ_free(&ډېرۍ->ډېرۍ, mem);

	SYS_پورټ_TRACING_OBJ_FUNC(k_ډېرۍ, حرر, ډېرۍ);
	إذا (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&ډېرۍ->wait_q) != 0)) {
		z_reschedule(&ډېرۍ->قلف, کلی);
	} وإلا {
		k_spin_unlock(&ډېرۍ->قلف, کلی);
	}
}
