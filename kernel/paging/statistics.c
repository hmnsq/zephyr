/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel/mm/demand_paging.h>

ବାହ୍ୟ ସଂରଚନା k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ସଂରଚନା k_mem_paging_histogram_t z_paging_histogram_eviction;
ସଂରଚନା k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
ସଂରଚନା k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

ବାହ୍ୟ unsigned long
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ଅଙ୍କ_BINS];

ବାହ୍ୟ unsigned long
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ଅଙ୍କ_BINS];

#ଅନ୍ୟଥା
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak unsigned long
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ଅଙ୍କ_BINS] = {
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
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ଅଙ୍କ_BINS] = {
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

unsigned long k_mem_ଅଙ୍କ_pagefaults_get(ଶୂନ୍ୟ)
{
	unsigned long ret;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି;

	ଚାବି = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(ଚାବି);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ z_impl_k_mem_paging_stats_get(ସଂରଚନା k_mem_paging_stats_t *stats)
{
	ଯଦି (stats == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	/* Copy statistics */
	memcpy(stats, &paging_stats, ଆକାର(paging_stats));
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ
ଶୂନ୍ୟ z_vrfy_k_mem_paging_stats_get(ସଂରଚନା k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, ଆକାର(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
ଶୂନ୍ୟ z_impl_k_mem_paging_thread_stats_get(ସଂରଚନା k_thread *thread,
					  ସଂରଚନା k_mem_paging_stats_t *stats)
{
	ଯଦି ((thread == ଖାଲି) || (stats == ଖାଲି)) {
		ଫେରନ୍ତୁ;
	}

	/* Copy statistics */
	memcpy(stats, &thread->paging_stats, ଆକାର(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ
ଶୂନ୍ୟ z_vrfy_k_mem_paging_thread_stats_get(ସଂରଚନା k_thread *thread,
					  ସଂରଚନା k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, ଆକାର(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ଶୂନ୍ୟ z_paging_histogram_init(ଶୂନ୍ୟ)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, ଆକାର(z_paging_histogram_eviction));
	memcpy(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       ଆକାର(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_page_in, 0,
	       ଆକାର(z_paging_histogram_backing_store_page_in));
	memcpy(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ଆକାର(z_paging_histogram_backing_store_page_in.bounds));

	memset(&z_paging_histogram_backing_store_page_out, 0,
	       ଆକାର(z_paging_histogram_backing_store_page_out));
	memcpy(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ଆକାର(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ଶୂନ୍ୟ z_paging_histogram_inc(ସଂରଚନା k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;

	ପାଇଁ (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ଅଙ୍କ_BINS;
	     idx++) {
		ଯଦି (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			ଭାଙ୍ଗ;
		}
	}
}

ଶୂନ୍ୟ z_impl_k_mem_paging_histogram_eviction_get(
	ସଂରଚନା k_mem_paging_histogram_t *hist)
{
	ଯଦି (hist == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	/* Copy statistics */
	memcpy(hist, &z_paging_histogram_eviction,
	       ଆକାର(z_paging_histogram_eviction));
}

ଶୂନ୍ୟ z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	ସଂରଚନା k_mem_paging_histogram_t *hist)
{
	ଯଦି (hist == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_in,
	       ଆକାର(z_paging_histogram_backing_store_page_in));
}

ଶୂନ୍ୟ z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	ସଂରଚନା k_mem_paging_histogram_t *hist)
{
	ଯଦି (hist == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	/* Copy histogram */
	memcpy(hist, &z_paging_histogram_backing_store_page_out,
	       ଆକାର(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ
ଶୂନ୍ୟ z_vrfy_k_mem_paging_histogram_eviction_get(
	ସଂରଚନା k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, ଆକାର(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ
ଶୂନ୍ୟ z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	ସଂରଚନା k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, ଆକାର(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ
ଶୂନ୍ୟ z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	ସଂରଚନା k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, ଆକାର(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
