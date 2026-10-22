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

#جے_تعریف_شدہ Z_LIBC_حصے_کرو_EXISTS
K_APPMEM_حصے_کرو_DEFINE(z_libc_حصے_کرو);
#جے_انت /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#جے_تعریف_شدہ CONFIG_MBEDTLS
K_APPMEM_حصے_کرو_DEFINE(k_mbedtls_حصے_کرو);
#جے_انت /* CONFIG_MBEDTLS */

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
#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
ساكن بنية k_spinlock lists_جندرا;       /* kobj dlist */

#جے_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#إذا defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#تعریف_کرو ڈھیر_ELEMENT_انکڑے_ناپ(ناپ) \
	(حجم(بنية z_ڈھیر_انکڑے) + CONFIG_PRIVILEGED_ڈھیر_ناپ + \
	Z_THREAD_ڈھیر_OBJ_ALIGN(ناپ) + K_THREAD_ڈھیر_LEN(ناپ))
#وإلا
#تعریف_کرو ڈھیر_ELEMENT_انکڑے_ناپ(ناپ) (حجم(بنية z_ڈھیر_انکڑے) + \
	K_THREAD_ڈھیر_LEN(ناپ))
#جے_انت /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#وإلا
#تعریف_کرو ڈھیر_ELEMENT_انکڑے_ناپ(ناپ) K_THREAD_ڈھیر_LEN(ناپ)
#جے_انت /* CONFIG_GEN_PRIV_STACKS */

#جے_انت /* CONFIG_DYNAMIC_OBJECTS */
ساكن بنية k_spinlock obj_جندرا;         /* kobj struct data */

#تعریف_کرو MAX_THREAD_بٹ (CONFIG_MAX_THREAD_بائٹاں * بٹ_فی_BYTE)

#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
خارجي uint8_t _thread_idx_map[CONFIG_MAX_THREAD_بائٹاں];

خارجي صحيح z_msgq_cleanup(بنية k_msgq *q, bool locked);
خارجي صحيح z_ڈھیر_cleanup(بنية k_ڈھیر_2 *ڈھیر, bool locked);
خارجي صحيح z_timer_cleanup(بنية k_timer *timer, bool locked);
#جے_انت /* CONFIG_DYNAMIC_OBJECTS */

ساكن فراغ unref_check(بنية k_object *ko, uintptr_t اشاریہ, bool locked);

ثابت حرف *otype_منزل_str(تعداد k_objects otype)
{
	ثابت حرف *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#جے_تعریف_شدہ CONFIG_LOG
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
	ARG_بن_ورتیا(otype);
	ret = عدم;
#جے_انت /* CONFIG_LOG */
	أرجع ret;
}

بنية perm_ctx {
	صحيح ماپا_id;
	صحيح بچہ_id;
	بنية k_thread *ماپا;
};

#جے_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ڈھیر_لبھو(k_thread_ڈھیر_t *ڈھیر)
{
	بنية k_object *obj = k_object_لبھو(ڈھیر);

	__ASSERT(obj != عدم, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ڈھیر_ELEMENT,
		 "bad stack object");

	أرجع obj->انکڑے.ڈھیر_انکڑے->priv;
}
#جے_انت /* CONFIG_GEN_PRIV_STACKS */

#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS

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
#جے_تعریف_شدہ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#تعریف_کرو DYN_OBJ_انکڑے_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#وإلا
#تعریف_کرو DYN_OBJ_انکڑے_ALIGN_K_THREAD	(حجم(فراغ *))
#جے_انت /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#جے_تعریف_شدہ CONFIG_DYNAMIC_THREAD_ڈھیر_ناپ
#إذا defined(CONFIG_MPU_ڈھیر_GUARD) || defined(CONFIG_PMP_ڈھیر_GUARD)
#تعریف_کرو DYN_OBJ_انکڑے_ALIGN_K_THREAD_ڈھیر \
	Z_THREAD_ڈھیر_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ڈھیر_ناپ)
