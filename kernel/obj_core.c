/*
 * Copyright (c) 2023, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/obj_core.h>
#include <kernel_internal.h>

static struct k_spinlock  obj_core_lock;

sys_slist_t z_obj_type_list = SYS_SLIST_STATIC_INIT(&z_obj_type_list);

/* Registry of the objects registered at run time. It references the objects
 * and never stores anything inside them, so an object that is discarded
 * without being unregistered leaves a stale entry but cannot corrupt the
 * registry or any other object.
 */
struct obj_core_slot {
	atomic_ptr_t core;          /* struct k_obj_core *, NULL when free */
	struct k_obj_type *type;
};

/* Entries are allocated under obj_core_lock. They are released with an
 * atomic exchange, so unregistering and evicting never take the lock and
 * can run from any context, including a walk callback.
 */
static struct obj_core_slot registry[CONFIG_OBJ_CORE_MAX_DYNAMIC_OBJECTS];

static struct k_obj_core *slot_core(const struct obj_core_slot *slot)
{
	반환 atomic_ptr_get(&slot->core);
}

static bool range_contains(const struct k_obj_range *range, const void *ptr)
{
	만약 (range->indirect) {
		위해 (const void *const *pp = range->start; pp < (const void *const *)range->end;
		     pp = (const void *const *)((const uint8_t *)pp + range->stride)) {
			만약 (*pp == ptr) {
				반환 true;
			}
		}
		반환 false;
	}

	반환 (ptr >= range->start) && (ptr < range->end);
}

static bool type_ranges_contain(const struct k_obj_type *type, const void *ptr)
{
	위해 (size_t i = 0; i < ARRAY_SIZE(type->statics); i++) {
		const struct k_obj_range *range = &type->statics[i];

		만약 (range->stride == 0U) {
			break;
		}
		만약 (range_contains(range, ptr)) {
			반환 true;
		}
	}

	반환 false;
}

/* Object core of the range element */
static struct k_obj_core *range_core(const struct k_obj_type *type,
				     const struct k_obj_range *range, const void *elem)
{
	const uint8_t *obj = range->indirect ? *(const uint8_t *const *)elem : elem;

	반환 (struct k_obj_core *)(obj + type->obj_core_offset);
}

/* Objects in the running thread's stack or in the interrupt stack are
 * transient by construction. Called with the registry lock held, which pins
 * the current CPU. There is no current thread before the kernel has set one
 * up, and never with CONFIG_MULTITHREADING=n.
 */
static bool in_stack_storage(const void *ptr)
{
	uintptr_t addr = (uintptr_t)ptr;
	const struct k_thread *thread = _current;
	uintptr_t irq_stack = (uintptr_t)K_KERNEL_STACK_BUFFER(
		z_interrupt_stacks[_current_cpu->id]);
	uintptr_t irq_stack_end =
		irq_stack + K_KERNEL_STACK_SIZEOF(z_interrupt_stacks[0]);

	만약 ((thread != NULL) && (thread->stack_info.size != 0U) &&
	    (addr >= thread->stack_info.start) &&
	    (addr < thread->stack_info.start + thread->stack_info.size)) {
		반환 true;
	}

	반환 (addr >= irq_stack) && (addr < irq_stack_end);
}

static struct obj_core_slot *slot_find(const struct k_obj_core *core)
{
	위해 (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		만약 (slot_core(&registry[i]) == core) {
			반환 &registry[i];
		}
	}

	반환 NULL;
}

/* A registered object whose storage no longer carries its type is stale.
 * The caller passes the object it read from the slot: the slot may be
 * cleared concurrently by an unregistration.
 */
static bool slot_stale(const struct obj_core_slot *slot, const struct k_obj_core *core)
{
	반환 core->type != slot->type;
}

static struct obj_core_slot *slot_alloc(void)
{
	struct obj_core_slot *slot = slot_find(NULL);

	만약 (slot != NULL) {
		반환 slot;
	}

	/* Full: reap every stale entry and reuse one of them */

