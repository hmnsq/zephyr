/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/iterable_sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <string.h>
/* private kernel APIs */
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ k_mem_slab_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	__ASSERT((obj_core != ರಿಕ್ತ) && (stats != ರಿಕ್ತ), "NULL parameter");

	ಸಂರಚನಾ k_mem_slab *slab;
	k_spinlock_ಕೀಲಿ_t   ಕೀಲಿ;

	slab = CONTAINER_OF(obj_core, ಸಂರಚನಾ k_mem_slab, obj_core);
	ಕೀಲಿ = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, ಪರಿಮಾಣ(slab->info));
	k_spin_unlock(&slab->lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ k_mem_slab_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	__ASSERT((obj_core != ರಿಕ್ತ) && (stats != ರಿಕ್ತ), "NULL parameter");

	ಸಂರಚನಾ k_mem_slab *slab;
	k_spinlock_ಕೀಲಿ_t   ಕೀಲಿ;
	ಸಂರಚನಾ sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, ಸಂರಚನಾ k_mem_slab, obj_core);
	ಕೀಲಿ = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.ಅಂಕೆ_blocks - slab->info.ಅಂಕೆ_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.ಅಂಕೆ_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#ಅನ್ಯಥಾ
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ k_mem_slab_stats_reset(ಸಂರಚನಾ k_obj_core *obj_core)
{
	__ASSERT(obj_core != ರಿಕ್ತ, "NULL parameter");

	ಸಂರಚನಾ k_mem_slab *slab;
	k_spinlock_ಕೀಲಿ_t   ಕೀಲಿ;

	slab = CONTAINER_OF(obj_core, ಸಂರಚನಾ k_mem_slab, obj_core);
	ಕೀಲಿ = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.ಅಂಕೆ_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಸಂರಚನಾ k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = ಪರಿಮಾಣ(ಸಂರಚನಾ k_mem_slab_info),
	.query_size = ಪರಿಮಾಣ(ಸಂರಚನಾ sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ರಿಕ್ತ,
	.enable = ರಿಕ್ತ,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#ಅನ್ಯಥಾ
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ರಿಕ್ತ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ create_free_list(ಸಂರಚನಾ k_mem_slab *slab)
{
	ವರ್ಣ *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(ಪರಿಮಾಣ(ಶೂನ್ಯ *) - 1)) != 0U) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಯದಿ (size_mul_overflow(slab->info.block_size, slab->info.ಅಂಕೆ_blocks, &total_size)) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}
	ಯದಿ (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	slab->free_list = ರಿಕ್ತ;
	p = (ವರ್ಣ *)(total_size - slab->info.block_size);

	ಪ್ರತಿ (uint32_t i = 0; i < slab->info.ಅಂಕೆ_blocks; i++) {
		*(ವರ್ಣ **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ init_mem_slab_module(ಶೂನ್ಯ)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		ಯದಿ (create_free_list(slab) < 0) {
			ಪ್ರತ್ಯಾಗಮ;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

ಪೂರ್ಣಾಙ್ಕ k_mem_slab_init(ಸಂರಚನಾ k_mem_slab *slab, ಶೂನ್ಯ *buffer,
		    size_t block_size, uint32_t ಅಂಕೆ_blocks)
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	slab->info.ಅಂಕೆ_blocks = ಅಂಕೆ_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.ಅಂಕೆ_used = 0U;
	slab->lock = (ಸಂರಚನಾ k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	ಯದಿ (rc < 0) {
		ಗಚ್ಛ out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  ಪರಿಮಾಣ(ಸಂರಚನಾ k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ bool slab_ptr_is_good(ಸಂರಚನಾ k_mem_slab *slab, ಧ್ರುವ ಶೂನ್ಯ *ptr)
{
	ಯದಿ (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಧ್ರುವ ವರ್ಣ *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	ಪ್ರತ್ಯಾಗಮ (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.ಅಂಕೆ_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

ಪೂರ್ಣಾಙ್ಕ k_mem_slab_alloc(ಸಂರಚನಾ k_mem_slab *slab, ಶೂನ್ಯ **mem, k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&slab->lock);
	ಪೂರ್ಣಾಙ್ಕ result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	ಯದಿ (slab->free_list != ರಿಕ್ತ) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(ವರ್ಣ **)(slab->free_list);
		slab->info.ಅಂಕೆ_used++;
		__ASSERT((slab->free_list == ರಿಕ್ತ &&
			  slab->info.ಅಂಕೆ_used == slab->info.ಅಂಕೆ_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.ಅಂಕೆ_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} ಅನ್ಯಥಾ ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ರಿಕ್ತ;
		result = -ENOMEM;
	} ಅನ್ಯಥಾ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, ಕೀಲಿ, &slab->wait_q, timeout);
		ಯದಿ (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		ಪ್ರತ್ಯಾಗಮ result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ result;
}

ಶೂನ್ಯ k_mem_slab_free(ಸಂರಚನಾ k_mem_slab *slab, ಶೂನ್ಯ *mem)
{
	ಯದಿ (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		ಪ್ರತ್ಯಾಗಮ;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, ಮುಕ್ತ, slab);
	ಯದಿ (unlikely(slab->free_list == ರಿಕ್ತ) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		ಯದಿ (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ಮುಕ್ತ, slab);
			z_reschedule(&slab->lock, ಕೀಲಿ);
			ಪ್ರತ್ಯಾಗಮ;
		}
	}
	*(ವರ್ಣ **) mem = slab->free_list;
	slab->free_list = (ವರ್ಣ *) mem;
	slab->info.ಅಂಕೆ_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ಮುಕ್ತ, slab);

	k_spin_unlock(&slab->lock, ಕೀಲಿ);
}

ಪೂರ್ಣಾಙ್ಕ k_mem_slab_runtime_stats_get(ಸಂರಚನಾ k_mem_slab *slab, ಸಂರಚನಾ sys_memory_stats *stats)
{
	ಯದಿ ((slab == ರಿಕ್ತ) || (stats == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.ಅಂಕೆ_used * slab->info.block_size;
	stats->free_bytes = (slab->info.ಅಂಕೆ_blocks - slab->info.ಅಂಕೆ_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#ಅನ್ಯಥಾ
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
ಪೂರ್ಣಾಙ್ಕ k_mem_slab_runtime_stats_reset_max(ಸಂರಚನಾ k_mem_slab *slab)
{
	ಯದಿ (slab == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.ಅಂಕೆ_used;

	k_spin_unlock(&slab->lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
