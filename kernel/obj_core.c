/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/kernel/obj_core.h>
#അന്തര്ഭാവ <kernel_internal.h>

സ്ഥിതം ഘടന k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
ഘടന obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	ഘടന k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
സ്ഥിതം ഘടന obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

സ്ഥിതം ഘടന k_obj_core *slot_core(സ്ഥിരം ഘടന obj_core_slot *slot)
{
	തിരികെ atomic_ptr_get(&slot->core);
}

സ്ഥിതം bool range_contains(സ്ഥിരം ഘടന k_obj_range *range, സ്ഥിരം ശൂന്യം *ptr)
{
	എങ്കിൽ (range->indirect) {
		വേണ്ടി (സ്ഥിരം ശൂന്യം *സ്ഥിരം *pp = range->start; pp < (സ്ഥിരം ശൂന്യം *സ്ഥിരം *)range->end;
		     pp = (സ്ഥിരം ശൂന്യം *സ്ഥിരം *)((സ്ഥിരം uint8_t *)pp + range->stride)) {
			എങ്കിൽ (*pp == ptr) {
				തിരികെ true;
			}
		}
		തിരികെ false;
	}

	തിരികെ (ptr >= range->start) && (ptr < range->end);
}

സ്ഥിതം bool type_ranges_contain(സ്ഥിരം ഘടന k_obj_type *type, സ്ഥിരം ശൂന്യം *ptr)
{
	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		സ്ഥിരം ഘടന k_obj_range *range = &type->statics[i];

		എങ്കിൽ (range->stride == 0U) {
			നിർത്തുക;
		}
		എങ്കിൽ (range_contains(range, ptr)) {
			തിരികെ true;
		}
	}

	തിരികെ false;
}

