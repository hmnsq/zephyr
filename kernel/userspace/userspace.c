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

#اگر_تعریف_شدہ Z_LIBC_تقسیم_EXISTS
K_APPMEM_تقسیم_DEFINE(z_libc_تقسیم);
#اگر_ختم /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#اگر_تعریف_شدہ CONFIG_MBEDTLS
K_APPMEM_تقسیم_DEFINE(k_mbedtls_تقسیم);
#اگر_ختم /* CONFIG_MBEDTLS */

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
#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
ساکن ساخت k_spinlock lists_تالا;       /* kobj dlist */

#اگر_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#اگر defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#تعریف_کرو ڈھیر_ELEMENT_ڈیٹا_سائز(سائز) \
	(حجم(ساخت z_ڈھیر_ڈیٹا) + CONFIG_PRIVILEGED_ڈھیر_سائز + \
	Z_THREAD_ڈھیر_OBJ_ALIGN(سائز) + K_THREAD_ڈھیر_LEN(سائز))
#ورنہ
#تعریف_کرو ڈھیر_ELEMENT_ڈیٹا_سائز(سائز) (حجم(ساخت z_ڈھیر_ڈیٹا) + \
	K_THREAD_ڈھیر_LEN(سائز))
#اگر_ختم /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#ورنہ
#تعریف_کرو ڈھیر_ELEMENT_ڈیٹا_سائز(سائز) K_THREAD_ڈھیر_LEN(سائز)
#اگر_ختم /* CONFIG_GEN_PRIV_STACKS */

#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS */
ساکن ساخت k_spinlock obj_تالا;         /* kobj struct data */

#تعریف_کرو MAX_THREAD_بٹ (CONFIG_MAX_THREAD_بائٹیں * بٹ_فی_BYTE)

#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
خارجی uint8_t _thread_idx_map[CONFIG_MAX_THREAD_بائٹیں];

خارجی صحیح z_msgq_cleanup(ساخت k_msgq *q, bool locked);
خارجی صحیح z_ڈھیر_cleanup(ساخت k_ڈھیر_2 *ڈھیر, bool locked);
خارجی صحیح z_timer_cleanup(ساخت k_timer *timer, bool locked);
#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS */

ساکن خالی unref_check(ساخت k_object *ko, uintptr_t اشاریہ, bool locked);

ثابت حرف *otype_مقصد_str(شمار k_objects otype)
{
	ثابت حرف *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#اگر_تعریف_شدہ CONFIG_LOG
	انتخاب (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	حالت K_OBJ_ANY:
		ret = "generic";
		توڑو;
	حالت K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		توڑو;
#شامل <zephyr/otype-to-str.h>
	طےشدہ:
		ret = "?";
		توڑو;
	}
#ورنہ
	ARG_غیر_مستعمل(otype);
	ret = تہی;
#اگر_ختم /* CONFIG_LOG */
	واپس ret;
}

ساخت perm_ctx {
	صحیح والد_id;
	صحیح بچہ_id;
	ساخت k_thread *والد;
};

#اگر_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ڈھیر_ڈھونڈو(k_thread_ڈھیر_t *ڈھیر)
{
	ساخت k_object *obj = k_object_ڈھونڈو(ڈھیر);

	__ASSERT(obj != تہی, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ڈھیر_ELEMENT,
		 "bad stack object");

	واپس obj->ڈیٹا.ڈھیر_ڈیٹا->priv;
}
#اگر_ختم /* CONFIG_GEN_PRIV_STACKS */

#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS

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
#اگر_تعریف_شدہ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#تعریف_کرو DYN_OBJ_ڈیٹا_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#ورنہ
#تعریف_کرو DYN_OBJ_ڈیٹا_ALIGN_K_THREAD	(حجم(خالی *))
#اگر_ختم /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#اگر_تعریف_شدہ CONFIG_DYNAMIC_THREAD_ڈھیر_سائز
#اگر defined(CONFIG_MPU_ڈھیر_GUARD) || defined(CONFIG_PMP_ڈھیر_GUARD)
#تعریف_کرو DYN_OBJ_ڈیٹا_ALIGN_K_THREAD_ڈھیر \
	Z_THREAD_ڈھیر_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ڈھیر_سائز)
