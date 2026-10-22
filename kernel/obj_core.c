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

स्थिर संरचना k_obj_core *slot_core(अचर संरचना obj_core_slot *slot)
{
	फर्काऊ atomic_ptr_get(&slot->core);
}

स्थिर bool range_contains(अचर संरचना k_obj_range *range, अचर शून्य *ptr)
{
	यदि (range->indirect) {
		लागि (अचर शून्य *अचर *pp = range->start; pp < (अचर शून्य *अचर *)range->end;
		     pp = (अचर शून्य *अचर *)((अचर uint8_t *)pp + range->stride)) {
			यदि (*pp == ptr) {
				फर्काऊ true;
			}
		}
		फर्काऊ false;
	}

	फर्काऊ (ptr >= range->start) && (ptr < range->end);
}

स्थिर bool type_ranges_contain(अचर संरचना k_obj_type *type, अचर शून्य *ptr)
{
	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(type->statics); i++) {
		अचर संरचना k_obj_range *range = &type->statics[i];

		यदि (range->stride == 0U) {
			रोक;
		}
		यदि (range_contains(range, ptr)) {
			फर्काऊ true;
		}
	}

	फर्काऊ false;
}

/* Object core of the range element */
स्थिर संरचना k_obj_core *range_core(अचर संरचना k_obj_type *type,
				     अचर संरचना k_obj_range *range, अचर शून्य *elem)
{
	अचर uint8_t *obj = range->indirect ? *(अचर uint8_t *अचर *)elem : elem;

	फर्काऊ (संरचना k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
स्थिर bool in_थाक_storage(अचर शून्य *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	अचर संरचना k_thread *thread = _current;
	uintptr_t irq_थाक = (uintptr_t)K_KERNEL_थाक_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_थाक_end =
		irq_थाक + K_KERNEL_थाक_SIZEOF(z_interrupt_stacks[0]);

	यदि ((thread != रिक्त) && (thread->थाक_info.size != 0U) &&
	    (addr >= thread->थाक_info.start) &&
	    (addr < thread->थाक_info.start + thread->थाक_info.size)) {
		फर्काऊ true;
	}

	फर्काऊ (addr >= irq_थाक) && (addr < irq_थाक_end);
}

स्थिर संरचना obj_core_slot *slot_find(अचर संरचना k_obj_core *core)
{
	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(registry); i++) {
		यदि (slot_core(&registry[i]) == core) {
			फर्काऊ &registry[i];
		}
	}

	फर्काऊ रिक्त;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
स्थिर bool slot_stale(अचर संरचना obj_core_slot *slot, अचर संरचना k_obj_core *core)
{
	फर्काऊ core->type != slot->type;
}

स्थिर संरचना obj_core_slot *slot_alloc(शून्य)
{
	संरचना obj_core_slot *slot = slot_find(रिक्त);

	यदि (slot != रिक्त) {
		फर्काऊ slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(registry); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		यदि ((core != रिक्त) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, रिक्त);
			slot = &registry[i];
		}
	}

	फर्काऊ slot;
}

संरचना k_obj_type *z_obj_type_init(संरचना k_obj_type *type,
				   uint32_t id, माप_प्रकार off)
{
	sys_slist_append(&z_obj_type_list, &type->गाँठो);
	type->id = id;
	type->obj_core_offset = off;
	स_रखो(type->statics, 0, परिमाण(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	फर्काऊ type;
}

शून्य k_obj_type_init_range(संरचना k_obj_type *type, अचर शून्य *start,
			   अचर शून्य *end, माप_प्रकार stride, bool indirect)
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

	यदि (type_ranges_contain(type, (अचर uint8_t *)obj_core - type->obj_core_offset)) {
		फर्काऊ;
	}

	k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);

	यदि (in_थाक_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, साँचो);
		फर्काऊ;
	}

	slot = slot_find(obj_core);
	यदि (slot == रिक्त) {
		slot = slot_alloc();
	}

	यदि (slot == रिक्त) {
		type->dropped++;
	} अन्यथा {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, साँचो);
}

शून्य k_obj_core_init_and_link(संरचना k_obj_core *obj_core,
			      संरचना k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

शून्य k_obj_core_unlink(संरचना k_obj_core *obj_core)
{
	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(registry); i++) {
		यदि (atomic_ptr_cas(&registry[i].core, obj_core, रिक्त)) {
			रोक;
		}
	}
}

