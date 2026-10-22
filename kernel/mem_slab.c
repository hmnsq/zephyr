/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/linker/sections.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/sys/check.h>
#అన్తర్భావ <zephyr/sys/iterable_sections.h>
#అన్తర్భావ <zephyr/sys/minmax.h>
#అన్తర్భావ <string.h>
/* private kernel APIs */
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <wait_q.h>

#నిర్వచితమైతే CONFIG_OBJ_CORE_MEM_SLAB
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_MEM_SLAB

స్థిర పూర్ణాంకం k_mem_slab_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	__ASSERT((obj_core != ఖాళీ) && (stats != ఖాళీ), "NULL parameter");

	నిర్మాణం k_mem_slab *slab;
	k_spinlock_కీలకం_t   కీలకం;

	slab = CONTAINER_యొక్క(obj_core, నిర్మాణం k_mem_slab, obj_core);
	కీలకం = k_spin_తాళం(&slab->తాళం);
	జ్ఞాపకం_నకలు(stats, &slab->info, పరిమాణం(slab->info));
	k_spin_unlock(&slab->తాళం, కీలకం);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం k_mem_slab_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	__ASSERT((obj_core != ఖాళీ) && (stats != ఖాళీ), "NULL parameter");

	నిర్మాణం k_mem_slab *slab;
	k_spinlock_కీలకం_t   కీలకం;
	నిర్మాణం sys_జ్ఞాపకం_stats *ptr = stats;

	slab = CONTAINER_యొక్క(obj_core, నిర్మాణం k_mem_slab, obj_core);
	కీలకం = k_spin_తాళం(&slab->తాళం);
	ptr->free_బైట్లు = (slab->info.అంకె_ఖండాలు - slab->info.అంకె_వాడిన) *
			  slab->info.ఖండం_కొలత;
	ptr->allocated_బైట్లు = slab->info.అంకె_వాడిన * slab->info.ఖండం_కొలత;
#నిర్వచితమైతే CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_బైట్లు = slab->info.max_వాడిన * slab->info.ఖండం_కొలత;
#లేకపోతే
	ptr->max_allocated_బైట్లు = 0;
#అయితే_ముగింపు /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->తాళం, కీలకం);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం k_mem_slab_stats_reset(నిర్మాణం k_obj_core *obj_core)
{
	__ASSERT(obj_core != ఖాళీ, "NULL parameter");

	నిర్మాణం k_mem_slab *slab;
	k_spinlock_కీలకం_t   కీలకం;

	slab = CONTAINER_యొక్క(obj_core, నిర్మాణం k_mem_slab, obj_core);
	కీలకం = k_spin_తాళం(&slab->తాళం);

#నిర్వచితమైతే CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_వాడిన = slab->info.అంకె_వాడిన;
#అయితే_ముగింపు /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->తాళం, కీలకం);

	తిరిగి 0;
}

స్థిర నిర్మాణం k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_కొలత = పరిమాణం(నిర్మాణం k_mem_slab_info),
	.query_కొలత = పరిమాణం(నిర్మాణం sys_జ్ఞాపకం_stats),
	.raw   = k_mem_slab_stats_raw,
	.query = k_mem_slab_stats_query,
	.reset = k_mem_slab_stats_reset,
	.disable = ఖాళీ,
	.enable = ఖాళీ,
};

K_OBJ_TYPE_DEFINE_STATS(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID,
			&mem_slab_stats_desc, info);
#లేకపోతే
K_OBJ_TYPE_DEFINE(obj_type_mem_slab, k_mem_slab, K_OBJ_TYPE_MEM_SLAB_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MEM_SLAB */

/**
 * @brief Initialize kernel memory slab subsystem.
 *
 * Perform any initialization of memory slabs that wasn't done at build time.
 * Currently this just involves creating the list of free blocks for each slab.
 *
 * @retval 0 on success.
 * @retval -EINVAL if @p slab contains invalid configuration and/or values.
 */
స్థిర పూర్ణాంకం create_free_list(నిర్మాణం k_mem_slab *slab)
{
	అక్షరం *p;
	size_t మొత్తం_కొలత;

	CHECKIF(slab->info.ఖండం_కొలత == 0U) {
		తిరిగి -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.ఖండం_కొలత | (uintptr_t)slab->బఫరు) &
				(పరిమాణం(శూన్య *) - 1)) != 0U) {
		తిరిగి -EINVAL;
	}

	ఒకవేళ (కొలత_mul_అధికప్రవాహం(slab->info.ఖండం_కొలత, slab->info.అంకె_ఖండాలు, &మొత్తం_కొలత)) {
		తిరిగి -EINVAL;
	}
	ఒకవేళ (కొలత_కూడు_అధికప్రవాహం((size_t)(uintptr_t)slab->బఫరు, మొత్తం_కొలత, &మొత్తం_కొలత)) {
		తిరిగి -EINVAL;
	}

	slab->free_list = ఖాళీ;
	p = (అక్షరం *)(మొత్తం_కొలత - slab->info.ఖండం_కొలత);

	కొరకు (uint32_t i = 0; i < slab->info.అంకె_ఖండాలు; i++) {
		*(అక్షరం **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.ఖండం_కొలత;
	}

	తిరిగి 0;
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
స్థిర శూన్య init_mem_slab_module(శూన్య)
{
	STRUCT_SECTION_FOREACH(k_mem_slab, slab) {
		ఒకవేళ (create_free_list(slab) < 0) {
			తిరిగి;
		}
		k_object_init(slab);
	}
}

K_KERNEL_INIT_PRE(init_mem_slab_module);

పూర్ణాంకం k_mem_slab_init(నిర్మాణం k_mem_slab *slab, శూన్య *బఫరు,
		    size_t ఖండం_కొలత, uint32_t అంకె_ఖండాలు)
{
	పూర్ణాంకం rc;

	slab->info.అంకె_ఖండాలు = అంకె_ఖండాలు;
	slab->info.ఖండం_కొలత = ఖండం_కొలత;
	slab->బఫరు = బఫరు;
	slab->info.అంకె_వాడిన = 0U;
	slab->తాళం = (నిర్మాణం k_spinlock) {};

#నిర్వచితమైతే CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_వాడిన = 0U;
#అయితే_ముగింపు /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	ఒకవేళ (rc < 0) {
		వెళ్ళు బహిర్గతం;
	}

#నిర్వచితమైతే CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MEM_SLAB */
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_రిజిస్టరు(K_OBJ_CORE(slab), &slab->info,
				  పరిమాణం(నిర్మాణం k_mem_slab_info));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
బహిర్గతం:
	SYS_పోర్టు_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	తిరిగి rc;
}

