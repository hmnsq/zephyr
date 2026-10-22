/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <string.h>
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/kernel/obj_core.h>
#अन्तर्भाव <kernel_internal.h>

स्थिर संरचना k_spinlock  obj_core_तालकम्;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
संरचना obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	संरचना k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
स्थिर संरचना obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

स्थिर संरचना k_obj_core *slot_core(ध्रुव संरचना obj_core_slot *slot)
{
	प्रत्यागम atomic_ptr_get(&slot->core);
}

स्थिर bool range_अन्तर्भवति(ध्रुव संरचना k_obj_range *range, ध्रुव शून्य *ptr)
{
	यदि (range->परोक्षम्) {
		प्रति (ध्रुव शून्य *ध्रुव *pp = range->start; pp < (ध्रुव शून्य *ध्रुव *)range->अन्तः;
		     pp = (ध्रुव शून्य *ध्रुव *)((ध्रुव uint8_t *)pp + range->stride)) {
			यदि (*pp == ptr) {
				प्रत्यागम true;
			}
		}
		प्रत्यागम false;
	}

	प्रत्यागम (ptr >= range->start) && (ptr < range->अन्तः);
}

स्थिर bool type_ranges_contain(ध्रुव संरचना k_obj_type *type, ध्रुव शून्य *ptr)
{
	प्रति (size_t i = 0; i < ARRAY_आकार(type->statics); i++) {
		ध्रुव संरचना k_obj_range *range = &type->statics[i];

		यदि (range->stride == 0U) {
			भङ्ग;
		}
		यदि (range_अन्तर्भवति(range, ptr)) {
			प्रत्यागम true;
		}
	}

	प्रत्यागम false;
}

