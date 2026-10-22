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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MEM_SLAB
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_MEM_SLAB

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ k_mem_slab_stats_raw(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	__ASSERT((obj_core != ಖಾಲಿ) && (stats != ಖಾಲಿ), "NULL parameter");

	ರಚನೆ k_mem_slab *slab;
	k_spinlock_ಕೀಲಿ_t   ಕೀಲಿ;

	slab = CONTAINER_ದ(obj_core, ರಚನೆ k_mem_slab, obj_core);
	ಕೀಲಿ = k_spin_ಬೀಗ(&slab->ಬೀಗ);
	ಸ್ಮರಣೆ_ನಕಲು(stats, &slab->info, ಗಾತ್ರ(slab->info));
	k_spin_unlock(&slab->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ k_mem_slab_stats_query(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	__ASSERT((obj_core != ಖಾಲಿ) && (stats != ಖಾಲಿ), "NULL parameter");

	ರಚನೆ k_mem_slab *slab;
	k_spinlock_ಕೀಲಿ_t   ಕೀಲಿ;
	ರಚನೆ sys_ಸ್ಮರಣೆ_stats *ptr = stats;

	slab = CONTAINER_ದ(obj_core, ರಚನೆ k_mem_slab, obj_core);
	ಕೀಲಿ = k_spin_ಬೀಗ(&slab->ಬೀಗ);
	ptr->free_ಬೈಟುಗಳು = (slab->info.ಅಂಕೆ_ಖಂಡಗಳು - slab->info.ಅಂಕೆ_ಬಳಸಿದ) *
			  slab->info.ಖಂಡ_ಗಾತ್ರ;
	ptr->allocated_ಬೈಟುಗಳು = slab->info.ಅಂಕೆ_ಬಳಸಿದ * slab->info.ಖಂಡ_ಗಾತ್ರ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_ಬೈಟುಗಳು = slab->info.max_ಬಳಸಿದ * slab->info.ಖಂಡ_ಗಾತ್ರ;
#ಇಲ್ಲದಿದ್ದರೆ
	ptr->max_allocated_ಬೈಟುಗಳು = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ k_mem_slab_stats_reset(ರಚನೆ k_obj_core *obj_core)
{
	__ASSERT(obj_core != ಖಾಲಿ, "NULL parameter");

	ರಚನೆ k_mem_slab *slab;
	k_spinlock_ಕೀಲಿ_t   ಕೀಲಿ;

	slab = CONTAINER_ದ(obj_core, ರಚನೆ k_mem_slab, obj_core);
	ಕೀಲಿ = k_spin_ಬೀಗ(&slab->ಬೀಗ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_ಬಳಸಿದ = slab->info.ಅಂಕೆ_ಬಳಸಿದ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ರಚನೆ k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ k_mem_slab_info),
	.query_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ sys_ಸ್ಮರಣೆ_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ಖಾಲಿ,
	.enable = ಖಾಲಿ,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#ಇಲ್ಲದಿದ್ದರೆ
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MEM_SLAB */

/**
 * @brief Initialize kernel memory slab subsystem.
 *
 * Perform any initialization of memory slabs that wasn't done at build time.
 * Currently this just involves creating the list of free blocks for each slab.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p slab contains invalid configuration and/or values.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ create_free_list(ರಚನೆ k_mem_slab *slab)
{
	ಅಕ್ಷರ *p;
	size_t ಒಟ್ಟು_ಗಾತ್ರ;

	CHECKIF(slab->info.ಖಂಡ_ಗಾತ್ರ == 0U) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.ಖಂಡ_ಗಾತ್ರ | (uintptr_t)slab->ಬಫರ್) &
				(ಗಾತ್ರ(ಶೂನ್ಯ *) - 1)) != 0U) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಆದರೆ (ಗಾತ್ರ_mul_ಅತಿಪ್ರವಾಹ(slab->info.ಖಂಡ_ಗಾತ್ರ, slab->info.ಅಂಕೆ_ಖಂಡಗಳು, &ಒಟ್ಟು_ಗಾತ್ರ)) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
	ಆದರೆ (ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ((size_t)(uintptr_t)slab->ಬಫರ್, ಒಟ್ಟು_ಗಾತ್ರ, &ಒಟ್ಟು_ಗಾತ್ರ)) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	slab->free_list = ಖಾಲಿ;
	p = (ಅಕ್ಷರ *)(ಒಟ್ಟು_ಗಾತ್ರ - slab->info.ಖಂಡ_ಗಾತ್ರ);

	ಗಾಗಿ (uint32_t i = 0; i < slab->info.ಅಂಕೆ_ಖಂಡಗಳು; i++) {
		*(ಅಕ್ಷರ **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.ಖಂಡ_ಗಾತ್ರ;
	}

	ಹಿಂತಿರುಗಿ 0;
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
		ಆದರೆ (create_free_list(slab) < 0) {
			ಹಿಂತಿರುಗಿ;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

ಪೂರ್ಣಾಂಕ k_mem_slab_init(ರಚನೆ k_mem_slab *slab, ಶೂನ್ಯ *ಬಫರ್,
		    size_t ಖಂಡ_ಗಾತ್ರ, uint32_t ಅಂಕೆ_ಖಂಡಗಳು)
{
	ಪೂರ್ಣಾಂಕ rc;

	slab->info.ಅಂಕೆ_ಖಂಡಗಳು = ಅಂಕೆ_ಖಂಡಗಳು;
	slab->info.ಖಂಡ_ಗಾತ್ರ = ಖಂಡ_ಗಾತ್ರ;
	slab->ಬಫರ್ = ಬಫರ್;
	slab->info.ಅಂಕೆ_ಬಳಸಿದ = 0U;
	slab->ಬೀಗ = (ರಚನೆ k_spinlock) {};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_ಬಳಸಿದ = 0U;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	ಆದರೆ (rc < 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MEM_SLAB */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_ರಿಜಿಸ್ಟರ್(K_OBJ_CORE(slab), &slab->info,
				  ಗಾತ್ರ(ರಚನೆ k_mem_slab_info));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
ಹೊರಹೋಗು:
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ bool slab_ptr_is_good(ರಚನೆ k_mem_slab *slab, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr)
{
	ಆದರೆ (!IS_ENABLED(CONFIG_MEM_SLAB_ಸೂಚಕ_VALIDATE)) {
		ಹಿಂತಿರುಗಿ true;
	}

	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ptr;
	ptrdiff_t ಸ್ಥಾನಾಂತರ = p - slab->ಬಫರ್;

	ಹಿಂತಿರುಗಿ (ಸ್ಥಾನಾಂತರ >= 0) &&
	       (ಸ್ಥಾನಾಂತರ < (slab->info.ಖಂಡ_ಗಾತ್ರ * slab->info.ಅಂಕೆ_ಖಂಡಗಳು)) &&
	       ((ಸ್ಥಾನಾಂತರ % slab->info.ಖಂಡ_ಗಾತ್ರ) == 0);
}

ಪೂರ್ಣಾಂಕ k_mem_slab_alloc(ರಚನೆ k_mem_slab *slab, ಶೂನ್ಯ **mem, k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&slab->ಬೀಗ);
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	ಆದರೆ (slab->free_list != ಖಾಲಿ) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(ಅಕ್ಷರ **)(slab->free_list);
		slab->info.ಅಂಕೆ_ಬಳಸಿದ++;
		__ASSERT((slab->free_list == ಖಾಲಿ &&
			  slab->info.ಅಂಕೆ_ಬಳಸಿದ == slab->info.ಅಂಕೆ_ಖಂಡಗಳು) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_ಬಳಸಿದ = max(slab->info.ಅಂಕೆ_ಬಳಸಿದ,
					  slab->info.max_ಬಳಸಿದ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		ಫಲಿತಾಂಶ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ಖಾಲಿ;
		ಫಲಿತಾಂಶ = -ENOMEM;
	} ಇಲ್ಲದಿದ್ದರೆ {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		ಫಲಿತಾಂಶ = z_pend_curr(&slab->ಬೀಗ, ಕೀಲಿ, &slab->wait_q, timeout);
		ಆದರೆ (ಫಲಿತಾಂಶ == 0) {
			*mem = _current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;
		}

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ಫಲಿತಾಂಶ);

		ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ಫಲಿತಾಂಶ);

	k_spin_unlock(&slab->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಶೂನ್ಯ k_mem_slab_free(ರಚನೆ k_mem_slab *slab, ಶೂನ್ಯ *mem)
{
	ಆದರೆ (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		ಹಿಂತಿರುಗಿ;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.ಖಂಡ_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&slab->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mem_slab, ಬಿಡುಗಡೆ, slab);
	ಆದರೆ (unlikely(slab->free_list == ಖಾಲಿ) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		ಆದರೆ (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ಬಿಡುಗಡೆ, slab);
			z_reschedule(&slab->ಬೀಗ, ಕೀಲಿ);
			ಹಿಂತಿರುಗಿ;
		}
	}
	*(ಅಕ್ಷರ **) mem = slab->free_list;
	slab->free_list = (ಅಕ್ಷರ *) mem;
	slab->info.ಅಂಕೆ_ಬಳಸಿದ--;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mem_slab, ಬಿಡುಗಡೆ, slab);

	k_spin_unlock(&slab->ಬೀಗ, ಕೀಲಿ);
}

ಪೂರ್ಣಾಂಕ k_mem_slab_runtime_stats_get(ರಚನೆ k_mem_slab *slab, ರಚನೆ sys_ಸ್ಮರಣೆ_stats *stats)
{
	ಆದರೆ ((slab == ಖಾಲಿ) || (stats == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&slab->ಬೀಗ);

	stats->allocated_ಬೈಟುಗಳು = slab->info.ಅಂಕೆ_ಬಳಸಿದ * slab->info.ಖಂಡ_ಗಾತ್ರ;
	stats->free_ಬೈಟುಗಳು = (slab->info.ಅಂಕೆ_ಖಂಡಗಳು - slab->info.ಅಂಕೆ_ಬಳಸಿದ) *
			    slab->info.ಖಂಡ_ಗಾತ್ರ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_ಬೈಟುಗಳು = slab->info.max_ಬಳಸಿದ *
				     slab->info.ಖಂಡ_ಗಾತ್ರ;
#ಇಲ್ಲದಿದ್ದರೆ
	stats->max_allocated_ಬೈಟುಗಳು = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
ಪೂರ್ಣಾಂಕ k_mem_slab_runtime_stats_reset_max(ರಚನೆ k_mem_slab *slab)
{
	ಆದರೆ (slab == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&slab->ಬೀಗ);

	slab->info.max_ಬಳಸಿದ = slab->info.ಅಂಕೆ_ಬಳಸಿದ;

	k_spin_unlock(&slab->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