/* Object core of the range element */
സ്ഥിതം ഘടന k_obj_core *range_core(സ്ഥിരം ഘടന k_obj_type *type,
				     സ്ഥിരം ഘടന k_obj_range *range, സ്ഥിരം ശൂന്യം *elem)
{
	സ്ഥിരം uint8_t *obj = range->indirect ? *(സ്ഥിരം uint8_t *സ്ഥിരം *)elem : elem;

	തിരികെ (ഘടന k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
സ്ഥിതം bool in_അട്ടി_storage(സ്ഥിരം ശൂന്യം *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	സ്ഥിരം ഘടന k_thread *thread = _current;
	uintptr_t irq_അട്ടി = (uintptr_t)K_KERNEL_അട്ടി_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_അട്ടി_end =
		irq_അട്ടി + K_KERNEL_അട്ടി_SIZEOF(z_interrupt_stacks[0]);

	എങ്കിൽ ((thread != ശൂന്യ) && (thread->അട്ടി_info.size != 0U) &&
	    (addr >= thread->അട്ടി_info.start) &&
	    (addr < thread->അട്ടി_info.start + thread->അട്ടി_info.size)) {
		തിരികെ true;
	}

	തിരികെ (addr >= irq_അട്ടി) && (addr < irq_അട്ടി_end);
}

സ്ഥിതം ഘടന obj_core_slot *slot_find(സ്ഥിരം ഘടന k_obj_core *core)
{
	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		എങ്കിൽ (slot_core(&registry[i]) == core) {
			തിരികെ &registry[i];
		}
	}

	തിരികെ ശൂന്യ;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
സ്ഥിതം bool slot_stale(സ്ഥിരം ഘടന obj_core_slot *slot, സ്ഥിരം ഘടന k_obj_core *core)
{
	തിരികെ core->type != slot->type;
}

സ്ഥിതം ഘടന obj_core_slot *slot_alloc(ശൂന്യം)
{
	ഘടന obj_core_slot *slot = slot_find(ശൂന്യ);

	എങ്കിൽ (slot != ശൂന്യ) {
		തിരികെ slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ഘടന k_obj_core *core = slot_core(&registry[i]);

		എങ്കിൽ ((core != ശൂന്യ) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, ശൂന്യ);
			slot = &registry[i];
		}
	}

	തിരികെ slot;
}

ഘടന k_obj_type *z_obj_type_init(ഘടന k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->കണ്ണി);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, വലുപ്പം(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	തിരികെ type;
}

ശൂന്യം k_obj_type_init_range(ഘടന k_obj_type *type, സ്ഥിരം ശൂന്യം *start,
			   സ്ഥിരം ശൂന്യം *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

ശൂന്യം k_obj_core_init(ഘടന k_obj_core *obj_core, ഘടന k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = ശൂന്യ;
#endif /* CONFIG_OBJ_CORE_STATS */
}

ശൂന്യം k_obj_core_link(ഘടന k_obj_core *obj_core)
{
	ഘടന k_obj_type *type = obj_core->type;
	ഘടന obj_core_slot *slot;

	എങ്കിൽ (type_ranges_contain(type, (സ്ഥിരം uint8_t *)obj_core - type->obj_core_offset)) {
		തിരികെ;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);

	എങ്കിൽ (in_അട്ടി_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, താക്കോൽ);
		തിരികെ;
	}

	slot = slot_find(obj_core);
	എങ്കിൽ (slot == ശൂന്യ) {
		slot = slot_alloc();
	}

	എങ്കിൽ (slot == ശൂന്യ) {
		type->dropped++;
	} അല്ലെങ്കിൽ {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);
}

ശൂന്യം k_obj_core_init_and_link(ഘടന k_obj_core *obj_core,
			      ഘടന k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

ശൂന്യം k_obj_core_unlink(ഘടന k_obj_core *obj_core)
{
	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		എങ്കിൽ (atomic_ptr_cas(&registry[i].core, obj_core, ശൂന്യ)) {
			നിർത്തുക;
		}
	}
}

ശൂന്യം k_obj_core_evict_range(സ്ഥിരം ശൂന്യം *addr, size_t len)
{
	സ്ഥിരം uint8_t *start = addr;
	സ്ഥിരം uint8_t *end = start + len;

	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ഘടന k_obj_core *core = slot_core(&registry[i]);

		എങ്കിൽ ((core != ശൂന്യ) && ((സ്ഥിരം uint8_t *)core >= start) &&
		    ((സ്ഥിരം uint8_t *)core < end)) {
			(ശൂന്യം)atomic_ptr_cas(&registry[i].core, core, ശൂന്യ);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
ശൂന്യം sys_heap_release_hook(ശൂന്യം *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	എങ്കിൽ (k_is_user_context()) {
		തിരികെ;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
സ്ഥിതം ശൂന്യം obj_core_init_all(ശൂന്യം)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->കണ്ണി);

		വേണ്ടി (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			സ്ഥിരം ഘടന k_obj_range *range = &type->statics[r];

			എങ്കിൽ (range->stride == 0U) {
				നിർത്തുക;
			}

			വേണ്ടി (സ്ഥിരം uint8_t *elem = range->start;
			     elem < (സ്ഥിരം uint8_t *)range->end; elem += range->stride) {
				ഘടന k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				എങ്കിൽ ((type->stats_desc != ശൂന്യ) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(ശൂന്യം *)((uint8_t *)obj_core -
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

ഘടന k_obj_type *k_obj_type_find(uint32_t type_id)
{
	ഘടന k_obj_type *rv = ശൂന്യ;
	sys_snode_t *കണ്ണി;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		എങ്കിൽ (type->id == type_id) {
			തിരികെ type;
		}
	}

	k_spinlock_താക്കോൽ_t  താക്കോൽ = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_കണ്ണി(&z_obj_type_list, കണ്ണി) {
		ഘടന k_obj_type *type = CONTAINER_OF(കണ്ണി, ഘടന k_obj_type, കണ്ണി);

		എങ്കിൽ (type->id == type_id) {
			rv = type;
			നിർത്തുക;
		}
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ walk_statics(ഘടന k_obj_type *type,
			പൂർണ്ണസംഖ്യ (*func)(ഘടന k_obj_core *obj_core, ശൂന്യം *data),
			ശൂന്യം *data)
{
	പൂർണ്ണസംഖ്യ status = 0;

	വേണ്ടി (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		സ്ഥിരം ഘടന k_obj_range *range = &type->statics[r];

		എങ്കിൽ (range->stride == 0U) {
			നിർത്തുക;
		}

		വേണ്ടി (സ്ഥിരം uint8_t *elem = range->start; elem < (സ്ഥിരം uint8_t *)range->end;
		     elem += range->stride) {
			ഘടന k_obj_core *obj_core = range_core(type, range, elem);

			എങ്കിൽ (obj_core->type != type) {
				തുടരുക;
			}

			status = func(obj_core, data);
			എങ്കിൽ (status != 0) {
				നിർത്തുക;
			}
		}
	}

	തിരികെ status;
}

പൂർണ്ണസംഖ്യ k_obj_type_walk_locked(ഘടന k_obj_type *type,
			   പൂർണ്ണസംഖ്യ (*func)(ഘടന k_obj_core *obj_core, ശൂന്യം *data),
			   ശൂന്യം *data)
{
	പൂർണ്ണസംഖ്യ  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	വേണ്ടി (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ഘടന obj_core_slot *slot = &registry[i];
		k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);
		ഘടന k_obj_core *core = slot_core(slot);

		എങ്കിൽ ((core == ശൂന്യ) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, താക്കോൽ);
			തുടരുക;
		}

		എങ്കിൽ (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, ശൂന്യ);
			k_spin_unlock(&obj_core_lock, താക്കോൽ);
			തുടരുക;
		}

		k_spin_unlock(&obj_core_lock, താക്കോൽ);

		status = func(core, data);
	}

	തിരികെ status;
}

പൂർണ്ണസംഖ്യ k_obj_type_walk_unlocked(ഘടന k_obj_type *type,
			   പൂർണ്ണസംഖ്യ (*func)(ഘടന k_obj_core *obj_core, ശൂന്യം *data),
			   ശൂന്യം *data)
{
	പൂർണ്ണസംഖ്യ  status;

	status = walk_statics(type, func, data);

	വേണ്ടി (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ഘടന k_obj_core *core = slot_core(&registry[i]);

		എങ്കിൽ ((core == ശൂന്യ) || (registry[i].type != type) ||
		    (core->type != type)) {
			തുടരുക;
		}

		status = func(core, data);
	}

	തിരികെ status;
}

#ifdef CONFIG_OBJ_CORE_STATS
പൂർണ്ണസംഖ്യ k_obj_core_stats_register(ഘടന k_obj_core *obj_core, ശൂന്യം *stats,
			      size_t stats_len)
{
	പൂർണ്ണസംഖ്യ rv;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);

	എങ്കിൽ (obj_core->type->stats_desc == ശൂന്യ) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} അല്ലെങ്കിൽ എങ്കിൽ (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} അല്ലെങ്കിൽ {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}

പൂർണ്ണസംഖ്യ k_obj_core_stats_deregister(ഘടന k_obj_core *obj_core)
{
	പൂർണ്ണസംഖ്യ rv;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);

	എങ്കിൽ (obj_core->type->stats_desc == ശൂന്യ) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} അല്ലെങ്കിൽ {
		obj_core->stats = ശൂന്യ;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}

പൂർണ്ണസംഖ്യ k_obj_core_stats_raw(ഘടന k_obj_core *obj_core, ശൂന്യം *stats,
			 size_t stats_len)
{
	പൂർണ്ണസംഖ്യ rv;
	ഘടന k_obj_core_stats_desc *desc;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	എങ്കിൽ ((desc == ശൂന്യ) || (desc->raw == ശൂന്യ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} അല്ലെങ്കിൽ എങ്കിൽ ((desc->raw_size != stats_len) || (obj_core->stats == ശൂന്യ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} അല്ലെങ്കിൽ {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}

പൂർണ്ണസംഖ്യ k_obj_core_stats_query(ഘടന k_obj_core *obj_core, ശൂന്യം *stats,
			   size_t stats_len)
{
	പൂർണ്ണസംഖ്യ rv;
	ഘടന k_obj_core_stats_desc *desc;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	എങ്കിൽ ((desc == ശൂന്യ) || (desc->query == ശൂന്യ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} അല്ലെങ്കിൽ എങ്കിൽ ((desc->query_size != stats_len) || (obj_core->stats == ശൂന്യ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} അല്ലെങ്കിൽ {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}

പൂർണ്ണസംഖ്യ k_obj_core_stats_reset(ഘടന k_obj_core *obj_core)
{
	പൂർണ്ണസംഖ്യ rv;
	ഘടന k_obj_core_stats_desc *desc;

	k_spinlock_താക്കോൽ_t  താക്കോൽ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	എങ്കിൽ ((desc == ശൂന്യ) || (desc->reset == ശൂന്യ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} അല്ലെങ്കിൽ എങ്കിൽ (obj_core->stats == ശൂന്യ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} അല്ലെങ്കിൽ {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}

പൂർണ്ണസംഖ്യ k_obj_core_stats_disable(ഘടന k_obj_core *obj_core)
{
	പൂർണ്ണസംഖ്യ rv;
	ഘടന k_obj_core_stats_desc *desc;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	എങ്കിൽ ((desc == ശൂന്യ) || (desc->disable == ശൂന്യ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} അല്ലെങ്കിൽ എങ്കിൽ (obj_core->stats == ശൂന്യ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} അല്ലെങ്കിൽ {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}

പൂർണ്ണസംഖ്യ k_obj_core_stats_enable(ഘടന k_obj_core *obj_core)
{
	പൂർണ്ണസംഖ്യ rv;
	ഘടന k_obj_core_stats_desc *desc;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	എങ്കിൽ ((desc == ശൂന്യ) || (desc->enable == ശൂന്യ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} അല്ലെങ്കിൽ എങ്കിൽ (obj_core->stats == ശൂന്യ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} അല്ലെങ്കിൽ {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, താക്കോൽ);

	തിരികെ rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
സ്ഥിതം ഘടന k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
സ്ഥിതം ഘടന k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = വലുപ്പം(ഘടന k_cycle_stats) * CONFIG_MP_MAX_അക്കം_CPUS,
	.query_size = വലുപ്പം(ഘടന k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ശൂന്യ,
	.disable = ശൂന്യ,
	.enable  = ശൂന്യ,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
സ്ഥിതം ശൂന്യം init_kernel_obj_core_list(ശൂന്യം)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(ഘടന z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  വലുപ്പം(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
