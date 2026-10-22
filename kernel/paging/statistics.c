/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <zephyr/kernel/mm/demand_paging.h>

बाह्य संरचना k_mem_paging_stats_t paging_stats;

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
संरचना k_mem_paging_histogram_t z_paging_histogram_eviction;
संरचना k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
संरचना k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#ifdef CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

बाह्य अचिन्हित दीर्घ
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्क_BINS];

बाह्य अचिन्हित दीर्घ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्क_BINS];

#अन्यथा
#define NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak अचिन्हित दीर्घ
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्क_BINS] = {
	NS_TO_CYC(1),
	NS_TO_CYC(5),
	NS_TO_CYC(10),
	NS_TO_CYC(50),
	NS_TO_CYC(100),
	NS_TO_CYC(200),
	NS_TO_CYC(500),
	NS_TO_CYC(1000),
	NS_TO_CYC(2000),
	अच_दीर्घ_उच्च
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak अचिन्हित दीर्घ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्क_BINS] = {
	NS_TO_CYC(10),
	NS_TO_CYC(100),
	NS_TO_CYC(125),
	NS_TO_CYC(250),
	NS_TO_CYC(500),
	NS_TO_CYC(1000),
	NS_TO_CYC(2000),
	NS_TO_CYC(5000),
	NS_TO_CYC(10000),
	अच_दीर्घ_उच्च
};
#endif /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

अचिन्हित दीर्घ k_mem_अङ्क_pagefaults_get(शून्य)
{
	अचिन्हित दीर्घ ret;
	अचिन्हित पूर्णाङ्क साँचो;

	साँचो = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(साँचो);

	फर्काऊ ret;
}

शून्य z_impl_k_mem_paging_stats_get(संरचना k_mem_paging_stats_t *stats)
{
	यदि (stats == रिक्त) {
		फर्काऊ;
	}

	/* Copy statistics */
	स_नकल(stats, &paging_stats, परिमाण(paging_stats));
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_stats_get(संरचना k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, परिमाण(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEMAND_PAGING_THREAD_STATS
शून्य z_impl_k_mem_paging_thread_stats_get(संरचना k_thread *thread,
					  संरचना k_mem_paging_stats_t *stats)
{
	यदि ((thread == रिक्त) || (stats == रिक्त)) {
		फर्काऊ;
	}

	/* Copy statistics */
	स_नकल(stats, &thread->paging_stats, परिमाण(thread->paging_stats));
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_thread_stats_get(संरचना k_thread *thread,
					  संरचना k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, परिमाण(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
शून्य z_paging_histogram_init(शून्य)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	स_रखो(&z_paging_histogram_eviction, 0, परिमाण(z_paging_histogram_eviction));
	स_नकल(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       परिमाण(z_paging_histogram_eviction.bounds));

	स_रखो(&z_paging_histogram_backing_store_page_in, 0,
	       परिमाण(z_paging_histogram_backing_store_page_in));
	स_नकल(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       परिमाण(z_paging_histogram_backing_store_page_in.bounds));

	स_रखो(&z_paging_histogram_backing_store_page_out, 0,
	       परिमाण(z_paging_histogram_backing_store_page_out));
	स_नकल(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       परिमाण(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
शून्य z_paging_histogram_inc(संरचना k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	पूर्णाङ्क idx;

	लागि (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्क_BINS;
	     idx++) {
		यदि (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			रोक;
		}
	}
}

शून्य z_impl_k_mem_paging_histogram_eviction_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	यदि (hist == रिक्त) {
		फर्काऊ;
	}

	/* Copy statistics */
	स_नकल(hist, &z_paging_histogram_eviction,
	       परिमाण(z_paging_histogram_eviction));
}

शून्य z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	यदि (hist == रिक्त) {
		फर्काऊ;
	}

	/* Copy histogram */
	स_नकल(hist, &z_paging_histogram_backing_store_page_in,
	       परिमाण(z_paging_histogram_backing_store_page_in));
}

शून्य z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	यदि (hist == रिक्त) {
		फर्काऊ;
	}

	/* Copy histogram */
	स_नकल(hist, &z_paging_histogram_backing_store_page_out,
	       परिमाण(z_paging_histogram_backing_store_page_out));
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_histogram_eviction_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, परिमाण(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, परिमाण(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, परिमाण(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
