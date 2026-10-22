/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/sys/rb.h>
#அந்தர்பாவ <zephyr/sys/sys_io.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <zephyr/syscall.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/device.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <zephyr/app_memory/app_memdomain.h>
#அந்தர்பாவ <zephyr/sys/libc-hooks.h>
#அந்தர்பாவ <zephyr/sys/mutex.h>
#அந்தர்பாவ <zephyr/sys/util.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <zephyr/linker/linker-defs.h>
#அந்தர்பாவ <zephyr/cache.h>
#அந்தர்பாவ <kernel_internal.h>

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

#அந்தர்பாவ <zephyr/logging/log.h>
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
நிலையான கட்டமைப்பு k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#எனில் defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define அடுக்கு_ELEMENT_DATA_SIZE(size) \
	(அளவு(கட்டமைப்பு z_அடுக்கு_data) + CONFIG_PRIVILEGED_அடுக்கு_SIZE + \
	Z_THREAD_அடுக்கு_OBJ_ALIGN(size) + K_THREAD_அடுக்கு_LEN(size))
#இல்லையெனில்
#define அடுக்கு_ELEMENT_DATA_SIZE(size) (அளவு(கட்டமைப்பு z_அடுக்கு_data) + \
	K_THREAD_அடுக்கு_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#இல்லையெனில்
#define அடுக்கு_ELEMENT_DATA_SIZE(size) K_THREAD_அடுக்கு_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
நிலையான கட்டமைப்பு k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
வெளி uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

