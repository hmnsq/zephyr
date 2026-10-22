/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#समावेश <zephyr/kernel.h>
#समावेश <string.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/sys/rb.h>
#समावेश <zephyr/sys/sys_io.h>
#समावेश <ksched.h>
#समावेश <zephyr/syscall.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/device.h>
#समावेश <stdbool.h>
#समावेश <zephyr/app_memory/app_memdomain.h>
#समावेश <zephyr/sys/libc-hooks.h>
#समावेश <zephyr/sys/mutex.h>
#समावेश <zephyr/sys/util.h>
#समावेश <inttypes.h>
#समावेश <zephyr/linker/linker-defs.h>
#समावेश <zephyr/cache.h>
#समावेश <kernel_internal.h>

#अगर_घोषित Z_LIBC_PARTITION_EXISTS
K_APPMEM_PARTITION_DEFINE(z_libc_partition);
#पूर्ण_अगर /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#अगर_घोषित CONFIG_MBEDTLS
K_APPMEM_PARTITION_DEFINE(k_mbedtls_partition);
#पूर्ण_अगर /* CONFIG_MBEDTLS */

#समावेश <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* The originally synchronization strategy made heavy use of recursive
 * irq_locking, which ports poorly to spinlocks which are
 * non-recursive.  Rather than try to redesign as part of
 * spinlockification, this uses multiple locks to preserve the
 * original semantics exactly.  The locks are named for the data they
 * protect where possible, or just for the code that uses them where
 * not.
 */
#अगर_घोषित CONFIG_DYNAMIC_OBJECTS
अटल काष्ठा k_spinlock lists_lock;       /* kobj dlist */

#अगर_घोषित CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#अगर defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#घोषणा ढेर_ELEMENT_तथ्य_आकार(आकार) \
	(माप(काष्ठा z_ढेर_तथ्य) + CONFIG_PRIVILEGED_ढेर_आकार + \
	Z_THREAD_ढेर_OBJ_ALIGN(आकार) + K_THREAD_ढेर_LEN(आकार))
#अन्यथा
#घोषणा ढेर_ELEMENT_तथ्य_आकार(आकार) (माप(काष्ठा z_ढेर_तथ्य) + \
	K_THREAD_ढेर_LEN(आकार))
#पूर्ण_अगर /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#अन्यथा
#घोषणा ढेर_ELEMENT_तथ्य_आकार(आकार) K_THREAD_ढेर_LEN(आकार)
#पूर्ण_अगर /* CONFIG_GEN_PRIV_STACKS */

#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS */
अटल काष्ठा k_spinlock obj_lock;         /* kobj struct data */

#घोषणा MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#अगर_घोषित CONFIG_DYNAMIC_OBJECTS
बाह्य uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

बाह्य पूर्णांक z_msgq_cleanup(काष्ठा k_msgq *q, bool locked);
बाह्य पूर्णांक z_ढेर_cleanup(काष्ठा k_ढेर *ढेर, bool locked);
बाह्य पूर्णांक z_timer_cleanup(काष्ठा k_timer *timer, bool locked);
#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS */

अटल व्योम unref_check(काष्ठा k_object *ko, uintptr_t सूचकांक, bool locked);

स्थिर अक्षर *otype_to_str(क्रमागत k_objects otype)
{
	स्थिर अक्षर *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#अगर_घोषित CONFIG_LOG
	चयन (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	हाल K_OBJ_ANY:
		ret = "generic";
		अवरोध;
	हाल K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		अवरोध;
#समावेश <zephyr/otype-to-str.h>
	शेष:
		ret = "?";
		अवरोध;
	}
#अन्यथा
	ARG_UNUSED(otype);
	ret = शून्य;
#पूर्ण_अगर /* CONFIG_LOG */
	वापस ret;
}

काष्ठा perm_ctx {
	पूर्णांक parent_id;
	पूर्णांक child_id;
	काष्ठा k_thread *parent;
};

#अगर_घोषित CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ढेर_find(k_thread_ढेर_t *ढेर)
{
	काष्ठा k_object *obj = k_object_find(ढेर);

	__ASSERT(obj != शून्य, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ढेर_ELEMENT,
		 "bad stack object");

	वापस obj->तथ्य.ढेर_तथ्य->priv;
}
#पूर्ण_अगर /* CONFIG_GEN_PRIV_STACKS */

