/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/kernel/obj_core.h>
#அந்தர்பாவ <kernel_internal.h>

நிலையான கட்டமைப்பு k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
கட்டமைப்பு obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	கட்டமைப்பு k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
நிலையான கட்டமைப்பு obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

நிலையான கட்டமைப்பு k_obj_core *slot_core(மாறிலி கட்டமைப்பு obj_core_slot *slot)
{
	திரும்பு atomic_ptr_get(&slot->core);
}

நிலையான bool range_contains(மாறிலி கட்டமைப்பு k_obj_range *range, மாறிலி வெறுமை *ptr)
{
	எனில் (range->indirect) {
		க்கு (மாறிலி வெறுமை *மாறிலி *pp = range->start; pp < (மாறிலி வெறுமை *மாறிலி *)range->end;
		     pp = (மாறிலி வெறுமை *மாறிலி *)((மாறிலி uint8_t *)pp + range->stride)) {
			எனில் (*pp == ptr) {
				திரும்பு true;
			}
		}
		திரும்பு false;
	}

	திரும்பு (ptr >= range->start) && (ptr < range->end);
}

நிலையான bool type_ranges_contain(மாறிலி கட்டமைப்பு k_obj_type *type, மாறிலி வெறுமை *ptr)
{
	க்கு (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		மாறிலி கட்டமைப்பு k_obj_range *range = &type->statics[i];

		எனில் (range->stride == 0U) {
			நிறுத்து;
		}
		எனில் (range_contains(range, ptr)) {
			திரும்பு true;
		}
	}

	திரும்பு false;
}

