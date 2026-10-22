/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <string.h>
#شامل <zephyr/kernel.h>
#شامل <zephyr/kernel/obj_core.h>
#شامل <kernel_internal.h>

ایستا ساختار k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
ساختار obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	ساختار k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
ایستا ساختار obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

ایستا ساختار k_obj_core *slot_core(ثابت ساختار obj_core_slot *slot)
{
	بازگشت atomic_ptr_get(&slot->core);
}

ایستا bool range_contains(ثابت ساختار k_obj_range *range, ثابت تهی *ptr)
{
	اگر (range->indirect) {
		برای (ثابت تهی *ثابت *pp = range->start; pp < (ثابت تهی *ثابت *)range->end;
		     pp = (ثابت تهی *ثابت *)((ثابت uint8_t *)pp + range->stride)) {
			اگر (*pp == ptr) {
				بازگشت true;
			}
		}
		بازگشت false;
	}

	بازگشت (ptr >= range->start) && (ptr < range->end);
}

ایستا bool type_ranges_contain(ثابت ساختار k_obj_type *type, ثابت تهی *ptr)
{
	برای (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		ثابت ساختار k_obj_range *range = &type->statics[i];

		اگر (range->stride == 0U) {
			بشکن;
		}
		اگر (range_contains(range, ptr)) {
			بازگشت true;
		}
	}

	بازگشت false;
}