#ورنہ
#تعریف_کرو DYN_OBJ_ڈیٹا_ALIGN_K_THREAD_ڈھیر \
	Z_THREAD_ڈھیر_OBJ_ALIGN(CONFIG_PRIVILEGED_ڈھیر_سائز)
#اگر_ختم /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#ورنہ
#تعریف_کرو DYN_OBJ_ڈیٹا_ALIGN_K_THREAD_ڈھیر \
	Z_THREAD_ڈھیر_OBJ_ALIGN(ARCH_ڈھیر_PTR_ALIGN)
#اگر_ختم /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#تعریف_کرو DYN_OBJ_ڈیٹا_ALIGN		\
	MAX(DYN_OBJ_ڈیٹا_ALIGN_K_THREAD, (حجم(خالی *)))

ساخت dyn_obj {
	ساخت k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	خالی *ڈیٹا;
};

خارجی ساخت k_object *z_object_gperf_ڈھونڈو(ثابت خالی *obj);
خارجی خالی z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     خالی *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
ساکن sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

ساکن size_t obj_سائز_get(شمار k_objects otype)
{
	size_t ret;

	انتخاب (otype) {
#شامل <zephyr/otype-to-size.h>
	طےشدہ:
		ret = حجم(ثابت ساخت device);
		توڑو;
	}

	واپس ret;
}

ساکن size_t obj_align_get(شمار k_objects otype)
{
	size_t ret;

	انتخاب (otype) {
	حالت K_OBJ_THREAD:
#اگر_تعریف_شدہ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#ورنہ
		ret = __alignof(ساخت dyn_obj);
#اگر_ختم /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		توڑو;
	طےشدہ:
		ret = __alignof(ساخت dyn_obj);
		توڑو;
	}

	واپس ret;
}

ساکن ساخت dyn_obj *dyn_object_ڈھونڈو_locked(ثابت خالی *obj)
{
	ساخت dyn_obj *گرہ;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, گرہ, dobj_list) {
		اگر (گرہ->kobj.name == obj) {
			واپس گرہ;
		}
	}

	واپس تہی;
}

ساکن ساخت dyn_obj *dyn_object_ڈھونڈو(ثابت خالی *obj)
{
	ساخت dyn_obj *گرہ;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_تالا(&lists_تالا);
	گرہ = dyn_object_ڈھونڈو_locked(obj);
	k_spin_unlock(&lists_تالا, کلید);

	واپس گرہ;
}

