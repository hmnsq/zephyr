/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <zephyr/linker/sections.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/sys/check.h>
#அந்தர்பாவ <zephyr/sys/iterable_sections.h>
#அந்தர்பாவ <zephyr/sys/minmax.h>
#அந்தர்பாவ <string.h>
/* private kernel APIs */
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <scheduler.h>
#அந்தர்பாவ <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

நிலையான முழு k_mem_slab_stats_raw(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	__ASSERT((obj_core != வெற்று) && (stats != வெற்று), "NULL parameter");

	கட்டமைப்பு k_mem_slab *slab;
	k_spinlock_சாவி_t   சாவி;

	slab = CONTAINER_OF(obj_core, கட்டமைப்பு k_mem_slab, obj_core);
	சாவி = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, அளவு(slab->info));
	k_spin_unlock(&slab->lock, சாவி);

	திரும்பு 0;
}

நிலையான முழு k_mem_slab_stats_query(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	__ASSERT((obj_core != வெற்று) && (stats != வெற்று), "NULL parameter");

	கட்டமைப்பு k_mem_slab *slab;
	k_spinlock_சாவி_t   சாவி;
	கட்டமைப்பு sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, கட்டமைப்பு k_mem_slab, obj_core);
	சாவி = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.இலக்கம்_blocks - slab->info.இலக்கம்_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.இலக்கம்_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#இல்லையெனில்
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, சாவி);

	திரும்பு 0;
}

நிலையான முழு k_mem_slab_stats_reset(கட்டமைப்பு k_obj_core *obj_core)
{
	__ASSERT(obj_core != வெற்று, "NULL parameter");

	கட்டமைப்பு k_mem_slab *slab;
	k_spinlock_சாவி_t   சாவி;

	slab = CONTAINER_OF(obj_core, கட்டமைப்பு k_mem_slab, obj_core);
	சாவி = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.இலக்கம்_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, சாவி);

	திரும்பு 0;
}

நிலையான கட்டமைப்பு k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = அளவு(கட்டமைப்பு k_mem_slab_info),
	.query_size = அளவு(கட்டமைப்பு sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = வெற்று,
	.enable = வெற்று,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#இல்லையெனில்
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, வெற்று);
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
நிலையான முழு create_free_list(கட்டமைப்பு k_mem_slab *slab)
{
	எழுத்து *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		திரும்பு -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(அளவு(வெறுமை *) - 1)) != 0U) {
		திரும்பு -EINVAL;
	}

	எனில் (size_mul_overflow(slab->info.block_size, slab->info.இலக்கம்_blocks, &total_size)) {
		திரும்பு -EINVAL;
	}
	எனில் (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		திரும்பு -EINVAL;
	}

	slab->free_list = வெற்று;
	p = (எழுத்து *)(total_size - slab->info.block_size);

	க்கு (uint32_t i = 0; i < slab->info.இலக்கம்_blocks; i++) {
		*(எழுத்து **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	திரும்பு 0;
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
நிலையான வெறுமை init_mem_slab_module(வெறுமை)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		எனில் (create_free_list(slab) < 0) {
			திரும்பு;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

முழு k_mem_slab_init(கட்டமைப்பு k_mem_slab *slab, வெறுமை *buffer,
		    size_t block_size, uint32_t இலக்கம்_blocks)
{
	முழு rc;

	slab->info.இலக்கம்_blocks = இலக்கம்_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.இலக்கம்_used = 0U;
	slab->lock = (கட்டமைப்பு k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	எனில் (rc < 0) {
		செல் out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  அளவு(கட்டமைப்பு k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	திரும்பு rc;
}

நிலையான bool slab_ptr_is_good(கட்டமைப்பு k_mem_slab *slab, மாறிலி வெறுமை *ptr)
{
	எனில் (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		திரும்பு true;
	}

	மாறிலி எழுத்து *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	திரும்பு (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.இலக்கம்_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

முழு k_mem_slab_alloc(கட்டமைப்பு k_mem_slab *slab, வெறுமை **mem, k_timeout_t timeout)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&slab->lock);
	முழு result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	எனில் (slab->free_list != வெற்று) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(எழுத்து **)(slab->free_list);
		slab->info.இலக்கம்_used++;
		__ASSERT((slab->free_list == வெற்று &&
			  slab->info.இலக்கம்_used == slab->info.இலக்கம்_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.இலக்கம்_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} இல்லையெனில் எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = வெற்று;
		result = -ENOMEM;
	} இல்லையெனில் {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, சாவி, &slab->wait_q, timeout);
		எனில் (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		திரும்பு result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, சாவி);

	திரும்பு result;
}

வெறுமை k_mem_slab_free(கட்டமைப்பு k_mem_slab *slab, வெறுமை *mem)
{
	எனில் (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		திரும்பு;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_சாவி_t சாவி = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, விடுவி, slab);
	எனில் (unlikely(slab->free_list == வெற்று) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		எனில் (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, விடுவி, slab);
			z_reschedule(&slab->lock, சாவி);
			திரும்பு;
		}
	}
	*(எழுத்து **) mem = slab->free_list;
	slab->free_list = (எழுத்து *) mem;
	slab->info.இலக்கம்_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, விடுவி, slab);

	k_spin_unlock(&slab->lock, சாவி);
}

முழு k_mem_slab_runtime_stats_get(கட்டமைப்பு k_mem_slab *slab, கட்டமைப்பு sys_memory_stats *stats)
{
	எனில் ((slab == வெற்று) || (stats == வெற்று)) {
		திரும்பு -EINVAL;
	}

	k_spinlock_சாவி_t சாவி = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.இலக்கம்_used * slab->info.block_size;
	stats->free_bytes = (slab->info.இலக்கம்_blocks - slab->info.இலக்கம்_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#இல்லையெனில்
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, சாவி);

	திரும்பு 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
முழு k_mem_slab_runtime_stats_reset_max(கட்டமைப்பு k_mem_slab *slab)
{
	எனில் (slab == வெற்று) {
		திரும்பு -EINVAL;
	}

	k_spinlock_சாவி_t சாவி = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.இலக்கம்_used;

	k_spin_unlock(&slab->lock, சாவி);

	திரும்பு 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
