/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/kernel/obj_core.h>
#অন্তর্ভাৱ <kernel_internal.h>

স্থির সংরচনা k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
সংরচনা obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	সংরচনা k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
স্থির সংরচনা obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

স্থির সংরচনা k_obj_core *slot_core(ধ্রুৱ সংরচনা obj_core_slot *slot)
{
	ঘূৰাই atomic_ptr_get(&slot->core);
}

স্থির bool range_contains(ধ্রুৱ সংরচনা k_obj_range *range, ধ্রুৱ শূন্য *ptr)
{
	যদি (range->indirect) {
		বাবে (ধ্রুৱ শূন্য *ধ্রুৱ *pp = range->start; pp < (ধ্রুৱ শূন্য *ধ্রুৱ *)range->end;
		     pp = (ধ্রুৱ শূন্য *ধ্রুৱ *)((ধ্রুৱ uint8_t *)pp + range->stride)) {
			যদি (*pp == ptr) {
				ঘূৰাই true;
			}
		}
		ঘূৰাই false;
	}

	ঘূৰাই (ptr >= range->start) && (ptr < range->end);
}

স্থির bool type_ranges_contain(ধ্রুৱ সংরচনা k_obj_type *type, ধ্রুৱ শূন্য *ptr)
{
	বাবে (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		ধ্রুৱ সংরচনা k_obj_range *range = &type->statics[i];

		যদি (range->stride == 0U) {
			ভঙ্গ;
		}
		যদি (range_contains(range, ptr)) {
			ঘূৰাই true;
		}
	}

	ঘূৰাই false;
}

