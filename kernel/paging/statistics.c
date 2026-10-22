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

ಬಾಹ್ಯ ರಚನೆ k_mem_paging_stats_t paging_stats;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ರಚನೆ k_mem_paging_histogram_t z_paging_histogram_eviction;
ರಚನೆ k_mem_paging_histogram_t z_paging_histogram_backing_store_ಪುಟ_ಒಳ;
ರಚನೆ k_mem_paging_histogram_t z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

ಬಾಹ್ಯ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS];

ಬಾಹ್ಯ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS];

#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು NS_ಗಮ್ಯ_CYC(ns)		(CONFIG_SYS_CLOCK_HW_ಚಕ್ರಗಳು_ಪ್ರತಿ_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS] = {
	NS_ಗಮ್ಯ_CYC(1),
	NS_ಗಮ್ಯ_CYC(5),
	NS_ಗಮ್ಯ_CYC(10),
	NS_ಗಮ್ಯ_CYC(50),
	NS_ಗಮ್ಯ_CYC(100),
	NS_ಗಮ್ಯ_CYC(200),
	NS_ಗಮ್ಯ_CYC(500),
	NS_ಗಮ್ಯ_CYC(1000),
	NS_ಗಮ್ಯ_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS] = {
	NS_ಗಮ್ಯ_CYC(10),
	NS_ಗಮ್ಯ_CYC(100),
	NS_ಗಮ್ಯ_CYC(125),
	NS_ಗಮ್ಯ_CYC(250),
	NS_ಗಮ್ಯ_CYC(500),
	NS_ಗಮ್ಯ_CYC(1000),
	NS_ಗಮ್ಯ_CYC(2000),
	NS_ಗಮ್ಯ_CYC(5000),
	NS_ಗಮ್ಯ_CYC(10000),
	ULONG_MAX
};
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ k_mem_ಅಂಕೆ_pagefaults_get(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ret;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ;

	ಕೀಲಿ = irq_ಬೀಗ();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ z_impl_k_mem_paging_stats_get(ರಚನೆ k_mem_paging_stats_t *stats)
{
	ಆದರೆ (stats == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	/* Copy statistics */
	ಸ್ಮರಣೆ_ನಕಲು(stats, &paging_stats, ಗಾತ್ರ(paging_stats));
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ
ಶೂನ್ಯ z_vrfy_k_mem_paging_stats_get(ರಚನೆ k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(stats, ಗಾತ್ರ(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_THREAD_STATS
ಶೂನ್ಯ z_impl_k_mem_paging_thread_stats_get(ರಚನೆ k_thread *thread,
					  ರಚನೆ k_mem_paging_stats_t *stats)
{
	ಆದರೆ ((thread == ಖಾಲಿ) || (stats == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ;
	}

	/* Copy statistics */
	ಸ್ಮರಣೆ_ನಕಲು(stats, &thread->paging_stats, ಗಾತ್ರ(thread->paging_stats));
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ
ಶೂನ್ಯ z_vrfy_k_mem_paging_thread_stats_get(ರಚನೆ k_thread *thread,
					  ರಚನೆ k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(stats, ಗಾತ್ರ(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ಶೂನ್ಯ z_paging_histogram_init(ಶೂನ್ಯ)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, ಗಾತ್ರ(z_paging_histogram_eviction));
	ಸ್ಮರಣೆ_ನಕಲು(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       ಗಾತ್ರ(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_ಪುಟ_ಒಳ, 0,
	       ಗಾತ್ರ(z_paging_histogram_backing_store_ಪುಟ_ಒಳ));
	ಸ್ಮರಣೆ_ನಕಲು(z_paging_histogram_backing_store_ಪುಟ_ಒಳ.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ಗಾತ್ರ(z_paging_histogram_backing_store_ಪುಟ_ಒಳ.bounds));

	memset(&z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು, 0,
	       ಗಾತ್ರ(z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು));
	ಸ್ಮರಣೆ_ನಕಲು(z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       ಗಾತ್ರ(z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ಶೂನ್ಯ z_paging_histogram_inc(ರಚನೆ k_mem_paging_histogram_t *hist,
			    uint32_t ಚಕ್ರಗಳು)
{
	ಪೂರ್ಣಾಂಕ idx;

	ಗಾಗಿ (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_ಅಂಕೆ_BINS;
	     idx++) {
		ಆದರೆ (ಚಕ್ರಗಳು <= hist->bounds[idx]) {
			hist->counts[idx]++;
			ನಿಲ್ಲಿಸು;
		}
	}
}

ಶೂನ್ಯ z_impl_k_mem_paging_histogram_eviction_get(
	ರಚನೆ k_mem_paging_histogram_t *hist)
{
	ಆದರೆ (hist == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	/* Copy statistics */
	ಸ್ಮರಣೆ_ನಕಲು(hist, &z_paging_histogram_eviction,
	       ಗಾತ್ರ(z_paging_histogram_eviction));
}

ಶೂನ್ಯ z_impl_k_mem_paging_histogram_backing_store_ಪುಟ_ಒಳ_get(
	ರಚನೆ k_mem_paging_histogram_t *hist)
{
	ಆದರೆ (hist == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	/* Copy histogram */
	ಸ್ಮರಣೆ_ನಕಲು(hist, &z_paging_histogram_backing_store_ಪುಟ_ಒಳ,
	       ಗಾತ್ರ(z_paging_histogram_backing_store_ಪುಟ_ಒಳ));
}

ಶೂನ್ಯ z_impl_k_mem_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು_get(
	ರಚನೆ k_mem_paging_histogram_t *hist)
{
	ಆದರೆ (hist == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	/* Copy histogram */
	ಸ್ಮರಣೆ_ನಕಲು(hist, &z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು,
	       ಗಾತ್ರ(z_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು));
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ
ಶೂನ್ಯ z_vrfy_k_mem_paging_histogram_eviction_get(
	ರಚನೆ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(hist, ಗಾತ್ರ(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ
ಶೂನ್ಯ z_vrfy_k_mem_paging_histogram_backing_store_ಪುಟ_ಒಳ_get(
	ರಚನೆ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(hist, ಗಾತ್ರ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_ಪುಟ_ಒಳ_get(hist);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ
ಶೂನ್ಯ z_vrfy_k_mem_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು_get(
	ರಚನೆ k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(hist, ಗಾತ್ರ(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_ಪುಟ_ಹೊರಹೋಗು_get(hist);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
