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

#यदि_परिभाषितम् Z_LIBC_PARTITION_EXISTS
K_APPMEM_PARTITION_DEFINE(z_libc_partition);
#यदि_अन्तः /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#यदि_परिभाषितम् CONFIG_MBEDTLS
K_APPMEM_PARTITION_DEFINE(k_mbedtls_partition);
#यदि_अन्तः /* CONFIG_MBEDTLS */

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
#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS
स्थिर संरचना k_spinlock lists_तालकम्;       /* kobj dlist */

#यदि_परिभाषितम् CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#यदि defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#परिभाषय राशिः_ELEMENT_दत्त_आकार(आकार) \
	(परिमाण(संरचना z_राशिः_दत्त) + CONFIG_PRIVILEGED_राशिः_आकार + \
	Z_THREAD_राशिः_OBJ_ALIGN(आकार) + K_THREAD_राशिः_LEN(आकार))
#अन्यथा
#परिभाषय राशिः_ELEMENT_दत्त_आकार(आकार) (परिमाण(संरचना z_राशिः_दत्त) + \
	K_THREAD_राशिः_LEN(आकार))
#यदि_अन्तः /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#अन्यथा
#परिभाषय राशिः_ELEMENT_दत्त_आकार(आकार) K_THREAD_राशिः_LEN(आकार)
#यदि_अन्तः /* CONFIG_GEN_PRIV_STACKS */

#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS */
स्थिर संरचना k_spinlock obj_तालकम्;         /* kobj struct data */

#परिभाषय MAX_THREAD_द्वयङ्काः (CONFIG_MAX_THREAD_अष्टकानि * द्वयङ्काः_प्रत्येकम्_BYTE)

#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS
बाह्य uint8_t _thread_idx_map[CONFIG_MAX_THREAD_अष्टकानि];

बाह्य पूर्णाङ्क z_msgq_cleanup(संरचना k_msgq *q, bool locked);
बाह्य पूर्णाङ्क z_राशिः_cleanup(संरचना k_राशिः *राशिः, bool locked);
बाह्य पूर्णाङ्क z_timer_cleanup(संरचना k_timer *timer, bool locked);
#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS */

स्थिर शून्य unref_check(संरचना k_object *ko, uintptr_t सूचकांक, bool locked);

ध्रुव वर्ण *otype_गन्तव्यम्_str(गणना k_objects otype)
{
	ध्रुव वर्ण *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#यदि_परिभाषितम् CONFIG_LOG
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
	ARG_अप्रयुक्तम्(otype);
	ret = रिक्त;
#यदि_अन्तः /* CONFIG_LOG */
	प्रत्यागम ret;
}

संरचना perm_ctx {
	पूर्णाङ्क parent_id;
	पूर्णाङ्क child_id;
	संरचना k_thread *parent;
};

#यदि_परिभाषितम् CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_राशिः_find(k_thread_राशिः_t *राशिः)
{
	संरचना k_object *obj = k_object_find(राशिः);

	__ASSERT(obj != रिक्त, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_राशिः_ELEMENT,
		 "bad stack object");

	प्रत्यागम obj->दत्त.राशिः_दत्त->priv;
}
#यदि_अन्तः /* CONFIG_GEN_PRIV_STACKS */

#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS

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
#यदि_परिभाषितम् ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#परिभाषय DYN_OBJ_दत्त_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#अन्यथा
#परिभाषय DYN_OBJ_दत्त_ALIGN_K_THREAD	(परिमाण(शून्य *))
#यदि_अन्तः /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#यदि_परिभाषितम् CONFIG_DYNAMIC_THREAD_राशिः_आकार
#यदि defined(CONFIG_MPU_राशिः_GUARD) || defined(CONFIG_PMP_राशिः_GUARD)
#परिभाषय DYN_OBJ_दत्त_ALIGN_K_THREAD_राशिः \
	Z_THREAD_राशिः_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_राशिः_आकार)
#अन्यथा
#परिभाषय DYN_OBJ_दत्त_ALIGN_K_THREAD_राशिः \
	Z_THREAD_राशिः_OBJ_ALIGN(CONFIG_PRIVILEGED_राशिः_आकार)
#यदि_अन्तः /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#अन्यथा
#परिभाषय DYN_OBJ_दत्त_ALIGN_K_THREAD_राशिः \
	Z_THREAD_राशिः_OBJ_ALIGN(ARCH_राशिः_PTR_ALIGN)
#यदि_अन्तः /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#परिभाषय DYN_OBJ_दत्त_ALIGN		\
	MAX(DYN_OBJ_दत्त_ALIGN_K_THREAD, (परिमाण(शून्य *)))

