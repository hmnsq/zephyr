/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <zephyr/linker/sections.h>
#شامل <zephyr/sys/dlist.h>
#شامل <zephyr/init.h>
#شامل <zephyr/sys/check.h>
#شامل <zephyr/sys/iterable_sections.h>
#شامل <zephyr/sys/minmax.h>
#شامل <string.h>
/* private kernel APIs */
#شامل <ksched.h>
#شامل <kernel_internal.h>
#شامل <scheduler.h>
#شامل <wait_q.h>

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_MEM_SLAB
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_MEM_SLAB

ساکن صحیح k_mem_slab_stats_raw(ساخت k_obj_core *obj_core, خالی *stats)
{
	__ASSERT((obj_core != تہی) && (stats != تہی), "NULL parameter");

	ساخت k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_کا(obj_core, ساخت k_mem_slab, obj_core);
	کلید = k_spin_تالا(&slab->تالا);
	حافظہ_نقل(stats, &slab->info, حجم(slab->info));
	k_spin_unlock(&slab->تالا, کلید);

	واپس 0;
}

ساکن صحیح k_mem_slab_stats_query(ساخت k_obj_core *obj_core, خالی *stats)
{
	__ASSERT((obj_core != تہی) && (stats != تہی), "NULL parameter");

	ساخت k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;
	ساخت sys_حافظہ_stats *ptr = stats;

	slab = CONTAINER_کا(obj_core, ساخت k_mem_slab, obj_core);
	کلید = k_spin_تالا(&slab->تالا);
	ptr->free_بائٹیں = (slab->info.عدد_بلاکیں - slab->info.عدد_مستعمل) *
			  slab->info.بلاک_سائز;
	ptr->allocated_بائٹیں = slab->info.عدد_مستعمل * slab->info.بلاک_سائز;
#اگر_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_بائٹیں = slab->info.max_مستعمل * slab->info.بلاک_سائز;
#ورنہ
	ptr->max_allocated_بائٹیں = 0;
#اگر_ختم /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->تالا, کلید);

	واپس 0;
}

ساکن صحیح k_mem_slab_stats_reset(ساخت k_obj_core *obj_core)
{
	__ASSERT(obj_core != تہی, "NULL parameter");

	ساخت k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_کا(obj_core, ساخت k_mem_slab, obj_core);
	کلید = k_spin_تالا(&slab->تالا);

#اگر_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_مستعمل = slab->info.عدد_مستعمل;
#اگر_ختم /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->تالا, کلید);

	واپس 0;
}

ساکن ساخت k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_سائز = حجم(ساخت k_mem_slab_info),
	.query_سائز = حجم(ساخت sys_حافظہ_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = تہی,
	.enable = تہی,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#ورنہ
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#اگر_ختم /* CONFIG_OBJ_CORE_MEM_SLAB */

/**
 * @brief Initialize kernel memory slab subsystem.
 *
 * Perform any initialization of memory slabs that wasn't done at build time.
 * Currently this just involves creating the list of free blocks for each slab.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p slab contains invalid configuration and/or values.
 */
ساکن صحیح create_free_list(ساخت k_mem_slab *slab)
{
	حرف *p;
	size_t کل_سائز;

	CHECKIF(slab->info.بلاک_سائز == 0U) {
		واپس -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.بلاک_سائز | (uintptr_t)slab->بفر) &
				(حجم(خالی *) - 1)) != 0U) {
		واپس -EINVAL;
	}

	اگر (سائز_mul_حد_سے_تجاوز(slab->info.بلاک_سائز, slab->info.عدد_بلاکیں, &کل_سائز)) {
		واپس -EINVAL;
	}
	اگر (سائز_جوڑو_حد_سے_تجاوز((size_t)(uintptr_t)slab->بفر, کل_سائز, &کل_سائز)) {
		واپس -EINVAL;
	}

	slab->free_list = تہی;
	p = (حرف *)(کل_سائز - slab->info.بلاک_سائز);

	برائے (uint32_t i = 0; i < slab->info.عدد_بلاکیں; i++) {
		*(حرف **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.بلاک_سائز;
	}

	واپس 0;
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
ساکن خالی init_mem_slab_module(خالی)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		اگر (create_free_list(slab) < 0) {
			واپس;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

صحیح k_mem_slab_init(ساخت k_mem_slab *slab, خالی *بفر,
		    size_t بلاک_سائز, uint32_t عدد_بلاکیں)
{
	صحیح rc;

	slab->info.عدد_بلاکیں = عدد_بلاکیں;
	slab->info.بلاک_سائز = بلاک_سائز;
	slab->بفر = بفر;
	slab->info.عدد_مستعمل = 0U;
	slab->تالا = (ساخت k_spinlock) {};

#اگر_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_مستعمل = 0U;
#اگر_ختم /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	اگر (rc < 0) {
		جاؤ برآمد;
	}

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#اگر_ختم /* CONFIG_OBJ_CORE_MEM_SLAB */
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(slab), &slab->info,
				  حجم(ساخت k_mem_slab_info));
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
برآمد:
	SYS_پورٹ_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	واپس rc;
}