#وإلا
#تعریف_کرو DYN_OBJ_انکڑے_ALIGN_K_THREAD_ڈھیر \
	Z_THREAD_ڈھیر_OBJ_ALIGN(CONFIG_PRIVILEGED_ڈھیر_ناپ)
#جے_انت /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#وإلا
#تعریف_کرو DYN_OBJ_انکڑے_ALIGN_K_THREAD_ڈھیر \
	Z_THREAD_ڈھیر_OBJ_ALIGN(ARCH_ڈھیر_PTR_ALIGN)
#جے_انت /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#تعریف_کرو DYN_OBJ_انکڑے_ALIGN		\
	MAX(DYN_OBJ_انکڑے_ALIGN_K_THREAD, (حجم(فراغ *)))

بنية dyn_obj {
	بنية k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	فراغ *انکڑے;
};

خارجي بنية k_object *z_object_gperf_لبھو(ثابت فراغ *obj);
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

ساكن size_t obj_ناپ_get(تعداد k_objects otype)
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
#جے_تعریف_شدہ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#وإلا
		ret = __alignof(بنية dyn_obj);
#جے_انت /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		اكسر;
	افتراضي:
		ret = __alignof(بنية dyn_obj);
		اكسر;
	}

	أرجع ret;
}

ساكن بنية dyn_obj *dyn_object_لبھو_locked(ثابت فراغ *obj)
{
	بنية dyn_obj *گرہ;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, گرہ, dobj_list) {
		إذا (گرہ->kobj.name == obj) {
			أرجع گرہ;
		}
	}

	أرجع عدم;
}