संरचना dyn_obj {
	संरचना k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	शून्य *दत्त;
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

स्थिर size_t obj_आकार_get(गणना k_objects otype)
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
#यदि_परिभाषितम् ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#अन्यथा
		ret = __alignof(संरचना dyn_obj);
#यदि_अन्तः /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		भङ्ग;
	स्वभाव:
		ret = __alignof(संरचना dyn_obj);
		भङ्ग;
	}

	प्रत्यागम ret;
}

स्थिर संरचना dyn_obj *dyn_object_find_locked(ध्रुव शून्य *obj)
{
	संरचना dyn_obj *ग्रन्थिः;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, ग्रन्थिः, dobj_list) {
		यदि (ग्रन्थिः->kobj.name == obj) {
			प्रत्यागम ग्रन्थिः;
		}
	}

	प्रत्यागम रिक्त;
}

स्थिर संरचना dyn_obj *dyn_object_find(ध्रुव शून्य *obj)
{
	संरचना dyn_obj *ग्रन्थिः;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&lists_तालकम्);
	ग्रन्थिः = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_तालकम्, कुञ्चिका);

	प्रत्यागम ग्रन्थिः;
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
	पूर्णाङ्क आधारः;
	bool ret = false;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	आधारः = 0;
	कुञ्चिका = k_spin_तालकम्(&lists_तालकम्);
	प्रति (i = 0; i < CONFIG_MAX_THREAD_अष्टकानि; i++) {
		idx = find_lsb_समुच्चयः(_thread_idx_map[i]);

		यदि (idx != 0) {
			संरचना dyn_obj *obj, *अग्र;

			*tidx = आधारः + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (शून्य *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, अग्र, dobj_list) {
				clear_perms_cb(&obj->kobj, (शून्य *)*tidx);
			}

			ret = true;
			भङ्ग;
		}

		आधारः += 8;
	}

	k_spin_unlock(&lists_तालकम्, कुञ्चिका);
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
	संरचना dyn_obj *obj, *अग्र;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (शून्य *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&lists_तालकम्);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, अग्र, dobj_list) {
		clear_perms_cb(&obj->kobj, (शून्य *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	पूर्णाङ्क आधारः = tidx / अङ्कः_द्वयङ्काः_2(_thread_idx_map[0]);
	पूर्णाङ्क विस्थापनम् = tidx % अङ्कः_द्वयङ्काः_2(_thread_idx_map[0]);

	_thread_idx_map[आधारः] |= BIT(विस्थापनम्);

	k_spin_unlock(&lists_तालकम्, कुञ्चिका);
}

स्थिर संरचना k_object *dynamic_object_create(गणना k_objects otype, size_t align,
					      size_t आकार)
{
	संरचना dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, परिमाण(संरचना dyn_obj));
	यदि (dyn == रिक्त) {
		प्रत्यागम रिक्त;
	}

	यदि (otype == K_OBJ_THREAD_राशिः_ELEMENT) {
		size_t adjusted_आकार;

		यदि (आकार == 0) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

		adjusted_आकार = राशिः_ELEMENT_दत्त_आकार(आकार);
		यदि (adjusted_आकार < आकार) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

		dyn->दत्त = z_thread_aligned_alloc(DYN_OBJ_दत्त_ALIGN_K_THREAD_राशिः,
						     adjusted_आकार);
		यदि (dyn->दत्त == रिक्त) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS_FORCE_राशिः_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		यदि (sys_cache_is_ptr_uncached(dyn->दत्त)) {
			dyn->दत्त = sys_cache_cached_ptr_get(dyn->दत्त);
		}
#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#यदि_परिभाषितम् CONFIG_GEN_PRIV_STACKS
		संरचना z_राशिः_दत्त *राशिः_दत्त = (संरचना z_राशिः_दत्त *)
			((uint8_t *)dyn->दत्त + adjusted_आकार - परिमाण(*राशिः_दत्त));
#यदि defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		राशिः_दत्त->priv = (शून्य *)आवर्तनम्_ऊर्ध्वम्(((uint8_t *)dyn->दत्त + आकार),
			  Z_THREAD_राशिः_OBJ_ALIGN(आकार));
#अन्यथा
		राशिः_दत्त->priv = (uint8_t *)dyn->दत्त;
#यदि_अन्तः /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		राशिः_दत्त->आकार = adjusted_आकार;
		dyn->kobj.दत्त.राशिः_दत्त = राशिः_दत्त;
		dyn->kobj.name = dyn->दत्त;
#अन्यथा
		dyn->kobj.name = dyn->दत्त;
		dyn->kobj.दत्त.राशिः_आकार = adjusted_आकार;
#यदि_अन्तः /* CONFIG_GEN_PRIV_STACKS */
	} अन्यथा {
		size_t योग_आकार = obj_आकार_get(otype) + आकार;

		यदि (योग_आकार < आकार) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}

		dyn->दत्त = z_thread_aligned_alloc(align, योग_आकार);
		यदि (dyn->दत्त == रिक्त) {
			k_free(dyn);
			प्रत्यागम रिक्त;
		}
		dyn->kobj.name = dyn->दत्त;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(शून्य)स्मृतिं_पूरय(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_अष्टकानि);

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&lists_तालकम्);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_तालकम्, कुञ्चिका);

	प्रत्यागम &dyn->kobj;
}

