/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel/obj_core.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>

ਸਥਿਰ ਬਣਤਰ k_spinlock  obj_core_ਤਾਲਾ;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
ਬਣਤਰ obj_core_ਥਾਂ {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	ਬਣਤਰ k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
ਸਥਿਰ ਬਣਤਰ obj_core_ਥਾਂ registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

ਸਥਿਰ ਬਣਤਰ k_obj_core *ਥਾਂ_core(ਅਚਲ ਬਣਤਰ obj_core_ਥਾਂ *ਥਾਂ)
{
	ਵਾਪਸ atomic_ptr_get(&ਥਾਂ->core);
}

ਸਥਿਰ bool range_ਸ਼ਾਮਲ_ਹੈ(ਅਚਲ ਬਣਤਰ k_obj_range *range, ਅਚਲ ਖਾਲੀ *ptr)
{
	ਜੇ (range->ਅਸਿੱਧਾ) {
		ਲਈ (ਅਚਲ ਖਾਲੀ *ਅਚਲ *pp = range->ਸ਼ੁਰੂ; pp < (ਅਚਲ ਖਾਲੀ *ਅਚਲ *)range->ਸਿਰਾ;
		     pp = (ਅਚਲ ਖਾਲੀ *ਅਚਲ *)((ਅਚਲ uint8_t *)pp + range->stride)) {
			ਜੇ (*pp == ptr) {
				ਵਾਪਸ true;
			}
		}
		ਵਾਪਸ false;
	}

	ਵਾਪਸ (ptr >= range->ਸ਼ੁਰੂ) && (ptr < range->ਸਿਰਾ);
}

ਸਥਿਰ bool type_ranges_contain(ਅਚਲ ਬਣਤਰ k_obj_type *type, ਅਚਲ ਖਾਲੀ *ptr)
{
	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(type->statics); i++) {
		ਅਚਲ ਬਣਤਰ k_obj_range *range = &type->statics[i];

		ਜੇ (range->stride == 0U) {
			ਰੋਕੋ;
		}
		ਜੇ (range_ਸ਼ਾਮਲ_ਹੈ(range, ptr)) {
			ਵਾਪਸ true;
		}
	}

	ਵਾਪਸ false;
}

