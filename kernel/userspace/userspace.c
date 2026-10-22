/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#شامل <zephyr/kernel.h>
#شامل <string.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/rb.h>
#شامل <zephyr/sys/sys_io.h>
#شامل <ksched.h>
#شامل <zephyr/syscall.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/device.h>
#شامل <stdbool.h>
#شامل <zephyr/app_memory/app_memdomain.h>
#شامل <zephyr/sys/libc-hooks.h>
#شامل <zephyr/sys/mutex.h>
#شامل <zephyr/sys/util.h>
#شامل <inttypes.h>
#شامل <zephyr/linker/linker-defs.h>
#شامل <zephyr/cache.h>
#شامل <kernel_internal.h>

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

#شامل <zephyr/logging/log.h>
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
ایستا ساختار k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#اگر defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define پشته_ELEMENT_DATA_SIZE(size) \
	(اندازه(ساختار z_پشته_data) + CONFIG_PRIVILEGED_پشته_SIZE + \
	Z_THREAD_پشته_OBJ_ALIGN(size) + K_THREAD_پشته_LEN(size))
#وگرنه
#define پشته_ELEMENT_DATA_SIZE(size) (اندازه(ساختار z_پشته_data) + \
	K_THREAD_پشته_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#وگرنه
#define پشته_ELEMENT_DATA_SIZE(size) K_THREAD_پشته_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
ایستا ساختار k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
بیرونی uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

