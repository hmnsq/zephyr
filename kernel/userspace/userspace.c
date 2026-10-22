/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ضمن <zephyr/kernel.h>
#ضمن <string.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/rb.h>
#ضمن <zephyr/sys/sys_io.h>
#ضمن <ksched.h>
#ضمن <zephyr/syscall.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/device.h>
#ضمن <stdbool.h>
#ضمن <zephyr/app_memory/app_memdomain.h>
#ضمن <zephyr/sys/libc-hooks.h>
#ضمن <zephyr/sys/mutex.h>
#ضمن <zephyr/sys/util.h>
#ضمن <inttypes.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/cache.h>
#ضمن <kernel_internal.h>

#که_تعریف_شوی Z_LIBC_برخه_کول_EXISTS
K_APPMEM_برخه_کول_DEFINE(z_libc_برخه_کول);
#د_که_پای /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#که_تعریف_شوی CONFIG_MBEDTLS
K_APPMEM_برخه_کول_DEFINE(k_mbedtls_برخه_کول);
#د_که_پای /* CONFIG_MBEDTLS */

#ضمن <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* The originally synchronization strategy made heavy use of recursive
 * irq_locking, which ports poorly to spinlocks which are
 * non-recursive.  Rather than try to redesign as part of
 * spinlockification, this uses multiple locks to preserve the
 * original semantics exactly.  The locks are named for the data they
 * protect where possible, or just for the code that uses them where
 * not.
 */
#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS
ساكن بنية k_spinlock lists_قلف;       /* kobj dlist */

#که_تعریف_شوی CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#إذا defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#تعریف_کړه ډېری_ELEMENT_معلومات_اندازه(اندازه) \
	(حجم(بنية z_ډېری_معلومات) + CONFIG_PRIVILEGED_ډېری_اندازه + \
	Z_THREAD_ډېری_OBJ_ALIGN(اندازه) + K_THREAD_ډېری_LEN(اندازه))
#وإلا
#تعریف_کړه ډېری_ELEMENT_معلومات_اندازه(اندازه) (حجم(بنية z_ډېری_معلومات) + \
	K_THREAD_ډېری_LEN(اندازه))
#د_که_پای /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#وإلا
#تعریف_کړه ډېری_ELEMENT_معلومات_اندازه(اندازه) K_THREAD_ډېری_LEN(اندازه)
#د_که_پای /* CONFIG_GEN_PRIV_STACKS */

#د_که_پای /* CONFIG_DYNAMIC_OBJECTS */
ساكن بنية k_spinlock obj_قلف;         /* kobj struct data */

#تعریف_کړه MAX_THREAD_بټونه (CONFIG_MAX_THREAD_بایټونه * بټونه_فی_BYTE)

#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS
خارجي uint8_t _thread_idx_map[CONFIG_MAX_THREAD_بایټونه];

خارجي صحيح z_msgq_cleanup(بنية k_msgq *q, bool locked);
خارجي صحيح z_ډېری_cleanup(بنية k_ډېری *ډېری, bool locked);
خارجي صحيح z_timer_cleanup(بنية k_timer *timer, bool locked);
#د_که_پای /* CONFIG_DYNAMIC_OBJECTS */

ساكن فراغ unref_check(بنية k_object *ko, uintptr_t شاخص, bool locked);

ثابت حرف *otype_منزل_str(تعداد k_objects otype)
{
	ثابت حرف *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#که_تعریف_شوی CONFIG_LOG
	بدل (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	حالة K_OBJ_ANY:
		ret = "generic";
		اكسر;
	حالة K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		اكسر;
#ضمن <zephyr/otype-to-str.h>
	افتراضي:
		ret = "?";
		اكسر;
	}
#وإلا
	ARG_ناکارول_شوی(otype);
	ret = عدم;
#د_که_پای /* CONFIG_LOG */
	أرجع ret;
}

بنية perm_ctx {
	صحيح پلار_id;
	صحيح ماشوم_id;
	بنية k_thread *پلار;
};

