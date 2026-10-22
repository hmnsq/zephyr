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

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
بنية k_mem_paging_histogram_t z_paging_histogram_eviction;
بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_ورقہ_وچ;
بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_ورقہ_باہرلا;

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

خارجي بنانشانی لما
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS];

خارجي بنانشانی لما
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS];

#وإلا
#تعریف_کرو NS_منزل_CYC(ns)		(CONFIG_SYS_CLOCK_HW_چکراں_فی_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak بنانشانی لما
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS] = {
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
__weak بنانشانی لما
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS] = {
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
#جے_انت /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

بنانشانی لما k_mem_عدد_pagefaults_get(فراغ)
{
	بنانشانی لما ret;
	بنانشانی صحيح کلید;

	کلید = irq_جندرا();
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
	حافظہ_نقل(stats, &paging_stats, حجم(paging_stats));
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_stats_get(بنية k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#ضمن <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
فراغ z_impl_k_mem_paging_thread_stats_get(بنية k_thread *thread,
					  بنية k_mem_paging_stats_t *stats)
{
	إذا ((thread == عدم) || (stats == عدم)) {
		أرجع;
	}

	/* Copy statistics */
	حافظہ_نقل(stats, &thread->paging_stats, حجم(thread->paging_stats));
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_thread_stats_get(بنية k_thread *thread,
					  بنية k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#ضمن <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

#جے_انت /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#جے_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
فراغ z_paging_histogram_init(فراغ)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, حجم(z_paging_histogram_eviction));
	حافظہ_نقل(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       حجم(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_ورقہ_وچ, 0,
	       حجم(z_paging_histogram_backing_store_ورقہ_وچ));
	حافظہ_نقل(z_paging_histogram_backing_store_ورقہ_وچ.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_ورقہ_وچ.bounds));

	memset(&z_paging_histogram_backing_store_ورقہ_باہرلا, 0,
	       حجم(z_paging_histogram_backing_store_ورقہ_باہرلا));
	حافظہ_نقل(z_paging_histogram_backing_store_ورقہ_باہرلا.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_ورقہ_باہرلا.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
فراغ z_paging_histogram_inc(بنية k_mem_paging_histogram_t *hist,
			    uint32_t چکراں)
{
	صحيح idx;

	لكل (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS;
	     idx++) {
		إذا (چکراں <= hist->bounds[idx]) {
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
	حافظہ_نقل(hist, &z_paging_histogram_eviction,
	       حجم(z_paging_histogram_eviction));
}

فراغ z_impl_k_mem_paging_histogram_backing_store_ورقہ_وچ_get(
	بنية k_mem_paging_histogram_t *hist)
{
	إذا (hist == عدم) {
		أرجع;
	}

	/* Copy histogram */
	حافظہ_نقل(hist, &z_paging_histogram_backing_store_ورقہ_وچ,
	       حجم(z_paging_histogram_backing_store_ورقہ_وچ));
}

فراغ z_impl_k_mem_paging_histogram_backing_store_ورقہ_باہرلا_get(
	بنية k_mem_paging_histogram_t *hist)
{
	إذا (hist == عدم) {
		أرجع;
	}

	/* Copy histogram */
	حافظہ_نقل(hist, &z_paging_histogram_backing_store_ورقہ_باہرلا,
	       حجم(z_paging_histogram_backing_store_ورقہ_باہرلا));
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_eviction_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_backing_store_ورقہ_وچ_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_ورقہ_وچ_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ساكن مضمن
فراغ z_vrfy_k_mem_paging_histogram_backing_store_ورقہ_باہرلا_get(
	بنية k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_ورقہ_باہرلا_get(hist);
}
#ضمن <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

#جے_انت /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