	위해 (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		struct k_obj_core *core = slot_core(&registry[i]);

		만약 ((core != NULL) && slot_stale(&registry[i], core)) {
			atomic_ptr_set(&registry[i].core, NULL);
			slot = &registry[i];
		}
	}

	반환 slot;
}

struct k_obj_type *z_obj_type_init(struct k_obj_type *type,
				   uint32_t id, size_t off)
{
	sys_slist_append(&z_obj_type_list, &type->node);
	type->id = id;
	type->obj_core_offset = off;
	memset(type->statics, 0, sizeof(type->statics));
	type->dropped = 0;
	type->skipped = 0;

	반환 type;
}

void k_obj_type_init_range(struct k_obj_type *type, const void *start,
			   const void *end, size_t stride, bool indirect)
{
	type->statics[0].start = start;
	type->statics[0].end = end;
	type->statics[0].stride = stride;
	type->statics[0].indirect = indirect;
}

void k_obj_core_init(struct k_obj_core *obj_core, struct k_obj_type *type)
{
	obj_core->type = type;
#ifdef CONFIG_OBJ_CORE_STATS
	obj_core->stats = NULL;
#endif /* CONFIG_OBJ_CORE_STATS */
}

void k_obj_core_link(struct k_obj_core *obj_core)
{
	struct k_obj_type *type = obj_core->type;
	struct obj_core_slot *slot;

	만약 (type_ranges_contain(type, (const uint8_t *)obj_core - type->obj_core_offset)) {
		반환;
	}

	k_spinlock_key_t key = k_spin_lock(&obj_core_lock);

	만약 (in_stack_storage(obj_core)) {
		type->skipped++;
		k_spin_unlock(&obj_core_lock, key);
		반환;
	}

	slot = slot_find(obj_core);
	만약 (slot == NULL) {
		slot = slot_alloc();
	}

	만약 (slot == NULL) {
		type->dropped++;
	} 아니면 {
		slot->type = type;
		atomic_ptr_set(&slot->core, obj_core);
	}

	k_spin_unlock(&obj_core_lock, key);
}

void k_obj_core_init_and_link(struct k_obj_core *obj_core,
			      struct k_obj_type *type)
{
	k_obj_core_init(obj_core, type);
	k_obj_core_link(obj_core);
}

void k_obj_core_unlink(struct k_obj_core *obj_core)
{
	위해 (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		만약 (atomic_ptr_cas(&registry[i].core, obj_core, NULL)) {
			break;
		}
	}
}

void k_obj_core_evict_range(const void *addr, size_t len)
{
	const uint8_t *start = addr;
	const uint8_t *end = start + len;

	위해 (size_t i = 0; i < ARRAY_SIZE(registry); i++) {
		struct k_obj_core *core = slot_core(&registry[i]);

		만약 ((core != NULL) && ((const uint8_t *)core >= start) &&
		    ((const uint8_t *)core < end)) {
			(void)atomic_ptr_cas(&registry[i].core, core, NULL);
		}
	}
}

#ifdef CONFIG_OBJ_CORE_EVICT_ON_FREE
void sys_heap_release_hook(void *mem, size_t bytes)
{
	/* The registry is kernel data; user mode releases never hold
	 * registered objects.
	 */
	만약 (k_is_user_context()) {
		반환;
	}

	k_obj_core_evict_range(mem, bytes);
}
#endif /* CONFIG_OBJ_CORE_EVICT_ON_FREE */

/* Add the object types defined at build time to the type list and initialize
 * the object cores of their permanent objects.
 */