/* Object core of the range element */
நிலையான கட்டமைப்பு k_obj_core *range_core(மாறிலி கட்டமைப்பு k_obj_type *type,
				     மாறிலி கட்டமைப்பு k_obj_range *range, மாறிலி வெறுமை *elem)
{
	மாறிலி uint8_t *obj = range->indirect ? *(மாறிலி uint8_t *மாறிலி *)elem : elem;

	திரும்பு (கட்டமைப்பு k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
நிலையான bool in_அடுக்கு_storage(மாறிலி வெறுமை *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	மாறிலி கட்டமைப்பு k_thread *thread = _current;
	uintptr_t irq_அடுக்கு = (uintptr_t)K_KERNEL_அடுக்கு_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_அடுக்கு_end =
		irq_அடுக்கு + K_KERNEL_அடுக்கு_SIZEOF(z_interrupt_stacks[0]);

	எனில் ((thread != வெற்று) && (thread->அடுக்கு_info.size != 0U) &&
	    (addr >= thread->அடுக்கு_info.start) &&
	    (addr < thread->அடுக்கு_info.start + thread->அடுக்கு_info.size)) {
		திரும்பு true;
	}

	திரும்பு (addr >= irq_அடுக்கு) && (addr < irq_அடுக்கு_end);
}

நிலையான கட்டமைப்பு obj_core_slot *slot_find(மாறிலி கட்டமைப்பு k_obj_core *core)
{
	க்கு (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		எனில் (slot_core(&registry[i]) == core) {
			திரும்பு &registry[i];
		}
	}

	திரும்பு வெற்று;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
நிலையான bool slot_stale(மாறிலி கட்டமைப்பு obj_core_slot *slot, மாறிலி கட்டமைப்பு k_obj_core *core)
{
	திரும்பு core->type != slot->type;
}

நிலையான கட்டமைப்பு obj_core_slot *slot_alloc(வெறுமை)
{
	கட்டமைப்பு obj_core_slot *slot = slot_find(வெற்று);

	எனில் (slot != வெற்று) {
		திரும்பு slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	க்கு (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		கட்டமைப்பு k_obj_core *core = slot_core(&registry[i]);

		எனில் ((core != வெற்று) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, வெற்று);
			slot = &registry[i];
		}
	}

	திரும்பு slot;
}

கட்டமைப்பு k_obj_type *z_obj_type_init(கட்டமைப்பு k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->முனை);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, அளவு(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	திரும்பு type;
}

வெறுமை k_obj_type_init_range(கட்டமைப்பு k_obj_type *type, மாறிலி வெறுமை *start,
			   மாறிலி வெறுமை *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

வெறுமை k_obj_core_init(கட்டமைப்பு k_obj_core *obj_core, கட்டமைப்பு k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = வெற்று;
#endif /* CONFIG_OBJ_CORE_STATS */
}

வெறுமை k_obj_core_link(கட்டமைப்பு k_obj_core *obj_core)
{
	கட்டமைப்பு k_obj_type *type = obj_core->type;
	கட்டமைப்பு obj_core_slot *slot;

	எனில் (type_ranges_contain(type, (மாறிலி uint8_t *)obj_core - type->obj_core_offset)) {
		திரும்பு;
	}

	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);

	எனில் (in_அடுக்கு_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, சாவி);
		திரும்பு;
	}

	slot = slot_find(obj_core);
	எனில் (slot == வெற்று) {
		slot = slot_alloc();
	}

	எனில் (slot == வெற்று) {
		type->dropped++;
	} இல்லையெனில் {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, சாவி);
}

வெறுமை k_obj_core_init_and_link(கட்டமைப்பு k_obj_core *obj_core,
			      கட்டமைப்பு k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

வெறுமை k_obj_core_unlink(கட்டமைப்பு k_obj_core *obj_core)
{
	க்கு (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		எனில் (atomic_ptr_cas(&registry[i].core, obj_core, வெற்று)) {
			நிறுத்து;
		}
	}
}

வெறுமை k_obj_core_evict_range(மாறிலி வெறுமை *addr, size_t len)
{
	மாறிலி uint8_t *start = addr;
	மாறிலி uint8_t *end = start + len;

	க்கு (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		கட்டமைப்பு k_obj_core *core = slot_core(&registry[i]);

		எனில் ((core != வெற்று) && ((மாறிலி uint8_t *)core >= start) &&
		    ((மாறிலி uint8_t *)core < end)) {
			(வெறுமை)atomic_ptr_cas(&registry[i].core, core, வெற்று);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
வெறுமை sys_heap_release_hook(வெறுமை *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	எனில் (k_is_user_context()) {
		திரும்பு;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
நிலையான வெறுமை obj_core_init_all(வெறுமை)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->முனை);

		க்கு (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			மாறிலி கட்டமைப்பு k_obj_range *range = &type->statics[r];

			எனில் (range->stride == 0U) {
				நிறுத்து;
			}

			க்கு (மாறிலி uint8_t *elem = range->start;
			     elem < (மாறிலி uint8_t *)range->end; elem += range->stride) {
				கட்டமைப்பு k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				எனில் ((type->stats_desc != வெற்று) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(வெறுமை *)((uint8_t *)obj_core -
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

கட்டமைப்பு k_obj_type *k_obj_type_find(uint32_t type_id)
{
	கட்டமைப்பு k_obj_type *rv = வெற்று;
	sys_snode_t *முனை;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		எனில் (type->id == type_id) {
			திரும்பு type;
		}
	}

	k_spinlock_சாவி_t  சாவி = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_முனை(&z_obj_type_list, முனை) {
		கட்டமைப்பு k_obj_type *type = CONTAINER_OF(முனை, கட்டமைப்பு k_obj_type, முனை);

		எனில் (type->id == type_id) {
			rv = type;
			நிறுத்து;
		}
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
நிலையான முழு walk_statics(கட்டமைப்பு k_obj_type *type,
			முழு (*func)(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *data),
			வெறுமை *data)
{
	முழு status = 0;

	க்கு (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		மாறிலி கட்டமைப்பு k_obj_range *range = &type->statics[r];

		எனில் (range->stride == 0U) {
			நிறுத்து;
		}

		க்கு (மாறிலி uint8_t *elem = range->start; elem < (மாறிலி uint8_t *)range->end;
		     elem += range->stride) {
			கட்டமைப்பு k_obj_core *obj_core = range_core(type, range, elem);

			எனில் (obj_core->type != type) {
				தொடர்;
			}

			status = func(obj_core, data);
			எனில் (status != 0) {
				நிறுத்து;
			}
		}
	}

	திரும்பு status;
}

முழு k_obj_type_walk_locked(கட்டமைப்பு k_obj_type *type,
			   முழு (*func)(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *data),
			   வெறுமை *data)
{
	முழு  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	க்கு (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		கட்டமைப்பு obj_core_slot *slot = &registry[i];
		k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);
		கட்டமைப்பு k_obj_core *core = slot_core(slot);

		எனில் ((core == வெற்று) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, சாவி);
			தொடர்;
		}

		எனில் (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, வெற்று);
			k_spin_unlock(&obj_core_lock, சாவி);
			தொடர்;
		}

		k_spin_unlock(&obj_core_lock, சாவி);

		status = func(core, data);
	}

	திரும்பு status;
}

முழு k_obj_type_walk_unlocked(கட்டமைப்பு k_obj_type *type,
			   முழு (*func)(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *data),
			   வெறுமை *data)
{
	முழு  status;

	status = walk_statics(type, func, data);

	க்கு (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		கட்டமைப்பு k_obj_core *core = slot_core(&registry[i]);

		எனில் ((core == வெற்று) || (registry[i].type != type) ||
		    (core->type != type)) {
			தொடர்;
		}

		status = func(core, data);
	}

	திரும்பு status;
}

#ifdef CONFIG_OBJ_CORE_STATS
முழு k_obj_core_stats_register(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats,
			      size_t stats_len)
{
	முழு rv;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);

	எனில் (obj_core->type->stats_desc == வெற்று) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} இல்லையெனில் எனில் (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} இல்லையெனில் {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}

முழு k_obj_core_stats_deregister(கட்டமைப்பு k_obj_core *obj_core)
{
	முழு rv;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);

	எனில் (obj_core->type->stats_desc == வெற்று) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} இல்லையெனில் {
		obj_core->stats = வெற்று;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}

முழு k_obj_core_stats_raw(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats,
			 size_t stats_len)
{
	முழு rv;
	கட்டமைப்பு k_obj_core_stats_desc *desc;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	எனில் ((desc == வெற்று) || (desc->raw == வெற்று)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} இல்லையெனில் எனில் ((desc->raw_size != stats_len) || (obj_core->stats == வெற்று)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} இல்லையெனில் {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}

முழு k_obj_core_stats_query(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats,
			   size_t stats_len)
{
	முழு rv;
	கட்டமைப்பு k_obj_core_stats_desc *desc;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	எனில் ((desc == வெற்று) || (desc->query == வெற்று)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} இல்லையெனில் எனில் ((desc->query_size != stats_len) || (obj_core->stats == வெற்று)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} இல்லையெனில் {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}

முழு k_obj_core_stats_reset(கட்டமைப்பு k_obj_core *obj_core)
{
	முழு rv;
	கட்டமைப்பு k_obj_core_stats_desc *desc;

	k_spinlock_சாவி_t  சாவி = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	எனில் ((desc == வெற்று) || (desc->reset == வெற்று)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} இல்லையெனில் எனில் (obj_core->stats == வெற்று) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} இல்லையெனில் {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}

முழு k_obj_core_stats_disable(கட்டமைப்பு k_obj_core *obj_core)
{
	முழு rv;
	கட்டமைப்பு k_obj_core_stats_desc *desc;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	எனில் ((desc == வெற்று) || (desc->disable == வெற்று)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} இல்லையெனில் எனில் (obj_core->stats == வெற்று) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} இல்லையெனில் {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}

முழு k_obj_core_stats_enable(கட்டமைப்பு k_obj_core *obj_core)
{
	முழு rv;
	கட்டமைப்பு k_obj_core_stats_desc *desc;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	எனில் ((desc == வெற்று) || (desc->enable == வெற்று)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} இல்லையெனில் எனில் (obj_core->stats == வெற்று) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} இல்லையெனில் {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, சாவி);

	திரும்பு rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
நிலையான கட்டமைப்பு k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
நிலையான கட்டமைப்பு k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = அளவு(கட்டமைப்பு k_cycle_stats) * CONFIG_MP_MAX_இலக்கம்_CPUS,
	.query_size = அளவு(கட்டமைப்பு k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = வெற்று,
	.disable = வெற்று,
	.enable  = வெற்று,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
நிலையான வெறுமை init_kernel_obj_core_list(வெறுமை)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(கட்டமைப்பு z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  அளவு(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