வெளி முழு z_msgq_cleanup(கட்டமைப்பு k_msgq *q, bool locked);
வெளி முழு z_அடுக்கு_cleanup(கட்டமைப்பு k_அடுக்கு *அடுக்கு, bool locked);
வெளி முழு z_timer_cleanup(கட்டமைப்பு k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

நிலையான வெறுமை unref_check(கட்டமைப்பு k_object *ko, uintptr_t index, bool locked);

மாறிலி எழுத்து *otype_to_str(எண்ணிக்கை k_objects otype)
{
	மாறிலி எழுத்து *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	தேர்வு (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	நிலை K_OBJ_ANY:
		ret = "generic";
		நிறுத்து;
	நிலை K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		நிறுத்து;
#அந்தர்பாவ <zephyr/otype-to-str.h>
	இயல்பு:
		ret = "?";
		நிறுத்து;
	}
#இல்லையெனில்
	ARG_UNUSED(otype);
	ret = வெற்று;
#endif /* CONFIG_LOG */
	திரும்பு ret;
}

கட்டமைப்பு perm_ctx {
	முழு parent_id;
	முழு child_id;
	கட்டமைப்பு k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_அடுக்கு_find(k_thread_அடுக்கு_t *அடுக்கு)
{
	கட்டமைப்பு k_object *obj = k_object_find(அடுக்கு);

	__ASSERT(obj != வெற்று, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_அடுக்கு_ELEMENT,
		 "bad stack object");

	திரும்பு obj->data.அடுக்கு_data->priv;
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
#இல்லையெனில்
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(அளவு(வெறுமை *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_அடுக்கு_SIZE
#எனில் defined(CONFIG_MPU_அடுக்கு_GUARD) || defined(CONFIG_PMP_அடுக்கு_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_அடுக்கு \
	Z_THREAD_அடுக்கு_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_அடுக்கு_SIZE)
#இல்லையெனில்
#define DYN_OBJ_DATA_ALIGN_K_THREAD_அடுக்கு \
	Z_THREAD_அடுக்கு_OBJ_ALIGN(CONFIG_PRIVILEGED_அடுக்கு_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#இல்லையெனில்
#define DYN_OBJ_DATA_ALIGN_K_THREAD_அடுக்கு \
	Z_THREAD_அடுக்கு_OBJ_ALIGN(ARCH_அடுக்கு_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (அளவு(வெறுமை *)))

கட்டமைப்பு dyn_obj {
	கட்டமைப்பு k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	வெறுமை *data;
};

வெளி கட்டமைப்பு k_object *z_object_gperf_find(மாறிலி வெறுமை *obj);
வெளி வெறுமை z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     வெறுமை *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
நிலையான sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

நிலையான size_t obj_size_get(எண்ணிக்கை k_objects otype)
{
	size_t ret;

	தேர்வு (otype) {
#அந்தர்பாவ <zephyr/otype-to-size.h>
	இயல்பு:
		ret = அளவு(மாறிலி கட்டமைப்பு device);
		நிறுத்து;
	}

	திரும்பு ret;
}

நிலையான size_t obj_align_get(எண்ணிக்கை k_objects otype)
{
	size_t ret;

	தேர்வு (otype) {
	நிலை K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#இல்லையெனில்
		ret = __alignof(கட்டமைப்பு dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		நிறுத்து;
	இயல்பு:
		ret = __alignof(கட்டமைப்பு dyn_obj);
		நிறுத்து;
	}

	திரும்பு ret;
}

நிலையான கட்டமைப்பு dyn_obj *dyn_object_find_locked(மாறிலி வெறுமை *obj)
{
	கட்டமைப்பு dyn_obj *முனை;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, முனை, dobj_list) {
		எனில் (முனை->kobj.name == obj) {
			திரும்பு முனை;
		}
	}

	திரும்பு வெற்று;
}

நிலையான கட்டமைப்பு dyn_obj *dyn_object_find(மாறிலி வெறுமை *obj)
{
	கட்டமைப்பு dyn_obj *முனை;
	k_spinlock_சாவி_t சாவி;

	சாவி = k_spin_lock(&lists_lock);
	முனை = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, சாவி);

	திரும்பு முனை;
}

நிலையான வெறுமை clear_perms_cb(கட்டமைப்பு k_object *ko, வெறுமை *ctx_ptr)
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
நிலையான bool thread_idx_alloc(uintptr_t *tidx)
{
	முழு i;
	முழு idx;
	முழு base;
	bool ret = false;
	k_spinlock_சாவி_t சாவி;

	base = 0;
	சாவி = k_spin_lock(&lists_lock);
	க்கு (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		எனில் (idx != 0) {
			கட்டமைப்பு dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (வெறுமை *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (வெறுமை *)*tidx);
			}

			ret = true;
			நிறுத்து;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, சாவி);
	திரும்பு ret;
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
நிலையான வெறுமை thread_idx_free(uintptr_t tidx)
{
	கட்டமைப்பு dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (வெறுமை *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_சாவி_t சாவி = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (வெறுமை *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	முழு base = tidx / இலக்கம்_BITS(_thread_idx_map[0]);
	முழு offset = tidx % இலக்கம்_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, சாவி);
}

நிலையான கட்டமைப்பு k_object *dynamic_object_create(எண்ணிக்கை k_objects otype, size_t align,
					      size_t size)
{
	கட்டமைப்பு dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, அளவு(கட்டமைப்பு dyn_obj));
	எனில் (dyn == வெற்று) {
		திரும்பு வெற்று;
	}

	எனில் (otype == K_OBJ_THREAD_அடுக்கு_ELEMENT) {
		size_t adjusted_size;

		எனில் (size == 0) {
			k_free(dyn);
			திரும்பு வெற்று;
		}

		adjusted_size = அடுக்கு_ELEMENT_DATA_SIZE(size);
		எனில் (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			திரும்பு வெற்று;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_அடுக்கு,
						     adjusted_size);
		எனில் (dyn->data == வெற்று) {
			k_free(dyn);
			திரும்பு வெற்று;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_அடுக்கு_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		எனில் (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		கட்டமைப்பு z_அடுக்கு_data *அடுக்கு_data = (கட்டமைப்பு z_அடுக்கு_data *)
			((uint8_t *)dyn->data + adjusted_size - அளவு(*அடுக்கு_data));
#எனில் defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		அடுக்கு_data->priv = (வெறுமை *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_அடுக்கு_OBJ_ALIGN(size));
#இல்லையெனில்
		அடுக்கு_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		அடுக்கு_data->size = adjusted_size;
		dyn->kobj.data.அடுக்கு_data = அடுக்கு_data;
		dyn->kobj.name = dyn->data;
#இல்லையெனில்
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.அடுக்கு_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} இல்லையெனில் {
		size_t total_size = obj_size_get(otype) + size;

		எனில் (total_size < size) {
			k_free(dyn);
			திரும்பு வெற்று;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		எனில் (dyn->data == வெற்று) {
			k_free(dyn);
			திரும்பு வெற்று;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(வெறுமை)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_சாவி_t சாவி = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, சாவி);

	திரும்பு &dyn->kobj;
}

கட்டமைப்பு k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	கட்டமைப்பு k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	எனில் (obj == வெற்று) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	திரும்பு obj;
}

நிலையான வெறுமை *z_object_alloc(எண்ணிக்கை k_objects otype, size_t size)
{
	கட்டமைப்பு k_object *zo;
	uintptr_t tidx = 0;

	எனில் ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		திரும்பு வெற்று;
	}

	தேர்வு (otype) {
	நிலை K_OBJ_THREAD:
		எனில் (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			திரும்பு வெற்று;
		}
		நிறுத்து;
	/* The following are currently not allowed at all */
	நிலை K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	நிலை K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		திரும்பு வெற்று;
	இயல்பு:
		/* Remainder within bounds are permitted */
		நிறுத்து;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	எனில் (zo == வெற்று) {
		எனில் (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		திரும்பு வெற்று;
	}

	எனில் (otype == K_OBJ_THREAD) {
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

	திரும்பு zo->name;
}

வெறுமை *z_impl_k_object_alloc(எண்ணிக்கை k_objects otype)
{
	திரும்பு z_object_alloc(otype, 0);
}

வெறுமை *z_impl_k_object_alloc_size(எண்ணிக்கை k_objects otype, size_t size)
{
	திரும்பு z_object_alloc(otype, size);
}

வெறுமை k_object_free(வெறுமை *obj)
{
	கட்டமைப்பு dyn_obj *dyn = வெற்று;
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
	k_spinlock_சாவி_t சாவி = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	எனில் (dyn != வெற்று) {
		sys_dlist_remove(&dyn->dobj_list);

		எனில் (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, சாவி);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	எனில் (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	எனில் (dyn != வெற்று) {
#ifdef CONFIG_OBJ_CORE
		எனில் (dyn->kobj.type != K_OBJ_THREAD_அடுக்கு_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_அடுக்கு_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		எனில் (dyn->kobj.type == K_OBJ_THREAD_அடுக்கு_ELEMENT) {
			uint8_t *அடுக்கு;

			எனில் (sys_cache_is_ptr_cached(dyn->data)) {
				அடுக்கு = sys_cache_uncached_ptr_get(dyn->data);
			} இல்லையெனில் {
				அடுக்கு = dyn->data;
			}

			k_free(அடுக்கு);
		} இல்லையெனில்
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

கட்டமைப்பு k_object *k_object_find(மாறிலி வெறுமை *obj)
{
	கட்டமைப்பு k_object *ret;

	ret = z_object_gperf_find(obj);

	எனில் (ret == வெற்று) {
		கட்டமைப்பு dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		எனில் (dyn != வெற்று) {
			ret = &dyn->kobj;
		}
	}

	திரும்பு ret;
}

வெறுமை k_object_wordlist_foreach(_wordlist_cb_func_t func, வெறுமை *context)
{
	கட்டமைப்பு dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_சாவி_t சாவி = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, சாவி);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak கட்டமைப்பு k_object *z_object_gperf_find(மாறிலி வெறுமை *obj)
{
	ARG_UNUSED(obj);
	திரும்பு வெற்று;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak வெறுமை z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, வெறுமை *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#இல்லையெனில்
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak கட்டமைப்பு k_object *k_object_find(மாறிலி வெறுமை *obj)
{
	திரும்பு வெற்று;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak வெறுமை k_object_wordlist_foreach(_wordlist_cb_func_t func, வெறுமை *context)
{
}
#endif

நிலையான unsigned முழு thread_index_get(கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு k_object *ko;

	ko = k_object_find(thread);

	எனில் (ko == வெற்று) {
		திரும்பு -1;
	}

	திரும்பு ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
நிலையான வெறுமை unref_check(கட்டமைப்பு k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	எனில் ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		செல் out;
	}

	வெறுமை *vko = ko;

	கட்டமைப்பு dyn_obj *dyn = CONTAINER_OF(vko, கட்டமைப்பு dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, கட்டமைப்பு dyn_obj), "unaligned z_object");

	க்கு (முழு i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		எனில் (ko->perms[i] != 0U) {
			செல் out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	தேர்வு (ko->type) {
	நிலை K_OBJ_MSGQ:
		(வெறுமை)z_msgq_cleanup((கட்டமைப்பு k_msgq *)ko->name, locked);
		நிறுத்து;
	நிலை K_OBJ_அடுக்கு:
		(வெறுமை)z_அடுக்கு_cleanup((கட்டமைப்பு k_அடுக்கு *)ko->name, locked);
		நிறுத்து;
	நிலை K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		எனில் ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(வெறுமை)z_timer_cleanup((கட்டமைப்பு k_timer *)ko->name, locked);
		}
		நிறுத்து;
	இயல்பு:
		/* Nothing to do */
		நிறுத்து;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, சாவி);
}

நிலையான வெறுமை wordlist_cb(கட்டமைப்பு k_object *ko, வெறுமை *ctx_ptr)
{
	கட்டமைப்பு perm_ctx *ctx = (கட்டமைப்பு perm_ctx *)ctx_ptr;

	எனில் (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((கட்டமைப்பு k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

வெறுமை k_thread_perms_inherit(கட்டமைப்பு k_thread *parent, கட்டமைப்பு k_thread *child)
{
	கட்டமைப்பு perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	எனில் ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

வெறுமை k_thread_perms_set(கட்டமைப்பு k_object *ko, கட்டமைப்பு k_thread *thread)
{
	முழு index = thread_index_get(thread);

	எனில் (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

வெறுமை k_thread_perms_clear(கட்டமைப்பு k_object *ko, கட்டமைப்பு k_thread *thread)
{
	முழு index = thread_index_get(thread);

	எனில் (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_சாவி_t சாவி = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, சாவி);
#இல்லையெனில்
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

நிலையான வெறுமை clear_perms_cb_locked(கட்டமைப்பு k_object *ko, வெறுமை *ctx_ptr)
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
வெறுமை k_thread_perms_all_clear(கட்டமைப்பு k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	எனில் ((முழு)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (வெறுமை *)index);
	}
}

நிலையான முழு thread_perms_test(கட்டமைப்பு k_object *ko)
{
	முழு index;

	எனில் ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		திரும்பு 1;
	}

	index = thread_index_get(_current);
	எனில் (index != -1) {
		திரும்பு sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	திரும்பு 0;
}

நிலையான வெறுமை dump_permission_error(கட்டமைப்பு k_object *ko)
{
	முழு index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, அளவு(ko->perms), "permission bitmap");
}

வெறுமை k_object_dump_error(முழு retval, மாறிலி வெறுமை *obj, கட்டமைப்பு k_object *ko,
			எண்ணிக்கை k_objects otype)
{
	தேர்வு (retval) {
	நிலை -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		எனில் (ko == வெற்று) {
			LOG_ERR("address is not a known kernel object");
		} இல்லையெனில் {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		நிறுத்து;
	நிலை -EPERM:
		dump_permission_error(ko);
		நிறுத்து;
	நிலை -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		நிறுத்து;
	நிலை -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		நிறுத்து;
	இயல்பு:
		/* Not handled error */
		நிறுத்து;
	}
}

வெறுமை z_impl_k_object_access_grant(மாறிலி வெறுமை *object, கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு k_object *ko = k_object_find(object);

	எனில் (ko != வெற்று) {
		k_thread_perms_set(ko, thread);
	}
}

வெறுமை k_object_access_revoke(மாறிலி வெறுமை *object, கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு k_object *ko = k_object_find(object);

	எனில் (ko != வெற்று) {
		k_thread_perms_clear(ko, thread);
	}
}

வெறுமை k_object_access_revoke_others(மாறிலி வெறுமை *object)
{
	கட்டமைப்பு k_object *ko = k_object_find(object);

	எனில் (ko != வெற்று) {
		(வெறுமை)memset(ko->perms, 0, அளவு(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

வெறுமை z_impl_k_object_release(மாறிலி வெறுமை *object)
{
	k_object_access_revoke(object, _current);
}

வெறுமை k_object_access_all_grant(மாறிலி வெறுமை *object)
{
	கட்டமைப்பு k_object *ko = k_object_find(object);

	எனில் (ko != வெற்று) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

முழு k_object_validate(கட்டமைப்பு k_object *ko, எண்ணிக்கை k_objects otype,
		       எண்ணிக்கை _obj_init_check init)
{
	எனில் (unlikely(ko == வெற்று)) {
		திரும்பு -EBADF;
	}

	எனில் (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		எனில் ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			திரும்பு -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	எனில் (unlikely(thread_perms_test(ko) == 0)) {
		திரும்பு -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	எனில் (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		எனில் (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			திரும்பு -EINVAL;
		}
	} இல்லையெனில் எனில் (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		எனில் (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			திரும்பு -EADDRINUSE;
		}
	} இல்லையெனில் {
		/* _OBJ_INIT_ANY */
	}

	திரும்பு 0;
}

முழு z_impl_k_object_access_check(மாறிலி வெறுமை *object)
{
	திரும்பு k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

வெறுமை k_object_init(மாறிலி வெறுமை *obj)
{
	கட்டமைப்பு k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	எனில் (ko == வெற்று) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		திரும்பு;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

வெறுமை k_object_recycle(மாறிலி வெறுமை *obj)
{
	கட்டமைப்பு k_object *ko = k_object_find(obj);

	எனில் (ko != வெற்று) {
		(வெறுமை)memset(ko->perms, 0, அளவு(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

வெறுமை k_object_uninit(மாறிலி வெறுமை *obj)
{
	கட்டமைப்பு k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	எனில் (ko == வெற்று) {
		திரும்பு;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
வெறுமை *k_usermode_alloc_from_copy(மாறிலி வெறுமை *src, size_t size)
{
	வெறுமை *dst = வெற்று;

	/* Does the caller in user mode have access to read this memory? */
	எனில் (K_SYSCALL_MEMORY_READ(src, size)) {
		செல் out_err;
	}

	dst = z_thread_malloc(size);
	எனில் (dst == வெற்று) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		செல் out_err;
	}

	(வெறுமை)memcpy(dst, src, size);
out_err:
	திரும்பு dst;
}

நிலையான முழு user_copy(வெறுமை *dst, மாறிலி வெறுமை *src, size_t size, bool to_user)
{
	முழு ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	எனில் (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		செல் out_err;
	}

	(வெறுமை)memcpy(dst, src, size);
	ret = 0;
out_err:
	திரும்பு ret;
}

முழு k_usermode_from_copy(வெறுமை *dst, மாறிலி வெறுமை *src, size_t size)
{
	திரும்பு user_copy(dst, src, size, false);
}

முழு k_usermode_to_copy(வெறுமை *dst, மாறிலி வெறுமை *src, size_t size)
{
	திரும்பு user_copy(dst, src, size, true);
}

எழுத்து *k_usermode_string_alloc_copy(மாறிலி எழுத்து *src, size_t maxlen)
{
	size_t actual_len;
	முழு err;
	எழுத்து *ret = வெற்று;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	எனில் (err != 0) {
		செல் out;
	}
	எனில் (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		செல் out;
	}
	எனில் (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		செல் out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	எனில் (ret != வெற்று) {
		ret[actual_len - 1U] = '\0';
	}
out:
	திரும்பு ret;
}

முழு k_usermode_string_copy(எழுத்து *dst, மாறிலி எழுத்து *src, size_t maxlen)
{
	size_t actual_len;
	முழு ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	எனில் (err != 0) {
		ret = EFAULT;
		செல் out;
	}
	எனில் (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		செல் out;
	}
	எனில் (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		செல் out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	திரும்பு ret;
}

/*
 * Application memory region initialization
 */

வெளி எழுத்து __app_shmem_regions_start[];
வெளி எழுத்து __app_shmem_regions_end[];

நிலையான வெறுமை app_shmem_bss_zero(வெறுமை)
{
	கட்டமைப்பு z_app_region *region, *end;


	end = (கட்டமைப்பு z_app_region *)&__app_shmem_regions_end[0];
	region = (கட்டமைப்பு z_app_region *)&__app_shmem_regions_start[0];

	க்கு ( ; region < end; region++) {
		(வெறுமை)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

நிலையான uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     வெறுமை *ssf)
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

நிலையான uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, வெறுமை *ssf)
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

#அந்தர்பாவ <zephyr/syscall_dispatch.c>