static void obj_core_init_all(void)
{
	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		sys_slist_append(&z_obj_type_list, &type->node);

		위해 (size_t r = 0; r < ARRAY_SIZE(type->statics); r++) {
			const struct k_obj_range *range = &type->statics[r];

			만약 (range->stride == 0U) {
				break;
			}

			위해 (const uint8_t *elem = range->start;
			     elem < (const uint8_t *)range->end; elem += range->stride) {
				struct k_obj_core *obj_core = range_core(type, range, elem);

				k_obj_core_init(obj_core, type);
#ifdef CONFIG_OBJ_CORE_STATS
				만약 ((type->stats_desc != NULL) && (type->stats_size != 0)) {
					k_obj_core_stats_register(
						obj_core,
						(void *)((uint8_t *)obj_core -
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

struct k_obj_type *k_obj_type_find(uint32_t type_id)
{
	struct k_obj_type *rv = NULL;
	sys_snode_t *node;

	/* Types defined at build time are valid before the type list is */

	STRUCT_SECTION_FOREACH(k_obj_type, type) {
		만약 (type->id == type_id) {
			반환 type;
		}
	}

	k_spinlock_key_t  key = k_spin_lock(&obj_core_lock);

	SYS_SLIST_FOR_EACH_NODE(&z_obj_type_list, node) {
		struct k_obj_type *type = CONTAINER_OF(node, struct k_obj_type, node);

		만약 (type->id == type_id) {
			rv = type;
			break;
		}
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}

/* Invoke func on every permanent object of the type that carries its type
 * tag. Objects of a zero-initialized array that were never initialized do
 * not.
 */
static 정수 walk_statics(struct k_obj_type *type,
			정수 (*func)(struct k_obj_core *obj_core, void *data),
			void *data)
{
	정수 status = 0;

	위해 (size_t r = 0; (status == 0) && (r < ARRAY_SIZE(type->statics)); r++) {
		const struct k_obj_range *range = &type->statics[r];

		만약 (range->stride == 0U) {
			break;
		}

		위해 (const uint8_t *elem = range->start; elem < (const uint8_t *)range->end;
		     elem += range->stride) {
			struct k_obj_core *obj_core = range_core(type, range, elem);

			만약 (obj_core->type != type) {
				continue;
			}

			status = func(obj_core, data);
			만약 (status != 0) {
				break;
			}
		}
	}

	반환 status;
}

정수 k_obj_type_walk_locked(struct k_obj_type *type,
			   정수 (*func)(struct k_obj_core *obj_core, void *data),
			   void *data)
{
	정수  status;

	status = walk_statics(type, func, data);

	/* The lock covers reading and reaping each entry, not the callback,
	 * so the callback is free to use any kernel or object core function.
	 */
	위해 (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		struct obj_core_slot *slot = &registry[i];
		k_spinlock_key_t key = k_spin_lock(&obj_core_lock);
		struct k_obj_core *core = slot_core(slot);

		만약 ((core == NULL) || (slot->type != type)) {
			k_spin_unlock(&obj_core_lock, key);
			continue;
		}

		만약 (slot_stale(slot, core)) {
			atomic_ptr_set(&slot->core, NULL);
			k_spin_unlock(&obj_core_lock, key);
			continue;
		}

		k_spin_unlock(&obj_core_lock, key);

		status = func(core, data);
	}

	반환 status;
}

정수 k_obj_type_walk_unlocked(struct k_obj_type *type,
			   정수 (*func)(struct k_obj_core *obj_core, void *data),
			   void *data)
{
	정수  status;

	status = walk_statics(type, func, data);

	위해 (size_t i = 0; (status == 0) && (i < ARRAY_SIZE(registry)); i++) {
		struct k_obj_core *core = slot_core(&registry[i]);

		만약 ((core == NULL) || (registry[i].type != type) ||
		    (core->type != type)) {
			continue;
		}

		status = func(core, data);
	}

	반환 status;
}

#ifdef CONFIG_OBJ_CORE_STATS
정수 k_obj_core_stats_register(struct k_obj_core *obj_core, void *stats,
			      size_t stats_len)
{
	정수 rv;
	k_spinlock_key_t key = k_spin_lock(&obj_core_lock);

	만약 (obj_core->type->stats_desc == NULL) {
		/* Object type not configured for statistics. */
		rv = -ENOTSUP;
	} 아니면 만약 (obj_core->type->stats_desc->raw_size != stats_len) {
		/* Buffer size mismatch */
		rv = -EINVAL;
	} 아니면 {
		obj_core->stats = stats;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}

정수 k_obj_core_stats_deregister(struct k_obj_core *obj_core)
{
	정수 rv;
	k_spinlock_key_t key = k_spin_lock(&obj_core_lock);

	만약 (obj_core->type->stats_desc == NULL) {
		/* Object type not configured  for statistics. */
		rv = -ENOTSUP;
	} 아니면 {
		obj_core->stats = NULL;
		rv = 0;
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}

정수 k_obj_core_stats_raw(struct k_obj_core *obj_core, void *stats,
			 size_t stats_len)
{
	정수 rv;
	struct k_obj_core_stats_desc *desc;

	k_spinlock_key_t key = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	만약 ((desc == NULL) || (desc->raw == NULL)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} 아니면 만약 ((desc->raw_size != stats_len) || (obj_core->stats == NULL)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} 아니면 {
		rv = desc->raw(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}

정수 k_obj_core_stats_query(struct k_obj_core *obj_core, void *stats,
			   size_t stats_len)
{
	정수 rv;
	struct k_obj_core_stats_desc *desc;

	k_spinlock_key_t key = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	만약 ((desc == NULL) || (desc->query == NULL)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} 아니면 만약 ((desc->query_size != stats_len) || (obj_core->stats == NULL)) {
		/*
		 * Either the size of the stats buffer is wrong or
		 * the kernel object was not registered for statistics.
		 */
		rv = -EINVAL;
	} 아니면 {
		rv = desc->query(obj_core, stats);
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}

정수 k_obj_core_stats_reset(struct k_obj_core *obj_core)
{
	정수 rv;
	struct k_obj_core_stats_desc *desc;

	k_spinlock_key_t  key = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	만약 ((desc == NULL) || (desc->reset == NULL)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} 아니면 만약 (obj_core->stats == NULL) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} 아니면 {
		rv = desc->reset(obj_core);
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}

정수 k_obj_core_stats_disable(struct k_obj_core *obj_core)
{
	정수 rv;
	struct k_obj_core_stats_desc *desc;

	k_spinlock_key_t key = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	만약 ((desc == NULL) || (desc->disable == NULL)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} 아니면 만약 (obj_core->stats == NULL) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} 아니면 {
		rv = desc->disable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}

정수 k_obj_core_stats_enable(struct k_obj_core *obj_core)
{
	정수 rv;
	struct k_obj_core_stats_desc *desc;

	k_spinlock_key_t key = k_spin_lock(&obj_core_lock);

	desc = obj_core->type->stats_desc;
	만약 ((desc == NULL) || (desc->enable == NULL)) {
		/* The object type is not configured for this operation */
		rv = -ENOTSUP;
	} 아니면 만약 (obj_core->stats == NULL) {
		/* This kernel object is not configured for statistics */
		rv = -EINVAL;
	} 아니면 {
		rv = desc->enable(obj_core);
	}

	k_spin_unlock(&obj_core_lock, key);

	반환 rv;
}
#endif /* CONFIG_OBJ_CORE_STATS */

#ifdef CONFIG_OBJ_CORE_SYSTEM
static struct k_obj_type obj_type_kernel;

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
static struct k_obj_core_stats_desc kernel_stats_desc = {
	.raw_size = sizeof(struct k_cycle_stats) * CONFIG_MP_MAX_NUM_CPUS,
	.query_size = sizeof(struct k_thread_runtime_stats),
	.raw   = z_kernel_stats_raw,
	.query = z_kernel_stats_query,
	.reset = NULL,
	.disable = NULL,
	.enable  = NULL,
};
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

/* The kernel object is a singleton (_kernel), so it is registered and linked
 * here directly rather than through the object type table.
 */
static void init_kernel_obj_core_list(void)
{
	/* Initialize kernel object type */

	z_obj_type_init(&obj_type_kernel, K_OBJ_TYPE_KERNEL_ID,
			offsetof(struct z_kernel, obj_core));

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_type_stats_init(&obj_type_kernel, &kernel_stats_desc);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel), &obj_type_kernel);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel), _kernel.usage,
				  sizeof(_kernel.usage));
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
}

K_KERNEL_INIT_PRE(init_kernel_obj_core_list);
#endif /* CONFIG_OBJ_CORE_SYSTEM */