స్థిర bool slab_ptr_is_good(నిర్మాణం k_mem_slab *slab, స్థిరరాశి శూన్య *ptr)
{
	ఒకవేళ (!IS_ENABLED(CONFIG_MEM_SLAB_సూచిక_VALIDATE)) {
		తిరిగి true;
	}

	స్థిరరాశి అక్షరం *p = ptr;
	ptrdiff_t స్థానభ్రంశం = p - slab->బఫరు;

	తిరిగి (స్థానభ్రంశం >= 0) &&
	       (స్థానభ్రంశం < (slab->info.ఖండం_కొలత * slab->info.అంకె_ఖండాలు)) &&
	       ((స్థానభ్రంశం % slab->info.ఖండం_కొలత) == 0);
}

పూర్ణాంకం k_mem_slab_alloc(నిర్మాణం k_mem_slab *slab, శూన్య **mem, k_timeout_t timeout)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&slab->తాళం);
	పూర్ణాంకం ఫలితం;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	ఒకవేళ (slab->free_list != ఖాళీ) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(అక్షరం **)(slab->free_list);
		slab->info.అంకె_వాడిన++;
		__ASSERT((slab->free_list == ఖాళీ &&
			  slab->info.అంకె_వాడిన == slab->info.అంకె_ఖండాలు) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#నిర్వచితమైతే CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_వాడిన = max(slab->info.అంకె_వాడిన,
					  slab->info.max_వాడిన);
#అయితే_ముగింపు /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		ఫలితం = 0;
	} లేకపోతే ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = ఖాళీ;
		ఫలితం = -ENOMEM;
	} లేకపోతే {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		ఫలితం = z_pend_curr(&slab->తాళం, కీలకం, &slab->wait_q, timeout);
		ఒకవేళ (ఫలితం == 0) {
			*mem = _current->పునాది.మార్పిడి_దత్తాంశం;
		}

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ఫలితం);

		తిరిగి ఫలితం;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, ఫలితం);

	k_spin_unlock(&slab->తాళం, కీలకం);

	తిరిగి ఫలితం;
}

శూన్య k_mem_slab_free(నిర్మాణం k_mem_slab *slab, శూన్య *mem)
{
	ఒకవేళ (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		తిరిగి;
	}

#నిర్వచితమైతే CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.ఖండం_కొలత);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&slab->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mem_slab, విడుదల, slab);
	ఒకవేళ (unlikely(slab->free_list == ఖాళీ) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		ఒకవేళ (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mem_slab, విడుదల, slab);
			z_reschedule(&slab->తాళం, కీలకం);
			తిరిగి;
		}
	}
	*(అక్షరం **) mem = slab->free_list;
	slab->free_list = (అక్షరం *) mem;
	slab->info.అంకె_వాడిన--;

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mem_slab, విడుదల, slab);

	k_spin_unlock(&slab->తాళం, కీలకం);
}

పూర్ణాంకం k_mem_slab_runtime_stats_get(నిర్మాణం k_mem_slab *slab, నిర్మాణం sys_జ్ఞాపకం_stats *stats)
{
	ఒకవేళ ((slab == ఖాళీ) || (stats == ఖాళీ)) {
		తిరిగి -EINVAL;
	}

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&slab->తాళం);

	stats->allocated_బైట్లు = slab->info.అంకె_వాడిన * slab->info.ఖండం_కొలత;
	stats->free_బైట్లు = (slab->info.అంకె_ఖండాలు - slab->info.అంకె_వాడిన) *
			    slab->info.ఖండం_కొలత;
#నిర్వచితమైతే CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_బైట్లు = slab->info.max_వాడిన *
				     slab->info.ఖండం_కొలత;
#లేకపోతే
	stats->max_allocated_బైట్లు = 0;
#అయితే_ముగింపు /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->తాళం, కీలకం);

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
పూర్ణాంకం k_mem_slab_runtime_stats_reset_max(నిర్మాణం k_mem_slab *slab)
{
	ఒకవేళ (slab == ఖాళీ) {
		తిరిగి -EINVAL;
	}

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&slab->తాళం);

	slab->info.max_వాడిన = slab->info.అంకె_వాడిన;

	k_spin_unlock(&slab->తాళం, కీలకం);

	తిరిగి 0;
}
#అయితే_ముగింపు /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
