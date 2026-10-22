/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/linker/sections.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/dlist.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/check.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/iterable_sections.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/minmax.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
/* private kernel APIs */
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_slab_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	__ASSERT((obj_core != ଖାଲି) && (stats != ଖାଲି), "NULL parameter");

	ସଂରଚନା k_mem_slab *slab;
	k_spinlock_ଚାବି_t   ଚାବି;

	slab = CONTAINER_OF(obj_core, ସଂରଚନା k_mem_slab, obj_core);
	ଚାବି = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, ଆକାର(slab->info));
	k_spin_unlock(&slab->lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_slab_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	__ASSERT((obj_core != ଖାଲି) && (stats != ଖାଲି), "NULL parameter");

	ସଂରଚନା k_mem_slab *slab;
	k_spinlock_ଚାବି_t   ଚାବି;
	ସଂରଚନା sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, ସଂରଚନା k_mem_slab, obj_core);
	ଚାବି = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.ଅଙ୍କ_blocks - slab->info.ଅଙ୍କ_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.ଅଙ୍କ_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#ଅନ୍ୟଥା
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_slab_stats_reset(ସଂରଚନା k_obj_core *obj_core)
{
	__ASSERT(obj_core != ଖାଲି, "NULL parameter");

	ସଂରଚନା k_mem_slab *slab;
	k_spinlock_ଚାବି_t   ଚାବି;

	slab = CONTAINER_OF(obj_core, ସଂରଚନା k_mem_slab, obj_core);
	ଚାବି = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.ଅଙ୍କ_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ସଂରଚନା k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = ଆକାର(ସଂରଚନା k_mem_slab_info),
	.query_size = ଆକାର(ସଂରଚନା sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ଖାଲି,
	.enable = ଖାଲି,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#ଅନ୍ୟଥା
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ଖାଲି);
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */

/**
 * @brief Initialize kernel memory slab subsystem.
 *
 * Perform any initialization of memory slabs that wasn't done at build time.
 * Currently this just involves creating the list of free blocks for each slab.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p slab contains invalid configuration and/or values.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା create_free_list(ସଂରଚନା k_mem_slab *slab)
{
	ଅକ୍ଷର *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(ଆକାର(ଶୂନ୍ୟ *) - 1)) != 0U) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଯଦି (size_mul_overflow(slab->info.block_size, slab->info.ଅଙ୍କ_blocks, &total_size)) {
		ଫେରନ୍ତୁ -EINVAL;
	}
	ଯଦି (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	slab->free_list = ଖାଲି;
	p = (ଅକ୍ଷର *)(total_size - slab->info.block_size);

	ପାଇଁ (uint32_t i = 0; i < slab->info.ଅଙ୍କ_blocks; i++) {
		*(ଅକ୍ଷର **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	ଫେରନ୍ତୁ 0;
}

/**
 * @brief Complete initialization of statically defined memory slabs.
 *
 * Build the free block list for each statically defined slab. This is
 * mandatory functional initialization, required whether or not the object
 * core framework is enabled, so it stays as its own init rather than moving
 * into the object core registration (see K_OBJ_TYPE_DEFINE_STATS above, which
 * handles this slab type's object core duties).
 *
 * Initialization stops at the first slab with an invalid configuration
 * (create_free_list() fails), matching the previous SYS_INIT behavior whose
 * return value was discarded by the init runner.
 */
ସ୍ଥିର ଶୂନ୍ୟ init_mem_slab_module(ଶୂନ୍ୟ)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		ଯଦି (create_free_list(slab) < 0) {
			ଫେରନ୍ତୁ;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_slab_init(ସଂରଚନା k_mem_slab *slab, ଶୂନ୍ୟ *buffer,
		    size_t block_size, uint32_t ଅଙ୍କ_blocks)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	slab->info.ଅଙ୍କ_blocks = ଅଙ୍କ_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.ଅଙ୍କ_used = 0U;
	slab->lock = (ସଂରଚନା k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	ଯଦି (rc < 0) {
		ଯାଅ out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  ଆକାର(ସଂରଚନା k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର bool slab_ptr_is_good(ସଂରଚନା k_mem_slab *slab, ଅଚଳ ଶୂନ୍ୟ *ptr)
{
	ଯଦି (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		ଫେରନ୍ତୁ true;
	}

	ଅଚଳ ଅକ୍ଷର *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	ଫେରନ୍ତୁ (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.ଅଙ୍କ_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_slab_alloc(ସଂରଚନା k_mem_slab *slab, ଶୂନ୍ୟ **mem, k_timeout_t timeout)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&slab->lock);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	ଯଦି (slab->free_list != ଖାଲି) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(ଅକ୍ଷର **)(slab->free_list);
		slab->info.ଅଙ୍କ_used++;
		__ASSERT((slab->free_list == ଖାଲି &&
			  slab->info.ଅଙ୍କ_used == slab->info.ଅଙ୍କ_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.ଅଙ୍କ_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} ଅନ୍ୟଥା ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ଖାଲି;
		result = -ENOMEM;
	} ଅନ୍ୟଥା {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, ଚାବି, &slab->wait_q, timeout);
		ଯଦି (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		ଫେରନ୍ତୁ result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, ଚାବି);

	ଫେରନ୍ତୁ result;
}

ଶୂନ୍ୟ k_mem_slab_free(ସଂରଚନା k_mem_slab *slab, ଶୂନ୍ୟ *mem)
{
	ଯଦି (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		ଫେରନ୍ତୁ;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, ମୁକ୍ତ, slab);
	ଯଦି (unlikely(slab->free_list == ଖାଲି) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		ଯଦି (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ମୁକ୍ତ, slab);
			z_reschedule(&slab->lock, ଚାବି);
			ଫେରନ୍ତୁ;
		}
	}
	*(ଅକ୍ଷର **) mem = slab->free_list;
	slab->free_list = (ଅକ୍ଷର *) mem;
	slab->info.ଅଙ୍କ_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ମୁକ୍ତ, slab);

	k_spin_unlock(&slab->lock, ଚାବି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_slab_runtime_stats_get(ସଂରଚନା k_mem_slab *slab, ସଂରଚନା sys_memory_stats *stats)
{
	ଯଦି ((slab == ଖାଲି) || (stats == ଖାଲି)) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.ଅଙ୍କ_used * slab->info.block_size;
	stats->free_bytes = (slab->info.ଅଙ୍କ_blocks - slab->info.ଅଙ୍କ_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#ଅନ୍ୟଥା
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_mem_slab_runtime_stats_reset_max(ସଂରଚନା k_mem_slab *slab)
{
	ଯଦି (slab == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.ଅଙ୍କ_used;

	k_spin_unlock(&slab->lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