ساکن خالی clear_perms_cb(ساخت k_object *ko, خالی *ctx_ptr)
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
ساکن bool thread_idx_alloc(uintptr_t *tidx)
{
	صحیح i;
	صحیح idx;
	صحیح بنیاد;
	bool ret = false;
	k_spinlock_کلید_t کلید;

	بنیاد = 0;
	کلید = k_spin_تالا(&lists_تالا);
	برائے (i = 0; i < CONFIG_MAX_THREAD_بائٹیں; i++) {
		idx = ڈھونڈو_lsb_سیٹ(_thread_idx_map[i]);

		اگر (idx != 0) {
			ساخت dyn_obj *obj, *اگلا;

			*tidx = بنیاد + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(ثنائی_ہندسہ(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (خالی *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_محفوظ(&obj_list, obj, اگلا, dobj_list) {
				clear_perms_cb(&obj->kobj, (خالی *)*tidx);
			}

			ret = true;
			توڑو;
		}

		بنیاد += 8;
	}

	k_spin_unlock(&lists_تالا, کلید);
	واپس ret;
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
ساکن خالی thread_idx_free(uintptr_t tidx)
{
	ساخت dyn_obj *obj, *اگلا;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (خالی *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_کلید_t کلید = k_spin_تالا(&lists_تالا);

	SYS_DLIST_FOR_EACH_CONTAINER_محفوظ(&obj_list, obj, اگلا, dobj_list) {
		clear_perms_cb(&obj->kobj, (خالی *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	صحیح بنیاد = tidx / عدد_بٹ_2(_thread_idx_map[0]);
	صحیح انحراف = tidx % عدد_بٹ_2(_thread_idx_map[0]);

	_thread_idx_map[بنیاد] |= ثنائی_ہندسہ(انحراف);

	k_spin_unlock(&lists_تالا, کلید);
}

ساکن ساخت k_object *dynamic_object_create(شمار k_objects otype, size_t align,
					      size_t سائز)
{
	ساخت dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, حجم(ساخت dyn_obj));
	اگر (dyn == تہی) {
		واپس تہی;
	}

	اگر (otype == K_OBJ_THREAD_ڈھیر_ELEMENT) {
		size_t adjusted_سائز;

		اگر (سائز == 0) {
			k_free(dyn);
			واپس تہی;
		}

		adjusted_سائز = ڈھیر_ELEMENT_ڈیٹا_سائز(سائز);
		اگر (adjusted_سائز < سائز) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			واپس تہی;
		}

		dyn->ڈیٹا = z_thread_aligned_alloc(DYN_OBJ_ڈیٹا_ALIGN_K_THREAD_ڈھیر,
						     adjusted_سائز);
		اگر (dyn->ڈیٹا == تہی) {
			k_free(dyn);
			واپس تہی;
		}

#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS_FORCE_ڈھیر_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		اگر (sys_cache_is_ptr_uncached(dyn->ڈیٹا)) {
			dyn->ڈیٹا = sys_cache_cached_ptr_get(dyn->ڈیٹا);
		}
#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#اگر_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
		ساخت z_ڈھیر_ڈیٹا *ڈھیر_ڈیٹا = (ساخت z_ڈھیر_ڈیٹا *)
			((uint8_t *)dyn->ڈیٹا + adjusted_سائز - حجم(*ڈھیر_ڈیٹا));
#اگر defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ڈھیر_ڈیٹا->priv = (خالی *)باری_اوپر(((uint8_t *)dyn->ڈیٹا + سائز),
			  Z_THREAD_ڈھیر_OBJ_ALIGN(سائز));
#ورنہ
		ڈھیر_ڈیٹا->priv = (uint8_t *)dyn->ڈیٹا;
#اگر_ختم /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ڈھیر_ڈیٹا->سائز = adjusted_سائز;
		dyn->kobj.ڈیٹا.ڈھیر_ڈیٹا = ڈھیر_ڈیٹا;
		dyn->kobj.name = dyn->ڈیٹا;
#ورنہ
		dyn->kobj.name = dyn->ڈیٹا;
		dyn->kobj.ڈیٹا.ڈھیر_سائز = adjusted_سائز;
#اگر_ختم /* CONFIG_GEN_PRIV_STACKS */
	} ورنہ {
		size_t کل_سائز = obj_سائز_get(otype) + سائز;

		اگر (کل_سائز < سائز) {
			k_free(dyn);
			واپس تہی;
		}

		dyn->ڈیٹا = z_thread_aligned_alloc(align, کل_سائز);
		اگر (dyn->ڈیٹا == تہی) {
			k_free(dyn);
			واپس تہی;
		}
		dyn->kobj.name = dyn->ڈیٹا;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(خالی)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_بائٹیں);

	k_spinlock_کلید_t کلید = k_spin_تالا(&lists_تالا);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_تالا, کلید);

	واپس &dyn->kobj;
}

ساخت k_object *k_object_create_dynamic_aligned(size_t align, size_t سائز)
{
	ساخت k_object *obj = dynamic_object_create(K_OBJ_ANY, align, سائز);

	اگر (obj == تہی) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	واپس obj;
}

ساکن خالی *z_object_alloc(شمار k_objects otype, size_t سائز)
{
	ساخت k_object *zo;
	uintptr_t tidx = 0;

	اگر ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_آخری)) {
		LOG_ERR("bad object type %d requested", otype);
		واپس تہی;
	}

	انتخاب (otype) {
	حالت K_OBJ_THREAD:
		اگر (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			واپس تہی;
		}
		توڑو;
	/* The following are currently not allowed at all */
	حالت K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	حالت K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_مقصد_str(otype));
		واپس تہی;
	طےشدہ:
		/* Remainder within bounds are permitted */
		توڑو;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), سائز);
	اگر (zo == تہی) {
		اگر (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		واپس تہی;
	}

	اگر (otype == K_OBJ_THREAD) {
		zo->ڈیٹا.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_سیٹ(zo, _current);

	/* Activates reference counting logic for automatic disposal when
	 * all permissions have been revoked
	 */
	zo->flags |= K_OBJ_FLAG_ALLOC;

	واپس zo->name;
}

خالی *z_impl_k_object_alloc(شمار k_objects otype)
{
	واپس z_object_alloc(otype, 0);
}

خالی *z_impl_k_object_alloc_سائز(شمار k_objects otype, size_t سائز)
{
	واپس z_object_alloc(otype, سائز);
}

خالی k_object_free(خالی *obj)
{
	ساخت dyn_obj *dyn = تہی;
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
	k_spinlock_کلید_t کلید = k_spin_تالا(&lists_تالا);

	dyn = dyn_object_ڈھونڈو_locked(obj);

	اگر (dyn != تہی) {
		sys_dlist_remove(&dyn->dobj_list);

		اگر (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.ڈیٹا.thread_id;
		}
	}
	k_spin_unlock(&lists_تالا, کلید);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	اگر (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	اگر (dyn != تہی) {
#اگر_تعریف_شدہ CONFIG_OBJ_CORE
		اگر (dyn->kobj.type != K_OBJ_THREAD_ڈھیر_ELEMENT) {
			k_obj_core_evict_range(obj, obj_سائز_get(dyn->kobj.type));
		}
#اگر_ختم /* CONFIG_OBJ_CORE */
#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS_FORCE_ڈھیر_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		اگر (dyn->kobj.type == K_OBJ_THREAD_ڈھیر_ELEMENT) {
			uint8_t *ڈھیر;

			اگر (sys_cache_is_ptr_cached(dyn->ڈیٹا)) {
				ڈھیر = sys_cache_uncached_ptr_get(dyn->ڈیٹا);
			} ورنہ {
				ڈھیر = dyn->ڈیٹا;
			}

			k_free(ڈھیر);
		} ورنہ
#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->ڈیٹا);
		}
		k_free(dyn);
	}
}

