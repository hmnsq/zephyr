/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/linker/linker-defs.h>
#అన్తర్భావ <zephyr/sys/iterable_sections.h>
/* private kernel APIs */
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <wait_q.h>

పూర్ణాంకం k_కుప్ప_array_get(నిర్మాణం k_కుప్ప **కుప్ప)
{
	పూర్ణాంకం అంకె;

	/* Pointer to the start of the heap array */
	STRUCT_SECTION_GET(k_కుప్ప, 0, కుప్ప);
	/* Number of statically defined heaps */
	STRUCT_SECTION_లెక్క(k_కుప్ప, &అంకె);
	తిరిగి అంకె;
}

శూన్య k_కుప్ప_init(నిర్మాణం k_కుప్ప *కుప్ప, శూన్య *mem, size_t బైట్లు)
{
	z_waitq_init(&కుప్ప->wait_q);
	కుప్ప->తాళం = (నిర్మాణం k_spinlock) {};
	sys_కుప్ప_init(&కుప్ప->కుప్ప, mem, బైట్లు);

	SYS_పోర్టు_TRACING_OBJ_INIT(k_కుప్ప, కుప్ప);
}

స్థిర పూర్ణాంకం statics_init(శూన్య)
{
	STRUCT_SECTION_FOREACH(k_కుప్ప, కుప్ప) {
		k_కుప్ప_init(కుప్ప, కుప్ప->కుప్ప.init_mem, కుప్ప->కుప్ప.init_బైట్లు);
	}
	తిరిగి 0;
}

/*
 * Static heap init must stay a PRE_KERNEL_1 SYS_INIT: the heap KASAN shadow is
 * registered at (PRE_KERNEL_1, 0) via K_HEAP_KASAN_ENABLE() and must run before
 * sys_heap_init(). A K_KERNEL_INIT_PRE() hook would run ahead of that, leaving
 * the shadow unregistered and KASAN tracking disabled.
 */
SYS_INIT_NAMED(statics_init_pre, statics_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_ప్రాధాన్యత_OBJECTS);

రకనిర్వచనం శూన్య * (sys_కుప్ప_allocator_t)(నిర్మాణం sys_కుప్ప *కుప్ప, size_t align, size_t బైట్లు);

స్థిర శూన్య *z_కుప్ప_alloc_helper(నిర్మాణం k_కుప్ప *కుప్ప, size_t align, size_t బైట్లు,
				 k_timeout_t timeout,
				 sys_కుప్ప_allocator_t *sys_కుప్ప_allocator)
{
	k_timepoint_t చివర = sys_timepoint_calc(timeout);
	శూన్య *ret = ఖాళీ;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&కుప్ప->తాళం);

	__ASSERT(!arch_is_లోపల_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	bool blocked_alloc = false;

	అయితే (ret == ఖాళీ) {
		ret = sys_కుప్ప_allocator(&కుప్ప->కుప్ప, align, బైట్లు);

		ఒకవేళ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ఖాళీ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ఆపు;
		}

		ఒకవేళ (!blocked_alloc) {
			blocked_alloc = true;

			SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_కుప్ప, alloc_helper, కుప్ప, timeout);
		} లేకపోతే {
			/**
			 * @todo	Trace attempt to avoid empty trace segments
			 */
		}

		timeout = sys_timepoint_timeout(చివర);
		(శూన్య) z_pend_curr(&కుప్ప->తాళం, కీలకం, &కుప్ప->wait_q, timeout);
		కీలకం = k_spin_తాళం(&కుప్ప->తాళం);
	}

	k_spin_unlock(&కుప్ప->తాళం, కీలకం);
	తిరిగి ret;
}

శూన్య *k_కుప్ప_alloc(నిర్మాణం k_కుప్ప *కుప్ప, size_t బైట్లు, k_timeout_t timeout)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప, alloc, కుప్ప, timeout);

	శూన్య *ret = z_కుప్ప_alloc_helper(కుప్ప, 0, బైట్లు, timeout,
					sys_కుప్ప_noalign_alloc);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప, alloc, కుప్ప, timeout, ret);

	తిరిగి ret;
}

