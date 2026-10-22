/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/kernel/obj_core.h>
#অন্তর্ভাব <kernel_internal.h>

স্থির কাঠামো k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
কাঠামো obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	কাঠামো k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
স্থির কাঠামো obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

স্থির কাঠামো k_obj_core *slot_core(ধ্রুবক কাঠামো obj_core_slot *slot)
{
	ফেরত atomic_ptr_get(&slot->core);
}

স্থির bool range_contains(ধ্রুবক কাঠামো k_obj_range *range, ধ্রুবক শূন্য *ptr)
{
	যদি (range->indirect) {
		জন্য (ধ্রুবক শূন্য *ধ্রুবক *pp = range->start; pp < (ধ্রুবক শূন্য *ধ্রুবক *)range->end;
		     pp = (ধ্রুবক শূন্য *ধ্রুবক *)((ধ্রুবক uint8_t *)pp + range->stride)) {
			যদি (*pp == ptr) {
				ফেরত true;
			}
		}
		ফেরত false;
	}

	ফেরত (ptr >= range->start) && (ptr < range->end);
}

স্থির bool type_ranges_contain(ধ্রুবক কাঠামো k_obj_type *type, ধ্রুবক শূন্য *ptr)
{
	জন্য (size_t i = 0; i < ARRAY_আকার(type->statics); i++) {
		ধ্রুবক কাঠামো k_obj_range *range = &type->statics[i];

		যদি (range->stride == 0U) {
			বিরতি;
		}
		যদি (range_contains(range, ptr)) {
			ফেরত true;
		}
	}

	ফেরত false;
}

