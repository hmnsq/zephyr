/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel/obj_core.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>

ସ୍ଥିର ସଂରଚନା k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
ସଂରଚନା obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	ସଂରଚନା k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
ସ୍ଥିର ସଂରଚନା obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

ସ୍ଥିର ସଂରଚନା k_obj_core *slot_core(ଅଚଳ ସଂରଚନା obj_core_slot *slot)
{
	ଫେରନ୍ତୁ atomic_ptr_get(&slot->core);
}

ସ୍ଥିର bool range_contains(ଅଚଳ ସଂରଚନା k_obj_range *range, ଅଚଳ ଶୂନ୍ୟ *ptr)
{
	ଯଦି (range->indirect) {
		ପାଇଁ (ଅଚଳ ଶୂନ୍ୟ *ଅଚଳ *pp = range->start; pp < (ଅଚଳ ଶୂନ୍ୟ *ଅଚଳ *)range->end;
		     pp = (ଅଚଳ ଶୂନ୍ୟ *ଅଚଳ *)((ଅଚଳ uint8_t *)pp + range->stride)) {
			ଯଦି (*pp == ptr) {
				ଫେରନ୍ତୁ true;
			}
		}
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ (ptr >= range->start) && (ptr < range->end);
}

ସ୍ଥିର bool type_ranges_contain(ଅଚଳ ସଂରଚନା k_obj_type *type, ଅଚଳ ଶୂନ୍ୟ *ptr)
{
	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		ଅଚଳ ସଂରଚନା k_obj_range *range = &type->statics[i];

		ଯଦି (range->stride == 0U) {
			ଭାଙ୍ଗ;
		}
		ଯଦି (range_contains(range, ptr)) {
			ଫେରନ୍ତୁ true;
		}
	}

	ଫେରନ୍ତୁ false;
}

