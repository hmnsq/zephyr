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

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
संरचना k_mem_paging_histogram_t z_paging_histogram_eviction;
संरचना k_mem_paging_histogram_t z_paging_histogram_backing_store_page_अन्तर्;
संरचना k_mem_paging_histogram_t z_paging_histogram_backing_store_page_बहिर्गमनम्;

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

बाह्य अचिह्नित दीर्घ
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्कः_BINS];

बाह्य अचिह्नित दीर्घ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्कः_BINS];

#अन्यथा
#परिभाषय NS_गन्तव्यम्_CYC(ns)		(CONFIG_SYS_CLOCK_HW_आवृत्तयः_प्रत्येकम्_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak अचिह्नित दीर्घ
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्कः_BINS] = {
	NS_गन्तव्यम्_CYC(1),
	NS_गन्तव्यम्_CYC(5),
	NS_गन्तव्यम्_CYC(10),
	NS_गन्तव्यम्_CYC(50),
	NS_गन्तव्यम्_CYC(100),
	NS_गन्तव्यम्_CYC(200),
	NS_गन्तव्यम्_CYC(500),
	NS_गन्तव्यम्_CYC(1000),
	NS_गन्तव्यम्_CYC(2000),
	ULONG_MAX
};

/*
 * This provides the upper bounds of the bins in backing store timing histogram
 * (both page-in and page-out).
 */
__weak अचिह्नित दीर्घ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्कः_BINS] = {
	NS_गन्तव्यम्_CYC(10),
	NS_गन्तव्यम्_CYC(100),
	NS_गन्तव्यम्_CYC(125),
	NS_गन्तव्यम्_CYC(250),
	NS_गन्तव्यम्_CYC(500),
	NS_गन्तव्यम्_CYC(1000),
	NS_गन्तव्यम्_CYC(2000),
	NS_गन्तव्यम्_CYC(5000),
	NS_गन्तव्यम्_CYC(10000),
	ULONG_MAX
};
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

अचिह्नित दीर्घ k_mem_अङ्कः_pagefaults_get(शून्य)
{
	अचिह्नित दीर्घ ret;
	अचिह्नित पूर्णाङ्क कुञ्चिका;

	कुञ्चिका = irq_तालकम्();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(कुञ्चिका);

	प्रत्यागम ret;
}

शून्य z_impl_k_mem_paging_stats_get(संरचना k_mem_paging_stats_t *stats)
{
	यदि (stats == रिक्त) {
		प्रत्यागम;
	}

	/* Copy statistics */
	स्मृतिं_प्रतिलिख(stats, &paging_stats, परिमाण(paging_stats));
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_stats_get(संरचना k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(stats, परिमाण(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_THREAD_STATS
शून्य z_impl_k_mem_paging_thread_stats_get(संरचना k_thread *thread,
					  संरचना k_mem_paging_stats_t *stats)
{
	यदि ((thread == रिक्त) || (stats == रिक्त)) {
		प्रत्यागम;
	}

	/* Copy statistics */
	स्मृतिं_प्रतिलिख(stats, &thread->paging_stats, परिमाण(thread->paging_stats));
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_thread_stats_get(संरचना k_thread *thread,
					  संरचना k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(stats, परिमाण(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_अन्तः /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
शून्य z_paging_histogram_init(शून्य)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	स्मृतिं_पूरय(&z_paging_histogram_eviction, 0, परिमाण(z_paging_histogram_eviction));
	स्मृतिं_प्रतिलिख(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       परिमाण(z_paging_histogram_eviction.bounds));

	स्मृतिं_पूरय(&z_paging_histogram_backing_store_page_अन्तर्, 0,
	       परिमाण(z_paging_histogram_backing_store_page_अन्तर्));
	स्मृतिं_प्रतिलिख(z_paging_histogram_backing_store_page_अन्तर्.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       परिमाण(z_paging_histogram_backing_store_page_अन्तर्.bounds));

	स्मृतिं_पूरय(&z_paging_histogram_backing_store_page_बहिर्गमनम्, 0,
	       परिमाण(z_paging_histogram_backing_store_page_बहिर्गमनम्));
	स्मृतिं_प्रतिलिख(z_paging_histogram_backing_store_page_बहिर्गमनम्.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       परिमाण(z_paging_histogram_backing_store_page_बहिर्गमनम्.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
शून्य z_paging_histogram_inc(संरचना k_mem_paging_histogram_t *hist,
			    uint32_t आवृत्तयः)
{
	पूर्णाङ्क idx;

	प्रति (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अङ्कः_BINS;
	     idx++) {
		यदि (आवृत्तयः <= hist->bounds[idx]) {
			hist->counts[idx]++;
			भङ्ग;
		}
	}
}

शून्य z_impl_k_mem_paging_histogram_eviction_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	यदि (hist == रिक्त) {
		प्रत्यागम;
	}

	/* Copy statistics */
	स्मृतिं_प्रतिलिख(hist, &z_paging_histogram_eviction,
	       परिमाण(z_paging_histogram_eviction));
}

शून्य z_impl_k_mem_paging_histogram_backing_store_page_अन्तर्_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	यदि (hist == रिक्त) {
		प्रत्यागम;
	}

	/* Copy histogram */
	स्मृतिं_प्रतिलिख(hist, &z_paging_histogram_backing_store_page_अन्तर्,
	       परिमाण(z_paging_histogram_backing_store_page_अन्तर्));
}

शून्य z_impl_k_mem_paging_histogram_backing_store_page_बहिर्गमनम्_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	यदि (hist == रिक्त) {
		प्रत्यागम;
	}

	/* Copy histogram */
	स्मृतिं_प्रतिलिख(hist, &z_paging_histogram_backing_store_page_बहिर्गमनम्,
	       परिमाण(z_paging_histogram_backing_store_page_बहिर्गमनम्));
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_histogram_eviction_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(hist, परिमाण(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_histogram_backing_store_page_अन्तर्_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(hist, परिमाण(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_अन्तर्_get(hist);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

स्थिर अन्तःस्थ
शून्य z_vrfy_k_mem_paging_histogram_backing_store_page_बहिर्गमनम्_get(
	संरचना k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(hist, परिमाण(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_बहिर्गमनम्_get(hist);
}
#अन्तर्भाव <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
