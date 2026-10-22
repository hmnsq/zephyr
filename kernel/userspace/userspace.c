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

#اگر_تعریف_شده Z_LIBC_افراز_EXISTS
K_APPMEM_افراز_DEFINE(z_libc_افراز);
#پایان_اگر /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#اگر_تعریف_شده CONFIG_MBEDTLS
K_APPMEM_افراز_DEFINE(k_mbedtls_افراز);
#پایان_اگر /* CONFIG_MBEDTLS */

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
#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS
ساكن بنية k_spinlock lists_قفل;       /* kobj dlist */

#اگر_تعریف_شده CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#إذا defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#تعریف_کن پشته_ELEMENT_داده_اندازه(اندازه) \
	(حجم(بنية z_پشته_داده) + CONFIG_PRIVILEGED_پشته_اندازه + \
	Z_THREAD_پشته_OBJ_ALIGN(اندازه) + K_THREAD_پشته_LEN(اندازه))
#وإلا
#تعریف_کن پشته_ELEMENT_داده_اندازه(اندازه) (حجم(بنية z_پشته_داده) + \
	K_THREAD_پشته_LEN(اندازه))
#پایان_اگر /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#وإلا
#تعریف_کن پشته_ELEMENT_داده_اندازه(اندازه) K_THREAD_پشته_LEN(اندازه)
#پایان_اگر /* CONFIG_GEN_PRIV_STACKS */

#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS */
ساكن بنية k_spinlock obj_قفل;         /* kobj struct data */

#تعریف_کن MAX_THREAD_بیتها (CONFIG_MAX_THREAD_بایتها * بیتها_در_هر_BYTE)

#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS
خارجي uint8_t _thread_idx_map[CONFIG_MAX_THREAD_بایتها];

خارجي صحيح z_msgq_cleanup(بنية k_msgq *q, bool locked);
خارجي صحيح z_پشته_cleanup(بنية k_پشته *پشته, bool locked);
خارجي صحيح z_timer_cleanup(بنية k_timer *timer, bool locked);
#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS */

ساكن فراغ unref_check(بنية k_object *ko, uintptr_t شاخص, bool locked);

ثابت حرف *otype_مقصد_str(تعداد k_objects otype)
{
	ثابت حرف *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#اگر_تعریف_شده CONFIG_LOG
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
	ARG_استفادهنشده(otype);
	ret = عدم;
#پایان_اگر /* CONFIG_LOG */
	أرجع ret;
}

بنية perm_ctx {
	صحيح والد_id;
	صحيح فرزند_id;
	بنية k_thread *والد;
};

#اگر_تعریف_شده CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_پشته_پیدا_کن(k_thread_پشته_t *پشته)
{
	بنية k_object *obj = k_object_پیدا_کن(پشته);

	__ASSERT(obj != عدم, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_پشته_ELEMENT,
		 "bad stack object");

	أرجع obj->داده.پشته_داده->priv;
}
#پایان_اگر /* CONFIG_GEN_PRIV_STACKS */

#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS

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
#اگر_تعریف_شده ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#تعریف_کن DYN_OBJ_داده_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#وإلا
#تعریف_کن DYN_OBJ_داده_ALIGN_K_THREAD	(حجم(فراغ *))
#پایان_اگر /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#اگر_تعریف_شده CONFIG_DYNAMIC_THREAD_پشته_اندازه
#إذا defined(CONFIG_MPU_پشته_GUARD) || defined(CONFIG_PMP_پشته_GUARD)
#تعریف_کن DYN_OBJ_داده_ALIGN_K_THREAD_پشته \
	Z_THREAD_پشته_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_پشته_اندازه)
#وإلا
#تعریف_کن DYN_OBJ_داده_ALIGN_K_THREAD_پشته \
	Z_THREAD_پشته_OBJ_ALIGN(CONFIG_PRIVILEGED_پشته_اندازه)
#پایان_اگر /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#وإلا
#تعریف_کن DYN_OBJ_داده_ALIGN_K_THREAD_پشته \
	Z_THREAD_پشته_OBJ_ALIGN(ARCH_پشته_PTR_ALIGN)
#پایان_اگر /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#تعریف_کن DYN_OBJ_داده_ALIGN		\
	MAX(DYN_OBJ_داده_ALIGN_K_THREAD, (حجم(فراغ *)))

