/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/kernel/obj_core.h>
#અન્તર્ભાવ <kernel_internal.h>

સ્થિર સંરચના k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
સંરચના obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	સંરચના k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
સ્થિર સંરચના obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

સ્થિર સંરચના k_obj_core *slot_core(અચલ સંરચના obj_core_slot *slot)
{
	પાછા atomic_ptr_get(&slot->core);
}

સ્થિર bool range_contains(અચલ સંરચના k_obj_range *range, અચલ શૂન્ય *ptr)
{
	જો (range->indirect) {
		માટે (અચલ શૂન્ય *અચલ *pp = range->start; pp < (અચલ શૂન્ય *અચલ *)range->end;
		     pp = (અચલ શૂન્ય *અચલ *)((અચલ uint8_t *)pp + range->stride)) {
			જો (*pp == ptr) {
				પાછા true;
			}
		}
		પાછા false;
	}

	પાછા (ptr >= range->start) && (ptr < range->end);
}

સ્થિર bool type_ranges_contain(અચલ સંરચના k_obj_type *type, અચલ શૂન્ય *ptr)
{
	માટે (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		અચલ સંરચના k_obj_range *range = &type->statics[i];

		જો (range->stride == 0U) {
			તોડો;
		}
		જો (range_contains(range, ptr)) {
			પાછા true;
		}
	}

	પાછા false;
}

