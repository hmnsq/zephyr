/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/sys/rb.h>
#अन्तर्भाव <zephyr/sys/sys_io.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/syscall.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/device.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <zephyr/app_memory/app_memdomain.h>
#अन्तर्भाव <zephyr/sys/libc-hooks.h>
#अन्तर्भाव <zephyr/sys/mutex.h>
#अन्तर्भाव <zephyr/sys/util.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <zephyr/linker/linker-defs.h>
#अन्तर्भाव <zephyr/cache.h>
#अन्तर्भाव <kernel_internal.h>

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

#अन्तर्भाव <zephyr/logging/log.h>
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
स्थिर संरचना k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#यदि defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define ढेर_ELEMENT_DATA_SIZE(size) \
	(परिमाण(संरचना z_ढेर_data) + CONFIG_PRIVILEGED_ढेर_SIZE + \
	Z_THREAD_ढेर_OBJ_ALIGN(size) + K_THREAD_ढेर_LEN(size))
#अन्यथा
#define ढेर_ELEMENT_DATA_SIZE(size) (परिमाण(संरचना z_ढेर_data) + \
	K_THREAD_ढेर_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#अन्यथा
#define ढेर_ELEMENT_DATA_SIZE(size) K_THREAD_ढेर_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
स्थिर संरचना k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
बाह्य uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

