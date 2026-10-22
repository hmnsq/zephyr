/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/init.h>
#شامل <zephyr/linker/linker-defs.h>
#شامل <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#شامل <ksched.h>
#شامل <wait_q.h>

صحیح k_ڈھیر_array_get(ساخت k_ڈھیر **ڈھیر_2)
{
	صحیح عدد;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_ڈھیر, 0, ڈھیر_2);
	/* Number of statically defined heaps */
	STRUCT_SECTION_گنتی(k_ڈھیر, &عدد);
	واپس عدد;
}

خالی k_ڈھیر_init(ساخت k_ڈھیر *ڈھیر_2, خالی *mem, size_t بائٹیں)
{
	z_waitq_init(&ڈھیر_2->wait_q);
	ڈھیر_2->تالا = (ساخت k_spinlock) {};
	sys_ڈھیر_init(&ڈھیر_2->ڈھیر_2, mem, بائٹیں);

	SYS_پورٹ_TRACING_OBJ_INIT(k_ڈھیر, ڈھیر_2);
}

ساکن صحیح statics_init(خالی)
{
	STRUCT_SECTION_FOREACH(k_ڈھیر, ڈھیر_2) {
		k_ڈھیر_init(ڈھیر_2, ڈھیر_2->ڈھیر_2.init_mem, ڈھیر_2->ڈھیر_2.init_بائٹیں);
	}
	واپس 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_ترجیح_OBJECTS);

قسمتعریف خالی * (sys_ڈھیر_allocator_t)(ساخت sys_ڈھیر *ڈھیر_2, size_t align, size_t بائٹیں);

ساکن خالی *z_ڈھیر_alloc_helper(ساخت k_ڈھیر *ڈھیر_2, size_t align, size_t بائٹیں,
				 k_timeout_t timeout,
				 sys_ڈھیر_allocator_t *sys_ڈھیر_allocator)
{
	k_timepoint_t انتہا = sys_timepoint_calc(timeout);
	خالی *ret = تہی;

	k_spinlock_کلید_t کلید = k_spin_تالا(&ڈھیر_2->تالا);

	__ASSERT(!arch_is_اندر_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	جب (ret == تہی) {
		ret = sys_ڈھیر_allocator(&ڈھیر_2->ڈھیر_2, align, بائٹیں);

		اگر (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != تہی) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			توڑو;
		}

		اگر (!blocked_alloc) {
			blocked_alloc = true;

			SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_ڈھیر, alloc_helper, ڈھیر_2, timeout);
		} ورنہ {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(انتہا);
		(خالی) z_pend_curr(&ڈھیر_2->تالا, کلید, &ڈھیر_2->wait_q, timeout);
		کلید = k_spin_تالا(&ڈھیر_2->تالا);
	}

	k_spin_unlock(&ڈھیر_2->تالا, کلید);
	واپس ret;
}

خالی *k_ڈھیر_alloc(ساخت k_ڈھیر *ڈھیر_2, size_t بائٹیں, k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, alloc, ڈھیر_2, timeout);

	خالی *ret = z_ڈھیر_alloc_helper(ڈھیر_2, 0, بائٹیں, timeout,
					sys_ڈھیر_noalign_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, alloc, ڈھیر_2, timeout, ret);

	واپس ret;
}

خالی *k_ڈھیر_aligned_alloc(ساخت k_ڈھیر *ڈھیر_2, size_t align, size_t بائٹیں,
			k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, aligned_alloc, ڈھیر_2, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	خالی *ret = z_ڈھیر_alloc_helper(ڈھیر_2, align, بائٹیں, timeout,
					sys_ڈھیر_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_غیر_مستعمل(blocked_alloc);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, aligned_alloc, ڈھیر_2, timeout, ret);

	واپس ret;
}

خالی *k_ڈھیر_calloc(ساخت k_ڈھیر *ڈھیر_2, size_t عدد, size_t سائز, k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, صفر_حافظہ_دو, ڈھیر_2, timeout);

	خالی *ret = تہی;
	size_t bounds = 0U;

	اگر (!سائز_mul_حد_سے_تجاوز(عدد, سائز, &bounds)) {
		ret = k_ڈھیر_alloc(ڈھیر_2, bounds, timeout);
	}
	اگر (ret != تہی) {
		(خالی)memset(ret, 0, bounds);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, صفر_حافظہ_دو, ڈھیر_2, timeout, ret);

	واپس ret;
}

خالی *k_ڈھیر_realloc(ساخت k_ڈھیر *ڈھیر_2, خالی *ptr, size_t بائٹیں, k_timeout_t timeout)
{
	k_timepoint_t انتہا = sys_timepoint_calc(timeout);
	خالی *ret = تہی;

	k_spinlock_کلید_t کلید = k_spin_تالا(&ڈھیر_2->تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, realloc, ڈھیر_2, ptr, بائٹیں, timeout);

	__ASSERT(!arch_is_اندر_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	جب (ret == تہی) {
		ret = sys_ڈھیر_realloc(&ڈھیر_2->ڈھیر_2, ptr, بائٹیں);

		اگر (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != تہی) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			توڑو;
		}

		timeout = sys_timepoint_timeout(انتہا);
		(خالی) z_pend_curr(&ڈھیر_2->تالا, کلید, &ڈھیر_2->wait_q, timeout);
		کلید = k_spin_تالا(&ڈھیر_2->تالا);
	}

	k_spin_unlock(&ڈھیر_2->تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, realloc, ڈھیر_2, ptr, بائٹیں, timeout, ret);

	واپس ret;
}

خالی k_ڈھیر_free(ساخت k_ڈھیر *ڈھیر_2, خالی *mem)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&ڈھیر_2->تالا);

	sys_ڈھیر_free(&ڈھیر_2->ڈھیر_2, mem);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_ڈھیر, آزاد, ڈھیر_2);
	اگر (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&ڈھیر_2->wait_q) != 0)) {
		z_reschedule(&ڈھیر_2->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&ڈھیر_2->تالا, کلید);
	}
}