#अगर_घोषित CONFIG_DYNAMIC_OBJECTS

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
#अगर_घोषित ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#घोषणा DYN_OBJ_तथ्य_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#अन्यथा
#घोषणा DYN_OBJ_तथ्य_ALIGN_K_THREAD	(माप(व्योम *))
#पूर्ण_अगर /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#अगर_घोषित CONFIG_DYNAMIC_THREAD_ढेर_आकार
#अगर defined(CONFIG_MPU_ढेर_GUARD) || defined(CONFIG_PMP_ढेर_GUARD)
#घोषणा DYN_OBJ_तथ्य_ALIGN_K_THREAD_ढेर \
	Z_THREAD_ढेर_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ढेर_आकार)
#अन्यथा
#घोषणा DYN_OBJ_तथ्य_ALIGN_K_THREAD_ढेर \
	Z_THREAD_ढेर_OBJ_ALIGN(CONFIG_PRIVILEGED_ढेर_आकार)
#पूर्ण_अगर /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#अन्यथा
#घोषणा DYN_OBJ_तथ्य_ALIGN_K_THREAD_ढेर \
	Z_THREAD_ढेर_OBJ_ALIGN(ARCH_ढेर_PTR_ALIGN)
#पूर्ण_अगर /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#घोषणा DYN_OBJ_तथ्य_ALIGN		\
	MAX(DYN_OBJ_तथ्य_ALIGN_K_THREAD, (माप(व्योम *)))

काष्ठा dyn_obj {
	काष्ठा k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	व्योम *तथ्य;
};

बाह्य काष्ठा k_object *z_object_gperf_find(स्थिर व्योम *obj);
बाह्य व्योम z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     व्योम *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
अटल sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

अटल माप_प्रकार obj_आकार_get(क्रमागत k_objects otype)
{
	माप_प्रकार ret;

	चयन (otype) {
#समावेश <zephyr/otype-to-size.h>
	शेष:
		ret = माप(स्थिर काष्ठा device);
		अवरोध;
	}

	वापस ret;
}

अटल माप_प्रकार obj_align_get(क्रमागत k_objects otype)
{
	माप_प्रकार ret;

	चयन (otype) {
	हाल K_OBJ_THREAD:
#अगर_घोषित ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#अन्यथा
		ret = __alignof(काष्ठा dyn_obj);
#पूर्ण_अगर /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		अवरोध;
	शेष:
		ret = __alignof(काष्ठा dyn_obj);
		अवरोध;
	}

	वापस ret;
}

अटल काष्ठा dyn_obj *dyn_object_find_locked(स्थिर व्योम *obj)
{
	काष्ठा dyn_obj *गाँठ;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, गाँठ, dobj_list) {
		अगर (गाँठ->kobj.name == obj) {
			वापस गाँठ;
		}
	}

	वापस शून्य;
}

अटल काष्ठा dyn_obj *dyn_object_find(स्थिर व्योम *obj)
{
	काष्ठा dyn_obj *गाँठ;
	k_spinlock_कुंजी_t कुंजी;

	कुंजी = k_spin_lock(&lists_lock);
	गाँठ = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, कुंजी);

	वापस गाँठ;
}

