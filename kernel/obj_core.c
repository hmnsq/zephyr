/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <string.h>
#شامل <zephyr/kernel.h>
#شامل <zephyr/kernel/obj_core.h>
#شامل <kernel_internal.h>

ساکن ساخت k_spinlock  obj_core_تالا;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
ساخت obj_core_خانہ {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	ساخت k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
ساکن ساخت obj_core_خانہ registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

ساکن ساخت k_obj_core *خانہ_core(ثابت ساخت obj_core_خانہ *خانہ)
{
	واپس atomic_ptr_get(&خانہ->core);
}

ساکن bool range_شامل_ہے(ثابت ساخت k_obj_range *range, ثابت خالی *ptr)
{
	اگر (range->بالواسطہ) {
		برائے (ثابت خالی *ثابت *pp = range->آغاز; pp < (ثابت خالی *ثابت *)range->انتہا;
		     pp = (ثابت خالی *ثابت *)((ثابت uint8_t *)pp + range->stride)) {
			اگر (*pp == ptr) {
				واپس true;
			}
		}
		واپس false;
	}

	واپس (ptr >= range->آغاز) && (ptr < range->انتہا);
}

ساکن bool type_ranges_contain(ثابت ساخت k_obj_type *type, ثابت خالی *ptr)
{
	برائے (size_t i = 0; i < ARRAY_سائز(type->statics); i++) {
		ثابت ساخت k_obj_range *range = &type->statics[i];

		اگر (range->stride == 0U) {
			توڑو;
		}
		اگر (range_شامل_ہے(range, ptr)) {
			واپس true;
		}
	}

	واپس false;
}

/* Object core of the range element */
ساکن ساخت k_obj_core *range_core(ثابت ساخت k_obj_type *type,
				     ثابت ساخت k_obj_range *range, ثابت خالی *elem)
{
	ثابت uint8_t *obj = range->بالواسطہ ? *(ثابت uint8_t *ثابت *)elem : elem;

	واپس (ساخت k_obj_core *)(obj + type->obj_core_انحراف);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
ساکن bool اندر_ڈھیر_storage(ثابت خالی *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ثابت ساخت k_thread *thread = _current;
	uintptr_t irq_ڈھیر = (uintptr_t)K_KERNEL_ڈھیر_بفر(
		z_مداخلت_stacks[_current_cpu->id]);
	uintptr_t irq_ڈھیر_انتہا =
		irq_ڈھیر + K_KERNEL_ڈھیر_SIZEOF(z_مداخلت_stacks[0]);

	اگر ((thread != تہی) && (thread->ڈھیر_info.سائز != 0U) &&
	    (addr >= thread->ڈھیر_info.آغاز) &&
	    (addr < thread->ڈھیر_info.آغاز + thread->ڈھیر_info.سائز)) {
		واپس true;
	}

	واپس (addr >= irq_ڈھیر) && (addr < irq_ڈھیر_انتہا);
}

ساکن ساخت obj_core_خانہ *خانہ_ڈھونڈو(ثابت ساخت k_obj_core *core)
{
	برائے (size_t i = 0; i < ARRAY_سائز(registry); i++) {
		اگر (خانہ_core(&registry[i]) == core) {
			واپس &registry[i];
		}
	}

	واپس تہی;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
ساکن bool خانہ_stale(ثابت ساخت obj_core_خانہ *خانہ, ثابت ساخت k_obj_core *core)
{
	واپس core->type != خانہ->type;
}

ساکن ساخت obj_core_خانہ *خانہ_alloc(خالی)
{
	ساخت obj_core_خانہ *خانہ = خانہ_ڈھونڈو(تہی);

	اگر (خانہ != تہی) {
		واپس خانہ;
	}

	/* Full: reap every stale entry and reuse one of them */

	برائے (size_t i = 0; i < ARRAY_سائز(registry); i++) {
		ساخت k_obj_core *core = خانہ_core(&registry[i]);

		اگر ((core != تہی) && خانہ_stale(&registry[i], core)) {
			atomic_ptr_سیٹ(&registry[i].core, تہی);
			خانہ = &registry[i];
		}
	}

	واپس خانہ;
}

ساخت k_obj_type *z_obj_type_init(ساخت k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->گرہ);
	type->id = id;
	type->obj_core_انحراف = off;
	memset(type->statics, 0, حجم(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	واپس type;
}

خالی k_obj_type_init_range(ساخت k_obj_type *type, ثابت خالی *آغاز,
			   ثابت خالی *انتہا, size_t stride, bool بالواسطہ)
{
	type->statics[0].آغاز = آغاز;
	type->statics[0].انتہا = انتہا;
	type->statics[0].stride = stride;
	type->statics[0].بالواسطہ = بالواسطہ;
}

خالی k_obj_core_init(ساخت k_obj_core *obj_core, ساخت k_obj_type *type)
{
	obj_core->type = type;
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS
	obj_core->stats = تہی;
#اگر_ختم /* CONFIG_OBJ_CORE_STATS */
}

خالی k_obj_core_link(ساخت k_obj_core *obj_core)
{
	ساخت k_obj_type *type = obj_core->type;
	ساخت obj_core_خانہ *خانہ;

	اگر (type_ranges_contain(type, (ثابت uint8_t *)obj_core - type->obj_core_انحراف)) {
		واپس;
	}

	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);

	اگر (اندر_ڈھیر_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_تالا, کلید);
		واپس;
	}

	خانہ = خانہ_ڈھونڈو(obj_core);
	اگر (خانہ == تہی) {
		خانہ = خانہ_alloc();
	}

	اگر (خانہ == تہی) {
		type->dropped++;
	} ورنہ {
		خانہ->type = type;
		atomic_ptr_سیٹ(&خانہ->core, obj_core);
	}

	k_spin_unlock(&obj_core_تالا, کلید);
}

خالی k_obj_core_init_and_link(ساخت k_obj_core *obj_core,
			      ساخت k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

خالی k_obj_core_unlink(ساخت k_obj_core *obj_core)
{
	برائے (size_t i = 0; i < ARRAY_سائز(registry); i++) {
		اگر (atomic_ptr_cas(&registry[i].core, obj_core, تہی)) {
			توڑو;
		}
	}
}

خالی k_obj_core_evict_range(ثابت خالی *addr, size_t len)
{
	ثابت uint8_t *آغاز = addr;
	ثابت uint8_t *انتہا = آغاز + len;

	برائے (size_t i = 0; i < ARRAY_سائز(registry); i++) {
		ساخت k_obj_core *core = خانہ_core(&registry[i]);

		اگر ((core != تہی) && ((ثابت uint8_t *)core >= آغاز) &&
		    ((ثابت uint8_t *)core < انتہا)) {
			(خالی)atomic_ptr_cas(&registry[i].core, core, تہی);
		}
	}
}

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_EVICT_ON_FREE
خالی sys_ڈھیر_release_hook(خالی *mem, size_t بائٹیں)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	اگر (k_is_user_context()) {
		واپس;
	}

	k_obj_core_evict_range(mem, بائٹیں);
}
#اگر_ختم /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
ساکن خالی obj_core_init_all(خالی)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->گرہ);

		برائے (size_t r = 0; r < ARRAY_سائز(type->statics); r++) {
			ثابت ساخت k_obj_range *range = &type->statics[r];

			اگر (range->stride == 0U) {
				توڑو;
			}

			برائے (ثابت uint8_t *elem = range->آغاز;
			     elem < (ثابت uint8_t *)range->انتہا; elem += range->stride) {
				ساخت k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS
				اگر ((type->stats_desc != تہی) && (type->stats_سائز != 0)) {
					k_obj_core_stats_رجسٹر(
						obj_core,
						(خالی *)((uint8_t *)obj_core -
							 type->obj_core_انحراف +
							 type->stats_انحراف),
						type->stats_سائز);
				}
#اگر_ختم /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

ساخت k_obj_type *k_obj_type_ڈھونڈو(uint32_t type_id)
{
	ساخت k_obj_type *rv = تہی;
	sys_snode_t *گرہ;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		اگر (type->id == type_id) {
			واپس type;
		}
	}

	k_spinlock_کلید_t  کلید = k_spin_تالا(&obj_core_تالا);

	SYS_SLIST_FOR_EACH_گرہ(&z_obj_type_list, گرہ) {
		ساخت k_obj_type *type = CONTAINER_کا(گرہ, ساخت k_obj_type, گرہ);

		اگر (type->id == type_id) {
			rv = type;
			توڑو;
		}
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
ساکن صحیح walk_statics(ساخت k_obj_type *type,
			صحیح (*func)(ساخت k_obj_core *obj_core, خالی *ڈیٹا),
			خالی *ڈیٹا)
{
	صحیح حالت_2 = 0;

	برائے (size_t r = 0; (حالت_2 == 0) && (r < ARRAY_سائز(type->statics)); r++) {
		ثابت ساخت k_obj_range *range = &type->statics[r];

		اگر (range->stride == 0U) {
			توڑو;
		}

		برائے (ثابت uint8_t *elem = range->آغاز; elem < (ثابت uint8_t *)range->انتہا;
		     elem += range->stride) {
			ساخت k_obj_core *obj_core = range_core(type, range, elem);

			اگر (obj_core->type != type) {
				جاری;
			}

			حالت_2 = func(obj_core, ڈیٹا);
			اگر (حالت_2 != 0) {
				توڑو;
			}
		}
	}

	واپس حالت_2;
}

صحیح k_obj_type_walk_locked(ساخت k_obj_type *type,
			   صحیح (*func)(ساخت k_obj_core *obj_core, خالی *ڈیٹا),
			   خالی *ڈیٹا)
{
	صحیح  حالت_2;

	حالت_2 = walk_statics(type, func, ڈیٹا);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	برائے (size_t i = 0; (حالت_2 == 0) && (i < ARRAY_سائز(registry)); i++) {
		ساخت obj_core_خانہ *خانہ = &registry[i];
		k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);
		ساخت k_obj_core *core = خانہ_core(خانہ);

		اگر ((core == تہی) || (خانہ->type != type)) {
			k_spin_unlock(&obj_core_تالا, کلید);
			جاری;
		}

		اگر (خانہ_stale(خانہ, core)) {
			atomic_ptr_سیٹ(&خانہ->core, تہی);
			k_spin_unlock(&obj_core_تالا, کلید);
			جاری;
		}

		k_spin_unlock(&obj_core_تالا, کلید);

		حالت_2 = func(core, ڈیٹا);
	}

	واپس حالت_2;
}

صحیح k_obj_type_walk_unlocked(ساخت k_obj_type *type,
			   صحیح (*func)(ساخت k_obj_core *obj_core, خالی *ڈیٹا),
			   خالی *ڈیٹا)
{
	صحیح  حالت_2;

	حالت_2 = walk_statics(type, func, ڈیٹا);

	برائے (size_t i = 0; (حالت_2 == 0) && (i < ARRAY_سائز(registry)); i++) {
		ساخت k_obj_core *core = خانہ_core(&registry[i]);

		اگر ((core == تہی) || (registry[i].type != type) ||
		    (core->type != type)) {
			جاری;
		}

		حالت_2 = func(core, ڈیٹا);
	}

	واپس حالت_2;
}

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS
صحیح k_obj_core_stats_رجسٹر(ساخت k_obj_core *obj_core, خالی *stats,
			      size_t stats_len)
{
	صحیح rv;
	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);

	اگر (obj_core->type->stats_desc == تہی) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} ورنہ اگر (obj_core->type->stats_desc->raw_سائز != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} ورنہ {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}

صحیح k_obj_core_stats_deregister(ساخت k_obj_core *obj_core)
{
	صحیح rv;
	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);

	اگر (obj_core->type->stats_desc == تہی) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} ورنہ {
		obj_core->stats = تہی;
		rv = 0;
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}

صحیح k_obj_core_stats_raw(ساخت k_obj_core *obj_core, خالی *stats,
			 size_t stats_len)
{
	صحیح rv;
	ساخت k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);

	desc = obj_core->type->stats_desc;
	اگر ((desc == تہی) || (desc->raw == تہی)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ورنہ اگر ((desc->raw_سائز != stats_len) || (obj_core->stats == تہی)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ورنہ {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}

صحیح k_obj_core_stats_query(ساخت k_obj_core *obj_core, خالی *stats,
			   size_t stats_len)
{
	صحیح rv;
	ساخت k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);

	desc = obj_core->type->stats_desc;
	اگر ((desc == تہی) || (desc->query == تہی)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ورنہ اگر ((desc->query_سائز != stats_len) || (obj_core->stats == تہی)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ورنہ {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}

صحیح k_obj_core_stats_reset(ساخت k_obj_core *obj_core)
{
	صحیح rv;
	ساخت k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t  کلید = k_spin_تالا(&obj_core_تالا);

	desc = obj_core->type->stats_desc;
	اگر ((desc == تہی) || (desc->reset == تہی)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ورنہ اگر (obj_core->stats == تہی) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ورنہ {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}

صحیح k_obj_core_stats_disable(ساخت k_obj_core *obj_core)
{
	صحیح rv;
	ساخت k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);

	desc = obj_core->type->stats_desc;
	اگر ((desc == تہی) || (desc->disable == تہی)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ورنہ اگر (obj_core->stats == تہی) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ورنہ {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}

صحیح k_obj_core_stats_enable(ساخت k_obj_core *obj_core)
{
	صحیح rv;
	ساخت k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_core_تالا);

	desc = obj_core->type->stats_desc;
	اگر ((desc == تہی) || (desc->enable == تہی)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ورنہ اگر (obj_core->stats == تہی) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ورنہ {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_تالا, کلید);

	واپس rv;
}
#اگر_ختم /* CONFIG_OBJ_CORE_STATS */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_SYSTEM
ساکن ساخت k_obj_type obj_type_kernel;

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
ساکن ساخت k_obj_core_stats_desc kernel_stats_desc = {
	.raw_سائز = حجم(ساخت k_چکر_stats) * CONFIG_MP_MAX_عدد_CPUS,
	.query_سائز = حجم(ساخت k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = تہی,
	.disable = تہی,
	.enable  = تہی,
};
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
ساکن خالی init_kernel_obj_core_list(خالی)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(ساخت z_kernel, obj_core));

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(&_kernel), _kernel.usage,
				  حجم(_kernel.usage));
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#اگر_ختم /* CONFIG_OBJ_CORE_SYSTEM */
