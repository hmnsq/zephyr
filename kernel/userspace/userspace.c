/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <zephyr/sys/math_extras.h>
#অন্তর্ভাৱ <zephyr/sys/rb.h>
#অন্তর্ভাৱ <zephyr/sys/sys_io.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <zephyr/syscall.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <zephyr/device.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <zephyr/app_memory/app_memdomain.h>
#অন্তর্ভাৱ <zephyr/sys/libc-hooks.h>
#অন্তর্ভাৱ <zephyr/sys/mutex.h>
#অন্তর্ভাৱ <zephyr/sys/util.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <zephyr/linker/linker-defs.h>
#অন্তর্ভাৱ <zephyr/cache.h>
#অন্তর্ভাৱ <kernel_internal.h>

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

#অন্তর্ভাৱ <zephyr/logging/log.h>
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
স্থির সংরচনা k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#যদি defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define স্তূপ_ELEMENT_DATA_SIZE(size) \
	(পরিমাণ(সংরচনা z_স্তূপ_data) + CONFIG_PRIVILEGED_স্তূপ_SIZE + \
	Z_THREAD_স্তূপ_OBJ_ALIGN(size) + K_THREAD_স্তূপ_LEN(size))
#অন্যথা
#define স্তূপ_ELEMENT_DATA_SIZE(size) (পরিমাণ(সংরচনা z_স্তূপ_data) + \
	K_THREAD_স্তূপ_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#অন্যথা
#define স্তূপ_ELEMENT_DATA_SIZE(size) K_THREAD_স্তূপ_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
স্থির সংরচনা k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
বাহ্য uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