अटल व्योम clear_perms_cb(काष्ठा k_object *ko, व्योम *ctx_ptr)
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
अटल bool thread_idx_alloc(uintptr_t *tidx)
{
	पूर्णांक i;
	पूर्णांक idx;
	पूर्णांक base;
	bool ret = false;
	k_spinlock_कुंजी_t कुंजी;

	base = 0;
	कुंजी = k_spin_lock(&lists_lock);
	क्रम (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		अगर (idx != 0) {
			काष्ठा dyn_obj *obj, *अग्र;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (व्योम *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, अग्र, dobj_list) {
				clear_perms_cb(&obj->kobj, (व्योम *)*tidx);
			}

			ret = true;
			अवरोध;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, कुंजी);
	वापस ret;
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
अटल व्योम thread_idx_free(uintptr_t tidx)
{
	काष्ठा dyn_obj *obj, *अग्र;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (व्योम *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, अग्र, dobj_list) {
		clear_perms_cb(&obj->kobj, (व्योम *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	पूर्णांक base = tidx / अंक_BITS(_thread_idx_map[0]);
	पूर्णांक offset = tidx % अंक_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, कुंजी);
}

अटल काष्ठा k_object *dynamic_object_create(क्रमागत k_objects otype, माप_प्रकार align,
					      माप_प्रकार आकार)
{
	काष्ठा dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, माप(काष्ठा dyn_obj));
	अगर (dyn == शून्य) {
		वापस शून्य;
	}

	अगर (otype == K_OBJ_THREAD_ढेर_ELEMENT) {
		माप_प्रकार adjusted_आकार;

		अगर (आकार == 0) {
			k_free(dyn);
			वापस शून्य;
		}

		adjusted_आकार = ढेर_ELEMENT_तथ्य_आकार(आकार);
		अगर (adjusted_आकार < आकार) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			वापस शून्य;
		}

		dyn->तथ्य = z_thread_aligned_alloc(DYN_OBJ_तथ्य_ALIGN_K_THREAD_ढेर,
						     adjusted_आकार);
		अगर (dyn->तथ्य == शून्य) {
			k_free(dyn);
			वापस शून्य;
		}

#अगर_घोषित CONFIG_DYNAMIC_OBJECTS_FORCE_ढेर_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		अगर (sys_cache_is_ptr_uncached(dyn->तथ्य)) {
			dyn->तथ्य = sys_cache_cached_ptr_get(dyn->तथ्य);
		}
#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#अगर_घोषित CONFIG_GEN_PRIV_STACKS
		काष्ठा z_ढेर_तथ्य *ढेर_तथ्य = (काष्ठा z_ढेर_तथ्य *)
			((uint8_t *)dyn->तथ्य + adjusted_आकार - माप(*ढेर_तथ्य));
#अगर defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ढेर_तथ्य->priv = (व्योम *)ROUND_UP(((uint8_t *)dyn->तथ्य + आकार),
			  Z_THREAD_ढेर_OBJ_ALIGN(आकार));
#अन्यथा
		ढेर_तथ्य->priv = (uint8_t *)dyn->तथ्य;
#पूर्ण_अगर /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ढेर_तथ्य->आकार = adjusted_आकार;
		dyn->kobj.तथ्य.ढेर_तथ्य = ढेर_तथ्य;
		dyn->kobj.name = dyn->तथ्य;
#अन्यथा
		dyn->kobj.name = dyn->तथ्य;
		dyn->kobj.तथ्य.ढेर_आकार = adjusted_आकार;
#पूर्ण_अगर /* CONFIG_GEN_PRIV_STACKS */
	} अन्यथा {
		माप_प्रकार योग_आकार = obj_आकार_get(otype) + आकार;

		अगर (योग_आकार < आकार) {
			k_free(dyn);
			वापस शून्य;
		}

		dyn->तथ्य = z_thread_aligned_alloc(align, योग_आकार);
		अगर (dyn->तथ्य == शून्य) {
			k_free(dyn);
			वापस शून्य;
		}
		dyn->kobj.name = dyn->तथ्य;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(व्योम)स_रखो(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, कुंजी);

	वापस &dyn->kobj;
}

काष्ठा k_object *k_object_create_dynamic_aligned(माप_प्रकार align, माप_प्रकार आकार)
{
	काष्ठा k_object *obj = dynamic_object_create(K_OBJ_ANY, align, आकार);

	अगर (obj == शून्य) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	वापस obj;
}

अटल व्योम *z_object_alloc(क्रमागत k_objects otype, माप_प्रकार आकार)
{
	काष्ठा k_object *zo;
	uintptr_t tidx = 0;

	अगर ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		वापस शून्य;
	}

	चयन (otype) {
	हाल K_OBJ_THREAD:
		अगर (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			वापस शून्य;
		}
		अवरोध;
	/* The following are currently not allowed at all */
	हाल K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	हाल K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		वापस शून्य;
	शेष:
		/* Remainder within bounds are permitted */
		अवरोध;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), आकार);
	अगर (zo == शून्य) {
		अगर (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		वापस शून्य;
	}

	अगर (otype == K_OBJ_THREAD) {
		zo->तथ्य.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_set(zo, _current);

	/* Activates reference counting logic for automatic disposal when
	 * all permissions have been revoked
	 */
	zo->flags |= K_OBJ_FLAG_ALLOC;

	वापस zo->name;
}

व्योम *z_impl_k_object_alloc(क्रमागत k_objects otype)
{
	वापस z_object_alloc(otype, 0);
}

व्योम *z_impl_k_object_alloc_आकार(क्रमागत k_objects otype, माप_प्रकार आकार)
{
	वापस z_object_alloc(otype, आकार);
}

व्योम k_object_free(व्योम *obj)
{
	काष्ठा dyn_obj *dyn = शून्य;
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

	अगर (dyn != शून्य) {
		sys_dlist_remove(&dyn->dobj_list);

		अगर (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.तथ्य.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, कुंजी);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	अगर (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	अगर (dyn != शून्य) {
#अगर_घोषित CONFIG_OBJ_CORE
		अगर (dyn->kobj.type != K_OBJ_THREAD_ढेर_ELEMENT) {
			k_obj_core_evict_range(obj, obj_आकार_get(dyn->kobj.type));
		}
#पूर्ण_अगर /* CONFIG_OBJ_CORE */
#अगर_घोषित CONFIG_DYNAMIC_OBJECTS_FORCE_ढेर_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		अगर (dyn->kobj.type == K_OBJ_THREAD_ढेर_ELEMENT) {
			uint8_t *ढेर;

			अगर (sys_cache_is_ptr_cached(dyn->तथ्य)) {
				ढेर = sys_cache_uncached_ptr_get(dyn->तथ्य);
			} अन्यथा {
				ढेर = dyn->तथ्य;
			}

			k_free(ढेर);
		} अन्यथा
#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->तथ्य);
		}
		k_free(dyn);
	}
}

काष्ठा k_object *k_object_find(स्थिर व्योम *obj)
{
	काष्ठा k_object *ret;

	ret = z_object_gperf_find(obj);

	अगर (ret == शून्य) {
		काष्ठा dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		अगर (dyn != शून्य) {
			ret = &dyn->kobj;
		}
	}

	वापस ret;
}

व्योम k_object_wordlist_foreach(_wordlist_cb_func_t func, व्योम *context)
{
	काष्ठा dyn_obj *obj, *अग्र;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, अग्र, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, कुंजी);
}
#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#अगर_घोषित CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_तथ्य.text.dummies)
__weak काष्ठा k_object *z_object_gperf_find(स्थिर व्योम *obj)
{
	ARG_UNUSED(obj);
	वापस शून्य;
}
Z_GENERIC_SECTION(.kobject_तथ्य.text.dummies)
__weak व्योम z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, व्योम *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#अन्यथा
Z_GENERIC_SECTION(.kobject_तथ्य.text.dummies)
__weak काष्ठा k_object *k_object_find(स्थिर व्योम *obj)
{
	वापस शून्य;
}
Z_GENERIC_SECTION(.kobject_तथ्य.text.dummies)
__weak व्योम k_object_wordlist_foreach(_wordlist_cb_func_t func, व्योम *context)
{
}
#पूर्ण_अगर