بنية dyn_obj {
	بنية k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	فراغ *داده;
};

خارجي بنية k_object *z_object_gperf_پیدا_کن(ثابت فراغ *obj);
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
#اگر_تعریف_شده ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#وإلا
		ret = __alignof(بنية dyn_obj);
#پایان_اگر /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		اكسر;
	افتراضي:
		ret = __alignof(بنية dyn_obj);
		اكسر;
	}

	أرجع ret;
}

ساكن بنية dyn_obj *dyn_object_پیدا_کن_locked(ثابت فراغ *obj)
{
	بنية dyn_obj *گره;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, گره, dobj_list) {
		إذا (گره->kobj.name == obj) {
			أرجع گره;
		}
	}

	أرجع عدم;
}

ساكن بنية dyn_obj *dyn_object_پیدا_کن(ثابت فراغ *obj)
{
	بنية dyn_obj *گره;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_قفل(&lists_قفل);
	گره = dyn_object_پیدا_کن_locked(obj);
	k_spin_unlock(&lists_قفل, کلید);

	أرجع گره;
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
	صحيح پایه;
	bool ret = false;
	k_spinlock_کلید_t کلید;

	پایه = 0;
	کلید = k_spin_قفل(&lists_قفل);
	لكل (i = 0; i < CONFIG_MAX_THREAD_بایتها; i++) {
		idx = پیدا_کن_lsb_مجموعه(_thread_idx_map[i]);

		إذا (idx != 0) {
			بنية dyn_obj *obj, *بعدی;

			*tidx = پایه + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(بیت(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (فراغ *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_امن(&obj_list, obj, بعدی, dobj_list) {
				clear_perms_cb(&obj->kobj, (فراغ *)*tidx);
			}

			ret = true;
			اكسر;
		}

		پایه += 8;
	}

	k_spin_unlock(&lists_قفل, کلید);
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
	بنية dyn_obj *obj, *بعدی;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (فراغ *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_کلید_t کلید = k_spin_قفل(&lists_قفل);

	SYS_DLIST_FOR_EACH_CONTAINER_امن(&obj_list, obj, بعدی, dobj_list) {
		clear_perms_cb(&obj->kobj, (فراغ *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	صحيح پایه = tidx / رقم_بیتها_2(_thread_idx_map[0]);
	صحيح جابه_جایی = tidx % رقم_بیتها_2(_thread_idx_map[0]);

	_thread_idx_map[پایه] |= بیت(جابه_جایی);

	k_spin_unlock(&lists_قفل, کلید);
}

ساكن بنية k_object *dynamic_object_create(تعداد k_objects otype, size_t align,
					      size_t اندازه)
{
	بنية dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, حجم(بنية dyn_obj));
	إذا (dyn == عدم) {
		أرجع عدم;
	}

	إذا (otype == K_OBJ_THREAD_پشته_ELEMENT) {
		size_t adjusted_اندازه;

		إذا (اندازه == 0) {
			k_free(dyn);
			أرجع عدم;
		}

		adjusted_اندازه = پشته_ELEMENT_داده_اندازه(اندازه);
		إذا (adjusted_اندازه < اندازه) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			أرجع عدم;
		}

		dyn->داده = z_thread_aligned_alloc(DYN_OBJ_داده_ALIGN_K_THREAD_پشته,
						     adjusted_اندازه);
		إذا (dyn->داده == عدم) {
			k_free(dyn);
			أرجع عدم;
		}

#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS_FORCE_پشته_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		إذا (sys_cache_is_ptr_uncached(dyn->داده)) {
			dyn->داده = sys_cache_cached_ptr_get(dyn->داده);
		}
#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#اگر_تعریف_شده CONFIG_GEN_PRIV_STACKS
		بنية z_پشته_داده *پشته_داده = (بنية z_پشته_داده *)
			((uint8_t *)dyn->داده + adjusted_اندازه - حجم(*پشته_داده));
#إذا defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		پشته_داده->priv = (فراغ *)دور_به_بالا(((uint8_t *)dyn->داده + اندازه),
			  Z_THREAD_پشته_OBJ_ALIGN(اندازه));
#وإلا
		پشته_داده->priv = (uint8_t *)dyn->داده;
#پایان_اگر /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		پشته_داده->اندازه = adjusted_اندازه;
		dyn->kobj.داده.پشته_داده = پشته_داده;
		dyn->kobj.name = dyn->داده;
#وإلا
		dyn->kobj.name = dyn->داده;
		dyn->kobj.داده.پشته_اندازه = adjusted_اندازه;
#پایان_اگر /* CONFIG_GEN_PRIV_STACKS */
	} وإلا {
		size_t مجموع_اندازه = obj_اندازه_get(otype) + اندازه;

		إذا (مجموع_اندازه < اندازه) {
			k_free(dyn);
			أرجع عدم;
		}

		dyn->داده = z_thread_aligned_alloc(align, مجموع_اندازه);
		إذا (dyn->داده == عدم) {
			k_free(dyn);
			أرجع عدم;
		}
		dyn->kobj.name = dyn->داده;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(فراغ)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_بایتها);

	k_spinlock_کلید_t کلید = k_spin_قفل(&lists_قفل);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_قفل, کلید);

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

	إذا ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_آخرین)) {
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
			otype_مقصد_str(otype));
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
		zo->داده.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_مجموعه(zo, _current);

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
	k_spinlock_کلید_t کلید = k_spin_قفل(&lists_قفل);

	dyn = dyn_object_پیدا_کن_locked(obj);

	إذا (dyn != عدم) {
		sys_dlist_remove(&dyn->dobj_list);

		إذا (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.داده.thread_id;
		}
	}
	k_spin_unlock(&lists_قفل, کلید);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	إذا (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	إذا (dyn != عدم) {
#اگر_تعریف_شده CONFIG_OBJ_CORE
		إذا (dyn->kobj.type != K_OBJ_THREAD_پشته_ELEMENT) {
			k_obj_core_evict_range(obj, obj_اندازه_get(dyn->kobj.type));
		}
#پایان_اگر /* CONFIG_OBJ_CORE */
#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS_FORCE_پشته_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		إذا (dyn->kobj.type == K_OBJ_THREAD_پشته_ELEMENT) {
			uint8_t *پشته;

			إذا (sys_cache_is_ptr_cached(dyn->داده)) {
				پشته = sys_cache_uncached_ptr_get(dyn->داده);
			} وإلا {
				پشته = dyn->داده;
			}

			k_free(پشته);
		} وإلا
#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->داده);
		}
		k_free(dyn);
	}
}