#که_تعریف_شوی CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ډېری_پیدا_کړه(k_thread_ډېری_t *ډېری)
{
	بنية k_object *obj = k_object_پیدا_کړه(ډېری);

	__ASSERT(obj != عدم, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ډېری_ELEMENT,
		 "bad stack object");

	أرجع obj->معلومات.ډېری_معلومات->priv;
}
#د_که_پای /* CONFIG_GEN_PRIV_STACKS */

#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS

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
#که_تعریف_شوی ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#تعریف_کړه DYN_OBJ_معلومات_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#وإلا
#تعریف_کړه DYN_OBJ_معلومات_ALIGN_K_THREAD	(حجم(فراغ *))
#د_که_پای /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#که_تعریف_شوی CONFIG_DYNAMIC_THREAD_ډېری_اندازه
#إذا defined(CONFIG_MPU_ډېری_GUARD) || defined(CONFIG_PMP_ډېری_GUARD)
#تعریف_کړه DYN_OBJ_معلومات_ALIGN_K_THREAD_ډېری \
	Z_THREAD_ډېری_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ډېری_اندازه)
#وإلا
#تعریف_کړه DYN_OBJ_معلومات_ALIGN_K_THREAD_ډېری \
	Z_THREAD_ډېری_OBJ_ALIGN(CONFIG_PRIVILEGED_ډېری_اندازه)
#د_که_پای /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#وإلا
#تعریف_کړه DYN_OBJ_معلومات_ALIGN_K_THREAD_ډېری \
	Z_THREAD_ډېری_OBJ_ALIGN(ARCH_ډېری_PTR_ALIGN)
#د_که_پای /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#تعریف_کړه DYN_OBJ_معلومات_ALIGN		\
	MAX(DYN_OBJ_معلومات_ALIGN_K_THREAD, (حجم(فراغ *)))

بنية dyn_obj {
	بنية k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	فراغ *معلومات;
};

خارجي بنية k_object *z_object_gperf_پیدا_کړه(ثابت فراغ *obj);
خارجي فراغ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     فراغ *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
ساكن sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

ساكن size_t obj_اندازه_get(تعداد k_objects otype)
{
	size_t ret;

	بدل (otype) {
#ضمن <zephyr/otype-to-size.h>
	افتراضي:
		ret = حجم(ثابت بنية device);
		اكسر;
	}

	أرجع ret;
}

ساكن size_t obj_align_get(تعداد k_objects otype)
{
	size_t ret;

	بدل (otype) {
	حالة K_OBJ_THREAD:
#که_تعریف_شوی ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#وإلا
		ret = __alignof(بنية dyn_obj);
#د_که_پای /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		اكسر;
	افتراضي:
		ret = __alignof(بنية dyn_obj);
		اكسر;
	}

	أرجع ret;
}

ساكن بنية dyn_obj *dyn_object_پیدا_کړه_locked(ثابت فراغ *obj)
{
	بنية dyn_obj *غوټه;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, غوټه, dobj_list) {
		إذا (غوټه->kobj.name == obj) {
			أرجع غوټه;
		}
	}

	أرجع عدم;
}

ساكن بنية dyn_obj *dyn_object_پیدا_کړه(ثابت فراغ *obj)
{
	بنية dyn_obj *غوټه;
	k_spinlock_کلی_t کلی;

	کلی = k_spin_قلف(&lists_قلف);
	غوټه = dyn_object_پیدا_کړه_locked(obj);
	k_spin_unlock(&lists_قلف, کلی);

	أرجع غوټه;
}

