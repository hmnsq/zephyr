/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/obj_core.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>

ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
ಸಂರಚನಾ obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	ಸಂರಚನಾ k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
ಸ್ಥಿರ ಸಂರಚನಾ obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

ಸ್ಥಿರ ಸಂರಚನಾ k_obj_core *slot_core(ಧ್ರುವ ಸಂರಚನಾ obj_core_slot *slot)
{
	ಪ್ರತ್ಯಾಗಮ atomic_ptr_get(&slot->core);
}

ಸ್ಥಿರ bool range_contains(ಧ್ರುವ ಸಂರಚನಾ k_obj_range *range, ಧ್ರುವ ಶೂನ್ಯ *ptr)
{
	ಯದಿ (range->indirect) {
		ಪ್ರತಿ (ಧ್ರುವ ಶೂನ್ಯ *ಧ್ರುವ *pp = range->start; pp < (ಧ್ರುವ ಶೂನ್ಯ *ಧ್ರುವ *)range->end;
		     pp = (ಧ್ರುವ ಶೂನ್ಯ *ಧ್ರುವ *)((ಧ್ರುವ uint8_t *)pp + range->stride)) {
			ಯದಿ (*pp == ptr) {
				ಪ್ರತ್ಯಾಗಮ true;
			}
		}
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ (ptr >= range->start) && (ptr < range->end);
}

ಸ್ಥಿರ bool type_ranges_contain(ಧ್ರುವ ಸಂರಚನಾ k_obj_type *type, ಧ್ರುವ ಶೂನ್ಯ *ptr)
{
	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		ಧ್ರುವ ಸಂರಚನಾ k_obj_range *range = &type->statics[i];

		ಯದಿ (range->stride == 0U) {
			ಭಙ್ಗ;
		}
		ಯದಿ (range_contains(range, ptr)) {
			ಪ್ರತ್ಯಾಗಮ true;
		}
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

/* Object core of the range element */
ಸ್ಥಿರ ಸಂರಚನಾ k_obj_core *range_core(ಧ್ರುವ ಸಂರಚನಾ k_obj_type *type,
				     ಧ್ರುವ ಸಂರಚನಾ k_obj_range *range, ಧ್ರುವ ಶೂನ್ಯ *elem)
{
	ಧ್ರುವ uint8_t *obj = range->indirect ? *(ಧ್ರುವ uint8_t *ಧ್ರುವ *)elem : elem;

	ಪ್ರತ್ಯಾಗಮ (ಸಂರಚನಾ k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
ಸ್ಥಿರ bool in_ರಾಶಿ_storage(ಧ್ರುವ ಶೂನ್ಯ *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ಧ್ರುವ ಸಂರಚನಾ k_thread *thread = _current;
	uintptr_t irq_ರಾಶಿ = (uintptr_t)K_KERNEL_ರಾಶಿ_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_ರಾಶಿ_end =
		irq_ರಾಶಿ + K_KERNEL_ರಾಶಿ_SIZEOF(z_interrupt_stacks[0]);

	ಯದಿ ((thread != ರಿಕ್ತ) && (thread->ರಾಶಿ_info.size != 0U) &&
	    (addr >= thread->ರಾಶಿ_info.start) &&
	    (addr < thread->ರಾಶಿ_info.start + thread->ರಾಶಿ_info.size)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ (addr >= irq_ರಾಶಿ) && (addr < irq_ರಾಶಿ_end);
}

ಸ್ಥಿರ ಸಂರಚನಾ obj_core_slot *slot_find(ಧ್ರುವ ಸಂರಚನಾ k_obj_core *core)
{
	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ಯದಿ (slot_core(&registry[i]) == core) {
			ಪ್ರತ್ಯಾಗಮ &registry[i];
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
ಸ್ಥಿರ bool slot_stale(ಧ್ರುವ ಸಂರಚನಾ obj_core_slot *slot, ಧ್ರುವ ಸಂರಚನಾ k_obj_core *core)
{
	ಪ್ರತ್ಯಾಗಮ core->type != slot->type;
}

ಸ್ಥಿರ ಸಂರಚನಾ obj_core_slot *slot_alloc(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ obj_core_slot *slot = slot_find(ರಿಕ್ತ);

	ಯದಿ (slot != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ಸಂರಚನಾ k_obj_core *core = slot_core(&registry[i]);

		ಯದಿ ((core != ರಿಕ್ತ) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, ರಿಕ್ತ);
			slot = &registry[i];
		}
	}

	ಪ್ರತ್ಯಾಗಮ slot;
}

ಸಂರಚನಾ k_obj_type *z_obj_type_init(ಸಂರಚನಾ k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->ಗಂಟು);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, ಪರಿಮಾಣ(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	ಪ್ರತ್ಯಾಗಮ type;
}

ಶೂನ್ಯ k_obj_type_init_range(ಸಂರಚನಾ k_obj_type *type, ಧ್ರುವ ಶೂನ್ಯ *start,
			   ಧ್ರುವ ಶೂನ್ಯ *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

ಶೂನ್ಯ k_obj_core_init(ಸಂರಚನಾ k_obj_core *obj_core, ಸಂರಚನಾ k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = ರಿಕ್ತ;
#endif /* CONFIG_OBJ_CORE_STATS */
}

ಶೂನ್ಯ k_obj_core_link(ಸಂರಚನಾ k_obj_core *obj_core)
{
	ಸಂರಚನಾ k_obj_type *type = obj_core->type;
	ಸಂರಚನಾ obj_core_slot *slot;

	ಯದಿ (type_ranges_contain(type, (ಧ್ರುವ uint8_t *)obj_core - type->obj_core_offset)) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	ಯದಿ (in_ರಾಶಿ_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	slot = slot_find(obj_core);
	ಯದಿ (slot == ರಿಕ್ತ) {
		slot = slot_alloc();
	}

	ಯದಿ (slot == ರಿಕ್ತ) {
		type->dropped++;
	} ಅನ್ಯಥಾ {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);
}

ಶೂನ್ಯ k_obj_core_init_and_link(ಸಂರಚನಾ k_obj_core *obj_core,
			      ಸಂರಚನಾ k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

ಶೂನ್ಯ k_obj_core_unlink(ಸಂರಚನಾ k_obj_core *obj_core)
{
	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ಯದಿ (atomic_ptr_cas(&registry[i].core, obj_core, ರಿಕ್ತ)) {
			ಭಙ್ಗ;
		}
	}
}

ಶೂನ್ಯ k_obj_core_evict_range(ಧ್ರುವ ಶೂನ್ಯ *addr, size_t len)
{
	ಧ್ರುವ uint8_t *start = addr;
	ಧ್ರುವ uint8_t *end = start + len;

	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ಸಂರಚನಾ k_obj_core *core = slot_core(&registry[i]);

		ಯದಿ ((core != ರಿಕ್ತ) && ((ಧ್ರುವ uint8_t *)core >= start) &&
		    ((ಧ್ರುವ uint8_t *)core < end)) {
			(ಶೂನ್ಯ)atomic_ptr_cas(&registry[i].core, core, ರಿಕ್ತ);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
ಶೂನ್ಯ sys_heap_release_hook(ಶೂನ್ಯ *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	ಯದಿ (k_is_user_context()) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
ಸ್ಥಿರ ಶೂನ್ಯ obj_core_init_all(ಶೂನ್ಯ)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->ಗಂಟು);

		ಪ್ರತಿ (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			ಧ್ರುವ ಸಂರಚನಾ k_obj_range *range = &type->statics[r];

			ಯದಿ (range->stride == 0U) {
				ಭಙ್ಗ;
			}

			ಪ್ರತಿ (ಧ್ರುವ uint8_t *elem = range->start;
			     elem < (ಧ್ರುವ uint8_t *)range->end; elem += range->stride) {
				ಸಂರಚನಾ k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				ಯದಿ ((type->stats_desc != ರಿಕ್ತ) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(ಶೂನ್ಯ *)((uint8_t *)obj_core -
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

ಸಂರಚನಾ k_obj_type *k_obj_type_find(uint32_t type_id)
{
	ಸಂರಚನಾ k_obj_type *rv = ರಿಕ್ತ;
	sys_snode_t *ಗಂಟು;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		ಯದಿ (type->id == type_id) {
			ಪ್ರತ್ಯಾಗಮ type;
		}
	}

	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_ಗಂಟು(&z_obj_type_list, ಗಂಟು) {
		ಸಂರಚನಾ k_obj_type *type = CONTAINER_OF(ಗಂಟು, ಸಂರಚನಾ k_obj_type, ಗಂಟು);

		ಯದಿ (type->id == type_id) {
			rv = type;
			ಭಙ್ಗ;
		}
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ walk_statics(ಸಂರಚನಾ k_obj_type *type,
			ಪೂರ್ಣಾಙ್ಕ (*func)(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *data),
			ಶೂನ್ಯ *data)
{
	ಪೂರ್ಣಾಙ್ಕ status = 0;

	ಪ್ರತಿ (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		ಧ್ರುವ ಸಂರಚನಾ k_obj_range *range = &type->statics[r];

		ಯದಿ (range->stride == 0U) {
			ಭಙ್ಗ;
		}

		ಪ್ರತಿ (ಧ್ರುವ uint8_t *elem = range->start; elem < (ಧ್ರುವ uint8_t *)range->end;
		     elem += range->stride) {
			ಸಂರಚನಾ k_obj_core *obj_core = range_core(type, range, elem);

			ಯದಿ (obj_core->type != type) {
				ಅನುವರ್ತ;
			}

			status = func(obj_core, data);
			ಯದಿ (status != 0) {
				ಭಙ್ಗ;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ status;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_type_walk_locked(ಸಂರಚನಾ k_obj_type *type,
			   ಪೂರ್ಣಾಙ್ಕ (*func)(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *data),
			   ಶೂನ್ಯ *data)
{
	ಪೂರ್ಣಾಙ್ಕ  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	ಪ್ರತಿ (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ಸಂರಚನಾ obj_core_slot *slot = &registry[i];
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);
		ಸಂರಚನಾ k_obj_core *core = slot_core(slot);

		ಯದಿ ((core == ರಿಕ್ತ) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, ಕೀಲಿ);
			ಅನುವರ್ತ;
		}

		ಯದಿ (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, ರಿಕ್ತ);
			k_spin_unlock(&obj_core_lock, ಕೀಲಿ);
			ಅನುವರ್ತ;
		}

		k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

		status = func(core, data);
	}

	ಪ್ರತ್ಯಾಗಮ status;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_type_walk_unlocked(ಸಂರಚನಾ k_obj_type *type,
			   ಪೂರ್ಣಾಙ್ಕ (*func)(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *data),
			   ಶೂನ್ಯ *data)
{
	ಪೂರ್ಣಾಙ್ಕ  status;

	status = walk_statics(type, func, data);

	ಪ್ರತಿ (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ಸಂರಚನಾ k_obj_core *core = slot_core(&registry[i]);

		ಯದಿ ((core == ರಿಕ್ತ) || (registry[i].type != type) ||
		    (core->type != type)) {
			ಅನುವರ್ತ;
		}

		status = func(core, data);
	}

	ಪ್ರತ್ಯಾಗಮ status;
}

#ifdef CONFIG_OBJ_CORE_STATS
ಪೂರ್ಣಾಙ್ಕ k_obj_core_stats_register(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats,
			      size_t stats_len)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	ಯದಿ (obj_core->type->stats_desc == ರಿಕ್ತ) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} ಅನ್ಯಥಾ ಯದಿ (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} ಅನ್ಯಥಾ {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_core_stats_deregister(ಸಂರಚನಾ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	ಯದಿ (obj_core->type->stats_desc == ರಿಕ್ತ) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} ಅನ್ಯಥಾ {
		obj_core->stats = ರಿಕ್ತ;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_core_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats,
			 size_t stats_len)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	ಸಂರಚನಾ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ಯದಿ ((desc == ರಿಕ್ತ) || (desc->raw == ರಿಕ್ತ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಅನ್ಯಥಾ ಯದಿ ((desc->raw_size != stats_len) || (obj_core->stats == ರಿಕ್ತ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ಅನ್ಯಥಾ {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_core_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats,
			   size_t stats_len)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	ಸಂರಚನಾ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ಯದಿ ((desc == ರಿಕ್ತ) || (desc->query == ರಿಕ್ತ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಅನ್ಯಥಾ ಯದಿ ((desc->query_size != stats_len) || (obj_core->stats == ರಿಕ್ತ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ಅನ್ಯಥಾ {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_core_stats_reset(ಸಂರಚನಾ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	ಸಂರಚನಾ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ಯದಿ ((desc == ರಿಕ್ತ) || (desc->reset == ರಿಕ್ತ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಅನ್ಯಥಾ ಯದಿ (obj_core->stats == ರಿಕ್ತ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ಅನ್ಯಥಾ {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_core_stats_disable(ಸಂರಚನಾ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	ಸಂರಚನಾ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ಯದಿ ((desc == ರಿಕ್ತ) || (desc->disable == ರಿಕ್ತ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಅನ್ಯಥಾ ಯದಿ (obj_core->stats == ರಿಕ್ತ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ಅನ್ಯಥಾ {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}

ಪೂರ್ಣಾಙ್ಕ k_obj_core_stats_enable(ಸಂರಚನಾ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	ಸಂರಚನಾ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ಯದಿ ((desc == ರಿಕ್ತ) || (desc->enable == ರಿಕ್ತ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಅನ್ಯಥಾ ಯದಿ (obj_core->stats == ರಿಕ್ತ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ಅನ್ಯಥಾ {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
ಸ್ಥಿರ ಸಂರಚನಾ k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ಸ್ಥಿರ ಸಂರಚನಾ k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = ಪರಿಮಾಣ(ಸಂರಚನಾ k_cycle_stats) * CONFIG_MP_MAX_ಅಂಕೆ_CPUS,
	.query_size = ಪರಿಮಾಣ(ಸಂರಚನಾ k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ರಿಕ್ತ,
	.disable = ರಿಕ್ತ,
	.enable  = ರಿಕ್ತ,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ init_kernel_obj_core_list(ಶೂನ್ಯ)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(ಸಂರಚನಾ z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  ಪರಿಮಾಣ(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