بیرونی صحیح z_msgq_cleanup(ساختار k_msgq *q, bool locked);
بیرونی صحیح z_پشته_cleanup(ساختار k_پشته *پشته, bool locked);
بیرونی صحیح z_timer_cleanup(ساختار k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

ایستا تهی unref_check(ساختار k_object *ko, uintptr_t index, bool locked);

ثابت نویسه *otype_to_str(شمارش k_objects otype)
{
	ثابت نویسه *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	انتخاب (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	حالت K_OBJ_ANY:
		ret = "generic";
		بشکن;
	حالت K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		بشکن;
#شامل <zephyr/otype-to-str.h>
	پیشفرض:
		ret = "?";
		بشکن;
	}
#وگرنه
	ARG_UNUSED(otype);
	ret = پوچ;
#endif /* CONFIG_LOG */
	بازگشت ret;
}

ساختار perm_ctx {
	صحیح parent_id;
	صحیح child_id;
	ساختار k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_پشته_find(k_thread_پشته_t *پشته)
{
	ساختار k_object *obj = k_object_find(پشته);

	__ASSERT(obj != پوچ, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_پشته_ELEMENT,
		 "bad stack object");

	بازگشت obj->data.پشته_data->priv;
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
#وگرنه
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(اندازه(تهی *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_پشته_SIZE
#اگر defined(CONFIG_MPU_پشته_GUARD) || defined(CONFIG_PMP_پشته_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_پشته \
	Z_THREAD_پشته_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_پشته_SIZE)
#وگرنه
#define DYN_OBJ_DATA_ALIGN_K_THREAD_پشته \
	Z_THREAD_پشته_OBJ_ALIGN(CONFIG_PRIVILEGED_پشته_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#وگرنه
#define DYN_OBJ_DATA_ALIGN_K_THREAD_پشته \
	Z_THREAD_پشته_OBJ_ALIGN(ARCH_پشته_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (اندازه(تهی *)))

ساختار dyn_obj {
	ساختار k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	تهی *data;
};

بیرونی ساختار k_object *z_object_gperf_find(ثابت تهی *obj);
بیرونی تهی z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     تهی *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
ایستا sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

ایستا size_t obj_size_get(شمارش k_objects otype)
{
	size_t ret;

	انتخاب (otype) {
#شامل <zephyr/otype-to-size.h>
	پیشفرض:
		ret = اندازه(ثابت ساختار device);
		بشکن;
	}

	بازگشت ret;
}

ایستا size_t obj_align_get(شمارش k_objects otype)
{
	size_t ret;

	انتخاب (otype) {
	حالت K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#وگرنه
		ret = __alignof(ساختار dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		بشکن;
	پیشفرض:
		ret = __alignof(ساختار dyn_obj);
		بشکن;
	}

	بازگشت ret;
}

ایستا ساختار dyn_obj *dyn_object_find_locked(ثابت تهی *obj)
{
	ساختار dyn_obj *گره;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, گره, dobj_list) {
		اگر (گره->kobj.name == obj) {
			بازگشت گره;
		}
	}

	بازگشت پوچ;
}

ایستا ساختار dyn_obj *dyn_object_find(ثابت تهی *obj)
{
	ساختار dyn_obj *گره;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_lock(&lists_lock);
	گره = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, کلید);

	بازگشت گره;
}

ایستا تهی clear_perms_cb(ساختار k_object *ko, تهی *ctx_ptr)
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
ایستا bool thread_idx_alloc(uintptr_t *tidx)
{
	صحیح i;
	صحیح idx;
	صحیح base;
	bool ret = false;
	k_spinlock_کلید_t کلید;

	base = 0;
	کلید = k_spin_lock(&lists_lock);
	برای (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		اگر (idx != 0) {
			ساختار dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (تهی *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (تهی *)*tidx);
			}

			ret = true;
			بشکن;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, کلید);
	بازگشت ret;
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
ایستا تهی thread_idx_free(uintptr_t tidx)
{
	ساختار dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (تهی *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_کلید_t کلید = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (تهی *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	صحیح base = tidx / رقم_BITS(_thread_idx_map[0]);
	صحیح offset = tidx % رقم_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, کلید);
}

ایستا ساختار k_object *dynamic_object_create(شمارش k_objects otype, size_t align,
					      size_t size)
{
	ساختار dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, اندازه(ساختار dyn_obj));
	اگر (dyn == پوچ) {
		بازگشت پوچ;
	}

	اگر (otype == K_OBJ_THREAD_پشته_ELEMENT) {
		size_t adjusted_size;

		اگر (size == 0) {
			k_free(dyn);
			بازگشت پوچ;
		}

		adjusted_size = پشته_ELEMENT_DATA_SIZE(size);
		اگر (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			بازگشت پوچ;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_پشته,
						     adjusted_size);
		اگر (dyn->data == پوچ) {
			k_free(dyn);
			بازگشت پوچ;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_پشته_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		اگر (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		ساختار z_پشته_data *پشته_data = (ساختار z_پشته_data *)
			((uint8_t *)dyn->data + adjusted_size - اندازه(*پشته_data));
#اگر defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		پشته_data->priv = (تهی *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_پشته_OBJ_ALIGN(size));
#وگرنه
		پشته_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		پشته_data->size = adjusted_size;
		dyn->kobj.data.پشته_data = پشته_data;
		dyn->kobj.name = dyn->data;
#وگرنه
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.پشته_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} وگرنه {
		size_t total_size = obj_size_get(otype) + size;

		اگر (total_size < size) {
			k_free(dyn);
			بازگشت پوچ;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		اگر (dyn->data == پوچ) {
			k_free(dyn);
			بازگشت پوچ;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(تهی)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_کلید_t کلید = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, کلید);

	بازگشت &dyn->kobj;
}

ساختار k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	ساختار k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	اگر (obj == پوچ) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	بازگشت obj;
}

ایستا تهی *z_object_alloc(شمارش k_objects otype, size_t size)
{
	ساختار k_object *zo;
	uintptr_t tidx = 0;

	اگر ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		بازگشت پوچ;
	}

	انتخاب (otype) {
	حالت K_OBJ_THREAD:
		اگر (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			بازگشت پوچ;
		}
		بشکن;
	/* The following are currently not allowed at all */
	حالت K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	حالت K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		بازگشت پوچ;
	پیشفرض:
		/* Remainder within bounds are permitted */
		بشکن;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	اگر (zo == پوچ) {
		اگر (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		بازگشت پوچ;
	}

	اگر (otype == K_OBJ_THREAD) {
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

	بازگشت zo->name;
}

تهی *z_impl_k_object_alloc(شمارش k_objects otype)
{
	بازگشت z_object_alloc(otype, 0);
}

تهی *z_impl_k_object_alloc_size(شمارش k_objects otype, size_t size)
{
	بازگشت z_object_alloc(otype, size);
}

تهی k_object_free(تهی *obj)
{
	ساختار dyn_obj *dyn = پوچ;
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
	k_spinlock_کلید_t کلید = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	اگر (dyn != پوچ) {
		sys_dlist_remove(&dyn->dobj_list);

		اگر (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, کلید);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	اگر (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	اگر (dyn != پوچ) {
#ifdef CONFIG_OBJ_CORE
		اگر (dyn->kobj.type != K_OBJ_THREAD_پشته_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_پشته_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		اگر (dyn->kobj.type == K_OBJ_THREAD_پشته_ELEMENT) {
			uint8_t *پشته;

			اگر (sys_cache_is_ptr_cached(dyn->data)) {
				پشته = sys_cache_uncached_ptr_get(dyn->data);
			} وگرنه {
				پشته = dyn->data;
			}

			k_free(پشته);
		} وگرنه
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

ساختار k_object *k_object_find(ثابت تهی *obj)
{
	ساختار k_object *ret;

	ret = z_object_gperf_find(obj);

	اگر (ret == پوچ) {
		ساختار dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		اگر (dyn != پوچ) {
			ret = &dyn->kobj;
		}
	}

	بازگشت ret;
}

تهی k_object_wordlist_foreach(_wordlist_cb_func_t func, تهی *context)
{
	ساختار dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_کلید_t کلید = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, کلید);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ساختار k_object *z_object_gperf_find(ثابت تهی *obj)
{
	ARG_UNUSED(obj);
	بازگشت پوچ;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak تهی z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, تهی *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#وگرنه
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ساختار k_object *k_object_find(ثابت تهی *obj)
{
	بازگشت پوچ;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak تهی k_object_wordlist_foreach(_wordlist_cb_func_t func, تهی *context)
{
}
#endif

ایستا unsigned صحیح thread_index_get(ساختار k_thread *thread)
{
	ساختار k_object *ko;

	ko = k_object_find(thread);

	اگر (ko == پوچ) {
		بازگشت -1;
	}

	بازگشت ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ایستا تهی unref_check(ساختار k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	اگر ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		برو out;
	}

	تهی *vko = ko;

	ساختار dyn_obj *dyn = CONTAINER_OF(vko, ساختار dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, ساختار dyn_obj), "unaligned z_object");

	برای (صحیح i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		اگر (ko->perms[i] != 0U) {
			برو out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	انتخاب (ko->type) {
	حالت K_OBJ_MSGQ:
		(تهی)z_msgq_cleanup((ساختار k_msgq *)ko->name, locked);
		بشکن;
	حالت K_OBJ_پشته:
		(تهی)z_پشته_cleanup((ساختار k_پشته *)ko->name, locked);
		بشکن;
	حالت K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		اگر ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(تهی)z_timer_cleanup((ساختار k_timer *)ko->name, locked);
		}
		بشکن;
	پیشفرض:
		/* Nothing to do */
		بشکن;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, کلید);
}

ایستا تهی wordlist_cb(ساختار k_object *ko, تهی *ctx_ptr)
{
	ساختار perm_ctx *ctx = (ساختار perm_ctx *)ctx_ptr;

	اگر (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((ساختار k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

تهی k_thread_perms_inherit(ساختار k_thread *parent, ساختار k_thread *child)
{
	ساختار perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	اگر ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

تهی k_thread_perms_set(ساختار k_object *ko, ساختار k_thread *thread)
{
	صحیح index = thread_index_get(thread);

	اگر (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

تهی k_thread_perms_clear(ساختار k_object *ko, ساختار k_thread *thread)
{
	صحیح index = thread_index_get(thread);

	اگر (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_کلید_t کلید = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, کلید);
#وگرنه
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

ایستا تهی clear_perms_cb_locked(ساختار k_object *ko, تهی *ctx_ptr)
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
تهی k_thread_perms_all_clear(ساختار k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	اگر ((صحیح)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (تهی *)index);
	}
}

ایستا صحیح thread_perms_test(ساختار k_object *ko)
{
	صحیح index;

	اگر ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		بازگشت 1;
	}

	index = thread_index_get(_current);
	اگر (index != -1) {
		بازگشت sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	بازگشت 0;
}

ایستا تهی dump_permission_error(ساختار k_object *ko)
{
	صحیح index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, اندازه(ko->perms), "permission bitmap");
}

تهی k_object_dump_error(صحیح retval, ثابت تهی *obj, ساختار k_object *ko,
			شمارش k_objects otype)
{
	انتخاب (retval) {
	حالت -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		اگر (ko == پوچ) {
			LOG_ERR("address is not a known kernel object");
		} وگرنه {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		بشکن;
	حالت -EPERM:
		dump_permission_error(ko);
		بشکن;
	حالت -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		بشکن;
	حالت -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		بشکن;
	پیشفرض:
		/* Not handled error */
		بشکن;
	}
}

تهی z_impl_k_object_access_grant(ثابت تهی *object, ساختار k_thread *thread)
{
	ساختار k_object *ko = k_object_find(object);

	اگر (ko != پوچ) {
		k_thread_perms_set(ko, thread);
	}
}

تهی k_object_access_revoke(ثابت تهی *object, ساختار k_thread *thread)
{
	ساختار k_object *ko = k_object_find(object);

	اگر (ko != پوچ) {
		k_thread_perms_clear(ko, thread);
	}
}

تهی k_object_access_revoke_others(ثابت تهی *object)
{
	ساختار k_object *ko = k_object_find(object);

	اگر (ko != پوچ) {
		(تهی)memset(ko->perms, 0, اندازه(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

تهی z_impl_k_object_release(ثابت تهی *object)
{
	k_object_access_revoke(object, _current);
}

تهی k_object_access_all_grant(ثابت تهی *object)
{
	ساختار k_object *ko = k_object_find(object);

	اگر (ko != پوچ) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

صحیح k_object_validate(ساختار k_object *ko, شمارش k_objects otype,
		       شمارش _obj_init_check init)
{
	اگر (unlikely(ko == پوچ)) {
		بازگشت -EBADF;
	}

	اگر (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		اگر ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			بازگشت -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	اگر (unlikely(thread_perms_test(ko) == 0)) {
		بازگشت -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	اگر (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		اگر (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			بازگشت -EINVAL;
		}
	} وگرنه اگر (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		اگر (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			بازگشت -EADDRINUSE;
		}
	} وگرنه {
		/* _OBJ_INIT_ANY */
	}

	بازگشت 0;
}

صحیح z_impl_k_object_access_check(ثابت تهی *object)
{
	بازگشت k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

تهی k_object_init(ثابت تهی *obj)
{
	ساختار k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	اگر (ko == پوچ) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		بازگشت;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

تهی k_object_recycle(ثابت تهی *obj)
{
	ساختار k_object *ko = k_object_find(obj);

	اگر (ko != پوچ) {
		(تهی)memset(ko->perms, 0, اندازه(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

تهی k_object_uninit(ثابت تهی *obj)
{
	ساختار k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	اگر (ko == پوچ) {
		بازگشت;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
تهی *k_usermode_alloc_from_copy(ثابت تهی *src, size_t size)
{
	تهی *dst = پوچ;

	/* Does the caller in user mode have access to read this memory? */
	اگر (K_SYSCALL_MEMORY_READ(src, size)) {
		برو out_err;
	}

	dst = z_thread_malloc(size);
	اگر (dst == پوچ) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		برو out_err;
	}

	(تهی)memcpy(dst, src, size);
out_err:
	بازگشت dst;
}

ایستا صحیح user_copy(تهی *dst, ثابت تهی *src, size_t size, bool to_user)
{
	صحیح ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	اگر (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		برو out_err;
	}

	(تهی)memcpy(dst, src, size);
	ret = 0;
out_err:
	بازگشت ret;
}

صحیح k_usermode_from_copy(تهی *dst, ثابت تهی *src, size_t size)
{
	بازگشت user_copy(dst, src, size, false);
}

صحیح k_usermode_to_copy(تهی *dst, ثابت تهی *src, size_t size)
{
	بازگشت user_copy(dst, src, size, true);
}

نویسه *k_usermode_string_alloc_copy(ثابت نویسه *src, size_t maxlen)
{
	size_t actual_len;
	صحیح err;
	نویسه *ret = پوچ;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	اگر (err != 0) {
		برو out;
	}
	اگر (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		برو out;
	}
	اگر (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		برو out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	اگر (ret != پوچ) {
		ret[actual_len - 1U] = '\0';
	}
out:
	بازگشت ret;
}

صحیح k_usermode_string_copy(نویسه *dst, ثابت نویسه *src, size_t maxlen)
{
	size_t actual_len;
	صحیح ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	اگر (err != 0) {
		ret = EFAULT;
		برو out;
	}
	اگر (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		برو out;
	}
	اگر (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		برو out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	بازگشت ret;
}

/*
 * Application memory region initialization
 */

بیرونی نویسه __app_shmem_regions_start[];
بیرونی نویسه __app_shmem_regions_end[];

ایستا تهی app_shmem_bss_zero(تهی)
{
	ساختار z_app_region *region, *end;


	end = (ساختار z_app_region *)&__app_shmem_regions_end[0];
	region = (ساختار z_app_region *)&__app_shmem_regions_start[0];

	برای ( ; region < end; region++) {
		(تهی)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

ایستا uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     تهی *ssf)
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

ایستا uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, تهی *ssf)
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

#شامل <zephyr/syscall_dispatch.c>