بنية k_object *k_object_پیدا_کن(ثابت فراغ *obj)
{
	بنية k_object *ret;

	ret = z_object_gperf_پیدا_کن(obj);

	إذا (ret == عدم) {
		بنية dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_پیدا_کن(obj);
		إذا (dyn != عدم) {
			ret = &dyn->kobj;
		}
	}

	أرجع ret;
}

فراغ k_object_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
	بنية dyn_obj *obj, *بعدی;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_کلید_t کلید = k_spin_قفل(&lists_قفل);

	SYS_DLIST_FOR_EACH_CONTAINER_امن(&obj_list, obj, بعدی, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_قفل, کلید);
}
#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_داده.متن.dummies)
__weak بنية k_object *z_object_gperf_پیدا_کن(ثابت فراغ *obj)
{
	ARG_استفادهنشده(obj);
	أرجع عدم;
}
Z_GENERIC_SECTION(.kobject_داده.متن.dummies)
__weak فراغ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
	ARG_استفادهنشده(func);
	ARG_استفادهنشده(context);
}
#وإلا
Z_GENERIC_SECTION(.kobject_داده.متن.dummies)
__weak بنية k_object *k_object_پیدا_کن(ثابت فراغ *obj)
{
	أرجع عدم;
}
Z_GENERIC_SECTION(.kobject_داده.متن.dummies)
__weak فراغ k_object_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
}
#پایان_اگر

