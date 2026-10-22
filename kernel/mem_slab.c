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

#که_تعریف_شوی CONFIG_OBJ_CORE_MEM_SLAB
#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_MEM_SLAB

ساكن صحيح k_mem_slab_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلی_t   کلی;

	slab = CONTAINER_د(obj_core, بنية k_mem_slab, obj_core);
	کلی = k_spin_قلف(&slab->قلف);
	حافظه_نقل(stats, &slab->info, حجم(slab->info));
	k_spin_unlock(&slab->قلف, کلی);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلی_t   کلی;
	بنية sys_حافظه_stats *ptr = stats;

	slab = CONTAINER_د(obj_core, بنية k_mem_slab, obj_core);
	کلی = k_spin_قلف(&slab->قلف);
	ptr->free_بایټونه = (slab->info.عدد_بلاکونه - slab->info.عدد_کارول_شوی) *
			  slab->info.بلاک_اندازه;
	ptr->allocated_بایټونه = slab->info.عدد_کارول_شوی * slab->info.بلاک_اندازه;
#که_تعریف_شوی CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_بایټونه = slab->info.max_کارول_شوی * slab->info.بلاک_اندازه;
#وإلا
	ptr->max_allocated_بایټونه = 0;
#د_که_پای /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->قلف, کلی);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_reset(بنية k_obj_core *obj_core)
{
	__ASSERT(obj_core != عدم, "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلی_t   کلی;

	slab = CONTAINER_د(obj_core, بنية k_mem_slab, obj_core);
	کلی = k_spin_قلف(&slab->قلف);

#که_تعریف_شوی CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_کارول_شوی = slab->info.عدد_کارول_شوی;
#د_که_پای /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->قلف, کلی);

	أرجع 0;
}

ساكن بنية k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_اندازه = حجم(بنية k_mem_slab_info),
	.query_اندازه = حجم(بنية sys_حافظه_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = عدم,
	.enable = عدم,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#وإلا
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#د_که_پای /* CONFIG_OBJ_CORE_MEM_SLAB */

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
	size_t ټول_اندازه;

	CHECKIF(slab->info.بلاک_اندازه == 0U) {
		أرجع -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.بلاک_اندازه | (uintptr_t)slab->بفر) &
				(حجم(فراغ *) - 1)) != 0U) {
		أرجع -EINVAL;
	}

	إذا (اندازه_mul_ډکېدنه(slab->info.بلاک_اندازه, slab->info.عدد_بلاکونه, &ټول_اندازه)) {
		أرجع -EINVAL;
	}
	إذا (اندازه_جمع_کړه_ډکېدنه((size_t)(uintptr_t)slab->بفر, ټول_اندازه, &ټول_اندازه)) {
		أرجع -EINVAL;
	}

	slab->free_list = عدم;
	p = (حرف *)(ټول_اندازه - slab->info.بلاک_اندازه);

	لكل (uint32_t i = 0; i < slab->info.عدد_بلاکونه; i++) {
		*(حرف **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.بلاک_اندازه;
	}

	أرجع 0;
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
		إذا (create_free_list(slab) < 0) {
			أرجع;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

صحيح k_mem_slab_init(بنية k_mem_slab *slab, فراغ *بفر,
		    size_t بلاک_اندازه, uint32_t عدد_بلاکونه)
{
	صحيح rc;

	slab->info.عدد_بلاکونه = عدد_بلاکونه;
	slab->info.بلاک_اندازه = بلاک_اندازه;
	slab->بفر = بفر;
	slab->info.عدد_کارول_شوی = 0U;
	slab->قلف = (بنية k_spinlock) {};

#که_تعریف_شوی CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_کارول_شوی = 0U;
#د_که_پای /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	إذا (rc < 0) {
		اذهب بهر;
	}

#که_تعریف_شوی CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#د_که_پای /* CONFIG_OBJ_CORE_MEM_SLAB */
#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_ثبتوونکی(K_OBJ_CORE(slab), &slab->info,
				  حجم(بنية k_mem_slab_info));
#د_که_پای /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
بهر:
	SYS_پورټ_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	أرجع rc;
}