अटल अचिन्हित पूर्णांक thread_सूचकांक_get(काष्ठा k_thread *thread)
{
	काष्ठा k_object *ko;

	ko = k_object_find(thread);

	अगर (ko == शून्य) {
		वापस -1;
	}

	वापस ko->तथ्य.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
अटल व्योम unref_check(काष्ठा k_object *ko, uintptr_t सूचकांक, bool locked)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, सूचकांक);

#अगर_घोषित CONFIG_DYNAMIC_OBJECTS
	अगर ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		जाओ out;
	}

	व्योम *vko = ko;

	काष्ठा dyn_obj *dyn = CONTAINER_OF(vko, काष्ठा dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, काष्ठा dyn_obj), "unaligned z_object");

	क्रम (पूर्णांक i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		अगर (ko->perms[i] != 0U) {
			जाओ out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	चयन (ko->type) {
	हाल K_OBJ_MSGQ:
		(व्योम)z_msgq_cleanup((काष्ठा k_msgq *)ko->name, locked);
		अवरोध;
	हाल K_OBJ_ढेर:
		(व्योम)z_ढेर_cleanup((काष्ठा k_ढेर *)ko->name, locked);
		अवरोध;
	हाल K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		अगर ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(व्योम)z_timer_cleanup((काष्ठा k_timer *)ko->name, locked);
		}
		अवरोध;
	शेष:
		/* Nothing to do */
		अवरोध;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->तथ्य);
	k_free(dyn);
out:
#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, कुंजी);
}

