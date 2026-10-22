/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel/mm/demand_paging.h>

ਬਾਹਰੀ ਬਣਤਰ k_mem_paging_stats_t paging_stats;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ਬਣਤਰ k_mem_paging_histogram_t z_paging_histogram_eviction;
ਬਣਤਰ k_mem_paging_histogram_t z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ;
ਬਣਤਰ k_mem_paging_histogram_t z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ਅੰਕ_BINS];

ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ਅੰਕ_BINS];

#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NS_ਮੰਜ਼ਿਲ_CYC(ns)		(CONFIG_SYS_CLOCK_HW_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ਅੰਕ_BINS] = {
	NS_ਮੰਜ਼ਿਲ_CYC(1),
	NS_ਮੰਜ਼ਿਲ_CYC(5),
	NS_ਮੰਜ਼ਿਲ_CYC(10),
	NS_ਮੰਜ਼ਿਲ_CYC(50),
	NS_ਮੰਜ਼ਿਲ_CYC(100),
	NS_ਮੰਜ਼ਿਲ_CYC(200),
	NS_ਮੰਜ਼ਿਲ_CYC(500),
	NS_ਮੰਜ਼ਿਲ_CYC(1000),
	NS_ਮੰਜ਼ਿਲ_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ਅੰਕ_BINS] = {
	NS_ਮੰਜ਼ਿਲ_CYC(10),
	NS_ਮੰਜ਼ਿਲ_CYC(100),
	NS_ਮੰਜ਼ਿਲ_CYC(125),
	NS_ਮੰਜ਼ਿਲ_CYC(250),
	NS_ਮੰਜ਼ਿਲ_CYC(500),
	NS_ਮੰਜ਼ਿਲ_CYC(1000),
	NS_ਮੰਜ਼ਿਲ_CYC(2000),
	NS_ਮੰਜ਼ਿਲ_CYC(5000),
	NS_ਮੰਜ਼ਿਲ_CYC(10000),
	ULONG_MAX
};
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ k_mem_ਅੰਕ_pagefaults_get(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ret;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ;

	ਕੁੰਜੀ = irq_ਤਾਲਾ();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(ਕੁੰਜੀ);

	ਵਾਪਸ ret;
}

ਖਾਲੀ z_impl_k_mem_paging_stats_get(ਬਣਤਰ k_mem_paging_stats_t *stats)
{
	ਜੇ (stats == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	/* Copy statistics */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stats, &paging_stats, ਆਕਾਰ(paging_stats));
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ
ਖਾਲੀ z_vrfy_k_mem_paging_stats_get(ਬਣਤਰ k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(stats, ਆਕਾਰ(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_THREAD_STATS
ਖਾਲੀ z_impl_k_mem_paging_thread_stats_get(ਬਣਤਰ k_thread *thread,
					  ਬਣਤਰ k_mem_paging_stats_t *stats)
{
	ਜੇ ((thread == ਨਿਰਾ) || (stats == ਨਿਰਾ)) {
		ਵਾਪਸ;
	}

	/* Copy statistics */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stats, &thread->paging_stats, ਆਕਾਰ(thread->paging_stats));
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ
ਖਾਲੀ z_vrfy_k_mem_paging_thread_stats_get(ਬਣਤਰ k_thread *thread,
					  ਬਣਤਰ k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(stats, ਆਕਾਰ(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ਖਾਲੀ z_paging_histogram_init(ਖਾਲੀ)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, ਆਕਾਰ(z_paging_histogram_eviction));
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       ਆਕਾਰ(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ, 0,
	       ਆਕਾਰ(z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ));
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ਆਕਾਰ(z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ.bounds));

	memset(&z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ, 0,
	       ਆਕਾਰ(z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ));
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ਆਕਾਰ(z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ਖਾਲੀ z_paging_histogram_inc(ਬਣਤਰ k_mem_paging_histogram_t *hist,
			    uint32_t ਚੱਕਰਾਂ)
{
	ਪੂਰਨ idx;

	ਲਈ (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ਅੰਕ_BINS;
	     idx++) {
		ਜੇ (ਚੱਕਰਾਂ <= hist->bounds[idx]) {
			hist->counts[idx]++;
			ਰੋਕੋ;
		}
	}
}

ਖਾਲੀ z_impl_k_mem_paging_histogram_eviction_get(
	ਬਣਤਰ k_mem_paging_histogram_t *hist)
{
	ਜੇ (hist == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	/* Copy statistics */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(hist, &z_paging_histogram_eviction,
	       ਆਕਾਰ(z_paging_histogram_eviction));
}

ਖਾਲੀ z_impl_k_mem_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ_get(
	ਬਣਤਰ k_mem_paging_histogram_t *hist)
{
	ਜੇ (hist == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	/* Copy histogram */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(hist, &z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ,
	       ਆਕਾਰ(z_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ));
}

ਖਾਲੀ z_impl_k_mem_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ_get(
	ਬਣਤਰ k_mem_paging_histogram_t *hist)
{
	ਜੇ (hist == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	/* Copy histogram */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(hist, &z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ,
	       ਆਕਾਰ(z_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ));
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ
ਖਾਲੀ z_vrfy_k_mem_paging_histogram_eviction_get(
	ਬਣਤਰ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(hist, ਆਕਾਰ(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ
ਖਾਲੀ z_vrfy_k_mem_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ_get(
	ਬਣਤਰ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(hist, ਆਕਾਰ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_ਪੰਨਾ_ਵਿੱਚ_get(hist);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ
ਖਾਲੀ z_vrfy_k_mem_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ_get(
	ਬਣਤਰ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(hist, ਆਕਾਰ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_ਪੰਨਾ_ਬਾਹਰਲਾ_get(hist);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
