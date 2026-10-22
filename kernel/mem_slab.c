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

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MEM_SLAB
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_MEM_SLAB

स्थिर पूर्णाङ्क k_mem_slab_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	__ASSERT((obj_core != रिक्त) && (stats != रिक्त), "NULL parameter");

	संरचना k_mem_slab *slab;
	k_spinlock_कुञ्चिका_t   कुञ्चिका;

	slab = CONTAINER_सम्बन्धि(obj_core, संरचना k_mem_slab, obj_core);
	कुञ्चिका = k_spin_तालकम्(&slab->तालकम्);
	स्मृतिं_प्रतिलिख(stats, &slab->info, परिमाण(slab->info));
	k_spin_unlock(&slab->तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क k_mem_slab_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	__ASSERT((obj_core != रिक्त) && (stats != रिक्त), "NULL parameter");

	संरचना k_mem_slab *slab;
	k_spinlock_कुञ्चिका_t   कुञ्चिका;
	संरचना sys_स्मृतिः_stats *ptr = stats;

	slab = CONTAINER_सम्बन्धि(obj_core, संरचना k_mem_slab, obj_core);
	कुञ्चिका = k_spin_तालकम्(&slab->तालकम्);
	ptr->free_अष्टकानि = (slab->info.अङ्कः_खण्डानि - slab->info.अङ्कः_प्रयुक्तम्) *
			  slab->info.खण्डम्_आकार;
	ptr->allocated_अष्टकानि = slab->info.अङ्कः_प्रयुक्तम् * slab->info.खण्डम्_आकार;
#यदि_परिभाषितम् CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	ptr->max_allocated_अष्टकानि = slab->info.max_प्रयुक्तम् * slab->info.खण्डम्_आकार;
#अन्यथा
	ptr->max_allocated_अष्टकानि = 0;
#यदि_अन्तः /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
	k_spin_unlock(&slab->तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क k_mem_slab_stats_reset(संरचना k_obj_core *obj_core)
{
	__ASSERT(obj_core != रिक्त, "NULL parameter");

	संरचना k_mem_slab *slab;
	k_spinlock_कुञ्चिका_t   कुञ्चिका;

	slab = CONTAINER_सम्बन्धि(obj_core, संरचना k_mem_slab, obj_core);
	कुञ्चिका = k_spin_तालकम्(&slab->तालकम्);

#यदि_परिभाषितम् CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_प्रयुक्तम् = slab->info.अङ्कः_प्रयुक्तम्;
#यदि_अन्तः /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

स्थिर संरचना k_obj_core_stats_desc mem_slab_stats_desc = {
	.raw_आकार = परिमाण(संरचना k_mem_slab_info),
	.query_आकार = परिमाण(संरचना sys_स्मृतिः_stats),
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
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */
#यदि_अन्तः /* CONFIG_OBJ_CORE_MEM_SLAB */

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
	size_t योग_आकार;

	CHECKIF(slab->info.खण्डम्_आकार == 0U) {
		प्रत्यागम -EINVAL;
	}

	/* blocks must be word aligned */
	CHECKIF(((slab->info.खण्डम्_आकार | (uintptr_t)slab->बफर) &
				(परिमाण(शून्य *) - 1)) != 0U) {
		प्रत्यागम -EINVAL;
	}

	यदि (आकार_mul_अतिप्रवाहः(slab->info.खण्डम्_आकार, slab->info.अङ्कः_खण्डानि, &योग_आकार)) {
		प्रत्यागम -EINVAL;
	}
	यदि (आकार_सङ्कलय_अतिप्रवाहः((size_t)(uintptr_t)slab->बफर, योग_आकार, &योग_आकार)) {
		प्रत्यागम -EINVAL;
	}

	slab->free_list = रिक्त;
	p = (वर्ण *)(योग_आकार - slab->info.खण्डम्_आकार);

	प्रति (uint32_t i = 0; i < slab->info.अङ्कः_खण्डानि; i++) {
		*(वर्ण **)p = slab->free_list;
		slab->free_list = p;
		p -= slab->info.खण्डम्_आकार;
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

पूर्णाङ्क k_mem_slab_init(संरचना k_mem_slab *slab, शून्य *बफर,
		    size_t खण्डम्_आकार, uint32_t अङ्कः_खण्डानि)
{
	पूर्णाङ्क rc;

	slab->info.अङ्कः_खण्डानि = अङ्कः_खण्डानि;
	slab->info.खण्डम्_आकार = खण्डम्_आकार;
	slab->बफर = बफर;
	slab->info.अङ्कः_प्रयुक्तम् = 0U;
	slab->तालकम् = (संरचना k_spinlock) {};

#यदि_परिभाषितम् CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	slab->info.max_प्रयुक्तम् = 0U;
#यदि_अन्तः /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	rc = create_free_list(slab);
	यदि (rc < 0) {
		गच्छ बहिर्गमनम्;
	}

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MEM_SLAB
	k_obj_core_init_and_link(K_OBJ_CORE(slab), &obj_type_mem_slab);
#यदि_अन्तः /* CONFIG_OBJ_CORE_MEM_SLAB */
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_MEM_SLAB
	k_obj_core_stats_पञ्जिकाकोशः(K_OBJ_CORE(slab), &slab->info,
				  परिमाण(संरचना k_mem_slab_info));
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_MEM_SLAB */

	z_waitq_init(&slab->wait_q);
	k_object_init(slab);
बहिर्गमनम्:
	SYS_पत्तनम्_TRACING_OBJ_INIT(k_mem_slab, slab, rc);

	प्रत्यागम rc;
}

स्थिर bool slab_ptr_is_good(संरचना k_mem_slab *slab, ध्रुव शून्य *ptr)
{
	यदि (!IS_ENABLED(CONFIG_MEM_SLAB_POINTER_VALIDATE)) {
		प्रत्यागम true;
	}

	ध्रुव वर्ण *p = ptr;
	ptrdiff_t विस्थापनम् = p - slab->बफर;

	प्रत्यागम (विस्थापनम् >= 0) &&
	       (विस्थापनम् < (slab->info.खण्डम्_आकार * slab->info.अङ्कः_खण्डानि)) &&
	       ((विस्थापनम् % slab->info.खण्डम्_आकार) == 0);
}

पूर्णाङ्क k_mem_slab_alloc(संरचना k_mem_slab *slab, शून्य **mem, k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&slab->तालकम्);
	पूर्णाङ्क परिणाम;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mem_slab, alloc, slab, timeout);

	यदि (slab->free_list != रिक्त) {
		/* take a free block */
		*mem = slab->free_list;
		slab->free_list = *(वर्ण **)(slab->free_list);
		slab->info.अङ्कः_प्रयुक्तम्++;
		__ASSERT((slab->free_list == रिक्त &&
			  slab->info.अङ्कः_प्रयुक्तम् == slab->info.अङ्कः_खण्डानि) ||
			 slab_ptr_is_good(slab, slab->free_list),
			 "slab corruption detected");

#यदि_परिभाषितम् CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
		slab->info.max_प्रयुक्तम् = max(slab->info.अङ्कः_प्रयुक्तम्,
					  slab->info.max_प्रयुक्तम्);
#यदि_अन्तः /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

		परिणाम = 0;
	} अन्यथा यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT) ||
		   !IS_ENABLED(CONFIG_MULTITHREADING)) {
		/* don't wait for a free block to become available */
		*mem = रिक्त;
		परिणाम = -ENOMEM;
	} अन्यथा {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_mem_slab, alloc, slab, timeout);

		/* wait for a free block or timeout */
		परिणाम = z_pend_curr(&slab->तालकम्, कुञ्चिका, &slab->wait_q, timeout);
		यदि (परिणाम == 0) {
			*mem = _current->आधारः.swap_दत्त;
		}

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, परिणाम);

		प्रत्यागम परिणाम;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mem_slab, alloc, slab, timeout, परिणाम);

	k_spin_unlock(&slab->तालकम्, कुञ्चिका);

	प्रत्यागम परिणाम;
}

