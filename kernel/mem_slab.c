/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>

#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/linker/sections.h>
#समावेश <zephyr/sys/dlist.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/sys/check.h>
#समावेश <zephyr/sys/iterable_sections.h>
#समावेश <zephyr/sys/minmax.h>
#समावेश <string.h>
/* private kernel APIs */
#समावेश <ksched.h>
#समावेश <kernel_internal.h>
#समावेश <scheduler.h>
#समावेश <wait_q.h>

#अगर_घोषित CONFIG_OBJ_CORE_MEM_SLAB
#अगर_घोषित CONFIG_OBJ_CORE_STATS_MEM_SLAB

अटल पूर्णांक k_mem_slab_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	__ASSERT((obj_core != शून्य) && (stats != शून्य), "NULL parameter");

	काष्ठा k_mem_slab *slab;
	k_spinlock_कुंजी_t   कुंजी;

	slab = CONTAINER_OF(obj_core, काष्ठा k_mem_slab, obj_core);
	कुंजी = k_spin_lock(&slab->lock);
	स_नकल(stats, &slab->info, माप(slab->info));
	k_spin_unlock(&slab->lock, कुंजी);

	वापस 0;
}

अटल पूर्णांक k_mem_slab_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	__ASSERT((obj_core != शून्य) && (stats != शून्य), "NULL parameter");

	काष्ठा k_mem_slab *slab;
	k_spinlock_कुंजी_t   कुंजी;
	काष्ठा sys_memory_stats *ptr = stats;

	slab = CONTAINER_OF(obj_core, काष्ठा k_mem_slab, obj_core);
	कुंजी = k_spin_lock(&slab->lock);
	ptr->free_bytes = (slab->info.अंक_blocks - slab->info.अंक_used) *
			  slab->info.block_आकार;
	ptr->allocated_bytes = slab->info.अंक_used * slab->info.block_आकार;
#अगर_घोषित CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_bytes = slab->info.max_used * slab->info.block_आकार;
#अन्यथा
	ptr->max_allocated_bytes = 0;
#पूर्ण_अगर /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->lock, कुंजी);

	वापस 0;
}

अटल पूर्णांक k_mem_slab_stats_reset(काष्ठा k_obj_core *obj_core)
{
	__ASSERT(obj_core != शून्य, "NULL parameter");

	काष्ठा k_mem_slab *slab;
	k_spinlock_कुंजी_t   कुंजी;

	slab = CONTAINER_OF(obj_core, काष्ठा k_mem_slab, obj_core);
	कुंजी = k_spin_lock(&slab->lock);

#अगर_घोषित CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = slab->info.अंक_used;
#पूर्ण_अगर /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, कुंजी);

	वापस 0;
}

अटल काष्ठा k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_आकार = माप(काष्ठा k_mem_slab_info),
	.query_आकार = माप(काष्ठा sys_memory_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = शून्य,
	.enable = शून्य,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#अन्यथा
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#पूर्ण_अगर /* CONFIG_OBJ_CORE_MEM_SLAB */

/**
 * @brief Initialize kernel memory slab subsystem.
 *
 * Perform any initialization of memory slabs that wasn't done at build time.
 * Currently this just involves creating the list of free blocks for each slab.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p slab contains invalid configuration and/or values.
 */
अटल पूर्णांक create_free_list(काष्ठा k_mem_slab *slab)
{
	अक्षर *p;
	माप_प्रकार योग_आकार;

	CHECKIF(slab->info.block_आकार == 0U) {
		वापस -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.block_आकार | (uintptr_t)slab->बफर) &
				(माप(व्योम *) - 1)) != 0U) {
		वापस -EINVAL;
	}

	अगर (आकार_mul_overflow(slab->info.block_आकार, slab->info.अंक_blocks, &योग_आकार)) {
		वापस -EINVAL;
	}
	अगर (आकार_add_overflow((माप_प्रकार)(uintptr_t)slab->बफर, योग_आकार, &योग_आकार)) {
		वापस -EINVAL;
	}

	slab->free_list = शून्य;
	p = (अक्षर *)(योग_आकार - slab->info.block_आकार);

	क्रम (uint32_t i = 0; i < slab->info.अंक_blocks; i++) {
		*(अक्षर **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.block_आकार;
	}

	वापस 0;
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
अटल व्योम init_mem_slab_module(व्योम)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		अगर (create_free_list(slab) < 0) {
			वापस;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

पूर्णांक k_mem_slab_init(काष्ठा k_mem_slab *slab, व्योम *बफर,
		    माप_प्रकार block_आकार, uint32_t अंक_blocks)
{
	पूर्णांक rc;

	slab->info.अंक_blocks = अंक_blocks;
	slab->info.block_आकार = block_आकार;
	slab->बफर = बफर;
	slab->info.अंक_used = 0U;
	slab->lock = (काष्ठा k_spinlock) {};

#अगर_घोषित CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_used = 0U;
#पूर्ण_अगर /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	अगर (rc < 0) {
		जाओ out;
	}

#अगर_घोषित CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_MEM_SLAB */
#अगर_घोषित CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_register(K_OBJ_CORE(slab), &slab->info,
				  माप(काष्ठा k_mem_slab_info));
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
out:
	SYS_PORT_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	वापस rc;
}

