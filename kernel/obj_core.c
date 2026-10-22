/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/obj_core.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>

ಸ್ಥಿರ ರಚನೆ k_spinlock  obj_core_ಬೀಗ;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
ರಚನೆ obj_core_ಸ್ಥಾನ {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	ರಚನೆ k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
ಸ್ಥಿರ ರಚನೆ obj_core_ಸ್ಥಾನ registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

ಸ್ಥಿರ ರಚನೆ k_obj_core *ಸ್ಥಾನ_core(ಸ್ಥಿರಾಂಕ ರಚನೆ obj_core_ಸ್ಥಾನ *ಸ್ಥಾನ)
{
	ಹಿಂತಿರುಗಿ atomic_ptr_get(&ಸ್ಥಾನ->core);
}

ಸ್ಥಿರ bool range_ಒಳಗೊಂಡಿದೆ(ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_range *range, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr)
{
	ಆದರೆ (range->ಪರೋಕ್ಷ) {
		ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸ್ಥಿರಾಂಕ *pp = range->ಆರಂಭ; pp < (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸ್ಥಿರಾಂಕ *)range->ಕೊನೆ;
		     pp = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸ್ಥಿರಾಂಕ *)((ಸ್ಥಿರಾಂಕ uint8_t *)pp + range->stride)) {
			ಆದರೆ (*pp == ptr) {
				ಹಿಂತಿರುಗಿ true;
			}
		}
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ (ptr >= range->ಆರಂಭ) && (ptr < range->ಕೊನೆ);
}

ಸ್ಥಿರ bool type_ranges_contain(ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_type *type, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr)
{
	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(type->statics); i++) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_range *range = &type->statics[i];

		ಆದರೆ (range->stride == 0U) {
			ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (range_ಒಳಗೊಂಡಿದೆ(range, ptr)) {
			ಹಿಂತಿರುಗಿ true;
		}
	}

	ಹಿಂತಿರುಗಿ false;
}