/* Object core of the range element */
ਸਥਿਰ ਬਣਤਰ k_obj_core *range_core(ਅਚਲ ਬਣਤਰ k_obj_type *type,
				     ਅਚਲ ਬਣਤਰ k_obj_range *range, ਅਚਲ ਖਾਲੀ *elem)
{
	ਅਚਲ uint8_t *obj = range->ਅਸਿੱਧਾ ? *(ਅਚਲ uint8_t *ਅਚਲ *)elem : elem;

	ਵਾਪਸ (ਬਣਤਰ k_obj_core *)(obj + type->obj_core_ਵਿੱਥ);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
ਸਥਿਰ bool ਵਿੱਚ_ਢੇਰ_storage(ਅਚਲ ਖਾਲੀ *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ਅਚਲ ਬਣਤਰ k_thread *thread = _current;
	uintptr_t irq_ਢੇਰ = (uintptr_t)K_KERNEL_ਢੇਰ_ਬਫਰ(
		z_ਰੁਕਾਵਟ_stacks[_current_cpu->id]);
	uintptr_t irq_ਢੇਰ_ਸਿਰਾ =
		irq_ਢੇਰ + K_KERNEL_ਢੇਰ_SIZEOF(z_ਰੁਕਾਵਟ_stacks[0]);

	ਜੇ ((thread != ਨਿਰਾ) && (thread->ਢੇਰ_info.ਮਾਪ != 0U) &&
	    (addr >= thread->ਢੇਰ_info.ਸ਼ੁਰੂ) &&
	    (addr < thread->ਢੇਰ_info.ਸ਼ੁਰੂ + thread->ਢੇਰ_info.ਮਾਪ)) {
		ਵਾਪਸ true;
	}

	ਵਾਪਸ (addr >= irq_ਢੇਰ) && (addr < irq_ਢੇਰ_ਸਿਰਾ);
}

ਸਥਿਰ ਬਣਤਰ obj_core_ਥਾਂ *ਥਾਂ_ਲੱਭੋ(ਅਚਲ ਬਣਤਰ k_obj_core *core)
{
	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(registry); i++) {
		ਜੇ (ਥਾਂ_core(&registry[i]) == core) {
			ਵਾਪਸ &registry[i];
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
ਸਥਿਰ bool ਥਾਂ_stale(ਅਚਲ ਬਣਤਰ obj_core_ਥਾਂ *ਥਾਂ, ਅਚਲ ਬਣਤਰ k_obj_core *core)
{
	ਵਾਪਸ core->type != ਥਾਂ->type;
}

ਸਥਿਰ ਬਣਤਰ obj_core_ਥਾਂ *ਥਾਂ_alloc(ਖਾਲੀ)
{
	ਬਣਤਰ obj_core_ਥਾਂ *ਥਾਂ = ਥਾਂ_ਲੱਭੋ(ਨਿਰਾ);

	ਜੇ (ਥਾਂ != ਨਿਰਾ) {
		ਵਾਪਸ ਥਾਂ;
	}

	/* Full: reap every stale entry and reuse one of them */

	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(registry); i++) {
		ਬਣਤਰ k_obj_core *core = ਥਾਂ_core(&registry[i]);

		ਜੇ ((core != ਨਿਰਾ) && ਥਾਂ_stale(&registry[i], core)) {
			atomic_ptr_ਸੈੱਟ(&registry[i].core, ਨਿਰਾ);
			ਥਾਂ = &registry[i];
		}
	}

	ਵਾਪਸ ਥਾਂ;
}

ਬਣਤਰ k_obj_type *z_obj_type_init(ਬਣਤਰ k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->ਗੰਢ);
	type->id = id;
	type->obj_core_ਵਿੱਥ = off;
	memset(type->statics, 0, ਆਕਾਰ(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	ਵਾਪਸ type;
}

ਖਾਲੀ k_obj_type_init_range(ਬਣਤਰ k_obj_type *type, ਅਚਲ ਖਾਲੀ *ਸ਼ੁਰੂ,
			   ਅਚਲ ਖਾਲੀ *ਸਿਰਾ, size_t stride, bool ਅਸਿੱਧਾ)
{
	type->statics[0].ਸ਼ੁਰੂ = ਸ਼ੁਰੂ;
	type->statics[0].ਸਿਰਾ = ਸਿਰਾ;
	type->statics[0].stride = stride;
	type->statics[0].ਅਸਿੱਧਾ = ਅਸਿੱਧਾ;
}

ਖਾਲੀ k_obj_core_init(ਬਣਤਰ k_obj_core *obj_core, ਬਣਤਰ k_obj_type *type)
{
	obj_core->type = type;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS
	obj_core->stats = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS */
}

ਖਾਲੀ k_obj_core_link(ਬਣਤਰ k_obj_core *obj_core)
{
	ਬਣਤਰ k_obj_type *type = obj_core->type;
	ਬਣਤਰ obj_core_ਥਾਂ *ਥਾਂ;

	ਜੇ (type_ranges_contain(type, (ਅਚਲ uint8_t *)obj_core - type->obj_core_ਵਿੱਥ)) {
		ਵਾਪਸ;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	ਜੇ (ਵਿੱਚ_ਢੇਰ_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	ਥਾਂ = ਥਾਂ_ਲੱਭੋ(obj_core);
	ਜੇ (ਥਾਂ == ਨਿਰਾ) {
		ਥਾਂ = ਥਾਂ_alloc();
	}

	ਜੇ (ਥਾਂ == ਨਿਰਾ) {
		type->dropped++;
	} ਨਹੀਂਤਾਂ {
		ਥਾਂ->type = type;
		atomic_ptr_ਸੈੱਟ(&ਥਾਂ->core, obj_core);
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਖਾਲੀ k_obj_core_init_and_link(ਬਣਤਰ k_obj_core *obj_core,
			      ਬਣਤਰ k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

ਖਾਲੀ k_obj_core_unlink(ਬਣਤਰ k_obj_core *obj_core)
{
	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(registry); i++) {
		ਜੇ (atomic_ptr_cas(&registry[i].core, obj_core, ਨਿਰਾ)) {
			ਰੋਕੋ;
		}
	}
}

ਖਾਲੀ k_obj_core_evict_range(ਅਚਲ ਖਾਲੀ *addr, size_t len)
{
	ਅਚਲ uint8_t *ਸ਼ੁਰੂ = addr;
	ਅਚਲ uint8_t *ਸਿਰਾ = ਸ਼ੁਰੂ + len;

	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(registry); i++) {
		ਬਣਤਰ k_obj_core *core = ਥਾਂ_core(&registry[i]);

		ਜੇ ((core != ਨਿਰਾ) && ((ਅਚਲ uint8_t *)core >= ਸ਼ੁਰੂ) &&
		    ((ਅਚਲ uint8_t *)core < ਸਿਰਾ)) {
			(ਖਾਲੀ)atomic_ptr_cas(&registry[i].core, core, ਨਿਰਾ);
		}
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_EVICT_ON_FREE
ਖਾਲੀ sys_ਢੇਰ_release_hook(ਖਾਲੀ *mem, size_t ਬਾਈਟਾਂ)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	ਜੇ (k_is_user_context()) {
		ਵਾਪਸ;
	}

	k_obj_core_evict_range(mem, ਬਾਈਟਾਂ);
}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
ਸਥਿਰ ਖਾਲੀ obj_core_init_all(ਖਾਲੀ)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->ਗੰਢ);

		ਲਈ (size_t r = 0; r < ARRAY_ਮਾਪ(type->statics); r++) {
			ਅਚਲ ਬਣਤਰ k_obj_range *range = &type->statics[r];

			ਜੇ (range->stride == 0U) {
				ਰੋਕੋ;
			}

			ਲਈ (ਅਚਲ uint8_t *elem = range->ਸ਼ੁਰੂ;
			     elem < (ਅਚਲ uint8_t *)range->ਸਿਰਾ; elem += range->stride) {
				ਬਣਤਰ k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS
				ਜੇ ((type->stats_desc != ਨਿਰਾ) && (type->stats_ਮਾਪ != 0)) {
					k_obj_core_stats_ਰਜਿਸਟਰ(
						obj_core,
						(ਖਾਲੀ *)((uint8_t *)obj_core -
							 type->obj_core_ਵਿੱਥ +
							 type->stats_ਵਿੱਥ),
						type->stats_ਮਾਪ);
				}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

ਬਣਤਰ k_obj_type *k_obj_type_ਲੱਭੋ(uint32_t type_id)
{
	ਬਣਤਰ k_obj_type *rv = ਨਿਰਾ;
	sys_snode_t *ਗੰਢ;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		ਜੇ (type->id == type_id) {
			ਵਾਪਸ type;
		}
	}

	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	SYS_SLIST_FOR_EACH_ਗੰਢ(&z_obj_type_list, ਗੰਢ) {
		ਬਣਤਰ k_obj_type *type = CONTAINER_ਦਾ(ਗੰਢ, ਬਣਤਰ k_obj_type, ਗੰਢ);

		ਜੇ (type->id == type_id) {
			rv = type;
			ਰੋਕੋ;
		}
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
ਸਥਿਰ ਪੂਰਨ walk_statics(ਬਣਤਰ k_obj_type *type,
			ਪੂਰਨ (*func)(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *ਅੰਕੜੇ),
			ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਪੂਰਨ ਸਥਿਤੀ = 0;

	ਲਈ (size_t r = 0; (ਸਥਿਤੀ == 0) && (r < ARRAY_ਮਾਪ(type->statics)); r++) {
		ਅਚਲ ਬਣਤਰ k_obj_range *range = &type->statics[r];

		ਜੇ (range->stride == 0U) {
			ਰੋਕੋ;
		}

		ਲਈ (ਅਚਲ uint8_t *elem = range->ਸ਼ੁਰੂ; elem < (ਅਚਲ uint8_t *)range->ਸਿਰਾ;
		     elem += range->stride) {
			ਬਣਤਰ k_obj_core *obj_core = range_core(type, range, elem);

			ਜੇ (obj_core->type != type) {
				ਜਾਰੀ;
			}

			ਸਥਿਤੀ = func(obj_core, ਅੰਕੜੇ);
			ਜੇ (ਸਥਿਤੀ != 0) {
				ਰੋਕੋ;
			}
		}
	}

	ਵਾਪਸ ਸਥਿਤੀ;
}

ਪੂਰਨ k_obj_type_walk_locked(ਬਣਤਰ k_obj_type *type,
			   ਪੂਰਨ (*func)(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *ਅੰਕੜੇ),
			   ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਪੂਰਨ  ਸਥਿਤੀ;

	ਸਥਿਤੀ = walk_statics(type, func, ਅੰਕੜੇ);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	ਲਈ (size_t i = 0; (ਸਥਿਤੀ == 0) && (i < ARRAY_ਮਾਪ(registry)); i++) {
		ਬਣਤਰ obj_core_ਥਾਂ *ਥਾਂ = &registry[i];
		k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);
		ਬਣਤਰ k_obj_core *core = ਥਾਂ_core(ਥਾਂ);

		ਜੇ ((core == ਨਿਰਾ) || (ਥਾਂ->type != type)) {
			k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);
			ਜਾਰੀ;
		}

		ਜੇ (ਥਾਂ_stale(ਥਾਂ, core)) {
			atomic_ptr_ਸੈੱਟ(&ਥਾਂ->core, ਨਿਰਾ);
			k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);
			ਜਾਰੀ;
		}

		k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

		ਸਥਿਤੀ = func(core, ਅੰਕੜੇ);
	}

	ਵਾਪਸ ਸਥਿਤੀ;
}

ਪੂਰਨ k_obj_type_walk_unlocked(ਬਣਤਰ k_obj_type *type,
			   ਪੂਰਨ (*func)(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *ਅੰਕੜੇ),
			   ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਪੂਰਨ  ਸਥਿਤੀ;

	ਸਥਿਤੀ = walk_statics(type, func, ਅੰਕੜੇ);

	ਲਈ (size_t i = 0; (ਸਥਿਤੀ == 0) && (i < ARRAY_ਮਾਪ(registry)); i++) {
		ਬਣਤਰ k_obj_core *core = ਥਾਂ_core(&registry[i]);

		ਜੇ ((core == ਨਿਰਾ) || (registry[i].type != type) ||
		    (core->type != type)) {
			ਜਾਰੀ;
		}

		ਸਥਿਤੀ = func(core, ਅੰਕੜੇ);
	}

	ਵਾਪਸ ਸਥਿਤੀ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS
ਪੂਰਨ k_obj_core_stats_ਰਜਿਸਟਰ(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats,
			      size_t stats_len)
{
	ਪੂਰਨ rv;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	ਜੇ (obj_core->type->stats_desc == ਨਿਰਾ) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} ਨਹੀਂਤਾਂ ਜੇ (obj_core->type->stats_desc->raw_ਮਾਪ != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} ਨਹੀਂਤਾਂ {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}

ਪੂਰਨ k_obj_core_stats_deregister(ਬਣਤਰ k_obj_core *obj_core)
{
	ਪੂਰਨ rv;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	ਜੇ (obj_core->type->stats_desc == ਨਿਰਾ) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} ਨਹੀਂਤਾਂ {
		obj_core->stats = ਨਿਰਾ;
		rv = 0;
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}

ਪੂਰਨ k_obj_core_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats,
			 size_t stats_len)
{
	ਪੂਰਨ rv;
	ਬਣਤਰ k_obj_core_stats_desc *desc;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	desc = obj_core->type->stats_desc;
	ਜੇ ((desc == ਨਿਰਾ) || (desc->raw == ਨਿਰਾ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ਨਹੀਂਤਾਂ ਜੇ ((desc->raw_ਮਾਪ != stats_len) || (obj_core->stats == ਨਿਰਾ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ਨਹੀਂਤਾਂ {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}

ਪੂਰਨ k_obj_core_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats,
			   size_t stats_len)
{
	ਪੂਰਨ rv;
	ਬਣਤਰ k_obj_core_stats_desc *desc;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	desc = obj_core->type->stats_desc;
	ਜੇ ((desc == ਨਿਰਾ) || (desc->query == ਨਿਰਾ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ਨਹੀਂਤਾਂ ਜੇ ((desc->query_ਮਾਪ != stats_len) || (obj_core->stats == ਨਿਰਾ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ਨਹੀਂਤਾਂ {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}

ਪੂਰਨ k_obj_core_stats_reset(ਬਣਤਰ k_obj_core *obj_core)
{
	ਪੂਰਨ rv;
	ਬਣਤਰ k_obj_core_stats_desc *desc;

	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	desc = obj_core->type->stats_desc;
	ਜੇ ((desc == ਨਿਰਾ) || (desc->reset == ਨਿਰਾ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ਨਹੀਂਤਾਂ ਜੇ (obj_core->stats == ਨਿਰਾ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ਨਹੀਂਤਾਂ {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}

ਪੂਰਨ k_obj_core_stats_disable(ਬਣਤਰ k_obj_core *obj_core)
{
	ਪੂਰਨ rv;
	ਬਣਤਰ k_obj_core_stats_desc *desc;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	desc = obj_core->type->stats_desc;
	ਜੇ ((desc == ਨਿਰਾ) || (desc->disable == ਨਿਰਾ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ਨਹੀਂਤਾਂ ਜੇ (obj_core->stats == ਨਿਰਾ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ਨਹੀਂਤਾਂ {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}

ਪੂਰਨ k_obj_core_stats_enable(ਬਣਤਰ k_obj_core *obj_core)
{
	ਪੂਰਨ rv;
	ਬਣਤਰ k_obj_core_stats_desc *desc;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_core_ਤਾਲਾ);

	desc = obj_core->type->stats_desc;
	ਜੇ ((desc == ਨਿਰਾ) || (desc->enable == ਨਿਰਾ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ਨਹੀਂਤਾਂ ਜੇ (obj_core->stats == ਨਿਰਾ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ਨਹੀਂਤਾਂ {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ rv;
}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_SYSTEM
ਸਥਿਰ ਬਣਤਰ k_obj_type obj_type_kernel;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
ਸਥਿਰ ਬਣਤਰ k_obj_core_stats_desc kernel_stats_desc = {
	.raw_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ k_ਚੱਕਰ_stats) * CONFIG_MP_MAX_ਅੰਕ_CPUS,
	.query_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ਨਿਰਾ,
	.disable = ਨਿਰਾ,
	.enable  = ਨਿਰਾ,
};
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
ਸਥਿਰ ਖਾਲੀ init_kernel_obj_core_list(ਖਾਲੀ)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(ਬਣਤਰ z_kernel, obj_core));

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_ਰਜਿਸਟਰ(K_OBJ_CORE(&_kernel), _kernel.usage,
				  ਆਕਾਰ(_kernel.usage));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_SYSTEM */
