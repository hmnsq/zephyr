/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/kernel/mm/demand_paging.h>

બાહ્ય સંરચના k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
સંરચના k_mem_paging_histogram_t z_paging_histogram_eviction;
સંરચના k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
સંરચના k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

બાહ્ય unsigned long
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_અંક_BINS];

બાહ્ય unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_અંક_BINS];

#નહીંતર
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak unsigned long
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_અંક_BINS] = {
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
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_અંક_BINS] = {
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

unsigned long k_mem_અંક_pagefaults_get(શૂન્ય)
{
	unsigned long ret;
	unsigned પૂર્ણાંક ચાવી;

	ચાવી = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(ચાવી);

	પાછા ret;
}

શૂન્ય z_impl_k_mem_paging_stats_get(સંરચના k_mem_paging_stats_t *stats)
{
	જો (stats == ખાલી) {
		પાછા;
	}

	/* Copy statistics */
	memcpy(stats, &paging_stats, કદ(paging_stats));
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ
શૂન્ય z_vrfy_k_mem_paging_stats_get(સંરચના k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, કદ(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#અન્તર્ભાવ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
શૂન્ય z_impl_k_mem_paging_thread_stats_get(સંરચના k_thread *thread,
					  સંરચના k_mem_paging_stats_t *stats)
{
	જો ((thread == ખાલી) || (stats == ખાલી)) {
		પાછા;
	}

	/* Copy statistics */
	memcpy(stats, &thread->paging_stats, કદ(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ
શૂન્ય z_vrfy_k_mem_paging_thread_stats_get(સંરચના k_thread *thread,
					  સંરચના k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, કદ(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#અન્તર્ભાવ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
શૂન્ય z_paging_histogram_init(શૂન્ય)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, કદ(z_paging_histogram_eviction));
	memcpy(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       કદ(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_page_in, 0,
	       કદ(z_paging_histogram_backing_store_page_in));
	memcpy(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       કદ(z_paging_histogram_backing_store_page_in.bounds));

	memset(&z_paging_histogram_backing_store_page_out, 0,
	       કદ(z_paging_histogram_backing_store_page_out));
	memcpy(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       કદ(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
શૂન્ય z_paging_histogram_inc(સંરચના k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	પૂર્ણાંક idx;

	માટે (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_અંક_BINS;
	     idx++) {
		જો (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			તોડો;
		}
	}
}

શૂન્ય z_impl_k_mem_paging_histogram_eviction_get(
	સંરચના k_mem_paging_histogram_t *hist)
{
	જો (hist == ખાલી) {
		પાછા;
	}

	/* Copy statistics */
	memcpy(hist, &z_paging_histogram_eviction,
	       કદ(z_paging_histogram_eviction));
}

શૂન્ય z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	સંરચના k_mem_paging_histogram_t *hist)
{
	જો (hist == ખાલી) {
		પાછા;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_in,
	       કદ(z_paging_histogram_backing_store_page_in));
}

શૂન્ય z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	સંરચના k_mem_paging_histogram_t *hist)
{
	જો (hist == ખાલી) {
		પાછા;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_out,
	       કદ(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ
શૂન્ય z_vrfy_k_mem_paging_histogram_eviction_get(
	સંરચના k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, કદ(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#અન્તર્ભાવ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

સ્થિર અંતઃરેખ
શૂન્ય z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	સંરચના k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, કદ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#અન્તર્ભાવ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

સ્થિર અંતઃરેખ
શૂન્ય z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	સંરચના k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, કદ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#અન્તર્ભાવ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
