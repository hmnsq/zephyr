/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/rb.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/sys_io.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/syscall.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/device.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <zephyr/app_memory/app_memdomain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/libc-hooks.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/mutex.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/util.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/linker-defs.h>
#ಅನ್ತರ್ಭಾವ <zephyr/cache.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>

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

#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
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
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#ಯದಿ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define ರಾಶಿ_ELEMENT_DATA_SIZE(size) \
	(ಪರಿಮಾಣ(ಸಂರಚನಾ z_ರಾಶಿ_data) + CONFIG_PRIVILEGED_ರಾಶಿ_SIZE + \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(size) + K_THREAD_ರಾಶಿ_LEN(size))
#ಅನ್ಯಥಾ
#define ರಾಶಿ_ELEMENT_DATA_SIZE(size) (ಪರಿಮಾಣ(ಸಂರಚನಾ z_ರಾಶಿ_data) + \
	K_THREAD_ರಾಶಿ_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#ಅನ್ಯಥಾ
#define ರಾಶಿ_ELEMENT_DATA_SIZE(size) K_THREAD_ರಾಶಿ_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
ಬಾಹ್ಯ uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ z_msgq_cleanup(ಸಂರಚನಾ k_msgq *q, bool locked);
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ z_ರಾಶಿ_cleanup(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ, bool locked);
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ z_timer_cleanup(ಸಂರಚನಾ k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

ಸ್ಥಿರ ಶೂನ್ಯ unref_check(ಸಂರಚನಾ k_object *ko, uintptr_t index, bool locked);

ಧ್ರುವ ವರ್ಣ *otype_to_str(ಗಣನಾ k_objects otype)
{
	ಧ್ರುವ ವರ್ಣ *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	ವಿಕಲ್ಪ (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	ಪಕ್ಷ K_OBJ_ANY:
		ret = "generic";
		ಭಙ್ಗ;
	ಪಕ್ಷ K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		ಭಙ್ಗ;
#ಅನ್ತರ್ಭಾವ <zephyr/otype-to-str.h>
	ಸ್ವಭಾವ:
		ret = "?";
		ಭಙ್ಗ;
	}
#ಅನ್ಯಥಾ
	ARG_UNUSED(otype);
	ret = ರಿಕ್ತ;
#endif /* CONFIG_LOG */
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸಂರಚನಾ perm_ctx {
	ಪೂರ್ಣಾಙ್ಕ parent_id;
	ಪೂರ್ಣಾಙ್ಕ child_id;
	ಸಂರಚನಾ k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ರಾಶಿ_find(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ಸಂರಚನಾ k_object *obj = k_object_find(ರಾಶಿ);

	__ASSERT(obj != ರಿಕ್ತ, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ರಾಶಿ_ELEMENT,
		 "bad stack object");

	ಪ್ರತ್ಯಾಗಮ obj->data.ರಾಶಿ_data->priv;
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
#ಅನ್ಯಥಾ
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(ಪರಿಮಾಣ(ಶೂನ್ಯ *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_ರಾಶಿ_SIZE
#ಯದಿ defined(CONFIG_MPU_ರಾಶಿ_GUARD) || defined(CONFIG_PMP_ರಾಶಿ_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ರಾಶಿ \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ರಾಶಿ_SIZE)
#ಅನ್ಯಥಾ
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ರಾಶಿ \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(CONFIG_PRIVILEGED_ರಾಶಿ_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#ಅನ್ಯಥಾ
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ರಾಶಿ \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(ARCH_ರಾಶಿ_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (ಪರಿಮಾಣ(ಶೂನ್ಯ *)))

ಸಂರಚನಾ dyn_obj {
	ಸಂರಚನಾ k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	ಶೂನ್ಯ *data;
};

ಬಾಹ್ಯ ಸಂರಚನಾ k_object *z_object_gperf_find(ಧ್ರುವ ಶೂನ್ಯ *obj);
ಬಾಹ್ಯ ಶೂನ್ಯ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     ಶೂನ್ಯ *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
ಸ್ಥಿರ sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

ಸ್ಥಿರ size_t obj_size_get(ಗಣನಾ k_objects otype)
{
	size_t ret;

	ವಿಕಲ್ಪ (otype) {
#ಅನ್ತರ್ಭಾವ <zephyr/otype-to-size.h>
	ಸ್ವಭಾವ:
		ret = ಪರಿಮಾಣ(ಧ್ರುವ ಸಂರಚನಾ device);
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ size_t obj_align_get(ಗಣನಾ k_objects otype)
{
	size_t ret;

	ವಿಕಲ್ಪ (otype) {
	ಪಕ್ಷ K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#ಅನ್ಯಥಾ
		ret = __alignof(ಸಂರಚನಾ dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ret = __alignof(ಸಂರಚನಾ dyn_obj);
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಸಂರಚನಾ dyn_obj *dyn_object_find_locked(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಸಂರಚನಾ dyn_obj *ಗಂಟು;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, ಗಂಟು, dobj_list) {
		ಯದಿ (ಗಂಟು->kobj.name == obj) {
			ಪ್ರತ್ಯಾಗಮ ಗಂಟು;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಸಂರಚನಾ dyn_obj *dyn_object_find(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಸಂರಚನಾ dyn_obj *ಗಂಟು;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&lists_lock);
	ಗಂಟು = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ ಗಂಟು;
}

ಸ್ಥಿರ ಶೂನ್ಯ clear_perms_cb(ಸಂರಚನಾ k_object *ko, ಶೂನ್ಯ *ctx_ptr)
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
ಸ್ಥಿರ bool thread_idx_alloc(uintptr_t *tidx)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ idx;
	ಪೂರ್ಣಾಙ್ಕ base;
	bool ret = false;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	base = 0;
	ಕೀಲಿ = k_spin_lock(&lists_lock);
	ಪ್ರತಿ (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		ಯದಿ (idx != 0) {
			ಸಂರಚನಾ dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (ಶೂನ್ಯ *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (ಶೂನ್ಯ *)*tidx);
			}

			ret = true;
			ಭಙ್ಗ;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, ಕೀಲಿ);
	ಪ್ರತ್ಯಾಗಮ ret;
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
ಸ್ಥಿರ ಶೂನ್ಯ thread_idx_free(uintptr_t tidx)
{
	ಸಂರಚನಾ dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (ಶೂನ್ಯ *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (ಶೂನ್ಯ *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	ಪೂರ್ಣಾಙ್ಕ base = tidx / ಅಂಕೆ_BITS(_thread_idx_map[0]);
	ಪೂರ್ಣಾಙ್ಕ offset = tidx % ಅಂಕೆ_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, ಕೀಲಿ);
}

ಸ್ಥಿರ ಸಂರಚನಾ k_object *dynamic_object_create(ಗಣನಾ k_objects otype, size_t align,
					      size_t size)
{
	ಸಂರಚನಾ dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, ಪರಿಮಾಣ(ಸಂರಚನಾ dyn_obj));
	ಯದಿ (dyn == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (otype == K_OBJ_THREAD_ರಾಶಿ_ELEMENT) {
		size_t adjusted_size;

		ಯದಿ (size == 0) {
			k_free(dyn);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		adjusted_size = ರಾಶಿ_ELEMENT_DATA_SIZE(size);
		ಯದಿ (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_ರಾಶಿ,
						     adjusted_size);
		ಯದಿ (dyn->data == ರಿಕ್ತ) {
			k_free(dyn);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ರಾಶಿ_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		ಯದಿ (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		ಸಂರಚನಾ z_ರಾಶಿ_data *ರಾಶಿ_data = (ಸಂರಚನಾ z_ರಾಶಿ_data *)
			((uint8_t *)dyn->data + adjusted_size - ಪರಿಮಾಣ(*ರಾಶಿ_data));
#ಯದಿ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ರಾಶಿ_data->priv = (ಶೂನ್ಯ *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_ರಾಶಿ_OBJ_ALIGN(size));
#ಅನ್ಯಥಾ
		ರಾಶಿ_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ರಾಶಿ_data->size = adjusted_size;
		dyn->kobj.data.ರಾಶಿ_data = ರಾಶಿ_data;
		dyn->kobj.name = dyn->data;
#ಅನ್ಯಥಾ
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.ರಾಶಿ_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} ಅನ್ಯಥಾ {
		size_t total_size = obj_size_get(otype) + size;

		ಯದಿ (total_size < size) {
			k_free(dyn);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		ಯದಿ (dyn->data == ರಿಕ್ತ) {
			k_free(dyn);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(ಶೂನ್ಯ)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ &dyn->kobj;
}

ಸಂರಚನಾ k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	ಸಂರಚನಾ k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	ಯದಿ (obj == ರಿಕ್ತ) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	ಪ್ರತ್ಯಾಗಮ obj;
}

ಸ್ಥಿರ ಶೂನ್ಯ *z_object_alloc(ಗಣನಾ k_objects otype, size_t size)
{
	ಸಂರಚನಾ k_object *zo;
	uintptr_t tidx = 0;

	ಯದಿ ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ವಿಕಲ್ಪ (otype) {
	ಪಕ್ಷ K_OBJ_THREAD:
		ಯದಿ (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		ಭಙ್ಗ;
	/* The following are currently not allowed at all */
	ಪಕ್ಷ K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	ಪಕ್ಷ K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಸ್ವಭಾವ:
		/* Remainder within bounds are permitted */
		ಭಙ್ಗ;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	ಯದಿ (zo == ರಿಕ್ತ) {
		ಯದಿ (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (otype == K_OBJ_THREAD) {
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

	ಪ್ರತ್ಯಾಗಮ zo->name;
}

ಶೂನ್ಯ *z_impl_k_object_alloc(ಗಣನಾ k_objects otype)
{
	ಪ್ರತ್ಯಾಗಮ z_object_alloc(otype, 0);
}

ಶೂನ್ಯ *z_impl_k_object_alloc_size(ಗಣನಾ k_objects otype, size_t size)
{
	ಪ್ರತ್ಯಾಗಮ z_object_alloc(otype, size);
}

ಶೂನ್ಯ k_object_free(ಶೂನ್ಯ *obj)
{
	ಸಂರಚನಾ dyn_obj *dyn = ರಿಕ್ತ;
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
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	ಯದಿ (dyn != ರಿಕ್ತ) {
		sys_dlist_remove(&dyn->dobj_list);

		ಯದಿ (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, ಕೀಲಿ);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	ಯದಿ (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	ಯದಿ (dyn != ರಿಕ್ತ) {
#ifdef CONFIG_OBJ_CORE
		ಯದಿ (dyn->kobj.type != K_OBJ_THREAD_ರಾಶಿ_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ರಾಶಿ_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		ಯದಿ (dyn->kobj.type == K_OBJ_THREAD_ರಾಶಿ_ELEMENT) {
			uint8_t *ರಾಶಿ;

			ಯದಿ (sys_cache_is_ptr_cached(dyn->data)) {
				ರಾಶಿ = sys_cache_uncached_ptr_get(dyn->data);
			} ಅನ್ಯಥಾ {
				ರಾಶಿ = dyn->data;
			}

			k_free(ರಾಶಿ);
		} ಅನ್ಯಥಾ
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

ಸಂರಚನಾ k_object *k_object_find(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಸಂರಚನಾ k_object *ret;

	ret = z_object_gperf_find(obj);

	ಯದಿ (ret == ರಿಕ್ತ) {
		ಸಂರಚನಾ dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		ಯದಿ (dyn != ರಿಕ್ತ) {
			ret = &dyn->kobj;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ k_object_wordlist_foreach(_wordlist_cb_func_t func, ಶೂನ್ಯ *context)
{
	ಸಂರಚನಾ dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, ಕೀಲಿ);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ಸಂರಚನಾ k_object *z_object_gperf_find(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ARG_UNUSED(obj);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ಶೂನ್ಯ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, ಶೂನ್ಯ *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#ಅನ್ಯಥಾ
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ಸಂರಚನಾ k_object *k_object_find(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ಶೂನ್ಯ k_object_wordlist_foreach(_wordlist_cb_func_t func, ಶೂನ್ಯ *context)
{
}
#endif

ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ thread_index_get(ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ k_object *ko;

	ko = k_object_find(thread);

	ಯದಿ (ko == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಪ್ರತ್ಯಾಗಮ ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ಸ್ಥಿರ ಶೂನ್ಯ unref_check(ಸಂರಚನಾ k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	ಯದಿ ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		ಗಚ್ಛ out;
	}

	ಶೂನ್ಯ *vko = ko;

	ಸಂರಚನಾ dyn_obj *dyn = CONTAINER_OF(vko, ಸಂರಚನಾ dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, ಸಂರಚನಾ dyn_obj), "unaligned z_object");

	ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		ಯದಿ (ko->perms[i] != 0U) {
			ಗಚ್ಛ out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	ವಿಕಲ್ಪ (ko->type) {
	ಪಕ್ಷ K_OBJ_MSGQ:
		(ಶೂನ್ಯ)z_msgq_cleanup((ಸಂರಚನಾ k_msgq *)ko->name, locked);
		ಭಙ್ಗ;
	ಪಕ್ಷ K_OBJ_ರಾಶಿ:
		(ಶೂನ್ಯ)z_ರಾಶಿ_cleanup((ಸಂರಚನಾ k_ರಾಶಿ *)ko->name, locked);
		ಭಙ್ಗ;
	ಪಕ್ಷ K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		ಯದಿ ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(ಶೂನ್ಯ)z_timer_cleanup((ಸಂರಚನಾ k_timer *)ko->name, locked);
		}
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		/* Nothing to do */
		ಭಙ್ಗ;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, ಕೀಲಿ);
}

ಸ್ಥಿರ ಶೂನ್ಯ wordlist_cb(ಸಂರಚನಾ k_object *ko, ಶೂನ್ಯ *ctx_ptr)
{
	ಸಂರಚನಾ perm_ctx *ctx = (ಸಂರಚನಾ perm_ctx *)ctx_ptr;

	ಯದಿ (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((ಸಂರಚನಾ k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

ಶೂನ್ಯ k_thread_perms_inherit(ಸಂರಚನಾ k_thread *parent, ಸಂರಚನಾ k_thread *child)
{
	ಸಂರಚನಾ perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	ಯದಿ ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

ಶೂನ್ಯ k_thread_perms_set(ಸಂರಚನಾ k_object *ko, ಸಂರಚನಾ k_thread *thread)
{
	ಪೂರ್ಣಾಙ್ಕ index = thread_index_get(thread);

	ಯದಿ (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

ಶೂನ್ಯ k_thread_perms_clear(ಸಂರಚನಾ k_object *ko, ಸಂರಚನಾ k_thread *thread)
{
	ಪೂರ್ಣಾಙ್ಕ index = thread_index_get(thread);

	ಯದಿ (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, ಕೀಲಿ);
#ಅನ್ಯಥಾ
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ clear_perms_cb_locked(ಸಂರಚನಾ k_object *ko, ಶೂನ್ಯ *ctx_ptr)
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
ಶೂನ್ಯ k_thread_perms_all_clear(ಸಂರಚನಾ k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (ಶೂನ್ಯ *)index);
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ thread_perms_test(ಸಂರಚನಾ k_object *ko)
{
	ಪೂರ್ಣಾಙ್ಕ index;

	ಯದಿ ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	index = thread_index_get(_current);
	ಯದಿ (index != -1) {
		ಪ್ರತ್ಯಾಗಮ sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_permission_error(ಸಂರಚನಾ k_object *ko)
{
	ಪೂರ್ಣಾಙ್ಕ index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, ಪರಿಮಾಣ(ko->perms), "permission bitmap");
}

ಶೂನ್ಯ k_object_dump_error(ಪೂರ್ಣಾಙ್ಕ retval, ಧ್ರುವ ಶೂನ್ಯ *obj, ಸಂರಚನಾ k_object *ko,
			ಗಣನಾ k_objects otype)
{
	ವಿಕಲ್ಪ (retval) {
	ಪಕ್ಷ -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		ಯದಿ (ko == ರಿಕ್ತ) {
			LOG_ERR("address is not a known kernel object");
		} ಅನ್ಯಥಾ {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ -EPERM:
		dump_permission_error(ko);
		ಭಙ್ಗ;
	ಪಕ್ಷ -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		ಭಙ್ಗ;
	ಪಕ್ಷ -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		/* Not handled error */
		ಭಙ್ಗ;
	}
}

ಶೂನ್ಯ z_impl_k_object_access_grant(ಧ್ರುವ ಶೂನ್ಯ *object, ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ k_object *ko = k_object_find(object);

	ಯದಿ (ko != ರಿಕ್ತ) {
		k_thread_perms_set(ko, thread);
	}
}

ಶೂನ್ಯ k_object_access_revoke(ಧ್ರುವ ಶೂನ್ಯ *object, ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ k_object *ko = k_object_find(object);

	ಯದಿ (ko != ರಿಕ್ತ) {
		k_thread_perms_clear(ko, thread);
	}
}

ಶೂನ್ಯ k_object_access_revoke_others(ಧ್ರುವ ಶೂನ್ಯ *object)
{
	ಸಂರಚನಾ k_object *ko = k_object_find(object);

	ಯದಿ (ko != ರಿಕ್ತ) {
		(ಶೂನ್ಯ)memset(ko->perms, 0, ಪರಿಮಾಣ(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

ಶೂನ್ಯ z_impl_k_object_release(ಧ್ರುವ ಶೂನ್ಯ *object)
{
	k_object_access_revoke(object, _current);
}

ಶೂನ್ಯ k_object_access_all_grant(ಧ್ರುವ ಶೂನ್ಯ *object)
{
	ಸಂರಚನಾ k_object *ko = k_object_find(object);

	ಯದಿ (ko != ರಿಕ್ತ) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

ಪೂರ್ಣಾಙ್ಕ k_object_validate(ಸಂರಚನಾ k_object *ko, ಗಣನಾ k_objects otype,
		       ಗಣನಾ _obj_init_check init)
{
	ಯದಿ (unlikely(ko == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ -EBADF;
	}

	ಯದಿ (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		ಯದಿ ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			ಪ್ರತ್ಯಾಗಮ -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	ಯದಿ (unlikely(thread_perms_test(ko) == 0)) {
		ಪ್ರತ್ಯಾಗಮ -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	ಯದಿ (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		ಯದಿ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			ಪ್ರತ್ಯಾಗಮ -EINVAL;
		}
	} ಅನ್ಯಥಾ ಯದಿ (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		ಯದಿ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			ಪ್ರತ್ಯಾಗಮ -EADDRINUSE;
		}
	} ಅನ್ಯಥಾ {
		/* _OBJ_INIT_ANY */
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_object_access_check(ಧ್ರುವ ಶೂನ್ಯ *object)
{
	ಪ್ರತ್ಯಾಗಮ k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

ಶೂನ್ಯ k_object_init(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಸಂರಚನಾ k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	ಯದಿ (ko == ರಿಕ್ತ) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

ಶೂನ್ಯ k_object_recycle(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಸಂರಚನಾ k_object *ko = k_object_find(obj);

	ಯದಿ (ko != ರಿಕ್ತ) {
		(ಶೂನ್ಯ)memset(ko->perms, 0, ಪರಿಮಾಣ(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

ಶೂನ್ಯ k_object_uninit(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಸಂರಚನಾ k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	ಯದಿ (ko == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
ಶೂನ್ಯ *k_usermode_alloc_from_copy(ಧ್ರುವ ಶೂನ್ಯ *src, size_t size)
{
	ಶೂನ್ಯ *dst = ರಿಕ್ತ;

	/* Does the caller in user mode have access to read this memory? */
	ಯದಿ (K_SYSCALL_MEMORY_READ(src, size)) {
		ಗಚ್ಛ out_err;
	}

	dst = z_thread_malloc(size);
	ಯದಿ (dst == ರಿಕ್ತ) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		ಗಚ್ಛ out_err;
	}

	(ಶೂನ್ಯ)memcpy(dst, src, size);
out_err:
	ಪ್ರತ್ಯಾಗಮ dst;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ user_copy(ಶೂನ್ಯ *dst, ಧ್ರುವ ಶೂನ್ಯ *src, size_t size, bool to_user)
{
	ಪೂರ್ಣಾಙ್ಕ ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	ಯದಿ (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		ಗಚ್ಛ out_err;
	}

	(ಶೂನ್ಯ)memcpy(dst, src, size);
	ret = 0;
out_err:
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_usermode_from_copy(ಶೂನ್ಯ *dst, ಧ್ರುವ ಶೂನ್ಯ *src, size_t size)
{
	ಪ್ರತ್ಯಾಗಮ user_copy(dst, src, size, false);
}

ಪೂರ್ಣಾಙ್ಕ k_usermode_to_copy(ಶೂನ್ಯ *dst, ಧ್ರುವ ಶೂನ್ಯ *src, size_t size)
{
	ಪ್ರತ್ಯಾಗಮ user_copy(dst, src, size, true);
}

ವರ್ಣ *k_usermode_string_alloc_copy(ಧ್ರುವ ವರ್ಣ *src, size_t maxlen)
{
	size_t actual_len;
	ಪೂರ್ಣಾಙ್ಕ err;
	ವರ್ಣ *ret = ರಿಕ್ತ;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ಯದಿ (err != 0) {
		ಗಚ್ಛ out;
	}
	ಯದಿ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ಗಚ್ಛ out;
	}
	ಯದಿ (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ಗಚ್ಛ out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	ಯದಿ (ret != ರಿಕ್ತ) {
		ret[actual_len - 1U] = '\0';
	}
out:
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_usermode_string_copy(ವರ್ಣ *dst, ಧ್ರುವ ವರ್ಣ *src, size_t maxlen)
{
	size_t actual_len;
	ಪೂರ್ಣಾಙ್ಕ ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ಯದಿ (err != 0) {
		ret = EFAULT;
		ಗಚ್ಛ out;
	}
	ಯದಿ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		ಗಚ್ಛ out;
	}
	ಯದಿ (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		ಗಚ್ಛ out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	ಪ್ರತ್ಯಾಗಮ ret;
}

/*
 * Application memory region initialization
 */

ಬಾಹ್ಯ ವರ್ಣ __app_shmem_regions_start[];
ಬಾಹ್ಯ ವರ್ಣ __app_shmem_regions_end[];

ಸ್ಥಿರ ಶೂನ್ಯ app_shmem_bss_zero(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ z_app_region *region, *end;


	end = (ಸಂರಚನಾ z_app_region *)&__app_shmem_regions_end[0];
	region = (ಸಂರಚನಾ z_app_region *)&__app_shmem_regions_start[0];

	ಪ್ರತಿ ( ; region < end; region++) {
		(ಶೂನ್ಯ)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

ಸ್ಥಿರ uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     ಶೂನ್ಯ *ssf)
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

ಸ್ಥಿರ uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, ಶೂನ್ಯ *ssf)
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

#ಅನ್ತರ್ಭಾವ <zephyr/syscall_dispatch.c>