/* Object core of the range element */
স্থির সংরচনা k_obj_core *range_core(ধ্রুৱ সংরচনা k_obj_type *type,
				     ধ্রুৱ সংরচনা k_obj_range *range, ধ্রুৱ শূন্য *elem)
{
	ধ্রুৱ uint8_t *obj = range->indirect ? *(ধ্রুৱ uint8_t *ধ্রুৱ *)elem : elem;

	ঘূৰাই (সংরচনা k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
স্থির bool in_স্তূপ_storage(ধ্রুৱ শূন্য *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ধ্রুৱ সংরচনা k_thread *thread = _current;
	uintptr_t irq_স্তূপ = (uintptr_t)K_KERNEL_স্তূপ_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_স্তূপ_end =
		irq_স্তূপ + K_KERNEL_স্তূপ_SIZEOF(z_interrupt_stacks[0]);

	যদি ((thread != রিক্ত) && (thread->স্তূপ_info.size != 0U) &&
	    (addr >= thread->স্তূপ_info.start) &&
	    (addr < thread->স্তূপ_info.start + thread->স্তূপ_info.size)) {
		ঘূৰাই true;
	}

	ঘূৰাই (addr >= irq_স্তূপ) && (addr < irq_স্তূপ_end);
}

স্থির সংরচনা obj_core_slot *slot_find(ধ্রুৱ সংরচনা k_obj_core *core)
{
	বাবে (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		যদি (slot_core(&registry[i]) == core) {
			ঘূৰাই &registry[i];
		}
	}

	ঘূৰাই রিক্ত;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
স্থির bool slot_stale(ধ্রুৱ সংরচনা obj_core_slot *slot, ধ্রুৱ সংরচনা k_obj_core *core)
{
	ঘূৰাই core->type != slot->type;
}

স্থির সংরচনা obj_core_slot *slot_alloc(শূন্য)
{
	সংরচনা obj_core_slot *slot = slot_find(রিক্ত);

	যদি (slot != রিক্ত) {
		ঘূৰাই slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	বাবে (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		সংরচনা k_obj_core *core = slot_core(&registry[i]);

		যদি ((core != রিক্ত) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, রিক্ত);
			slot = &registry[i];
		}
	}

	ঘূৰাই slot;
}

সংরচনা k_obj_type *z_obj_type_init(সংরচনা k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->গ্ৰন্থি);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, পরিমাণ(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	ঘূৰাই type;
}

শূন্য k_obj_type_init_range(সংরচনা k_obj_type *type, ধ্রুৱ শূন্য *start,
			   ধ্রুৱ শূন্য *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

শূন্য k_obj_core_init(সংরচনা k_obj_core *obj_core, সংরচনা k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = রিক্ত;
#endif /* CONFIG_OBJ_CORE_STATS */
}

শূন্য k_obj_core_link(সংরচনা k_obj_core *obj_core)
{
	সংরচনা k_obj_type *type = obj_core->type;
	সংরচনা obj_core_slot *slot;

	যদি (type_ranges_contain(type, (ধ্রুৱ uint8_t *)obj_core - type->obj_core_offset)) {
		ঘূৰাই;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	যদি (in_স্তূপ_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, চাবি);
		ঘূৰাই;
	}

	slot = slot_find(obj_core);
	যদি (slot == রিক্ত) {
		slot = slot_alloc();
	}

	যদি (slot == রিক্ত) {
		type->dropped++;
	} অন্যথা {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);
}

শূন্য k_obj_core_init_and_link(সংরচনা k_obj_core *obj_core,
			      সংরচনা k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

শূন্য k_obj_core_unlink(সংরচনা k_obj_core *obj_core)
{
	বাবে (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		যদি (atomic_ptr_cas(&registry[i].core, obj_core, রিক্ত)) {
			ভঙ্গ;
		}
	}
}

শূন্য k_obj_core_evict_range(ধ্রুৱ শূন্য *addr, size_t len)
{
	ধ্রুৱ uint8_t *start = addr;
	ধ্রুৱ uint8_t *end = start + len;

	বাবে (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		সংরচনা k_obj_core *core = slot_core(&registry[i]);

		যদি ((core != রিক্ত) && ((ধ্রুৱ uint8_t *)core >= start) &&
		    ((ধ্রুৱ uint8_t *)core < end)) {
			(শূন্য)atomic_ptr_cas(&registry[i].core, core, রিক্ত);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
শূন্য sys_heap_release_hook(শূন্য *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	যদি (k_is_user_context()) {
		ঘূৰাই;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
স্থির শূন্য obj_core_init_all(শূন্য)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->গ্ৰন্থি);

		বাবে (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			ধ্রুৱ সংরচনা k_obj_range *range = &type->statics[r];

			যদি (range->stride == 0U) {
				ভঙ্গ;
			}

			বাবে (ধ্রুৱ uint8_t *elem = range->start;
			     elem < (ধ্রুৱ uint8_t *)range->end; elem += range->stride) {
				সংরচনা k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				যদি ((type->stats_desc != রিক্ত) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(শূন্য *)((uint8_t *)obj_core -
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

সংরচনা k_obj_type *k_obj_type_find(uint32_t type_id)
{
	সংরচনা k_obj_type *rv = রিক্ত;
	sys_snode_t *গ্ৰন্থি;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		যদি (type->id == type_id) {
			ঘূৰাই type;
		}
	}

	k_spinlock_চাবি_t  চাবি = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_গ্ৰন্থি(&z_obj_type_list, গ্ৰন্থি) {
		সংরচনা k_obj_type *type = CONTAINER_OF(গ্ৰন্থি, সংরচনা k_obj_type, গ্ৰন্থি);

		যদি (type->id == type_id) {
			rv = type;
			ভঙ্গ;
		}
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
স্থির পূর্ণাঙ্ক walk_statics(সংরচনা k_obj_type *type,
			পূর্ণাঙ্ক (*func)(সংরচনা k_obj_core *obj_core, শূন্য *data),
			শূন্য *data)
{
	পূর্ণাঙ্ক status = 0;

	বাবে (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		ধ্রুৱ সংরচনা k_obj_range *range = &type->statics[r];

		যদি (range->stride == 0U) {
			ভঙ্গ;
		}

		বাবে (ধ্রুৱ uint8_t *elem = range->start; elem < (ধ্রুৱ uint8_t *)range->end;
		     elem += range->stride) {
			সংরচনা k_obj_core *obj_core = range_core(type, range, elem);

			যদি (obj_core->type != type) {
				অনুৱর্ত;
			}

			status = func(obj_core, data);
			যদি (status != 0) {
				ভঙ্গ;
			}
		}
	}

	ঘূৰাই status;
}

পূর্ণাঙ্ক k_obj_type_walk_locked(সংরচনা k_obj_type *type,
			   পূর্ণাঙ্ক (*func)(সংরচনা k_obj_core *obj_core, শূন্য *data),
			   শূন্য *data)
{
	পূর্ণাঙ্ক  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	বাবে (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		সংরচনা obj_core_slot *slot = &registry[i];
		k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);
		সংরচনা k_obj_core *core = slot_core(slot);

		যদি ((core == রিক্ত) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, চাবি);
			অনুৱর্ত;
		}

		যদি (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, রিক্ত);
			k_spin_unlock(&obj_core_lock, চাবি);
			অনুৱর্ত;
		}

		k_spin_unlock(&obj_core_lock, চাবি);

		status = func(core, data);
	}

	ঘূৰাই status;
}

পূর্ণাঙ্ক k_obj_type_walk_unlocked(সংরচনা k_obj_type *type,
			   পূর্ণাঙ্ক (*func)(সংরচনা k_obj_core *obj_core, শূন্য *data),
			   শূন্য *data)
{
	পূর্ণাঙ্ক  status;

	status = walk_statics(type, func, data);

	বাবে (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		সংরচনা k_obj_core *core = slot_core(&registry[i]);

		যদি ((core == রিক্ত) || (registry[i].type != type) ||
		    (core->type != type)) {
			অনুৱর্ত;
		}

		status = func(core, data);
	}

	ঘূৰাই status;
}

#ifdef CONFIG_OBJ_CORE_STATS
পূর্ণাঙ্ক k_obj_core_stats_register(সংরচনা k_obj_core *obj_core, শূন্য *stats,
			      size_t stats_len)
{
	পূর্ণাঙ্ক rv;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	যদি (obj_core->type->stats_desc == রিক্ত) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} অন্যথা {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}

পূর্ণাঙ্ক k_obj_core_stats_deregister(সংরচনা k_obj_core *obj_core)
{
	পূর্ণাঙ্ক rv;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	যদি (obj_core->type->stats_desc == রিক্ত) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} অন্যথা {
		obj_core->stats = রিক্ত;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}

পূর্ণাঙ্ক k_obj_core_stats_raw(সংরচনা k_obj_core *obj_core, শূন্য *stats,
			 size_t stats_len)
{
	পূর্ণাঙ্ক rv;
	সংরচনা k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == রিক্ত) || (desc->raw == রিক্ত)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি ((desc->raw_size != stats_len) || (obj_core->stats == রিক্ত)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}

পূর্ণাঙ্ক k_obj_core_stats_query(সংরচনা k_obj_core *obj_core, শূন্য *stats,
			   size_t stats_len)
{
	পূর্ণাঙ্ক rv;
	সংরচনা k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == রিক্ত) || (desc->query == রিক্ত)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি ((desc->query_size != stats_len) || (obj_core->stats == রিক্ত)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}

পূর্ণাঙ্ক k_obj_core_stats_reset(সংরচনা k_obj_core *obj_core)
{
	পূর্ণাঙ্ক rv;
	সংরচনা k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t  চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == রিক্ত) || (desc->reset == রিক্ত)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->stats == রিক্ত) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}

পূর্ণাঙ্ক k_obj_core_stats_disable(সংরচনা k_obj_core *obj_core)
{
	পূর্ণাঙ্ক rv;
	সংরচনা k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == রিক্ত) || (desc->disable == রিক্ত)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->stats == রিক্ত) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}

পূর্ণাঙ্ক k_obj_core_stats_enable(সংরচনা k_obj_core *obj_core)
{
	পূর্ণাঙ্ক rv;
	সংরচনা k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == রিক্ত) || (desc->enable == রিক্ত)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->stats == রিক্ত) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ঘূৰাই rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
স্থির সংরচনা k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
স্থির সংরচনা k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = পরিমাণ(সংরচনা k_cycle_stats) * CONFIG_MP_MAX_অঙ্ক_CPUS,
	.query_size = পরিমাণ(সংরচনা k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = রিক্ত,
	.disable = রিক্ত,
	.enable  = রিক্ত,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
স্থির শূন্য init_kernel_obj_core_list(শূন্য)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(সংরচনা z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  পরিমাণ(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