संरचना k_object *k_object_create_dynamic_aligned(size_t align, size_t आकार)
{
	संरचना k_object *obj = dynamic_object_create(K_OBJ_ANY, align, आकार);

	यदि (obj == रिक्त) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	प्रत्यागम obj;
}

स्थिर शून्य *z_object_alloc(गणना k_objects otype, size_t आकार)
{
	संरचना k_object *zo;
	uintptr_t tidx = 0;

	यदि ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_चरमः)) {
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
			otype_गन्तव्यम्_str(otype));
		प्रत्यागम रिक्त;
	स्वभाव:
		/* Remainder within bounds are permitted */
		भङ्ग;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), आकार);
	यदि (zo == रिक्त) {
		यदि (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		प्रत्यागम रिक्त;
	}

	यदि (otype == K_OBJ_THREAD) {
		zo->दत्त.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_समुच्चयः(zo, _current);

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

शून्य *z_impl_k_object_alloc_आकार(गणना k_objects otype, size_t आकार)
{
	प्रत्यागम z_object_alloc(otype, आकार);
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
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&lists_तालकम्);

	dyn = dyn_object_find_locked(obj);

	यदि (dyn != रिक्त) {
		sys_dlist_remove(&dyn->dobj_list);

		यदि (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.दत्त.thread_id;
		}
	}
	k_spin_unlock(&lists_तालकम्, कुञ्चिका);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	यदि (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	यदि (dyn != रिक्त) {
#यदि_परिभाषितम् CONFIG_OBJ_CORE
		यदि (dyn->kobj.type != K_OBJ_THREAD_राशिः_ELEMENT) {
			k_obj_core_evict_range(obj, obj_आकार_get(dyn->kobj.type));
		}
#यदि_अन्तः /* CONFIG_OBJ_CORE */
#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS_FORCE_राशिः_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		यदि (dyn->kobj.type == K_OBJ_THREAD_राशिः_ELEMENT) {
			uint8_t *राशिः;

			यदि (sys_cache_is_ptr_cached(dyn->दत्त)) {
				राशिः = sys_cache_uncached_ptr_get(dyn->दत्त);
			} अन्यथा {
				राशिः = dyn->दत्त;
			}

			k_free(राशिः);
		} अन्यथा
#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->दत्त);
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
	संरचना dyn_obj *obj, *अग्र;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&lists_तालकम्);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, अग्र, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_तालकम्, कुञ्चिका);
}
#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_दत्त.text.dummies)
__weak संरचना k_object *z_object_gperf_find(ध्रुव शून्य *obj)
{
	ARG_अप्रयुक्तम्(obj);
	प्रत्यागम रिक्त;
}
Z_GENERIC_SECTION(.kobject_दत्त.text.dummies)
__weak शून्य z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, शून्य *context)
{
	ARG_अप्रयुक्तम्(func);
	ARG_अप्रयुक्तम्(context);
}
#अन्यथा
Z_GENERIC_SECTION(.kobject_दत्त.text.dummies)
__weak संरचना k_object *k_object_find(ध्रुव शून्य *obj)
{
	प्रत्यागम रिक्त;
}
Z_GENERIC_SECTION(.kobject_दत्त.text.dummies)
__weak शून्य k_object_wordlist_foreach(_wordlist_cb_func_t func, शून्य *context)
{
}
#यदि_अन्तः

स्थिर अचिह्नित पूर्णाङ्क thread_सूचकांक_get(संरचना k_thread *thread)
{
	संरचना k_object *ko;

	ko = k_object_find(thread);

	यदि (ko == रिक्त) {
		प्रत्यागम -1;
	}

	प्रत्यागम ko->दत्त.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
स्थिर शून्य unref_check(संरचना k_object *ko, uintptr_t सूचकांक, bool locked)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&obj_तालकम्);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, सूचकांक);

