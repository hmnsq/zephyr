/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <zephyr/linker/sections.h>
#شامل <zephyr/sys/dlist.h>
#شامل <zephyr/init.h>
#شامل <zephyr/sys/check.h>
#شامل <zephyr/sys/iterable_sections.h>
#شامل <zephyr/sys/minmax.h>
#شامل <string.h>
/* private kernel APIs */
#شامل <ksched.h>
#شامل <kernel_internal.h>
#شامل <scheduler.h>
#شامل <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

ایستا صحیح k_mem_slab_stats_raw(ساختار k_obj_core *obj_core, تهی *stats)
{
	__ASSERT((obj_core != پوچ) && (stats != پوچ), "NULL parameter");

	ساختار k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_OF(obj_core, ساختار k_mem_slab, obj_core);
	کلید = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, اندازه(slab->info));
	k_spin_unlock(&slab->lock, کلید);

	بازگشت 0;
}

ایستا صحیح k_mem_slab_stats_query(ساختار k_obj_core *obj_core, تهی *stats)
{
	__ASSERT((obj_core != پوچ) && (stats != پوچ), "NULL parameter");

	ساختار k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;
	ساختار sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, ساختار k_mem_slab, obj_core);
	کلید = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.رقم_blocks - slab->info.رقم_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.رقم_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#وگرنه
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, کلید);

	بازگشت 0;
}

ایستا صحیح k_mem_slab_stats_reset(ساختار k_obj_core *obj_core)
{
	__ASSERT(obj_core != پوچ, "NULL parameter");

	ساختار k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_OF(obj_core, ساختار k_mem_slab, obj_core);
	کلید = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.رقم_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, کلید);

	بازگشت 0;
}

ایستا ساختار k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = اندازه(ساختار k_mem_slab_info),
	.query_size = اندازه(ساختار sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = پوچ,
	.enable = پوچ,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#وگرنه
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, پوچ);
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
ایستا صحیح create_free_list(ساختار k_mem_slab *slab)
{
	نویسه *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		بازگشت -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(اندازه(تهی *) - 1)) != 0U) {
		بازگشت -EINVAL;
	}

	اگر (size_mul_overflow(slab->info.block_size, slab->info.رقم_blocks, &total_size)) {
		بازگشت -EINVAL;
	}
	اگر (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		بازگشت -EINVAL;
	}

	slab->free_list = پوچ;
	p = (نویسه *)(total_size - slab->info.block_size);

	برای (uint32_t i = 0; i < slab->info.رقم_blocks; i++) {
		*(نویسه **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	بازگشت 0;
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
ایستا تهی init_mem_slab_module(تهی)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		اگر (create_free_list(slab) < 0) {
			بازگشت;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

صحیح k_mem_slab_init(ساختار k_mem_slab *slab, تهی *buffer,
		    size_t block_size, uint32_t رقم_blocks)
{
	صحیح rc;

	slab->info.رقم_blocks = رقم_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.رقم_used = 0U;
	slab->lock = (ساختار k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	اگر (rc < 0) {
		برو out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  اندازه(ساختار k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	بازگشت rc;
}

ایستا bool slab_ptr_is_good(ساختار k_mem_slab *slab, ثابت تهی *ptr)
{
	اگر (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		بازگشت true;
	}

	ثابت نویسه *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	بازگشت (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.رقم_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

صحیح k_mem_slab_alloc(ساختار k_mem_slab *slab, تهی **mem, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&slab->lock);
	صحیح result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	اگر (slab->free_list != پوچ) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(نویسه **)(slab->free_list);
		slab->info.رقم_used++;
		__ASSERT((slab->free_list == پوچ &&
			  slab->info.رقم_used == slab->info.رقم_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.رقم_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} وگرنه اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = پوچ;
		result = -ENOMEM;
	} وگرنه {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, کلید, &slab->wait_q, timeout);
		اگر (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		بازگشت result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, کلید);

	بازگشت result;
}

تهی k_mem_slab_free(ساختار k_mem_slab *slab, تهی *mem)
{
	اگر (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		بازگشت;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_کلید_t کلید = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, آزاد, slab);
	اگر (unlikely(slab->free_list == پوچ) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		اگر (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, آزاد, slab);
			z_reschedule(&slab->lock, کلید);
			بازگشت;
		}
	}
	*(نویسه **) mem = slab->free_list;
	slab->free_list = (نویسه *) mem;
	slab->info.رقم_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, آزاد, slab);

	k_spin_unlock(&slab->lock, کلید);
}

صحیح k_mem_slab_runtime_stats_get(ساختار k_mem_slab *slab, ساختار sys_memory_stats *stats)
{
	اگر ((slab == پوچ) || (stats == پوچ)) {
		بازگشت -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.رقم_used * slab->info.block_size;
	stats->free_bytes = (slab->info.رقم_blocks - slab->info.رقم_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#وگرنه
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, کلید);

	بازگشت 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
صحیح k_mem_slab_runtime_stats_reset_max(ساختار k_mem_slab *slab)
{
	اگر (slab == پوچ) {
		بازگشت -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.رقم_used;

	k_spin_unlock(&slab->lock, کلید);

	بازگشت 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
