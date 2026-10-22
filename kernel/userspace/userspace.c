/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/sys/rb.h>
#અન્તર્ભાવ <zephyr/sys/sys_io.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/syscall.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/device.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <zephyr/app_memory/app_memdomain.h>
#અન્તર્ભાવ <zephyr/sys/libc-hooks.h>
#અન્તર્ભાવ <zephyr/sys/mutex.h>
#અન્તર્ભાવ <zephyr/sys/util.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <zephyr/linker/linker-defs.h>
#અન્તર્ભાવ <zephyr/cache.h>
#અન્તર્ભાવ <kernel_internal.h>

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

#અન્તર્ભાવ <zephyr/logging/log.h>
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
સ્થિર સંરચના k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#જો defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define ઢગલો_ELEMENT_DATA_SIZE(size) \
	(કદ(સંરચના z_ઢગલો_data) + CONFIG_PRIVILEGED_ઢગલો_SIZE + \
	Z_THREAD_ઢગલો_OBJ_ALIGN(size) + K_THREAD_ઢગલો_LEN(size))
#નહીંતર
#define ઢગલો_ELEMENT_DATA_SIZE(size) (કદ(સંરચના z_ઢગલો_data) + \
	K_THREAD_ઢગલો_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#નહીંતર
#define ઢગલો_ELEMENT_DATA_SIZE(size) K_THREAD_ઢગલો_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
સ્થિર સંરચના k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
બાહ્ય uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

