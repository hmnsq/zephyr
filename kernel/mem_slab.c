/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/linker/sections.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/sys/check.h>
#અન્તર્ભાવ <zephyr/sys/iterable_sections.h>
#અન્તર્ભાવ <zephyr/sys/minmax.h>
#અન્તર્ભાવ <string.h>
/* private kernel APIs */
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <scheduler.h>
#અન્તર્ભાવ <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

સ્થિર પૂર્ણાંક k_mem_slab_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	__ASSERT((obj_core != ખાલી) && (stats != ખાલી), "NULL parameter");

	સંરચના k_mem_slab *slab;
	k_spinlock_ચાવી_t   ચાવી;

	slab = CONTAINER_OF(obj_core, સંરચના k_mem_slab, obj_core);
	ચાવી = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, કદ(slab->info));
	k_spin_unlock(&slab->lock, ચાવી);

	પાછા 0;
}

સ્થિર પૂર્ણાંક k_mem_slab_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	__ASSERT((obj_core != ખાલી) && (stats != ખાલી), "NULL parameter");

	સંરચના k_mem_slab *slab;
	k_spinlock_ચાવી_t   ચાવી;
	સંરચના sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, સંરચના k_mem_slab, obj_core);
	ચાવી = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.અંક_blocks - slab->info.અંક_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.અંક_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#નહીંતર
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, ચાવી);

	પાછા 0;
}

સ્થિર પૂર્ણાંક k_mem_slab_stats_reset(સંરચના k_obj_core *obj_core)
{
	__ASSERT(obj_core != ખાલી, "NULL parameter");

	સંરચના k_mem_slab *slab;
	k_spinlock_ચાવી_t   ચાવી;

	slab = CONTAINER_OF(obj_core, સંરચના k_mem_slab, obj_core);
	ચાવી = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.અંક_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, ચાવી);

	પાછા 0;
}

સ્થિર સંરચના k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = કદ(સંરચના k_mem_slab_info),
	.query_size = કદ(સંરચના sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ખાલી,
	.enable = ખાલી,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#નહીંતર
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ખાલી);
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
સ્થિર પૂર્ણાંક create_free_list(સંરચના k_mem_slab *slab)
{
	અક્ષર *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		પાછા -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(કદ(શૂન્ય *) - 1)) != 0U) {
		પાછા -EINVAL;
	}

	જો (size_mul_overflow(slab->info.block_size, slab->info.અંક_blocks, &total_size)) {
		પાછા -EINVAL;
	}
	જો (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		પાછા -EINVAL;
	}

	slab->free_list = ખાલી;
	p = (અક્ષર *)(total_size - slab->info.block_size);

	માટે (uint32_t i = 0; i < slab->info.અંક_blocks; i++) {
		*(અક્ષર **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	પાછા 0;
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
સ્થિર શૂન્ય init_mem_slab_module(શૂન્ય)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		જો (create_free_list(slab) < 0) {
			પાછા;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

પૂર્ણાંક k_mem_slab_init(સંરચના k_mem_slab *slab, શૂન્ય *buffer,
		    size_t block_size, uint32_t અંક_blocks)
{
	પૂર્ણાંક rc;

	slab->info.અંક_blocks = અંક_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.અંક_used = 0U;
	slab->lock = (સંરચના k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	જો (rc < 0) {
		જાઓ out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  કદ(સંરચના k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	પાછા rc;
}

સ્થિર bool slab_ptr_is_good(સંરચના k_mem_slab *slab, અચલ શૂન્ય *ptr)
{
	જો (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		પાછા true;
	}

	અચલ અક્ષર *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	પાછા (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.અંક_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

પૂર્ણાંક k_mem_slab_alloc(સંરચના k_mem_slab *slab, શૂન્ય **mem, k_timeout_t timeout)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&slab->lock);
	પૂર્ણાંક result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	જો (slab->free_list != ખાલી) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(અક્ષર **)(slab->free_list);
		slab->info.અંક_used++;
		__ASSERT((slab->free_list == ખાલી &&
			  slab->info.અંક_used == slab->info.અંક_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.અંક_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} નહીંતર જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ખાલી;
		result = -ENOMEM;
	} નહીંતર {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, ચાવી, &slab->wait_q, timeout);
		જો (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		પાછા result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, ચાવી);

	પાછા result;
}

શૂન્ય k_mem_slab_free(સંરચના k_mem_slab *slab, શૂન્ય *mem)
{
	જો (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		પાછા;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, મુક્ત, slab);
	જો (unlikely(slab->free_list == ખાલી) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		જો (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, મુક્ત, slab);
			z_reschedule(&slab->lock, ચાવી);
			પાછા;
		}
	}
	*(અક્ષર **) mem = slab->free_list;
	slab->free_list = (અક્ષર *) mem;
	slab->info.અંક_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, મુક્ત, slab);

	k_spin_unlock(&slab->lock, ચાવી);
}

પૂર્ણાંક k_mem_slab_runtime_stats_get(સંરચના k_mem_slab *slab, સંરચના sys_memory_stats *stats)
{
	જો ((slab == ખાલી) || (stats == ખાલી)) {
		પાછા -EINVAL;
	}

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.અંક_used * slab->info.block_size;
	stats->free_bytes = (slab->info.અંક_blocks - slab->info.અંક_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#નહીંતર
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, ચાવી);

	પાછા 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
પૂર્ણાંક k_mem_slab_runtime_stats_reset_max(સંરચના k_mem_slab *slab)
{
	જો (slab == ખાલી) {
		પાછા -EINVAL;
	}

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.અંક_used;

	k_spin_unlock(&slab->lock, ચાવી);

	પાછા 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
