/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/sys/rb.h>
#include <zephyr/sys/sys_io.h>
#include <ksched.h>
#include <zephyr/syscall.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/device.h>
#include <stdbool.h>
#include <zephyr/app_memory/app_memdomain.h>
#include <zephyr/sys/libc-hooks.h>
#include <zephyr/sys/mutex.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/cache.h>
#include <kernel_internal.h>

#ifdef Z_LIBC_PARTITION_EXISTS
K_APPMEM_PARTITION_DEFINE(z_libc_partition);
#endif /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#ifdef CONFIG_MBEDTLS
K_APPMEM_PARTITION_DEFINE(k_mbedtls_partition);
#endif /* CONFIG_MBEDTLS */

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* The originally synchronization strategy made heavy use of recursive
 * irq_locking, which ports poorly to spinlocks which are
 * non-recursive.  Rather than try to redesign as part of
 * spinlockification, this uses multiple locks to preserve the
 * original semantics exactly.  The locks are named for the data they
 * protect where possible, or just for the code that uses them where
 * not.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
static sa_nracanaa k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#ᱡᱩᱫᱤ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define STACK_ELEMENT_DATA_SIZE(size) \
	(parimaa_na(sa_nracanaa z_stack_data) + CONFIG_PRIVILEGED_STACK_SIZE + \
	Z_THREAD_STACK_OBJ_ALIGN(size) + K_THREAD_STACK_LEN(size))
#ᱵᱟᱝ
#define STACK_ELEMENT_DATA_SIZE(size) (parimaa_na(sa_nracanaa z_stack_data) + \
	K_THREAD_STACK_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#ᱵᱟᱝ
#define STACK_ELEMENT_DATA_SIZE(size) K_THREAD_STACK_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
static sa_nracanaa k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
extern uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

