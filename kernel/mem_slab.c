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

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_MEM_SLAB
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_STATS_MEM_SLAB

ساكن صحيح k_mem_slab_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_ڪنجي_t   ڪنجي;

	slab = CONTAINER_جو(obj_core, بنية k_mem_slab, obj_core);
	ڪنجي = k_spin_تالو(&slab->تالو);
	يادگيري_نقل(stats, &slab->info, حجم(slab->info));
	k_spin_unlock(&slab->تالو, ڪنجي);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_ڪنجي_t   ڪنجي;
	بنية sys_يادگيري_stats *ptr = stats;

	slab = CONTAINER_جو(obj_core, بنية k_mem_slab, obj_core);
	ڪنجي = k_spin_تالو(&slab->تالو);
	ptr->free_بائيٽون = (slab->info.انگ_بلاڪون - slab->info.انگ_استعمال_ٿيل) *
			  slab->info.بلاڪ_سائيز;
	ptr->allocated_بائيٽون = slab->info.انگ_استعمال_ٿيل * slab->info.بلاڪ_سائيز;
#جيڪڏهن_وصف_ٿيل CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_بائيٽون = slab->info.max_استعمال_ٿيل * slab->info.بلاڪ_سائيز;
#وإلا
	ptr->max_allocated_بائيٽون = 0;
#جيڪڏهن_پورو /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->تالو, ڪنجي);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_reset(بنية k_obj_core *obj_core)
{
	__ASSERT(obj_core != عدم, "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_ڪنجي_t   ڪنجي;

	slab = CONTAINER_جو(obj_core, بنية k_mem_slab, obj_core);
	ڪنجي = k_spin_تالو(&slab->تالو);

#جيڪڏهن_وصف_ٿيل CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_استعمال_ٿيل = slab->info.انگ_استعمال_ٿيل;
#جيڪڏهن_پورو /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->تالو, ڪنجي);

	أرجع 0;
}

ساكن بنية k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_سائيز = حجم(بنية k_mem_slab_info),
	.query_سائيز = حجم(بنية sys_يادگيري_stats),
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
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_MEM_SLAB */

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
	size_t ڪل_سائيز;

	CHECKIF(slab->info.بلاڪ_سائيز == 0U) {
		أرجع -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.بلاڪ_سائيز | (uintptr_t)slab->بفر) &
				(حجم(فراغ *) - 1)) != 0U) {
		أرجع -EINVAL;
	}

	إذا (سائيز_mul_حد_کان_مٿي(slab->info.بلاڪ_سائيز, slab->info.انگ_بلاڪون, &ڪل_سائيز)) {
		أرجع -EINVAL;
	}
	إذا (سائيز_جوڙيو_حد_کان_مٿي((size_t)(uintptr_t)slab->بفر, ڪل_سائيز, &ڪل_سائيز)) {
		أرجع -EINVAL;
	}

	slab->free_list = عدم;
	p = (حرف *)(ڪل_سائيز - slab->info.بلاڪ_سائيز);

	لكل (uint32_t i = 0; i < slab->info.انگ_بلاڪون; i++) {
		*(حرف **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.بلاڪ_سائيز;
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
		    size_t بلاڪ_سائيز, uint32_t انگ_بلاڪون)
{
	صحيح rc;

	slab->info.انگ_بلاڪون = انگ_بلاڪون;
	slab->info.بلاڪ_سائيز = بلاڪ_سائيز;
	slab->بفر = بفر;
	slab->info.انگ_استعمال_ٿيل = 0U;
	slab->تالو = (بنية k_spinlock) {};

#جيڪڏهن_وصف_ٿيل CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_استعمال_ٿيل = 0U;
#جيڪڏهن_پورو /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	إذا (rc < 0) {
		اذهب ٻاهريون;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_MEM_SLAB */
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_رجسٽر(K_OBJ_CORE(slab), &slab->info,
				  حجم(بنية k_mem_slab_info));
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
ٻاهريون:
	SYS_پورٽ_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	أرجع rc;
}

ساكن bool slab_ptr_is_good(بنية k_mem_slab *slab, ثابت فراغ *ptr)
{
	إذا (!IS_ENABLED(CONFIG_MEM_SLAB_اشارو_VALIDATE)) {
		أرجع true;
	}

	ثابت حرف *p = ptr;
	ptrdiff_t وٿي = p - slab->بفر;

	أرجع (وٿي >= 0) &&
	       (وٿي < (slab->info.بلاڪ_سائيز * slab->info.انگ_بلاڪون)) &&
	       ((وٿي % slab->info.بلاڪ_سائيز) == 0);
}

صحيح k_mem_slab_alloc(بنية k_mem_slab *slab, فراغ **mem, k_timeout_t timeout)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&slab->تالو);
	صحيح نتيجو;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	إذا (slab->free_list != عدم) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(حرف **)(slab->free_list);
		slab->info.انگ_استعمال_ٿيل++;
		__ASSERT((slab->free_list == عدم &&
			  slab->info.انگ_استعمال_ٿيل == slab->info.انگ_بلاڪون) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#جيڪڏهن_وصف_ٿيل CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_استعمال_ٿيل = max(slab->info.انگ_استعمال_ٿيل,
					  slab->info.max_استعمال_ٿيل);
#جيڪڏهن_پورو /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		نتيجو = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = عدم;
		نتيجو = -ENOMEM;
	} وإلا {
		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		نتيجو = z_pend_curr(&slab->تالو, ڪنجي, &slab->wait_q, timeout);
		إذا (نتيجو == 0) {
			*mem = _current->بنياد.مٽا_سٽا_معلومات;
		}

		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتيجو);

		أرجع نتيجو;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتيجو);

	k_spin_unlock(&slab->تالو, ڪنجي);

	أرجع نتيجو;
}

فراغ k_mem_slab_free(بنية k_mem_slab *slab, فراغ *mem)
{
	إذا (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		أرجع;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.بلاڪ_سائيز);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&slab->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, حرر, slab);
	إذا (unlikely(slab->free_list == عدم) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		إذا (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);
			z_reschedule(&slab->تالو, ڪنجي);
			أرجع;
		}
	}
	*(حرف **) mem = slab->free_list;
	slab->free_list = (حرف *) mem;
	slab->info.انگ_استعمال_ٿيل--;

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);

	k_spin_unlock(&slab->تالو, ڪنجي);
}

صحيح k_mem_slab_runtime_stats_get(بنية k_mem_slab *slab, بنية sys_يادگيري_stats *stats)
{
	إذا ((slab == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&slab->تالو);

	stats->allocated_بائيٽون = slab->info.انگ_استعمال_ٿيل * slab->info.بلاڪ_سائيز;
	stats->free_بائيٽون = (slab->info.انگ_بلاڪون - slab->info.انگ_استعمال_ٿيل) *
			    slab->info.بلاڪ_سائيز;
#جيڪڏهن_وصف_ٿيل CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_بائيٽون = slab->info.max_استعمال_ٿيل *
				     slab->info.بلاڪ_سائيز;
#وإلا
	stats->max_allocated_بائيٽون = 0;
#جيڪڏهن_پورو /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->تالو, ڪنجي);

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
صحيح k_mem_slab_runtime_stats_reset_max(بنية k_mem_slab *slab)
{
	إذا (slab == عدم) {
		أرجع -EINVAL;
	}

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&slab->تالو);

	slab->info.max_استعمال_ٿيل = slab->info.انگ_استعمال_ٿيل;

	k_spin_unlock(&slab->تالو, ڪنجي);

	أرجع 0;
}
#جيڪڏهن_پورو /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