ساخت k_object *k_object_ڈھونڈو(ثابت خالی *obj)
{
	ساخت k_object *ret;

	ret = z_object_gperf_ڈھونڈو(obj);

	اگر (ret == تہی) {
		ساخت dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_ڈھونڈو(obj);
		اگر (dyn != تہی) {
			ret = &dyn->kobj;
		}
	}

	واپس ret;
}

خالی k_object_wordlist_foreach(_wordlist_cb_func_t func, خالی *context)
{
	ساخت dyn_obj *obj, *اگلا;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_کلید_t کلید = k_spin_تالا(&lists_تالا);

	SYS_DLIST_FOR_EACH_CONTAINER_محفوظ(&obj_list, obj, اگلا, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_تالا, کلید);
}
#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_ڈیٹا.متن.dummies)
__weak ساخت k_object *z_object_gperf_ڈھونڈو(ثابت خالی *obj)
{
	ARG_غیر_مستعمل(obj);
	واپس تہی;
}
Z_GENERIC_SECTION(.kobject_ڈیٹا.متن.dummies)
__weak خالی z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, خالی *context)
{
	ARG_غیر_مستعمل(func);
	ARG_غیر_مستعمل(context);
}
#ورنہ
Z_GENERIC_SECTION(.kobject_ڈیٹا.متن.dummies)
__weak ساخت k_object *k_object_ڈھونڈو(ثابت خالی *obj)
{
	واپس تہی;
}
Z_GENERIC_SECTION(.kobject_ڈیٹا.متن.dummies)
__weak خالی k_object_wordlist_foreach(_wordlist_cb_func_t func, خالی *context)
{
}
#اگر_ختم

