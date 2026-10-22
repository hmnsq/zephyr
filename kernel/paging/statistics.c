/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <kernel_internal.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/toolchain.h>
#شامل <zephyr/kernel/mm/demand_paging.h>

بیرونی ساختار k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ساختار k_mem_paging_histogram_t z_paging_histogram_eviction;
ساختار k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
ساختار k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

بیرونی unsigned long
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS];

بیرونی unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS];

#وگرنه
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak unsigned long
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS] = {
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
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS] = {
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

unsigned long k_mem_رقم_pagefaults_get(تهی)
{
	unsigned long ret;
	unsigned صحیح کلید;

	کلید = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(کلید);

	بازگشت ret;
}

تهی z_impl_k_mem_paging_stats_get(ساختار k_mem_paging_stats_t *stats)
{
	اگر (stats == پوچ) {
		بازگشت;
	}

	/* Copy statistics */
	memcpy(stats, &paging_stats, اندازه(paging_stats));
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی
تهی z_vrfy_k_mem_paging_stats_get(ساختار k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, اندازه(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#شامل <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
تهی z_impl_k_mem_paging_thread_stats_get(ساختار k_thread *thread,
					  ساختار k_mem_paging_stats_t *stats)
{
	اگر ((thread == پوچ) || (stats == پوچ)) {
		بازگشت;
	}

	/* Copy statistics */
	memcpy(stats, &thread->paging_stats, اندازه(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی
تهی z_vrfy_k_mem_paging_thread_stats_get(ساختار k_thread *thread,
					  ساختار k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, اندازه(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#شامل <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
تهی z_paging_histogram_init(تهی)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, اندازه(z_paging_histogram_eviction));
	memcpy(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       اندازه(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_page_in, 0,
	       اندازه(z_paging_histogram_backing_store_page_in));
	memcpy(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       اندازه(z_paging_histogram_backing_store_page_in.bounds));

	memset(&z_paging_histogram_backing_store_page_out, 0,
	       اندازه(z_paging_histogram_backing_store_page_out));
	memcpy(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       اندازه(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
تهی z_paging_histogram_inc(ساختار k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	صحیح idx;

	برای (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_رقم_BINS;
	     idx++) {
		اگر (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			بشکن;
		}
	}
}

تهی z_impl_k_mem_paging_histogram_eviction_get(
	ساختار k_mem_paging_histogram_t *hist)
{
	اگر (hist == پوچ) {
		بازگشت;
	}

	/* Copy statistics */
	memcpy(hist, &z_paging_histogram_eviction,
	       اندازه(z_paging_histogram_eviction));
}

تهی z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	ساختار k_mem_paging_histogram_t *hist)
{
	اگر (hist == پوچ) {
		بازگشت;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_in,
	       اندازه(z_paging_histogram_backing_store_page_in));
}

تهی z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	ساختار k_mem_paging_histogram_t *hist)
{
	اگر (hist == پوچ) {
		بازگشت;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_out,
	       اندازه(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی
تهی z_vrfy_k_mem_paging_histogram_eviction_get(
	ساختار k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, اندازه(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#شامل <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ایستا درونخطی
تهی z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	ساختار k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, اندازه(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#شامل <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ایستا درونخطی
تهی z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	ساختار k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, اندازه(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#شامل <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