extern puur_naa_nka z_msgq_cleanup(sa_nracanaa k_msgq *q, bool locked);
extern puur_naa_nka z_stack_cleanup(sa_nracanaa k_stack *stack, bool locked);
extern puur_naa_nka z_timer_cleanup(sa_nracanaa k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

static void unref_check(sa_nracanaa k_object *ko, uintptr_t index, bool locked);

const var_na *otype_to_str(ga_nanaa k_objects otype)
{
	const var_na *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	switch (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	pak_sa K_OBJ_ANY:
		ret = "generic";
		bha_nga;
	pak_sa K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		bha_nga;
#include <zephyr/otype-to-str.h>
	default:
		ret = "?";
		bha_nga;
	}
#ᱵᱟᱝ
	ARG_UNUSED(otype);
	ret = NULL;
#endif /* CONFIG_LOG */
	ᱨᱩᱣᱟᱹᱲ ret;
}

sa_nracanaa perm_ctx {
	puur_naa_nka parent_id;
	puur_naa_nka child_id;
	sa_nracanaa k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_stack_find(k_thread_stack_t *stack)
{
	sa_nracanaa k_object *obj = k_object_find(stack);

	__ASSERT(obj != NULL, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_STACK_ELEMENT,
		 "bad stack object");

	ᱨᱩᱣᱟᱹᱲ obj->data.stack_data->priv;
}
#endif /* CONFIG_GEN_PRIV_STACKS */

#ifdef CONFIG_DYNAMIC_OBJECTS

/*
 * Note that dyn_obj->data is where the kernel object resides
 * so it is the one that actually needs to be aligned.
 * Due to the need to get the fields inside struct dyn_obj
 * from kernel object pointers (i.e. from data[]), the offset
 * from data[] needs to be fixed at build time. Therefore,
 * data[] is declared with __aligned(), such that when dyn_obj
 * is allocated with alignment, data[] is also aligned.
 * Due to this requirement, data[] needs to be aligned with
 * the maximum alignment needed for all kernel objects
 * (hence the following DYN_OBJ_DATA_ALIGN).
 */
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#ᱵᱟᱝ
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(parimaa_na(void *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_STACK_SIZE
#ᱡᱩᱫᱤ defined(CONFIG_MPU_STACK_GUARD) || defined(CONFIG_PMP_STACK_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_STACK \
	Z_THREAD_STACK_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_STACK_SIZE)
#ᱵᱟᱝ
#define DYN_OBJ_DATA_ALIGN_K_THREAD_STACK \
	Z_THREAD_STACK_OBJ_ALIGN(CONFIG_PRIVILEGED_STACK_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#ᱵᱟᱝ
#define DYN_OBJ_DATA_ALIGN_K_THREAD_STACK \
	Z_THREAD_STACK_OBJ_ALIGN(ARCH_STACK_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (parimaa_na(void *)))

sa_nracanaa dyn_obj {
	sa_nracanaa k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	void *data;
};

extern sa_nracanaa k_object *z_object_gperf_find(const void *obj);
extern void z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     void *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
static sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

static size_t obj_size_get(ga_nanaa k_objects otype)
{
	size_t ret;

	switch (otype) {
#include <zephyr/otype-to-size.h>
	default:
		ret = parimaa_na(const sa_nracanaa device);
		bha_nga;
	}

	ᱨᱩᱣᱟᱹᱲ ret;
}

static size_t obj_align_get(ga_nanaa k_objects otype)
{
	size_t ret;

	switch (otype) {
	pak_sa K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#ᱵᱟᱝ
		ret = __alignof(sa_nracanaa dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		bha_nga;
	default:
		ret = __alignof(sa_nracanaa dyn_obj);
		bha_nga;
	}

	ᱨᱩᱣᱟᱹᱲ ret;
}

static sa_nracanaa dyn_obj *dyn_object_find_locked(const void *obj)
{
	sa_nracanaa dyn_obj *node;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, node, dobj_list) {
		ᱡᱩᱫᱤ (node->kobj.name == obj) {
			ᱨᱩᱣᱟᱹᱲ node;
		}
	}

	ᱨᱩᱣᱟᱹᱲ NULL;
}

static sa_nracanaa dyn_obj *dyn_object_find(const void *obj)
{
	sa_nracanaa dyn_obj *node;
	k_spinlock_key_t key;

	key = k_spin_lock(&lists_lock);
	node = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, key);

	ᱨᱩᱣᱟᱹᱲ node;
}

static void clear_perms_cb(sa_nracanaa k_object *ko, void *ctx_ptr)
{
	uintptr_t id = (uintptr_t)ctx_ptr;

	unref_check(ko, id, false);
}

/**
 * @internal
 *
 * @brief Allocate a new thread index for a new thread.
 *
 * This finds an unused thread index that can be assigned to a new
 * thread. If too many threads have been allocated, the kernel will
 * run out of indexes and this function will fail.
 *
 * Note that if an unused index is found, that index will be marked as
 * used after return of this function.
 *
 * @param tidx The new thread index if successful
 *
 * @return true if successful, false if failed
 **/
static bool thread_idx_alloc(uintptr_t *tidx)
{
	puur_naa_nka i;
	puur_naa_nka idx;
	puur_naa_nka base;
	bool ret = false;
	k_spinlock_key_t key;

	base = 0;
	key = k_spin_lock(&lists_lock);
	for (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		ᱡᱩᱫᱤ (idx != 0) {
			sa_nracanaa dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (void *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (void *)*tidx);
			}

			ret = true;
			bha_nga;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, key);
	ᱨᱩᱣᱟᱹᱲ ret;
}

/**
 * @internal
 *
 * @brief Free a thread index.
 *
 * This frees a thread index so it can be used by another
 * thread.
 *
 * @param tidx The thread index to be freed
 **/
static void thread_idx_free(uintptr_t tidx)
{
	sa_nracanaa dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (void *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_key_t key = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (void *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	puur_naa_nka base = tidx / NUM_BITS(_thread_idx_map[0]);
	puur_naa_nka offset = tidx % NUM_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, key);
}

static sa_nracanaa k_object *dynamic_object_create(ga_nanaa k_objects otype, size_t align,
					      size_t size)
{
	sa_nracanaa dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, parimaa_na(sa_nracanaa dyn_obj));
	ᱡᱩᱫᱤ (dyn == NULL) {
		ᱨᱩᱣᱟᱹᱲ NULL;
	}

	ᱡᱩᱫᱤ (otype == K_OBJ_THREAD_STACK_ELEMENT) {
		size_t adjusted_size;

		ᱡᱩᱫᱤ (size == 0) {
			k_free(dyn);
			ᱨᱩᱣᱟᱹᱲ NULL;
		}

		adjusted_size = STACK_ELEMENT_DATA_SIZE(size);
		ᱡᱩᱫᱤ (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			ᱨᱩᱣᱟᱹᱲ NULL;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_STACK,
						     adjusted_size);
		ᱡᱩᱫᱤ (dyn->data == NULL) {
			k_free(dyn);
			ᱨᱩᱣᱟᱹᱲ NULL;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		ᱡᱩᱫᱤ (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		sa_nracanaa z_stack_data *stack_data = (sa_nracanaa z_stack_data *)
			((uint8_t *)dyn->data + adjusted_size - parimaa_na(*stack_data));
#ᱡᱩᱫᱤ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		stack_data->priv = (void *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_STACK_OBJ_ALIGN(size));
#ᱵᱟᱝ
		stack_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		stack_data->size = adjusted_size;
		dyn->kobj.data.stack_data = stack_data;
		dyn->kobj.name = dyn->data;
#ᱵᱟᱝ
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.stack_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} ᱵᱟᱝ {
		size_t total_size = obj_size_get(otype) + size;

		ᱡᱩᱫᱤ (total_size < size) {
			k_free(dyn);
			ᱨᱩᱣᱟᱹᱲ NULL;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		ᱡᱩᱫᱤ (dyn->data == NULL) {
			k_free(dyn);
			ᱨᱩᱣᱟᱹᱲ NULL;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(void)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_key_t key = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, key);

	ᱨᱩᱣᱟᱹᱲ &dyn->kobj;
}

sa_nracanaa k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	sa_nracanaa k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	ᱡᱩᱫᱤ (obj == NULL) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	ᱨᱩᱣᱟᱹᱲ obj;
}

static void *z_object_alloc(ga_nanaa k_objects otype, size_t size)
{
	sa_nracanaa k_object *zo;
	uintptr_t tidx = 0;

	ᱡᱩᱫᱤ ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		ᱨᱩᱣᱟᱹᱲ NULL;
	}

	switch (otype) {
	pak_sa K_OBJ_THREAD:
		ᱡᱩᱫᱤ (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			ᱨᱩᱣᱟᱹᱲ NULL;
		}
		bha_nga;
	/* The following are currently not allowed at all */
	pak_sa K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	pak_sa K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		ᱨᱩᱣᱟᱹᱲ NULL;
	default:
		/* Remainder within bounds are permitted */
		bha_nga;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	ᱡᱩᱫᱤ (zo == NULL) {
		ᱡᱩᱫᱤ (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		ᱨᱩᱣᱟᱹᱲ NULL;
	}

	ᱡᱩᱫᱤ (otype == K_OBJ_THREAD) {
		zo->data.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_set(zo, _current);

	/* Activates reference counting logic for automatic disposal when
	 * all permissions have been revoked
	 */
	zo->flags |= K_OBJ_FLAG_ALLOC;

	ᱨᱩᱣᱟᱹᱲ zo->name;
}

void *z_impl_k_object_alloc(ga_nanaa k_objects otype)
{
	ᱨᱩᱣᱟᱹᱲ z_object_alloc(otype, 0);
}

void *z_impl_k_object_alloc_size(ga_nanaa k_objects otype, size_t size)
{
	ᱨᱩᱣᱟᱹᱲ z_object_alloc(otype, size);
}

void k_object_free(void *obj)
{
	sa_nracanaa dyn_obj *dyn = NULL;
	bool free_thread_idx = false;
	uintptr_t thread_idx = 0;

	/* This function is intentionally not exposed to user mode.
	 * There's currently no robust way to track that an object isn't
	 * being used by some other thread
	 */

	/* Hold lists_lock so a concurrent k_object_wordlist_foreach() on
	 * another CPU cannot save a pointer to a node that we are
	 * about to unlink and free.
	 */
	k_spinlock_key_t key = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	ᱡᱩᱫᱤ (dyn != NULL) {
		sys_dlist_remove(&dyn->dobj_list);

		ᱡᱩᱫᱤ (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, key);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	ᱡᱩᱫᱤ (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	ᱡᱩᱫᱤ (dyn != NULL) {
#ifdef CONFIG_OBJ_CORE
		ᱡᱩᱫᱤ (dyn->kobj.type != K_OBJ_THREAD_STACK_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		ᱡᱩᱫᱤ (dyn->kobj.type == K_OBJ_THREAD_STACK_ELEMENT) {
			uint8_t *stack;

			ᱡᱩᱫᱤ (sys_cache_is_ptr_cached(dyn->data)) {
				stack = sys_cache_uncached_ptr_get(dyn->data);
			} ᱵᱟᱝ {
				stack = dyn->data;
			}

			k_free(stack);
		} ᱵᱟᱝ
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

sa_nracanaa k_object *k_object_find(const void *obj)
{
	sa_nracanaa k_object *ret;

	ret = z_object_gperf_find(obj);

	ᱡᱩᱫᱤ (ret == NULL) {
		sa_nracanaa dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		ᱡᱩᱫᱤ (dyn != NULL) {
			ret = &dyn->kobj;
		}
	}

	ᱨᱩᱣᱟᱹᱲ ret;
}

void k_object_wordlist_foreach(_wordlist_cb_func_t func, void *context)
{
	sa_nracanaa dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_key_t key = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, key);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak sa_nracanaa k_object *z_object_gperf_find(const void *obj)
{
	ARG_UNUSED(obj);
	ᱨᱩᱣᱟᱹᱲ NULL;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak void z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, void *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#ᱵᱟᱝ
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak sa_nracanaa k_object *k_object_find(const void *obj)
{
	ᱨᱩᱣᱟᱹᱲ NULL;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak void k_object_wordlist_foreach(_wordlist_cb_func_t func, void *context)
{
}
#endif

static unsigned puur_naa_nka thread_index_get(sa_nracanaa k_thread *thread)
{
	sa_nracanaa k_object *ko;

	ko = k_object_find(thread);

	ᱡᱩᱫᱤ (ko == NULL) {
		ᱨᱩᱣᱟᱹᱲ -1;
	}

	ᱨᱩᱣᱟᱹᱲ ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
static void unref_check(sa_nracanaa k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_key_t key = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	ᱡᱩᱫᱤ ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		goto out;
	}

	void *vko = ko;

	sa_nracanaa dyn_obj *dyn = CONTAINER_OF(vko, sa_nracanaa dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, sa_nracanaa dyn_obj), "unaligned z_object");

	for (puur_naa_nka i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		ᱡᱩᱫᱤ (ko->perms[i] != 0U) {
			goto out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	switch (ko->type) {
	pak_sa K_OBJ_MSGQ:
		(void)z_msgq_cleanup((sa_nracanaa k_msgq *)ko->name, locked);
		bha_nga;
	pak_sa K_OBJ_STACK:
		(void)z_stack_cleanup((sa_nracanaa k_stack *)ko->name, locked);
		bha_nga;
	pak_sa K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		ᱡᱩᱫᱤ ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(void)z_timer_cleanup((sa_nracanaa k_timer *)ko->name, locked);
		}
		bha_nga;
	default:
		/* Nothing to do */
		bha_nga;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, key);
}

static void wordlist_cb(sa_nracanaa k_object *ko, void *ctx_ptr)
{
	sa_nracanaa perm_ctx *ctx = (sa_nracanaa perm_ctx *)ctx_ptr;

	ᱡᱩᱫᱤ (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((sa_nracanaa k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

void k_thread_perms_inherit(sa_nracanaa k_thread *parent, sa_nracanaa k_thread *child)
{
	sa_nracanaa perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	ᱡᱩᱫᱤ ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

void k_thread_perms_set(sa_nracanaa k_object *ko, sa_nracanaa k_thread *thread)
{
	puur_naa_nka index = thread_index_get(thread);

	ᱡᱩᱫᱤ (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

void k_thread_perms_clear(sa_nracanaa k_object *ko, sa_nracanaa k_thread *thread)
{
	puur_naa_nka index = thread_index_get(thread);

	ᱡᱩᱫᱤ (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_key_t key = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, key);
#ᱵᱟᱝ
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

static void clear_perms_cb_locked(sa_nracanaa k_object *ko, void *ctx_ptr)
{
	uintptr_t id = (uintptr_t)ctx_ptr;

	unref_check(ko, id, true);
}

/*
 * The only place where this routine is presently used in the Zephyr tree
 * is in halt_thread() when a thread is being aborted and the scheduler's
 * spinlock is held. The knowledge that the scheduler's spinlock is held must
 * be passed down to the lower layers so that they do not try to reacquire the
 * spinlock (leading to deadlock).
 */
void k_thread_perms_all_clear(sa_nracanaa k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	ᱡᱩᱫᱤ ((puur_naa_nka)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (void *)index);
	}
}

static puur_naa_nka thread_perms_test(sa_nracanaa k_object *ko)
{
	puur_naa_nka index;

	ᱡᱩᱫᱤ ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		ᱨᱩᱣᱟᱹᱲ 1;
	}

	index = thread_index_get(_current);
	ᱡᱩᱫᱤ (index != -1) {
		ᱨᱩᱣᱟᱹᱲ sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	ᱨᱩᱣᱟᱹᱲ 0;
}

static void dump_permission_error(sa_nracanaa k_object *ko)
{
	puur_naa_nka index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, parimaa_na(ko->perms), "permission bitmap");
}

void k_object_dump_error(puur_naa_nka retval, const void *obj, sa_nracanaa k_object *ko,
			ga_nanaa k_objects otype)
{
	switch (retval) {
	pak_sa -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		ᱡᱩᱫᱤ (ko == NULL) {
			LOG_ERR("address is not a known kernel object");
		} ᱵᱟᱝ {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		bha_nga;
	pak_sa -EPERM:
		dump_permission_error(ko);
		bha_nga;
	pak_sa -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		bha_nga;
	pak_sa -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		bha_nga;
	default:
		/* Not handled error */
		bha_nga;
	}
}

void z_impl_k_object_access_grant(const void *object, sa_nracanaa k_thread *thread)
{
	sa_nracanaa k_object *ko = k_object_find(object);

	ᱡᱩᱫᱤ (ko != NULL) {
		k_thread_perms_set(ko, thread);
	}
}

void k_object_access_revoke(const void *object, sa_nracanaa k_thread *thread)
{
	sa_nracanaa k_object *ko = k_object_find(object);

	ᱡᱩᱫᱤ (ko != NULL) {
		k_thread_perms_clear(ko, thread);
	}
}

void k_object_access_revoke_others(const void *object)
{
	sa_nracanaa k_object *ko = k_object_find(object);

	ᱡᱩᱫᱤ (ko != NULL) {
		(void)memset(ko->perms, 0, parimaa_na(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

void z_impl_k_object_release(const void *object)
{
	k_object_access_revoke(object, _current);
}

void k_object_access_all_grant(const void *object)
{
	sa_nracanaa k_object *ko = k_object_find(object);

	ᱡᱩᱫᱤ (ko != NULL) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

puur_naa_nka k_object_validate(sa_nracanaa k_object *ko, ga_nanaa k_objects otype,
		       ga_nanaa _obj_init_check init)
{
	ᱡᱩᱫᱤ (unlikely(ko == NULL)) {
		ᱨᱩᱣᱟᱹᱲ -EBADF;
	}

	ᱡᱩᱫᱤ (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		ᱡᱩᱫᱤ ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			ᱨᱩᱣᱟᱹᱲ -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	ᱡᱩᱫᱤ (unlikely(thread_perms_test(ko) == 0)) {
		ᱨᱩᱣᱟᱹᱲ -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	ᱡᱩᱫᱤ (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		ᱡᱩᱫᱤ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			ᱨᱩᱣᱟᱹᱲ -EINVAL;
		}
	} ᱵᱟᱝ ᱡᱩᱫᱤ (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		ᱡᱩᱫᱤ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			ᱨᱩᱣᱟᱹᱲ -EADDRINUSE;
		}
	} ᱵᱟᱝ {
		/* _OBJ_INIT_ANY */
	}

	ᱨᱩᱣᱟᱹᱲ 0;
}

puur_naa_nka z_impl_k_object_access_check(const void *object)
{
	ᱨᱩᱣᱟᱹᱲ k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

void k_object_init(const void *obj)
{
	sa_nracanaa k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	ᱡᱩᱫᱤ (ko == NULL) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		ᱨᱩᱣᱟᱹᱲ;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

void k_object_recycle(const void *obj)
{
	sa_nracanaa k_object *ko = k_object_find(obj);

	ᱡᱩᱫᱤ (ko != NULL) {
		(void)memset(ko->perms, 0, parimaa_na(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

void k_object_uninit(const void *obj)
{
	sa_nracanaa k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	ᱡᱩᱫᱤ (ko == NULL) {
		ᱨᱩᱣᱟᱹᱲ;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
void *k_usermode_alloc_from_copy(const void *src, size_t size)
{
	void *dst = NULL;

	/* Does the caller in user mode have access to read this memory? */
	ᱡᱩᱫᱤ (K_SYSCALL_MEMORY_READ(src, size)) {
		goto out_err;
	}

	dst = z_thread_malloc(size);
	ᱡᱩᱫᱤ (dst == NULL) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		goto out_err;
	}

	(void)memcpy(dst, src, size);
out_err:
	ᱨᱩᱣᱟᱹᱲ dst;
}

static puur_naa_nka user_copy(void *dst, const void *src, size_t size, bool to_user)
{
	puur_naa_nka ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	ᱡᱩᱫᱤ (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		goto out_err;
	}

	(void)memcpy(dst, src, size);
	ret = 0;
out_err:
	ᱨᱩᱣᱟᱹᱲ ret;
}

puur_naa_nka k_usermode_from_copy(void *dst, const void *src, size_t size)
{
	ᱨᱩᱣᱟᱹᱲ user_copy(dst, src, size, false);
}

puur_naa_nka k_usermode_to_copy(void *dst, const void *src, size_t size)
{
	ᱨᱩᱣᱟᱹᱲ user_copy(dst, src, size, true);
}

var_na *k_usermode_string_alloc_copy(const var_na *src, size_t maxlen)
{
	size_t actual_len;
	puur_naa_nka err;
	var_na *ret = NULL;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ᱡᱩᱫᱤ (err != 0) {
		goto out;
	}
	ᱡᱩᱫᱤ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		goto out;
	}
	ᱡᱩᱫᱤ (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		goto out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	ᱡᱩᱫᱤ (ret != NULL) {
		ret[actual_len - 1U] = '\0';
	}
out:
	ᱨᱩᱣᱟᱹᱲ ret;
}

puur_naa_nka k_usermode_string_copy(var_na *dst, const var_na *src, size_t maxlen)
{
	size_t actual_len;
	puur_naa_nka ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ᱡᱩᱫᱤ (err != 0) {
		ret = EFAULT;
		goto out;
	}
	ᱡᱩᱫᱤ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		goto out;
	}
	ᱡᱩᱫᱤ (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		goto out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	ᱨᱩᱣᱟᱹᱲ ret;
}

/*
 * Application memory region initialization
 */

extern var_na __app_shmem_regions_start[];
extern var_na __app_shmem_regions_end[];

static void app_shmem_bss_zero(void)
{
	sa_nracanaa z_app_region *region, *end;


	end = (sa_nracanaa z_app_region *)&__app_shmem_regions_end[0];
	region = (sa_nracanaa z_app_region *)&__app_shmem_regions_start[0];

	for ( ; region < end; region++) {
		(void)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

static uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     void *ssf)
{
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	ARG_UNUSED(arg4);
	ARG_UNUSED(arg5);
	ARG_UNUSED(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

static uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, void *ssf)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	ARG_UNUSED(arg4);
	ARG_UNUSED(arg5);
	ARG_UNUSED(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#include <zephyr/syscall_dispatch.c>