બાહ્ય પૂર્ણાંક z_msgq_cleanup(સંરચના k_msgq *q, bool locked);
બાહ્ય પૂર્ણાંક z_ઢગલો_cleanup(સંરચના k_ઢગલો *ઢગલો, bool locked);
બાહ્ય પૂર્ણાંક z_timer_cleanup(સંરચના k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

સ્થિર શૂન્ય unref_check(સંરચના k_object *ko, uintptr_t index, bool locked);

અચલ અક્ષર *otype_to_str(ગણના k_objects otype)
{
	અચલ અક્ષર *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	પસંદગી (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	સ્થિતિ K_OBJ_ANY:
		ret = "generic";
		તોડો;
	સ્થિતિ K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		તોડો;
#અન્તર્ભાવ <zephyr/otype-to-str.h>
	મૂળભૂત:
		ret = "?";
		તોડો;
	}
#નહીંતર
	ARG_UNUSED(otype);
	ret = ખાલી;
#endif /* CONFIG_LOG */
	પાછા ret;
}

સંરચના perm_ctx {
	પૂર્ણાંક parent_id;
	પૂર્ણાંક child_id;
	સંરચના k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ઢગલો_find(k_thread_ઢગલો_t *ઢગલો)
{
	સંરચના k_object *obj = k_object_find(ઢગલો);

	__ASSERT(obj != ખાલી, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ઢગલો_ELEMENT,
		 "bad stack object");

	પાછા obj->data.ઢગલો_data->priv;
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
#નહીંતર
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(કદ(શૂન્ય *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_ઢગલો_SIZE
#જો defined(CONFIG_MPU_ઢગલો_GUARD) || defined(CONFIG_PMP_ઢગલો_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ઢગલો \
	Z_THREAD_ઢગલો_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ઢગલો_SIZE)
#નહીંતર
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ઢગલો \
	Z_THREAD_ઢગલો_OBJ_ALIGN(CONFIG_PRIVILEGED_ઢગલો_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#નહીંતર
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ઢગલો \
	Z_THREAD_ઢગલો_OBJ_ALIGN(ARCH_ઢગલો_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (કદ(શૂન્ય *)))

સંરચના dyn_obj {
	સંરચના k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	શૂન્ય *data;
};

બાહ્ય સંરચના k_object *z_object_gperf_find(અચલ શૂન્ય *obj);
બાહ્ય શૂન્ય z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     શૂન્ય *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
સ્થિર sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

સ્થિર size_t obj_size_get(ગણના k_objects otype)
{
	size_t ret;

	પસંદગી (otype) {
#અન્તર્ભાવ <zephyr/otype-to-size.h>
	મૂળભૂત:
		ret = કદ(અચલ સંરચના device);
		તોડો;
	}

	પાછા ret;
}

સ્થિર size_t obj_align_get(ગણના k_objects otype)
{
	size_t ret;

	પસંદગી (otype) {
	સ્થિતિ K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#નહીંતર
		ret = __alignof(સંરચના dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		તોડો;
	મૂળભૂત:
		ret = __alignof(સંરચના dyn_obj);
		તોડો;
	}

	પાછા ret;
}

સ્થિર સંરચના dyn_obj *dyn_object_find_locked(અચલ શૂન્ય *obj)
{
	સંરચના dyn_obj *ગાંઠ;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, ગાંઠ, dobj_list) {
		જો (ગાંઠ->kobj.name == obj) {
			પાછા ગાંઠ;
		}
	}

	પાછા ખાલી;
}

સ્થિર સંરચના dyn_obj *dyn_object_find(અચલ શૂન્ય *obj)
{
	સંરચના dyn_obj *ગાંઠ;
	k_spinlock_ચાવી_t ચાવી;

	ચાવી = k_spin_lock(&lists_lock);
	ગાંઠ = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, ચાવી);

	પાછા ગાંઠ;
}

સ્થિર શૂન્ય clear_perms_cb(સંરચના k_object *ko, શૂન્ય *ctx_ptr)
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
સ્થિર bool thread_idx_alloc(uintptr_t *tidx)
{
	પૂર્ણાંક i;
	પૂર્ણાંક idx;
	પૂર્ણાંક base;
	bool ret = false;
	k_spinlock_ચાવી_t ચાવી;

	base = 0;
	ચાવી = k_spin_lock(&lists_lock);
	માટે (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		જો (idx != 0) {
			સંરચના dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (શૂન્ય *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (શૂન્ય *)*tidx);
			}

			ret = true;
			તોડો;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, ચાવી);
	પાછા ret;
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
સ્થિર શૂન્ય thread_idx_free(uintptr_t tidx)
{
	સંરચના dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (શૂન્ય *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (શૂન્ય *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	પૂર્ણાંક base = tidx / અંક_BITS(_thread_idx_map[0]);
	પૂર્ણાંક offset = tidx % અંક_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, ચાવી);
}

સ્થિર સંરચના k_object *dynamic_object_create(ગણના k_objects otype, size_t align,
					      size_t size)
{
	સંરચના dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, કદ(સંરચના dyn_obj));
	જો (dyn == ખાલી) {
		પાછા ખાલી;
	}

	જો (otype == K_OBJ_THREAD_ઢગલો_ELEMENT) {
		size_t adjusted_size;

		જો (size == 0) {
			k_free(dyn);
			પાછા ખાલી;
		}

		adjusted_size = ઢગલો_ELEMENT_DATA_SIZE(size);
		જો (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			પાછા ખાલી;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_ઢગલો,
						     adjusted_size);
		જો (dyn->data == ખાલી) {
			k_free(dyn);
			પાછા ખાલી;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ઢગલો_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		જો (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		સંરચના z_ઢગલો_data *ઢગલો_data = (સંરચના z_ઢગલો_data *)
			((uint8_t *)dyn->data + adjusted_size - કદ(*ઢગલો_data));
#જો defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ઢગલો_data->priv = (શૂન્ય *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_ઢગલો_OBJ_ALIGN(size));
#નહીંતર
		ઢગલો_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ઢગલો_data->size = adjusted_size;
		dyn->kobj.data.ઢગલો_data = ઢગલો_data;
		dyn->kobj.name = dyn->data;
#નહીંતર
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.ઢગલો_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} નહીંતર {
		size_t total_size = obj_size_get(otype) + size;

		જો (total_size < size) {
			k_free(dyn);
			પાછા ખાલી;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		જો (dyn->data == ખાલી) {
			k_free(dyn);
			પાછા ખાલી;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(શૂન્ય)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, ચાવી);

	પાછા &dyn->kobj;
}

સંરચના k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	સંરચના k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	જો (obj == ખાલી) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	પાછા obj;
}

સ્થિર શૂન્ય *z_object_alloc(ગણના k_objects otype, size_t size)
{
	સંરચના k_object *zo;
	uintptr_t tidx = 0;

	જો ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		પાછા ખાલી;
	}

	પસંદગી (otype) {
	સ્થિતિ K_OBJ_THREAD:
		જો (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			પાછા ખાલી;
		}
		તોડો;
	/* The following are currently not allowed at all */
	સ્થિતિ K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	સ્થિતિ K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		પાછા ખાલી;
	મૂળભૂત:
		/* Remainder within bounds are permitted */
		તોડો;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	જો (zo == ખાલી) {
		જો (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		પાછા ખાલી;
	}

	જો (otype == K_OBJ_THREAD) {
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

	પાછા zo->name;
}

શૂન્ય *z_impl_k_object_alloc(ગણના k_objects otype)
{
	પાછા z_object_alloc(otype, 0);
}

શૂન્ય *z_impl_k_object_alloc_size(ગણના k_objects otype, size_t size)
{
	પાછા z_object_alloc(otype, size);
}

શૂન્ય k_object_free(શૂન્ય *obj)
{
	સંરચના dyn_obj *dyn = ખાલી;
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
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	જો (dyn != ખાલી) {
		sys_dlist_remove(&dyn->dobj_list);

		જો (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, ચાવી);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	જો (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	જો (dyn != ખાલી) {
#ifdef CONFIG_OBJ_CORE
		જો (dyn->kobj.type != K_OBJ_THREAD_ઢગલો_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ઢગલો_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		જો (dyn->kobj.type == K_OBJ_THREAD_ઢગલો_ELEMENT) {
			uint8_t *ઢગલો;

			જો (sys_cache_is_ptr_cached(dyn->data)) {
				ઢગલો = sys_cache_uncached_ptr_get(dyn->data);
			} નહીંતર {
				ઢગલો = dyn->data;
			}

			k_free(ઢગલો);
		} નહીંતર
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

સંરચના k_object *k_object_find(અચલ શૂન્ય *obj)
{
	સંરચના k_object *ret;

	ret = z_object_gperf_find(obj);

	જો (ret == ખાલી) {
		સંરચના dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		જો (dyn != ખાલી) {
			ret = &dyn->kobj;
		}
	}

	પાછા ret;
}

શૂન્ય k_object_wordlist_foreach(_wordlist_cb_func_t func, શૂન્ય *context)
{
	સંરચના dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, ચાવી);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak સંરચના k_object *z_object_gperf_find(અચલ શૂન્ય *obj)
{
	ARG_UNUSED(obj);
	પાછા ખાલી;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak શૂન્ય z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, શૂન્ય *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#નહીંતર
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak સંરચના k_object *k_object_find(અચલ શૂન્ય *obj)
{
	પાછા ખાલી;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak શૂન્ય k_object_wordlist_foreach(_wordlist_cb_func_t func, શૂન્ય *context)
{
}
#endif

સ્થિર unsigned પૂર્ણાંક thread_index_get(સંરચના k_thread *thread)
{
	સંરચના k_object *ko;

	ko = k_object_find(thread);

	જો (ko == ખાલી) {
		પાછા -1;
	}

	પાછા ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
સ્થિર શૂન્ય unref_check(સંરચના k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	જો ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		જાઓ out;
	}

	શૂન્ય *vko = ko;

	સંરચના dyn_obj *dyn = CONTAINER_OF(vko, સંરચના dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, સંરચના dyn_obj), "unaligned z_object");

	માટે (પૂર્ણાંક i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		જો (ko->perms[i] != 0U) {
			જાઓ out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	પસંદગી (ko->type) {
	સ્થિતિ K_OBJ_MSGQ:
		(શૂન્ય)z_msgq_cleanup((સંરચના k_msgq *)ko->name, locked);
		તોડો;
	સ્થિતિ K_OBJ_ઢગલો:
		(શૂન્ય)z_ઢગલો_cleanup((સંરચના k_ઢગલો *)ko->name, locked);
		તોડો;
	સ્થિતિ K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		જો ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(શૂન્ય)z_timer_cleanup((સંરચના k_timer *)ko->name, locked);
		}
		તોડો;
	મૂળભૂત:
		/* Nothing to do */
		તોડો;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, ચાવી);
}

સ્થિર શૂન્ય wordlist_cb(સંરચના k_object *ko, શૂન્ય *ctx_ptr)
{
	સંરચના perm_ctx *ctx = (સંરચના perm_ctx *)ctx_ptr;

	જો (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((સંરચના k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

શૂન્ય k_thread_perms_inherit(સંરચના k_thread *parent, સંરચના k_thread *child)
{
	સંરચના perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	જો ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

શૂન્ય k_thread_perms_set(સંરચના k_object *ko, સંરચના k_thread *thread)
{
	પૂર્ણાંક index = thread_index_get(thread);

	જો (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

શૂન્ય k_thread_perms_clear(સંરચના k_object *ko, સંરચના k_thread *thread)
{
	પૂર્ણાંક index = thread_index_get(thread);

	જો (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_ચાવી_t ચાવી = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, ચાવી);
#નહીંતર
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

સ્થિર શૂન્ય clear_perms_cb_locked(સંરચના k_object *ko, શૂન્ય *ctx_ptr)
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
શૂન્ય k_thread_perms_all_clear(સંરચના k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	જો ((પૂર્ણાંક)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (શૂન્ય *)index);
	}
}

સ્થિર પૂર્ણાંક thread_perms_test(સંરચના k_object *ko)
{
	પૂર્ણાંક index;

	જો ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		પાછા 1;
	}

	index = thread_index_get(_current);
	જો (index != -1) {
		પાછા sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	પાછા 0;
}

સ્થિર શૂન્ય dump_permission_error(સંરચના k_object *ko)
{
	પૂર્ણાંક index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, કદ(ko->perms), "permission bitmap");
}

શૂન્ય k_object_dump_error(પૂર્ણાંક retval, અચલ શૂન્ય *obj, સંરચના k_object *ko,
			ગણના k_objects otype)
{
	પસંદગી (retval) {
	સ્થિતિ -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		જો (ko == ખાલી) {
			LOG_ERR("address is not a known kernel object");
		} નહીંતર {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		તોડો;
	સ્થિતિ -EPERM:
		dump_permission_error(ko);
		તોડો;
	સ્થિતિ -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		તોડો;
	સ્થિતિ -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		તોડો;
	મૂળભૂત:
		/* Not handled error */
		તોડો;
	}
}

શૂન્ય z_impl_k_object_access_grant(અચલ શૂન્ય *object, સંરચના k_thread *thread)
{
	સંરચના k_object *ko = k_object_find(object);

	જો (ko != ખાલી) {
		k_thread_perms_set(ko, thread);
	}
}

શૂન્ય k_object_access_revoke(અચલ શૂન્ય *object, સંરચના k_thread *thread)
{
	સંરચના k_object *ko = k_object_find(object);

	જો (ko != ખાલી) {
		k_thread_perms_clear(ko, thread);
	}
}

શૂન્ય k_object_access_revoke_others(અચલ શૂન્ય *object)
{
	સંરચના k_object *ko = k_object_find(object);

	જો (ko != ખાલી) {
		(શૂન્ય)memset(ko->perms, 0, કદ(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

શૂન્ય z_impl_k_object_release(અચલ શૂન્ય *object)
{
	k_object_access_revoke(object, _current);
}

શૂન્ય k_object_access_all_grant(અચલ શૂન્ય *object)
{
	સંરચના k_object *ko = k_object_find(object);

	જો (ko != ખાલી) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

પૂર્ણાંક k_object_validate(સંરચના k_object *ko, ગણના k_objects otype,
		       ગણના _obj_init_check init)
{
	જો (unlikely(ko == ખાલી)) {
		પાછા -EBADF;
	}

	જો (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		જો ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			પાછા -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	જો (unlikely(thread_perms_test(ko) == 0)) {
		પાછા -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	જો (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		જો (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			પાછા -EINVAL;
		}
	} નહીંતર જો (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		જો (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			પાછા -EADDRINUSE;
		}
	} નહીંતર {
		/* _OBJ_INIT_ANY */
	}

	પાછા 0;
}

પૂર્ણાંક z_impl_k_object_access_check(અચલ શૂન્ય *object)
{
	પાછા k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

શૂન્ય k_object_init(અચલ શૂન્ય *obj)
{
	સંરચના k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	જો (ko == ખાલી) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		પાછા;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

શૂન્ય k_object_recycle(અચલ શૂન્ય *obj)
{
	સંરચના k_object *ko = k_object_find(obj);

	જો (ko != ખાલી) {
		(શૂન્ય)memset(ko->perms, 0, કદ(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

શૂન્ય k_object_uninit(અચલ શૂન્ય *obj)
{
	સંરચના k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	જો (ko == ખાલી) {
		પાછા;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
શૂન્ય *k_usermode_alloc_from_copy(અચલ શૂન્ય *src, size_t size)
{
	શૂન્ય *dst = ખાલી;

	/* Does the caller in user mode have access to read this memory? */
	જો (K_SYSCALL_MEMORY_READ(src, size)) {
		જાઓ out_err;
	}

	dst = z_thread_malloc(size);
	જો (dst == ખાલી) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		જાઓ out_err;
	}

	(શૂન્ય)memcpy(dst, src, size);
out_err:
	પાછા dst;
}

સ્થિર પૂર્ણાંક user_copy(શૂન્ય *dst, અચલ શૂન્ય *src, size_t size, bool to_user)
{
	પૂર્ણાંક ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	જો (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		જાઓ out_err;
	}

	(શૂન્ય)memcpy(dst, src, size);
	ret = 0;
out_err:
	પાછા ret;
}

પૂર્ણાંક k_usermode_from_copy(શૂન્ય *dst, અચલ શૂન્ય *src, size_t size)
{
	પાછા user_copy(dst, src, size, false);
}

પૂર્ણાંક k_usermode_to_copy(શૂન્ય *dst, અચલ શૂન્ય *src, size_t size)
{
	પાછા user_copy(dst, src, size, true);
}

અક્ષર *k_usermode_string_alloc_copy(અચલ અક્ષર *src, size_t maxlen)
{
	size_t actual_len;
	પૂર્ણાંક err;
	અક્ષર *ret = ખાલી;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	જો (err != 0) {
		જાઓ out;
	}
	જો (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		જાઓ out;
	}
	જો (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		જાઓ out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	જો (ret != ખાલી) {
		ret[actual_len - 1U] = '\0';
	}
out:
	પાછા ret;
}

પૂર્ણાંક k_usermode_string_copy(અક્ષર *dst, અચલ અક્ષર *src, size_t maxlen)
{
	size_t actual_len;
	પૂર્ણાંક ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	જો (err != 0) {
		ret = EFAULT;
		જાઓ out;
	}
	જો (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		જાઓ out;
	}
	જો (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		જાઓ out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	પાછા ret;
}

/*
 * Application memory region initialization
 */

બાહ્ય અક્ષર __app_shmem_regions_start[];
બાહ્ય અક્ષર __app_shmem_regions_end[];

સ્થિર શૂન્ય app_shmem_bss_zero(શૂન્ય)
{
	સંરચના z_app_region *region, *end;


	end = (સંરચના z_app_region *)&__app_shmem_regions_end[0];
	region = (સંરચના z_app_region *)&__app_shmem_regions_start[0];

	માટે ( ; region < end; region++) {
		(શૂન્ય)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

સ્થિર uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     શૂન્ય *ssf)
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

સ્થિર uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, શૂન્ય *ssf)
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

#અન્તર્ભાવ <zephyr/syscall_dispatch.c>