ساكن فراغ clear_perms_cb(بنية k_object *ko, فراغ *ctx_ptr)
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
ساكن bool thread_idx_alloc(uintptr_t *tidx)
{
	صحيح i;
	صحيح idx;
	صحيح بنسټ;
	bool ret = false;
	k_spinlock_کلی_t کلی;

	بنسټ = 0;
	کلی = k_spin_قلف(&lists_قلف);
	لكل (i = 0; i < CONFIG_MAX_THREAD_بایټونه; i++) {
		idx = پیدا_کړه_lsb_سېټ(_thread_idx_map[i]);

		إذا (idx != 0) {
			بنية dyn_obj *obj, *بل;

			*tidx = بنسټ + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(بټ(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (فراغ *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_خوندي(&obj_list, obj, بل, dobj_list) {
				clear_perms_cb(&obj->kobj, (فراغ *)*tidx);
			}

			ret = true;
			اكسر;
		}

		بنسټ += 8;
	}

	k_spin_unlock(&lists_قلف, کلی);
	أرجع ret;
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
ساكن فراغ thread_idx_free(uintptr_t tidx)
{
	بنية dyn_obj *obj, *بل;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (فراغ *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_کلی_t کلی = k_spin_قلف(&lists_قلف);

	SYS_DLIST_FOR_EACH_CONTAINER_خوندي(&obj_list, obj, بل, dobj_list) {
		clear_perms_cb(&obj->kobj, (فراغ *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	صحيح بنسټ = tidx / عدد_بټونه_2(_thread_idx_map[0]);
	صحيح دننه_واټن = tidx % عدد_بټونه_2(_thread_idx_map[0]);

	_thread_idx_map[بنسټ] |= بټ(دننه_واټن);

	k_spin_unlock(&lists_قلف, کلی);
}

ساكن بنية k_object *dynamic_object_create(تعداد k_objects otype, size_t align,
					      size_t اندازه)
{
	بنية dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, حجم(بنية dyn_obj));
	إذا (dyn == عدم) {
		أرجع عدم;
	}

	إذا (otype == K_OBJ_THREAD_ډېری_ELEMENT) {
		size_t adjusted_اندازه;

		إذا (اندازه == 0) {
			k_free(dyn);
			أرجع عدم;
		}

		adjusted_اندازه = ډېری_ELEMENT_معلومات_اندازه(اندازه);
		إذا (adjusted_اندازه < اندازه) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			أرجع عدم;
		}

		dyn->معلومات = z_thread_aligned_alloc(DYN_OBJ_معلومات_ALIGN_K_THREAD_ډېری,
						     adjusted_اندازه);
		إذا (dyn->معلومات == عدم) {
			k_free(dyn);
			أرجع عدم;
		}

#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS_FORCE_ډېری_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		إذا (sys_cache_is_ptr_uncached(dyn->معلومات)) {
			dyn->معلومات = sys_cache_cached_ptr_get(dyn->معلومات);
		}
#د_که_پای /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#که_تعریف_شوی CONFIG_GEN_PRIV_STACKS
		بنية z_ډېری_معلومات *ډېری_معلومات = (بنية z_ډېری_معلومات *)
			((uint8_t *)dyn->معلومات + adjusted_اندازه - حجم(*ډېری_معلومات));
#إذا defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ډېری_معلومات->priv = (فراغ *)پړاو_پورته(((uint8_t *)dyn->معلومات + اندازه),
			  Z_THREAD_ډېری_OBJ_ALIGN(اندازه));
#وإلا
		ډېری_معلومات->priv = (uint8_t *)dyn->معلومات;
#د_که_پای /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ډېری_معلومات->اندازه = adjusted_اندازه;
		dyn->kobj.معلومات.ډېری_معلومات = ډېری_معلومات;
		dyn->kobj.name = dyn->معلومات;
#وإلا
		dyn->kobj.name = dyn->معلومات;
		dyn->kobj.معلومات.ډېری_اندازه = adjusted_اندازه;
#د_که_پای /* CONFIG_GEN_PRIV_STACKS */
	} وإلا {
		size_t ټول_اندازه = obj_اندازه_get(otype) + اندازه;

		إذا (ټول_اندازه < اندازه) {
			k_free(dyn);
			أرجع عدم;
		}

		dyn->معلومات = z_thread_aligned_alloc(align, ټول_اندازه);
		إذا (dyn->معلومات == عدم) {
			k_free(dyn);
			أرجع عدم;
		}
		dyn->kobj.name = dyn->معلومات;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(فراغ)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_بایټونه);

	k_spinlock_کلی_t کلی = k_spin_قلف(&lists_قلف);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_قلف, کلی);

	أرجع &dyn->kobj;
}

بنية k_object *k_object_create_dynamic_aligned(size_t align, size_t اندازه)
{
	بنية k_object *obj = dynamic_object_create(K_OBJ_ANY, align, اندازه);

	إذا (obj == عدم) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	أرجع obj;
}

ساكن فراغ *z_object_alloc(تعداد k_objects otype, size_t اندازه)
{
	بنية k_object *zo;
	uintptr_t tidx = 0;

	إذا ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_وروستی)) {
		LOG_ERR("bad object type %d requested", otype);
		أرجع عدم;
	}

	بدل (otype) {
	حالة K_OBJ_THREAD:
		إذا (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			أرجع عدم;
		}
		اكسر;
	/* The following are currently not allowed at all */
	حالة K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	حالة K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_منزل_str(otype));
		أرجع عدم;
	افتراضي:
		/* Remainder within bounds are permitted */
		اكسر;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), اندازه);
	إذا (zo == عدم) {
		إذا (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		أرجع عدم;
	}

	إذا (otype == K_OBJ_THREAD) {
		zo->معلومات.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_سېټ(zo, _current);

	/* Activates reference counting logic for automatic disposal when
	 * all permissions have been revoked
	 */
	zo->flags |= K_OBJ_FLAG_ALLOC;

	أرجع zo->name;
}

فراغ *z_impl_k_object_alloc(تعداد k_objects otype)
{
	أرجع z_object_alloc(otype, 0);
}

فراغ *z_impl_k_object_alloc_اندازه(تعداد k_objects otype, size_t اندازه)
{
	أرجع z_object_alloc(otype, اندازه);
}

فراغ k_object_free(فراغ *obj)
{
	بنية dyn_obj *dyn = عدم;
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
	k_spinlock_کلی_t کلی = k_spin_قلف(&lists_قلف);

	dyn = dyn_object_پیدا_کړه_locked(obj);

	إذا (dyn != عدم) {
		sys_dlist_remove(&dyn->dobj_list);

		إذا (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.معلومات.thread_id;
		}
	}
	k_spin_unlock(&lists_قلف, کلی);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	إذا (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	إذا (dyn != عدم) {
#که_تعریف_شوی CONFIG_OBJ_CORE
		إذا (dyn->kobj.type != K_OBJ_THREAD_ډېری_ELEMENT) {
			k_obj_core_evict_range(obj, obj_اندازه_get(dyn->kobj.type));
		}
#د_که_پای /* CONFIG_OBJ_CORE */
#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS_FORCE_ډېری_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		إذا (dyn->kobj.type == K_OBJ_THREAD_ډېری_ELEMENT) {
			uint8_t *ډېری;

			إذا (sys_cache_is_ptr_cached(dyn->معلومات)) {
				ډېری = sys_cache_uncached_ptr_get(dyn->معلومات);
			} وإلا {
				ډېری = dyn->معلومات;
			}

			k_free(ډېری);
		} وإلا
#د_که_پای /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->معلومات);
		}
		k_free(dyn);
	}
}

