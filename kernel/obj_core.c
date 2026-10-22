/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <string.h>
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/kernel/obj_core.h>
#समावेश <kernel_internal.h>

अटल काष्ठा k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
काष्ठा obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	काष्ठा k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
अटल काष्ठा obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

अटल काष्ठा k_obj_core *slot_core(स्थिर काष्ठा obj_core_slot *slot)
{
	वापस atomic_ptr_get(&slot->core);
}

अटल bool range_contains(स्थिर काष्ठा k_obj_range *range, स्थिर व्योम *ptr)
{
	अगर (range->indirect) {
		क्रम (स्थिर व्योम *स्थिर *pp = range->start; pp < (स्थिर व्योम *स्थिर *)range->end;
		     pp = (स्थिर व्योम *स्थिर *)((स्थिर uint8_t *)pp + range->stride)) {
			अगर (*pp == ptr) {
				वापस true;
			}
		}
		वापस false;
	}

	वापस (ptr >= range->start) && (ptr < range->end);
}

अटल bool type_ranges_contain(स्थिर काष्ठा k_obj_type *type, स्थिर व्योम *ptr)
{
	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(type->statics); i++) {
		स्थिर काष्ठा k_obj_range *range = &type->statics[i];

		अगर (range->stride == 0U) {
			अवरोध;
		}
		अगर (range_contains(range, ptr)) {
			वापस true;
		}
	}

	वापस false;
}

