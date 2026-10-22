/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/kernel/mm/demand_paging.h>

బాహ్య నిర్మాణం k_mem_paging_stats_t paging_stats;

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
నిర్మాణం k_mem_paging_histogram_t z_paging_histogram_eviction;
నిర్మాణం k_mem_paging_histogram_t z_paging_histogram_backing_store_పుట_లోపల;
నిర్మాణం k_mem_paging_histogram_t z_paging_histogram_backing_store_పుట_బహిర్గతం;

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

బాహ్య గుర్తులేని దీర్ఘ
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_అంకె_BINS];

బాహ్య గుర్తులేని దీర్ఘ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_అంకె_BINS];

#లేకపోతే
#నిర్వచించు NS_గమ్యం_CYC(ns)		(CONFIG_SYS_CLOCK_HW_చక్రాలు_చొప్పున_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak గుర్తులేని దీర్ఘ
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_అంకె_BINS] = {
	NS_గమ్యం_CYC(1),
	NS_గమ్యం_CYC(5),
	NS_గమ్యం_CYC(10),
	NS_గమ్యం_CYC(50),
	NS_గమ్యం_CYC(100),
	NS_గమ్యం_CYC(200),
	NS_గమ్యం_CYC(500),
	NS_గమ్యం_CYC(1000),
	NS_గమ్యం_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak గుర్తులేని దీర్ఘ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_అంకె_BINS] = {
	NS_గమ్యం_CYC(10),
	NS_గమ్యం_CYC(100),
	NS_గమ్యం_CYC(125),
	NS_గమ్యం_CYC(250),
	NS_గమ్యం_CYC(500),
	NS_గమ్యం_CYC(1000),
	NS_గమ్యం_CYC(2000),
	NS_గమ్యం_CYC(5000),
	NS_గమ్యం_CYC(10000),
	ULONG_MAX
};
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

గుర్తులేని దీర్ఘ k_mem_అంకె_pagefaults_get(శూన్య)
{
	గుర్తులేని దీర్ఘ ret;
	గుర్తులేని పూర్ణాంకం కీలకం;

	కీలకం = irq_తాళం();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(కీలకం);

	తిరిగి ret;
}

శూన్య z_impl_k_mem_paging_stats_get(నిర్మాణం k_mem_paging_stats_t *stats)
{
	ఒకవేళ (stats == ఖాళీ) {
		తిరిగి;
	}

	/* Copy statistics */
	జ్ఞాపకం_నకలు(stats, &paging_stats, పరిమాణం(paging_stats));
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస
శూన్య z_vrfy_k_mem_paging_stats_get(నిర్మాణం k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(stats, పరిమాణం(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#అన్తర్భావ <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_THREAD_STATS
శూన్య z_impl_k_mem_paging_thread_stats_get(నిర్మాణం k_thread *thread,
					  నిర్మాణం k_mem_paging_stats_t *stats)
{
	ఒకవేళ ((thread == ఖాళీ) || (stats == ఖాళీ)) {
		తిరిగి;
	}

	/* Copy statistics */
	జ్ఞాపకం_నకలు(stats, &thread->paging_stats, పరిమాణం(thread->paging_stats));
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస
శూన్య z_vrfy_k_mem_paging_thread_stats_get(నిర్మాణం k_thread *thread,
					  నిర్మాణం k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(stats, పరిమాణం(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#అన్తర్భావ <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
శూన్య z_paging_histogram_init(శూన్య)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, పరిమాణం(z_paging_histogram_eviction));
	జ్ఞాపకం_నకలు(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       పరిమాణం(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_పుట_లోపల, 0,
	       పరిమాణం(z_paging_histogram_backing_store_పుట_లోపల));
	జ్ఞాపకం_నకలు(z_paging_histogram_backing_store_పుట_లోపల.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       పరిమాణం(z_paging_histogram_backing_store_పుట_లోపల.bounds));

	memset(&z_paging_histogram_backing_store_పుట_బహిర్గతం, 0,
	       పరిమాణం(z_paging_histogram_backing_store_పుట_బహిర్గతం));
	జ్ఞాపకం_నకలు(z_paging_histogram_backing_store_పుట_బహిర్గతం.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       పరిమాణం(z_paging_histogram_backing_store_పుట_బహిర్గతం.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
శూన్య z_paging_histogram_inc(నిర్మాణం k_mem_paging_histogram_t *hist,
			    uint32_t చక్రాలు)
{
	పూర్ణాంకం idx;

	కొరకు (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_అంకె_BINS;
	     idx++) {
		ఒకవేళ (చక్రాలు <= hist->bounds[idx]) {
			hist->counts[idx]++;
			ఆపు;
		}
	}
}

శూన్య z_impl_k_mem_paging_histogram_eviction_get(
	నిర్మాణం k_mem_paging_histogram_t *hist)
{
	ఒకవేళ (hist == ఖాళీ) {
		తిరిగి;
	}

	/* Copy statistics */
	జ్ఞాపకం_నకలు(hist, &z_paging_histogram_eviction,
	       పరిమాణం(z_paging_histogram_eviction));
}

శూన్య z_impl_k_mem_paging_histogram_backing_store_పుట_లోపల_get(
	నిర్మాణం k_mem_paging_histogram_t *hist)
{
	ఒకవేళ (hist == ఖాళీ) {
		తిరిగి;
	}

	/* Copy histogram */
	జ్ఞాపకం_నకలు(hist, &z_paging_histogram_backing_store_పుట_లోపల,
	       పరిమాణం(z_paging_histogram_backing_store_పుట_లోపల));
}

శూన్య z_impl_k_mem_paging_histogram_backing_store_పుట_బహిర్గతం_get(
	నిర్మాణం k_mem_paging_histogram_t *hist)
{
	ఒకవేళ (hist == ఖాళీ) {
		తిరిగి;
	}

	/* Copy histogram */
	జ్ఞాపకం_నకలు(hist, &z_paging_histogram_backing_store_పుట_బహిర్గతం,
	       పరిమాణం(z_paging_histogram_backing_store_పుట_బహిర్గతం));
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస
శూన్య z_vrfy_k_mem_paging_histogram_eviction_get(
	నిర్మాణం k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(hist, పరిమాణం(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#అన్తర్భావ <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

స్థిర అంతఃవరుస
శూన్య z_vrfy_k_mem_paging_histogram_backing_store_పుట_లోపల_get(
	నిర్మాణం k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(hist, పరిమాణం(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_పుట_లోపల_get(hist);
}
#అన్తర్భావ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

స్థిర అంతఃవరుస
శూన్య z_vrfy_k_mem_paging_histogram_backing_store_పుట_బహిర్గతం_get(
	నిర్మాణం k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(hist, పరిమాణం(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_పుట_బహిర్గతం_get(hist);
}
#అన్తర్భావ <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