بنية k_object *k_object_پیدا_کړه(ثابت فراغ *obj)
{
	بنية k_object *ret;

	ret = z_object_gperf_پیدا_کړه(obj);

	إذا (ret == عدم) {
		بنية dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_پیدا_کړه(obj);
		إذا (dyn != عدم) {
			ret = &dyn->kobj;
		}
	}

	أرجع ret;
}

فراغ k_object_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
	بنية dyn_obj *obj, *بل;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_کلی_t کلی = k_spin_قلف(&lists_قلف);

	SYS_DLIST_FOR_EACH_CONTAINER_خوندي(&obj_list, obj, بل, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_قلف, کلی);
}
#د_که_پای /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_معلومات.متن.dummies)
__weak بنية k_object *z_object_gperf_پیدا_کړه(ثابت فراغ *obj)
{
	ARG_ناکارول_شوی(obj);
	أرجع عدم;
}
Z_GENERIC_SECTION(.kobject_معلومات.متن.dummies)
__weak فراغ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
	ARG_ناکارول_شوی(func);
	ARG_ناکارول_شوی(context);
}
#وإلا
Z_GENERIC_SECTION(.kobject_معلومات.متن.dummies)
__weak بنية k_object *k_object_پیدا_کړه(ثابت فراغ *obj)
{
	أرجع عدم;
}
Z_GENERIC_SECTION(.kobject_معلومات.متن.dummies)
__weak فراغ k_object_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
}
#د_که_پای