/* Object core of the range element */
अटल काष्ठा k_obj_core *range_core(स्थिर काष्ठा k_obj_type *type,
				     स्थिर काष्ठा k_obj_range *range, स्थिर व्योम *elem)
{
	स्थिर uint8_t *obj = range->indirect ? *(स्थिर uint8_t *स्थिर *)elem : elem;

	वापस (काष्ठा k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
अटल bool in_ढेर_storage(स्थिर व्योम *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	स्थिर काष्ठा k_thread *thread = _current;
	uintptr_t irq_ढेर = (uintptr_t)K_KERNEL_ढेर_बफर(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_ढेर_end =
		irq_ढेर + K_KERNEL_ढेर_SIZEOF(z_interrupt_stacks[0]);

	अगर ((thread != शून्य) && (thread->ढेर_info.आकार != 0U) &&
	    (addr >= thread->ढेर_info.start) &&
	    (addr < thread->ढेर_info.start + thread->ढेर_info.आकार)) {
		वापस true;
	}

	वापस (addr >= irq_ढेर) && (addr < irq_ढेर_end);
}

अटल काष्ठा obj_core_slot *slot_find(स्थिर काष्ठा k_obj_core *core)
{
	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(registry); i++) {
		अगर (slot_core(&registry[i]) == core) {
			वापस &registry[i];
		}
	}

	वापस शून्य;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
अटल bool slot_stale(स्थिर काष्ठा obj_core_slot *slot, स्थिर काष्ठा k_obj_core *core)
{
	वापस core->type != slot->type;
}

अटल काष्ठा obj_core_slot *slot_alloc(व्योम)
{
	काष्ठा obj_core_slot *slot = slot_find(शून्य);

	अगर (slot != शून्य) {
		वापस slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(registry); i++) {
		काष्ठा k_obj_core *core = slot_core(&registry[i]);

		अगर ((core != शून्य) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, शून्य);
			slot = &registry[i];
		}
	}

	वापस slot;
}

काष्ठा k_obj_type *z_obj_type_init(काष्ठा k_obj_type *type,
				   uint32_t id, माप_प्रकार off)
{
	sys_slist_append(&z_obj_type_list, &type->गाँठ);
	type->id = id;
	type->obj_core_offset = off;
	स_रखो(type->statics, 0, माप(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	वापस type;
}

व्योम k_obj_type_init_range(काष्ठा k_obj_type *type, स्थिर व्योम *start,
			   स्थिर व्योम *end, माप_प्रकार stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

व्योम k_obj_core_init(काष्ठा k_obj_core *obj_core, काष्ठा k_obj_type *type)
{
	obj_core->type = type;
#अगर_घोषित CONFIG_OBJ_CORE_STATS
	obj_core->stats = शून्य;
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS */
}

व्योम k_obj_core_link(काष्ठा k_obj_core *obj_core)
{
	काष्ठा k_obj_type *type = obj_core->type;
	काष्ठा obj_core_slot *slot;

	अगर (type_ranges_contain(type, (स्थिर uint8_t *)obj_core - type->obj_core_offset)) {
		वापस;
	}

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);

	अगर (in_ढेर_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, कुंजी);
		वापस;
	}

	slot = slot_find(obj_core);
	अगर (slot == शून्य) {
		slot = slot_alloc();
	}

	अगर (slot == शून्य) {
		type->dropped++;
	} अन्यथा {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुंजी);
}

व्योम k_obj_core_init_and_link(काष्ठा k_obj_core *obj_core,
			      काष्ठा k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

व्योम k_obj_core_unlink(काष्ठा k_obj_core *obj_core)
{
	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(registry); i++) {
		अगर (atomic_ptr_cas(&registry[i].core, obj_core, शून्य)) {
			अवरोध;
		}
	}
}

व्योम k_obj_core_evict_range(स्थिर व्योम *addr, माप_प्रकार len)
{
	स्थिर uint8_t *start = addr;
	स्थिर uint8_t *end = start + len;

	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(registry); i++) {
		काष्ठा k_obj_core *core = slot_core(&registry[i]);

		अगर ((core != शून्य) && ((स्थिर uint8_t *)core >= start) &&
		    ((स्थिर uint8_t *)core < end)) {
			(व्योम)atomic_ptr_cas(&registry[i].core, core, शून्य);
		}
	}
}

#अगर_घोषित CONFIG_OBJ_CORE_EVICT_ON_FREE
व्योम sys_heap_release_hook(व्योम *mem, माप_प्रकार bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	अगर (k_is_user_context()) {
		वापस;
	}

	k_obj_core_evict_range(mem, bytes);
}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
अटल व्योम obj_core_init_all(व्योम)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->गाँठ);

		क्रम (माप_प्रकार r = 0; r < ARRAY_आकार(type->statics); r++) {
			स्थिर काष्ठा k_obj_range *range = &type->statics[r];

			अगर (range->stride == 0U) {
				अवरोध;
			}

			क्रम (स्थिर uint8_t *elem = range->start;
			     elem < (स्थिर uint8_t *)range->end; elem += range->stride) {
				काष्ठा k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#अगर_घोषित CONFIG_OBJ_CORE_STATS
				अगर ((type->stats_desc != शून्य) && (type->stats_आकार != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(व्योम *)((uint8_t *)obj_core -
							 type->obj_core_offset +
							 type->stats_offset),
						type->stats_आकार);
				}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

काष्ठा k_obj_type *k_obj_type_find(uint32_t type_id)
{
	काष्ठा k_obj_type *rv = शून्य;
	sys_snode_t *गाँठ;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		अगर (type->id == type_id) {
			वापस type;
		}
	}

	k_spinlock_कुंजी_t  कुंजी = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_गाँठ(&z_obj_type_list, गाँठ) {
		काष्ठा k_obj_type *type = CONTAINER_OF(गाँठ, काष्ठा k_obj_type, गाँठ);

		अगर (type->id == type_id) {
			rv = type;
			अवरोध;
		}
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
अटल पूर्णांक walk_statics(काष्ठा k_obj_type *type,
			पूर्णांक (*func)(काष्ठा k_obj_core *obj_core, व्योम *तथ्य),
			व्योम *तथ्य)
{
	पूर्णांक status = 0;

	क्रम (माप_प्रकार r = 0; (status == 0) && (r < ARRAY_आकार(type->statics)); r++) {
		स्थिर काष्ठा k_obj_range *range = &type->statics[r];

		अगर (range->stride == 0U) {
			अवरोध;
		}

		क्रम (स्थिर uint8_t *elem = range->start; elem < (स्थिर uint8_t *)range->end;
		     elem += range->stride) {
			काष्ठा k_obj_core *obj_core = range_core(type, range, elem);

			अगर (obj_core->type != type) {
				जारी;
			}

			status = func(obj_core, तथ्य);
			अगर (status != 0) {
				अवरोध;
			}
		}
	}

	वापस status;
}

पूर्णांक k_obj_type_walk_locked(काष्ठा k_obj_type *type,
			   पूर्णांक (*func)(काष्ठा k_obj_core *obj_core, व्योम *तथ्य),
			   व्योम *तथ्य)
{
	पूर्णांक  status;

	status = walk_statics(type, func, तथ्य);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	क्रम (माप_प्रकार i = 0; (status == 0) && (i < ARRAY_आकार(registry)); i++) {
		काष्ठा obj_core_slot *slot = &registry[i];
		k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);
		काष्ठा k_obj_core *core = slot_core(slot);

		अगर ((core == शून्य) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, कुंजी);
			जारी;
		}

		अगर (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, शून्य);
			k_spin_unlock(&obj_core_lock, कुंजी);
			जारी;
		}

		k_spin_unlock(&obj_core_lock, कुंजी);

		status = func(core, तथ्य);
	}

	वापस status;
}

पूर्णांक k_obj_type_walk_unlocked(काष्ठा k_obj_type *type,
			   पूर्णांक (*func)(काष्ठा k_obj_core *obj_core, व्योम *तथ्य),
			   व्योम *तथ्य)
{
	पूर्णांक  status;

	status = walk_statics(type, func, तथ्य);

	क्रम (माप_प्रकार i = 0; (status == 0) && (i < ARRAY_आकार(registry)); i++) {
		काष्ठा k_obj_core *core = slot_core(&registry[i]);

		अगर ((core == शून्य) || (registry[i].type != type) ||
		    (core->type != type)) {
			जारी;
		}

		status = func(core, तथ्य);
	}

	वापस status;
}

#अगर_घोषित CONFIG_OBJ_CORE_STATS
पूर्णांक k_obj_core_stats_register(काष्ठा k_obj_core *obj_core, व्योम *stats,
			      माप_प्रकार stats_len)
{
	पूर्णांक rv;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);

	अगर (obj_core->type->stats_desc == शून्य) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} अन्यथा अगर (obj_core->type->stats_desc->raw_आकार != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} अन्यथा {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}

