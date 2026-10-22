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

#جے_تعریف_شدہ CONFIG_OBJ_CORE_MEM_SLAB
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_MEM_SLAB

ساكن صحيح k_mem_slab_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_دا(obj_core, بنية k_mem_slab, obj_core);
	کلید = k_spin_جندرا(&slab->جندرا);
	حافظہ_نقل(stats, &slab->info, حجم(slab->info));
	k_spin_unlock(&slab->جندرا, کلید);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;
	بنية sys_حافظہ_stats *ptr = stats;

	slab = CONTAINER_دا(obj_core, بنية k_mem_slab, obj_core);
	کلید = k_spin_جندرا(&slab->جندرا);
	ptr->free_بائٹاں = (slab->info.عدد_بلاکاں - slab->info.عدد_ورتیا) *
			  slab->info.بلاک_ناپ;
	ptr->allocated_بائٹاں = slab->info.عدد_ورتیا * slab->info.بلاک_ناپ;
#جے_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_بائٹاں = slab->info.max_ورتیا * slab->info.بلاک_ناپ;
#وإلا
	ptr->max_allocated_بائٹاں = 0;
#جے_انت /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->جندرا, کلید);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_reset(بنية k_obj_core *obj_core)
{
	__ASSERT(obj_core != عدم, "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_دا(obj_core, بنية k_mem_slab, obj_core);
	کلید = k_spin_جندرا(&slab->جندرا);

#جے_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_ورتیا = slab->info.عدد_ورتیا;
#جے_انت /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->جندرا, کلید);

	أرجع 0;
}

ساكن بنية k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_ناپ = حجم(بنية k_mem_slab_info),
	.query_ناپ = حجم(بنية sys_حافظہ_stats),
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
#جے_انت /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#جے_انت /* CONFIG_OBJ_CORE_MEM_SLAB */

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
	size_t کل_ناپ;

	CHECKIF(slab->info.بلاک_ناپ == 0U) {
		أرجع -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.بلاک_ناپ | (uintptr_t)slab->بفر) &
				(حجم(فراغ *) - 1)) != 0U) {
		أرجع -EINVAL;
	}

	إذا (ناپ_mul_حد_توں_ودھ(slab->info.بلاک_ناپ, slab->info.عدد_بلاکاں, &کل_ناپ)) {
		أرجع -EINVAL;
	}
	إذا (ناپ_جوڑو_حد_توں_ودھ((size_t)(uintptr_t)slab->بفر, کل_ناپ, &کل_ناپ)) {
		أرجع -EINVAL;
	}

	slab->free_list = عدم;
	p = (حرف *)(کل_ناپ - slab->info.بلاک_ناپ);

	لكل (uint32_t i = 0; i < slab->info.عدد_بلاکاں; i++) {
		*(حرف **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.بلاک_ناپ;
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
		    size_t بلاک_ناپ, uint32_t عدد_بلاکاں)
{
	صحيح rc;

	slab->info.عدد_بلاکاں = عدد_بلاکاں;
	slab->info.بلاک_ناپ = بلاک_ناپ;
	slab->بفر = بفر;
	slab->info.عدد_ورتیا = 0U;
	slab->جندرا = (بنية k_spinlock) {};

#جے_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_ورتیا = 0U;
#جے_انت /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	إذا (rc < 0) {
		اذهب باہرلا;
	}

#جے_تعریف_شدہ CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#جے_انت /* CONFIG_OBJ_CORE_MEM_SLAB */
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(slab), &slab->info,
				  حجم(بنية k_mem_slab_info));
#جے_انت /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
باہرلا:
	SYS_پورٹ_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	أرجع rc;
}

ساكن bool slab_ptr_is_good(بنية k_mem_slab *slab, ثابت فراغ *ptr)
{
	إذا (!IS_ENABLED(CONFIG_MEM_SLAB_اشارہ_گر_VALIDATE)) {
		أرجع true;
	}

	ثابت حرف *p = ptr;
	ptrdiff_t وتھ = p - slab->بفر;

	أرجع (وتھ >= 0) &&
	       (وتھ < (slab->info.بلاک_ناپ * slab->info.عدد_بلاکاں)) &&
	       ((وتھ % slab->info.بلاک_ناپ) == 0);
}

صحيح k_mem_slab_alloc(بنية k_mem_slab *slab, فراغ **mem, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&slab->جندرا);
	صحيح نتیجہ;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	إذا (slab->free_list != عدم) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(حرف **)(slab->free_list);
		slab->info.عدد_ورتیا++;
		__ASSERT((slab->free_list == عدم &&
			  slab->info.عدد_ورتیا == slab->info.عدد_بلاکاں) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#جے_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_ورتیا = max(slab->info.عدد_ورتیا,
					  slab->info.max_ورتیا);
#جے_انت /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		نتیجہ = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = عدم;
		نتیجہ = -ENOMEM;
	} وإلا {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		نتیجہ = z_pend_curr(&slab->جندرا, کلید, &slab->wait_q, timeout);
		إذا (نتیجہ == 0) {
			*mem = _current->ادھار.وٹاندرا_انکڑے;
		}

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتیجہ);

		أرجع نتیجہ;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتیجہ);

	k_spin_unlock(&slab->جندرا, کلید);

	أرجع نتیجہ;
}

فراغ k_mem_slab_free(بنية k_mem_slab *slab, فراغ *mem)
{
	إذا (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		أرجع;
	}

#جے_تعریف_شدہ CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.بلاک_ناپ);
#جے_انت /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_کلید_t کلید = k_spin_جندرا(&slab->جندرا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, حرر, slab);
	إذا (unlikely(slab->free_list == عدم) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		إذا (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);
			z_reschedule(&slab->جندرا, کلید);
			أرجع;
		}
	}
	*(حرف **) mem = slab->free_list;
	slab->free_list = (حرف *) mem;
	slab->info.عدد_ورتیا--;

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);

	k_spin_unlock(&slab->جندرا, کلید);
}

صحيح k_mem_slab_runtime_stats_get(بنية k_mem_slab *slab, بنية sys_حافظہ_stats *stats)
{
	إذا ((slab == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_جندرا(&slab->جندرا);

	stats->allocated_بائٹاں = slab->info.عدد_ورتیا * slab->info.بلاک_ناپ;
	stats->free_بائٹاں = (slab->info.عدد_بلاکاں - slab->info.عدد_ورتیا) *
			    slab->info.بلاک_ناپ;
#جے_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_بائٹاں = slab->info.max_ورتیا *
				     slab->info.بلاک_ناپ;
#وإلا
	stats->max_allocated_بائٹاں = 0;
#جے_انت /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->جندرا, کلید);

	أرجع 0;
}

#جے_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
صحيح k_mem_slab_runtime_stats_reset_max(بنية k_mem_slab *slab)
{
	إذا (slab == عدم) {
		أرجع -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_جندرا(&slab->جندرا);

	slab->info.max_ورتیا = slab->info.عدد_ورتیا;

	k_spin_unlock(&slab->جندرا, کلید);

	أرجع 0;
}
#جے_انت /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