ساکن غیرعلامتی صحیح thread_اشاریہ_get(ساخت k_thread *thread)
{
	ساخت k_object *ko;

	ko = k_object_ڈھونڈو(thread);

	اگر (ko == تہی) {
		واپس -1;
	}

	واپس ko->ڈیٹا.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ساکن خالی unref_check(ساخت k_object *ko, uintptr_t اشاریہ, bool locked)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&obj_تالا);

	sys_bitfield_clear_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);

#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
	اگر ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		جاؤ برآمد;
	}

	خالی *vko = ko;

	ساخت dyn_obj *dyn = CONTAINER_کا(vko, ساخت dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, ساخت dyn_obj), "unaligned z_object");

	برائے (صحیح i = 0; i < CONFIG_MAX_THREAD_بائٹیں; i++) {
		اگر (ko->perms[i] != 0U) {
			جاؤ برآمد;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	انتخاب (ko->type) {
	حالت K_OBJ_MSGQ:
		(خالی)z_msgq_cleanup((ساخت k_msgq *)ko->name, locked);
		توڑو;
	حالت K_OBJ_ڈھیر:
		(خالی)z_ڈھیر_cleanup((ساخت k_ڈھیر_2 *)ko->name, locked);
		توڑو;
	حالت K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		اگر ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(خالی)z_timer_cleanup((ساخت k_timer *)ko->name, locked);
		}
		توڑو;
	طےشدہ:
		/* Nothing to do */
		توڑو;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->ڈیٹا);
	k_free(dyn);
برآمد:
#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_تالا, کلید);
}

ساکن خالی wordlist_cb(ساخت k_object *ko, خالی *ctx_ptr)
{
	ساخت perm_ctx *ctx = (ساخت perm_ctx *)ctx_ptr;

	اگر (sys_bitfield_test_ثنائی_ہندسہ((mem_addr_t)&ko->perms, ctx->والد_id) &&
				  ((ساخت k_thread *)ko->name != ctx->والد)) {
		sys_bitfield_سیٹ_ثنائی_ہندسہ((mem_addr_t)&ko->perms, ctx->بچہ_id);
	}
}

خالی k_thread_perms_inherit(ساخت k_thread *والد, ساخت k_thread *بچہ)
{
	ساخت perm_ctx ctx = {
		thread_اشاریہ_get(والد),
		thread_اشاریہ_get(بچہ),
		والد
	};

	اگر ((ctx.والد_id != -1) && (ctx.بچہ_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

خالی k_thread_perms_سیٹ(ساخت k_object *ko, ساخت k_thread *thread)
{
	صحیح اشاریہ = thread_اشاریہ_get(thread);

	اگر (اشاریہ != -1) {
		sys_bitfield_سیٹ_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);
	}
}

خالی k_thread_perms_clear(ساخت k_object *ko, ساخت k_thread *thread)
{
	صحیح اشاریہ = thread_اشاریہ_get(thread);

	اگر (اشاریہ != -1) {
		sys_bitfield_clear_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
		k_spinlock_کلید_t کلید = k_spin_تالا(&lists_تالا);

		unref_check(ko, اشاریہ, false);
		k_spin_unlock(&lists_تالا, کلید);
#ورنہ
		unref_check(ko, اشاریہ, false);
#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS */
	}
}

ساکن خالی clear_perms_cb_locked(ساخت k_object *ko, خالی *ctx_ptr)
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
خالی k_thread_perms_all_clear(ساخت k_thread *thread)
{
	uintptr_t اشاریہ = thread_اشاریہ_get(thread);

	اگر ((صحیح)اشاریہ != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (خالی *)اشاریہ);
	}
}

ساکن صحیح thread_perms_test(ساخت k_object *ko)
{
	صحیح اشاریہ;

	اگر ((ko->flags & K_OBJ_FLAG_عوامی) != 0U) {
		واپس 1;
	}

	اشاریہ = thread_اشاریہ_get(_current);
	اگر (اشاریہ != -1) {
		واپس sys_bitfield_test_ثنائی_ہندسہ((mem_addr_t)&ko->perms, اشاریہ);
	}
	واپس 0;
}

ساکن خالی dump_permission_error(ساخت k_object *ko)
{
	صحیح اشاریہ = thread_اشاریہ_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, اشاریہ,
		otype_مقصد_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, حجم(ko->perms), "permission bitmap");
}