ساكن bool slab_ptr_is_good(بنية k_mem_slab *slab, ثابت فراغ *ptr)
{
	إذا (!IS_ENABLED(CONFIG_MEM_SLAB_اشاره_ګر_VALIDATE)) {
		أرجع true;
	}

	ثابت حرف *p = ptr;
	ptrdiff_t دننه_واټن = p - slab->بفر;

	أرجع (دننه_واټن >= 0) &&
	       (دننه_واټن < (slab->info.بلاک_اندازه * slab->info.عدد_بلاکونه)) &&
	       ((دننه_واټن % slab->info.بلاک_اندازه) == 0);
}

صحيح k_mem_slab_alloc(بنية k_mem_slab *slab, فراغ **mem, k_timeout_t timeout)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&slab->قلف);
	صحيح پایله;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	إذا (slab->free_list != عدم) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(حرف **)(slab->free_list);
		slab->info.عدد_کارول_شوی++;
		__ASSERT((slab->free_list == عدم &&
			  slab->info.عدد_کارول_شوی == slab->info.عدد_بلاکونه) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#که_تعریف_شوی CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_کارول_شوی = max(slab->info.عدد_کارول_شوی,
					  slab->info.max_کارول_شوی);
#د_که_پای /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		پایله = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = عدم;
		پایله = -ENOMEM;
	} وإلا {
		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		پایله = z_pend_curr(&slab->قلف, کلی, &slab->wait_q, timeout);
		إذا (پایله == 0) {
			*mem = _current->بنسټ.بدلون_معلومات;
		}

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, پایله);

		أرجع پایله;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, پایله);

	k_spin_unlock(&slab->قلف, کلی);

	أرجع پایله;
}

فراغ k_mem_slab_free(بنية k_mem_slab *slab, فراغ *mem)
{
	إذا (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		أرجع;
	}

#که_تعریف_شوی CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.بلاک_اندازه);
#د_که_پای /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_کلی_t کلی = k_spin_قلف(&slab->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, حرر, slab);
	إذا (unlikely(slab->free_list == عدم) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		إذا (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);
			z_reschedule(&slab->قلف, کلی);
			أرجع;
		}
	}
	*(حرف **) mem = slab->free_list;
	slab->free_list = (حرف *) mem;
	slab->info.عدد_کارول_شوی--;

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);

	k_spin_unlock(&slab->قلف, کلی);
}

صحيح k_mem_slab_runtime_stats_get(بنية k_mem_slab *slab, بنية sys_حافظه_stats *stats)
{
	إذا ((slab == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

	k_spinlock_کلی_t کلی = k_spin_قلف(&slab->قلف);

	stats->allocated_بایټونه = slab->info.عدد_کارول_شوی * slab->info.بلاک_اندازه;
	stats->free_بایټونه = (slab->info.عدد_بلاکونه - slab->info.عدد_کارول_شوی) *
			    slab->info.بلاک_اندازه;
#که_تعریف_شوی CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_بایټونه = slab->info.max_کارول_شوی *
				     slab->info.بلاک_اندازه;
#وإلا
	stats->max_allocated_بایټونه = 0;
#د_که_پای /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->قلف, کلی);

	أرجع 0;
}

#که_تعریف_شوی CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
صحيح k_mem_slab_runtime_stats_reset_max(بنية k_mem_slab *slab)
{
	إذا (slab == عدم) {
		أرجع -EINVAL;
	}

	k_spinlock_کلی_t کلی = k_spin_قلف(&slab->قلف);

	slab->info.max_کارول_شوی = slab->info.عدد_کارول_شوی;

	k_spin_unlock(&slab->قلف, کلی);

	أرجع 0;
}
#د_که_پای /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