ساكن بینښه صحيح thread_شاخص_get(بنية k_thread *thread)
{
	بنية k_object *ko;

	ko = k_object_پیدا_کړه(thread);

	إذا (ko == عدم) {
		أرجع -1;
	}

	أرجع ko->معلومات.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ساكن فراغ unref_check(بنية k_object *ko, uintptr_t شاخص, bool locked)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&obj_قلف);

	sys_bitfield_clear_بټ((mem_addr_t)&ko->perms, شاخص);

#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS
	إذا ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		اذهب بهر;
	}

	فراغ *vko = ko;

	بنية dyn_obj *dyn = CONTAINER_د(vko, بنية dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, بنية dyn_obj), "unaligned z_object");

	لكل (صحيح i = 0; i < CONFIG_MAX_THREAD_بایټونه; i++) {
		إذا (ko->perms[i] != 0U) {
			اذهب بهر;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	بدل (ko->type) {
	حالة K_OBJ_MSGQ:
		(فراغ)z_msgq_cleanup((بنية k_msgq *)ko->name, locked);
		اكسر;
	حالة K_OBJ_ډېری:
		(فراغ)z_ډېری_cleanup((بنية k_ډېری *)ko->name, locked);
		اكسر;
	حالة K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		إذا ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(فراغ)z_timer_cleanup((بنية k_timer *)ko->name, locked);
		}
		اكسر;
	افتراضي:
		/* Nothing to do */
		اكسر;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->معلومات);
	k_free(dyn);
بهر:
#د_که_پای /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_قلف, کلی);
}

ساكن فراغ wordlist_cb(بنية k_object *ko, فراغ *ctx_ptr)
{
	بنية perm_ctx *ctx = (بنية perm_ctx *)ctx_ptr;

	إذا (sys_bitfield_test_بټ((mem_addr_t)&ko->perms, ctx->پلار_id) &&
				  ((بنية k_thread *)ko->name != ctx->پلار)) {
		sys_bitfield_سېټ_بټ((mem_addr_t)&ko->perms, ctx->ماشوم_id);
	}
}

