/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/mm/demand_paging.h>

ಬಾಹ್ಯ ಸಂರಚನಾ k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ಸಂರಚನಾ k_mem_paging_histogram_t z_paging_histogram_eviction;
ಸಂರಚನಾ k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
ಸಂರಚನಾ k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

ಬಾಹ್ಯ unsigned long
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS];

ಬಾಹ್ಯ unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS];

#ಅನ್ಯಥಾ
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak unsigned long
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS] = {
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
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS] = {
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

unsigned long k_mem_ಅಂಕೆ_pagefaults_get(ಶೂನ್ಯ)
{
	unsigned long ret;
	unsigned ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ;

	ಕೀಲಿ = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ z_impl_k_mem_paging_stats_get(ಸಂರಚನಾ k_mem_paging_stats_t *stats)
{
	ಯದಿ (stats == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Copy statistics */
	memcpy(stats, &paging_stats, ಪರಿಮಾಣ(paging_stats));
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ
ಶೂನ್ಯ z_vrfy_k_mem_paging_stats_get(ಸಂರಚನಾ k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, ಪರಿಮಾಣ(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
ಶೂನ್ಯ z_impl_k_mem_paging_thread_stats_get(ಸಂರಚನಾ k_thread *thread,
					  ಸಂರಚನಾ k_mem_paging_stats_t *stats)
{
	ಯದಿ ((thread == ರಿಕ್ತ) || (stats == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Copy statistics */
	memcpy(stats, &thread->paging_stats, ಪರಿಮಾಣ(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ
ಶೂನ್ಯ z_vrfy_k_mem_paging_thread_stats_get(ಸಂರಚನಾ k_thread *thread,
					  ಸಂರಚನಾ k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, ಪರಿಮಾಣ(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ಶೂನ್ಯ z_paging_histogram_init(ಶೂನ್ಯ)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, ಪರಿಮಾಣ(z_paging_histogram_eviction));
	memcpy(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       ಪರಿಮಾಣ(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_page_in, 0,
	       ಪರಿಮಾಣ(z_paging_histogram_backing_store_page_in));
	memcpy(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ಪರಿಮಾಣ(z_paging_histogram_backing_store_page_in.bounds));

	memset(&z_paging_histogram_backing_store_page_out, 0,
	       ಪರಿಮಾಣ(z_paging_histogram_backing_store_page_out));
	memcpy(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ಪರಿಮಾಣ(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ಶೂನ್ಯ z_paging_histogram_inc(ಸಂರಚನಾ k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	ಪೂರ್ಣಾಙ್ಕ idx;

	ಪ್ರತಿ (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS;
	     idx++) {
		ಯದಿ (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			ಭಙ್ಗ;
		}
	}
}

ಶೂನ್ಯ z_impl_k_mem_paging_histogram_eviction_get(
	ಸಂರಚನಾ k_mem_paging_histogram_t *hist)
{
	ಯದಿ (hist == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Copy statistics */
	memcpy(hist, &z_paging_histogram_eviction,
	       ಪರಿಮಾಣ(z_paging_histogram_eviction));
}

ಶೂನ್ಯ z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	ಸಂರಚನಾ k_mem_paging_histogram_t *hist)
{
	ಯದಿ (hist == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_in,
	       ಪರಿಮಾಣ(z_paging_histogram_backing_store_page_in));
}

ಶೂನ್ಯ z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	ಸಂರಚನಾ k_mem_paging_histogram_t *hist)
{
	ಯದಿ (hist == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_out,
	       ಪರಿಮಾಣ(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ
ಶೂನ್ಯ z_vrfy_k_mem_paging_histogram_eviction_get(
	ಸಂರಚನಾ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, ಪರಿಮಾಣ(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ
ಶೂನ್ಯ z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	ಸಂರಚನಾ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, ಪರಿಮಾಣ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ
ಶೂನ್ಯ z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	ಸಂರಚನಾ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, ಪರಿಮಾಣ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