/* Object core of the range element */
ಸ್ಥಿರ ರಚನೆ k_obj_core *range_core(ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_type *type,
				     ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_range *range, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *elem)
{
	ಸ್ಥಿರಾಂಕ uint8_t *obj = range->ಪರೋಕ್ಷ ? *(ಸ್ಥಿರಾಂಕ uint8_t *ಸ್ಥಿರಾಂಕ *)elem : elem;

	ಹಿಂತಿರುಗಿ (ರಚನೆ k_obj_core *)(obj + type->obj_core_ಸ್ಥಾನಾಂತರ);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
ಸ್ಥಿರ bool ಒಳ_ರಾಶಿ_storage(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread = _current;
	uintptr_t irq_ರಾಶಿ = (uintptr_t)K_KERNEL_ರಾಶಿ_ಬಫರ್(
		z_ಅಡಚಣೆ_stacks[_current_cpu->id]);
	uintptr_t irq_ರಾಶಿ_ಕೊನೆ =
		irq_ರಾಶಿ + K_KERNEL_ರಾಶಿ_SIZEOF(z_ಅಡಚಣೆ_stacks[0]);

	ಆದರೆ ((thread != ಖಾಲಿ) && (thread->ರಾಶಿ_info.ಗಾತ್ರ_2 != 0U) &&
	    (addr >= thread->ರಾಶಿ_info.ಆರಂಭ) &&
	    (addr < thread->ರಾಶಿ_info.ಆರಂಭ + thread->ರಾಶಿ_info.ಗಾತ್ರ_2)) {
		ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ (addr >= irq_ರಾಶಿ) && (addr < irq_ರಾಶಿ_ಕೊನೆ);
}

ಸ್ಥಿರ ರಚನೆ obj_core_ಸ್ಥಾನ *ಸ್ಥಾನ_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_core *core)
{
	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(registry); i++) {
		ಆದರೆ (ಸ್ಥಾನ_core(&registry[i]) == core) {
			ಹಿಂತಿರುಗಿ &registry[i];
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
ಸ್ಥಿರ bool ಸ್ಥಾನ_stale(ಸ್ಥಿರಾಂಕ ರಚನೆ obj_core_ಸ್ಥಾನ *ಸ್ಥಾನ, ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_core *core)
{
	ಹಿಂತಿರುಗಿ core->type != ಸ್ಥಾನ->type;
}

ಸ್ಥಿರ ರಚನೆ obj_core_ಸ್ಥಾನ *ಸ್ಥಾನ_alloc(ಶೂನ್ಯ)
{
	ರಚನೆ obj_core_ಸ್ಥಾನ *ಸ್ಥಾನ = ಸ್ಥಾನ_ಕಂಡುಹಿಡಿ(ಖಾಲಿ);

	ಆದರೆ (ಸ್ಥಾನ != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಸ್ಥಾನ;
	}

	/* Full: reap every stale entry and reuse one of them */

	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(registry); i++) {
		ರಚನೆ k_obj_core *core = ಸ್ಥಾನ_core(&registry[i]);

		ಆದರೆ ((core != ಖಾಲಿ) && ಸ್ಥಾನ_stale(&registry[i], core)) {
			atomic_ptr_ಗಣ(&registry[i].core, ಖಾಲಿ);
			ಸ್ಥಾನ = &registry[i];
		}
	}

	ಹಿಂತಿರುಗಿ ಸ್ಥಾನ;
}

ರಚನೆ k_obj_type *z_obj_type_init(ರಚನೆ k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->ಗಂಟು);
	type->id = id;
	type->obj_core_ಸ್ಥಾನಾಂತರ = off;
	memset(type->statics, 0, ಗಾತ್ರ(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	ಹಿಂತಿರುಗಿ type;
}

ಶೂನ್ಯ k_obj_type_init_range(ರಚನೆ k_obj_type *type, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಆರಂಭ,
			   ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಕೊನೆ, size_t stride, bool ಪರೋಕ್ಷ)
{
	type->statics[0].ಆರಂಭ = ಆರಂಭ;
	type->statics[0].ಕೊನೆ = ಕೊನೆ;
	type->statics[0].stride = stride;
	type->statics[0].ಪರೋಕ್ಷ = ಪರೋಕ್ಷ;
}

ಶೂನ್ಯ k_obj_core_init(ರಚನೆ k_obj_core *obj_core, ರಚನೆ k_obj_type *type)
{
	obj_core->type = type;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS
	obj_core->stats = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS */
}

ಶೂನ್ಯ k_obj_core_link(ರಚನೆ k_obj_core *obj_core)
{
	ರಚನೆ k_obj_type *type = obj_core->type;
	ರಚನೆ obj_core_ಸ್ಥಾನ *ಸ್ಥಾನ;

	ಆದರೆ (type_ranges_contain(type, (ಸ್ಥಿರಾಂಕ uint8_t *)obj_core - type->obj_core_ಸ್ಥಾನಾಂತರ)) {
		ಹಿಂತಿರುಗಿ;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	ಆದರೆ (ಒಳ_ರಾಶಿ_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	ಸ್ಥಾನ = ಸ್ಥಾನ_ಕಂಡುಹಿಡಿ(obj_core);
	ಆದರೆ (ಸ್ಥಾನ == ಖಾಲಿ) {
		ಸ್ಥಾನ = ಸ್ಥಾನ_alloc();
	}

	ಆದರೆ (ಸ್ಥಾನ == ಖಾಲಿ) {
		type->dropped++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಸ್ಥಾನ->type = type;
		atomic_ptr_ಗಣ(&ಸ್ಥಾನ->core, obj_core);
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);
}

ಶೂನ್ಯ k_obj_core_init_and_link(ರಚನೆ k_obj_core *obj_core,
			      ರಚನೆ k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

ಶೂನ್ಯ k_obj_core_unlink(ರಚನೆ k_obj_core *obj_core)
{
	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(registry); i++) {
		ಆದರೆ (atomic_ptr_cas(&registry[i].core, obj_core, ಖಾಲಿ)) {
			ನಿಲ್ಲಿಸು;
		}
	}
}

ಶೂನ್ಯ k_obj_core_evict_range(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *addr, size_t len)
{
	ಸ್ಥಿರಾಂಕ uint8_t *ಆರಂಭ = addr;
	ಸ್ಥಿರಾಂಕ uint8_t *ಕೊನೆ = ಆರಂಭ + len;

	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(registry); i++) {
		ರಚನೆ k_obj_core *core = ಸ್ಥಾನ_core(&registry[i]);

		ಆದರೆ ((core != ಖಾಲಿ) && ((ಸ್ಥಿರಾಂಕ uint8_t *)core >= ಆರಂಭ) &&
		    ((ಸ್ಥಿರಾಂಕ uint8_t *)core < ಕೊನೆ)) {
			(ಶೂನ್ಯ)atomic_ptr_cas(&registry[i].core, core, ಖಾಲಿ);
		}
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_EVICT_ON_FREE
ಶೂನ್ಯ sys_ರಾಶಿ_release_hook(ಶೂನ್ಯ *mem, size_t ಬೈಟುಗಳು)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	ಆದರೆ (k_is_user_context()) {
		ಹಿಂತಿರುಗಿ;
	}

	k_obj_core_evict_range(mem, ಬೈಟುಗಳು);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
ಸ್ಥಿರ ಶೂನ್ಯ obj_core_init_all(ಶೂನ್ಯ)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->ಗಂಟು);

		ಗಾಗಿ (size_t r = 0; r < ARRAY_ಗಾತ್ರ(type->statics); r++) {
			ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_range *range = &type->statics[r];

			ಆದರೆ (range->stride == 0U) {
				ನಿಲ್ಲಿಸು;
			}

			ಗಾಗಿ (ಸ್ಥಿರಾಂಕ uint8_t *elem = range->ಆರಂಭ;
			     elem < (ಸ್ಥಿರಾಂಕ uint8_t *)range->ಕೊನೆ; elem += range->stride) {
				ರಚನೆ k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS
				ಆದರೆ ((type->stats_desc != ಖಾಲಿ) && (type->stats_ಗಾತ್ರ != 0)) {
					k_obj_core_stats_ರಿಜಿಸ್ಟರ್(
						obj_core,
						(ಶೂನ್ಯ *)((uint8_t *)obj_core -
							 type->obj_core_ಸ್ಥಾನಾಂತರ +
							 type->stats_ಸ್ಥಾನಾಂತರ),
						type->stats_ಗಾತ್ರ);
				}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

ರಚನೆ k_obj_type *k_obj_type_ಕಂಡುಹಿಡಿ(uint32_t type_id)
{
	ರಚನೆ k_obj_type *rv = ಖಾಲಿ;
	sys_snode_t *ಗಂಟು;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		ಆದರೆ (type->id == type_id) {
			ಹಿಂತಿರುಗಿ type;
		}
	}

	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	SYS_SLIST_FOR_EACH_ಗಂಟು(&z_obj_type_list, ಗಂಟು) {
		ರಚನೆ k_obj_type *type = CONTAINER_ದ(ಗಂಟು, ರಚನೆ k_obj_type, ಗಂಟು);

		ಆದರೆ (type->id == type_id) {
			rv = type;
			ನಿಲ್ಲಿಸು;
		}
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ walk_statics(ರಚನೆ k_obj_type *type,
			ಪೂರ್ಣಾಂಕ (*func)(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *ದತ್ತಾಂಶ),
			ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = 0;

	ಗಾಗಿ (size_t r = 0; (ಸ್ಥಿತಿ_2 == 0) && (r < ARRAY_ಗಾತ್ರ(type->statics)); r++) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ k_obj_range *range = &type->statics[r];

		ಆದರೆ (range->stride == 0U) {
			ನಿಲ್ಲಿಸು;
		}

		ಗಾಗಿ (ಸ್ಥಿರಾಂಕ uint8_t *elem = range->ಆರಂಭ; elem < (ಸ್ಥಿರಾಂಕ uint8_t *)range->ಕೊನೆ;
		     elem += range->stride) {
			ರಚನೆ k_obj_core *obj_core = range_core(type, range, elem);

			ಆದರೆ (obj_core->type != type) {
				ಮುಂದುವರಿ;
			}

			ಸ್ಥಿತಿ_2 = func(obj_core, ದತ್ತಾಂಶ);
			ಆದರೆ (ಸ್ಥಿತಿ_2 != 0) {
				ನಿಲ್ಲಿಸು;
			}
		}
	}

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

ಪೂರ್ಣಾಂಕ k_obj_type_walk_locked(ರಚನೆ k_obj_type *type,
			   ಪೂರ್ಣಾಂಕ (*func)(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *ದತ್ತಾಂಶ),
			   ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ  ಸ್ಥಿತಿ_2;

	ಸ್ಥಿತಿ_2 = walk_statics(type, func, ದತ್ತಾಂಶ);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	ಗಾಗಿ (size_t i = 0; (ಸ್ಥಿತಿ_2 == 0) && (i < ARRAY_ಗಾತ್ರ(registry)); i++) {
		ರಚನೆ obj_core_ಸ್ಥಾನ *ಸ್ಥಾನ = &registry[i];
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);
		ರಚನೆ k_obj_core *core = ಸ್ಥಾನ_core(ಸ್ಥಾನ);

		ಆದರೆ ((core == ಖಾಲಿ) || (ಸ್ಥಾನ->type != type)) {
			k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ಸ್ಥಾನ_stale(ಸ್ಥಾನ, core)) {
			atomic_ptr_ಗಣ(&ಸ್ಥಾನ->core, ಖಾಲಿ);
			k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);
			ಮುಂದುವರಿ;
		}

		k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

		ಸ್ಥಿತಿ_2 = func(core, ದತ್ತಾಂಶ);
	}

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

ಪೂರ್ಣಾಂಕ k_obj_type_walk_unlocked(ರಚನೆ k_obj_type *type,
			   ಪೂರ್ಣಾಂಕ (*func)(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *ದತ್ತಾಂಶ),
			   ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ  ಸ್ಥಿತಿ_2;

	ಸ್ಥಿತಿ_2 = walk_statics(type, func, ದತ್ತಾಂಶ);

	ಗಾಗಿ (size_t i = 0; (ಸ್ಥಿತಿ_2 == 0) && (i < ARRAY_ಗಾತ್ರ(registry)); i++) {
		ರಚನೆ k_obj_core *core = ಸ್ಥಾನ_core(&registry[i]);

		ಆದರೆ ((core == ಖಾಲಿ) || (registry[i].type != type) ||
		    (core->type != type)) {
			ಮುಂದುವರಿ;
		}

		ಸ್ಥಿತಿ_2 = func(core, ದತ್ತಾಂಶ);
	}

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS
ಪೂರ್ಣಾಂಕ k_obj_core_stats_ರಿಜಿಸ್ಟರ್(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats,
			      size_t stats_len)
{
	ಪೂರ್ಣಾಂಕ rv;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	ಆದರೆ (obj_core->type->stats_desc == ಖಾಲಿ) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (obj_core->type->stats_desc->raw_ಗಾತ್ರ != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}

ಪೂರ್ಣಾಂಕ k_obj_core_stats_deregister(ರಚನೆ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಂಕ rv;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	ಆದರೆ (obj_core->type->stats_desc == ಖಾಲಿ) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} ಇಲ್ಲದಿದ್ದರೆ {
		obj_core->stats = ಖಾಲಿ;
		rv = 0;
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}

ಪೂರ್ಣಾಂಕ k_obj_core_stats_raw(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats,
			 size_t stats_len)
{
	ಪೂರ್ಣಾಂಕ rv;
	ರಚನೆ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	desc = obj_core->type->stats_desc;
	ಆದರೆ ((desc == ಖಾಲಿ) || (desc->raw == ಖಾಲಿ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((desc->raw_ಗಾತ್ರ != stats_len) || (obj_core->stats == ಖಾಲಿ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}

ಪೂರ್ಣಾಂಕ k_obj_core_stats_query(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats,
			   size_t stats_len)
{
	ಪೂರ್ಣಾಂಕ rv;
	ರಚನೆ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	desc = obj_core->type->stats_desc;
	ಆದರೆ ((desc == ಖಾಲಿ) || (desc->query == ಖಾಲಿ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((desc->query_ಗಾತ್ರ != stats_len) || (obj_core->stats == ಖಾಲಿ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}

ಪೂರ್ಣಾಂಕ k_obj_core_stats_reset(ರಚನೆ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಂಕ rv;
	ರಚನೆ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	desc = obj_core->type->stats_desc;
	ಆದರೆ ((desc == ಖಾಲಿ) || (desc->reset == ಖಾಲಿ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (obj_core->stats == ಖಾಲಿ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}

ಪೂರ್ಣಾಂಕ k_obj_core_stats_disable(ರಚನೆ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಂಕ rv;
	ರಚನೆ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	desc = obj_core->type->stats_desc;
	ಆದರೆ ((desc == ಖಾಲಿ) || (desc->disable == ಖಾಲಿ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (obj_core->stats == ಖಾಲಿ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}

ಪೂರ್ಣಾಂಕ k_obj_core_stats_enable(ರಚನೆ k_obj_core *obj_core)
{
	ಪೂರ್ಣಾಂಕ rv;
	ರಚನೆ k_obj_core_stats_desc *desc;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_core_ಬೀಗ);

	desc = obj_core->type->stats_desc;
	ಆದರೆ ((desc == ಖಾಲಿ) || (desc->enable == ಖಾಲಿ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (obj_core->stats == ಖಾಲಿ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ rv;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_SYSTEM
ಸ್ಥಿರ ರಚನೆ k_obj_type obj_type_kernel;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_SYSTEM
ಸ್ಥಿರ ರಚನೆ k_obj_core_stats_desc kernel_stats_desc = {
	.raw_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ k_ಚಕ್ರ_stats) * CONFIG_MP_MAX_ಅಂಕೆ_CPUS,
	.query_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ಖಾಲಿ,
	.disable = ಖಾಲಿ,
	.enable  = ಖಾಲಿ,
};
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ init_kernel_obj_core_list(ಶೂನ್ಯ)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(ರಚನೆ z_kernel, obj_core));

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_ರಿಜಿಸ್ಟರ್(K_OBJ_CORE(&_kernel), _kernel.usage,
				  ಗಾತ್ರ(_kernel.usage));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_SYSTEM */