ساكن بنية dyn_obj *dyn_object_لبھو(ثابت فراغ *obj)
{
	بنية dyn_obj *گرہ;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_جندرا(&lists_جندرا);
	گرہ = dyn_object_لبھو_locked(obj);
	k_spin_unlock(&lists_جندرا, کلید);

	أرجع گرہ;
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
	صحيح ادھار;
	bool ret = false;
	k_spinlock_کلید_t کلید;

	ادھار = 0;
	کلید = k_spin_جندرا(&lists_جندرا);
	لكل (i = 0; i < CONFIG_MAX_THREAD_بائٹاں; i++) {
		idx = لبھو_lsb_سیٹ(_thread_idx_map[i]);

		إذا (idx != 0) {
			بنية dyn_obj *obj, *اگلا;

			*tidx = ادھار + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(ثنائی_ہندسہ(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (فراغ *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_محفوظ(&obj_list, obj, اگلا, dobj_list) {
				clear_perms_cb(&obj->kobj, (فراغ *)*tidx);
			}

			ret = true;
			اكسر;
		}

		ادھار += 8;
	}

	k_spin_unlock(&lists_جندرا, کلید);
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
	بنية dyn_obj *obj, *اگلا;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (فراغ *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_کلید_t کلید = k_spin_جندرا(&lists_جندرا);

	SYS_DLIST_FOR_EACH_CONTAINER_محفوظ(&obj_list, obj, اگلا, dobj_list) {
		clear_perms_cb(&obj->kobj, (فراغ *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	صحيح ادھار = tidx / عدد_بٹ_2(_thread_idx_map[0]);
	صحيح وتھ = tidx % عدد_بٹ_2(_thread_idx_map[0]);

	_thread_idx_map[ادھار] |= ثنائی_ہندسہ(وتھ);

	k_spin_unlock(&lists_جندرا, کلید);
}

ساكن بنية k_object *dynamic_object_create(تعداد k_objects otype, size_t align,
					      size_t ناپ)
{
	بنية dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, حجم(بنية dyn_obj));
	إذا (dyn == عدم) {
		أرجع عدم;
	}

	إذا (otype == K_OBJ_THREAD_ڈھیر_ELEMENT) {
		size_t adjusted_ناپ;

		إذا (ناپ == 0) {
			k_free(dyn);
			أرجع عدم;
		}

		adjusted_ناپ = ڈھیر_ELEMENT_انکڑے_ناپ(ناپ);
		إذا (adjusted_ناپ < ناپ) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			أرجع عدم;
		}

		dyn->انکڑے = z_thread_aligned_alloc(DYN_OBJ_انکڑے_ALIGN_K_THREAD_ڈھیر,
						     adjusted_ناپ);
		إذا (dyn->انکڑے == عدم) {
			k_free(dyn);
			أرجع عدم;
		}

#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS_FORCE_ڈھیر_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		إذا (sys_cache_is_ptr_uncached(dyn->انکڑے)) {
			dyn->انکڑے = sys_cache_cached_ptr_get(dyn->انکڑے);
		}
#جے_انت /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#جے_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
		بنية z_ڈھیر_انکڑے *ڈھیر_انکڑے = (بنية z_ڈھیر_انکڑے *)
			((uint8_t *)dyn->انکڑے + adjusted_ناپ - حجم(*ڈھیر_انکڑے));
#إذا defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ڈھیر_انکڑے->priv = (فراغ *)گیڑا_اتے(((uint8_t *)dyn->انکڑے + ناپ),
			  Z_THREAD_ڈھیر_OBJ_ALIGN(ناپ));
#وإلا
		ڈھیر_انکڑے->priv = (uint8_t *)dyn->انکڑے;
#جے_انت /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ڈھیر_انکڑے->ناپ = adjusted_ناپ;
		dyn->kobj.انکڑے.ڈھیر_انکڑے = ڈھیر_انکڑے;
		dyn->kobj.name = dyn->انکڑے;
#وإلا
		dyn->kobj.name = dyn->انکڑے;
		dyn->kobj.انکڑے.ڈھیر_ناپ = adjusted_ناپ;
#جے_انت /* CONFIG_GEN_PRIV_STACKS */
	} وإلا {
		size_t کل_ناپ = obj_ناپ_get(otype) + ناپ;

		إذا (کل_ناپ < ناپ) {
			k_free(dyn);
			أرجع عدم;
		}

		dyn->انکڑے = z_thread_aligned_alloc(align, کل_ناپ);
		إذا (dyn->انکڑے == عدم) {
			k_free(dyn);
			أرجع عدم;
		}
		dyn->kobj.name = dyn->انکڑے;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(فراغ)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_بائٹاں);

	k_spinlock_کلید_t کلید = k_spin_جندرا(&lists_جندرا);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_جندرا, کلید);

	أرجع &dyn->kobj;
}

بنية k_object *k_object_create_dynamic_aligned(size_t align, size_t ناپ)
{
	بنية k_object *obj = dynamic_object_create(K_OBJ_ANY, align, ناپ);

	إذا (obj == عدم) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	أرجع obj;
}

ساكن فراغ *z_object_alloc(تعداد k_objects otype, size_t ناپ)
{
	بنية k_object *zo;
	uintptr_t tidx = 0;

	إذا ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_آخری)) {
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

	zo = dynamic_object_create(otype, obj_align_get(otype), ناپ);
	إذا (zo == عدم) {
		إذا (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		أرجع عدم;
	}

	إذا (otype == K_OBJ_THREAD) {
		zo->انکڑے.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_سیٹ(zo, _current);

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

فراغ *z_impl_k_object_alloc_ناپ(تعداد k_objects otype, size_t ناپ)
{
	أرجع z_object_alloc(otype, ناپ);
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
	k_spinlock_کلید_t کلید = k_spin_جندرا(&lists_جندرا);

	dyn = dyn_object_لبھو_locked(obj);

	إذا (dyn != عدم) {
		sys_dlist_remove(&dyn->dobj_list);

		إذا (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.انکڑے.thread_id;
		}
	}
	k_spin_unlock(&lists_جندرا, کلید);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	إذا (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	إذا (dyn != عدم) {
#جے_تعریف_شدہ CONFIG_OBJ_CORE
		إذا (dyn->kobj.type != K_OBJ_THREAD_ڈھیر_ELEMENT) {
			k_obj_core_evict_range(obj, obj_ناپ_get(dyn->kobj.type));
		}
#جے_انت /* CONFIG_OBJ_CORE */
#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS_FORCE_ڈھیر_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		إذا (dyn->kobj.type == K_OBJ_THREAD_ڈھیر_ELEMENT) {
			uint8_t *ڈھیر;

			إذا (sys_cache_is_ptr_cached(dyn->انکڑے)) {
				ڈھیر = sys_cache_uncached_ptr_get(dyn->انکڑے);
			} وإلا {
				ڈھیر = dyn->انکڑے;
			}

			k_free(ڈھیر);
		} وإلا
#جے_انت /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->انکڑے);
		}
		k_free(dyn);
	}
}

بنية k_object *k_object_لبھو(ثابت فراغ *obj)
{
	بنية k_object *ret;

	ret = z_object_gperf_لبھو(obj);

	إذا (ret == عدم) {
		بنية dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_لبھو(obj);
		إذا (dyn != عدم) {
			ret = &dyn->kobj;
		}
	}

	أرجع ret;
}

فراغ k_object_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
	بنية dyn_obj *obj, *اگلا;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_کلید_t کلید = k_spin_جندرا(&lists_جندرا);

	SYS_DLIST_FOR_EACH_CONTAINER_محفوظ(&obj_list, obj, اگلا, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_جندرا, کلید);
}
#جے_انت /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_انکڑے.متن.dummies)
__weak بنية k_object *z_object_gperf_لبھو(ثابت فراغ *obj)
{
	ARG_بن_ورتیا(obj);
	أرجع عدم;
}
Z_GENERIC_SECTION(.kobject_انکڑے.متن.dummies)
__weak فراغ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
	ARG_بن_ورتیا(func);
	ARG_بن_ورتیا(context);
}
#وإلا
Z_GENERIC_SECTION(.kobject_انکڑے.متن.dummies)
__weak بنية k_object *k_object_لبھو(ثابت فراغ *obj)
{
	أرجع عدم;
}
Z_GENERIC_SECTION(.kobject_انکڑے.متن.dummies)
__weak فراغ k_object_wordlist_foreach(_wordlist_cb_func_t func, فراغ *context)
{
}
#جے_انت

ساكن بنانشانی صحيح thread_اشاریہ_get(بنية k_thread *thread)
{
	بنية k_object *ko;

	ko = k_object_لبھو(thread);

	إذا (ko == عدم) {
		أرجع -1;
	}

	أرجع ko->انکڑے.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ساكن فراغ unref_check(بنية k_object *ko, uintptr_t اشاریہ, bool locked)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&obj_جندرا);

	sys_bitfield_clear_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);

