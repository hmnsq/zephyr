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

صحيح k_انباشته_array_get(بنية k_انباشته **انباشته)
{
	صحيح رقم;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_انباشته, 0, انباشته);
	/* Number of statically defined heaps */
	STRUCT_SECTION_شمار(k_انباشته, &رقم);
	أرجع رقم;
}

فراغ k_انباشته_init(بنية k_انباشته *انباشته, فراغ *mem, size_t بایتها)
{
	z_waitq_init(&انباشته->wait_q);
	انباشته->قفل = (بنية k_spinlock) {};
	sys_انباشته_init(&انباشته->انباشته, mem, بایتها);

	SYS_پورت_TRACING_OBJ_INIT(k_انباشته, انباشته);
}

ساكن صحيح statics_init(فراغ)
{
	STRUCT_SECTION_FOREACH(k_انباشته, انباشته) {
		k_انباشته_init(انباشته, انباشته->انباشته.init_mem, انباشته->انباشته.init_بایتها);
	}
	أرجع 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_اولویت_OBJECTS);

تعريفنوع فراغ * (sys_انباشته_allocator_t)(بنية sys_انباشته *انباشته, size_t align, size_t بایتها);

ساكن فراغ *z_انباشته_alloc_helper(بنية k_انباشته *انباشته, size_t align, size_t بایتها,
				 k_timeout_t timeout,
				 sys_انباشته_allocator_t *sys_انباشته_allocator)
{
	k_timepoint_t انتها = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_کلید_t کلید = k_spin_قفل(&انباشته->قفل);

	__ASSERT(!arch_is_درون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	طالما (ret == عدم) {
		ret = sys_انباشته_allocator(&انباشته->انباشته, align, بایتها);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		إذا (!blocked_alloc) {
			blocked_alloc = true;

			SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_انباشته, alloc_helper, انباشته, timeout);
		} وإلا {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(انتها);
		(فراغ) z_pend_curr(&انباشته->قفل, کلید, &انباشته->wait_q, timeout);
		کلید = k_spin_قفل(&انباشته->قفل);
	}

	k_spin_unlock(&انباشته->قفل, کلید);
	أرجع ret;
}

فراغ *k_انباشته_alloc(بنية k_انباشته *انباشته, size_t بایتها, k_timeout_t timeout)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته, alloc, انباشته, timeout);

	فراغ *ret = z_انباشته_alloc_helper(انباشته, 0, بایتها, timeout,
					sys_انباشته_noalign_alloc);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته, alloc, انباشته, timeout, ret);

	أرجع ret;
}

فراغ *k_انباشته_aligned_alloc(بنية k_انباشته *انباشته, size_t align, size_t بایتها,
			k_timeout_t timeout)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته, aligned_alloc, انباشته, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	فراغ *ret = z_انباشته_alloc_helper(انباشته, align, بایتها, timeout,
					sys_انباشته_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_استفادهنشده(blocked_alloc);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته, aligned_alloc, انباشته, timeout, ret);

	أرجع ret;
}

فراغ *k_انباشته_calloc(بنية k_انباشته *انباشته, size_t رقم, size_t اندازه, k_timeout_t timeout)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته, حافظه_صفر_بده, انباشته, timeout);

	فراغ *ret = عدم;
	size_t bounds = 0U;

	إذا (!اندازه_mul_سرریز(رقم, اندازه, &bounds)) {
		ret = k_انباشته_alloc(انباشته, bounds, timeout);
	}
	إذا (ret != عدم) {
		(فراغ)memset(ret, 0, bounds);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته, حافظه_صفر_بده, انباشته, timeout, ret);

	أرجع ret;
}

فراغ *k_انباشته_realloc(بنية k_انباشته *انباشته, فراغ *ptr, size_t بایتها, k_timeout_t timeout)
{
	k_timepoint_t انتها = sys_timepoint_calc(timeout);
	فراغ *ret = عدم;

	k_spinlock_کلید_t کلید = k_spin_قفل(&انباشته->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_انباشته, realloc, انباشته, ptr, بایتها, timeout);

	__ASSERT(!arch_is_درون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	طالما (ret == عدم) {
		ret = sys_انباشته_realloc(&انباشته->انباشته, ptr, بایتها);

		إذا (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != عدم) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			اكسر;
		}

		timeout = sys_timepoint_timeout(انتها);
		(فراغ) z_pend_curr(&انباشته->قفل, کلید, &انباشته->wait_q, timeout);
		کلید = k_spin_قفل(&انباشته->قفل);
	}

	k_spin_unlock(&انباشته->قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_انباشته, realloc, انباشته, ptr, بایتها, timeout, ret);

	أرجع ret;
}

فراغ k_انباشته_free(بنية k_انباشته *انباشته, فراغ *mem)
{
	k_spinlock_کلید_t کلید = k_spin_قفل(&انباشته->قفل);

	sys_انباشته_free(&انباشته->انباشته, mem);

	SYS_پورت_TRACING_OBJ_FUNC(k_انباشته, حرر, انباشته);
	إذا (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&انباشته->wait_q) != 0)) {
		z_reschedule(&انباشته->قفل, کلید);
	} وإلا {
		k_spin_unlock(&انباشته->قفل, کلید);
	}
}
