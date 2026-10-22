/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>

#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <zephyr/linker/sections.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/sys/check.h>
#অন্তর্ভাব <zephyr/sys/iterable_sections.h>
#অন্তর্ভাব <zephyr/sys/minmax.h>
#অন্তর্ভাব <string.h>
/* private kernel APIs */
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <scheduler.h>
#অন্তর্ভাব <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

স্থির পূর্ণসংখ্যা k_mem_slab_stats_raw(কাঠামো k_obj_core *obj_core, শূন্য *stats)
{
	__ASSERT((obj_core != ফাঁকা) && (stats != ফাঁকা), "NULL parameter");

	কাঠামো k_mem_slab *slab;
	k_spinlock_চাবি_t   চাবি;

	slab = CONTAINER_OF(obj_core, কাঠামো k_mem_slab, obj_core);
	চাবি = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, আকার(slab->info));
	k_spin_unlock(&slab->lock, চাবি);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা k_mem_slab_stats_query(কাঠামো k_obj_core *obj_core, শূন্য *stats)
{
	__ASSERT((obj_core != ফাঁকা) && (stats != ফাঁকা), "NULL parameter");

	কাঠামো k_mem_slab *slab;
	k_spinlock_চাবি_t   চাবি;
	কাঠামো sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, কাঠামো k_mem_slab, obj_core);
	চাবি = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.অঙ্ক_blocks - slab->info.অঙ্ক_used) *
			  slab->info.block_আকার;
	ptr->allocated_bytes = slab->info.অঙ্ক_used * slab->info.block_আকার;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_আকার;
#অন্যথা
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, চাবি);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা k_mem_slab_stats_reset(কাঠামো k_obj_core *obj_core)
{
	__ASSERT(obj_core != ফাঁকা, "NULL parameter");

	কাঠামো k_mem_slab *slab;
	k_spinlock_চাবি_t   চাবি;

	slab = CONTAINER_OF(obj_core, কাঠামো k_mem_slab, obj_core);
	চাবি = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.অঙ্ক_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, চাবি);

	ফেরত 0;
}

স্থির কাঠামো k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_আকার = আকার(কাঠামো k_mem_slab_info),
	.query_আকার = আকার(কাঠামো sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ফাঁকা,
	.enable = ফাঁকা,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#অন্যথা
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ফাঁকা);
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
স্থির পূর্ণসংখ্যা create_free_list(কাঠামো k_mem_slab *slab)
{
	অক্ষর *p;
	size_t যোগ_আকার;

	CHECKIF(slab->info.block_আকার == 0U) {
		ফেরত -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_আকার | (uintptr_t)slab->বাফার) &
				(আকার(শূন্য *) - 1)) != 0U) {
		ফেরত -EINVAL;
	}

	যদি (আকার_mul_overflow(slab->info.block_আকার, slab->info.অঙ্ক_blocks, &যোগ_আকার)) {
		ফেরত -EINVAL;
	}
	যদি (আকার_add_overflow((size_t)(uintptr_t)slab->বাফার, যোগ_আকার, &যোগ_আকার)) {
		ফেরত -EINVAL;
	}

	slab->free_list = ফাঁকা;
	p = (অক্ষর *)(যোগ_আকার - slab->info.block_আকার);

	জন্য (uint32_t i = 0; i < slab->info.অঙ্ক_blocks; i++) {
		*(অক্ষর **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_আকার;
	}

	ফেরত 0;
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
স্থির শূন্য init_mem_slab_module(শূন্য)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		যদি (create_free_list(slab) < 0) {
			ফেরত;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

পূর্ণসংখ্যা k_mem_slab_init(কাঠামো k_mem_slab *slab, শূন্য *বাফার,
		    size_t block_আকার, uint32_t অঙ্ক_blocks)
{
	পূর্ণসংখ্যা rc;

	slab->info.অঙ্ক_blocks = অঙ্ক_blocks;
	slab->info.block_আকার = block_আকার;
	slab->বাফার = বাফার;
	slab->info.অঙ্ক_used = 0U;
	slab->lock = (কাঠামো k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	যদি (rc < 0) {
		যাও out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  আকার(কাঠামো k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	ফেরত rc;
}

স্থির bool slab_ptr_is_good(কাঠামো k_mem_slab *slab, ধ্রুবক শূন্য *ptr)
{
	যদি (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		ফেরত true;
	}

	ধ্রুবক অক্ষর *p = ptr;
	ptrdiff_t offset = p - slab->বাফার;

	ফেরত (offset >= 0) &&
	       (offset < (slab->info.block_আকার * slab->info.অঙ্ক_blocks)) &&
	       ((offset % slab->info.block_আকার) == 0);
}

পূর্ণসংখ্যা k_mem_slab_alloc(কাঠামো k_mem_slab *slab, শূন্য **mem, k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&slab->lock);
	পূর্ণসংখ্যা ফলাফল;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	যদি (slab->free_list != ফাঁকা) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(অক্ষর **)(slab->free_list);
		slab->info.অঙ্ক_used++;
		__ASSERT((slab->free_list == ফাঁকা &&
			  slab->info.অঙ্ক_used == slab->info.অঙ্ক_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.অঙ্ক_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		ফলাফল = 0;
	} অন্যথা যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ফাঁকা;
		ফলাফল = -ENOMEM;
	} অন্যথা {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		ফলাফল = z_pend_curr(&slab->lock, চাবি, &slab->wait_q, timeout);
		যদি (ফলাফল == 0) {
			*mem = _current->base.swap_তথ্য;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ফলাফল);

		ফেরত ফলাফল;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ফলাফল);

	k_spin_unlock(&slab->lock, চাবি);

	ফেরত ফলাফল;
}

শূন্য k_mem_slab_free(কাঠামো k_mem_slab *slab, শূন্য *mem)
{
	যদি (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		ফেরত;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_আকার);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_চাবি_t চাবি = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, মুক্ত, slab);
	যদি (unlikely(slab->free_list == ফাঁকা) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		যদি (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, মুক্ত, slab);
			z_reschedule(&slab->lock, চাবি);
			ফেরত;
		}
	}
	*(অক্ষর **) mem = slab->free_list;
	slab->free_list = (অক্ষর *) mem;
	slab->info.অঙ্ক_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, মুক্ত, slab);

	k_spin_unlock(&slab->lock, চাবি);
}

পূর্ণসংখ্যা k_mem_slab_runtime_stats_get(কাঠামো k_mem_slab *slab, কাঠামো sys_memory_stats *stats)
{
	যদি ((slab == ফাঁকা) || (stats == ফাঁকা)) {
		ফেরত -EINVAL;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.অঙ্ক_used * slab->info.block_আকার;
	stats->free_bytes = (slab->info.অঙ্ক_blocks - slab->info.অঙ্ক_used) *
			    slab->info.block_আকার;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_আকার;
#অন্যথা
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, চাবি);

	ফেরত 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
পূর্ণসংখ্যা k_mem_slab_runtime_stats_reset_max(কাঠামো k_mem_slab *slab)
{
	যদি (slab == ফাঁকা) {
		ফেরত -EINVAL;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.অঙ্ক_used;

	k_spin_unlock(&slab->lock, চাবি);

	ফেরত 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