శూన్య *k_కుప్ప_aligned_alloc(నిర్మాణం k_కుప్ప *కుప్ప, size_t align, size_t బైట్లు,
			k_timeout_t timeout)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప, aligned_alloc, కుప్ప, timeout);

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		 "align must be a power of 2");

	శూన్య *ret = z_కుప్ప_alloc_helper(కుప్ప, align, బైట్లు, timeout,
					sys_కుప్ప_aligned_alloc);

	/*
	 * modules/debug/percepio/TraceRecorder/kernelports/Zephyr/include/tracing_tracerecorder.h
	 * contains a concealed non-parameterized direct reference to a local
	 * variable through the SYS_PORT_TRACING_OBJ_FUNC_EXIT macro below
	 * that is no longer in scope. Provide a dummy stub for compilation
	 * to still succeed until that module's layering violation is fixed.
	 */
	bool blocked_alloc = false; ARG_వాడని(blocked_alloc);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప, aligned_alloc, కుప్ప, timeout, ret);

	తిరిగి ret;
}

శూన్య *k_కుప్ప_calloc(నిర్మాణం k_కుప్ప *కుప్ప, size_t అంకె, size_t కొలత, k_timeout_t timeout)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప, సున్నా_జ్ఞాపకం_ఇవ్వు, కుప్ప, timeout);

	శూన్య *ret = ఖాళీ;
	size_t bounds = 0U;

	ఒకవేళ (!కొలత_mul_అధికప్రవాహం(అంకె, కొలత, &bounds)) {
		ret = k_కుప్ప_alloc(కుప్ప, bounds, timeout);
	}
	ఒకవేళ (ret != ఖాళీ) {
		(శూన్య)memset(ret, 0, bounds);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప, సున్నా_జ్ఞాపకం_ఇవ్వు, కుప్ప, timeout, ret);

	తిరిగి ret;
}

శూన్య *k_కుప్ప_realloc(నిర్మాణం k_కుప్ప *కుప్ప, శూన్య *ptr, size_t బైట్లు, k_timeout_t timeout)
{
	k_timepoint_t చివర = sys_timepoint_calc(timeout);
	శూన్య *ret = ఖాళీ;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&కుప్ప->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_కుప్ప, realloc, కుప్ప, ptr, బైట్లు, timeout);

	__ASSERT(!arch_is_లోపల_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	అయితే (ret == ఖాళీ) {
		ret = sys_కుప్ప_realloc(&కుప్ప->కుప్ప, ptr, బైట్లు);

		ఒకవేళ (!IS_ENABLED(CONFIG_MULTITHREADING) ||
		    (ret != ఖాళీ) || K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
			ఆపు;
		}

		timeout = sys_timepoint_timeout(చివర);
		(శూన్య) z_pend_curr(&కుప్ప->తాళం, కీలకం, &కుప్ప->wait_q, timeout);
		కీలకం = k_spin_తాళం(&కుప్ప->తాళం);
	}

	k_spin_unlock(&కుప్ప->తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_కుప్ప, realloc, కుప్ప, ptr, బైట్లు, timeout, ret);

	తిరిగి ret;
}

శూన్య k_కుప్ప_free(నిర్మాణం k_కుప్ప *కుప్ప, శూన్య *mem)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&కుప్ప->తాళం);

	sys_కుప్ప_free(&కుప్ప->కుప్ప, mem);

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_కుప్ప, విడుదల, కుప్ప);
	ఒకవేళ (IS_ENABLED(CONFIG_MULTITHREADING) && (z_unpend_all(&కుప్ప->wait_q) != 0)) {
		z_reschedule(&కుప్ప->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&కుప్ప->తాళం, కీలకం);
	}
}
