/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/iterable_sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/minmax.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
/* private kernel APIs */
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MEM_SLAB
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_MEM_SLAB

ਸਥਿਰ ਪੂਰਨ k_mem_slab_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	__ASSERT((obj_core != ਨਿਰਾ) && (stats != ਨਿਰਾ), "NULL parameter");

	ਬਣਤਰ k_mem_slab *slab;
	k_spinlock_ਕੁੰਜੀ_t   ਕੁੰਜੀ;

	slab = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ k_mem_slab, obj_core);
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&slab->ਤਾਲਾ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stats, &slab->info, ਆਕਾਰ(slab->info));
	k_spin_unlock(&slab->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ k_mem_slab_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	__ASSERT((obj_core != ਨਿਰਾ) && (stats != ਨਿਰਾ), "NULL parameter");

	ਬਣਤਰ k_mem_slab *slab;
	k_spinlock_ਕੁੰਜੀ_t   ਕੁੰਜੀ;
	ਬਣਤਰ sys_ਸਮ੍ਰਿਤੀ_stats *ptr = stats;

	slab = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ k_mem_slab, obj_core);
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&slab->ਤਾਲਾ);
	ptr->free_ਬਾਈਟਾਂ = (slab->info.ਅੰਕ_ਖੰਡਾਂ - slab->info.ਅੰਕ_ਵਰਤਿਆ) *
			  slab->info.ਖੰਡ_ਮਾਪ;
	ptr->allocated_ਬਾਈਟਾਂ = slab->info.ਅੰਕ_ਵਰਤਿਆ * slab->info.ਖੰਡ_ਮਾਪ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_ਬਾਈਟਾਂ = slab->info.max_ਵਰਤਿਆ * slab->info.ਖੰਡ_ਮਾਪ;
#ਨਹੀਂਤਾਂ
	ptr->max_allocated_ਬਾਈਟਾਂ = 0;
#ਜੇ_ਅੰਤ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ k_mem_slab_stats_reset(ਬਣਤਰ k_obj_core *obj_core)
{
	__ASSERT(obj_core != ਨਿਰਾ, "NULL parameter");

	ਬਣਤਰ k_mem_slab *slab;
	k_spinlock_ਕੁੰਜੀ_t   ਕੁੰਜੀ;

	slab = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ k_mem_slab, obj_core);
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&slab->ਤਾਲਾ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_ਵਰਤਿਆ = slab->info.ਅੰਕ_ਵਰਤਿਆ;
#ਜੇ_ਅੰਤ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਬਣਤਰ k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ k_mem_slab_info),
	.query_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ sys_ਸਮ੍ਰਿਤੀ_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ਨਿਰਾ,
	.enable = ਨਿਰਾ,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#ਨਹੀਂਤਾਂ
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MEM_SLAB */

/**
 * @brief Initialize kernel memory slab subsystem.
 *
 * Perform any initialization of memory slabs that wasn't done at build time.
 * Currently this just involves creating the list of free blocks for each slab.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p slab contains invalid configuration and/or values.
 */
