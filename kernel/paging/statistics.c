/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <zephyr/kernel/mm/demand_paging.h>

வெளி கட்டமைப்பு k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
கட்டமைப்பு k_mem_paging_histogram_t z_paging_histogram_eviction;
கட்டமைப்பு k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
கட்டமைப்பு k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

வெளி unsigned long
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_இலக்கம்_BINS];

வெளி unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_இலக்கம்_BINS];

#இல்லையெனில்
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak unsigned long
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_இலக்கம்_BINS] = {
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
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_இலக்கம்_BINS] = {
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

unsigned long k_mem_இலக்கம்_pagefaults_get(வெறுமை)
{
	unsigned long ret;
	unsigned முழு சாவி;

	சாவி = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(சாவி);

	திரும்பு ret;
}

வெறுமை z_impl_k_mem_paging_stats_get(கட்டமைப்பு k_mem_paging_stats_t *stats)
{
	எனில் (stats == வெற்று) {
		திரும்பு;
	}

	/* Copy statistics */
	memcpy(stats, &paging_stats, அளவு(paging_stats));
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி
வெறுமை z_vrfy_k_mem_paging_stats_get(கட்டமைப்பு k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, அளவு(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#அந்தர்பாவ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
வெறுமை z_impl_k_mem_paging_thread_stats_get(கட்டமைப்பு k_thread *thread,
					  கட்டமைப்பு k_mem_paging_stats_t *stats)
{
	எனில் ((thread == வெற்று) || (stats == வெற்று)) {
		திரும்பு;
	}

	/* Copy statistics */
	memcpy(stats, &thread->paging_stats, அளவு(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி
வெறுமை z_vrfy_k_mem_paging_thread_stats_get(கட்டமைப்பு k_thread *thread,
					  கட்டமைப்பு k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, அளவு(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#அந்தர்பாவ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
வெறுமை z_paging_histogram_init(வெறுமை)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, அளவு(z_paging_histogram_eviction));
	memcpy(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       அளவு(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_page_in, 0,
	       அளவு(z_paging_histogram_backing_store_page_in));
	memcpy(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       அளவு(z_paging_histogram_backing_store_page_in.bounds));

	memset(&z_paging_histogram_backing_store_page_out, 0,
	       அளவு(z_paging_histogram_backing_store_page_out));
	memcpy(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       அளவு(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
வெறுமை z_paging_histogram_inc(கட்டமைப்பு k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	முழு idx;

	க்கு (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_இலக்கம்_BINS;
	     idx++) {
		எனில் (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			நிறுத்து;
		}
	}
}

வெறுமை z_impl_k_mem_paging_histogram_eviction_get(
	கட்டமைப்பு k_mem_paging_histogram_t *hist)
{
	எனில் (hist == வெற்று) {
		திரும்பு;
	}

	/* Copy statistics */
	memcpy(hist, &z_paging_histogram_eviction,
	       அளவு(z_paging_histogram_eviction));
}

வெறுமை z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	கட்டமைப்பு k_mem_paging_histogram_t *hist)
{
	எனில் (hist == வெற்று) {
		திரும்பு;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_in,
	       அளவு(z_paging_histogram_backing_store_page_in));
}

வெறுமை z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	கட்டமைப்பு k_mem_paging_histogram_t *hist)
{
	எனில் (hist == வெற்று) {
		திரும்பு;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_out,
	       அளவு(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி
வெறுமை z_vrfy_k_mem_paging_histogram_eviction_get(
	கட்டமைப்பு k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, அளவு(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#அந்தர்பாவ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

நிலையான உள்வரி
வெறுமை z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	கட்டமைப்பு k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, அளவு(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#அந்தர்பாவ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

நிலையான உள்வரி
வெறுமை z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	கட்டமைப்பு k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, அளவு(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#அந்தர்பாவ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
