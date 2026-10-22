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

#اگر_تعریف_شده CONFIG_OBJ_CORE_MEM_SLAB
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_MEM_SLAB

ساكن صحيح k_mem_slab_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_از(obj_core, بنية k_mem_slab, obj_core);
	کلید = k_spin_قفل(&slab->قفل);
	کپی_حافظه(stats, &slab->info, حجم(slab->info));
	k_spin_unlock(&slab->قفل, کلید);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	__ASSERT((obj_core != عدم) && (stats != عدم), "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;
	بنية sys_حافظه_stats *ptr = stats;

	slab = CONTAINER_از(obj_core, بنية k_mem_slab, obj_core);
	کلید = k_spin_قفل(&slab->قفل);
	ptr->free_بایتها = (slab->info.رقم_بلوکها - slab->info.رقم_استفاده_شده) *
			  slab->info.بلوک_اندازه;
	ptr->allocated_بایتها = slab->info.رقم_استفاده_شده * slab->info.بلوک_اندازه;
#اگر_تعریف_شده CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_بایتها = slab->info.max_استفاده_شده * slab->info.بلوک_اندازه;
#وإلا
	ptr->max_allocated_بایتها = 0;
#پایان_اگر /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->قفل, کلید);

	أرجع 0;
}

ساكن صحيح k_mem_slab_stats_reset(بنية k_obj_core *obj_core)
{
	__ASSERT(obj_core != عدم, "NULL parameter");

	بنية k_mem_slab *slab;
	k_spinlock_کلید_t   کلید;

	slab = CONTAINER_از(obj_core, بنية k_mem_slab, obj_core);
	کلید = k_spin_قفل(&slab->قفل);

#اگر_تعریف_شده CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_استفاده_شده = slab->info.رقم_استفاده_شده;
#پایان_اگر /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->قفل, کلید);

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
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#پایان_اگر /* CONFIG_OBJ_CORE_MEM_SLAB */

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
	size_t مجموع_اندازه;

	CHECKIF(slab->info.بلوک_اندازه == 0U) {
		أرجع -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.بلوک_اندازه | (uintptr_t)slab->میانگیر) &
				(حجم(فراغ *) - 1)) != 0U) {
		أرجع -EINVAL;
	}

	إذا (اندازه_mul_سرریز(slab->info.بلوک_اندازه, slab->info.رقم_بلوکها, &مجموع_اندازه)) {
		أرجع -EINVAL;
	}
	إذا (اندازه_جمع_کن_سرریز((size_t)(uintptr_t)slab->میانگیر, مجموع_اندازه, &مجموع_اندازه)) {
		أرجع -EINVAL;
	}

	slab->free_list = عدم;
	p = (حرف *)(مجموع_اندازه - slab->info.بلوک_اندازه);

	لكل (uint32_t i = 0; i < slab->info.رقم_بلوکها; i++) {
		*(حرف **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.بلوک_اندازه;
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

صحيح k_mem_slab_init(بنية k_mem_slab *slab, فراغ *میانگیر,
		    size_t بلوک_اندازه, uint32_t رقم_بلوکها)
{
	صحيح rc;

	slab->info.رقم_بلوکها = رقم_بلوکها;
	slab->info.بلوک_اندازه = بلوک_اندازه;
	slab->میانگیر = میانگیر;
	slab->info.رقم_استفاده_شده = 0U;
	slab->قفل = (بنية k_spinlock) {};

#اگر_تعریف_شده CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_استفاده_شده = 0U;
#پایان_اگر /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	إذا (rc < 0) {
		اذهب بیرون;
	}

#اگر_تعریف_شده CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#پایان_اگر /* CONFIG_OBJ_CORE_MEM_SLAB */
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_ثبات(K_OBJ_CORE(slab), &slab->info,
				  حجم(بنية k_mem_slab_info));
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
بیرون:
	SYS_پورت_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	أرجع rc;
}

ساكن bool slab_ptr_is_good(بنية k_mem_slab *slab, ثابت فراغ *ptr)
{
	إذا (!IS_ENABLED(CONFIG_MEM_SLAB_اشاره_گر_VALIDATE)) {
		أرجع true;
	}

	ثابت حرف *p = ptr;
	ptrdiff_t جابه_جایی = p - slab->میانگیر;

	أرجع (جابه_جایی >= 0) &&
	       (جابه_جایی < (slab->info.بلوک_اندازه * slab->info.رقم_بلوکها)) &&
	       ((جابه_جایی % slab->info.بلوک_اندازه) == 0);
}

صحيح k_mem_slab_alloc(بنية k_mem_slab *slab, فراغ **mem, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_قفل(&slab->قفل);
	صحيح نتیجه;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	إذا (slab->free_list != عدم) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(حرف **)(slab->free_list);
		slab->info.رقم_استفاده_شده++;
		__ASSERT((slab->free_list == عدم &&
			  slab->info.رقم_استفاده_شده == slab->info.رقم_بلوکها) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#اگر_تعریف_شده CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_استفاده_شده = max(slab->info.رقم_استفاده_شده,
					  slab->info.max_استفاده_شده);
#پایان_اگر /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		نتیجه = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = عدم;
		نتیجه = -ENOMEM;
	} وإلا {
		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		نتیجه = z_pend_curr(&slab->قفل, کلید, &slab->wait_q, timeout);
		إذا (نتیجه == 0) {
			*mem = _current->پایه.جابجا_داده;
		}

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتیجه);

		أرجع نتیجه;
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, نتیجه);

	k_spin_unlock(&slab->قفل, کلید);

	أرجع نتیجه;
}

فراغ k_mem_slab_free(بنية k_mem_slab *slab, فراغ *mem)
{
	إذا (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		أرجع;
	}

#اگر_تعریف_شده CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.بلوک_اندازه);
#پایان_اگر /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_کلید_t کلید = k_spin_قفل(&slab->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_mem_slab, حرر, slab);
	إذا (unlikely(slab->free_list == عدم) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		إذا (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);
			z_reschedule(&slab->قفل, کلید);
			أرجع;
		}
	}
	*(حرف **) mem = slab->free_list;
	slab->free_list = (حرف *) mem;
	slab->info.رقم_استفاده_شده--;

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mem_slab, حرر, slab);

	k_spin_unlock(&slab->قفل, کلید);
}

صحيح k_mem_slab_runtime_stats_get(بنية k_mem_slab *slab, بنية sys_حافظه_stats *stats)
{
	إذا ((slab == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_قفل(&slab->قفل);

	stats->allocated_بایتها = slab->info.رقم_استفاده_شده * slab->info.بلوک_اندازه;
	stats->free_بایتها = (slab->info.رقم_بلوکها - slab->info.رقم_استفاده_شده) *
			    slab->info.بلوک_اندازه;
#اگر_تعریف_شده CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_بایتها = slab->info.max_استفاده_شده *
				     slab->info.بلوک_اندازه;
#وإلا
	stats->max_allocated_بایتها = 0;
#پایان_اگر /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->قفل, کلید);

	أرجع 0;
}

#اگر_تعریف_شده CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
صحيح k_mem_slab_runtime_stats_reset_max(بنية k_mem_slab *slab)
{
	إذا (slab == عدم) {
		أرجع -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_قفل(&slab->قفل);

	slab->info.max_استفاده_شده = slab->info.رقم_استفاده_شده;

	k_spin_unlock(&slab->قفل, کلید);

	أرجع 0;
}
#پایان_اگر /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
