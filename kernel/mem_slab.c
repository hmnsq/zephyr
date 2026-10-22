/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <zephyr/linker/sections.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>
#അന്തര്ഭാവ <zephyr/sys/iterable_sections.h>
#അന്തര്ഭാവ <zephyr/sys/minmax.h>
#അന്തര്ഭാവ <string.h>
/* private kernel APIs */
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <scheduler.h>
#അന്തര്ഭാവ <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

സ്ഥിതം പൂർണ്ണസംഖ്യ k_mem_slab_stats_raw(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	__ASSERT((obj_core != ശൂന്യ) && (stats != ശൂന്യ), "NULL parameter");

	ഘടന k_mem_slab *slab;
	k_spinlock_താക്കോൽ_t   താക്കോൽ;

	slab = CONTAINER_OF(obj_core, ഘടന k_mem_slab, obj_core);
	താക്കോൽ = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, വലുപ്പം(slab->info));
	k_spin_unlock(&slab->lock, താക്കോൽ);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ k_mem_slab_stats_query(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	__ASSERT((obj_core != ശൂന്യ) && (stats != ശൂന്യ), "NULL parameter");

	ഘടന k_mem_slab *slab;
	k_spinlock_താക്കോൽ_t   താക്കോൽ;
	ഘടന sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, ഘടന k_mem_slab, obj_core);
	താക്കോൽ = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.അക്കം_blocks - slab->info.അക്കം_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.അക്കം_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#അല്ലെങ്കിൽ
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, താക്കോൽ);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ k_mem_slab_stats_reset(ഘടന k_obj_core *obj_core)
{
	__ASSERT(obj_core != ശൂന്യ, "NULL parameter");

	ഘടന k_mem_slab *slab;
	k_spinlock_താക്കോൽ_t   താക്കോൽ;

	slab = CONTAINER_OF(obj_core, ഘടന k_mem_slab, obj_core);
	താക്കോൽ = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.അക്കം_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, താക്കോൽ);

	തിരികെ 0;
}

സ്ഥിതം ഘടന k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = വലുപ്പം(ഘടന k_mem_slab_info),
	.query_size = വലുപ്പം(ഘടന sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ശൂന്യ,
	.enable = ശൂന്യ,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#അല്ലെങ്കിൽ
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ശൂന്യ);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ create_free_list(ഘടന k_mem_slab *slab)
{
	അക്ഷരം *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		തിരികെ -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(വലുപ്പം(ശൂന്യം *) - 1)) != 0U) {
		തിരികെ -EINVAL;
	}

	എങ്കിൽ (size_mul_overflow(slab->info.block_size, slab->info.അക്കം_blocks, &total_size)) {
		തിരികെ -EINVAL;
	}
	എങ്കിൽ (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		തിരികെ -EINVAL;
	}

	slab->free_list = ശൂന്യ;
	p = (അക്ഷരം *)(total_size - slab->info.block_size);

	വേണ്ടി (uint32_t i = 0; i < slab->info.അക്കം_blocks; i++) {
		*(അക്ഷരം **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	തിരികെ 0;
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
സ്ഥിതം ശൂന്യം init_mem_slab_module(ശൂന്യം)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		എങ്കിൽ (create_free_list(slab) < 0) {
			തിരികെ;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

പൂർണ്ണസംഖ്യ k_mem_slab_init(ഘടന k_mem_slab *slab, ശൂന്യം *buffer,
		    size_t block_size, uint32_t അക്കം_blocks)
{
	പൂർണ്ണസംഖ്യ rc;

	slab->info.അക്കം_blocks = അക്കം_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.അക്കം_used = 0U;
	slab->lock = (ഘടന k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	എങ്കിൽ (rc < 0) {
		പോകുക out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  വലുപ്പം(ഘടന k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	തിരികെ rc;
}

സ്ഥിതം bool slab_ptr_is_good(ഘടന k_mem_slab *slab, സ്ഥിരം ശൂന്യം *ptr)
{
	എങ്കിൽ (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		തിരികെ true;
	}

	സ്ഥിരം അക്ഷരം *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	തിരികെ (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.അക്കം_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

പൂർണ്ണസംഖ്യ k_mem_slab_alloc(ഘടന k_mem_slab *slab, ശൂന്യം **mem, k_timeout_t timeout)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&slab->lock);
	പൂർണ്ണസംഖ്യ result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	എങ്കിൽ (slab->free_list != ശൂന്യ) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(അക്ഷരം **)(slab->free_list);
		slab->info.അക്കം_used++;
		__ASSERT((slab->free_list == ശൂന്യ &&
			  slab->info.അക്കം_used == slab->info.അക്കം_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.അക്കം_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ശൂന്യ;
		result = -ENOMEM;
	} അല്ലെങ്കിൽ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, താക്കോൽ, &slab->wait_q, timeout);
		എങ്കിൽ (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		തിരികെ result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, താക്കോൽ);

	തിരികെ result;
}

ശൂന്യം k_mem_slab_free(ഘടന k_mem_slab *slab, ശൂന്യം *mem)
{
	എങ്കിൽ (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		തിരികെ;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, സ്വതന്ത്രം, slab);
	എങ്കിൽ (unlikely(slab->free_list == ശൂന്യ) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		എങ്കിൽ (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, സ്വതന്ത്രം, slab);
			z_reschedule(&slab->lock, താക്കോൽ);
			തിരികെ;
		}
	}
	*(അക്ഷരം **) mem = slab->free_list;
	slab->free_list = (അക്ഷരം *) mem;
	slab->info.അക്കം_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, സ്വതന്ത്രം, slab);

	k_spin_unlock(&slab->lock, താക്കോൽ);
}

പൂർണ്ണസംഖ്യ k_mem_slab_runtime_stats_get(ഘടന k_mem_slab *slab, ഘടന sys_memory_stats *stats)
{
	എങ്കിൽ ((slab == ശൂന്യ) || (stats == ശൂന്യ)) {
		തിരികെ -EINVAL;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.അക്കം_used * slab->info.block_size;
	stats->free_bytes = (slab->info.അക്കം_blocks - slab->info.അക്കം_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#അല്ലെങ്കിൽ
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, താക്കോൽ);

	തിരികെ 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
പൂർണ്ണസംഖ്യ k_mem_slab_runtime_stats_reset_max(ഘടന k_mem_slab *slab)
{
	എങ്കിൽ (slab == ശൂന്യ) {
		തിരികെ -EINVAL;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.അക്കം_used;

	k_spin_unlock(&slab->lock, താക്കോൽ);

	തിരികെ 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