अटल व्योम wordlist_cb(काष्ठा k_object *ko, व्योम *ctx_ptr)
{
	काष्ठा perm_ctx *ctx = (काष्ठा perm_ctx *)ctx_ptr;

	अगर (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((काष्ठा k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

व्योम k_thread_perms_inherit(काष्ठा k_thread *parent, काष्ठा k_thread *child)
{
	काष्ठा perm_ctx ctx = {
		thread_सूचकांक_get(parent),
		thread_सूचकांक_get(child),
		parent
	};

	अगर ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

व्योम k_thread_perms_set(काष्ठा k_object *ko, काष्ठा k_thread *thread)
{
	पूर्णांक सूचकांक = thread_सूचकांक_get(thread);

	अगर (सूचकांक != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, सूचकांक);
	}
}

व्योम k_thread_perms_clear(काष्ठा k_object *ko, काष्ठा k_thread *thread)
{
	पूर्णांक सूचकांक = thread_सूचकांक_get(thread);

	अगर (सूचकांक != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, सूचकांक);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#अगर_घोषित CONFIG_DYNAMIC_OBJECTS
		k_spinlock_कुंजी_t कुंजी = k_spin_lock(&lists_lock);

		unref_check(ko, सूचकांक, false);
		k_spin_unlock(&lists_lock, कुंजी);
#अन्यथा
		unref_check(ko, सूचकांक, false);
#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS */
	}
}

अटल व्योम clear_perms_cb_locked(काष्ठा k_object *ko, व्योम *ctx_ptr)
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
व्योम k_thread_perms_all_clear(काष्ठा k_thread *thread)
{
	uintptr_t सूचकांक = thread_सूचकांक_get(thread);

	अगर ((पूर्णांक)सूचकांक != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (व्योम *)सूचकांक);
	}
}

अटल पूर्णांक thread_perms_test(काष्ठा k_object *ko)
{
	पूर्णांक सूचकांक;

	अगर ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		वापस 1;
	}

	सूचकांक = thread_सूचकांक_get(_current);
	अगर (सूचकांक != -1) {
		वापस sys_bitfield_test_bit((mem_addr_t)&ko->perms, सूचकांक);
	}
	वापस 0;
}

अटल व्योम dump_permission_error(काष्ठा k_object *ko)
{
	पूर्णांक सूचकांक = thread_सूचकांक_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, सूचकांक,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, माप(ko->perms), "permission bitmap");
}

