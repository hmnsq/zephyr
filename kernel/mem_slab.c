/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <zephyr/linker/sections.h>
#अन्तर्भाव <zephyr/sys/dlist.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/sys/check.h>
#अन्तर्भाव <zephyr/sys/iterable_sections.h>
#अन्तर्भाव <zephyr/sys/minmax.h>
#अन्तर्भाव <string.h>
/* private kernel APIs */
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

स्थिर पूर्णाङ्क k_mem_slab_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	__ASSERT((obj_core != रिक्त) && (stats != रिक्त), "NULL parameter");

	संरचना k_mem_slab *slab;
	k_spinlock_किल्ली_t   किल्ली;

	slab = CONTAINER_OF(obj_core, संरचना k_mem_slab, obj_core);
	किल्ली = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, परिमाण(slab->info));
	k_spin_unlock(&slab->lock, किल्ली);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क k_mem_slab_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	__ASSERT((obj_core != रिक्त) && (stats != रिक्त), "NULL parameter");

	संरचना k_mem_slab *slab;
	k_spinlock_किल्ली_t   किल्ली;
	संरचना sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, संरचना k_mem_slab, obj_core);
	किल्ली = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.अंक_blocks - slab->info.अंक_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.अंक_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#अन्यथा
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, किल्ली);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क k_mem_slab_stats_reset(संरचना k_obj_core *obj_core)
{
	__ASSERT(obj_core != रिक्त, "NULL parameter");

	संरचना k_mem_slab *slab;
	k_spinlock_किल्ली_t   किल्ली;

	slab = CONTAINER_OF(obj_core, संरचना k_mem_slab, obj_core);
	किल्ली = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.अंक_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, किल्ली);

	प्रत्यागम 0;
}

स्थिर संरचना k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = परिमाण(संरचना k_mem_slab_info),
	.query_size = परिमाण(संरचना sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = रिक्त,
	.enable = रिक्त,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#अन्यथा
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, रिक्त);
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
स्थिर पूर्णाङ्क create_free_list(संरचना k_mem_slab *slab)
{
	वर्ण *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		प्रत्यागम -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(परिमाण(शून्य *) - 1)) != 0U) {
		प्रत्यागम -EINVAL;
	}

	यदि (size_mul_overflow(slab->info.block_size, slab->info.अंक_blocks, &total_size)) {
		प्रत्यागम -EINVAL;
	}
	यदि (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		प्रत्यागम -EINVAL;
	}

	slab->free_list = रिक्त;
	p = (वर्ण *)(total_size - slab->info.block_size);

	प्रति (uint32_t i = 0; i < slab->info.अंक_blocks; i++) {
		*(वर्ण **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	प्रत्यागम 0;
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
स्थिर शून्य init_mem_slab_module(शून्य)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		यदि (create_free_list(slab) < 0) {
			प्रत्यागम;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

पूर्णाङ्क k_mem_slab_init(संरचना k_mem_slab *slab, शून्य *buffer,
		    size_t block_size, uint32_t अंक_blocks)
{
	पूर्णाङ्क rc;

	slab->info.अंक_blocks = अंक_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.अंक_used = 0U;
	slab->lock = (संरचना k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	यदि (rc < 0) {
		गच्छ out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  परिमाण(संरचना k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	प्रत्यागम rc;
}

स्थिर bool slab_ptr_is_good(संरचना k_mem_slab *slab, ध्रुव शून्य *ptr)
{
	यदि (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		प्रत्यागम true;
	}

	ध्रुव वर्ण *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	प्रत्यागम (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.अंक_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

पूर्णाङ्क k_mem_slab_alloc(संरचना k_mem_slab *slab, शून्य **mem, k_timeout_t timeout)
{
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&slab->lock);
	पूर्णाङ्क result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	यदि (slab->free_list != रिक्त) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(वर्ण **)(slab->free_list);
		slab->info.अंक_used++;
		__ASSERT((slab->free_list == रिक्त &&
			  slab->info.अंक_used == slab->info.अंक_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.अंक_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} अन्यथा यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = रिक्त;
		result = -ENOMEM;
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, किल्ली, &slab->wait_q, timeout);
		यदि (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		प्रत्यागम result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, किल्ली);

	प्रत्यागम result;
}

शून्य k_mem_slab_free(संरचना k_mem_slab *slab, शून्य *mem)
{
	यदि (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		प्रत्यागम;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, मुक्त, slab);
	यदि (unlikely(slab->free_list == रिक्त) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		यदि (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, मुक्त, slab);
			z_reschedule(&slab->lock, किल्ली);
			प्रत्यागम;
		}
	}
	*(वर्ण **) mem = slab->free_list;
	slab->free_list = (वर्ण *) mem;
	slab->info.अंक_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, मुक्त, slab);

	k_spin_unlock(&slab->lock, किल्ली);
}

पूर्णाङ्क k_mem_slab_runtime_stats_get(संरचना k_mem_slab *slab, संरचना sys_memory_stats *stats)
{
	यदि ((slab == रिक्त) || (stats == रिक्त)) {
		प्रत्यागम -EINVAL;
	}

	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.अंक_used * slab->info.block_size;
	stats->free_bytes = (slab->info.अंक_blocks - slab->info.अंक_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#अन्यथा
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, किल्ली);

	प्रत्यागम 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
पूर्णाङ्क k_mem_slab_runtime_stats_reset_max(संरचना k_mem_slab *slab)
{
	यदि (slab == रिक्त) {
		प्रत्यागम -EINVAL;
	}

	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.अंक_used;

	k_spin_unlock(&slab->lock, किल्ली);

	प्रत्यागम 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