/* Object core of the range element */
સ્થિર સંરચના k_obj_core *range_core(અચલ સંરચના k_obj_type *type,
				     અચલ સંરચના k_obj_range *range, અચલ શૂન્ય *elem)
{
	અચલ uint8_t *obj = range->indirect ? *(અચલ uint8_t *અચલ *)elem : elem;

	પાછા (સંરચના k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
સ્થિર bool in_ઢગલો_storage(અચલ શૂન્ય *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	અચલ સંરચના k_thread *thread = _current;
	uintptr_t irq_ઢગલો = (uintptr_t)K_KERNEL_ઢગલો_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_ઢગલો_end =
		irq_ઢગલો + K_KERNEL_ઢગલો_SIZEOF(z_interrupt_stacks[0]);

	જો ((thread != ખાલી) && (thread->ઢગલો_info.size != 0U) &&
	    (addr >= thread->ઢગલો_info.start) &&
	    (addr < thread->ઢગલો_info.start + thread->ઢગલો_info.size)) {
		પાછા true;
	}

	પાછા (addr >= irq_ઢગલો) && (addr < irq_ઢગલો_end);
}

સ્થિર સંરચના obj_core_slot *slot_find(અચલ સંરચના k_obj_core *core)
{
	માટે (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		જો (slot_core(&registry[i]) == core) {
			પાછા &registry[i];
		}
	}

	પાછા ખાલી;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
સ્થિર bool slot_stale(અચલ સંરચના obj_core_slot *slot, અચલ સંરચના k_obj_core *core)
{
	પાછા core->type != slot->type;
}

સ્થિર સંરચના obj_core_slot *slot_alloc(શૂન્ય)
{
	સંરચના obj_core_slot *slot = slot_find(ખાલી);

	જો (slot != ખાલી) {
		પાછા slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	માટે (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		સંરચના k_obj_core *core = slot_core(&registry[i]);

		જો ((core != ખાલી) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, ખાલી);
			slot = &registry[i];
		}
	}

	પાછા slot;
}

સંરચના k_obj_type *z_obj_type_init(સંરચના k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->ગાંઠ);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, કદ(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	પાછા type;
}

શૂન્ય k_obj_type_init_range(સંરચના k_obj_type *type, અચલ શૂન્ય *start,
			   અચલ શૂન્ય *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

શૂન્ય k_obj_core_init(સંરચના k_obj_core *obj_core, સંરચના k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = ખાલી;
#endif /* CONFIG_OBJ_CORE_STATS */
}

શૂન્ય k_obj_core_link(સંરચના k_obj_core *obj_core)
{
	સંરચના k_obj_type *type = obj_core->type;
	સંરચના obj_core_slot *slot;

	જો (type_ranges_contain(type, (અચલ uint8_t *)obj_core - type->obj_core_offset)) {
		પાછા;
	}

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);

	જો (in_ઢગલો_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, ચાવી);
		પાછા;
	}

	slot = slot_find(obj_core);
	જો (slot == ખાલી) {
		slot = slot_alloc();
	}

	જો (slot == ખાલી) {
		type->dropped++;
	} નહીંતર {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, ચાવી);
}

શૂન્ય k_obj_core_init_and_link(સંરચના k_obj_core *obj_core,
			      સંરચના k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

શૂન્ય k_obj_core_unlink(સંરચના k_obj_core *obj_core)
{
	માટે (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		જો (atomic_ptr_cas(&registry[i].core, obj_core, ખાલી)) {
			તોડો;
		}
	}
}

શૂન્ય k_obj_core_evict_range(અચલ શૂન્ય *addr, size_t len)
{
	અચલ uint8_t *start = addr;
	અચલ uint8_t *end = start + len;

	માટે (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		સંરચના k_obj_core *core = slot_core(&registry[i]);

		જો ((core != ખાલી) && ((અચલ uint8_t *)core >= start) &&
		    ((અચલ uint8_t *)core < end)) {
			(શૂન્ય)atomic_ptr_cas(&registry[i].core, core, ખાલી);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
શૂન્ય sys_heap_release_hook(શૂન્ય *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	જો (k_is_user_context()) {
		પાછા;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
સ્થિર શૂન્ય obj_core_init_all(શૂન્ય)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->ગાંઠ);

		માટે (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			અચલ સંરચના k_obj_range *range = &type->statics[r];

			જો (range->stride == 0U) {
				તોડો;
			}

			માટે (અચલ uint8_t *elem = range->start;
			     elem < (અચલ uint8_t *)range->end; elem += range->stride) {
				સંરચના k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				જો ((type->stats_desc != ખાલી) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(શૂન્ય *)((uint8_t *)obj_core -
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

સંરચના k_obj_type *k_obj_type_find(uint32_t type_id)
{
	સંરચના k_obj_type *rv = ખાલી;
	sys_snode_t *ગાંઠ;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		જો (type->id == type_id) {
			પાછા type;
		}
	}

	k_spinlock_ચાવી_t  ચાવી = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_ગાંઠ(&z_obj_type_list, ગાંઠ) {
		સંરચના k_obj_type *type = CONTAINER_OF(ગાંઠ, સંરચના k_obj_type, ગાંઠ);

		જો (type->id == type_id) {
			rv = type;
			તોડો;
		}
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
સ્થિર પૂર્ણાંક walk_statics(સંરચના k_obj_type *type,
			પૂર્ણાંક (*func)(સંરચના k_obj_core *obj_core, શૂન્ય *data),
			શૂન્ય *data)
{
	પૂર્ણાંક status = 0;

	માટે (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		અચલ સંરચના k_obj_range *range = &type->statics[r];

		જો (range->stride == 0U) {
			તોડો;
		}

		માટે (અચલ uint8_t *elem = range->start; elem < (અચલ uint8_t *)range->end;
		     elem += range->stride) {
			સંરચના k_obj_core *obj_core = range_core(type, range, elem);

			જો (obj_core->type != type) {
				ચાલુ;
			}

			status = func(obj_core, data);
			જો (status != 0) {
				તોડો;
			}
		}
	}

	પાછા status;
}

પૂર્ણાંક k_obj_type_walk_locked(સંરચના k_obj_type *type,
			   પૂર્ણાંક (*func)(સંરચના k_obj_core *obj_core, શૂન્ય *data),
			   શૂન્ય *data)
{
	પૂર્ણાંક  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	માટે (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		સંરચના obj_core_slot *slot = &registry[i];
		k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);
		સંરચના k_obj_core *core = slot_core(slot);

		જો ((core == ખાલી) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, ચાવી);
			ચાલુ;
		}

		જો (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, ખાલી);
			k_spin_unlock(&obj_core_lock, ચાવી);
			ચાલુ;
		}

		k_spin_unlock(&obj_core_lock, ચાવી);

		status = func(core, data);
	}

	પાછા status;
}

પૂર્ણાંક k_obj_type_walk_unlocked(સંરચના k_obj_type *type,
			   પૂર્ણાંક (*func)(સંરચના k_obj_core *obj_core, શૂન્ય *data),
			   શૂન્ય *data)
{
	પૂર્ણાંક  status;

	status = walk_statics(type, func, data);

	માટે (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		સંરચના k_obj_core *core = slot_core(&registry[i]);

		જો ((core == ખાલી) || (registry[i].type != type) ||
		    (core->type != type)) {
			ચાલુ;
		}

		status = func(core, data);
	}

	પાછા status;
}

#ifdef CONFIG_OBJ_CORE_STATS
પૂર્ણાંક k_obj_core_stats_register(સંરચના k_obj_core *obj_core, શૂન્ય *stats,
			      size_t stats_len)
{
	પૂર્ણાંક rv;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);

	જો (obj_core->type->stats_desc == ખાલી) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} નહીંતર જો (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} નહીંતર {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}

પૂર્ણાંક k_obj_core_stats_deregister(સંરચના k_obj_core *obj_core)
{
	પૂર્ણાંક rv;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);

	જો (obj_core->type->stats_desc == ખાલી) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} નહીંતર {
		obj_core->stats = ખાલી;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}

પૂર્ણાંક k_obj_core_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats,
			 size_t stats_len)
{
	પૂર્ણાંક rv;
	સંરચના k_obj_core_stats_desc *desc;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	જો ((desc == ખાલી) || (desc->raw == ખાલી)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} નહીંતર જો ((desc->raw_size != stats_len) || (obj_core->stats == ખાલી)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} નહીંતર {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}

પૂર્ણાંક k_obj_core_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats,
			   size_t stats_len)
{
	પૂર્ણાંક rv;
	સંરચના k_obj_core_stats_desc *desc;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	જો ((desc == ખાલી) || (desc->query == ખાલી)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} નહીંતર જો ((desc->query_size != stats_len) || (obj_core->stats == ખાલી)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} નહીંતર {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}

પૂર્ણાંક k_obj_core_stats_reset(સંરચના k_obj_core *obj_core)
{
	પૂર્ણાંક rv;
	સંરચના k_obj_core_stats_desc *desc;

	k_spinlock_ચાવી_t  ચાવી = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	જો ((desc == ખાલી) || (desc->reset == ખાલી)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} નહીંતર જો (obj_core->stats == ખાલી) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} નહીંતર {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}

પૂર્ણાંક k_obj_core_stats_disable(સંરચના k_obj_core *obj_core)
{
	પૂર્ણાંક rv;
	સંરચના k_obj_core_stats_desc *desc;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	જો ((desc == ખાલી) || (desc->disable == ખાલી)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} નહીંતર જો (obj_core->stats == ખાલી) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} નહીંતર {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}

પૂર્ણાંક k_obj_core_stats_enable(સંરચના k_obj_core *obj_core)
{
	પૂર્ણાંક rv;
	સંરચના k_obj_core_stats_desc *desc;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	જો ((desc == ખાલી) || (desc->enable == ખાલી)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} નહીંતર જો (obj_core->stats == ખાલી) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} નહીંતર {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ચાવી);

	પાછા rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
સ્થિર સંરચના k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
સ્થિર સંરચના k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = કદ(સંરચના k_cycle_stats) * CONFIG_MP_MAX_અંક_CPUS,
	.query_size = કદ(સંરચના k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ખાલી,
	.disable = ખાલી,
	.enable  = ખાલી,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
સ્થિર શૂન્ય init_kernel_obj_core_list(શૂન્ય)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(સંરચના z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  કદ(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