فراغ k_thread_perms_inherit(بنية k_thread *پلار, بنية k_thread *ماشوم)
{
	بنية perm_ctx ctx = {
		thread_شاخص_get(پلار),
		thread_شاخص_get(ماشوم),
		پلار
	};

	إذا ((ctx.پلار_id != -1) && (ctx.ماشوم_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

فراغ k_thread_perms_سېټ(بنية k_object *ko, بنية k_thread *thread)
{
	صحيح شاخص = thread_شاخص_get(thread);

	إذا (شاخص != -1) {
		sys_bitfield_سېټ_بټ((mem_addr_t)&ko->perms, شاخص);
	}
}

فراغ k_thread_perms_clear(بنية k_object *ko, بنية k_thread *thread)
{
	صحيح شاخص = thread_شاخص_get(thread);

	إذا (شاخص != -1) {
		sys_bitfield_clear_بټ((mem_addr_t)&ko->perms, شاخص);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS
		k_spinlock_کلی_t کلی = k_spin_قلف(&lists_قلف);

		unref_check(ko, شاخص, false);
		k_spin_unlock(&lists_قلف, کلی);
#وإلا
		unref_check(ko, شاخص, false);
#د_که_پای /* CONFIG_DYNAMIC_OBJECTS */
	}
}

ساكن فراغ clear_perms_cb_locked(بنية k_object *ko, فراغ *ctx_ptr)
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
فراغ k_thread_perms_all_clear(بنية k_thread *thread)
{
	uintptr_t شاخص = thread_شاخص_get(thread);

	إذا ((صحيح)شاخص != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (فراغ *)شاخص);
	}
}

ساكن صحيح thread_perms_test(بنية k_object *ko)
{
	صحيح شاخص;

	إذا ((ko->flags & K_OBJ_FLAG_عامه) != 0U) {
		أرجع 1;
	}

	شاخص = thread_شاخص_get(_current);
	إذا (شاخص != -1) {
		أرجع sys_bitfield_test_بټ((mem_addr_t)&ko->perms, شاخص);
	}
	أرجع 0;
}

ساكن فراغ dump_permission_error(بنية k_object *ko)
{
	صحيح شاخص = thread_شاخص_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, شاخص,
		otype_منزل_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, حجم(ko->perms), "permission bitmap");
}

فراغ k_object_dump_error(صحيح retval, ثابت فراغ *obj, بنية k_object *ko,
			تعداد k_objects otype)
{
	بدل (retval) {
	حالة -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_منزل_str(otype));
		إذا (ko == عدم) {
			LOG_ERR("address is not a known kernel object");
		} وإلا {
			LOG_ERR("address is actually a %s",
				otype_منزل_str(ko->type));
		}
		اكسر;
	حالة -EPERM:
		dump_permission_error(ko);
		اكسر;
	حالة -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		اكسر;
	حالة -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_منزل_str(otype));
		اكسر;
	افتراضي:
		/* Not handled error */
		اكسر;
	}
}

فراغ z_impl_k_object_access_grant(ثابت فراغ *object, بنية k_thread *thread)
{
	بنية k_object *ko = k_object_پیدا_کړه(object);

	إذا (ko != عدم) {
		k_thread_perms_سېټ(ko, thread);
	}
}

فراغ k_object_access_revoke(ثابت فراغ *object, بنية k_thread *thread)
{
	بنية k_object *ko = k_object_پیدا_کړه(object);

	إذا (ko != عدم) {
		k_thread_perms_clear(ko, thread);
	}
}