বাহ্য পূর্ণাঙ্ক z_msgq_cleanup(সংরচনা k_msgq *q, bool locked);
বাহ্য পূর্ণাঙ্ক z_স্তূপ_cleanup(সংরচনা k_স্তূপ *স্তূপ, bool locked);
বাহ্য পূর্ণাঙ্ক z_timer_cleanup(সংরচনা k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

স্থির শূন্য unref_check(সংরচনা k_object *ko, uintptr_t index, bool locked);

ধ্রুৱ ৱর্ণ *otype_to_str(গণনা k_objects otype)
{
	ধ্রুৱ ৱর্ণ *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	ৱিকল্প (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	পক্ষ K_OBJ_ANY:
		ret = "generic";
		ভঙ্গ;
	পক্ষ K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		ভঙ্গ;
#অন্তর্ভাৱ <zephyr/otype-to-str.h>
	স্ৱভাৱ:
		ret = "?";
		ভঙ্গ;
	}
#অন্যথা
	ARG_UNUSED(otype);
	ret = রিক্ত;
#endif /* CONFIG_LOG */
	ঘূৰাই ret;
}

সংরচনা perm_ctx {
	পূর্ণাঙ্ক parent_id;
	পূর্ণাঙ্ক child_id;
	সংরচনা k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_স্তূপ_find(k_thread_স্তূপ_t *স্তূপ)
{
	সংরচনা k_object *obj = k_object_find(স্তূপ);

	__ASSERT(obj != রিক্ত, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_স্তূপ_ELEMENT,
		 "bad stack object");

	ঘূৰাই obj->data.স্তূপ_data->priv;
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
#অন্যথা
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(পরিমাণ(শূন্য *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_স্তূপ_SIZE
#যদি defined(CONFIG_MPU_স্তূপ_GUARD) || defined(CONFIG_PMP_স্তূপ_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_স্তূপ \
	Z_THREAD_স্তূপ_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_স্তূপ_SIZE)
#অন্যথা
#define DYN_OBJ_DATA_ALIGN_K_THREAD_স্তূপ \
	Z_THREAD_স্তূপ_OBJ_ALIGN(CONFIG_PRIVILEGED_স্তূপ_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#অন্যথা
#define DYN_OBJ_DATA_ALIGN_K_THREAD_স্তূপ \
	Z_THREAD_স্তূপ_OBJ_ALIGN(ARCH_স্তূপ_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (পরিমাণ(শূন্য *)))

সংরচনা dyn_obj {
	সংরচনা k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	শূন্য *data;
};

বাহ্য সংরচনা k_object *z_object_gperf_find(ধ্রুৱ শূন্য *obj);
বাহ্য শূন্য z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     শূন্য *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
স্থির sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

স্থির size_t obj_size_get(গণনা k_objects otype)
{
	size_t ret;

	ৱিকল্প (otype) {
#অন্তর্ভাৱ <zephyr/otype-to-size.h>
	স্ৱভাৱ:
		ret = পরিমাণ(ধ্রুৱ সংরচনা device);
		ভঙ্গ;
	}

	ঘূৰাই ret;
}

স্থির size_t obj_align_get(গণনা k_objects otype)
{
	size_t ret;

	ৱিকল্প (otype) {
	পক্ষ K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#অন্যথা
		ret = __alignof(সংরচনা dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		ভঙ্গ;
	স্ৱভাৱ:
		ret = __alignof(সংরচনা dyn_obj);
		ভঙ্গ;
	}

	ঘূৰাই ret;
}

স্থির সংরচনা dyn_obj *dyn_object_find_locked(ধ্রুৱ শূন্য *obj)
{
	সংরচনা dyn_obj *গ্ৰন্থি;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, গ্ৰন্থি, dobj_list) {
		যদি (গ্ৰন্থি->kobj.name == obj) {
			ঘূৰাই গ্ৰন্থি;
		}
	}

	ঘূৰাই রিক্ত;
}

স্থির সংরচনা dyn_obj *dyn_object_find(ধ্রুৱ শূন্য *obj)
{
	সংরচনা dyn_obj *গ্ৰন্থি;
	k_spinlock_চাবি_t চাবি;

	চাবি = k_spin_lock(&lists_lock);
	গ্ৰন্থি = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, চাবি);

	ঘূৰাই গ্ৰন্থি;
}

স্থির শূন্য clear_perms_cb(সংরচনা k_object *ko, শূন্য *ctx_ptr)
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
স্থির bool thread_idx_alloc(uintptr_t *tidx)
{
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক idx;
	পূর্ণাঙ্ক base;
	bool ret = false;
	k_spinlock_চাবি_t চাবি;

	base = 0;
	চাবি = k_spin_lock(&lists_lock);
	বাবে (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		যদি (idx != 0) {
			সংরচনা dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (শূন্য *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (শূন্য *)*tidx);
			}

			ret = true;
			ভঙ্গ;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, চাবি);
	ঘূৰাই ret;
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
স্থির শূন্য thread_idx_free(uintptr_t tidx)
{
	সংরচনা dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (শূন্য *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_চাবি_t চাবি = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (শূন্য *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	পূর্ণাঙ্ক base = tidx / অঙ্ক_BITS(_thread_idx_map[0]);
	পূর্ণাঙ্ক offset = tidx % অঙ্ক_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, চাবি);
}

স্থির সংরচনা k_object *dynamic_object_create(গণনা k_objects otype, size_t align,
					      size_t size)
{
	সংরচনা dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, পরিমাণ(সংরচনা dyn_obj));
	যদি (dyn == রিক্ত) {
		ঘূৰাই রিক্ত;
	}

	যদি (otype == K_OBJ_THREAD_স্তূপ_ELEMENT) {
		size_t adjusted_size;

		যদি (size == 0) {
			k_free(dyn);
			ঘূৰাই রিক্ত;
		}

		adjusted_size = স্তূপ_ELEMENT_DATA_SIZE(size);
		যদি (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			ঘূৰাই রিক্ত;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_স্তূপ,
						     adjusted_size);
		যদি (dyn->data == রিক্ত) {
			k_free(dyn);
			ঘূৰাই রিক্ত;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_স্তূপ_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		যদি (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		সংরচনা z_স্তূপ_data *স্তূপ_data = (সংরচনা z_স্তূপ_data *)
			((uint8_t *)dyn->data + adjusted_size - পরিমাণ(*স্তূপ_data));
#যদি defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		স্তূপ_data->priv = (শূন্য *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_স্তূপ_OBJ_ALIGN(size));
#অন্যথা
		স্তূপ_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		স্তূপ_data->size = adjusted_size;
		dyn->kobj.data.স্তূপ_data = স্তূপ_data;
		dyn->kobj.name = dyn->data;
#অন্যথা
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.স্তূপ_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} অন্যথা {
		size_t total_size = obj_size_get(otype) + size;

		যদি (total_size < size) {
			k_free(dyn);
			ঘূৰাই রিক্ত;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		যদি (dyn->data == রিক্ত) {
			k_free(dyn);
			ঘূৰাই রিক্ত;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(শূন্য)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, চাবি);

	ঘূৰাই &dyn->kobj;
}

সংরচনা k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	সংরচনা k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	যদি (obj == রিক্ত) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	ঘূৰাই obj;
}

স্থির শূন্য *z_object_alloc(গণনা k_objects otype, size_t size)
{
	সংরচনা k_object *zo;
	uintptr_t tidx = 0;

	যদি ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		ঘূৰাই রিক্ত;
	}

	ৱিকল্প (otype) {
	পক্ষ K_OBJ_THREAD:
		যদি (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			ঘূৰাই রিক্ত;
		}
		ভঙ্গ;
	/* The following are currently not allowed at all */
	পক্ষ K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	পক্ষ K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		ঘূৰাই রিক্ত;
	স্ৱভাৱ:
		/* Remainder within bounds are permitted */
		ভঙ্গ;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	যদি (zo == রিক্ত) {
		যদি (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		ঘূৰাই রিক্ত;
	}

	যদি (otype == K_OBJ_THREAD) {
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

	ঘূৰাই zo->name;
}

শূন্য *z_impl_k_object_alloc(গণনা k_objects otype)
{
	ঘূৰাই z_object_alloc(otype, 0);
}

শূন্য *z_impl_k_object_alloc_size(গণনা k_objects otype, size_t size)
{
	ঘূৰাই z_object_alloc(otype, size);
}

শূন্য k_object_free(শূন্য *obj)
{
	সংরচনা dyn_obj *dyn = রিক্ত;
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
	k_spinlock_চাবি_t চাবি = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	যদি (dyn != রিক্ত) {
		sys_dlist_remove(&dyn->dobj_list);

		যদি (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, চাবি);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	যদি (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	যদি (dyn != রিক্ত) {
#ifdef CONFIG_OBJ_CORE
		যদি (dyn->kobj.type != K_OBJ_THREAD_স্তূপ_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_স্তূপ_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		যদি (dyn->kobj.type == K_OBJ_THREAD_স্তূপ_ELEMENT) {
			uint8_t *স্তূপ;

			যদি (sys_cache_is_ptr_cached(dyn->data)) {
				স্তূপ = sys_cache_uncached_ptr_get(dyn->data);
			} অন্যথা {
				স্তূপ = dyn->data;
			}

			k_free(স্তূপ);
		} অন্যথা
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

সংরচনা k_object *k_object_find(ধ্রুৱ শূন্য *obj)
{
	সংরচনা k_object *ret;

	ret = z_object_gperf_find(obj);

	যদি (ret == রিক্ত) {
		সংরচনা dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		যদি (dyn != রিক্ত) {
			ret = &dyn->kobj;
		}
	}

	ঘূৰাই ret;
}

শূন্য k_object_wordlist_foreach(_wordlist_cb_func_t func, শূন্য *context)
{
	সংরচনা dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, চাবি);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak সংরচনা k_object *z_object_gperf_find(ধ্রুৱ শূন্য *obj)
{
	ARG_UNUSED(obj);
	ঘূৰাই রিক্ত;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak শূন্য z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, শূন্য *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#অন্যথা
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak সংরচনা k_object *k_object_find(ধ্রুৱ শূন্য *obj)
{
	ঘূৰাই রিক্ত;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak শূন্য k_object_wordlist_foreach(_wordlist_cb_func_t func, শূন্য *context)
{
}
#endif

স্থির unsigned পূর্ণাঙ্ক thread_index_get(সংরচনা k_thread *thread)
{
	সংরচনা k_object *ko;

	ko = k_object_find(thread);

	যদি (ko == রিক্ত) {
		ঘূৰাই -1;
	}

	ঘূৰাই ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
স্থির শূন্য unref_check(সংরচনা k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	যদি ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		গচ্ছ out;
	}

	শূন্য *vko = ko;

	সংরচনা dyn_obj *dyn = CONTAINER_OF(vko, সংরচনা dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, সংরচনা dyn_obj), "unaligned z_object");

	বাবে (পূর্ণাঙ্ক i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		যদি (ko->perms[i] != 0U) {
			গচ্ছ out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	ৱিকল্প (ko->type) {
	পক্ষ K_OBJ_MSGQ:
		(শূন্য)z_msgq_cleanup((সংরচনা k_msgq *)ko->name, locked);
		ভঙ্গ;
	পক্ষ K_OBJ_স্তূপ:
		(শূন্য)z_স্তূপ_cleanup((সংরচনা k_স্তূপ *)ko->name, locked);
		ভঙ্গ;
	পক্ষ K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		যদি ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(শূন্য)z_timer_cleanup((সংরচনা k_timer *)ko->name, locked);
		}
		ভঙ্গ;
	স্ৱভাৱ:
		/* Nothing to do */
		ভঙ্গ;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, চাবি);
}

স্থির শূন্য wordlist_cb(সংরচনা k_object *ko, শূন্য *ctx_ptr)
{
	সংরচনা perm_ctx *ctx = (সংরচনা perm_ctx *)ctx_ptr;

	যদি (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((সংরচনা k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

শূন্য k_thread_perms_inherit(সংরচনা k_thread *parent, সংরচনা k_thread *child)
{
	সংরচনা perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	যদি ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

শূন্য k_thread_perms_set(সংরচনা k_object *ko, সংরচনা k_thread *thread)
{
	পূর্ণাঙ্ক index = thread_index_get(thread);

	যদি (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

শূন্য k_thread_perms_clear(সংরচনা k_object *ko, সংরচনা k_thread *thread)
{
	পূর্ণাঙ্ক index = thread_index_get(thread);

	যদি (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_চাবি_t চাবি = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, চাবি);
#অন্যথা
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

স্থির শূন্য clear_perms_cb_locked(সংরচনা k_object *ko, শূন্য *ctx_ptr)
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
শূন্য k_thread_perms_all_clear(সংরচনা k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	যদি ((পূর্ণাঙ্ক)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (শূন্য *)index);
	}
}

স্থির পূর্ণাঙ্ক thread_perms_test(সংরচনা k_object *ko)
{
	পূর্ণাঙ্ক index;

	যদি ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		ঘূৰাই 1;
	}

	index = thread_index_get(_current);
	যদি (index != -1) {
		ঘূৰাই sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	ঘূৰাই 0;
}

স্থির শূন্য dump_permission_error(সংরচনা k_object *ko)
{
	পূর্ণাঙ্ক index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, পরিমাণ(ko->perms), "permission bitmap");
}

শূন্য k_object_dump_error(পূর্ণাঙ্ক retval, ধ্রুৱ শূন্য *obj, সংরচনা k_object *ko,
			গণনা k_objects otype)
{
	ৱিকল্প (retval) {
	পক্ষ -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		যদি (ko == রিক্ত) {
			LOG_ERR("address is not a known kernel object");
		} অন্যথা {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		ভঙ্গ;
	পক্ষ -EPERM:
		dump_permission_error(ko);
		ভঙ্গ;
	পক্ষ -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		ভঙ্গ;
	পক্ষ -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		ভঙ্গ;
	স্ৱভাৱ:
		/* Not handled error */
		ভঙ্গ;
	}
}

শূন্য z_impl_k_object_access_grant(ধ্রুৱ শূন্য *object, সংরচনা k_thread *thread)
{
	সংরচনা k_object *ko = k_object_find(object);

	যদি (ko != রিক্ত) {
		k_thread_perms_set(ko, thread);
	}
}

শূন্য k_object_access_revoke(ধ্রুৱ শূন্য *object, সংরচনা k_thread *thread)
{
	সংরচনা k_object *ko = k_object_find(object);

	যদি (ko != রিক্ত) {
		k_thread_perms_clear(ko, thread);
	}
}

শূন্য k_object_access_revoke_others(ধ্রুৱ শূন্য *object)
{
	সংরচনা k_object *ko = k_object_find(object);

	যদি (ko != রিক্ত) {
		(শূন্য)memset(ko->perms, 0, পরিমাণ(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

শূন্য z_impl_k_object_release(ধ্রুৱ শূন্য *object)
{
	k_object_access_revoke(object, _current);
}

শূন্য k_object_access_all_grant(ধ্রুৱ শূন্য *object)
{
	সংরচনা k_object *ko = k_object_find(object);

	যদি (ko != রিক্ত) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

পূর্ণাঙ্ক k_object_validate(সংরচনা k_object *ko, গণনা k_objects otype,
		       গণনা _obj_init_check init)
{
	যদি (unlikely(ko == রিক্ত)) {
		ঘূৰাই -EBADF;
	}

	যদি (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		যদি ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			ঘূৰাই -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	যদি (unlikely(thread_perms_test(ko) == 0)) {
		ঘূৰাই -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	যদি (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		যদি (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			ঘূৰাই -EINVAL;
		}
	} অন্যথা যদি (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		যদি (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			ঘূৰাই -EADDRINUSE;
		}
	} অন্যথা {
		/* _OBJ_INIT_ANY */
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক z_impl_k_object_access_check(ধ্রুৱ শূন্য *object)
{
	ঘূৰাই k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

শূন্য k_object_init(ধ্রুৱ শূন্য *obj)
{
	সংরচনা k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	যদি (ko == রিক্ত) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		ঘূৰাই;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

শূন্য k_object_recycle(ধ্রুৱ শূন্য *obj)
{
	সংরচনা k_object *ko = k_object_find(obj);

	যদি (ko != রিক্ত) {
		(শূন্য)memset(ko->perms, 0, পরিমাণ(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

শূন্য k_object_uninit(ধ্রুৱ শূন্য *obj)
{
	সংরচনা k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	যদি (ko == রিক্ত) {
		ঘূৰাই;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
শূন্য *k_usermode_alloc_from_copy(ধ্রুৱ শূন্য *src, size_t size)
{
	শূন্য *dst = রিক্ত;

	/* Does the caller in user mode have access to read this memory? */
	যদি (K_SYSCALL_MEMORY_READ(src, size)) {
		গচ্ছ out_err;
	}

	dst = z_thread_malloc(size);
	যদি (dst == রিক্ত) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		গচ্ছ out_err;
	}

	(শূন্য)memcpy(dst, src, size);
out_err:
	ঘূৰাই dst;
}

স্থির পূর্ণাঙ্ক user_copy(শূন্য *dst, ধ্রুৱ শূন্য *src, size_t size, bool to_user)
{
	পূর্ণাঙ্ক ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	যদি (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		গচ্ছ out_err;
	}

	(শূন্য)memcpy(dst, src, size);
	ret = 0;
out_err:
	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_usermode_from_copy(শূন্য *dst, ধ্রুৱ শূন্য *src, size_t size)
{
	ঘূৰাই user_copy(dst, src, size, false);
}

পূর্ণাঙ্ক k_usermode_to_copy(শূন্য *dst, ধ্রুৱ শূন্য *src, size_t size)
{
	ঘূৰাই user_copy(dst, src, size, true);
}

ৱর্ণ *k_usermode_string_alloc_copy(ধ্রুৱ ৱর্ণ *src, size_t maxlen)
{
	size_t actual_len;
	পূর্ণাঙ্ক err;
	ৱর্ণ *ret = রিক্ত;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	যদি (err != 0) {
		গচ্ছ out;
	}
	যদি (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		গচ্ছ out;
	}
	যদি (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		গচ্ছ out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	যদি (ret != রিক্ত) {
		ret[actual_len - 1U] = '\0';
	}
out:
	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_usermode_string_copy(ৱর্ণ *dst, ধ্রুৱ ৱর্ণ *src, size_t maxlen)
{
	size_t actual_len;
	পূর্ণাঙ্ক ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	যদি (err != 0) {
		ret = EFAULT;
		গচ্ছ out;
	}
	যদি (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		গচ্ছ out;
	}
	যদি (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		গচ্ছ out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	ঘূৰাই ret;
}

/*
 * Application memory region initialization
 */

বাহ্য ৱর্ণ __app_shmem_regions_start[];
বাহ্য ৱর্ণ __app_shmem_regions_end[];

স্থির শূন্য app_shmem_bss_zero(শূন্য)
{
	সংরচনা z_app_region *region, *end;


	end = (সংরচনা z_app_region *)&__app_shmem_regions_end[0];
	region = (সংরচনা z_app_region *)&__app_shmem_regions_start[0];

	বাবে ( ; region < end; region++) {
		(শূন্য)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

স্থির uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     শূন্য *ssf)
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

স্থির uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, শূন্য *ssf)
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

#অন্তর্ভাৱ <zephyr/syscall_dispatch.c>
