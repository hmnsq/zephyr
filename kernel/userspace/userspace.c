/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/sys/rb.h>
#അന്തര്ഭാവ <zephyr/sys/sys_io.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/syscall.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/device.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <zephyr/app_memory/app_memdomain.h>
#അന്തര്ഭാവ <zephyr/sys/libc-hooks.h>
#അന്തര്ഭാവ <zephyr/sys/mutex.h>
#അന്തര്ഭാവ <zephyr/sys/util.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <zephyr/linker/linker-defs.h>
#അന്തര്ഭാവ <zephyr/cache.h>
#അന്തര്ഭാവ <kernel_internal.h>

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

#അന്തര്ഭാവ <zephyr/logging/log.h>
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
സ്ഥിതം ഘടന k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#എങ്കിൽ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define അട്ടി_ELEMENT_DATA_SIZE(size) \
	(വലുപ്പം(ഘടന z_അട്ടി_data) + CONFIG_PRIVILEGED_അട്ടി_SIZE + \
	Z_THREAD_അട്ടി_OBJ_ALIGN(size) + K_THREAD_അട്ടി_LEN(size))
#അല്ലെങ്കിൽ
#define അട്ടി_ELEMENT_DATA_SIZE(size) (വലുപ്പം(ഘടന z_അട്ടി_data) + \
	K_THREAD_അട്ടി_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#അല്ലെങ്കിൽ
#define അട്ടി_ELEMENT_DATA_SIZE(size) K_THREAD_അട്ടി_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
സ്ഥിതം ഘടന k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
ബാഹ്യ uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

