/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <kernel_internal.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/kernel/mm/demand_paging.h>

बाह्य काष्ठा k_mem_paging_stats_t paging_stats;

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
काष्ठा k_mem_paging_histogram_t z_paging_histogram_eviction;
काष्ठा k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
काष्ठा k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;

#अगर_घोषित CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS

/*
 * The frequency of timing functions is highly dependent on
 * architecture, SoC or board. It is also not available at build time.
 * Therefore, the bounds for the timing histograms needs to be defined
 * externally to this file, and must be tailored to the platform
 * being used.
 */

बाह्य अचिन्हित दीर्घ
k_mem_paging_eviction_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अंक_BINS];

बाह्य अचिन्हित दीर्घ
k_mem_paging_backing_store_histogram_bounds[
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अंक_BINS];

#अन्यथा
#घोषणा NS_TO_CYC(ns)		(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000U * ns)

/*
 * This provides the upper bounds of the bins in eviction timing histogram.
 */
__weak अचिन्हित दीर्घ
k_mem_paging_eviction_histogram_bounds[CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अंक_BINS] = {
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
	CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अंक_BINS] = {
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
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

अचिन्हित दीर्घ k_mem_अंक_pagefaults_get(व्योम)
{
	अचिन्हित दीर्घ ret;
	अचिन्हित पूर्णांक कुंजी;

	कुंजी = irq_lock();
	ret = paging_stats.pagefaults.cnt;
	irq_unlock(कुंजी);

	वापस ret;
}

व्योम z_impl_k_mem_paging_stats_get(काष्ठा k_mem_paging_stats_t *stats)
{
	अगर (stats == शून्य) {
		वापस;
	}

	/* Copy statistics */
	स_नकल(stats, &paging_stats, माप(paging_stats));
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत
व्योम z_vrfy_k_mem_paging_stats_get(काष्ठा k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, माप(*stats)));
	z_impl_k_mem_paging_stats_get(stats);
}
#समावेश <zephyr/syscalls/k_mem_paging_stats_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

#अगर_घोषित CONFIG_DEMAND_PAGING_THREAD_STATS
व्योम z_impl_k_mem_paging_thread_stats_get(काष्ठा k_thread *thread,
					  काष्ठा k_mem_paging_stats_t *stats)
{
	अगर ((thread == शून्य) || (stats == शून्य)) {
		वापस;
	}

	/* Copy statistics */
	स_नकल(stats, &thread->paging_stats, माप(thread->paging_stats));
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत
व्योम z_vrfy_k_mem_paging_thread_stats_get(काष्ठा k_thread *thread,
					  काष्ठा k_mem_paging_stats_t *stats)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(stats, माप(*stats)));
	z_impl_k_mem_paging_thread_stats_get(thread, stats);
}
#समावेश <zephyr/syscalls/k_mem_paging_thread_stats_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
व्योम z_paging_histogram_init(व्योम)
{
	/*
	 * Zero out the histogram structs and copy the bounds.
	 * The copying is done as the histogram structs need
	 * to be pinned in memory and never swapped out, while
	 * the source bound array may not be pinned.
	 */

	स_रखो(&z_paging_histogram_eviction, 0, माप(z_paging_histogram_eviction));
	स_नकल(z_paging_histogram_eviction.bounds,
	       k_mem_paging_eviction_histogram_bounds,
	       माप(z_paging_histogram_eviction.bounds));

	स_रखो(&z_paging_histogram_backing_store_page_in, 0,
	       माप(z_paging_histogram_backing_store_page_in));
	स_नकल(z_paging_histogram_backing_store_page_in.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       माप(z_paging_histogram_backing_store_page_in.bounds));

	स_रखो(&z_paging_histogram_backing_store_page_out, 0,
	       माप(z_paging_histogram_backing_store_page_out));
	स_नकल(z_paging_histogram_backing_store_page_out.bounds,
	       k_mem_paging_backing_store_histogram_bounds,
	       माप(z_paging_histogram_backing_store_page_out.bounds));
}

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
व्योम z_paging_histogram_inc(काष्ठा k_mem_paging_histogram_t *hist,
			    uint32_t cycles)
{
	पूर्णांक idx;

	क्रम (idx = 0;
	     idx < CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM_अंक_BINS;
	     idx++) {
		अगर (cycles <= hist->bounds[idx]) {
			hist->counts[idx]++;
			अवरोध;
		}
	}
}

व्योम z_impl_k_mem_paging_histogram_eviction_get(
	काष्ठा k_mem_paging_histogram_t *hist)
{
	अगर (hist == शून्य) {
		वापस;
	}

	/* Copy statistics */
	स_नकल(hist, &z_paging_histogram_eviction,
	       माप(z_paging_histogram_eviction));
}

व्योम z_impl_k_mem_paging_histogram_backing_store_page_in_get(
	काष्ठा k_mem_paging_histogram_t *hist)
{
	अगर (hist == शून्य) {
		वापस;
	}

	/* Copy histogram */
	स_नकल(hist, &z_paging_histogram_backing_store_page_in,
	       माप(z_paging_histogram_backing_store_page_in));
}

व्योम z_impl_k_mem_paging_histogram_backing_store_page_out_get(
	काष्ठा k_mem_paging_histogram_t *hist)
{
	अगर (hist == शून्य) {
		वापस;
	}

	/* Copy histogram */
	स_नकल(hist, &z_paging_histogram_backing_store_page_out,
	       माप(z_paging_histogram_backing_store_page_out));
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत
व्योम z_vrfy_k_mem_paging_histogram_eviction_get(
	काष्ठा k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, माप(*hist)));
	z_impl_k_mem_paging_histogram_eviction_get(hist);
}
#समावेश <zephyr/syscalls/k_mem_paging_histogram_eviction_get_mrsh.c>

अटल अंतरभूत
व्योम z_vrfy_k_mem_paging_histogram_backing_store_page_in_get(
	काष्ठा k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, माप(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_in_get(hist);
}
#समावेश <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_in_get_mrsh.c>

अटल अंतरभूत
व्योम z_vrfy_k_mem_paging_histogram_backing_store_page_out_get(
	काष्ठा k_mem_paging_histogram_t *hist)
{
	K_OOPS(K_SYSCALL_MEMORY_WRITE(hist, माप(*hist)));
	z_impl_k_mem_paging_histogram_backing_store_page_out_get(hist);
}
#समावेश <zephyr/syscalls/k_mem_paging_histogram_backing_store_page_out_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