#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS
	यदि ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		गच्छ बहिर्गमनम्;
	}

	शून्य *vko = ko;

	संरचना dyn_obj *dyn = CONTAINER_सम्बन्धि(vko, संरचना dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, संरचना dyn_obj), "unaligned z_object");

	प्रति (पूर्णाङ्क i = 0; i < CONFIG_MAX_THREAD_अष्टकानि; i++) {
		यदि (ko->perms[i] != 0U) {
			गच्छ बहिर्गमनम्;
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
	पक्ष K_OBJ_राशिः:
		(शून्य)z_राशिः_cleanup((संरचना k_राशिः *)ko->name, locked);
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
	k_free(dyn->दत्त);
	k_free(dyn);
बहिर्गमनम्:
#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_तालकम्, कुञ्चिका);
}

स्थिर शून्य wordlist_cb(संरचना k_object *ko, शून्य *ctx_ptr)
{
	संरचना perm_ctx *ctx = (संरचना perm_ctx *)ctx_ptr;

	यदि (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((संरचना k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_समुच्चयः_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

शून्य k_thread_perms_inherit(संरचना k_thread *parent, संरचना k_thread *child)
{
	संरचना perm_ctx ctx = {
		thread_सूचकांक_get(parent),
		thread_सूचकांक_get(child),
		parent
	};

	यदि ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

शून्य k_thread_perms_समुच्चयः(संरचना k_object *ko, संरचना k_thread *thread)
{
	पूर्णाङ्क सूचकांक = thread_सूचकांक_get(thread);

	यदि (सूचकांक != -1) {
		sys_bitfield_समुच्चयः_bit((mem_addr_t)&ko->perms, सूचकांक);
	}
}

शून्य k_thread_perms_clear(संरचना k_object *ko, संरचना k_thread *thread)
{
	पूर्णाङ्क सूचकांक = thread_सूचकांक_get(thread);

	यदि (सूचकांक != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, सूचकांक);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS
		k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&lists_तालकम्);

		unref_check(ko, सूचकांक, false);
		k_spin_unlock(&lists_तालकम्, कुञ्चिका);
#अन्यथा
		unref_check(ko, सूचकांक, false);
#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS */
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
	uintptr_t सूचकांक = thread_सूचकांक_get(thread);

	यदि ((पूर्णाङ्क)सूचकांक != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (शून्य *)सूचकांक);
	}
}

स्थिर पूर्णाङ्क thread_perms_test(संरचना k_object *ko)
{
	पूर्णाङ्क सूचकांक;

	यदि ((ko->flags & K_OBJ_FLAG_लौकिकम्) != 0U) {
		प्रत्यागम 1;
	}

	सूचकांक = thread_सूचकांक_get(_current);
	यदि (सूचकांक != -1) {
		प्रत्यागम sys_bitfield_test_bit((mem_addr_t)&ko->perms, सूचकांक);
	}
	प्रत्यागम 0;
}

स्थिर शून्य dump_permission_error(संरचना k_object *ko)
{
	पूर्णाङ्क सूचकांक = thread_सूचकांक_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, सूचकांक,
		otype_गन्तव्यम्_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, परिमाण(ko->perms), "permission bitmap");
}

शून्य k_object_dump_error(पूर्णाङ्क retval, ध्रुव शून्य *obj, संरचना k_object *ko,
			गणना k_objects otype)
{
	विकल्प (retval) {
	पक्ष -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_गन्तव्यम्_str(otype));
		यदि (ko == रिक्त) {
			LOG_ERR("address is not a known kernel object");
		} अन्यथा {
			LOG_ERR("address is actually a %s",
				otype_गन्तव्यम्_str(ko->type));
		}
		भङ्ग;
	पक्ष -EPERM:
		dump_permission_error(ko);
		भङ्ग;
	पक्ष -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		भङ्ग;
	पक्ष -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_गन्तव्यम्_str(otype));
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
		k_thread_perms_समुच्चयः(ko, thread);
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
		(शून्य)स्मृतिं_पूरय(ko->perms, 0, परिमाण(ko->perms));
		k_thread_perms_समुच्चयः(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_लौकिकम्;
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
		ko->flags |= K_OBJ_FLAG_लौकिकम्;
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
		    (ko->type > K_OBJ_DRIVER_चरमः)) {
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
		(शून्य)स्मृतिं_पूरय(ko->perms, 0, परिमाण(ko->perms));
		k_thread_perms_समुच्चयः(ko, _current);
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
शून्य *k_usermode_alloc_from_copy(ध्रुव शून्य *src, size_t आकार)
{
	शून्य *dst = रिक्त;

	/* Does the caller in user mode have access to read this memory? */
	यदि (K_SYSCALL_स्मृतिः_READ(src, आकार)) {
		गच्छ बहिर्गमनम्_err;
	}

	dst = z_thread_malloc(आकार);
	यदि (dst == रिक्त) {
		LOG_ERR("out of thread resource pool memory (%zu)", आकार);
		गच्छ बहिर्गमनम्_err;
	}

	(शून्य)स्मृतिं_प्रतिलिख(dst, src, आकार);
बहिर्गमनम्_err:
	प्रत्यागम dst;
}

स्थिर पूर्णाङ्क user_copy(शून्य *dst, ध्रुव शून्य *src, size_t आकार, bool गन्तव्यम्_user)
{
	पूर्णाङ्क ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	यदि (गन्तव्यम्_user ? K_SYSCALL_स्मृतिः_WRITE(dst, आकार) :
			K_SYSCALL_स्मृतिः_READ(src, आकार)) {
		गच्छ बहिर्गमनम्_err;
	}

	(शून्य)स्मृतिं_प्रतिलिख(dst, src, आकार);
	ret = 0;
बहिर्गमनम्_err:
	प्रत्यागम ret;
}

पूर्णाङ्क k_usermode_from_copy(शून्य *dst, ध्रुव शून्य *src, size_t आकार)
{
	प्रत्यागम user_copy(dst, src, आकार, false);
}

पूर्णाङ्क k_usermode_गन्तव्यम्_copy(शून्य *dst, ध्रुव शून्य *src, size_t आकार)
{
	प्रत्यागम user_copy(dst, src, आकार, true);
}

वर्ण *k_usermode_string_alloc_copy(ध्रुव वर्ण *src, size_t maxlen)
{
	size_t actual_len;
	पूर्णाङ्क err;
	वर्ण *ret = रिक्त;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	यदि (err != 0) {
		गच्छ बहिर्गमनम्;
	}
	यदि (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		गच्छ बहिर्गमनम्;
	}
	यदि (आकार_सङ्कलय_अतिप्रवाहः(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		गच्छ बहिर्गमनम्;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	यदि (ret != रिक्त) {
		ret[actual_len - 1U] = '\0';
	}
बहिर्गमनम्:
	प्रत्यागम ret;
}

पूर्णाङ्क k_usermode_string_copy(वर्ण *dst, ध्रुव वर्ण *src, size_t maxlen)
{
	size_t actual_len;
	पूर्णाङ्क ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	यदि (err != 0) {
		ret = EFAULT;
		गच्छ बहिर्गमनम्;
	}
	यदि (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		गच्छ बहिर्गमनम्;
	}
	यदि (आकार_सङ्कलय_अतिप्रवाहः(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		गच्छ बहिर्गमनम्;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
बहिर्गमनम्:
	प्रत्यागम ret;
}

/*
 * Application memory region initialization
 */

बाह्य वर्ण __app_shmem_regions_start[];
बाह्य वर्ण __app_shmem_regions_end[];

स्थिर शून्य app_shmem_bss_zero(शून्य)
{
	संरचना z_app_region *region, *अन्तः;


	अन्तः = (संरचना z_app_region *)&__app_shmem_regions_end[0];
	region = (संरचना z_app_region *)&__app_shmem_regions_start[0];

	प्रति ( ; region < अन्तः; region++) {
		(शून्य)स्मृतिं_पूरय(region->bss_start, 0, region->bss_आकार);
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
	ARG_अप्रयुक्तम्(arg2);
	ARG_अप्रयुक्तम्(arg3);
	ARG_अप्रयुक्तम्(arg4);
	ARG_अप्रयुक्तम्(arg5);
	ARG_अप्रयुक्तम्(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	संहिता_UNREACHABLE; /* LCOV_EXCL_LINE */
}

स्थिर uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, शून्य *ssf)
{
	ARG_अप्रयुक्तम्(arg1);
	ARG_अप्रयुक्तम्(arg2);
	ARG_अप्रयुक्तम्(arg3);
	ARG_अप्रयुक्तम्(arg4);
	ARG_अप्रयुक्तम्(arg5);
	ARG_अप्रयुक्तम्(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	संहिता_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#अन्तर्भाव <zephyr/syscall_dispatch.c>
