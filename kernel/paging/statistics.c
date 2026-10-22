/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <zephyr/kernel/mm/demand_paging.h>

বাহ্যিক কাঠামো k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
কাঠামো k_mem_paging_histogram_t z_paging_histogram_eviction;
কাঠামো k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
কাঠামো k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

বাহ্যিক unsigned long
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_অঙ্ক_BINS];

বাহ্যিক unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_অঙ্ক_BINS];

#অন্যথা
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak unsigned long
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_অঙ্ক_BINS] = {
	NS_TO_CYC(1),
	NS_TO_CYC(5),
	NS_TO_CYC(10),
	NS_TO_CYC(50),
	NS_TO_CYC(100),
	NS_TO_CYC(200),
	NS_TO_CYC(500),
	NS_TO_CYC(1000),
	NS_TO_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_অঙ্ক_BINS] = {
	NS_TO_CYC(10),
	NS_TO_CYC(100),
	NS_TO_CYC(125),
	NS_TO_CYC(250),
	NS_TO_CYC(500),
	NS_TO_CYC(1000),
	NS_TO_CYC(2000),
	NS_TO_CYC(5000),
	NS_TO_CYC(10000),
	ULONG_MAX
};
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

unsigned long k_mem_অঙ্ক_pagefaults_get(শূন্য)
{
	unsigned long ret;
	unsigned পূর্ণসংখ্যা চাবি;

	চাবি = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(চাবি);

	ফেরত ret;
}

শূন্য z_impl_k_mem_paging_stats_get(কাঠামো k_mem_paging_stats_t *stats)
{
	যদি (stats == ফাঁকা) {
		ফেরত;
	}

	/* Copy statistics */
	memcpy(stats, &paging_stats, আকার(paging_stats));
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ
শূন্য z_vrfy_k_mem_paging_stats_get(কাঠামো k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, আকার(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#অন্তর্ভাব <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
শূন্য z_impl_k_mem_paging_thread_stats_get(কাঠামো k_thread *thread,
					  কাঠামো k_mem_paging_stats_t *stats)
{
	যদি ((thread == ফাঁকা) || (stats == ফাঁকা)) {
		ফেরত;
	}

	/* Copy statistics */
	memcpy(stats, &thread->paging_stats, আকার(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ
শূন্য z_vrfy_k_mem_paging_thread_stats_get(কাঠামো k_thread *thread,
					  কাঠামো k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, আকার(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#অন্তর্ভাব <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
শূন্য z_paging_histogram_init(শূন্য)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, আকার(z_paging_histogram_eviction));
	memcpy(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       আকার(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_page_in, 0,
	       আকার(z_paging_histogram_backing_store_page_in));
	memcpy(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       আকার(z_paging_histogram_backing_store_page_in.bounds));

	memset(&z_paging_histogram_backing_store_page_out, 0,
	       আকার(z_paging_histogram_backing_store_page_out));
	memcpy(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       আকার(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
শূন্য z_paging_histogram_inc(কাঠামো k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	পূর্ণসংখ্যা idx;

	জন্য (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_অঙ্ক_BINS;
	     idx++) {
		যদি (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			বিরতি;
		}
	}
}

শূন্য z_impl_k_mem_paging_histogram_eviction_get(
	কাঠামো k_mem_paging_histogram_t *hist)
{
	যদি (hist == ফাঁকা) {
		ফেরত;
	}

	/* Copy statistics */
	memcpy(hist, &z_paging_histogram_eviction,
	       আকার(z_paging_histogram_eviction));
}

শূন্য z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	কাঠামো k_mem_paging_histogram_t *hist)
{
	যদি (hist == ফাঁকা) {
		ফেরত;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_in,
	       আকার(z_paging_histogram_backing_store_page_in));
}

শূন্য z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	কাঠামো k_mem_paging_histogram_t *hist)
{
	যদি (hist == ফাঁকা) {
		ফেরত;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_out,
	       আকার(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ
শূন্য z_vrfy_k_mem_paging_histogram_eviction_get(
	কাঠামো k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, আকার(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#অন্তর্ভাব <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

স্থির অন্তঃরেখ
শূন্য z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	কাঠামো k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, আকার(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#অন্তর্ভাব <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

স্থির অন্তঃরেখ
শূন্য z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	কাঠামো k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, আকার(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#অন্তর্ভাব <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