/* Object core of the range element */
स्थिर संरचना k_obj_core *range_core(ध्रुव संरचना k_obj_type *type,
				     ध्रुव संरचना k_obj_range *range, ध्रुव शून्य *elem)
{
	ध्रुव uint8_t *obj = range->परोक्षम् ? *(ध्रुव uint8_t *ध्रुव *)elem : elem;

	प्रत्यागम (संरचना k_obj_core *)(obj + type->obj_core_विस्थापनम्);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
स्थिर bool अन्तर्_राशिः_storage(ध्रुव शून्य *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ध्रुव संरचना k_thread *thread = _current;
	uintptr_t irq_राशिः = (uintptr_t)K_KERNEL_राशिः_बफर(
		z_व्यवधानम्_stacks[_current_cpu->id]);
	uintptr_t irq_राशिः_अन्तः =
		irq_राशिः + K_KERNEL_राशिः_SIZEOF(z_व्यवधानम्_stacks[0]);

	यदि ((thread != रिक्त) && (thread->राशिः_info.आकार != 0U) &&
	    (addr >= thread->राशिः_info.start) &&
	    (addr < thread->राशिः_info.start + thread->राशिः_info.आकार)) {
		प्रत्यागम true;
	}

	प्रत्यागम (addr >= irq_राशिः) && (addr < irq_राशिः_अन्तः);
}

स्थिर संरचना obj_core_slot *slot_find(ध्रुव संरचना k_obj_core *core)
{
	प्रति (size_t i = 0; i < ARRAY_आकार(registry); i++) {
		यदि (slot_core(&registry[i]) == core) {
			प्रत्यागम &registry[i];
		}
	}

	प्रत्यागम रिक्त;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
स्थिर bool slot_stale(ध्रुव संरचना obj_core_slot *slot, ध्रुव संरचना k_obj_core *core)
{
	प्रत्यागम core->type != slot->type;
}

स्थिर संरचना obj_core_slot *slot_alloc(शून्य)
{
	संरचना obj_core_slot *slot = slot_find(रिक्त);

	यदि (slot != रिक्त) {
		प्रत्यागम slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	प्रति (size_t i = 0; i < ARRAY_आकार(registry); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		यदि ((core != रिक्त) && slot_stale(&registry[i], core)) {
			atomic_ptr_समुच्चयः(&registry[i].core, रिक्त);
			slot = &registry[i];
		}
	}

	प्रत्यागम slot;
}

संरचना k_obj_type *z_obj_type_init(संरचना k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->ग्रन्थिः);
	type->id = id;
	type->obj_core_विस्थापनम् = off;
	स्मृतिं_पूरय(type->statics, 0, परिमाण(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	प्रत्यागम type;
}

शून्य k_obj_type_init_range(संरचना k_obj_type *type, ध्रुव शून्य *start,
			   ध्रुव शून्य *अन्तः, size_t stride, bool परोक्षम्)
{
	type->statics[0].start = start;
	type->statics[0].अन्तः = अन्तः;
	type->statics[0].stride = stride;
	type->statics[0].परोक्षम् = परोक्षम्;
}

शून्य k_obj_core_init(संरचना k_obj_core *obj_core, संरचना k_obj_type *type)
{
	obj_core->type = type;
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS
	obj_core->stats = रिक्त;
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS */
}

शून्य k_obj_core_link(संरचना k_obj_core *obj_core)
{
	संरचना k_obj_type *type = obj_core->type;
	संरचना obj_core_slot *slot;

	यदि (type_ranges_contain(type, (ध्रुव uint8_t *)obj_core - type->obj_core_विस्थापनम्)) {
		प्रत्यागम;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	यदि (अन्तर्_राशिः_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);
		प्रत्यागम;
	}

	slot = slot_find(obj_core);
	यदि (slot == रिक्त) {
		slot = slot_alloc();
	}

	यदि (slot == रिक्त) {
		type->dropped++;
	} अन्यथा {
		slot->type = type;
		atomic_ptr_समुच्चयः(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);
}

शून्य k_obj_core_init_and_link(संरचना k_obj_core *obj_core,
			      संरचना k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

शून्य k_obj_core_unlink(संरचना k_obj_core *obj_core)
{
	प्रति (size_t i = 0; i < ARRAY_आकार(registry); i++) {
		यदि (atomic_ptr_cas(&registry[i].core, obj_core, रिक्त)) {
			भङ्ग;
		}
	}
}

शून्य k_obj_core_evict_range(ध्रुव शून्य *addr, size_t len)
{
	ध्रुव uint8_t *start = addr;
	ध्रुव uint8_t *अन्तः = start + len;

	प्रति (size_t i = 0; i < ARRAY_आकार(registry); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		यदि ((core != रिक्त) && ((ध्रुव uint8_t *)core >= start) &&
		    ((ध्रुव uint8_t *)core < अन्तः)) {
			(शून्य)atomic_ptr_cas(&registry[i].core, core, रिक्त);
		}
	}
}

#यदि_परिभाषितम् CONFIG_OBJ_CORE_EVICT_ON_FREE
शून्य sys_heap_release_hook(शून्य *mem, size_t अष्टकानि)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	यदि (k_is_user_context()) {
		प्रत्यागम;
	}

	k_obj_core_evict_range(mem, अष्टकानि);
}
#यदि_अन्तः /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
स्थिर शून्य obj_core_init_all(शून्य)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->ग्रन्थिः);

		प्रति (size_t r = 0; r < ARRAY_आकार(type->statics); r++) {
			ध्रुव संरचना k_obj_range *range = &type->statics[r];

			यदि (range->stride == 0U) {
				भङ्ग;
			}

			प्रति (ध्रुव uint8_t *elem = range->start;
			     elem < (ध्रुव uint8_t *)range->अन्तः; elem += range->stride) {
				संरचना k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS
				यदि ((type->stats_desc != रिक्त) && (type->stats_आकार != 0)) {
					k_obj_core_stats_पञ्जिकाकोशः(
						obj_core,
						(शून्य *)((uint8_t *)obj_core -
							 type->obj_core_विस्थापनम् +
							 type->stats_विस्थापनम्),
						type->stats_आकार);
				}
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

संरचना k_obj_type *k_obj_type_find(uint32_t type_id)
{
	संरचना k_obj_type *rv = रिक्त;
	sys_snode_t *ग्रन्थिः;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		यदि (type->id == type_id) {
			प्रत्यागम type;
		}
	}

	k_spinlock_कुञ्चिका_t  कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	SYS_SLIST_FOR_EACH_ग्रन्थिः(&z_obj_type_list, ग्रन्थिः) {
		संरचना k_obj_type *type = CONTAINER_सम्बन्धि(ग्रन्थिः, संरचना k_obj_type, ग्रन्थिः);

		यदि (type->id == type_id) {
			rv = type;
			भङ्ग;
		}
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
स्थिर पूर्णाङ्क walk_statics(संरचना k_obj_type *type,
			पूर्णाङ्क (*func)(संरचना k_obj_core *obj_core, शून्य *दत्त),
			शून्य *दत्त)
{
	पूर्णाङ्क स्थितिः = 0;

	प्रति (size_t r = 0; (स्थितिः == 0) && (r < ARRAY_आकार(type->statics)); r++) {
		ध्रुव संरचना k_obj_range *range = &type->statics[r];

		यदि (range->stride == 0U) {
			भङ्ग;
		}

		प्रति (ध्रुव uint8_t *elem = range->start; elem < (ध्रुव uint8_t *)range->अन्तः;
		     elem += range->stride) {
			संरचना k_obj_core *obj_core = range_core(type, range, elem);

			यदि (obj_core->type != type) {
				अनुवर्त;
			}

			स्थितिः = func(obj_core, दत्त);
			यदि (स्थितिः != 0) {
				भङ्ग;
			}
		}
	}

	प्रत्यागम स्थितिः;
}

पूर्णाङ्क k_obj_type_walk_locked(संरचना k_obj_type *type,
			   पूर्णाङ्क (*func)(संरचना k_obj_core *obj_core, शून्य *दत्त),
			   शून्य *दत्त)
{
	पूर्णाङ्क  स्थितिः;

	स्थितिः = walk_statics(type, func, दत्त);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	प्रति (size_t i = 0; (स्थितिः == 0) && (i < ARRAY_आकार(registry)); i++) {
		संरचना obj_core_slot *slot = &registry[i];
		k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);
		संरचना k_obj_core *core = slot_core(slot);

		यदि ((core == रिक्त) || (slot->type != type)) {
			k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);
			अनुवर्त;
		}

		यदि (slot_stale(slot, core)) {
			atomic_ptr_समुच्चयः(&slot->core, रिक्त);
			k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);
			अनुवर्त;
		}

		k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

		स्थितिः = func(core, दत्त);
	}

	प्रत्यागम स्थितिः;
}

पूर्णाङ्क k_obj_type_walk_unlocked(संरचना k_obj_type *type,
			   पूर्णाङ्क (*func)(संरचना k_obj_core *obj_core, शून्य *दत्त),
			   शून्य *दत्त)
{
	पूर्णाङ्क  स्थितिः;

	स्थितिः = walk_statics(type, func, दत्त);

	प्रति (size_t i = 0; (स्थितिः == 0) && (i < ARRAY_आकार(registry)); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		यदि ((core == रिक्त) || (registry[i].type != type) ||
		    (core->type != type)) {
			अनुवर्त;
		}

		स्थितिः = func(core, दत्त);
	}

	प्रत्यागम स्थितिः;
}

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS
पूर्णाङ्क k_obj_core_stats_पञ्जिकाकोशः(संरचना k_obj_core *obj_core, शून्य *stats,
			      size_t stats_len)
{
	पूर्णाङ्क rv;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	यदि (obj_core->type->stats_desc == रिक्त) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} अन्यथा यदि (obj_core->type->stats_desc->raw_आकार != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} अन्यथा {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}

पूर्णाङ्क k_obj_core_stats_deregister(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	यदि (obj_core->type->stats_desc == रिक्त) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} अन्यथा {
		obj_core->stats = रिक्त;
		rv = 0;
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}

पूर्णाङ्क k_obj_core_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats,
			 size_t stats_len)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	desc = obj_core->type->stats_desc;
	यदि ((desc == रिक्त) || (desc->raw == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा यदि ((desc->raw_आकार != stats_len) || (obj_core->stats == रिक्त)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}

पूर्णाङ्क k_obj_core_stats_query(संरचना k_obj_core *obj_core, शून्य *stats,
			   size_t stats_len)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	desc = obj_core->type->stats_desc;
	यदि ((desc == रिक्त) || (desc->query == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा यदि ((desc->query_आकार != stats_len) || (obj_core->stats == रिक्त)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}

पूर्णाङ्क k_obj_core_stats_reset(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t  कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	desc = obj_core->type->stats_desc;
	यदि ((desc == रिक्त) || (desc->reset == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा यदि (obj_core->stats == रिक्त) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}

पूर्णाङ्क k_obj_core_stats_disable(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	desc = obj_core->type->stats_desc;
	यदि ((desc == रिक्त) || (desc->disable == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा यदि (obj_core->stats == रिक्त) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}

पूर्णाङ्क k_obj_core_stats_enable(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_core_तालकम्);

	desc = obj_core->type->stats_desc;
	यदि ((desc == रिक्त) || (desc->enable == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा यदि (obj_core->stats == रिक्त) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_तालकम्, कुञ्चिका);

	प्रत्यागम rv;
}
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_SYSTEM
स्थिर संरचना k_obj_type obj_type_kernel;

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_SYSTEM
स्थिर संरचना k_obj_core_stats_desc kernel_stats_desc = {
	.raw_आकार = परिमाण(संरचना k_आवृत्तिः_stats) * CONFIG_MP_MAX_अङ्कः_CPUS,
	.query_आकार = परिमाण(संरचना k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = रिक्त,
	.disable = रिक्त,
	.enable  = रिक्त,
};
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
स्थिर शून्य init_kernel_obj_core_list(शून्य)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(संरचना z_kernel, obj_core));

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_पञ्जिकाकोशः(K_OBJ_CORE(&_kernel), _kernel.usage,
				  परिमाण(_kernel.usage));
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#यदि_अन्तः /* CONFIG_OBJ_CORE_SYSTEM */