ساکن bool slab_ptr_is_good(ساخت k_mem_slab *slab, ثابت خالی *ptr)
{
	اگر (!IS_ENABLED(CONFIG_MEM_SLAB_اشارہ_گر_VALIDATE)) {
		واپس true;
	}

	ثابت حرف *p = ptr;
	ptrdiff_t انحراف = p - slab->بفر;

	واپس (انحراف >= 0) &&
	       (انحراف < (slab->info.بلاک_سائز * slab->info.عدد_بلاکیں)) &&
	       ((انحراف % slab->info.بلاک_سائز) == 0);
}

صحیح k_mem_slab_alloc(ساخت k_mem_slab *slab, خالی **mem, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&slab->تالا);
	صحیح نتیجہ;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	اگر (slab->free_list != تہی) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(حرف **)(slab->free_list);
		slab->info.عدد_مستعمل++;
		__ASSERT((slab->free_list == تہی &&
			  slab->info.عدد_مستعمل == slab->info.عدد_بلاکیں) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#اگر_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_مستعمل = max(slab->info.عدد_مستعمل,
					  slab->info.max_مستعمل);
#اگر_ختم /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		نتیجہ = 0;
	} ورنہ اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = تہی;
		نتیجہ = -ENOMEM;
	} ورنہ {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		نتیجہ = z_pend_curr(&slab->تالا, کلید, &slab->wait_q, timeout);
		اگر (نتیجہ == 0) {
			*mem = _current->بنیاد.تبادلہ_ڈیٹا;
		}

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتیجہ);

		واپس نتیجہ;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتیجہ);

	k_spin_unlock(&slab->تالا, کلید);

	واپس نتیجہ;
}

خالی k_mem_slab_free(ساخت k_mem_slab *slab, خالی *mem)
{
	اگر (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		واپس;
	}

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.بلاک_سائز);
#اگر_ختم /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_کلید_t کلید = k_spin_تالا(&slab->تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_mem_slab, آزاد, slab);
	اگر (unlikely(slab->free_list == تہی) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		اگر (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, آزاد, slab);
			z_reschedule(&slab->تالا, کلید);
			واپس;
		}
	}
	*(حرف **) mem = slab->free_list;
	slab->free_list = (حرف *) mem;
	slab->info.عدد_مستعمل--;

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_mem_slab, آزاد, slab);

	k_spin_unlock(&slab->تالا, کلید);
}

صحیح k_mem_slab_runtime_stats_get(ساخت k_mem_slab *slab, ساخت sys_حافظہ_stats *stats)
{
	اگر ((slab == تہی) || (stats == تہی)) {
		واپس -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_تالا(&slab->تالا);

	stats->allocated_بائٹیں = slab->info.عدد_مستعمل * slab->info.بلاک_سائز;
	stats->free_بائٹیں = (slab->info.عدد_بلاکیں - slab->info.عدد_مستعمل) *
			    slab->info.بلاک_سائز;
#اگر_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_بائٹیں = slab->info.max_مستعمل *
				     slab->info.بلاک_سائز;
#ورنہ
	stats->max_allocated_بائٹیں = 0;
#اگر_ختم /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->تالا, کلید);

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
صحیح k_mem_slab_runtime_stats_reset_max(ساخت k_mem_slab *slab)
{
	اگر (slab == تہی) {
		واپس -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_تالا(&slab->تالا);

	slab->info.max_مستعمل = slab->info.عدد_مستعمل;

	k_spin_unlock(&slab->تالا, کلید);

	واپس 0;
}
#اگر_ختم /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
