/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <zephyr/kernel/mm/demand_paging.h>

ബാഹ്യ ഘടന k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ഘടന k_mem_paging_histogram_t z_paging_histogram_eviction;
ഘടന k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
ഘടന k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

ബാഹ്യ unsigned long
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_അക്കം_BINS];

ബാഹ്യ unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_അക്കം_BINS];

#അല്ലെങ്കിൽ
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak unsigned long
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_അക്കം_BINS] = {
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
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_അക്കം_BINS] = {
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

unsigned long k_mem_അക്കം_pagefaults_get(ശൂന്യം)
{
	unsigned long ret;
	unsigned പൂർണ്ണസംഖ്യ താക്കോൽ;

	താക്കോൽ = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(താക്കോൽ);

	തിരികെ ret;
}

ശൂന്യം z_impl_k_mem_paging_stats_get(ഘടന k_mem_paging_stats_t *stats)
{
	എങ്കിൽ (stats == ശൂന്യ) {
		തിരികെ;
	}

	/* Copy statistics */
	memcpy(stats, &paging_stats, വലുപ്പം(paging_stats));
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ
ശൂന്യം z_vrfy_k_mem_paging_stats_get(ഘടന k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, വലുപ്പം(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
ശൂന്യം z_impl_k_mem_paging_thread_stats_get(ഘടന k_thread *thread,
					  ഘടന k_mem_paging_stats_t *stats)
{
	എങ്കിൽ ((thread == ശൂന്യ) || (stats == ശൂന്യ)) {
		തിരികെ;
	}

	/* Copy statistics */
	memcpy(stats, &thread->paging_stats, വലുപ്പം(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ
ശൂന്യം z_vrfy_k_mem_paging_thread_stats_get(ഘടന k_thread *thread,
					  ഘടന k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, വലുപ്പം(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ശൂന്യം z_paging_histogram_init(ശൂന്യം)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, വലുപ്പം(z_paging_histogram_eviction));
	memcpy(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       വലുപ്പം(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_page_in, 0,
	       വലുപ്പം(z_paging_histogram_backing_store_page_in));
	memcpy(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       വലുപ്പം(z_paging_histogram_backing_store_page_in.bounds));

	memset(&z_paging_histogram_backing_store_page_out, 0,
	       വലുപ്പം(z_paging_histogram_backing_store_page_out));
	memcpy(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       വലുപ്പം(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ശൂന്യം z_paging_histogram_inc(ഘടന k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	പൂർണ്ണസംഖ്യ idx;

	വേണ്ടി (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_അക്കം_BINS;
	     idx++) {
		എങ്കിൽ (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			നിർത്തുക;
		}
	}
}

ശൂന്യം z_impl_k_mem_paging_histogram_eviction_get(
	ഘടന k_mem_paging_histogram_t *hist)
{
	എങ്കിൽ (hist == ശൂന്യ) {
		തിരികെ;
	}

	/* Copy statistics */
	memcpy(hist, &z_paging_histogram_eviction,
	       വലുപ്പം(z_paging_histogram_eviction));
}

ശൂന്യം z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	ഘടന k_mem_paging_histogram_t *hist)
{
	എങ്കിൽ (hist == ശൂന്യ) {
		തിരികെ;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_in,
	       വലുപ്പം(z_paging_histogram_backing_store_page_in));
}

ശൂന്യം z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	ഘടന k_mem_paging_histogram_t *hist)
{
	എങ്കിൽ (hist == ശൂന്യ) {
		തിരികെ;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_out,
	       വലുപ്പം(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ
ശൂന്യം z_vrfy_k_mem_paging_histogram_eviction_get(
	ഘടന k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, വലുപ്പം(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

സ്ഥിതം വരിയിൽ
ശൂന്യം z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	ഘടന k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, വലുപ്പം(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

സ്ഥിതം വരിയിൽ
ശൂന്യം z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	ഘടന k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, വലുപ്പം(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
