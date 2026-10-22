/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <string.h>
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/kernel/obj_core.h>
#అన్తర్భావ <kernel_internal.h>

స్థిర నిర్మాణం k_spinlock  obj_core_తాళం;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
నిర్మాణం obj_core_స్థానం {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	నిర్మాణం k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
స్థిర నిర్మాణం obj_core_స్థానం registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

స్థిర నిర్మాణం k_obj_core *స్థానం_core(స్థిరరాశి నిర్మాణం obj_core_స్థానం *స్థానం)
{
	తిరిగి atomic_ptr_get(&స్థానం->core);
}

స్థిర bool range_కలిగియుంది(స్థిరరాశి నిర్మాణం k_obj_range *range, స్థిరరాశి శూన్య *ptr)
{
	ఒకవేళ (range->పరోక్ష) {
		కొరకు (స్థిరరాశి శూన్య *స్థిరరాశి *pp = range->ప్రారంభం; pp < (స్థిరరాశి శూన్య *స్థిరరాశి *)range->చివర;
		     pp = (స్థిరరాశి శూన్య *స్థిరరాశి *)((స్థిరరాశి uint8_t *)pp + range->stride)) {
			ఒకవేళ (*pp == ptr) {
				తిరిగి true;
			}
		}
		తిరిగి false;
	}

	తిరిగి (ptr >= range->ప్రారంభం) && (ptr < range->చివర);
}

స్థిర bool type_ranges_contain(స్థిరరాశి నిర్మాణం k_obj_type *type, స్థిరరాశి శూన్య *ptr)
{
	కొరకు (size_t i = 0; i < ARRAY_కొలత(type->statics); i++) {
		స్థిరరాశి నిర్మాణం k_obj_range *range = &type->statics[i];

		ఒకవేళ (range->stride == 0U) {
			ఆపు;
		}
		ఒకవేళ (range_కలిగియుంది(range, ptr)) {
			తిరిగి true;
		}
	}

	తిరిగి false;
}

/* Object core of the range element */
స్థిర నిర్మాణం k_obj_core *range_core(స్థిరరాశి నిర్మాణం k_obj_type *type,
				     స్థిరరాశి నిర్మాణం k_obj_range *range, స్థిరరాశి శూన్య *elem)
{
	స్థిరరాశి uint8_t *obj = range->పరోక్ష ? *(స్థిరరాశి uint8_t *స్థిరరాశి *)elem : elem;

	తిరిగి (నిర్మాణం k_obj_core *)(obj + type->obj_core_స్థానభ్రంశం);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
స్థిర bool లోపల_దొంతర_storage(స్థిరరాశి శూన్య *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	స్థిరరాశి నిర్మాణం k_thread *thread = _current;
	uintptr_t irq_దొంతర = (uintptr_t)K_KERNEL_దొంతర_బఫరు(
		z_అంతరాయం_stacks[_current_cpu->id]);
	uintptr_t irq_దొంతర_చివర =
		irq_దొంతర + K_KERNEL_దొంతర_SIZEOF(z_అంతరాయం_stacks[0]);

	ఒకవేళ ((thread != ఖాళీ) && (thread->దొంతర_info.కొలత != 0U) &&
	    (addr >= thread->దొంతర_info.ప్రారంభం) &&
	    (addr < thread->దొంతర_info.ప్రారంభం + thread->దొంతర_info.కొలత)) {
		తిరిగి true;
	}

	తిరిగి (addr >= irq_దొంతర) && (addr < irq_దొంతర_చివర);
}

స్థిర నిర్మాణం obj_core_స్థానం *స్థానం_కనుగొను(స్థిరరాశి నిర్మాణం k_obj_core *core)
{
	కొరకు (size_t i = 0; i < ARRAY_కొలత(registry); i++) {
		ఒకవేళ (స్థానం_core(&registry[i]) == core) {
			తిరిగి &registry[i];
		}
	}

	తిరిగి ఖాళీ;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
స్థిర bool స్థానం_stale(స్థిరరాశి నిర్మాణం obj_core_స్థానం *స్థానం, స్థిరరాశి నిర్మాణం k_obj_core *core)
{
	తిరిగి core->type != స్థానం->type;
}

స్థిర నిర్మాణం obj_core_స్థానం *స్థానం_alloc(శూన్య)
{
	నిర్మాణం obj_core_స్థానం *స్థానం = స్థానం_కనుగొను(ఖాళీ);

	ఒకవేళ (స్థానం != ఖాళీ) {
		తిరిగి స్థానం;
	}

	/* Full: reap every stale entry and reuse one of them */

	కొరకు (size_t i = 0; i < ARRAY_కొలత(registry); i++) {
		నిర్మాణం k_obj_core *core = స్థానం_core(&registry[i]);

		ఒకవేళ ((core != ఖాళీ) && స్థానం_stale(&registry[i], core)) {
			atomic_ptr_సమితి(&registry[i].core, ఖాళీ);
			స్థానం = &registry[i];
		}
	}

	తిరిగి స్థానం;
}

నిర్మాణం k_obj_type *z_obj_type_init(నిర్మాణం k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->కణుపు);
	type->id = id;
	type->obj_core_స్థానభ్రంశం = off;
	memset(type->statics, 0, పరిమాణం(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	తిరిగి type;
}

శూన్య k_obj_type_init_range(నిర్మాణం k_obj_type *type, స్థిరరాశి శూన్య *ప్రారంభం,
			   స్థిరరాశి శూన్య *చివర, size_t stride, bool పరోక్ష)
{
	type->statics[0].ప్రారంభం = ప్రారంభం;
	type->statics[0].చివర = చివర;
	type->statics[0].stride = stride;
	type->statics[0].పరోక్ష = పరోక్ష;
}

శూన్య k_obj_core_init(నిర్మాణం k_obj_core *obj_core, నిర్మాణం k_obj_type *type)
{
	obj_core->type = type;
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS
	obj_core->stats = ఖాళీ;
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS */
}

శూన్య k_obj_core_link(నిర్మాణం k_obj_core *obj_core)
{
	నిర్మాణం k_obj_type *type = obj_core->type;
	నిర్మాణం obj_core_స్థానం *స్థానం;

	ఒకవేళ (type_ranges_contain(type, (స్థిరరాశి uint8_t *)obj_core - type->obj_core_స్థానభ్రంశం)) {
		తిరిగి;
	}

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);

	ఒకవేళ (లోపల_దొంతర_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_తాళం, కీలకం);
		తిరిగి;
	}

	స్థానం = స్థానం_కనుగొను(obj_core);
	ఒకవేళ (స్థానం == ఖాళీ) {
		స్థానం = స్థానం_alloc();
	}

	ఒకవేళ (స్థానం == ఖాళీ) {
		type->dropped++;
	} లేకపోతే {
		స్థానం->type = type;
		atomic_ptr_సమితి(&స్థానం->core, obj_core);
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);
}

శూన్య k_obj_core_init_and_link(నిర్మాణం k_obj_core *obj_core,
			      నిర్మాణం k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

శూన్య k_obj_core_unlink(నిర్మాణం k_obj_core *obj_core)
{
	కొరకు (size_t i = 0; i < ARRAY_కొలత(registry); i++) {
		ఒకవేళ (atomic_ptr_cas(&registry[i].core, obj_core, ఖాళీ)) {
			ఆపు;
		}
	}
}

శూన్య k_obj_core_evict_range(స్థిరరాశి శూన్య *addr, size_t len)
{
	స్థిరరాశి uint8_t *ప్రారంభం = addr;
	స్థిరరాశి uint8_t *చివర = ప్రారంభం + len;

	కొరకు (size_t i = 0; i < ARRAY_కొలత(registry); i++) {
		నిర్మాణం k_obj_core *core = స్థానం_core(&registry[i]);

		ఒకవేళ ((core != ఖాళీ) && ((స్థిరరాశి uint8_t *)core >= ప్రారంభం) &&
		    ((స్థిరరాశి uint8_t *)core < చివర)) {
			(శూన్య)atomic_ptr_cas(&registry[i].core, core, ఖాళీ);
		}
	}
}

#నిర్వచితమైతే CONFIG_OBJ_CORE_EVICT_ON_FREE
శూన్య sys_కుప్ప_release_hook(శూన్య *mem, size_t బైట్లు)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	ఒకవేళ (k_is_user_context()) {
		తిరిగి;
	}

	k_obj_core_evict_range(mem, బైట్లు);
}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
స్థిర శూన్య obj_core_init_all(శూన్య)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->కణుపు);

		కొరకు (size_t r = 0; r < ARRAY_కొలత(type->statics); r++) {
			స్థిరరాశి నిర్మాణం k_obj_range *range = &type->statics[r];

			ఒకవేళ (range->stride == 0U) {
				ఆపు;
			}

			కొరకు (స్థిరరాశి uint8_t *elem = range->ప్రారంభం;
			     elem < (స్థిరరాశి uint8_t *)range->చివర; elem += range->stride) {
				నిర్మాణం k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS
				ఒకవేళ ((type->stats_desc != ఖాళీ) && (type->stats_కొలత != 0)) {
					k_obj_core_stats_రిజిస్టరు(
						obj_core,
						(శూన్య *)((uint8_t *)obj_core -
							 type->obj_core_స్థానభ్రంశం +
							 type->stats_స్థానభ్రంశం),
						type->stats_కొలత);
				}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS */
			}
		}
	}
}