فراغ k_object_access_revoke_others(ثابت فراغ *object)
{
	بنية k_object *ko = k_object_پیدا_کړه(object);

	إذا (ko != عدم) {
		(فراغ)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_سېټ(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_عامه;
	}
}

فراغ z_impl_k_object_release(ثابت فراغ *object)
{
	k_object_access_revoke(object, _current);
}

فراغ k_object_access_all_grant(ثابت فراغ *object)
{
	بنية k_object *ko = k_object_پیدا_کړه(object);

	إذا (ko != عدم) {
		ko->flags |= K_OBJ_FLAG_عامه;
	}
}

صحيح k_object_validate(بنية k_object *ko, تعداد k_objects otype,
		       تعداد _obj_init_check init)
{
	إذا (unlikely(ko == عدم)) {
		أرجع -EBADF;
	}

	إذا (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		إذا ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_لومړی) ||
		    (ko->type > K_OBJ_DRIVER_وروستی)) {
			أرجع -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	إذا (unlikely(thread_perms_test(ko) == 0)) {
		أرجع -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	إذا (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		إذا (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			أرجع -EINVAL;
		}
	} وإلا إذا (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		إذا (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			أرجع -EADDRINUSE;
		}
	} وإلا {
		/* _OBJ_INIT_ANY */
	}

	أرجع 0;
}

صحيح z_impl_k_object_access_check(ثابت فراغ *object)
{
	أرجع k_object_validate(k_object_پیدا_کړه(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

فراغ k_object_init(ثابت فراغ *obj)
{
	بنية k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_پیدا_کړه(obj);
	إذا (ko == عدم) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		أرجع;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

فراغ k_object_recycle(ثابت فراغ *obj)
{
	بنية k_object *ko = k_object_پیدا_کړه(obj);

	إذا (ko != عدم) {
		(فراغ)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_سېټ(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

فراغ k_object_uninit(ثابت فراغ *obj)
{
	بنية k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_پیدا_کړه(obj);
	إذا (ko == عدم) {
		أرجع;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
فراغ *k_usermode_alloc_from_copy(ثابت فراغ *src, size_t اندازه)
{
	فراغ *dst = عدم;

	/* Does the caller in user mode have access to read this memory? */
	إذا (K_SYSCALL_حافظه_READ(src, اندازه)) {
		اذهب بهر_err;
	}

	dst = z_thread_malloc(اندازه);
	إذا (dst == عدم) {
		LOG_ERR("out of thread resource pool memory (%zu)", اندازه);
		اذهب بهر_err;
	}

	(فراغ)حافظه_نقل(dst, src, اندازه);
بهر_err:
	أرجع dst;
}

ساكن صحيح user_copy(فراغ *dst, ثابت فراغ *src, size_t اندازه, bool منزل_user)
{
	صحيح ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	إذا (منزل_user ? K_SYSCALL_حافظه_WRITE(dst, اندازه) :
			K_SYSCALL_حافظه_READ(src, اندازه)) {
		اذهب بهر_err;
	}

	(فراغ)حافظه_نقل(dst, src, اندازه);
	ret = 0;
بهر_err:
	أرجع ret;
}

صحيح k_usermode_from_copy(فراغ *dst, ثابت فراغ *src, size_t اندازه)
{
	أرجع user_copy(dst, src, اندازه, false);
}

صحيح k_usermode_منزل_copy(فراغ *dst, ثابت فراغ *src, size_t اندازه)
{
	أرجع user_copy(dst, src, اندازه, true);
}

حرف *k_usermode_string_alloc_copy(ثابت حرف *src, size_t maxlen)
{
	size_t actual_len;
	صحيح err;
	حرف *ret = عدم;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	إذا (err != 0) {
		اذهب بهر;
	}
	إذا (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		اذهب بهر;
	}
	إذا (اندازه_جمع_کړه_ډکېدنه(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		اذهب بهر;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	إذا (ret != عدم) {
		ret[actual_len - 1U] = '\0';
	}
بهر:
	أرجع ret;
}

صحيح k_usermode_string_copy(حرف *dst, ثابت حرف *src, size_t maxlen)
{
	size_t actual_len;
	صحيح ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	إذا (err != 0) {
		ret = EFAULT;
		اذهب بهر;
	}
	إذا (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		اذهب بهر;
	}
	إذا (اندازه_جمع_کړه_ډکېدنه(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		اذهب بهر;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
بهر:
	أرجع ret;
}

/*
 * Application memory region initialization
 */

خارجي حرف __app_shmem_regions_start[];
خارجي حرف __app_shmem_regions_end[];

ساكن فراغ app_shmem_bss_zero(فراغ)
{
	بنية z_app_region *region, *پای_ټکی;


	پای_ټکی = (بنية z_app_region *)&__app_shmem_regions_end[0];
	region = (بنية z_app_region *)&__app_shmem_regions_start[0];

	لكل ( ; region < پای_ټکی; region++) {
		(فراغ)memset(region->bss_پیل, 0, region->bss_اندازه);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

ساكن uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     فراغ *ssf)
{
	ARG_ناکارول_شوی(arg2);
	ARG_ناکارول_شوی(arg3);
	ARG_ناکارول_شوی(arg4);
	ARG_ناکارول_شوی(arg5);
	ARG_ناکارول_شوی(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	کوډ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساكن uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, فراغ *ssf)
{
	ARG_ناکارول_شوی(arg1);
	ARG_ناکارول_شوی(arg2);
	ARG_ناکارول_شوی(arg3);
	ARG_ناکارول_شوی(arg4);
	ARG_ناکارول_شوی(arg5);
	ARG_ناکارول_شوی(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	کوډ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#ضمن <zephyr/syscall_dispatch.c>
