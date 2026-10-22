/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <string.h>
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/kernel/obj_core.h>
#अन्तर्भाव <kernel_internal.h>

स्थिर संरचना k_spinlock  obj_core_lock;

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
	पटिनिवत्त atomic_ptr_get(&slot->core);
}

स्थिर bool range_contains(ध्रुव संरचना k_obj_range *range, ध्रुव शून्य *ptr)
{
	सचे (range->indirect) {
		अत्थाय (ध्रुव शून्य *ध्रुव *pp = range->start; pp < (ध्रुव शून्य *ध्रुव *)range->end;
		     pp = (ध्रुव शून्य *ध्रुव *)((ध्रुव uint8_t *)pp + range->stride)) {
			सचे (*pp == ptr) {
				पटिनिवत्त true;
			}
		}
		पटिनिवत्त false;
	}

	पटिनिवत्त (ptr >= range->start) && (ptr < range->end);
}

स्थिर bool type_ranges_contain(ध्रुव संरचना k_obj_type *type, ध्रुव शून्य *ptr)
{
	अत्थाय (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		ध्रुव संरचना k_obj_range *range = &type->statics[i];

		सचे (range->stride == 0U) {
			भङ्ग;
		}
		सचे (range_contains(range, ptr)) {
			पटिनिवत्त true;
		}
	}

	पटिनिवत्त false;
}