ബാഹ്യ പൂർണ്ണസംഖ്യ z_msgq_cleanup(ഘടന k_msgq *q, bool locked);
ബാഹ്യ പൂർണ്ണസംഖ്യ z_അട്ടി_cleanup(ഘടന k_അട്ടി *അട്ടി, bool locked);
ബാഹ്യ പൂർണ്ണസംഖ്യ z_timer_cleanup(ഘടന k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

സ്ഥിതം ശൂന്യം unref_check(ഘടന k_object *ko, uintptr_t index, bool locked);

സ്ഥിരം അക്ഷരം *otype_to_str(എണ്ണൽ k_objects otype)
{
	സ്ഥിരം അക്ഷരം *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	തിരഞ്ഞെടുപ്പ് (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	അവസ്ഥ K_OBJ_ANY:
		ret = "generic";
		നിർത്തുക;
	അവസ്ഥ K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		നിർത്തുക;
#അന്തര്ഭാവ <zephyr/otype-to-str.h>
	സ്ഥിരസ്ഥിതി:
		ret = "?";
		നിർത്തുക;
	}
#അല്ലെങ്കിൽ
	ARG_UNUSED(otype);
	ret = ശൂന്യ;
#endif /* CONFIG_LOG */
	തിരികെ ret;
}

ഘടന perm_ctx {
	പൂർണ്ണസംഖ്യ parent_id;
	പൂർണ്ണസംഖ്യ child_id;
	ഘടന k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_അട്ടി_find(k_thread_അട്ടി_t *അട്ടി)
{
	ഘടന k_object *obj = k_object_find(അട്ടി);

	__ASSERT(obj != ശൂന്യ, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_അട്ടി_ELEMENT,
		 "bad stack object");

	തിരികെ obj->data.അട്ടി_data->priv;
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
#അല്ലെങ്കിൽ
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(വലുപ്പം(ശൂന്യം *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_അട്ടി_SIZE
#എങ്കിൽ defined(CONFIG_MPU_അട്ടി_GUARD) || defined(CONFIG_PMP_അട്ടി_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_അട്ടി \
	Z_THREAD_അട്ടി_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_അട്ടി_SIZE)
#അല്ലെങ്കിൽ
#define DYN_OBJ_DATA_ALIGN_K_THREAD_അട്ടി \
	Z_THREAD_അട്ടി_OBJ_ALIGN(CONFIG_PRIVILEGED_അട്ടി_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#അല്ലെങ്കിൽ
#define DYN_OBJ_DATA_ALIGN_K_THREAD_അട്ടി \
	Z_THREAD_അട്ടി_OBJ_ALIGN(ARCH_അട്ടി_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (വലുപ്പം(ശൂന്യം *)))

ഘടന dyn_obj {
	ഘടന k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	ശൂന്യം *data;
};

ബാഹ്യ ഘടന k_object *z_object_gperf_find(സ്ഥിരം ശൂന്യം *obj);
ബാഹ്യ ശൂന്യം z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     ശൂന്യം *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
സ്ഥിതം sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

സ്ഥിതം size_t obj_size_get(എണ്ണൽ k_objects otype)
{
	size_t ret;

	തിരഞ്ഞെടുപ്പ് (otype) {
#അന്തര്ഭാവ <zephyr/otype-to-size.h>
	സ്ഥിരസ്ഥിതി:
		ret = വലുപ്പം(സ്ഥിരം ഘടന device);
		നിർത്തുക;
	}

	തിരികെ ret;
}

സ്ഥിതം size_t obj_align_get(എണ്ണൽ k_objects otype)
{
	size_t ret;

	തിരഞ്ഞെടുപ്പ് (otype) {
	അവസ്ഥ K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#അല്ലെങ്കിൽ
		ret = __alignof(ഘടന dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		ret = __alignof(ഘടന dyn_obj);
		നിർത്തുക;
	}

	തിരികെ ret;
}

സ്ഥിതം ഘടന dyn_obj *dyn_object_find_locked(സ്ഥിരം ശൂന്യം *obj)
{
	ഘടന dyn_obj *കണ്ണി;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, കണ്ണി, dobj_list) {
		എങ്കിൽ (കണ്ണി->kobj.name == obj) {
			തിരികെ കണ്ണി;
		}
	}

	തിരികെ ശൂന്യ;
}

സ്ഥിതം ഘടന dyn_obj *dyn_object_find(സ്ഥിരം ശൂന്യം *obj)
{
	ഘടന dyn_obj *കണ്ണി;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	താക്കോൽ = k_spin_lock(&lists_lock);
	കണ്ണി = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, താക്കോൽ);

	തിരികെ കണ്ണി;
}

സ്ഥിതം ശൂന്യം clear_perms_cb(ഘടന k_object *ko, ശൂന്യം *ctx_ptr)
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
സ്ഥിതം bool thread_idx_alloc(uintptr_t *tidx)
{
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ idx;
	പൂർണ്ണസംഖ്യ base;
	bool ret = false;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	base = 0;
	താക്കോൽ = k_spin_lock(&lists_lock);
	വേണ്ടി (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		എങ്കിൽ (idx != 0) {
			ഘടന dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (ശൂന്യം *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (ശൂന്യം *)*tidx);
			}

			ret = true;
			നിർത്തുക;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, താക്കോൽ);
	തിരികെ ret;
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
സ്ഥിതം ശൂന്യം thread_idx_free(uintptr_t tidx)
{
	ഘടന dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (ശൂന്യം *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (ശൂന്യം *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	പൂർണ്ണസംഖ്യ base = tidx / അക്കം_BITS(_thread_idx_map[0]);
	പൂർണ്ണസംഖ്യ offset = tidx % അക്കം_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, താക്കോൽ);
}

സ്ഥിതം ഘടന k_object *dynamic_object_create(എണ്ണൽ k_objects otype, size_t align,
					      size_t size)
{
	ഘടന dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, വലുപ്പം(ഘടന dyn_obj));
	എങ്കിൽ (dyn == ശൂന്യ) {
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (otype == K_OBJ_THREAD_അട്ടി_ELEMENT) {
		size_t adjusted_size;

		എങ്കിൽ (size == 0) {
			k_free(dyn);
			തിരികെ ശൂന്യ;
		}

		adjusted_size = അട്ടി_ELEMENT_DATA_SIZE(size);
		എങ്കിൽ (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			തിരികെ ശൂന്യ;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_അട്ടി,
						     adjusted_size);
		എങ്കിൽ (dyn->data == ശൂന്യ) {
			k_free(dyn);
			തിരികെ ശൂന്യ;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_അട്ടി_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		എങ്കിൽ (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		ഘടന z_അട്ടി_data *അട്ടി_data = (ഘടന z_അട്ടി_data *)
			((uint8_t *)dyn->data + adjusted_size - വലുപ്പം(*അട്ടി_data));
#എങ്കിൽ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		അട്ടി_data->priv = (ശൂന്യം *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_അട്ടി_OBJ_ALIGN(size));
#അല്ലെങ്കിൽ
		അട്ടി_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		അട്ടി_data->size = adjusted_size;
		dyn->kobj.data.അട്ടി_data = അട്ടി_data;
		dyn->kobj.name = dyn->data;
#അല്ലെങ്കിൽ
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.അട്ടി_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} അല്ലെങ്കിൽ {
		size_t total_size = obj_size_get(otype) + size;

		എങ്കിൽ (total_size < size) {
			k_free(dyn);
			തിരികെ ശൂന്യ;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		എങ്കിൽ (dyn->data == ശൂന്യ) {
			k_free(dyn);
			തിരികെ ശൂന്യ;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(ശൂന്യം)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, താക്കോൽ);

	തിരികെ &dyn->kobj;
}

ഘടന k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	ഘടന k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	എങ്കിൽ (obj == ശൂന്യ) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	തിരികെ obj;
}

സ്ഥിതം ശൂന്യം *z_object_alloc(എണ്ണൽ k_objects otype, size_t size)
{
	ഘടന k_object *zo;
	uintptr_t tidx = 0;

	എങ്കിൽ ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		തിരികെ ശൂന്യ;
	}

	തിരഞ്ഞെടുപ്പ് (otype) {
	അവസ്ഥ K_OBJ_THREAD:
		എങ്കിൽ (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			തിരികെ ശൂന്യ;
		}
		നിർത്തുക;
	/* The following are currently not allowed at all */
	അവസ്ഥ K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	അവസ്ഥ K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		തിരികെ ശൂന്യ;
	സ്ഥിരസ്ഥിതി:
		/* Remainder within bounds are permitted */
		നിർത്തുക;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	എങ്കിൽ (zo == ശൂന്യ) {
		എങ്കിൽ (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (otype == K_OBJ_THREAD) {
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

	തിരികെ zo->name;
}

ശൂന്യം *z_impl_k_object_alloc(എണ്ണൽ k_objects otype)
{
	തിരികെ z_object_alloc(otype, 0);
}

ശൂന്യം *z_impl_k_object_alloc_size(എണ്ണൽ k_objects otype, size_t size)
{
	തിരികെ z_object_alloc(otype, size);
}

ശൂന്യം k_object_free(ശൂന്യം *obj)
{
	ഘടന dyn_obj *dyn = ശൂന്യ;
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
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	എങ്കിൽ (dyn != ശൂന്യ) {
		sys_dlist_remove(&dyn->dobj_list);

		എങ്കിൽ (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, താക്കോൽ);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	എങ്കിൽ (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	എങ്കിൽ (dyn != ശൂന്യ) {
#ifdef CONFIG_OBJ_CORE
		എങ്കിൽ (dyn->kobj.type != K_OBJ_THREAD_അട്ടി_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_അട്ടി_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		എങ്കിൽ (dyn->kobj.type == K_OBJ_THREAD_അട്ടി_ELEMENT) {
			uint8_t *അട്ടി;

			എങ്കിൽ (sys_cache_is_ptr_cached(dyn->data)) {
				അട്ടി = sys_cache_uncached_ptr_get(dyn->data);
			} അല്ലെങ്കിൽ {
				അട്ടി = dyn->data;
			}

			k_free(അട്ടി);
		} അല്ലെങ്കിൽ
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

ഘടന k_object *k_object_find(സ്ഥിരം ശൂന്യം *obj)
{
	ഘടന k_object *ret;

	ret = z_object_gperf_find(obj);

	എങ്കിൽ (ret == ശൂന്യ) {
		ഘടന dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		എങ്കിൽ (dyn != ശൂന്യ) {
			ret = &dyn->kobj;
		}
	}

	തിരികെ ret;
}

ശൂന്യം k_object_wordlist_foreach(_wordlist_cb_func_t func, ശൂന്യം *context)
{
	ഘടന dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, താക്കോൽ);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ഘടന k_object *z_object_gperf_find(സ്ഥിരം ശൂന്യം *obj)
{
	ARG_UNUSED(obj);
	തിരികെ ശൂന്യ;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ശൂന്യം z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, ശൂന്യം *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#അല്ലെങ്കിൽ
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ഘടന k_object *k_object_find(സ്ഥിരം ശൂന്യം *obj)
{
	തിരികെ ശൂന്യ;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ശൂന്യം k_object_wordlist_foreach(_wordlist_cb_func_t func, ശൂന്യം *context)
{
}
#endif

സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ thread_index_get(ഘടന k_thread *thread)
{
	ഘടന k_object *ko;

	ko = k_object_find(thread);

	എങ്കിൽ (ko == ശൂന്യ) {
		തിരികെ -1;
	}

	തിരികെ ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
സ്ഥിതം ശൂന്യം unref_check(ഘടന k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	എങ്കിൽ ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		പോകുക out;
	}

	ശൂന്യം *vko = ko;

	ഘടന dyn_obj *dyn = CONTAINER_OF(vko, ഘടന dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, ഘടന dyn_obj), "unaligned z_object");

	വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		എങ്കിൽ (ko->perms[i] != 0U) {
			പോകുക out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	തിരഞ്ഞെടുപ്പ് (ko->type) {
	അവസ്ഥ K_OBJ_MSGQ:
		(ശൂന്യം)z_msgq_cleanup((ഘടന k_msgq *)ko->name, locked);
		നിർത്തുക;
	അവസ്ഥ K_OBJ_അട്ടി:
		(ശൂന്യം)z_അട്ടി_cleanup((ഘടന k_അട്ടി *)ko->name, locked);
		നിർത്തുക;
	അവസ്ഥ K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		എങ്കിൽ ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(ശൂന്യം)z_timer_cleanup((ഘടന k_timer *)ko->name, locked);
		}
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		/* Nothing to do */
		നിർത്തുക;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, താക്കോൽ);
}

സ്ഥിതം ശൂന്യം wordlist_cb(ഘടന k_object *ko, ശൂന്യം *ctx_ptr)
{
	ഘടന perm_ctx *ctx = (ഘടന perm_ctx *)ctx_ptr;

	എങ്കിൽ (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((ഘടന k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

ശൂന്യം k_thread_perms_inherit(ഘടന k_thread *parent, ഘടന k_thread *child)
{
	ഘടന perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	എങ്കിൽ ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

ശൂന്യം k_thread_perms_set(ഘടന k_object *ko, ഘടന k_thread *thread)
{
	പൂർണ്ണസംഖ്യ index = thread_index_get(thread);

	എങ്കിൽ (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

ശൂന്യം k_thread_perms_clear(ഘടന k_object *ko, ഘടന k_thread *thread)
{
	പൂർണ്ണസംഖ്യ index = thread_index_get(thread);

	എങ്കിൽ (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, താക്കോൽ);
#അല്ലെങ്കിൽ
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

സ്ഥിതം ശൂന്യം clear_perms_cb_locked(ഘടന k_object *ko, ശൂന്യം *ctx_ptr)
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
ശൂന്യം k_thread_perms_all_clear(ഘടന k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	എങ്കിൽ ((പൂർണ്ണസംഖ്യ)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (ശൂന്യം *)index);
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ thread_perms_test(ഘടന k_object *ko)
{
	പൂർണ്ണസംഖ്യ index;

	എങ്കിൽ ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		തിരികെ 1;
	}

	index = thread_index_get(_current);
	എങ്കിൽ (index != -1) {
		തിരികെ sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം dump_permission_error(ഘടന k_object *ko)
{
	പൂർണ്ണസംഖ്യ index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, വലുപ്പം(ko->perms), "permission bitmap");
}

ശൂന്യം k_object_dump_error(പൂർണ്ണസംഖ്യ retval, സ്ഥിരം ശൂന്യം *obj, ഘടന k_object *ko,
			എണ്ണൽ k_objects otype)
{
	തിരഞ്ഞെടുപ്പ് (retval) {
	അവസ്ഥ -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		എങ്കിൽ (ko == ശൂന്യ) {
			LOG_ERR("address is not a known kernel object");
		} അല്ലെങ്കിൽ {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		നിർത്തുക;
	അവസ്ഥ -EPERM:
		dump_permission_error(ko);
		നിർത്തുക;
	അവസ്ഥ -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		നിർത്തുക;
	അവസ്ഥ -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		/* Not handled error */
		നിർത്തുക;
	}
}

ശൂന്യം z_impl_k_object_access_grant(സ്ഥിരം ശൂന്യം *object, ഘടന k_thread *thread)
{
	ഘടന k_object *ko = k_object_find(object);

	എങ്കിൽ (ko != ശൂന്യ) {
		k_thread_perms_set(ko, thread);
	}
}

ശൂന്യം k_object_access_revoke(സ്ഥിരം ശൂന്യം *object, ഘടന k_thread *thread)
{
	ഘടന k_object *ko = k_object_find(object);

	എങ്കിൽ (ko != ശൂന്യ) {
		k_thread_perms_clear(ko, thread);
	}
}

ശൂന്യം k_object_access_revoke_others(സ്ഥിരം ശൂന്യം *object)
{
	ഘടന k_object *ko = k_object_find(object);

	എങ്കിൽ (ko != ശൂന്യ) {
		(ശൂന്യം)memset(ko->perms, 0, വലുപ്പം(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

ശൂന്യം z_impl_k_object_release(സ്ഥിരം ശൂന്യം *object)
{
	k_object_access_revoke(object, _current);
}

ശൂന്യം k_object_access_all_grant(സ്ഥിരം ശൂന്യം *object)
{
	ഘടന k_object *ko = k_object_find(object);

	എങ്കിൽ (ko != ശൂന്യ) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

പൂർണ്ണസംഖ്യ k_object_validate(ഘടന k_object *ko, എണ്ണൽ k_objects otype,
		       എണ്ണൽ _obj_init_check init)
{
	എങ്കിൽ (unlikely(ko == ശൂന്യ)) {
		തിരികെ -EBADF;
	}

	എങ്കിൽ (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		എങ്കിൽ ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			തിരികെ -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	എങ്കിൽ (unlikely(thread_perms_test(ko) == 0)) {
		തിരികെ -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	എങ്കിൽ (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		എങ്കിൽ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			തിരികെ -EINVAL;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		എങ്കിൽ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			തിരികെ -EADDRINUSE;
		}
	} അല്ലെങ്കിൽ {
		/* _OBJ_INIT_ANY */
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_impl_k_object_access_check(സ്ഥിരം ശൂന്യം *object)
{
	തിരികെ k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

ശൂന്യം k_object_init(സ്ഥിരം ശൂന്യം *obj)
{
	ഘടന k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	എങ്കിൽ (ko == ശൂന്യ) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		തിരികെ;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

ശൂന്യം k_object_recycle(സ്ഥിരം ശൂന്യം *obj)
{
	ഘടന k_object *ko = k_object_find(obj);

	എങ്കിൽ (ko != ശൂന്യ) {
		(ശൂന്യം)memset(ko->perms, 0, വലുപ്പം(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

ശൂന്യം k_object_uninit(സ്ഥിരം ശൂന്യം *obj)
{
	ഘടന k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	എങ്കിൽ (ko == ശൂന്യ) {
		തിരികെ;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
ശൂന്യം *k_usermode_alloc_from_copy(സ്ഥിരം ശൂന്യം *src, size_t size)
{
	ശൂന്യം *dst = ശൂന്യ;

	/* Does the caller in user mode have access to read this memory? */
	എങ്കിൽ (K_SYSCALL_MEMORY_READ(src, size)) {
		പോകുക out_err;
	}

	dst = z_thread_malloc(size);
	എങ്കിൽ (dst == ശൂന്യ) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		പോകുക out_err;
	}

	(ശൂന്യം)memcpy(dst, src, size);
out_err:
	തിരികെ dst;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ user_copy(ശൂന്യം *dst, സ്ഥിരം ശൂന്യം *src, size_t size, bool to_user)
{
	പൂർണ്ണസംഖ്യ ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	എങ്കിൽ (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		പോകുക out_err;
	}

	(ശൂന്യം)memcpy(dst, src, size);
	ret = 0;
out_err:
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_usermode_from_copy(ശൂന്യം *dst, സ്ഥിരം ശൂന്യം *src, size_t size)
{
	തിരികെ user_copy(dst, src, size, false);
}

പൂർണ്ണസംഖ്യ k_usermode_to_copy(ശൂന്യം *dst, സ്ഥിരം ശൂന്യം *src, size_t size)
{
	തിരികെ user_copy(dst, src, size, true);
}

അക്ഷരം *k_usermode_string_alloc_copy(സ്ഥിരം അക്ഷരം *src, size_t maxlen)
{
	size_t actual_len;
	പൂർണ്ണസംഖ്യ err;
	അക്ഷരം *ret = ശൂന്യ;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	എങ്കിൽ (err != 0) {
		പോകുക out;
	}
	എങ്കിൽ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		പോകുക out;
	}
	എങ്കിൽ (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		പോകുക out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	എങ്കിൽ (ret != ശൂന്യ) {
		ret[actual_len - 1U] = '\0';
	}
out:
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_usermode_string_copy(അക്ഷരം *dst, സ്ഥിരം അക്ഷരം *src, size_t maxlen)
{
	size_t actual_len;
	പൂർണ്ണസംഖ്യ ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	എങ്കിൽ (err != 0) {
		ret = EFAULT;
		പോകുക out;
	}
	എങ്കിൽ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		പോകുക out;
	}
	എങ്കിൽ (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		പോകുക out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	തിരികെ ret;
}

/*
 * Application memory region initialization
 */

ബാഹ്യ അക്ഷരം __app_shmem_regions_start[];
ബാഹ്യ അക്ഷരം __app_shmem_regions_end[];

സ്ഥിതം ശൂന്യം app_shmem_bss_zero(ശൂന്യം)
{
	ഘടന z_app_region *region, *end;


	end = (ഘടന z_app_region *)&__app_shmem_regions_end[0];
	region = (ഘടന z_app_region *)&__app_shmem_regions_start[0];

	വേണ്ടി ( ; region < end; region++) {
		(ശൂന്യം)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

സ്ഥിതം uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     ശൂന്യം *ssf)
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

സ്ഥിതം uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, ശൂന്യം *ssf)
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

#അന്തര്ഭാവ <zephyr/syscall_dispatch.c>