ساكن بیعلامت صحيح thread_شاخص_get(بنية k_thread *thread)
{
	بنية k_object *ko;

	ko = k_object_پیدا_کن(thread);

	إذا (ko == عدم) {
		أرجع -1;
	}

	أرجع ko->داده.thread_id;
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
	k_spinlock_کلید_t کلید = k_spin_قفل(&obj_قفل);

	sys_bitfield_clear_بیت((mem_addr_t)&ko->perms, شاخص);

#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS
	إذا ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		اذهب بیرون;
	}

	فراغ *vko = ko;

	بنية dyn_obj *dyn = CONTAINER_از(vko, بنية dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, بنية dyn_obj), "unaligned z_object");

	لكل (صحيح i = 0; i < CONFIG_MAX_THREAD_بایتها; i++) {
		إذا (ko->perms[i] != 0U) {
			اذهب بیرون;
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
	حالة K_OBJ_پشته:
		(فراغ)z_پشته_cleanup((بنية k_پشته *)ko->name, locked);
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
	k_free(dyn->داده);
	k_free(dyn);
بیرون:
#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_قفل, کلید);
}

ساكن فراغ wordlist_cb(بنية k_object *ko, فراغ *ctx_ptr)
{
	بنية perm_ctx *ctx = (بنية perm_ctx *)ctx_ptr;

	إذا (sys_bitfield_test_بیت((mem_addr_t)&ko->perms, ctx->والد_id) &&
				  ((بنية k_thread *)ko->name != ctx->والد)) {
		sys_bitfield_مجموعه_بیت((mem_addr_t)&ko->perms, ctx->فرزند_id);
	}
}

فراغ k_thread_perms_inherit(بنية k_thread *والد, بنية k_thread *فرزند)
{
	بنية perm_ctx ctx = {
		thread_شاخص_get(والد),
		thread_شاخص_get(فرزند),
		والد
	};

	إذا ((ctx.والد_id != -1) && (ctx.فرزند_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

فراغ k_thread_perms_مجموعه(بنية k_object *ko, بنية k_thread *thread)
{
	صحيح شاخص = thread_شاخص_get(thread);

	إذا (شاخص != -1) {
		sys_bitfield_مجموعه_بیت((mem_addr_t)&ko->perms, شاخص);
	}
}

فراغ k_thread_perms_clear(بنية k_object *ko, بنية k_thread *thread)
{
	صحيح شاخص = thread_شاخص_get(thread);

	إذا (شاخص != -1) {
		sys_bitfield_clear_بیت((mem_addr_t)&ko->perms, شاخص);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS
		k_spinlock_کلید_t کلید = k_spin_قفل(&lists_قفل);

		unref_check(ko, شاخص, false);
		k_spin_unlock(&lists_قفل, کلید);
#وإلا
		unref_check(ko, شاخص, false);
#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS */
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

	إذا ((ko->flags & K_OBJ_FLAG_عمومی) != 0U) {
		أرجع 1;
	}

	شاخص = thread_شاخص_get(_current);
	إذا (شاخص != -1) {
		أرجع sys_bitfield_test_بیت((mem_addr_t)&ko->perms, شاخص);
	}
	أرجع 0;
}

ساكن فراغ dump_permission_error(بنية k_object *ko)
{
	صحيح شاخص = thread_شاخص_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, شاخص,
		otype_مقصد_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, حجم(ko->perms), "permission bitmap");
}

فراغ k_object_dump_error(صحيح retval, ثابت فراغ *obj, بنية k_object *ko,
			تعداد k_objects otype)
{
	بدل (retval) {
	حالة -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_مقصد_str(otype));
		إذا (ko == عدم) {
			LOG_ERR("address is not a known kernel object");
		} وإلا {
			LOG_ERR("address is actually a %s",
				otype_مقصد_str(ko->type));
		}
		اكسر;
	حالة -EPERM:
		dump_permission_error(ko);
		اكسر;
	حالة -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		اكسر;
	حالة -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_مقصد_str(otype));
		اكسر;
	افتراضي:
		/* Not handled error */
		اكسر;
	}
}

فراغ z_impl_k_object_access_grant(ثابت فراغ *object, بنية k_thread *thread)
{
	بنية k_object *ko = k_object_پیدا_کن(object);

	إذا (ko != عدم) {
		k_thread_perms_مجموعه(ko, thread);
	}
}

فراغ k_object_access_revoke(ثابت فراغ *object, بنية k_thread *thread)
{
	بنية k_object *ko = k_object_پیدا_کن(object);

	إذا (ko != عدم) {
		k_thread_perms_clear(ko, thread);
	}
}

فراغ k_object_access_revoke_others(ثابت فراغ *object)
{
	بنية k_object *ko = k_object_پیدا_کن(object);

	إذا (ko != عدم) {
		(فراغ)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_مجموعه(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_عمومی;
	}
}

فراغ z_impl_k_object_release(ثابت فراغ *object)
{
	k_object_access_revoke(object, _current);
}

فراغ k_object_access_all_grant(ثابت فراغ *object)
{
	بنية k_object *ko = k_object_پیدا_کن(object);

	إذا (ko != عدم) {
		ko->flags |= K_OBJ_FLAG_عمومی;
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
		    (ko->type < K_OBJ_DRIVER_اولی) ||
		    (ko->type > K_OBJ_DRIVER_آخرین)) {
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
	أرجع k_object_validate(k_object_پیدا_کن(object), K_OBJ_ANY, _OBJ_INIT_ANY);
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

	ko = k_object_پیدا_کن(obj);
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
	بنية k_object *ko = k_object_پیدا_کن(obj);

	إذا (ko != عدم) {
		(فراغ)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_مجموعه(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

فراغ k_object_uninit(ثابت فراغ *obj)
{
	بنية k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_پیدا_کن(obj);
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
		اذهب بیرون_err;
	}

	dst = z_thread_malloc(اندازه);
	إذا (dst == عدم) {
		LOG_ERR("out of thread resource pool memory (%zu)", اندازه);
		اذهب بیرون_err;
	}

	(فراغ)کپی_حافظه(dst, src, اندازه);
بیرون_err:
	أرجع dst;
}

ساكن صحيح user_copy(فراغ *dst, ثابت فراغ *src, size_t اندازه, bool مقصد_user)
{
	صحيح ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	إذا (مقصد_user ? K_SYSCALL_حافظه_WRITE(dst, اندازه) :
			K_SYSCALL_حافظه_READ(src, اندازه)) {
		اذهب بیرون_err;
	}

	(فراغ)کپی_حافظه(dst, src, اندازه);
	ret = 0;
بیرون_err:
	أرجع ret;
}

صحيح k_usermode_from_copy(فراغ *dst, ثابت فراغ *src, size_t اندازه)
{
	أرجع user_copy(dst, src, اندازه, false);
}

صحيح k_usermode_مقصد_copy(فراغ *dst, ثابت فراغ *src, size_t اندازه)
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
		اذهب بیرون;
	}
	إذا (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		اذهب بیرون;
	}
	إذا (اندازه_جمع_کن_سرریز(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		اذهب بیرون;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	إذا (ret != عدم) {
		ret[actual_len - 1U] = '\0';
	}
بیرون:
	أرجع ret;
}

صحيح k_usermode_string_copy(حرف *dst, ثابت حرف *src, size_t maxlen)
{
	size_t actual_len;
	صحيح ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	إذا (err != 0) {
		ret = EFAULT;
		اذهب بیرون;
	}
	إذا (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		اذهب بیرون;
	}
	إذا (اندازه_جمع_کن_سرریز(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		اذهب بیرون;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
بیرون:
	أرجع ret;
}

/*
 * Application memory region initialization
 */

خارجي حرف __app_shmem_regions_start[];
خارجي حرف __app_shmem_regions_end[];

ساكن فراغ app_shmem_bss_zero(فراغ)
{
	بنية z_app_region *region, *انتها;


	انتها = (بنية z_app_region *)&__app_shmem_regions_end[0];
	region = (بنية z_app_region *)&__app_shmem_regions_start[0];

	لكل ( ; region < انتها; region++) {
		(فراغ)memset(region->bss_شروع, 0, region->bss_اندازه);
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
	ARG_استفادهنشده(arg2);
	ARG_استفادهنشده(arg3);
	ARG_استفادهنشده(arg4);
	ARG_استفادهنشده(arg5);
	ARG_استفادهنشده(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	کد_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساكن uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, فراغ *ssf)
{
	ARG_استفادهنشده(arg1);
	ARG_استفادهنشده(arg2);
	ARG_استفادهنشده(arg3);
	ARG_استفادهنشده(arg4);
	ARG_استفادهنشده(arg5);
	ARG_استفادهنشده(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	کد_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#ضمن <zephyr/syscall_dispatch.c>