K_KERNEL_INIT_PRE(obj_core_init_all);

నిర్మాణం k_obj_type *k_obj_type_కనుగొను(uint32_t type_id)
{
	నిర్మాణం k_obj_type *rv = ఖాళీ;
	sys_snode_t *కణుపు;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		ఒకవేళ (type->id == type_id) {
			తిరిగి type;
		}
	}

	k_spinlock_కీలకం_t  కీలకం = k_spin_తాళం(&obj_core_తాళం);

	SYS_SLIST_FOR_EACH_కణుపు(&z_obj_type_list, కణుపు) {
		నిర్మాణం k_obj_type *type = CONTAINER_యొక్క(కణుపు, నిర్మాణం k_obj_type, కణుపు);

		ఒకవేళ (type->id == type_id) {
			rv = type;
			ఆపు;
		}
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
స్థిర పూర్ణాంకం walk_statics(నిర్మాణం k_obj_type *type,
			పూర్ణాంకం (*func)(నిర్మాణం k_obj_core *obj_core, శూన్య *దత్తాంశం),
			శూన్య *దత్తాంశం)
{
	పూర్ణాంకం స్థితి_2 = 0;

	కొరకు (size_t r = 0; (స్థితి_2 == 0) && (r < ARRAY_కొలత(type->statics)); r++) {
		స్థిరరాశి నిర్మాణం k_obj_range *range = &type->statics[r];

		ఒకవేళ (range->stride == 0U) {
			ఆపు;
		}

		కొరకు (స్థిరరాశి uint8_t *elem = range->ప్రారంభం; elem < (స్థిరరాశి uint8_t *)range->చివర;
		     elem += range->stride) {
			నిర్మాణం k_obj_core *obj_core = range_core(type, range, elem);

			ఒకవేళ (obj_core->type != type) {
				కొనసాగు;
			}

			స్థితి_2 = func(obj_core, దత్తాంశం);
			ఒకవేళ (స్థితి_2 != 0) {
				ఆపు;
			}
		}
	}

	తిరిగి స్థితి_2;
}

పూర్ణాంకం k_obj_type_walk_locked(నిర్మాణం k_obj_type *type,
			   పూర్ణాంకం (*func)(నిర్మాణం k_obj_core *obj_core, శూన్య *దత్తాంశం),
			   శూన్య *దత్తాంశం)
{
	పూర్ణాంకం  స్థితి_2;

	స్థితి_2 = walk_statics(type, func, దత్తాంశం);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	కొరకు (size_t i = 0; (స్థితి_2 == 0) && (i < ARRAY_కొలత(registry)); i++) {
		నిర్మాణం obj_core_స్థానం *స్థానం = &registry[i];
		k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);
		నిర్మాణం k_obj_core *core = స్థానం_core(స్థానం);

		ఒకవేళ ((core == ఖాళీ) || (స్థానం->type != type)) {
			k_spin_unlock(&obj_core_తాళం, కీలకం);
			కొనసాగు;
		}

		ఒకవేళ (స్థానం_stale(స్థానం, core)) {
			atomic_ptr_సమితి(&స్థానం->core, ఖాళీ);
			k_spin_unlock(&obj_core_తాళం, కీలకం);
			కొనసాగు;
		}

		k_spin_unlock(&obj_core_తాళం, కీలకం);

		స్థితి_2 = func(core, దత్తాంశం);
	}

	తిరిగి స్థితి_2;
}