#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
	إذا ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		اذهب باہرلا;
	}

	فراغ *vko = ko;

	بنية dyn_obj *dyn = CONTAINER_دا(vko, بنية dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, بنية dyn_obj), "unaligned z_object");

	لكل (صحيح i = 0; i < CONFIG_MAX_THREAD_بائٹاں; i++) {
		إذا (ko->perms[i] != 0U) {
			اذهب باہرلا;
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
	حالة K_OBJ_ڈھیر:
		(فراغ)z_ڈھیر_cleanup((بنية k_ڈھیر_2 *)ko->name, locked);
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
	k_free(dyn->انکڑے);
	k_free(dyn);
باہرلا:
#جے_انت /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_جندرا, کلید);
}

ساكن فراغ wordlist_cb(بنية k_object *ko, فراغ *ctx_ptr)
{
	بنية perm_ctx *ctx = (بنية perm_ctx *)ctx_ptr;

	إذا (sys_bitfield_test_ثنائی_ہندسہ((mem_addr_t)&ko->perms, ctx->ماپا_id) &&
				  ((بنية k_thread *)ko->name != ctx->ماپا)) {
		sys_bitfield_سیٹ_ثنائی_ہندسہ((mem_addr_t)&ko->perms, ctx->بچہ_id);
	}
}

فراغ k_thread_perms_inherit(بنية k_thread *ماپا, بنية k_thread *بچہ)
{
	بنية perm_ctx ctx = {
		thread_اشاریہ_get(ماپا),
		thread_اشاریہ_get(بچہ),
		ماپا
	};

	إذا ((ctx.ماپا_id != -1) && (ctx.بچہ_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

فراغ k_thread_perms_سیٹ(بنية k_object *ko, بنية k_thread *thread)
{
	صحيح اشاریہ = thread_اشاریہ_get(thread);

	إذا (اشاریہ != -1) {
		sys_bitfield_سیٹ_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);
	}
}

فراغ k_thread_perms_clear(بنية k_object *ko, بنية k_thread *thread)
{
	صحيح اشاریہ = thread_اشاریہ_get(thread);

	إذا (اشاریہ != -1) {
		sys_bitfield_clear_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
		k_spinlock_کلید_t کلید = k_spin_جندرا(&lists_جندرا);

		unref_check(ko, اشاریہ, false);
		k_spin_unlock(&lists_جندرا, کلید);
#وإلا
		unref_check(ko, اشاریہ, false);
#جے_انت /* CONFIG_DYNAMIC_OBJECTS */
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
	uintptr_t اشاریہ = thread_اشاریہ_get(thread);

	إذا ((صحيح)اشاریہ != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (فراغ *)اشاریہ);
	}
}

ساكن صحيح thread_perms_test(بنية k_object *ko)
{
	صحيح اشاریہ;

	إذا ((ko->flags & K_OBJ_FLAG_عوامی) != 0U) {
		أرجع 1;
	}

	اشاریہ = thread_اشاریہ_get(_current);
	إذا (اشاریہ != -1) {
		أرجع sys_bitfield_test_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);
	}
	أرجع 0;
}

ساكن فراغ dump_permission_error(بنية k_object *ko)
{
	صحيح اشاریہ = thread_اشاریہ_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, اشاریہ,
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
	بنية k_object *ko = k_object_لبھو(object);

	إذا (ko != عدم) {
		k_thread_perms_سیٹ(ko, thread);
	}
}

فراغ k_object_access_revoke(ثابت فراغ *object, بنية k_thread *thread)
{
	بنية k_object *ko = k_object_لبھو(object);

	إذا (ko != عدم) {
		k_thread_perms_clear(ko, thread);
	}
}

فراغ k_object_access_revoke_others(ثابت فراغ *object)
{
	بنية k_object *ko = k_object_لبھو(object);

	إذا (ko != عدم) {
		(فراغ)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_سیٹ(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_عوامی;
	}
}

فراغ z_impl_k_object_release(ثابت فراغ *object)
{
	k_object_access_revoke(object, _current);
}

فراغ k_object_access_all_grant(ثابت فراغ *object)
{
	بنية k_object *ko = k_object_لبھو(object);

	إذا (ko != عدم) {
		ko->flags |= K_OBJ_FLAG_عوامی;
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
		    (ko->type < K_OBJ_DRIVER_پہلا) ||
		    (ko->type > K_OBJ_DRIVER_آخری)) {
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
	أرجع k_object_validate(k_object_لبھو(object), K_OBJ_ANY, _OBJ_INIT_ANY);
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

	ko = k_object_لبھو(obj);
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
	بنية k_object *ko = k_object_لبھو(obj);

	إذا (ko != عدم) {
		(فراغ)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_سیٹ(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

فراغ k_object_uninit(ثابت فراغ *obj)
{
	بنية k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_لبھو(obj);
	إذا (ko == عدم) {
		أرجع;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
فراغ *k_usermode_alloc_from_copy(ثابت فراغ *src, size_t ناپ)
{
	فراغ *dst = عدم;

	/* Does the caller in user mode have access to read this memory? */
	إذا (K_SYSCALL_حافظہ_READ(src, ناپ)) {
		اذهب باہرلا_err;
	}

	dst = z_thread_malloc(ناپ);
	إذا (dst == عدم) {
		LOG_ERR("out of thread resource pool memory (%zu)", ناپ);
		اذهب باہرلا_err;
	}

	(فراغ)حافظہ_نقل(dst, src, ناپ);
باہرلا_err:
	أرجع dst;
}

ساكن صحيح user_copy(فراغ *dst, ثابت فراغ *src, size_t ناپ, bool منزل_user)
{
	صحيح ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	إذا (منزل_user ? K_SYSCALL_حافظہ_WRITE(dst, ناپ) :
			K_SYSCALL_حافظہ_READ(src, ناپ)) {
		اذهب باہرلا_err;
	}

	(فراغ)حافظہ_نقل(dst, src, ناپ);
	ret = 0;
باہرلا_err:
	أرجع ret;
}

صحيح k_usermode_from_copy(فراغ *dst, ثابت فراغ *src, size_t ناپ)
{
	أرجع user_copy(dst, src, ناپ, false);
}

صحيح k_usermode_منزل_copy(فراغ *dst, ثابت فراغ *src, size_t ناپ)
{
	أرجع user_copy(dst, src, ناپ, true);
}

حرف *k_usermode_string_alloc_copy(ثابت حرف *src, size_t maxlen)
{
	size_t actual_len;
	صحيح err;
	حرف *ret = عدم;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	إذا (err != 0) {
		اذهب باہرلا;
	}
	إذا (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		اذهب باہرلا;
	}
	إذا (ناپ_جوڑو_حد_توں_ودھ(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		اذهب باہرلا;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	إذا (ret != عدم) {
		ret[actual_len - 1U] = '\0';
	}
باہرلا:
	أرجع ret;
}

صحيح k_usermode_string_copy(حرف *dst, ثابت حرف *src, size_t maxlen)
{
	size_t actual_len;
	صحيح ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	إذا (err != 0) {
		ret = EFAULT;
		اذهب باہرلا;
	}
	إذا (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		اذهب باہرلا;
	}
	إذا (ناپ_جوڑو_حد_توں_ودھ(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		اذهب باہرلا;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
باہرلا:
	أرجع ret;
}

/*
 * Application memory region initialization
 */

خارجي حرف __app_shmem_regions_start[];
خارجي حرف __app_shmem_regions_end[];

ساكن فراغ app_shmem_bss_zero(فراغ)
{
	بنية z_app_region *region, *سرا;


	سرا = (بنية z_app_region *)&__app_shmem_regions_end[0];
	region = (بنية z_app_region *)&__app_shmem_regions_start[0];

	لكل ( ; region < سرا; region++) {
		(فراغ)memset(region->bss_شروع, 0, region->bss_ناپ);
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
	ARG_بن_ورتیا(arg2);
	ARG_بن_ورتیا(arg3);
	ARG_بن_ورتیا(arg4);
	ARG_بن_ورتیا(arg5);
	ARG_بن_ورتیا(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	کوڈ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساكن uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, فراغ *ssf)
{
	ARG_بن_ورتیا(arg1);
	ARG_بن_ورتیا(arg2);
	ARG_بن_ورتیا(arg3);
	ARG_بن_ورتیا(arg4);
	ARG_بن_ورتیا(arg5);
	ARG_بن_ورتیا(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	کوڈ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#ضمن <zephyr/syscall_dispatch.c>