बाह्य पूर्णाङ्क z_msgq_cleanup(संरचना k_msgq *q, bool locked);
बाह्य पूर्णाङ्क z_ढेर_cleanup(संरचना k_ढेर *ढेर, bool locked);
बाह्य पूर्णाङ्क z_timer_cleanup(संरचना k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

स्थिर शून्य unref_check(संरचना k_object *ko, uintptr_t index, bool locked);

ध्रुव वर्ण *otype_to_str(गणना k_objects otype)
{
	ध्रुव वर्ण *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	विकल्प (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	पक्ष K_OBJ_ANY:
		ret = "generic";
		भङ्ग;
	पक्ष K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		भङ्ग;
#अन्तर्भाव <zephyr/otype-to-str.h>
	स्वभाव:
		ret = "?";
		भङ्ग;
	}
#अन्यथा
	ARG_UNUSED(otype);
	ret = रिक्त;
#endif /* CONFIG_LOG */
	प्रत्यागम ret;
}

संरचना perm_ctx {
	पूर्णाङ्क parent_id;
	पूर्णाङ्क child_id;
	संरचना k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ढेर_find(k_thread_ढेर_t *ढेर)
{
	संरचना k_object *obj = k_object_find(ढेर);

	__ASSERT(obj != रिक्त, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ढेर_ELEMENT,
		 "bad stack object");

	प्रत्यागम obj->data.ढेर_data->priv;
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
#अन्यथा
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(परिमाण(शून्य *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_ढेर_SIZE
#यदि defined(CONFIG_MPU_ढेर_GUARD) || defined(CONFIG_PMP_ढेर_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ढेर \
	Z_THREAD_ढेर_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ढेर_SIZE)
#अन्यथा
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ढेर \
	Z_THREAD_ढेर_OBJ_ALIGN(CONFIG_PRIVILEGED_ढेर_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#अन्यथा
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ढेर \
	Z_THREAD_ढेर_OBJ_ALIGN(ARCH_ढेर_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (परिमाण(शून्य *)))

संरचना dyn_obj {
	संरचना k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	शून्य *data;
};

बाह्य संरचना k_object *z_object_gperf_find(ध्रुव शून्य *obj);
बाह्य शून्य z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     शून्य *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
स्थिर sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

स्थिर size_t obj_size_get(गणना k_objects otype)
{
	size_t ret;

	विकल्प (otype) {
#अन्तर्भाव <zephyr/otype-to-size.h>
	स्वभाव:
		ret = परिमाण(ध्रुव संरचना device);
		भङ्ग;
	}

	प्रत्यागम ret;
}

स्थिर size_t obj_align_get(गणना k_objects otype)
{
	size_t ret;

	विकल्प (otype) {
	पक्ष K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#अन्यथा
		ret = __alignof(संरचना dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		भङ्ग;
	स्वभाव:
		ret = __alignof(संरचना dyn_obj);
		भङ्ग;
	}

	प्रत्यागम ret;
}

स्थिर संरचना dyn_obj *dyn_object_find_locked(ध्रुव शून्य *obj)
{
	संरचना dyn_obj *गाँठ;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, गाँठ, dobj_list) {
		यदि (गाँठ->kobj.name == obj) {
			प्रत्यागम गाँठ;
		}
	}

	प्रत्यागम रिक्त;
}

स्थिर संरचना dyn_obj *dyn_object_find(ध्रुव शून्य *obj)
{
	संरचना dyn_obj *गाँठ;
	k_spinlock_कुंजी_t कुंजी;

	कुंजी = k_spin_lock(&lists_lock);
	गाँठ = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, कुंजी);

	प्रत्यागम गाँठ;
}

स्थिर शून्य clear_perms_cb(संरचना k_object *ko, शून्य *ctx_ptr)
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
स्थिर bool thread_idx_alloc(uintptr_t *tidx)
{
	पूर्णाङ्क i;
	पूर्णाङ्क idx;
	पूर्णाङ्क base;
	bool ret = false;
	k_spinlock_कुंजी_t कुंजी;

	base = 0;
	कुंजी = k_spin_lock(&lists_lock);
	प्रति (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		यदि (idx != 0) {
			संरचना dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (शून्य *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (शून्य *)*tidx);
			}

			ret = true;
			भङ्ग;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, कुंजी);
	प्रत्यागम ret;
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
स्थिर शून्य thread_idx_free(uintptr_t tidx)
{
	संरचना dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (शून्य *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (शून्य *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	पूर्णाङ्क base = tidx / अंक_BITS(_thread_idx_map[0]);
	पूर्णाङ्क offset = tidx % अंक_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, कुंजी);
}

स्थिर संरचना k_object *dynamic_object_create(गणना k_objects otype, size_t align,
					      size_t size)
{
	संरचना dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, परिमाण(संरचना dyn_obj));
	यदि (dyn == रिक्त) {
		प्रत्यागम रिक्त;
	}

	यदि (otype == K_OBJ_THREAD_ढेर_ELEMENT) {
		size_t adjusted_size;

		यदि (size == 0) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

		adjusted_size = ढेर_ELEMENT_DATA_SIZE(size);
		यदि (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_ढेर,
						     adjusted_size);
		यदि (dyn->data == रिक्त) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ढेर_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		यदि (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		संरचना z_ढेर_data *ढेर_data = (संरचना z_ढेर_data *)
			((uint8_t *)dyn->data + adjusted_size - परिमाण(*ढेर_data));
#यदि defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ढेर_data->priv = (शून्य *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_ढेर_OBJ_ALIGN(size));
#अन्यथा
		ढेर_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ढेर_data->size = adjusted_size;
		dyn->kobj.data.ढेर_data = ढेर_data;
		dyn->kobj.name = dyn->data;
#अन्यथा
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.ढेर_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} अन्यथा {
		size_t total_size = obj_size_get(otype) + size;

		यदि (total_size < size) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		यदि (dyn->data == रिक्त) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(शून्य)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, कुंजी);

	प्रत्यागम &dyn->kobj;
}

संरचना k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	संरचना k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	यदि (obj == रिक्त) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	प्रत्यागम obj;
}

स्थिर शून्य *z_object_alloc(गणना k_objects otype, size_t size)
{
	संरचना k_object *zo;
	uintptr_t tidx = 0;

	यदि ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		प्रत्यागम रिक्त;
	}

	विकल्प (otype) {
	पक्ष K_OBJ_THREAD:
		यदि (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			प्रत्यागम रिक्त;
		}
		भङ्ग;
	/* The following are currently not allowed at all */
	पक्ष K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	पक्ष K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		प्रत्यागम रिक्त;
	स्वभाव:
		/* Remainder within bounds are permitted */
		भङ्ग;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	यदि (zo == रिक्त) {
		यदि (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		प्रत्यागम रिक्त;
	}

	यदि (otype == K_OBJ_THREAD) {
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

	प्रत्यागम zo->name;
}

शून्य *z_impl_k_object_alloc(गणना k_objects otype)
{
	प्रत्यागम z_object_alloc(otype, 0);
}

शून्य *z_impl_k_object_alloc_size(गणना k_objects otype, size_t size)
{
	प्रत्यागम z_object_alloc(otype, size);
}

शून्य k_object_free(शून्य *obj)
{
	संरचना dyn_obj *dyn = रिक्त;
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
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	यदि (dyn != रिक्त) {
		sys_dlist_remove(&dyn->dobj_list);

		यदि (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, कुंजी);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	यदि (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	यदि (dyn != रिक्त) {
#ifdef CONFIG_OBJ_CORE
		यदि (dyn->kobj.type != K_OBJ_THREAD_ढेर_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ढेर_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		यदि (dyn->kobj.type == K_OBJ_THREAD_ढेर_ELEMENT) {
			uint8_t *ढेर;

			यदि (sys_cache_is_ptr_cached(dyn->data)) {
				ढेर = sys_cache_uncached_ptr_get(dyn->data);
			} अन्यथा {
				ढेर = dyn->data;
			}

			k_free(ढेर);
		} अन्यथा
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

संरचना k_object *k_object_find(ध्रुव शून्य *obj)
{
	संरचना k_object *ret;

	ret = z_object_gperf_find(obj);

	यदि (ret == रिक्त) {
		संरचना dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		यदि (dyn != रिक्त) {
			ret = &dyn->kobj;
		}
	}

	प्रत्यागम ret;
}

शून्य k_object_wordlist_foreach(_wordlist_cb_func_t func, शून्य *context)
{
	संरचना dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, कुंजी);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak संरचना k_object *z_object_gperf_find(ध्रुव शून्य *obj)
{
	ARG_UNUSED(obj);
	प्रत्यागम रिक्त;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak शून्य z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, शून्य *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#अन्यथा
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak संरचना k_object *k_object_find(ध्रुव शून्य *obj)
{
	प्रत्यागम रिक्त;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak शून्य k_object_wordlist_foreach(_wordlist_cb_func_t func, शून्य *context)
{
}
#endif

स्थिर unsigned पूर्णाङ्क thread_index_get(संरचना k_thread *thread)
{
	संरचना k_object *ko;

	ko = k_object_find(thread);

	यदि (ko == रिक्त) {
		प्रत्यागम -1;
	}

	प्रत्यागम ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
स्थिर शून्य unref_check(संरचना k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	यदि ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		गच्छ out;
	}

	शून्य *vko = ko;

	संरचना dyn_obj *dyn = CONTAINER_OF(vko, संरचना dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, संरचना dyn_obj), "unaligned z_object");

	प्रति (पूर्णाङ्क i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		यदि (ko->perms[i] != 0U) {
			गच्छ out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	विकल्प (ko->type) {
	पक्ष K_OBJ_MSGQ:
		(शून्य)z_msgq_cleanup((संरचना k_msgq *)ko->name, locked);
		भङ्ग;
	पक्ष K_OBJ_ढेर:
		(शून्य)z_ढेर_cleanup((संरचना k_ढेर *)ko->name, locked);
		भङ्ग;
	पक्ष K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		यदि ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(शून्य)z_timer_cleanup((संरचना k_timer *)ko->name, locked);
		}
		भङ्ग;
	स्वभाव:
		/* Nothing to do */
		भङ्ग;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, कुंजी);
}

स्थिर शून्य wordlist_cb(संरचना k_object *ko, शून्य *ctx_ptr)
{
	संरचना perm_ctx *ctx = (संरचना perm_ctx *)ctx_ptr;

	यदि (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((संरचना k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

शून्य k_thread_perms_inherit(संरचना k_thread *parent, संरचना k_thread *child)
{
	संरचना perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	यदि ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

शून्य k_thread_perms_set(संरचना k_object *ko, संरचना k_thread *thread)
{
	पूर्णाङ्क index = thread_index_get(thread);

	यदि (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

शून्य k_thread_perms_clear(संरचना k_object *ko, संरचना k_thread *thread)
{
	पूर्णाङ्क index = thread_index_get(thread);

	यदि (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, कुंजी);
#अन्यथा
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

स्थिर शून्य clear_perms_cb_locked(संरचना k_object *ko, शून्य *ctx_ptr)
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
शून्य k_thread_perms_all_clear(संरचना k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	यदि ((पूर्णाङ्क)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (शून्य *)index);
	}
}

स्थिर पूर्णाङ्क thread_perms_test(संरचना k_object *ko)
{
	पूर्णाङ्क index;

	यदि ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		प्रत्यागम 1;
	}

	index = thread_index_get(_current);
	यदि (index != -1) {
		प्रत्यागम sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	प्रत्यागम 0;
}

स्थिर शून्य dump_permission_error(संरचना k_object *ko)
{
	पूर्णाङ्क index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, परिमाण(ko->perms), "permission bitmap");
}

शून्य k_object_dump_error(पूर्णाङ्क retval, ध्रुव शून्य *obj, संरचना k_object *ko,
			गणना k_objects otype)
{
	विकल्प (retval) {
	पक्ष -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		यदि (ko == रिक्त) {
			LOG_ERR("address is not a known kernel object");
		} अन्यथा {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		भङ्ग;
	पक्ष -EPERM:
		dump_permission_error(ko);
		भङ्ग;
	पक्ष -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		भङ्ग;
	पक्ष -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		भङ्ग;
	स्वभाव:
		/* Not handled error */
		भङ्ग;
	}
}

शून्य z_impl_k_object_access_grant(ध्रुव शून्य *object, संरचना k_thread *thread)
{
	संरचना k_object *ko = k_object_find(object);

	यदि (ko != रिक्त) {
		k_thread_perms_set(ko, thread);
	}
}

शून्य k_object_access_revoke(ध्रुव शून्य *object, संरचना k_thread *thread)
{
	संरचना k_object *ko = k_object_find(object);

	यदि (ko != रिक्त) {
		k_thread_perms_clear(ko, thread);
	}
}

शून्य k_object_access_revoke_others(ध्रुव शून्य *object)
{
	संरचना k_object *ko = k_object_find(object);

	यदि (ko != रिक्त) {
		(शून्य)memset(ko->perms, 0, परिमाण(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

शून्य z_impl_k_object_release(ध्रुव शून्य *object)
{
	k_object_access_revoke(object, _current);
}

शून्य k_object_access_all_grant(ध्रुव शून्य *object)
{
	संरचना k_object *ko = k_object_find(object);

	यदि (ko != रिक्त) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

पूर्णाङ्क k_object_validate(संरचना k_object *ko, गणना k_objects otype,
		       गणना _obj_init_check init)
{
	यदि (unlikely(ko == रिक्त)) {
		प्रत्यागम -EBADF;
	}

	यदि (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		यदि ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			प्रत्यागम -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	यदि (unlikely(thread_perms_test(ko) == 0)) {
		प्रत्यागम -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	यदि (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		यदि (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			प्रत्यागम -EINVAL;
		}
	} अन्यथा यदि (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		यदि (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			प्रत्यागम -EADDRINUSE;
		}
	} अन्यथा {
		/* _OBJ_INIT_ANY */
	}

	प्रत्यागम 0;
}

पूर्णाङ्क z_impl_k_object_access_check(ध्रुव शून्य *object)
{
	प्रत्यागम k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

शून्य k_object_init(ध्रुव शून्य *obj)
{
	संरचना k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	यदि (ko == रिक्त) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		प्रत्यागम;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

शून्य k_object_recycle(ध्रुव शून्य *obj)
{
	संरचना k_object *ko = k_object_find(obj);

	यदि (ko != रिक्त) {
		(शून्य)memset(ko->perms, 0, परिमाण(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

शून्य k_object_uninit(ध्रुव शून्य *obj)
{
	संरचना k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	यदि (ko == रिक्त) {
		प्रत्यागम;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
शून्य *k_usermode_alloc_from_copy(ध्रुव शून्य *src, size_t size)
{
	शून्य *dst = रिक्त;

	/* Does the caller in user mode have access to read this memory? */
	यदि (K_SYSCALL_MEMORY_READ(src, size)) {
		गच्छ out_err;
	}

	dst = z_thread_malloc(size);
	यदि (dst == रिक्त) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		गच्छ out_err;
	}

	(शून्य)memcpy(dst, src, size);
out_err:
	प्रत्यागम dst;
}

स्थिर पूर्णाङ्क user_copy(शून्य *dst, ध्रुव शून्य *src, size_t size, bool to_user)
{
	पूर्णाङ्क ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	यदि (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		गच्छ out_err;
	}

	(शून्य)memcpy(dst, src, size);
	ret = 0;
out_err:
	प्रत्यागम ret;
}

पूर्णाङ्क k_usermode_from_copy(शून्य *dst, ध्रुव शून्य *src, size_t size)
{
	प्रत्यागम user_copy(dst, src, size, false);
}

पूर्णाङ्क k_usermode_to_copy(शून्य *dst, ध्रुव शून्य *src, size_t size)
{
	प्रत्यागम user_copy(dst, src, size, true);
}

वर्ण *k_usermode_string_alloc_copy(ध्रुव वर्ण *src, size_t maxlen)
{
	size_t actual_len;
	पूर्णाङ्क err;
	वर्ण *ret = रिक्त;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	यदि (err != 0) {
		गच्छ out;
	}
	यदि (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		गच्छ out;
	}
	यदि (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		गच्छ out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	यदि (ret != रिक्त) {
		ret[actual_len - 1U] = '\0';
	}
out:
	प्रत्यागम ret;
}

पूर्णाङ्क k_usermode_string_copy(वर्ण *dst, ध्रुव वर्ण *src, size_t maxlen)
{
	size_t actual_len;
	पूर्णाङ्क ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	यदि (err != 0) {
		ret = EFAULT;
		गच्छ out;
	}
	यदि (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		गच्छ out;
	}
	यदि (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		गच्छ out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	प्रत्यागम ret;
}

/*
 * Application memory region initialization
 */

बाह्य वर्ण __app_shmem_regions_start[];
बाह्य वर्ण __app_shmem_regions_end[];

स्थिर शून्य app_shmem_bss_zero(शून्य)
{
	संरचना z_app_region *region, *end;


	end = (संरचना z_app_region *)&__app_shmem_regions_end[0];
	region = (संरचना z_app_region *)&__app_shmem_regions_start[0];

	प्रति ( ; region < end; region++) {
		(शून्य)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

स्थिर uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     शून्य *ssf)
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

स्थिर uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, शून्य *ssf)
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

#अन्तर्भाव <zephyr/syscall_dispatch.c>