/* Object core of the range element */
ସ୍ଥିର ସଂରଚନା k_obj_core *range_core(ଅଚଳ ସଂରଚନା k_obj_type *type,
				     ଅଚଳ ସଂରଚନା k_obj_range *range, ଅଚଳ ଶୂନ୍ୟ *elem)
{
	ଅଚଳ uint8_t *obj = range->indirect ? *(ଅଚଳ uint8_t *ଅଚଳ *)elem : elem;

	ଫେରନ୍ତୁ (ସଂରଚନା k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
ସ୍ଥିର bool in_ଗଦା_storage(ଅଚଳ ଶୂନ୍ୟ *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	ଅଚଳ ସଂରଚନା k_thread *thread = _current;
	uintptr_t irq_ଗଦା = (uintptr_t)K_KERNEL_ଗଦା_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_ଗଦା_end =
		irq_ଗଦା + K_KERNEL_ଗଦା_SIZEOF(z_interrupt_stacks[0]);

	ଯଦି ((thread != ଖାଲି) && (thread->ଗଦା_info.size != 0U) &&
	    (addr >= thread->ଗଦା_info.start) &&
	    (addr < thread->ଗଦା_info.start + thread->ଗଦା_info.size)) {
		ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ (addr >= irq_ଗଦା) && (addr < irq_ଗଦା_end);
}

ସ୍ଥିର ସଂରଚନା obj_core_slot *slot_find(ଅଚଳ ସଂରଚନା k_obj_core *core)
{
	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ଯଦି (slot_core(&registry[i]) == core) {
			ଫେରନ୍ତୁ &registry[i];
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
ସ୍ଥିର bool slot_stale(ଅଚଳ ସଂରଚନା obj_core_slot *slot, ଅଚଳ ସଂରଚନା k_obj_core *core)
{
	ଫେରନ୍ତୁ core->type != slot->type;
}

ସ୍ଥିର ସଂରଚନା obj_core_slot *slot_alloc(ଶୂନ୍ୟ)
{
	ସଂରଚନା obj_core_slot *slot = slot_find(ଖାଲି);

	ଯଦି (slot != ଖାଲି) {
		ଫେରନ୍ତୁ slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ସଂରଚନା k_obj_core *core = slot_core(&registry[i]);

		ଯଦି ((core != ଖାଲି) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, ଖାଲି);
			slot = &registry[i];
		}
	}

	ଫେରନ୍ତୁ slot;
}

ସଂରଚନା k_obj_type *z_obj_type_init(ସଂରଚନା k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->ଗଣ୍ଠି);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, ଆକାର(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	ଫେରନ୍ତୁ type;
}

ଶୂନ୍ୟ k_obj_type_init_range(ସଂରଚନା k_obj_type *type, ଅଚଳ ଶୂନ୍ୟ *start,
			   ଅଚଳ ଶୂନ୍ୟ *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

ଶୂନ୍ୟ k_obj_core_init(ସଂରଚନା k_obj_core *obj_core, ସଂରଚନା k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = ଖାଲି;
#endif /* CONFIG_OBJ_CORE_STATS */
}

ଶୂନ୍ୟ k_obj_core_link(ସଂରଚନା k_obj_core *obj_core)
{
	ସଂରଚନା k_obj_type *type = obj_core->type;
	ସଂରଚନା obj_core_slot *slot;

	ଯଦି (type_ranges_contain(type, (ଅଚଳ uint8_t *)obj_core - type->obj_core_offset)) {
		ଫେରନ୍ତୁ;
	}

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);

	ଯଦି (in_ଗଦା_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, ଚାବି);
		ଫେରନ୍ତୁ;
	}

	slot = slot_find(obj_core);
	ଯଦି (slot == ଖାଲି) {
		slot = slot_alloc();
	}

	ଯଦି (slot == ଖାଲି) {
		type->dropped++;
	} ଅନ୍ୟଥା {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);
}

ଶୂନ୍ୟ k_obj_core_init_and_link(ସଂରଚନା k_obj_core *obj_core,
			      ସଂରଚନା k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

ଶୂନ୍ୟ k_obj_core_unlink(ସଂରଚନା k_obj_core *obj_core)
{
	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ଯଦି (atomic_ptr_cas(&registry[i].core, obj_core, ଖାଲି)) {
			ଭାଙ୍ଗ;
		}
	}
}

ଶୂନ୍ୟ k_obj_core_evict_range(ଅଚଳ ଶୂନ୍ୟ *addr, size_t len)
{
	ଅଚଳ uint8_t *start = addr;
	ଅଚଳ uint8_t *end = start + len;

	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		ସଂରଚନା k_obj_core *core = slot_core(&registry[i]);

		ଯଦି ((core != ଖାଲି) && ((ଅଚଳ uint8_t *)core >= start) &&
		    ((ଅଚଳ uint8_t *)core < end)) {
			(ଶୂନ୍ୟ)atomic_ptr_cas(&registry[i].core, core, ଖାଲି);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
ଶୂନ୍ୟ sys_heap_release_hook(ଶୂନ୍ୟ *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	ଯଦି (k_is_user_context()) {
		ଫେରନ୍ତୁ;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
ସ୍ଥିର ଶୂନ୍ୟ obj_core_init_all(ଶୂନ୍ୟ)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->ଗଣ୍ଠି);

		ପାଇଁ (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			ଅଚଳ ସଂରଚନା k_obj_range *range = &type->statics[r];

			ଯଦି (range->stride == 0U) {
				ଭାଙ୍ଗ;
			}

			ପାଇଁ (ଅଚଳ uint8_t *elem = range->start;
			     elem < (ଅଚଳ uint8_t *)range->end; elem += range->stride) {
				ସଂରଚନା k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				ଯଦି ((type->stats_desc != ଖାଲି) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(ଶୂନ୍ୟ *)((uint8_t *)obj_core -
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

ସଂରଚନା k_obj_type *k_obj_type_find(uint32_t type_id)
{
	ସଂରଚନା k_obj_type *rv = ଖାଲି;
	sys_snode_t *ଗଣ୍ଠି;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		ଯଦି (type->id == type_id) {
			ଫେରନ୍ତୁ type;
		}
	}

	k_spinlock_ଚାବି_t  ଚାବି = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_ଗଣ୍ଠି(&z_obj_type_list, ଗଣ୍ଠି) {
		ସଂରଚନା k_obj_type *type = CONTAINER_OF(ଗଣ୍ଠି, ସଂରଚନା k_obj_type, ଗଣ୍ଠି);

		ଯଦି (type->id == type_id) {
			rv = type;
			ଭାଙ୍ଗ;
		}
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା walk_statics(ସଂରଚନା k_obj_type *type,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*func)(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *data),
			ଶୂନ୍ୟ *data)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = 0;

	ପାଇଁ (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		ଅଚଳ ସଂରଚନା k_obj_range *range = &type->statics[r];

		ଯଦି (range->stride == 0U) {
			ଭାଙ୍ଗ;
		}

		ପାଇଁ (ଅଚଳ uint8_t *elem = range->start; elem < (ଅଚଳ uint8_t *)range->end;
		     elem += range->stride) {
			ସଂରଚନା k_obj_core *obj_core = range_core(type, range, elem);

			ଯଦି (obj_core->type != type) {
				ଜାରି;
			}

			status = func(obj_core, data);
			ଯଦି (status != 0) {
				ଭାଙ୍ଗ;
			}
		}
	}

	ଫେରନ୍ତୁ status;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_type_walk_locked(ସଂରଚନା k_obj_type *type,
			   ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*func)(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *data),
			   ଶୂନ୍ୟ *data)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	ପାଇଁ (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ସଂରଚନା obj_core_slot *slot = &registry[i];
		k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);
		ସଂରଚନା k_obj_core *core = slot_core(slot);

		ଯଦି ((core == ଖାଲି) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, ଚାବି);
			ଜାରି;
		}

		ଯଦି (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, ଖାଲି);
			k_spin_unlock(&obj_core_lock, ଚାବି);
			ଜାରି;
		}

		k_spin_unlock(&obj_core_lock, ଚାବି);

		status = func(core, data);
	}

	ଫେରନ୍ତୁ status;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_type_walk_unlocked(ସଂରଚନା k_obj_type *type,
			   ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*func)(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *data),
			   ଶୂନ୍ୟ *data)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା  status;

	status = walk_statics(type, func, data);

	ପାଇଁ (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		ସଂରଚନା k_obj_core *core = slot_core(&registry[i]);

		ଯଦି ((core == ଖାଲି) || (registry[i].type != type) ||
		    (core->type != type)) {
			ଜାରି;
		}

		status = func(core, data);
	}

	ଫେରନ୍ତୁ status;
}

#ifdef CONFIG_OBJ_CORE_STATS
ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_core_stats_register(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats,
			      size_t stats_len)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);

	ଯଦି (obj_core->type->stats_desc == ଖାଲି) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} ଅନ୍ୟଥା ଯଦି (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} ଅନ୍ୟଥା {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_core_stats_deregister(ସଂରଚନା k_obj_core *obj_core)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);

	ଯଦି (obj_core->type->stats_desc == ଖାଲି) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} ଅନ୍ୟଥା {
		obj_core->stats = ଖାଲି;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_core_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats,
			 size_t stats_len)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	ସଂରଚନା k_obj_core_stats_desc *desc;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ଯଦି ((desc == ଖାଲି) || (desc->raw == ଖାଲି)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ଅନ୍ୟଥା ଯଦି ((desc->raw_size != stats_len) || (obj_core->stats == ଖାଲି)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ଅନ୍ୟଥା {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_core_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats,
			   size_t stats_len)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	ସଂରଚନା k_obj_core_stats_desc *desc;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ଯଦି ((desc == ଖାଲି) || (desc->query == ଖାଲି)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ଅନ୍ୟଥା ଯଦି ((desc->query_size != stats_len) || (obj_core->stats == ଖାଲି)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} ଅନ୍ୟଥା {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_core_stats_reset(ସଂରଚନା k_obj_core *obj_core)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	ସଂରଚନା k_obj_core_stats_desc *desc;

	k_spinlock_ଚାବି_t  ଚାବି = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ଯଦି ((desc == ଖାଲି) || (desc->reset == ଖାଲି)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ଅନ୍ୟଥା ଯଦି (obj_core->stats == ଖାଲି) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ଅନ୍ୟଥା {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_core_stats_disable(ସଂରଚନା k_obj_core *obj_core)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	ସଂରଚନା k_obj_core_stats_desc *desc;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ଯଦି ((desc == ଖାଲି) || (desc->disable == ଖାଲି)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ଅନ୍ୟଥା ଯଦି (obj_core->stats == ଖାଲି) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ଅନ୍ୟଥା {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_obj_core_stats_enable(ସଂରଚନା k_obj_core *obj_core)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	ସଂରଚନା k_obj_core_stats_desc *desc;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	ଯଦି ((desc == ଖାଲି) || (desc->enable == ଖାଲି)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} ଅନ୍ୟଥା ଯଦି (obj_core->stats == ଖାଲି) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} ଅନ୍ୟଥା {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, ଚାବି);

	ଫେରନ୍ତୁ rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
ସ୍ଥିର ସଂରଚନା k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ସ୍ଥିର ସଂରଚନା k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = ଆକାର(ସଂରଚନା k_cycle_stats) * CONFIG_MP_MAX_ଅଙ୍କ_CPUS,
	.query_size = ଆକାର(ସଂରଚନା k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ଖାଲି,
	.disable = ଖାଲି,
	.enable  = ଖାଲି,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
ସ୍ଥିର ଶୂନ୍ୟ init_kernel_obj_core_list(ଶୂନ୍ୟ)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(ସଂରଚନା z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  ଆକାର(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