/* Object core of the range element */
स्थिर संरचना k_obj_core *range_core(ध्रुव संरचना k_obj_type *type,
				     ध्रुव संरचना k_obj_range *range, ध्रुव शून्य *elem)
{
	ध्रुव uint8_t *obj = range->indirect ? *(ध्रुव uint8_t *ध्रुव *)elem : elem;

	पटिनिवत्त (संरचना k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
स्थिर bool in_राशिः_storage(ध्रुव शून्य *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ध्रुव संरचना k_thread *thread = _current;
	uintptr_t irq_राशिः = (uintptr_t)K_KERNEL_राशिः_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_राशिः_end =
		irq_राशिः + K_KERNEL_राशिः_SIZEOF(z_interrupt_stacks[0]);

	सचे ((thread != रिक्त) && (thread->राशिः_info.size != 0U) &&
	    (addr >= thread->राशिः_info.start) &&
	    (addr < thread->राशिः_info.start + thread->राशिः_info.size)) {
		पटिनिवत्त true;
	}

	पटिनिवत्त (addr >= irq_राशिः) && (addr < irq_राशिः_end);
}

स्थिर संरचना obj_core_slot *slot_find(ध्रुव संरचना k_obj_core *core)
{
	अत्थाय (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		सचे (slot_core(&registry[i]) == core) {
			पटिनिवत्त &registry[i];
		}
	}

	पटिनिवत्त रिक्त;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
स्थिर bool slot_stale(ध्रुव संरचना obj_core_slot *slot, ध्रुव संरचना k_obj_core *core)
{
	पटिनिवत्त core->type != slot->type;
}

स्थिर संरचना obj_core_slot *slot_alloc(शून्य)
{
	संरचना obj_core_slot *slot = slot_find(रिक्त);

	सचे (slot != रिक्त) {
		पटिनिवत्त slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	अत्थाय (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		सचे ((core != रिक्त) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, रिक्त);
			slot = &registry[i];
		}
	}

	पटिनिवत्त slot;
}

संरचना k_obj_type *z_obj_type_init(संरचना k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->ग्रन्थिः);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, परिमाण(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	पटिनिवत्त type;
}

शून्य k_obj_type_init_range(संरचना k_obj_type *type, ध्रुव शून्य *start,
			   ध्रुव शून्य *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

शून्य k_obj_core_init(संरचना k_obj_core *obj_core, संरचना k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = रिक्त;
#endif /* CONFIG_OBJ_CORE_STATS */
}

शून्य k_obj_core_link(संरचना k_obj_core *obj_core)
{
	संरचना k_obj_type *type = obj_core->type;
	संरचना obj_core_slot *slot;

	सचे (type_ranges_contain(type, (ध्रुव uint8_t *)obj_core - type->obj_core_offset)) {
		पटिनिवत्त;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);

	सचे (in_राशिः_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, कुञ्चिका);
		पटिनिवत्त;
	}

	slot = slot_find(obj_core);
	सचे (slot == रिक्त) {
		slot = slot_alloc();
	}

	सचे (slot == रिक्त) {
		type->dropped++;
	} अञ्ञथा {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);
}

शून्य k_obj_core_init_and_link(संरचना k_obj_core *obj_core,
			      संरचना k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

शून्य k_obj_core_unlink(संरचना k_obj_core *obj_core)
{
	अत्थाय (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		सचे (atomic_ptr_cas(&registry[i].core, obj_core, रिक्त)) {
			भङ्ग;
		}
	}
}

शून्य k_obj_core_evict_range(ध्रुव शून्य *addr, size_t len)
{
	ध्रुव uint8_t *start = addr;
	ध्रुव uint8_t *end = start + len;

	अत्थाय (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		सचे ((core != रिक्त) && ((ध्रुव uint8_t *)core >= start) &&
		    ((ध्रुव uint8_t *)core < end)) {
			(शून्य)atomic_ptr_cas(&registry[i].core, core, रिक्त);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
शून्य sys_heap_release_hook(शून्य *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	सचे (k_is_user_context()) {
		पटिनिवत्त;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
स्थिर शून्य obj_core_init_all(शून्य)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->ग्रन्थिः);

		अत्थाय (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			ध्रुव संरचना k_obj_range *range = &type->statics[r];

			सचे (range->stride == 0U) {
				भङ्ग;
			}

			अत्थाय (ध्रुव uint8_t *elem = range->start;
			     elem < (ध्रुव uint8_t *)range->end; elem += range->stride) {
				संरचना k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				सचे ((type->stats_desc != रिक्त) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(शून्य *)((uint8_t *)obj_core -
							 type->obj_core_offset +
							 type->stats_offset),
						type->stats_size);
				}
#endif /* CONFIG_OBJ_CORE_STATS */
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
		सचे (type->id == type_id) {
			पटिनिवत्त type;
		}
	}

	k_spinlock_कुञ्चिका_t  कुञ्चिका = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_ग्रन्थिः(&z_obj_type_list, ग्रन्थिः) {
		संरचना k_obj_type *type = CONTAINER_OF(ग्रन्थिः, संरचना k_obj_type, ग्रन्थिः);

		सचे (type->id == type_id) {
			rv = type;
			भङ्ग;
		}
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
स्थिर पूर्णाङ्क walk_statics(संरचना k_obj_type *type,
			पूर्णाङ्क (*func)(संरचना k_obj_core *obj_core, शून्य *data),
			शून्य *data)
{
	पूर्णाङ्क status = 0;

	अत्थाय (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		ध्रुव संरचना k_obj_range *range = &type->statics[r];

		सचे (range->stride == 0U) {
			भङ्ग;
		}

		अत्थाय (ध्रुव uint8_t *elem = range->start; elem < (ध्रुव uint8_t *)range->end;
		     elem += range->stride) {
			संरचना k_obj_core *obj_core = range_core(type, range, elem);

			सचे (obj_core->type != type) {
				अनुवर्त;
			}

			status = func(obj_core, data);
			सचे (status != 0) {
				भङ्ग;
			}
		}
	}

	पटिनिवत्त status;
}

पूर्णाङ्क k_obj_type_walk_locked(संरचना k_obj_type *type,
			   पूर्णाङ्क (*func)(संरचना k_obj_core *obj_core, शून्य *data),
			   शून्य *data)
{
	पूर्णाङ्क  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	अत्थाय (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		संरचना obj_core_slot *slot = &registry[i];
		k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);
		संरचना k_obj_core *core = slot_core(slot);

		सचे ((core == रिक्त) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, कुञ्चिका);
			अनुवर्त;
		}

		सचे (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, रिक्त);
			k_spin_unlock(&obj_core_lock, कुञ्चिका);
			अनुवर्त;
		}

		k_spin_unlock(&obj_core_lock, कुञ्चिका);

		status = func(core, data);
	}

	पटिनिवत्त status;
}

पूर्णाङ्क k_obj_type_walk_unlocked(संरचना k_obj_type *type,
			   पूर्णाङ्क (*func)(संरचना k_obj_core *obj_core, शून्य *data),
			   शून्य *data)
{
	पूर्णाङ्क  status;

	status = walk_statics(type, func, data);

	अत्थाय (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		सचे ((core == रिक्त) || (registry[i].type != type) ||
		    (core->type != type)) {
			अनुवर्त;
		}

		status = func(core, data);
	}

	पटिनिवत्त status;
}

#ifdef CONFIG_OBJ_CORE_STATS
पूर्णाङ्क k_obj_core_stats_register(संरचना k_obj_core *obj_core, शून्य *stats,
			      size_t stats_len)
{
	पूर्णाङ्क rv;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);

	सचे (obj_core->type->stats_desc == रिक्त) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} अञ्ञथा सचे (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} अञ्ञथा {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}

पूर्णाङ्क k_obj_core_stats_deregister(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);

	सचे (obj_core->type->stats_desc == रिक्त) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} अञ्ञथा {
		obj_core->stats = रिक्त;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}

पूर्णाङ्क k_obj_core_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats,
			 size_t stats_len)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	सचे ((desc == रिक्त) || (desc->raw == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अञ्ञथा सचे ((desc->raw_size != stats_len) || (obj_core->stats == रिक्त)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अञ्ञथा {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}

पूर्णाङ्क k_obj_core_stats_query(संरचना k_obj_core *obj_core, शून्य *stats,
			   size_t stats_len)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	सचे ((desc == रिक्त) || (desc->query == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अञ्ञथा सचे ((desc->query_size != stats_len) || (obj_core->stats == रिक्त)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अञ्ञथा {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}

पूर्णाङ्क k_obj_core_stats_reset(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t  कुञ्चिका = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	सचे ((desc == रिक्त) || (desc->reset == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अञ्ञथा सचे (obj_core->stats == रिक्त) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अञ्ञथा {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}

पूर्णाङ्क k_obj_core_stats_disable(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	सचे ((desc == रिक्त) || (desc->disable == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अञ्ञथा सचे (obj_core->stats == रिक्त) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अञ्ञथा {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}

पूर्णाङ्क k_obj_core_stats_enable(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	सचे ((desc == रिक्त) || (desc->enable == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अञ्ञथा सचे (obj_core->stats == रिक्त) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अञ्ञथा {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुञ्चिका);

	पटिनिवत्त rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
स्थिर संरचना k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
स्थिर संरचना k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = परिमाण(संरचना k_cycle_stats) * CONFIG_MP_MAX_अङ्कः_CPUS,
	.query_size = परिमाण(संरचना k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = रिक्त,
	.disable = रिक्त,
	.enable  = रिक्त,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
स्थिर शून्य init_kernel_obj_core_list(शून्य)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(संरचना z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  परिमाण(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
