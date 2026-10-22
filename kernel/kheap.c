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

صحيح k_ڈھیر_array_get(بنية k_ڈھیر **ڈھیر_2)
{
	صحيح عدد;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_ڈھیر, 0, ڈھیر_2);
	/* Number of statically defined heaps */
	STRUCT_SECTION_گنتی(k_ڈھیر, &عدد);
	أرجع عدد;
}

فراغ k_ڈھیر_init(بنية k_ڈھیر *ڈھیر_2, فراغ *mem, size_t بائٹاں)
{
	z_waitq_init(&ڈھیر_2->wait_q);
	ڈھیر_2->جندرا = (بنية k_spinlock) {};
	sys_ڈھیر_init(&ڈھیر_2->ڈھیر_2, mem, بائٹاں);

	SYS_پورٹ_TRACING_OBJ_INIT(k_ڈھیر, ڈھیر_2);
}

ساكن صحيح statics_init(فراغ)
{
	STRUCT_SECTION_FOREACH(k_ڈھیر, ڈھیر_2) {
		k_ڈھیر_init(ڈھیر_2, ڈھیر_2->ڈھیر_2.init_mem, ڈھیر_2->ڈھیر_2.init_بائٹاں);
	}
	أرجع 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_ترجیح_OBJECTS);

تعريفنوع فراغ * (sys_ڈھیر_allocator_t)(بنية sys_ڈھیر *ڈھیر_2, size_t align, size_t بائٹاں);

ساكن فراغ *z_ڈھیر_alloc_helper(بنية k_ڈھیر *ڈھیر_2, size_t align, size_t بائٹاں,
				 k_timeout_t timeout,
				 sys_ڈھیر_allocator_t *sys_ڈھیر_allocator)
{
	k_timepoint_t سرا = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&ڈھیر_2->جندرا);

	__ASSERT(!arch_is_وچ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	طالما (ret == عدم) {
		ret = sys_ڈھیر_allocator(&ڈھیر_2->ڈھیر_2, align, بائٹاں);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		إذا (!blocked_alloc) {
			blocked_alloc = true;

			SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_ڈھیر, alloc_helper, ڈھیر_2, timeout);
		} وإلا {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(سرا);
		(فراغ) z_pend_curr(&ڈھیر_2->جندرا, کلید, &ڈھیر_2->wait_q, timeout);
		کلید = k_spin_جندرا(&ڈھیر_2->جندرا);
	}

	k_spin_unlock(&ڈھیر_2->جندرا, کلید);
	أرجع ret;
}

فراغ *k_ڈھیر_alloc(بنية k_ڈھیر *ڈھیر_2, size_t بائٹاں, k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, alloc, ڈھیر_2, timeout);

	فراغ *ret = z_ڈھیر_alloc_helper(ڈھیر_2, 0, بائٹاں, timeout,
					sys_ڈھیر_noalign_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, alloc, ڈھیر_2, timeout, ret);

	أرجع ret;
}

فراغ *k_ڈھیر_aligned_alloc(بنية k_ڈھیر *ڈھیر_2, size_t align, size_t بائٹاں,
			k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, aligned_alloc, ڈھیر_2, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	فراغ *ret = z_ڈھیر_alloc_helper(ڈھیر_2, align, بائٹاں, timeout,
					sys_ڈھیر_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_بن_ورتیا(blocked_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, aligned_alloc, ڈھیر_2, timeout, ret);

	أرجع ret;
}

فراغ *k_ڈھیر_calloc(بنية k_ڈھیر *ڈھیر_2, size_t عدد, size_t ناپ, k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, صفر_حافظہ_دیو, ڈھیر_2, timeout);

	فراغ *ret = عدم;
	size_t bounds = 0U;

	إذا (!ناپ_mul_حد_توں_ودھ(عدد, ناپ, &bounds)) {
		ret = k_ڈھیر_alloc(ڈھیر_2, bounds, timeout);
	}
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, صفر_حافظہ_دیو, ڈھیر_2, timeout, ret);

	أرجع ret;
}

فراغ *k_ڈھیر_realloc(بنية k_ڈھیر *ڈھیر_2, فراغ *ptr, size_t بائٹاں, k_timeout_t timeout)
{
	k_timepoint_t سرا = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&ڈھیر_2->جندرا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, realloc, ڈھیر_2, ptr, بائٹاں, timeout);

	__ASSERT(!arch_is_وچ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	طالما (ret == عدم) {
		ret = sys_ڈھیر_realloc(&ڈھیر_2->ڈھیر_2, ptr, بائٹاں);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		timeout = sys_timepoint_timeout(سرا);
		(فراغ) z_pend_curr(&ڈھیر_2->جندرا, کلید, &ڈھیر_2->wait_q, timeout);
		کلید = k_spin_جندرا(&ڈھیر_2->جندرا);
	}

	k_spin_unlock(&ڈھیر_2->جندرا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, realloc, ڈھیر_2, ptr, بائٹاں, timeout, ret);

	أرجع ret;
}

فراغ k_ڈھیر_free(بنية k_ڈھیر *ڈھیر_2, فراغ *mem)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&ڈھیر_2->جندرا);

	sys_ڈھیر_free(&ڈھیر_2->ڈھیر_2, mem);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_ڈھیر, حرر, ڈھیر_2);
	إذا (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&ڈھیر_2->wait_q) != 0)) {
		z_reschedule(&ڈھیر_2->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&ڈھیر_2->جندرا, کلید);
	}
}
