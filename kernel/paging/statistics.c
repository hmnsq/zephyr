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

#جيڪڏهن_وصف_ٿيل CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
بنية k_mem_paging_histogram_t z_paging_histogram_eviction;
بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحو_اندريون;
بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحو_ٻاهريون;

#جيڪڏهن_وصف_ٿيل CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

خارجي بنانشان ڊگهو
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_انگ_BINS];

خارجي بنانشان ڊگهو
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_انگ_BINS];

#وإلا
#وصف_ڪريو NS_منزل_CYC(ns)		(CONFIG_SYS_CLOCK_HW_چڪرن_في_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak بنانشان ڊگهو
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_انگ_BINS] = {
	NS_منزل_CYC(1),
	NS_منزل_CYC(5),
	NS_منزل_CYC(10),
	NS_منزل_CYC(50),
	NS_منزل_CYC(100),
	NS_منزل_CYC(200),
	NS_منزل_CYC(500),
	NS_منزل_CYC(1000),
	NS_منزل_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak بنانشان ڊگهو
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_انگ_BINS] = {
	NS_منزل_CYC(10),
	NS_منزل_CYC(100),
	NS_منزل_CYC(125),
	NS_منزل_CYC(250),
	NS_منزل_CYC(500),
	NS_منزل_CYC(1000),
	NS_منزل_CYC(2000),
	NS_منزل_CYC(5000),
	NS_منزل_CYC(10000),
	ULONG_MAX
};
#جيڪڏهن_پورو /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#جيڪڏهن_پورو /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

بنانشان ڊگهو k_mem_انگ_pagefaults_get(فراغ)
{
	بنانشان ڊگهو ret;
	بنانشان صحيح ڪنجي;

	ڪنجي = irq_تالو();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(ڪنجي);

	أرجع ret;
}

فراغ z_impl_k_mem_paging_stats_get(بنية k_mem_paging_stats_t *stats)
{
	إذا (stats == عدم) {
		أرجع;
	}

	/* Copy statistics */
	يادگيري_نقل(stats, &paging_stats, حجم(paging_stats));
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_stats_get(بنية k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_يادگيري_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#ضمن <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_وصف_ٿيل CONFIG_DEMAND_PAGING_THREAD_STATS
فراغ z_impl_k_mem_paging_thread_stats_get(بنية k_thread *thread,
					  بنية k_mem_paging_stats_t *stats)
{
	إذا ((thread == عدم) || (stats == عدم)) {
		أرجع;
	}

	/* Copy statistics */
	يادگيري_نقل(stats, &thread->paging_stats, حجم(thread->paging_stats));
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_thread_stats_get(بنية k_thread *thread,
					  بنية k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#ضمن <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_پورو /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#جيڪڏهن_وصف_ٿيل CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
فراغ z_paging_histogram_init(فراغ)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, حجم(z_paging_histogram_eviction));
	يادگيري_نقل(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       حجم(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_صفحو_اندريون, 0,
	       حجم(z_paging_histogram_backing_store_صفحو_اندريون));
	يادگيري_نقل(z_paging_histogram_backing_store_صفحو_اندريون.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_صفحو_اندريون.bounds));

	memset(&z_paging_histogram_backing_store_صفحو_ٻاهريون, 0,
	       حجم(z_paging_histogram_backing_store_صفحو_ٻاهريون));
	يادگيري_نقل(z_paging_histogram_backing_store_صفحو_ٻاهريون.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_صفحو_ٻاهريون.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
فراغ z_paging_histogram_inc(بنية k_mem_paging_histogram_t *hist,
			    uint32_t چڪرن)
{
	صحيح idx;

	لكل (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_انگ_BINS;
	     idx++) {
		إذا (چڪرن <= hist->bounds[idx]) {
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
	يادگيري_نقل(hist, &z_paging_histogram_eviction,
	       حجم(z_paging_histogram_eviction));
}

فراغ z_impl_k_mem_paging_histogram_backing_store_صفحو_اندريون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	إذا (hist == عدم) {
		أرجع;
	}

	/* Copy histogram */
	يادگيري_نقل(hist, &z_paging_histogram_backing_store_صفحو_اندريون,
	       حجم(z_paging_histogram_backing_store_صفحو_اندريون));
}

فراغ z_impl_k_mem_paging_histogram_backing_store_صفحو_ٻاهريون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	إذا (hist == عدم) {
		أرجع;
	}

	/* Copy histogram */
	يادگيري_نقل(hist, &z_paging_histogram_backing_store_صفحو_ٻاهريون,
	       حجم(z_paging_histogram_backing_store_صفحو_ٻاهريون));
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_eviction_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_يادگيري_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_backing_store_صفحو_اندريون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_يادگيري_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_صفحو_اندريون_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_backing_store_صفحو_ٻاهريون_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_يادگيري_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_صفحو_ٻاهريون_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_پورو /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
