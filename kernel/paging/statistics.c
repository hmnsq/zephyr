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

خارجی ساخت k_mem_paging_stats_t paging_stats;

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
ساخت k_mem_paging_histogram_t z_paging_histogram_eviction;
ساخت k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحہ_اندر;
ساخت k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحہ_برآمد;

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

خارجی غیرعلامتی طویل
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS];

خارجی غیرعلامتی طویل
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS];

#ورنہ
#تعریف_کرو NS_مقصد_CYC(ns)		(CONFIG_SYS_CLOCK_HW_ادوار_فی_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak غیرعلامتی طویل
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS] = {
	NS_مقصد_CYC(1),
	NS_مقصد_CYC(5),
	NS_مقصد_CYC(10),
	NS_مقصد_CYC(50),
	NS_مقصد_CYC(100),
	NS_مقصد_CYC(200),
	NS_مقصد_CYC(500),
	NS_مقصد_CYC(1000),
	NS_مقصد_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak غیرعلامتی طویل
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS] = {
	NS_مقصد_CYC(10),
	NS_مقصد_CYC(100),
	NS_مقصد_CYC(125),
	NS_مقصد_CYC(250),
	NS_مقصد_CYC(500),
	NS_مقصد_CYC(1000),
	NS_مقصد_CYC(2000),
	NS_مقصد_CYC(5000),
	NS_مقصد_CYC(10000),
	ULONG_MAX
};
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

غیرعلامتی طویل k_mem_عدد_pagefaults_get(خالی)
{
	غیرعلامتی طویل ret;
	غیرعلامتی صحیح کلید;

	کلید = irq_تالا();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(کلید);

	واپس ret;
}

خالی z_impl_k_mem_paging_stats_get(ساخت k_mem_paging_stats_t *stats)
{
	اگر (stats == تہی) {
		واپس;
	}

	/* Copy statistics */
	حافظہ_نقل(stats, &paging_stats, حجم(paging_stats));
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی
خالی z_vrfy_k_mem_paging_stats_get(ساخت k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#شامل <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
خالی z_impl_k_mem_paging_thread_stats_get(ساخت k_thread *thread,
					  ساخت k_mem_paging_stats_t *stats)
{
	اگر ((thread == تہی) || (stats == تہی)) {
		واپس;
	}

	/* Copy statistics */
	حافظہ_نقل(stats, &thread->paging_stats, حجم(thread->paging_stats));
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی
خالی z_vrfy_k_mem_paging_thread_stats_get(ساخت k_thread *thread,
					  ساخت k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(stats, حجم(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#شامل <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_ختم /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
خالی z_paging_histogram_init(خالی)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	memset(&z_paging_histogram_eviction, 0, حجم(z_paging_histogram_eviction));
	حافظہ_نقل(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       حجم(z_paging_histogram_eviction.bounds));

	memset(&z_paging_histogram_backing_store_صفحہ_اندر, 0,
	       حجم(z_paging_histogram_backing_store_صفحہ_اندر));
	حافظہ_نقل(z_paging_histogram_backing_store_صفحہ_اندر.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_صفحہ_اندر.bounds));

	memset(&z_paging_histogram_backing_store_صفحہ_برآمد, 0,
	       حجم(z_paging_histogram_backing_store_صفحہ_برآمد));
	حافظہ_نقل(z_paging_histogram_backing_store_صفحہ_برآمد.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       حجم(z_paging_histogram_backing_store_صفحہ_برآمد.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
خالی z_paging_histogram_inc(ساخت k_mem_paging_histogram_t *hist,
			    uint32_t ادوار)
{
	صحیح idx;

	برائے (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_عدد_BINS;
	     idx++) {
		اگر (ادوار <= hist->bounds[idx]) {
			hist->counts[idx]++;
			توڑو;
		}
	}
}

خالی z_impl_k_mem_paging_histogram_eviction_get(
	ساخت k_mem_paging_histogram_t *hist)
{
	اگر (hist == تہی) {
		واپس;
	}

	/* Copy statistics */
	حافظہ_نقل(hist, &z_paging_histogram_eviction,
	       حجم(z_paging_histogram_eviction));
}

خالی z_impl_k_mem_paging_histogram_backing_store_صفحہ_اندر_get(
	ساخت k_mem_paging_histogram_t *hist)
{
	اگر (hist == تہی) {
		واپس;
	}

	/* Copy histogram */
	حافظہ_نقل(hist, &z_paging_histogram_backing_store_صفحہ_اندر,
	       حجم(z_paging_histogram_backing_store_صفحہ_اندر));
}

خالی z_impl_k_mem_paging_histogram_backing_store_صفحہ_برآمد_get(
	ساخت k_mem_paging_histogram_t *hist)
{
	اگر (hist == تہی) {
		واپس;
	}

	/* Copy histogram */
	حافظہ_نقل(hist, &z_paging_histogram_backing_store_صفحہ_برآمد,
	       حجم(z_paging_histogram_backing_store_صفحہ_برآمد));
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی
خالی z_vrfy_k_mem_paging_histogram_eviction_get(
	ساخت k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#شامل <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

ساکن اندرونی
خالی z_vrfy_k_mem_paging_histogram_backing_store_صفحہ_اندر_get(
	ساخت k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_صفحہ_اندر_get(hist);
}
#شامل <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

ساکن اندرونی
خالی z_vrfy_k_mem_paging_histogram_backing_store_صفحہ_برآمد_get(
	ساخت k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_حافظہ_WRITE(hist, حجم(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_صفحہ_برآمد_get(hist);
}
#شامل <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