خالی k_object_dump_error(صحیح retval, ثابت خالی *obj, ساخت k_object *ko,
			شمار k_objects otype)
{
	انتخاب (retval) {
	حالت -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_مقصد_str(otype));
		اگر (ko == تہی) {
			LOG_ERR("address is not a known kernel object");
		} ورنہ {
			LOG_ERR("address is actually a %s",
				otype_مقصد_str(ko->type));
		}
		توڑو;
	حالت -EPERM:
		dump_permission_error(ko);
		توڑو;
	حالت -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		توڑو;
	حالت -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_مقصد_str(otype));
		توڑو;
	طےشدہ:
		/* Not handled error */
		توڑو;
	}
}

خالی z_impl_k_object_access_grant(ثابت خالی *object, ساخت k_thread *thread)
{
	ساخت k_object *ko = k_object_ڈھونڈو(object);

	اگر (ko != تہی) {
		k_thread_perms_سیٹ(ko, thread);
	}
}

خالی k_object_access_revoke(ثابت خالی *object, ساخت k_thread *thread)
{
	ساخت k_object *ko = k_object_ڈھونڈو(object);

	اگر (ko != تہی) {
		k_thread_perms_clear(ko, thread);
	}
}

خالی k_object_access_revoke_others(ثابت خالی *object)
{
	ساخت k_object *ko = k_object_ڈھونڈو(object);

	اگر (ko != تہی) {
		(خالی)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_سیٹ(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_عوامی;
	}
}

خالی z_impl_k_object_release(ثابت خالی *object)
{
	k_object_access_revoke(object, _current);
}

خالی k_object_access_all_grant(ثابت خالی *object)
{
	ساخت k_object *ko = k_object_ڈھونڈو(object);

	اگر (ko != تہی) {
		ko->flags |= K_OBJ_FLAG_عوامی;
	}
}

صحیح k_object_validate(ساخت k_object *ko, شمار k_objects otype,
		       شمار _obj_init_check init)
{
	اگر (unlikely(ko == تہی)) {
		واپس -EBADF;
	}

	اگر (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		اگر ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_پہلا) ||
		    (ko->type > K_OBJ_DRIVER_آخری)) {
			واپس -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	اگر (unlikely(thread_perms_test(ko) == 0)) {
		واپس -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	اگر (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		اگر (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			واپس -EINVAL;
		}
	} ورنہ اگر (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		اگر (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			واپس -EADDRINUSE;
		}
	} ورنہ {
		/* _OBJ_INIT_ANY */
	}

	واپس 0;
}

صحیح z_impl_k_object_access_check(ثابت خالی *object)
{
	واپس k_object_validate(k_object_ڈھونڈو(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

خالی k_object_init(ثابت خالی *obj)
{
	ساخت k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_ڈھونڈو(obj);
	اگر (ko == تہی) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		واپس;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

خالی k_object_recycle(ثابت خالی *obj)
{
	ساخت k_object *ko = k_object_ڈھونڈو(obj);

	اگر (ko != تہی) {
		(خالی)memset(ko->perms, 0, حجم(ko->perms));
		k_thread_perms_سیٹ(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

خالی k_object_uninit(ثابت خالی *obj)
{
	ساخت k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_ڈھونڈو(obj);
	اگر (ko == تہی) {
		واپس;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
خالی *k_usermode_alloc_from_copy(ثابت خالی *src, size_t سائز)
{
	خالی *dst = تہی;

	/* Does the caller in user mode have access to read this memory? */
	اگر (K_SYSCALL_حافظہ_READ(src, سائز)) {
		جاؤ برآمد_err;
	}

	dst = z_thread_malloc(سائز);
	اگر (dst == تہی) {
		LOG_ERR("out of thread resource pool memory (%zu)", سائز);
		جاؤ برآمد_err;
	}

	(خالی)حافظہ_نقل(dst, src, سائز);
برآمد_err:
	واپس dst;
}

ساکن صحیح user_copy(خالی *dst, ثابت خالی *src, size_t سائز, bool مقصد_user)
{
	صحیح ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	اگر (مقصد_user ? K_SYSCALL_حافظہ_WRITE(dst, سائز) :
			K_SYSCALL_حافظہ_READ(src, سائز)) {
		جاؤ برآمد_err;
	}

	(خالی)حافظہ_نقل(dst, src, سائز);
	ret = 0;
برآمد_err:
	واپس ret;
}

صحیح k_usermode_from_copy(خالی *dst, ثابت خالی *src, size_t سائز)
{
	واپس user_copy(dst, src, سائز, false);
}

صحیح k_usermode_مقصد_copy(خالی *dst, ثابت خالی *src, size_t سائز)
{
	واپس user_copy(dst, src, سائز, true);
}

حرف *k_usermode_string_alloc_copy(ثابت حرف *src, size_t maxlen)
{
	size_t actual_len;
	صحیح err;
	حرف *ret = تہی;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	اگر (err != 0) {
		جاؤ برآمد;
	}
	اگر (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		جاؤ برآمد;
	}
	اگر (سائز_جوڑو_حد_سے_تجاوز(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		جاؤ برآمد;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	اگر (ret != تہی) {
		ret[actual_len - 1U] = '\0';
	}
برآمد:
	واپس ret;
}

صحیح k_usermode_string_copy(حرف *dst, ثابت حرف *src, size_t maxlen)
{
	size_t actual_len;
	صحیح ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	اگر (err != 0) {
		ret = EFAULT;
		جاؤ برآمد;
	}
	اگر (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		جاؤ برآمد;
	}
	اگر (سائز_جوڑو_حد_سے_تجاوز(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		جاؤ برآمد;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
برآمد:
	واپس ret;
}

/*
 * Application memory region initialization
 */

خارجی حرف __app_shmem_regions_start[];
خارجی حرف __app_shmem_regions_end[];

ساکن خالی app_shmem_bss_zero(خالی)
{
	ساخت z_app_region *region, *انتہا;


	انتہا = (ساخت z_app_region *)&__app_shmem_regions_end[0];
	region = (ساخت z_app_region *)&__app_shmem_regions_start[0];

	برائے ( ; region < انتہا; region++) {
		(خالی)memset(region->bss_آغاز, 0, region->bss_سائز);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

ساکن uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     خالی *ssf)
{
	ARG_غیر_مستعمل(arg2);
	ARG_غیر_مستعمل(arg3);
	ARG_غیر_مستعمل(arg4);
	ARG_غیر_مستعمل(arg5);
	ARG_غیر_مستعمل(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	رمز_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساکن uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, خالی *ssf)
{
	ARG_غیر_مستعمل(arg1);
	ARG_غیر_مستعمل(arg2);
	ARG_غیر_مستعمل(arg3);
	ARG_غیر_مستعمل(arg4);
	ARG_غیر_مستعمل(arg5);
	ARG_غیر_مستعمل(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	رمز_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#شامل <zephyr/syscall_dispatch.c>