ਸਥਿਰ ਪੂਰਨ create_free_list(ਬਣਤਰ k_mem_slab *slab)
{
	ਅੱਖਰ *p;
	size_t ਕੁੱਲ_ਮਾਪ;

	CHECKIF(slab->info.ਖੰਡ_ਮਾਪ == 0U) {
		ਵਾਪਸ -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.ਖੰਡ_ਮਾਪ | (uintptr_t)slab->ਬਫਰ) &
				(ਆਕਾਰ(ਖਾਲੀ *) - 1)) != 0U) {
		ਵਾਪਸ -EINVAL;
	}

	ਜੇ (ਮਾਪ_mul_ਉਛਾਲ(slab->info.ਖੰਡ_ਮਾਪ, slab->info.ਅੰਕ_ਖੰਡਾਂ, &ਕੁੱਲ_ਮਾਪ)) {
		ਵਾਪਸ -EINVAL;
	}
	ਜੇ (ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ((size_t)(uintptr_t)slab->ਬਫਰ, ਕੁੱਲ_ਮਾਪ, &ਕੁੱਲ_ਮਾਪ)) {
		ਵਾਪਸ -EINVAL;
	}

	slab->free_list = ਨਿਰਾ;
	p = (ਅੱਖਰ *)(ਕੁੱਲ_ਮਾਪ - slab->info.ਖੰਡ_ਮਾਪ);

	ਲਈ (uint32_t i = 0; i < slab->info.ਅੰਕ_ਖੰਡਾਂ; i++) {
		*(ਅੱਖਰ **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.ਖੰਡ_ਮਾਪ;
	}

	ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ init_mem_slab_module(ਖਾਲੀ)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		ਜੇ (create_free_list(slab) < 0) {
			ਵਾਪਸ;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

ਪੂਰਨ k_mem_slab_init(ਬਣਤਰ k_mem_slab *slab, ਖਾਲੀ *ਬਫਰ,
		    size_t ਖੰਡ_ਮਾਪ, uint32_t ਅੰਕ_ਖੰਡਾਂ)
{
	ਪੂਰਨ rc;

	slab->info.ਅੰਕ_ਖੰਡਾਂ = ਅੰਕ_ਖੰਡਾਂ;
	slab->info.ਖੰਡ_ਮਾਪ = ਖੰਡ_ਮਾਪ;
	slab->ਬਫਰ = ਬਫਰ;
	slab->info.ਅੰਕ_ਵਰਤਿਆ = 0U;
	slab->ਤਾਲਾ = (ਬਣਤਰ k_spinlock) {};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_ਵਰਤਿਆ = 0U;
#ਜੇ_ਅੰਤ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	ਜੇ (rc < 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MEM_SLAB */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_ਰਜਿਸਟਰ(K_OBJ_CORE(slab), &slab->info,
				  ਆਕਾਰ(ਬਣਤਰ k_mem_slab_info));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
ਬਾਹਰਲਾ:
	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	ਵਾਪਸ rc;
}

ਸਥਿਰ bool slab_ptr_is_good(ਬਣਤਰ k_mem_slab *slab, ਅਚਲ ਖਾਲੀ *ptr)
{
	ਜੇ (!IS_ENABLED(CONFIG_MEM_SLAB_ਸੂਚਕ_VALIDATE)) {
		ਵਾਪਸ true;
	}

	ਅਚਲ ਅੱਖਰ *p = ptr;
	ptrdiff_t ਵਿੱਥ = p - slab->ਬਫਰ;

	ਵਾਪਸ (ਵਿੱਥ >= 0) &&
	       (ਵਿੱਥ < (slab->info.ਖੰਡ_ਮਾਪ * slab->info.ਅੰਕ_ਖੰਡਾਂ)) &&
	       ((ਵਿੱਥ % slab->info.ਖੰਡ_ਮਾਪ) == 0);
}

ਪੂਰਨ k_mem_slab_alloc(ਬਣਤਰ k_mem_slab *slab, ਖਾਲੀ **mem, k_timeout_t timeout)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&slab->ਤਾਲਾ);
	ਪੂਰਨ ਨਤੀਜਾ;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	ਜੇ (slab->free_list != ਨਿਰਾ) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(ਅੱਖਰ **)(slab->free_list);
		slab->info.ਅੰਕ_ਵਰਤਿਆ++;
		__ASSERT((slab->free_list == ਨਿਰਾ &&
			  slab->info.ਅੰਕ_ਵਰਤਿਆ == slab->info.ਅੰਕ_ਖੰਡਾਂ) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_ਵਰਤਿਆ = max(slab->info.ਅੰਕ_ਵਰਤਿਆ,
					  slab->info.max_ਵਰਤਿਆ);
#ਜੇ_ਅੰਤ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		ਨਤੀਜਾ = 0;
	} ਨਹੀਂਤਾਂ ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ਨਿਰਾ;
		ਨਤੀਜਾ = -ENOMEM;
	} ਨਹੀਂਤਾਂ {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		ਨਤੀਜਾ = z_pend_curr(&slab->ਤਾਲਾ, ਕੁੰਜੀ, &slab->wait_q, timeout);
		ਜੇ (ਨਤੀਜਾ == 0) {
			*mem = _current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;
		}

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ਨਤੀਜਾ);

		ਵਾਪਸ ਨਤੀਜਾ;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ਨਤੀਜਾ);

	k_spin_unlock(&slab->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ਨਤੀਜਾ;
}

ਖਾਲੀ k_mem_slab_free(ਬਣਤਰ k_mem_slab *slab, ਖਾਲੀ *mem)
{
	ਜੇ (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		ਵਾਪਸ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.ਖੰਡ_ਮਾਪ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&slab->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, ਮੁਕਤ, slab);
	ਜੇ (unlikely(slab->free_list == ਨਿਰਾ) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		ਜੇ (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ਮੁਕਤ, slab);
			z_reschedule(&slab->ਤਾਲਾ, ਕੁੰਜੀ);
			ਵਾਪਸ;
		}
	}
	*(ਅੱਖਰ **) mem = slab->free_list;
	slab->free_list = (ਅੱਖਰ *) mem;
	slab->info.ਅੰਕ_ਵਰਤਿਆ--;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ਮੁਕਤ, slab);

	k_spin_unlock(&slab->ਤਾਲਾ, ਕੁੰਜੀ);
}

ਪੂਰਨ k_mem_slab_runtime_stats_get(ਬਣਤਰ k_mem_slab *slab, ਬਣਤਰ sys_ਸਮ੍ਰਿਤੀ_stats *stats)
{
	ਜੇ ((slab == ਨਿਰਾ) || (stats == ਨਿਰਾ)) {
		ਵਾਪਸ -EINVAL;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&slab->ਤਾਲਾ);

	stats->allocated_ਬਾਈਟਾਂ = slab->info.ਅੰਕ_ਵਰਤਿਆ * slab->info.ਖੰਡ_ਮਾਪ;
	stats->free_ਬਾਈਟਾਂ = (slab->info.ਅੰਕ_ਖੰਡਾਂ - slab->info.ਅੰਕ_ਵਰਤਿਆ) *
			    slab->info.ਖੰਡ_ਮਾਪ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_ਬਾਈਟਾਂ = slab->info.max_ਵਰਤਿਆ *
				     slab->info.ਖੰਡ_ਮਾਪ;
#ਨਹੀਂਤਾਂ
	stats->max_allocated_ਬਾਈਟਾਂ = 0;
#ਜੇ_ਅੰਤ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
ਪੂਰਨ k_mem_slab_runtime_stats_reset_max(ਬਣਤਰ k_mem_slab *slab)
{
	ਜੇ (slab == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&slab->ਤਾਲਾ);

	slab->info.max_ਵਰਤਿਆ = slab->info.ਅੰਕ_ਵਰਤਿਆ;

	k_spin_unlock(&slab->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