शून्य k_obj_core_evict_range(अचर शून्य *addr, माप_प्रकार len)
{
	अचर uint8_t *start = addr;
	अचर uint8_t *end = start + len;

	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(registry); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		यदि ((core != रिक्त) && ((अचर uint8_t *)core >= start) &&
		    ((अचर uint8_t *)core < end)) {
			(शून्य)atomic_ptr_cas(&registry[i].core, core, रिक्त);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
शून्य sys_heap_release_hook(शून्य *mem, माप_प्रकार bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	यदि (k_is_user_context()) {
		फर्काऊ;
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
		sys_slist_append(&z_obj_type_list, &type->गाँठो);

		लागि (माप_प्रकार r = 0; r < ARRAY_SIZE(type->statics); r++) {
			अचर संरचना k_obj_range *range = &type->statics[r];

			यदि (range->stride == 0U) {
				रोक;
			}

			लागि (अचर uint8_t *elem = range->start;
			     elem < (अचर uint8_t *)range->end; elem += range->stride) {
				संरचना k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				यदि ((type->stats_desc != रिक्त) && (type->stats_size != 0)) {
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
	sys_snode_t *गाँठो;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		यदि (type->id == type_id) {
			फर्काऊ type;
		}
	}

	k_spinlock_साँचो_t  साँचो = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_गाँठो(&z_obj_type_list, गाँठो) {
		संरचना k_obj_type *type = CONTAINER_OF(गाँठो, संरचना k_obj_type, गाँठो);

		यदि (type->id == type_id) {
			rv = type;
			रोक;
		}
	}

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
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

	लागि (माप_प्रकार r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		अचर संरचना k_obj_range *range = &type->statics[r];

		यदि (range->stride == 0U) {
			रोक;
		}

		लागि (अचर uint8_t *elem = range->start; elem < (अचर uint8_t *)range->end;
		     elem += range->stride) {
			संरचना k_obj_core *obj_core = range_core(type, range, elem);

			यदि (obj_core->type != type) {
				जारी;
			}

			status = func(obj_core, data);
			यदि (status != 0) {
				रोक;
			}
		}
	}

	फर्काऊ status;
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
	लागि (माप_प्रकार i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		संरचना obj_core_slot *slot = &registry[i];
		k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);
		संरचना k_obj_core *core = slot_core(slot);

		यदि ((core == रिक्त) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, साँचो);
			जारी;
		}

		यदि (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, रिक्त);
			k_spin_unlock(&obj_core_lock, साँचो);
			जारी;
		}

		k_spin_unlock(&obj_core_lock, साँचो);

		status = func(core, data);
	}

	फर्काऊ status;
}

पूर्णाङ्क k_obj_type_walk_unlocked(संरचना k_obj_type *type,
			   पूर्णाङ्क (*func)(संरचना k_obj_core *obj_core, शून्य *data),
			   शून्य *data)
{
	पूर्णाङ्क  status;

	status = walk_statics(type, func, data);

	लागि (माप_प्रकार i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		संरचना k_obj_core *core = slot_core(&registry[i]);

		यदि ((core == रिक्त) || (registry[i].type != type) ||
		    (core->type != type)) {
			जारी;
		}

		status = func(core, data);
	}

	फर्काऊ status;
}

#ifdef CONFIG_OBJ_CORE_STATS
पूर्णाङ्क k_obj_core_stats_register(संरचना k_obj_core *obj_core, शून्य *stats,
			      माप_प्रकार stats_len)
{
	पूर्णाङ्क rv;
	k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);

	यदि (obj_core->type->stats_desc == रिक्त) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} अन्यथा यदि (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} अन्यथा {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
}

पूर्णाङ्क k_obj_core_stats_deregister(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);

	यदि (obj_core->type->stats_desc == रिक्त) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} अन्यथा {
		obj_core->stats = रिक्त;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
}

पूर्णाङ्क k_obj_core_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats,
			 माप_प्रकार stats_len)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	यदि ((desc == रिक्त) || (desc->raw == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा यदि ((desc->raw_size != stats_len) || (obj_core->stats == रिक्त)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
}

पूर्णाङ्क k_obj_core_stats_query(संरचना k_obj_core *obj_core, शून्य *stats,
			   माप_प्रकार stats_len)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	यदि ((desc == रिक्त) || (desc->query == रिक्त)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा यदि ((desc->query_size != stats_len) || (obj_core->stats == रिक्त)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
}

पूर्णाङ्क k_obj_core_stats_reset(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_साँचो_t  साँचो = k_spin_lock(&obj_core_lock);

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

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
}

पूर्णाङ्क k_obj_core_stats_disable(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);

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

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
}

पूर्णाङ्क k_obj_core_stats_enable(संरचना k_obj_core *obj_core)
{
	पूर्णाङ्क rv;
	संरचना k_obj_core_stats_desc *desc;

	k_spinlock_साँचो_t साँचो = k_spin_lock(&obj_core_lock);

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

	k_spin_unlock(&obj_core_lock, साँचो);

	फर्काऊ rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
स्थिर संरचना k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
स्थिर संरचना k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = परिमाण(संरचना k_cycle_stats) * CONFIG_MP_MAX_अङ्क_CPUS,
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
			दुरत्व(संरचना z_kernel, obj_core));

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