పూర్ణాంకం k_obj_type_walk_unlocked(నిర్మాణం k_obj_type *type,
			   పూర్ణాంకం (*func)(నిర్మాణం k_obj_core *obj_core, శూన్య *దత్తాంశం),
			   శూన్య *దత్తాంశం)
{
	పూర్ణాంకం  స్థితి_2;

	స్థితి_2 = walk_statics(type, func, దత్తాంశం);

	కొరకు (size_t i = 0; (స్థితి_2 == 0) && (i < ARRAY_కొలత(registry)); i++) {
		నిర్మాణం k_obj_core *core = స్థానం_core(&registry[i]);

		ఒకవేళ ((core == ఖాళీ) || (registry[i].type != type) ||
		    (core->type != type)) {
			కొనసాగు;
		}

		స్థితి_2 = func(core, దత్తాంశం);
	}

	తిరిగి స్థితి_2;
}

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS
పూర్ణాంకం k_obj_core_stats_రిజిస్టరు(నిర్మాణం k_obj_core *obj_core, శూన్య *stats,
			      size_t stats_len)
{
	పూర్ణాంకం rv;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);

	ఒకవేళ (obj_core->type->stats_desc == ఖాళీ) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} లేకపోతే ఒకవేళ (obj_core->type->stats_desc->raw_కొలత != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} లేకపోతే {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}