व्योम k_object_dump_error(पूर्णांक retval, स्थिर व्योम *obj, काष्ठा k_object *ko,
			क्रमागत k_objects otype)
{
	चयन (retval) {
	हाल -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		अगर (ko == शून्य) {
			LOG_ERR("address is not a known kernel object");
		} अन्यथा {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		अवरोध;
	हाल -EPERM:
		dump_permission_error(ko);
		अवरोध;
	हाल -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		अवरोध;
	हाल -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		अवरोध;
	शेष:
		/* Not handled error */
		अवरोध;
	}
}

व्योम z_impl_k_object_access_grant(स्थिर व्योम *object, काष्ठा k_thread *thread)
{
	काष्ठा k_object *ko = k_object_find(object);

	अगर (ko != शून्य) {
		k_thread_perms_set(ko, thread);
	}
}

व्योम k_object_access_revoke(स्थिर व्योम *object, काष्ठा k_thread *thread)
{
	काष्ठा k_object *ko = k_object_find(object);

	अगर (ko != शून्य) {
		k_thread_perms_clear(ko, thread);
	}
}

व्योम k_object_access_revoke_others(स्थिर व्योम *object)
{
	काष्ठा k_object *ko = k_object_find(object);

	अगर (ko != शून्य) {
		(व्योम)स_रखो(ko->perms, 0, माप(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

व्योम z_impl_k_object_release(स्थिर व्योम *object)
{
	k_object_access_revoke(object, _current);
}

व्योम k_object_access_all_grant(स्थिर व्योम *object)
{
	काष्ठा k_object *ko = k_object_find(object);

	अगर (ko != शून्य) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

पूर्णांक k_object_validate(काष्ठा k_object *ko, क्रमागत k_objects otype,
		       क्रमागत _obj_init_check init)
{
	अगर (unlikely(ko == शून्य)) {
		वापस -EBADF;
	}

	अगर (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		अगर ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			वापस -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	अगर (unlikely(thread_perms_test(ko) == 0)) {
		वापस -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	अगर (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		अगर (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			वापस -EINVAL;
		}
	} अन्यथा अगर (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		अगर (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			वापस -EADDRINUSE;
		}
	} अन्यथा {
		/* _OBJ_INIT_ANY */
	}

	वापस 0;
}

पूर्णांक z_impl_k_object_access_check(स्थिर व्योम *object)
{
	वापस k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

व्योम k_object_init(स्थिर व्योम *obj)
{
	काष्ठा k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	अगर (ko == शून्य) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		वापस;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

व्योम k_object_recycle(स्थिर व्योम *obj)
{
	काष्ठा k_object *ko = k_object_find(obj);

	अगर (ko != शून्य) {
		(व्योम)स_रखो(ko->perms, 0, माप(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

व्योम k_object_uninit(स्थिर व्योम *obj)
{
	काष्ठा k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	अगर (ko == शून्य) {
		वापस;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
व्योम *k_usermode_alloc_from_copy(स्थिर व्योम *src, माप_प्रकार आकार)
{
	व्योम *dst = शून्य;

	/* Does the caller in user mode have access to read this memory? */
	अगर (K_SYSCALL_MEMORY_READ(src, आकार)) {
		जाओ out_err;
	}

	dst = z_thread_malloc(आकार);
	अगर (dst == शून्य) {
		LOG_ERR("out of thread resource pool memory (%zu)", आकार);
		जाओ out_err;
	}

	(व्योम)स_नकल(dst, src, आकार);
out_err:
	वापस dst;
}

अटल पूर्णांक user_copy(व्योम *dst, स्थिर व्योम *src, माप_प्रकार आकार, bool to_user)
{
	पूर्णांक ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	अगर (to_user ? K_SYSCALL_MEMORY_WRITE(dst, आकार) :
			K_SYSCALL_MEMORY_READ(src, आकार)) {
		जाओ out_err;
	}

	(व्योम)स_नकल(dst, src, आकार);
	ret = 0;
out_err:
	वापस ret;
}

पूर्णांक k_usermode_from_copy(व्योम *dst, स्थिर व्योम *src, माप_प्रकार आकार)
{
	वापस user_copy(dst, src, आकार, false);
}

पूर्णांक k_usermode_to_copy(व्योम *dst, स्थिर व्योम *src, माप_प्रकार आकार)
{
	वापस user_copy(dst, src, आकार, true);
}

अक्षर *k_usermode_string_alloc_copy(स्थिर अक्षर *src, माप_प्रकार maxlen)
{
	माप_प्रकार actual_len;
	पूर्णांक err;
	अक्षर *ret = शून्य;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	अगर (err != 0) {
		जाओ out;
	}
	अगर (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		जाओ out;
	}
	अगर (आकार_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		जाओ out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	अगर (ret != शून्य) {
		ret[actual_len - 1U] = '\0';
	}
out:
	वापस ret;
}

पूर्णांक k_usermode_string_copy(अक्षर *dst, स्थिर अक्षर *src, माप_प्रकार maxlen)
{
	माप_प्रकार actual_len;
	पूर्णांक ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	अगर (err != 0) {
		ret = EFAULT;
		जाओ out;
	}
	अगर (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		जाओ out;
	}
	अगर (आकार_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		जाओ out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	वापस ret;
}

/*
 * Application memory region initialization
 */

बाह्य अक्षर __app_shmem_regions_start[];
बाह्य अक्षर __app_shmem_regions_end[];

अटल व्योम app_shmem_bss_zero(व्योम)
{
	काष्ठा z_app_region *region, *end;


	end = (काष्ठा z_app_region *)&__app_shmem_regions_end[0];
	region = (काष्ठा z_app_region *)&__app_shmem_regions_start[0];

	क्रम ( ; region < end; region++) {
		(व्योम)स_रखो(region->bss_start, 0, region->bss_आकार);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

अटल uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     व्योम *ssf)
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

अटल uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, व्योम *ssf)
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

#समावेश <zephyr/syscall_dispatch.c>