/* Object core of the range element */
স্থির কাঠামো k_obj_core *range_core(ধ্রুবক কাঠামো k_obj_type *type,
				     ধ্রুবক কাঠামো k_obj_range *range, ধ্রুবক শূন্য *elem)
{
	ধ্রুবক uint8_t *obj = range->indirect ? *(ধ্রুবক uint8_t *ধ্রুবক *)elem : elem;

	ফেরত (কাঠামো k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
স্থির bool in_স্তূপ_storage(ধ্রুবক শূন্য *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ধ্রুবক কাঠামো k_thread *thread = _current;
	uintptr_t irq_স্তূপ = (uintptr_t)K_KERNEL_স্তূপ_বাফার(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_স্তূপ_end =
		irq_স্তূপ + K_KERNEL_স্তূপ_SIZEOF(z_interrupt_stacks[0]);

	যদি ((thread != ফাঁকা) && (thread->স্তূপ_info.আকার_2 != 0U) &&
	    (addr >= thread->স্তূপ_info.start) &&
	    (addr < thread->স্তূপ_info.start + thread->স্তূপ_info.আকার_2)) {
		ফেরত true;
	}

	ফেরত (addr >= irq_স্তূপ) && (addr < irq_স্তূপ_end);
}

স্থির কাঠামো obj_core_slot *slot_find(ধ্রুবক কাঠামো k_obj_core *core)
{
	জন্য (size_t i = 0; i < ARRAY_আকার(registry); i++) {
		যদি (slot_core(&registry[i]) == core) {
			ফেরত &registry[i];
		}
	}

	ফেরত ফাঁকা;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
স্থির bool slot_stale(ধ্রুবক কাঠামো obj_core_slot *slot, ধ্রুবক কাঠামো k_obj_core *core)
{
	ফেরত core->type != slot->type;
}

স্থির কাঠামো obj_core_slot *slot_alloc(শূন্য)
{
	কাঠামো obj_core_slot *slot = slot_find(ফাঁকা);

	যদি (slot != ফাঁকা) {
		ফেরত slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	জন্য (size_t i = 0; i < ARRAY_আকার(registry); i++) {
		কাঠামো k_obj_core *core = slot_core(&registry[i]);

		যদি ((core != ফাঁকা) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, ফাঁকা);
			slot = &registry[i];
		}
	}

	ফেরত slot;
}

কাঠামো k_obj_type *z_obj_type_init(কাঠামো k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->গ্রন্থি);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, আকার(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	ফেরত type;
}

শূন্য k_obj_type_init_range(কাঠামো k_obj_type *type, ধ্রুবক শূন্য *start,
			   ধ্রুবক শূন্য *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

শূন্য k_obj_core_init(কাঠামো k_obj_core *obj_core, কাঠামো k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = ফাঁকা;
#endif /* CONFIG_OBJ_CORE_STATS */
}

শূন্য k_obj_core_link(কাঠামো k_obj_core *obj_core)
{
	কাঠামো k_obj_type *type = obj_core->type;
	কাঠামো obj_core_slot *slot;

	যদি (type_ranges_contain(type, (ধ্রুবক uint8_t *)obj_core - type->obj_core_offset)) {
		ফেরত;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	যদি (in_স্তূপ_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, চাবি);
		ফেরত;
	}

	slot = slot_find(obj_core);
	যদি (slot == ফাঁকা) {
		slot = slot_alloc();
	}

	যদি (slot == ফাঁকা) {
		type->dropped++;
	} অন্যথা {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);
}

শূন্য k_obj_core_init_and_link(কাঠামো k_obj_core *obj_core,
			      কাঠামো k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

শূন্য k_obj_core_unlink(কাঠামো k_obj_core *obj_core)
{
	জন্য (size_t i = 0; i < ARRAY_আকার(registry); i++) {
		যদি (atomic_ptr_cas(&registry[i].core, obj_core, ফাঁকা)) {
			বিরতি;
		}
	}
}

শূন্য k_obj_core_evict_range(ধ্রুবক শূন্য *addr, size_t len)
{
	ধ্রুবক uint8_t *start = addr;
	ধ্রুবক uint8_t *end = start + len;

	জন্য (size_t i = 0; i < ARRAY_আকার(registry); i++) {
		কাঠামো k_obj_core *core = slot_core(&registry[i]);

		যদি ((core != ফাঁকা) && ((ধ্রুবক uint8_t *)core >= start) &&
		    ((ধ্রুবক uint8_t *)core < end)) {
			(শূন্য)atomic_ptr_cas(&registry[i].core, core, ফাঁকা);
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
		ফেরত;
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
		sys_slist_append(&z_obj_type_list, &type->গ্রন্থি);

		জন্য (size_t r = 0; r < ARRAY_আকার(type->statics); r++) {
			ধ্রুবক কাঠামো k_obj_range *range = &type->statics[r];

			যদি (range->stride == 0U) {
				বিরতি;
			}

			জন্য (ধ্রুবক uint8_t *elem = range->start;
			     elem < (ধ্রুবক uint8_t *)range->end; elem += range->stride) {
				কাঠামো k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				যদি ((type->stats_desc != ফাঁকা) && (type->stats_আকার != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(শূন্য *)((uint8_t *)obj_core -
							 type->obj_core_offset +
							 type->stats_offset),
						type->stats_আকার);
				}
#endif /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

কাঠামো k_obj_type *k_obj_type_find(uint32_t type_id)
{
	কাঠামো k_obj_type *rv = ফাঁকা;
	sys_snode_t *গ্রন্থি;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		যদি (type->id == type_id) {
			ফেরত type;
		}
	}

	k_spinlock_চাবি_t  চাবি = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_গ্রন্থি(&z_obj_type_list, গ্রন্থি) {
		কাঠামো k_obj_type *type = CONTAINER_OF(গ্রন্থি, কাঠামো k_obj_type, গ্রন্থি);

		যদি (type->id == type_id) {
			rv = type;
			বিরতি;
		}
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
স্থির পূর্ণসংখ্যা walk_statics(কাঠামো k_obj_type *type,
			পূর্ণসংখ্যা (*func)(কাঠামো k_obj_core *obj_core, শূন্য *তথ্য),
			শূন্য *তথ্য)
{
	পূর্ণসংখ্যা status = 0;

	জন্য (size_t r = 0; (status == 0) && (r < ARRAY_আকার(type->statics)); r++) {
		ধ্রুবক কাঠামো k_obj_range *range = &type->statics[r];

		যদি (range->stride == 0U) {
			বিরতি;
		}

		জন্য (ধ্রুবক uint8_t *elem = range->start; elem < (ধ্রুবক uint8_t *)range->end;
		     elem += range->stride) {
			কাঠামো k_obj_core *obj_core = range_core(type, range, elem);

			যদি (obj_core->type != type) {
				চালু;
			}

			status = func(obj_core, তথ্য);
			যদি (status != 0) {
				বিরতি;
			}
		}
	}

	ফেরত status;
}

পূর্ণসংখ্যা k_obj_type_walk_locked(কাঠামো k_obj_type *type,
			   পূর্ণসংখ্যা (*func)(কাঠামো k_obj_core *obj_core, শূন্য *তথ্য),
			   শূন্য *তথ্য)
{
	পূর্ণসংখ্যা  status;

	status = walk_statics(type, func, তথ্য);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	জন্য (size_t i = 0; (status == 0) && (i < ARRAY_আকার(registry)); i++) {
		কাঠামো obj_core_slot *slot = &registry[i];
		k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);
		কাঠামো k_obj_core *core = slot_core(slot);

		যদি ((core == ফাঁকা) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, চাবি);
			চালু;
		}

		যদি (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, ফাঁকা);
			k_spin_unlock(&obj_core_lock, চাবি);
			চালু;
		}

		k_spin_unlock(&obj_core_lock, চাবি);

		status = func(core, তথ্য);
	}

	ফেরত status;
}

পূর্ণসংখ্যা k_obj_type_walk_unlocked(কাঠামো k_obj_type *type,
			   পূর্ণসংখ্যা (*func)(কাঠামো k_obj_core *obj_core, শূন্য *তথ্য),
			   শূন্য *তথ্য)
{
	পূর্ণসংখ্যা  status;

	status = walk_statics(type, func, তথ্য);

	জন্য (size_t i = 0; (status == 0) && (i < ARRAY_আকার(registry)); i++) {
		কাঠামো k_obj_core *core = slot_core(&registry[i]);

		যদি ((core == ফাঁকা) || (registry[i].type != type) ||
		    (core->type != type)) {
			চালু;
		}

		status = func(core, তথ্য);
	}

	ফেরত status;
}

#ifdef CONFIG_OBJ_CORE_STATS
পূর্ণসংখ্যা k_obj_core_stats_register(কাঠামো k_obj_core *obj_core, শূন্য *stats,
			      size_t stats_len)
{
	পূর্ণসংখ্যা rv;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	যদি (obj_core->type->stats_desc == ফাঁকা) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->type->stats_desc->raw_আকার != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} অন্যথা {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}

পূর্ণসংখ্যা k_obj_core_stats_deregister(কাঠামো k_obj_core *obj_core)
{
	পূর্ণসংখ্যা rv;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	যদি (obj_core->type->stats_desc == ফাঁকা) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} অন্যথা {
		obj_core->stats = ফাঁকা;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}

পূর্ণসংখ্যা k_obj_core_stats_raw(কাঠামো k_obj_core *obj_core, শূন্য *stats,
			 size_t stats_len)
{
	পূর্ণসংখ্যা rv;
	কাঠামো k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == ফাঁকা) || (desc->raw == ফাঁকা)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি ((desc->raw_আকার != stats_len) || (obj_core->stats == ফাঁকা)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}

পূর্ণসংখ্যা k_obj_core_stats_query(কাঠামো k_obj_core *obj_core, শূন্য *stats,
			   size_t stats_len)
{
	পূর্ণসংখ্যা rv;
	কাঠামো k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == ফাঁকা) || (desc->query == ফাঁকা)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি ((desc->query_আকার != stats_len) || (obj_core->stats == ফাঁকা)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}

পূর্ণসংখ্যা k_obj_core_stats_reset(কাঠামো k_obj_core *obj_core)
{
	পূর্ণসংখ্যা rv;
	কাঠামো k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t  চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == ফাঁকা) || (desc->reset == ফাঁকা)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->stats == ফাঁকা) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}

পূর্ণসংখ্যা k_obj_core_stats_disable(কাঠামো k_obj_core *obj_core)
{
	পূর্ণসংখ্যা rv;
	কাঠামো k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == ফাঁকা) || (desc->disable == ফাঁকা)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->stats == ফাঁকা) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}

পূর্ণসংখ্যা k_obj_core_stats_enable(কাঠামো k_obj_core *obj_core)
{
	পূর্ণসংখ্যা rv;
	কাঠামো k_obj_core_stats_desc *desc;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	যদি ((desc == ফাঁকা) || (desc->enable == ফাঁকা)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} অন্যথা যদি (obj_core->stats == ফাঁকা) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} অন্যথা {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, চাবি);

	ফেরত rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
স্থির কাঠামো k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
স্থির কাঠামো k_obj_core_stats_desc kernel_stats_desc = {
	.raw_আকার = আকার(কাঠামো k_cycle_stats) * CONFIG_MP_MAX_অঙ্ক_CPUS,
	.query_আকার = আকার(কাঠামো k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ফাঁকা,
	.disable = ফাঁকা,
	.enable  = ফাঁকা,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
স্থির শূন্য init_kernel_obj_core_list(শূন্য)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(কাঠামো z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  আকার(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
