/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/sys/dlist.h>
#include <zephyr/init.h>
#include <zephyr/sys/check.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/minmax.h>
#include <string.h>
/* private kernel APIs */
#include <ksched.h>
#include <kernel_internal.h>
#include <scheduler.h>
#include <wait_q.h>

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB

static ຈຳນວນເຕັມ k_mem_slab_stats_raw(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	__ASSERT((obj_core != NULL) && (stats != NULL), "NULL parameter");

	sa_nracanaa k_mem_slab *slab;
	k_spinlock_key_t   key;

	slab = CONTAINER_OF(obj_core, sa_nracanaa k_mem_slab, obj_core);
	key = k_spin_lock(&slab->lock);
	memcpy(stats, &slab->info, parimaa_na(slab->info));
	k_spin_unlock(&slab->lock, key);

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ k_mem_slab_stats_query(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	__ASSERT((obj_core != NULL) && (stats != NULL), "NULL parameter");

	sa_nracanaa k_mem_slab *slab;
	k_spinlock_key_t   key;
	sa_nracanaa sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, sa_nracanaa k_mem_slab, obj_core);
	key = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.num_blocks - slab->info.num_used) *
			  slab->info.block_size;
	ptr->allocated_bytes = slab->info.num_used * slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_size;
#ບໍ່ດັ່ງນັ້ນ
	ptr->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, key);

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ k_mem_slab_stats_reset(sa_nracanaa k_obj_core *obj_core)
{
	__ASSERT(obj_core != NULL, "NULL parameter");

	sa_nracanaa k_mem_slab *slab;
	k_spinlock_key_t   key;

	slab = CONTAINER_OF(obj_core, sa_nracanaa k_mem_slab, obj_core);
	key = k_spin_lock(&slab->lock);

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.num_used;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, key);

	ສົ່ງຄືນ 0;
}

static sa_nracanaa k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_size = parimaa_na(sa_nracanaa k_mem_slab_info),
	.query_size = parimaa_na(sa_nracanaa sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = NULL,
	.enable = NULL,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#ບໍ່ດັ່ງນັ້ນ
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, NULL);
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
static ຈຳນວນເຕັມ create_free_list(sa_nracanaa k_mem_slab *slab)
{
	var_na *p;
	size_t total_size;

	CHECKIF(slab->info.block_size == 0U) {
		ສົ່ງຄືນ -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_size | (uintptr_t)slab->buffer) &
				(parimaa_na(void *) - 1)) != 0U) {
		ສົ່ງຄືນ -EINVAL;
	}

	ຖ້າ (size_mul_overflow(slab->info.block_size, slab->info.num_blocks, &total_size)) {
		ສົ່ງຄືນ -EINVAL;
	}
	ຖ້າ (size_add_overflow((size_t)(uintptr_t)slab->buffer, total_size, &total_size)) {
		ສົ່ງຄືນ -EINVAL;
	}

	slab->free_list = NULL;
	p = (var_na *)(total_size - slab->info.block_size);

	ສຳລັບ (uint32_t i = 0; i < slab->info.num_blocks; i++) {
		*(var_na **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_size;
	}

	ສົ່ງຄືນ 0;
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
static void init_mem_slab_module(void)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		ຖ້າ (create_free_list(slab) < 0) {
			ສົ່ງຄືນ;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

ຈຳນວນເຕັມ k_mem_slab_init(sa_nracanaa k_mem_slab *slab, void *buffer,
		    size_t block_size, uint32_t num_blocks)
{
	ຈຳນວນເຕັມ rc;

	slab->info.num_blocks = num_blocks;
	slab->info.block_size = block_size;
	slab->buffer = buffer;
	slab->info.num_used = 0U;
	slab->lock = (sa_nracanaa k_spinlock) {};

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	ຖ້າ (rc < 0) {
		goto out;
	}

#ifdef CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#endif /* CONFIG_OBJ_CORE_MEM_SLAB */
#ifdef CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  parimaa_na(sa_nracanaa k_mem_slab_info));
#endif /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	ສົ່ງຄືນ rc;
}

static bool slab_ptr_is_good(sa_nracanaa k_mem_slab *slab, const void *ptr)
{
	ຖ້າ (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		ສົ່ງຄືນ true;
	}

	const var_na *p = ptr;
	ptrdiff_t offset = p - slab->buffer;

	ສົ່ງຄືນ (offset >= 0) &&
	       (offset < (slab->info.block_size * slab->info.num_blocks)) &&
	       ((offset % slab->info.block_size) == 0);
}

ຈຳນວນເຕັມ k_mem_slab_alloc(sa_nracanaa k_mem_slab *slab, void **mem, k_timeout_t timeout)
{
	k_spinlock_key_t key = k_spin_lock(&slab->lock);
	ຈຳນວນເຕັມ result;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	ຖ້າ (slab->free_list != NULL) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(var_na **)(slab->free_list);
		slab->info.num_used++;
		__ASSERT((slab->free_list == NULL &&
			  slab->info.num_used == slab->info.num_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.num_used,
					  slab->info.max_used);
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		result = 0;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = NULL;
		result = -ENOMEM;
	} ບໍ່ດັ່ງນັ້ນ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		result = z_pend_curr(&slab->lock, key, &slab->wait_q, timeout);
		ຖ້າ (result == 0) {
			*mem = _current->base.swap_data;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

		ສົ່ງຄືນ result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, result);

	k_spin_unlock(&slab->lock, key);

	ສົ່ງຄືນ result;
}

void k_mem_slab_free(sa_nracanaa k_mem_slab *slab, void *mem)
{
	ຖ້າ (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		ສົ່ງຄືນ;
	}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_size);
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_key_t key = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, free, slab);
	ຖ້າ (unlikely(slab->free_list == NULL) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		ຖ້າ (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, free, slab);
			z_reschedule(&slab->lock, key);
			ສົ່ງຄືນ;
		}
	}
	*(var_na **) mem = slab->free_list;
	slab->free_list = (var_na *) mem;
	slab->info.num_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, free, slab);

	k_spin_unlock(&slab->lock, key);
}

ຈຳນວນເຕັມ k_mem_slab_runtime_stats_get(sa_nracanaa k_mem_slab *slab, sa_nracanaa sys_memory_stats *stats)
{
	ຖ້າ ((slab == NULL) || (stats == NULL)) {
		ສົ່ງຄືນ -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.num_used * slab->info.block_size;
	stats->free_bytes = (slab->info.num_blocks - slab->info.num_used) *
			    slab->info.block_size;
#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_size;
#ບໍ່ດັ່ງນັ້ນ
	stats->max_allocated_bytes = 0;
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, key);

	ສົ່ງຄືນ 0;
}

#ifdef CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
ຈຳນວນເຕັມ k_mem_slab_runtime_stats_reset_max(sa_nracanaa k_mem_slab *slab)
{
	ຖ້າ (slab == NULL) {
		ສົ່ງຄືນ -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.num_used;

	k_spin_unlock(&slab->lock, key);

	ສົ່ງຄືນ 0;
}
#endif /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