अटल bool slab_ptr_is_good(काष्ठा k_mem_slab *slab, स्थिर व्योम *ptr)
{
	अगर (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		वापस true;
	}

	स्थिर अक्षर *p = ptr;
	सूचक_भेद_प्रकार offset = p - slab->बफर;

	वापस (offset >= 0) &&
	       (offset < (slab->info.block_आकार * slab->info.अंक_blocks)) &&
	       ((offset % slab->info.block_आकार) == 0);
}

पूर्णांक k_mem_slab_alloc(काष्ठा k_mem_slab *slab, व्योम **mem, k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&slab->lock);
	पूर्णांक परिणाम;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	अगर (slab->free_list != शून्य) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(अक्षर **)(slab->free_list);
		slab->info.अंक_used++;
		__ASSERT((slab->free_list == शून्य &&
			  slab->info.अंक_used == slab->info.अंक_blocks) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#अगर_घोषित CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_used = max(slab->info.अंक_used,
					  slab->info.max_used);
#पूर्ण_अगर /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		परिणाम = 0;
	} अन्यथा अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = शून्य;
		परिणाम = -ENOMEM;
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		परिणाम = z_pend_curr(&slab->lock, कुंजी, &slab->wait_q, timeout);
		अगर (परिणाम == 0) {
			*mem = _current->base.swap_तथ्य;
		}

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, परिणाम);

		वापस परिणाम;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, परिणाम);

	k_spin_unlock(&slab->lock, कुंजी);

	वापस परिणाम;
}

व्योम k_mem_slab_free(काष्ठा k_mem_slab *slab, व्योम *mem)
{
	अगर (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		वापस;
	}

#अगर_घोषित CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.block_आकार);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&slab->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mem_slab, मुक्त, slab);
	अगर (unlikely(slab->free_list == शून्य) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		अगर (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, मुक्त, slab);
			z_reschedule(&slab->lock, कुंजी);
			वापस;
		}
	}
	*(अक्षर **) mem = slab->free_list;
	slab->free_list = (अक्षर *) mem;
	slab->info.अंक_used--;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mem_slab, मुक्त, slab);

	k_spin_unlock(&slab->lock, कुंजी);
}

पूर्णांक k_mem_slab_runtime_stats_get(काष्ठा k_mem_slab *slab, काष्ठा sys_memory_stats *stats)
{
	अगर ((slab == शून्य) || (stats == शून्य)) {
		वापस -EINVAL;
	}

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&slab->lock);

	stats->allocated_bytes = slab->info.अंक_used * slab->info.block_आकार;
	stats->free_bytes = (slab->info.अंक_blocks - slab->info.अंक_used) *
			    slab->info.block_आकार;
#अगर_घोषित CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_bytes = slab->info.max_used *
				     slab->info.block_आकार;
#अन्यथा
	stats->max_allocated_bytes = 0;
#पूर्ण_अगर /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->lock, कुंजी);

	वापस 0;
}

#अगर_घोषित CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
पूर्णांक k_mem_slab_runtime_stats_reset_max(काष्ठा k_mem_slab *slab)
{
	अगर (slab == शून्य) {
		वापस -EINVAL;
	}

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&slab->lock);

	slab->info.max_used = slab->info.अंक_used;

	k_spin_unlock(&slab->lock, कुंजी);

	वापस 0;
}
#पूर्ण_अगर /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
