/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/linker/sections.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/sys/check.h>
#ضمن <zephyr/sys/iterable_sections.h>
#ضمن <zephyr/sys/minmax.h>
#ضمن <string.h>
/* private kernel APIs */
#ضمن <ksched.h>
#ضمن <kernel_internal.h>
#ضمن <scheduler.h>
#ضمن <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

ساكن صحيح k_mem_slab_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_key_t   key;

	slab = CONTAINER_OF(obj_core, بنية k_mem_slab, obj_core);
	key = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, حجم(slab->info));
	k_spin_unlock(&slab->lock, key);

	ܦܢܐ 0;
}

ساكن صحيح k_mem_slab_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_key_t   key;
	بنية sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, بنية k_mem_slab, obj_core);
	key = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.num_blocks - slab->info.num_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.num_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#ܐܚܪܢܐ
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, key);

	ܦܢܐ 0;
}

ساكن صحيح k_mem_slab_stats_reset(بنية k_obj_core *obj_core)
{
	__ASSERT(obj_core != عدم, "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_key_t   key;

	slab = CONTAINER_OF(obj_core, بنية k_mem_slab, obj_core);
	key = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.num_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, key);

	ܦܢܐ 0;
}

ساكن بنية k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = حجم(بنية k_mem_slab_info),
	.query_size = حجم(بنية sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = عدم,
	.enable = عدم,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#ܐܚܪܢܐ
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, عدم);
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
ساكن صحيح create_free_list(بنية k_mem_slab *slab)
{
	حرف *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		ܦܢܐ -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(حجم(فراغ *) - 1)) != 0U) {
		ܦܢܐ -EINVAL;
	}

	ܐܢ (size_mul_overflow(slab->info.block_size, slab->info.num_blocks, &total_size)) {
		ܦܢܐ -EINVAL;
	}
	ܐܢ (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		ܦܢܐ -EINVAL;
	}

	slab->free_list = عدم;
	p = (حرف *)(total_size - slab->info.block_size);

	لكل (uint32_t i = 0; i < slab->info.num_blocks; i++) {
		*(حرف **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	ܦܢܐ 0;
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
ساكن فراغ init_mem_slab_module(فراغ)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		ܐܢ (create_free_list(slab) < 0) {
			ܦܢܐ;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

صحيح k_mem_slab_init(بنية k_mem_slab *slab, فراغ *buffer,
		    size_t block_size, uint32_t num_blocks)
{
	صحيح rc;

	slab->info.num_blocks = num_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.num_used = 0U;
	slab->lock = (بنية k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	ܐܢ (rc < 0) {
		اذهب out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  حجم(بنية k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	ܦܢܐ rc;
}

ساكن bool slab_ptr_is_good(بنية k_mem_slab *slab, ثابت فراغ *ptr)
{
	ܐܢ (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		ܦܢܐ true;
	}

	ثابت حرف *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	ܦܢܐ (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.num_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

صحيح k_mem_slab_alloc(بنية k_mem_slab *slab, فراغ **mem, k_timeout_t timeout)
{
	k_spinlock_key_t key = k_spin_lock(&slab->lock);
	صحيح result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	ܐܢ (slab->free_list != عدم) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(حرف **)(slab->free_list);
		slab->info.num_used++;
		__ASSERT((slab->free_list == عدم &&
			  slab->info.num_used == slab->info.num_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.num_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} ܐܚܪܢܐ ܐܢ (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = عدم;
		result = -ENOMEM;
	} ܐܚܪܢܐ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, key, &slab->wait_q, timeout);
		ܐܢ (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		ܦܢܐ result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, key);

	ܦܢܐ result;
}

فراغ k_mem_slab_free(بنية k_mem_slab *slab, فراغ *mem)
{
	ܐܢ (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		ܦܢܐ;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_key_t key = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, حرر, slab);
	ܐܢ (unlikely(slab->free_list == عدم) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		ܐܢ (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);
			z_reschedule(&slab->lock, key);
			ܦܢܐ;
		}
	}
	*(حرف **) mem = slab->free_list;
	slab->free_list = (حرف *) mem;
	slab->info.num_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);

	k_spin_unlock(&slab->lock, key);
}

صحيح k_mem_slab_runtime_stats_get(بنية k_mem_slab *slab, بنية sys_memory_stats *stats)
{
	ܐܢ ((slab == عدم) || (stats == عدم)) {
		ܦܢܐ -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.num_used * slab->info.block_size;
	stats->free_bytes = (slab->info.num_blocks - slab->info.num_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#ܐܚܪܢܐ
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, key);

	ܦܢܐ 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
صحيح k_mem_slab_runtime_stats_reset_max(بنية k_mem_slab *slab)
{
	ܐܢ (slab == عدم) {
		ܦܢܐ -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.num_used;

	k_spin_unlock(&slab->lock, key);

	ܦܢܐ 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