పూర్ణాంకం k_obj_core_stats_deregister(నిర్మాణం k_obj_core *obj_core)
{
	పూర్ణాంకం rv;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);

	ఒకవేళ (obj_core->type->stats_desc == ఖాళీ) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} లేకపోతే {
		obj_core->stats = ఖాళీ;
		rv = 0;
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}

పూర్ణాంకం k_obj_core_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats,
			 size_t stats_len)
{
	పూర్ణాంకం rv;
	నిర్మాణం k_obj_core_stats_desc *desc;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);

	desc = obj_core->type->stats_desc;
	ఒకవేళ ((desc == ఖాళీ) || (desc->raw == ఖాళీ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} లేకపోతే ఒకవేళ ((desc->raw_కొలత != stats_len) || (obj_core->stats == ఖాళీ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} లేకపోతే {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}

పూర్ణాంకం k_obj_core_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats,
			   size_t stats_len)
{
	పూర్ణాంకం rv;
	నిర్మాణం k_obj_core_stats_desc *desc;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);

	desc = obj_core->type->stats_desc;
	ఒకవేళ ((desc == ఖాళీ) || (desc->query == ఖాళీ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} లేకపోతే ఒకవేళ ((desc->query_కొలత != stats_len) || (obj_core->stats == ఖాళీ)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} లేకపోతే {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}

పూర్ణాంకం k_obj_core_stats_reset(నిర్మాణం k_obj_core *obj_core)
{
	పూర్ణాంకం rv;
	నిర్మాణం k_obj_core_stats_desc *desc;

	k_spinlock_కీలకం_t  కీలకం = k_spin_తాళం(&obj_core_తాళం);

	desc = obj_core->type->stats_desc;
	ఒకవేళ ((desc == ఖాళీ) || (desc->reset == ఖాళీ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} లేకపోతే ఒకవేళ (obj_core->stats == ఖాళీ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} లేకపోతే {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}

పూర్ణాంకం k_obj_core_stats_disable(నిర్మాణం k_obj_core *obj_core)
{
	పూర్ణాంకం rv;
	నిర్మాణం k_obj_core_stats_desc *desc;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);

	desc = obj_core->type->stats_desc;
	ఒకవేళ ((desc == ఖాళీ) || (desc->disable == ఖాళీ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} లేకపోతే ఒకవేళ (obj_core->stats == ఖాళీ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} లేకపోతే {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}

పూర్ణాంకం k_obj_core_stats_enable(నిర్మాణం k_obj_core *obj_core)
{
	పూర్ణాంకం rv;
	నిర్మాణం k_obj_core_stats_desc *desc;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_core_తాళం);

	desc = obj_core->type->stats_desc;
	ఒకవేళ ((desc == ఖాళీ) || (desc->enable == ఖాళీ)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} లేకపోతే ఒకవేళ (obj_core->stats == ఖాళీ) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} లేకపోతే {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_తాళం, కీలకం);

	తిరిగి rv;
}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS */

#నిర్వచితమైతే CONFIG_OBJ_CORE_SYSTEM
స్థిర నిర్మాణం k_obj_type obj_type_kernel;

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
స్థిర నిర్మాణం k_obj_core_stats_desc kernel_stats_desc = {
	.raw_కొలత = పరిమాణం(నిర్మాణం k_చక్రం_stats) * CONFIG_MP_MAX_అంకె_CPUS,
	.query_కొలత = పరిమాణం(నిర్మాణం k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = ఖాళీ,
	.disable = ఖాళీ,
	.enable  = ఖాళీ,
};
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
స్థిర శూన్య init_kernel_obj_core_list(శూన్య)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(నిర్మాణం z_kernel, obj_core));

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_రిజిస్టరు(K_OBJ_CORE(&_kernel), _kernel.usage,
				  పరిమాణం(_kernel.usage));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_SYSTEM */