/* Object core of the range element */
ایستا ساختار k_obj_core *range_core(ثابت ساختار k_obj_type *type,
				     ثابت ساختار k_obj_range *range, ثابت تهی *elem)
{
	ثابت uint8_t *obj = range->indirect ? *(ثابت uint8_t *ثابت *)elem : elem;

	بازگشت (ساختار k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
ایستا bool in_پشته_storage(ثابت تهی *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ثابت ساختار k_thread *thread = _current;
	uintptr_t irq_پشته = (uintptr_t)K_KERNEL_پشته_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_پشته_end =
		irq_پشته + K_KERNEL_پشته_SIZEOF(z_interrupt_stacks[0]);

	اگر ((thread != پوچ) && (thread->پشته_info.size != 0U) &&
	    (addr >= thread->پشته_info.start) &&
	    (addr < thread->پشته_info.start + thread->پشته_info.size)) {
		بازگشت true;
	}

	بازگشت (addr >= irq_پشته) && (addr < irq_پشته_end);
}

ایستا ساختار obj_core_slot *slot_find(ثابت ساختار k_obj_core *core)
{
	برای (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		اگر (slot_core(&registry[i]) == core) {
			بازگشت &registry[i];
		}
	}

	بازگشت پوچ;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
ایستا bool slot_stale(ثابت ساختار obj_core_slot *slot, ثابت ساختار k_obj_core *core)
{
	بازگشت core->type != slot->type;
}

ایستا ساختار obj_core_slot *slot_alloc(تهی)
{
	ساختار obj_core_slot *slot = slot_find(پوچ);

	اگر (slot != پوچ) {
		بازگشت slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	برای (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ساختار k_obj_core *core = slot_core(&registry[i]);

		اگر ((core != پوچ) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, پوچ);
			slot = &registry[i];
		}
	}

	بازگشت slot;
}

ساختار k_obj_type *z_obj_type_init(ساختار k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->گره);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, اندازه(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	بازگشت type;
}

تهی k_obj_type_init_range(ساختار k_obj_type *type, ثابت تهی *start,
			   ثابت تهی *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

تهی k_obj_core_init(ساختار k_obj_core *obj_core, ساختار k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = پوچ;
#endif /* CONFIG_OBJ_CORE_STATS */
}

تهی k_obj_core_link(ساختار k_obj_core *obj_core)
{
	ساختار k_obj_type *type = obj_core->type;
	ساختار obj_core_slot *slot;

	اگر (type_ranges_contain(type, (ثابت uint8_t *)obj_core - type->obj_core_offset)) {
		بازگشت;
	}

	k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);

	اگر (in_پشته_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, کلید);
		بازگشت;
	}

	slot = slot_find(obj_core);
	اگر (slot == پوچ) {
		slot = slot_alloc();
	}

	اگر (slot == پوچ) {
		type->dropped++;
	} وگرنه {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, کلید);
}

تهی k_obj_core_init_and_link(ساختار k_obj_core *obj_core,
			      ساختار k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

تهی k_obj_core_unlink(ساختار k_obj_core *obj_core)
{
	برای (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		اگر (atomic_ptr_cas(&registry[i].core, obj_core, پوچ)) {
			بشکن;
		}
	}
}

تهی k_obj_core_evict_range(ثابت تهی *addr, size_t len)
{
	ثابت uint8_t *start = addr;
	ثابت uint8_t *end = start + len;

	برای (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ساختار k_obj_core *core = slot_core(&registry[i]);

		اگر ((core != پوچ) && ((ثابت uint8_t *)core >= start) &&
		    ((ثابت uint8_t *)core < end)) {
			(تهی)atomic_ptr_cas(&registry[i].core, core, پوچ);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
تهی sys_heap_release_hook(تهی *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	اگر (k_is_user_context()) {
		بازگشت;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
ایستا تهی obj_core_init_all(تهی)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->گره);

		برای (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			ثابت ساختار k_obj_range *range = &type->statics[r];

			اگر (range->stride == 0U) {
				بشکن;
			}

			برای (ثابت uint8_t *elem = range->start;
			     elem < (ثابت uint8_t *)range->end; elem += range->stride) {
				ساختار k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				اگر ((type->stats_desc != پوچ) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(تهی *)((uint8_t *)obj_core -
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

ساختار k_obj_type *k_obj_type_find(uint32_t type_id)
{
	ساختار k_obj_type *rv = پوچ;
	sys_snode_t *گره;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		اگر (type->id == type_id) {
			بازگشت type;
		}
	}

	k_spinlock_کلید_t  کلید = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_گره(&z_obj_type_list, گره) {
		ساختار k_obj_type *type = CONTAINER_OF(گره, ساختار k_obj_type, گره);

		اگر (type->id == type_id) {
			rv = type;
			بشکن;
		}
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
ایستا صحیح walk_statics(ساختار k_obj_type *type,
			صحیح (*func)(ساختار k_obj_core *obj_core, تهی *data),
			تهی *data)
{
	صحیح status = 0;

	برای (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		ثابت ساختار k_obj_range *range = &type->statics[r];

		اگر (range->stride == 0U) {
			بشکن;
		}

		برای (ثابت uint8_t *elem = range->start; elem < (ثابت uint8_t *)range->end;
		     elem += range->stride) {
			ساختار k_obj_core *obj_core = range_core(type, range, elem);

			اگر (obj_core->type != type) {
				ادامه;
			}

			status = func(obj_core, data);
			اگر (status != 0) {
				بشکن;
			}
		}
	}

	بازگشت status;
}

صحیح k_obj_type_walk_locked(ساختار k_obj_type *type,
			   صحیح (*func)(ساختار k_obj_core *obj_core, تهی *data),
			   تهی *data)
{
	صحیح  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	برای (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ساختار obj_core_slot *slot = &registry[i];
		k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);
		ساختار k_obj_core *core = slot_core(slot);

		اگر ((core == پوچ) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, کلید);
			ادامه;
		}

		اگر (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, پوچ);
			k_spin_unlock(&obj_core_lock, کلید);
			ادامه;
		}

		k_spin_unlock(&obj_core_lock, کلید);

		status = func(core, data);
	}

	بازگشت status;
}

صحیح k_obj_type_walk_unlocked(ساختار k_obj_type *type,
			   صحیح (*func)(ساختار k_obj_core *obj_core, تهی *data),
			   تهی *data)
{
	صحیح  status;

	status = walk_statics(type, func, data);

	برای (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ساختار k_obj_core *core = slot_core(&registry[i]);

		اگر ((core == پوچ) || (registry[i].type != type) ||
		    (core->type != type)) {
			ادامه;
		}

		status = func(core, data);
	}

	بازگشت status;
}

#ifdef CONFIG_OBJ_CORE_STATS
صحیح k_obj_core_stats_register(ساختار k_obj_core *obj_core, تهی *stats,
			      size_t stats_len)
{
	صحیح rv;
	k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);

	اگر (obj_core->type->stats_desc == پوچ) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} وگرنه اگر (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} وگرنه {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}

صحیح k_obj_core_stats_deregister(ساختار k_obj_core *obj_core)
{
	صحیح rv;
	k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);

	اگر (obj_core->type->stats_desc == پوچ) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} وگرنه {
		obj_core->stats = پوچ;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}

صحیح k_obj_core_stats_raw(ساختار k_obj_core *obj_core, تهی *stats,
			 size_t stats_len)
{
	صحیح rv;
	ساختار k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	اگر ((desc == پوچ) || (desc->raw == پوچ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وگرنه اگر ((desc->raw_size != stats_len) || (obj_core->stats == پوچ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} وگرنه {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}

صحیح k_obj_core_stats_query(ساختار k_obj_core *obj_core, تهی *stats,
			   size_t stats_len)
{
	صحیح rv;
	ساختار k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	اگر ((desc == پوچ) || (desc->query == پوچ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وگرنه اگر ((desc->query_size != stats_len) || (obj_core->stats == پوچ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} وگرنه {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}

صحیح k_obj_core_stats_reset(ساختار k_obj_core *obj_core)
{
	صحیح rv;
	ساختار k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t  کلید = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	اگر ((desc == پوچ) || (desc->reset == پوچ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وگرنه اگر (obj_core->stats == پوچ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} وگرنه {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}

صحیح k_obj_core_stats_disable(ساختار k_obj_core *obj_core)
{
	صحیح rv;
	ساختار k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	اگر ((desc == پوچ) || (desc->disable == پوچ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وگرنه اگر (obj_core->stats == پوچ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} وگرنه {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}

صحیح k_obj_core_stats_enable(ساختار k_obj_core *obj_core)
{
	صحیح rv;
	ساختار k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	اگر ((desc == پوچ) || (desc->enable == پوچ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وگرنه اگر (obj_core->stats == پوچ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} وگرنه {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, کلید);

	بازگشت rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
ایستا ساختار k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ایستا ساختار k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = اندازه(ساختار k_cycle_stats) * CONFIG_MP_MAX_رقم_CPUS,
	.query_size = اندازه(ساختار k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = پوچ,
	.disable = پوچ,
	.enable  = پوچ,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
ایستا تهی init_kernel_obj_core_list(تهی)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(ساختار z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  اندازه(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
