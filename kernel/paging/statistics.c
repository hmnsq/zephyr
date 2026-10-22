/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/kernel/mm/demand_paging.h>

خارجي بنية k_mem_paging_stats_t paging_stats;

#اگر_تعریف_شده CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
بنية k_mem_paging_histogram_t z_paging_histogram_eviction;
بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحه_درون;
بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحه_بیرون;

#اگر_تعریف_شده CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

خارجي بیعلامت دراز
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS];

خارجي بیعلامت دراز
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS];

#وإلا
#تعریف_کن NS_مقصد_CYC(ns)		(CONFIG_SYS_CLOCK_HW_چرخهها_در_هر_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak بیعلامت دراز
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS] = {
	NS_مقصد_CYC(1),
	NS_مقصد_CYC(5),
	NS_مقصد_CYC(10),
	NS_مقصد_CYC(50),
	NS_مقصد_CYC(100),
	NS_مقصد_CYC(200),
	NS_مقصد_CYC(500),
	NS_مقصد_CYC(1000),
	NS_مقصد_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak بیعلامت دراز
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS] = {
	NS_مقصد_CYC(10),
	NS_مقصد_CYC(100),
	NS_مقصد_CYC(125),
	NS_مقصد_CYC(250),
	NS_مقصد_CYC(500),
	NS_مقصد_CYC(1000),
	NS_مقصد_CYC(2000),
	NS_مقصد_CYC(5000),
	NS_مقصد_CYC(10000),
	ULONG_MAX
};
#پایان_اگر /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#پایان_اگر /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

بیعلامت دراز k_mem_رقم_pagefaults_get(فراغ)
{
	بیعلامت دراز ret;
	بیعلامت صحيح کلید;

	کلید = irq_قفل();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(کلید);

	أرجع ret;
}

فراغ z_impl_k_mem_paging_stats_get(بنية k_mem_paging_stats_t *stats)
{
	إذا (stats == عدم) {
		أرجع;
	}

	/* Copy statistics */
	کپی_حافظه(stats, &paging_stats, حجم(paging_stats));
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_stats_get(بنية k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_حافظه_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#ضمن <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

#اگر_تعریف_شده CONFIG_DEMAND_PAGING_THREAD_STATS
فراغ z_impl_k_mem_paging_thread_stats_get(بنية k_thread *thread,
					  بنية k_mem_paging_stats_t *stats)
{
	إذا ((thread == عدم) || (stats == عدم)) {
		أرجع;
	}

	/* Copy statistics */
	کپی_حافظه(stats, &thread->paging_stats, حجم(thread->paging_stats));
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_thread_stats_get(بنية k_thread *thread,
					  بنية k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_حافظه_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#ضمن <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

#پایان_اگر /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#اگر_تعریف_شده CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
فراغ z_paging_histogram_init(فراغ)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, حجم(z_paging_histogram_eviction));
	کپی_حافظه(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       حجم(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_صفحه_درون, 0,
	       حجم(z_paging_histogram_backing_store_صفحه_درون));
	کپی_حافظه(z_paging_histogram_backing_store_صفحه_درون.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_صفحه_درون.bounds));

	memset(&z_paging_histogram_backing_store_صفحه_بیرون, 0,
	       حجم(z_paging_histogram_backing_store_صفحه_بیرون));
	کپی_حافظه(z_paging_histogram_backing_store_صفحه_بیرون.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_صفحه_بیرون.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
فراغ z_paging_histogram_inc(بنية k_mem_paging_histogram_t *hist,
			    uint32_t چرخهها)
{
	صحيح idx;

	لكل (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS;
	     idx++) {
		إذا (چرخهها <= hist->bounds[idx]) {
			hist->counts[idx]++;
			اكسر;
		}
	}
}

فراغ z_impl_k_mem_paging_histogram_eviction_get(
	بنية k_mem_paging_histogram_t *hist)
{
	إذا (hist == عدم) {
		أرجع;
	}

	/* Copy statistics */
	کپی_حافظه(hist, &z_paging_histogram_eviction,
	       حجم(z_paging_histogram_eviction));
}

فراغ z_impl_k_mem_paging_histogram_backing_store_صفحه_درون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	إذا (hist == عدم) {
		أرجع;
	}

	/* Copy histogram */
	کپی_حافظه(hist, &z_paging_histogram_backing_store_صفحه_درون,
	       حجم(z_paging_histogram_backing_store_صفحه_درون));
}

فراغ z_impl_k_mem_paging_histogram_backing_store_صفحه_بیرون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	إذا (hist == عدم) {
		أرجع;
	}

	/* Copy histogram */
	کپی_حافظه(hist, &z_paging_histogram_backing_store_صفحه_بیرون,
	       حجم(z_paging_histogram_backing_store_صفحه_بیرون));
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_eviction_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظه_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_backing_store_صفحه_درون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظه_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_صفحه_درون_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_backing_store_صفحه_بیرون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظه_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_صفحه_بیرون_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

#پایان_اگر /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