पूर्णांक k_obj_core_stats_deregister(काष्ठा k_obj_core *obj_core)
{
	पूर्णांक rv;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);

	अगर (obj_core->type->stats_desc == शून्य) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} अन्यथा {
		obj_core->stats = शून्य;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}

पूर्णांक k_obj_core_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats,
			 माप_प्रकार stats_len)
{
	पूर्णांक rv;
	काष्ठा k_obj_core_stats_desc *desc;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	अगर ((desc == शून्य) || (desc->raw == शून्य)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा अगर ((desc->raw_आकार != stats_len) || (obj_core->stats == शून्य)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}

पूर्णांक k_obj_core_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats,
			   माप_प्रकार stats_len)
{
	पूर्णांक rv;
	काष्ठा k_obj_core_stats_desc *desc;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	अगर ((desc == शून्य) || (desc->query == शून्य)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा अगर ((desc->query_आकार != stats_len) || (obj_core->stats == शून्य)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}

पूर्णांक k_obj_core_stats_reset(काष्ठा k_obj_core *obj_core)
{
	पूर्णांक rv;
	काष्ठा k_obj_core_stats_desc *desc;

	k_spinlock_कुंजी_t  कुंजी = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	अगर ((desc == शून्य) || (desc->reset == शून्य)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा अगर (obj_core->stats == शून्य) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}

पूर्णांक k_obj_core_stats_disable(काष्ठा k_obj_core *obj_core)
{
	पूर्णांक rv;
	काष्ठा k_obj_core_stats_desc *desc;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	अगर ((desc == शून्य) || (desc->disable == शून्य)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा अगर (obj_core->stats == शून्य) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}

पूर्णांक k_obj_core_stats_enable(काष्ठा k_obj_core *obj_core)
{
	पूर्णांक rv;
	काष्ठा k_obj_core_stats_desc *desc;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	अगर ((desc == शून्य) || (desc->enable == शून्य)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} अन्यथा अगर (obj_core->stats == शून्य) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} अन्यथा {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, कुंजी);

	वापस rv;
}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS */

#अगर_घोषित CONFIG_OBJ_CORE_SYSTEM
अटल काष्ठा k_obj_type obj_type_kernel;

#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
अटल काष्ठा k_obj_core_stats_desc kernel_stats_desc = {
	.raw_आकार = माप(काष्ठा k_cycle_stats) * CONFIG_MP_MAX_अंक_CPUS,
	.query_आकार = माप(काष्ठा k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = शून्य,
	.disable = शून्य,
	.enable  = शून्य,
};
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
अटल व्योम init_kernel_obj_core_list(व्योम)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			दुरत्व(काष्ठा z_kernel, obj_core));

#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  माप(_kernel.usage));
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_SYSTEM */
