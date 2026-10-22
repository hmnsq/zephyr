/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <string.h>
#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/obj_core.h>
#ضمن <kernel_internal.h>

ساكن بنية k_spinlock  obj_core_جندرا;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
بنية obj_core_تھاں {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	بنية k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
ساكن بنية obj_core_تھاں registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

ساكن بنية k_obj_core *تھاں_core(ثابت بنية obj_core_تھاں *تھاں)
{
	أرجع atomic_ptr_get(&تھاں->core);
}

ساكن bool range_شامل_اے(ثابت بنية k_obj_range *range, ثابت فراغ *ptr)
{
	إذا (range->اسدھا) {
		لكل (ثابت فراغ *ثابت *pp = range->شروع; pp < (ثابت فراغ *ثابت *)range->سرا;
		     pp = (ثابت فراغ *ثابت *)((ثابت uint8_t *)pp + range->stride)) {
			إذا (*pp == ptr) {
				أرجع true;
			}
		}
		أرجع false;
	}

	أرجع (ptr >= range->شروع) && (ptr < range->سرا);
}

ساكن bool type_ranges_contain(ثابت بنية k_obj_type *type, ثابت فراغ *ptr)
{
	لكل (size_t i = 0; i < ARRAY_ناپ(type->statics); i++) {
		ثابت بنية k_obj_range *range = &type->statics[i];

		إذا (range->stride == 0U) {
			اكسر;
		}
		إذا (range_شامل_اے(range, ptr)) {
			أرجع true;
		}
	}

	أرجع false;
}

/* Object core of the range element */
ساكن بنية k_obj_core *range_core(ثابت بنية k_obj_type *type,
				     ثابت بنية k_obj_range *range, ثابت فراغ *elem)
{
	ثابت uint8_t *obj = range->اسدھا ? *(ثابت uint8_t *ثابت *)elem : elem;

	أرجع (بنية k_obj_core *)(obj + type->obj_core_وتھ);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
ساكن bool وچ_ڈھیر_storage(ثابت فراغ *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ثابت بنية k_thread *thread = _current;
	uintptr_t irq_ڈھیر = (uintptr_t)K_KERNEL_ڈھیر_بفر(
		z_رکاوٹ_stacks[_current_cpu->id]);
	uintptr_t irq_ڈھیر_سرا =
		irq_ڈھیر + K_KERNEL_ڈھیر_SIZEOF(z_رکاوٹ_stacks[0]);

	إذا ((thread != عدم) && (thread->ڈھیر_info.ناپ != 0U) &&
	    (addr >= thread->ڈھیر_info.شروع) &&
	    (addr < thread->ڈھیر_info.شروع + thread->ڈھیر_info.ناپ)) {
		أرجع true;
	}

	أرجع (addr >= irq_ڈھیر) && (addr < irq_ڈھیر_سرا);
}

ساكن بنية obj_core_تھاں *تھاں_لبھو(ثابت بنية k_obj_core *core)
{
	لكل (size_t i = 0; i < ARRAY_ناپ(registry); i++) {
		إذا (تھاں_core(&registry[i]) == core) {
			أرجع &registry[i];
		}
	}

	أرجع عدم;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
ساكن bool تھاں_stale(ثابت بنية obj_core_تھاں *تھاں, ثابت بنية k_obj_core *core)
{
	أرجع core->type != تھاں->type;
}

ساكن بنية obj_core_تھاں *تھاں_alloc(فراغ)
{
	بنية obj_core_تھاں *تھاں = تھاں_لبھو(عدم);

	إذا (تھاں != عدم) {
		أرجع تھاں;
	}

	/* Full: reap every stale entry and reuse one of them */

	لكل (size_t i = 0; i < ARRAY_ناپ(registry); i++) {
		بنية k_obj_core *core = تھاں_core(&registry[i]);

		إذا ((core != عدم) && تھاں_stale(&registry[i], core)) {
			atomic_ptr_سیٹ(&registry[i].core, عدم);
			تھاں = &registry[i];
		}
	}

	أرجع تھاں;
}

بنية k_obj_type *z_obj_type_init(بنية k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->گرہ);
	type->id = id;
	type->obj_core_وتھ = off;
	memset(type->statics, 0, حجم(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	أرجع type;
}

فراغ k_obj_type_init_range(بنية k_obj_type *type, ثابت فراغ *شروع,
			   ثابت فراغ *سرا, size_t stride, bool اسدھا)
{
	type->statics[0].شروع = شروع;
	type->statics[0].سرا = سرا;
	type->statics[0].stride = stride;
	type->statics[0].اسدھا = اسدھا;
}

فراغ k_obj_core_init(بنية k_obj_core *obj_core, بنية k_obj_type *type)
{
	obj_core->type = type;
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS
	obj_core->stats = عدم;
#جے_انت /* CONFIG_OBJ_CORE_STATS */
}

فراغ k_obj_core_link(بنية k_obj_core *obj_core)
{
	بنية k_obj_type *type = obj_core->type;
	بنية obj_core_تھاں *تھاں;

	إذا (type_ranges_contain(type, (ثابت uint8_t *)obj_core - type->obj_core_وتھ)) {
		أرجع;
	}

	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);

	إذا (وچ_ڈھیر_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_جندرا, کلید);
		أرجع;
	}

	تھاں = تھاں_لبھو(obj_core);
	إذا (تھاں == عدم) {
		تھاں = تھاں_alloc();
	}

	إذا (تھاں == عدم) {
		type->dropped++;
	} وإلا {
		تھاں->type = type;
		atomic_ptr_سیٹ(&تھاں->core, obj_core);
	}

	k_spin_unlock(&obj_core_جندرا, کلید);
}

فراغ k_obj_core_init_and_link(بنية k_obj_core *obj_core,
			      بنية k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

فراغ k_obj_core_unlink(بنية k_obj_core *obj_core)
{
	لكل (size_t i = 0; i < ARRAY_ناپ(registry); i++) {
		إذا (atomic_ptr_cas(&registry[i].core, obj_core, عدم)) {
			اكسر;
		}
	}
}

فراغ k_obj_core_evict_range(ثابت فراغ *addr, size_t len)
{
	ثابت uint8_t *شروع = addr;
	ثابت uint8_t *سرا = شروع + len;

	لكل (size_t i = 0; i < ARRAY_ناپ(registry); i++) {
		بنية k_obj_core *core = تھاں_core(&registry[i]);

		إذا ((core != عدم) && ((ثابت uint8_t *)core >= شروع) &&
		    ((ثابت uint8_t *)core < سرا)) {
			(فراغ)atomic_ptr_cas(&registry[i].core, core, عدم);
		}
	}
}

#جے_تعریف_شدہ CONFIG_OBJ_CORE_EVICT_ON_FREE
فراغ sys_ڈھیر_release_hook(فراغ *mem, size_t بائٹاں)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	إذا (k_is_user_context()) {
		أرجع;
	}

	k_obj_core_evict_range(mem, بائٹاں);
}
#جے_انت /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
ساكن فراغ obj_core_init_all(فراغ)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->گرہ);

		لكل (size_t r = 0; r < ARRAY_ناپ(type->statics); r++) {
			ثابت بنية k_obj_range *range = &type->statics[r];

			إذا (range->stride == 0U) {
				اكسر;
			}

			لكل (ثابت uint8_t *elem = range->شروع;
			     elem < (ثابت uint8_t *)range->سرا; elem += range->stride) {
				بنية k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS
				إذا ((type->stats_desc != عدم) && (type->stats_ناپ != 0)) {
					k_obj_core_stats_رجسٹر(
						obj_core,
						(فراغ *)((uint8_t *)obj_core -
							 type->obj_core_وتھ +
							 type->stats_وتھ),
						type->stats_ناپ);
				}
#جے_انت /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

بنية k_obj_type *k_obj_type_لبھو(uint32_t type_id)
{
	بنية k_obj_type *rv = عدم;
	sys_snode_t *گرہ;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		إذا (type->id == type_id) {
			أرجع type;
		}
	}

	k_spinlock_کلید_t  کلید = k_spin_جندرا(&obj_core_جندرا);

	SYS_SLIST_FOR_EACH_گرہ(&z_obj_type_list, گرہ) {
		بنية k_obj_type *type = CONTAINER_دا(گرہ, بنية k_obj_type, گرہ);

		إذا (type->id == type_id) {
			rv = type;
			اكسر;
		}
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
ساكن صحيح walk_statics(بنية k_obj_type *type,
			صحيح (*func)(بنية k_obj_core *obj_core, فراغ *انکڑے),
			فراغ *انکڑے)
{
	صحيح حالت = 0;

	لكل (size_t r = 0; (حالت == 0) && (r < ARRAY_ناپ(type->statics)); r++) {
		ثابت بنية k_obj_range *range = &type->statics[r];

		إذا (range->stride == 0U) {
			اكسر;
		}

		لكل (ثابت uint8_t *elem = range->شروع; elem < (ثابت uint8_t *)range->سرا;
		     elem += range->stride) {
			بنية k_obj_core *obj_core = range_core(type, range, elem);

			إذا (obj_core->type != type) {
				استمر;
			}

			حالت = func(obj_core, انکڑے);
			إذا (حالت != 0) {
				اكسر;
			}
		}
	}

	أرجع حالت;
}

صحيح k_obj_type_walk_locked(بنية k_obj_type *type,
			   صحيح (*func)(بنية k_obj_core *obj_core, فراغ *انکڑے),
			   فراغ *انکڑے)
{
	صحيح  حالت;

	حالت = walk_statics(type, func, انکڑے);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	لكل (size_t i = 0; (حالت == 0) && (i < ARRAY_ناپ(registry)); i++) {
		بنية obj_core_تھاں *تھاں = &registry[i];
		k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);
		بنية k_obj_core *core = تھاں_core(تھاں);

		إذا ((core == عدم) || (تھاں->type != type)) {
			k_spin_unlock(&obj_core_جندرا, کلید);
			استمر;
		}

		إذا (تھاں_stale(تھاں, core)) {
			atomic_ptr_سیٹ(&تھاں->core, عدم);
			k_spin_unlock(&obj_core_جندرا, کلید);
			استمر;
		}

		k_spin_unlock(&obj_core_جندرا, کلید);

		حالت = func(core, انکڑے);
	}

	أرجع حالت;
}

صحيح k_obj_type_walk_unlocked(بنية k_obj_type *type,
			   صحيح (*func)(بنية k_obj_core *obj_core, فراغ *انکڑے),
			   فراغ *انکڑے)
{
	صحيح  حالت;

	حالت = walk_statics(type, func, انکڑے);

	لكل (size_t i = 0; (حالت == 0) && (i < ARRAY_ناپ(registry)); i++) {
		بنية k_obj_core *core = تھاں_core(&registry[i]);

		إذا ((core == عدم) || (registry[i].type != type) ||
		    (core->type != type)) {
			استمر;
		}

		حالت = func(core, انکڑے);
	}

	أرجع حالت;
}

#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS
صحيح k_obj_core_stats_رجسٹر(بنية k_obj_core *obj_core, فراغ *stats,
			      size_t stats_len)
{
	صحيح rv;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);

	إذا (obj_core->type->stats_desc == عدم) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} وإلا إذا (obj_core->type->stats_desc->raw_ناپ != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} وإلا {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}

صحيح k_obj_core_stats_deregister(بنية k_obj_core *obj_core)
{
	صحيح rv;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);

	إذا (obj_core->type->stats_desc == عدم) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} وإلا {
		obj_core->stats = عدم;
		rv = 0;
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}

صحيح k_obj_core_stats_raw(بنية k_obj_core *obj_core, فراغ *stats,
			 size_t stats_len)
{
	صحيح rv;
	بنية k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);

	desc = obj_core->type->stats_desc;
	إذا ((desc == عدم) || (desc->raw == عدم)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وإلا إذا ((desc->raw_ناپ != stats_len) || (obj_core->stats == عدم)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} وإلا {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}

صحيح k_obj_core_stats_query(بنية k_obj_core *obj_core, فراغ *stats,
			   size_t stats_len)
{
	صحيح rv;
	بنية k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);

	desc = obj_core->type->stats_desc;
	إذا ((desc == عدم) || (desc->query == عدم)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وإلا إذا ((desc->query_ناپ != stats_len) || (obj_core->stats == عدم)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} وإلا {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}

صحيح k_obj_core_stats_reset(بنية k_obj_core *obj_core)
{
	صحيح rv;
	بنية k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t  کلید = k_spin_جندرا(&obj_core_جندرا);

	desc = obj_core->type->stats_desc;
	إذا ((desc == عدم) || (desc->reset == عدم)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وإلا إذا (obj_core->stats == عدم) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} وإلا {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}

صحيح k_obj_core_stats_disable(بنية k_obj_core *obj_core)
{
	صحيح rv;
	بنية k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);

	desc = obj_core->type->stats_desc;
	إذا ((desc == عدم) || (desc->disable == عدم)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وإلا إذا (obj_core->stats == عدم) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} وإلا {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}

صحيح k_obj_core_stats_enable(بنية k_obj_core *obj_core)
{
	صحيح rv;
	بنية k_obj_core_stats_desc *desc;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_core_جندرا);

	desc = obj_core->type->stats_desc;
	إذا ((desc == عدم) || (desc->enable == عدم)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} وإلا إذا (obj_core->stats == عدم) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} وإلا {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_جندرا, کلید);

	أرجع rv;
}
#جے_انت /* CONFIG_OBJ_CORE_STATS */

#جے_تعریف_شدہ CONFIG_OBJ_CORE_SYSTEM
ساكن بنية k_obj_type obj_type_kernel;

#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
ساكن بنية k_obj_core_stats_desc kernel_stats_desc = {
	.raw_ناپ = حجم(بنية k_چکر_stats) * CONFIG_MP_MAX_عدد_CPUS,
	.query_ناپ = حجم(بنية k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = عدم,
	.disable = عدم,
	.enable  = عدم,
};
#جے_انت /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
ساكن فراغ init_kernel_obj_core_list(فراغ)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(بنية z_kernel, obj_core));

#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#جے_انت /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(&_kernel), _kernel.usage,
				  حجم(_kernel.usage));
#جے_انت /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#جے_انت /* CONFIG_OBJ_CORE_SYSTEM */