शून्य k_mem_slab_free(संरचना k_mem_slab *slab, शून्य *mem)
{
	यदि (!slab_ptr_is_good(slab, mem)) {
		__ASSERT(false, "Invalid memory pointer provided");
		k_panic();
		प्रत्यागम;
	}

#यदि_परिभाषितम् CONFIG_OBJ_CORE_EVICT_ON_FREE
	k_obj_core_evict_range(mem, slab->info.खण्डम्_आकार);
#यदि_अन्तः /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&slab->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mem_slab, मुक्त, slab);
	यदि (unlikely(slab->free_list == रिक्त) && IS_ENABLED(CONFIG_MULTITHREADING)) {
		यदि (z_sched_wake(&slab->wait_q, 0, mem)) {
			SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mem_slab, मुक्त, slab);
			z_reschedule(&slab->तालकम्, कुञ्चिका);
			प्रत्यागम;
		}
	}
	*(वर्ण **) mem = slab->free_list;
	slab->free_list = (वर्ण *) mem;
	slab->info.अङ्कः_प्रयुक्तम्--;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mem_slab, मुक्त, slab);

	k_spin_unlock(&slab->तालकम्, कुञ्चिका);
}

पूर्णाङ्क k_mem_slab_runtime_stats_get(संरचना k_mem_slab *slab, संरचना sys_स्मृतिः_stats *stats)
{
	यदि ((slab == रिक्त) || (stats == रिक्त)) {
		प्रत्यागम -EINVAL;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&slab->तालकम्);

	stats->allocated_अष्टकानि = slab->info.अङ्कः_प्रयुक्तम् * slab->info.खण्डम्_आकार;
	stats->free_अष्टकानि = (slab->info.अङ्कः_खण्डानि - slab->info.अङ्कः_प्रयुक्तम्) *
			    slab->info.खण्डम्_आकार;
#यदि_परिभाषितम् CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
	stats->max_allocated_अष्टकानि = slab->info.max_प्रयुक्तम् *
				     slab->info.खण्डम्_आकार;
#अन्यथा
	stats->max_allocated_अष्टकानि = 0;
#यदि_अन्तः /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */

	k_spin_unlock(&slab->तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION
पूर्णाङ्क k_mem_slab_runtime_stats_reset_max(संरचना k_mem_slab *slab)
{
	यदि (slab == रिक्त) {
		प्रत्यागम -EINVAL;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&slab->तालकम्);

	slab->info.max_प्रयुक्तम् = slab->info.अङ्कः_प्रयुक्तम्;

	k_spin_unlock(&slab->तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}
#यदि_अन्तः /* CONFIG_MEM_SLAB_TRACE_MAX_UTILIZATION */
