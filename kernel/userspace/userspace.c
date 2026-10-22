/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/rb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/sys_io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscall.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/device.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/app_memory/app_memdomain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/libc-hooks.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/mutex.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/util.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/linker-defs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/cache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_LIBC_ਹਿੱਸੇ_ਕਰੋ_EXISTS
K_APPMEM_ਹਿੱਸੇ_ਕਰੋ_DEFINE(z_libc_ਹਿੱਸੇ_ਕਰੋ);
#ਜੇ_ਅੰਤ /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MBEDTLS
K_APPMEM_ਹਿੱਸੇ_ਕਰੋ_DEFINE(k_mbedtls_ਹਿੱਸੇ_ਕਰੋ);
#ਜੇ_ਅੰਤ /* CONFIG_MBEDTLS */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* The originally synchronization strategy made heavy use of recursive
 * irq_locking, which ports poorly to spinlocks which are
 * non-recursive.  Rather than try to redesign as part of
 * spinlockification, this uses multiple locks to preserve the
 * original semantics exactly.  The locks are named for the data they
 * protect where possible, or just for the code that uses them where
 * not.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS
ਸਥਿਰ ਬਣਤਰ k_spinlock lists_ਤਾਲਾ;       /* kobj dlist */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#ਜੇ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ELEMENT_ਅੰਕੜੇ_ਮਾਪ(ਮਾਪ) \
	(ਆਕਾਰ(ਬਣਤਰ z_ਢੇਰ_ਅੰਕੜੇ) + CONFIG_PRIVILEGED_ਢੇਰ_ਮਾਪ + \
	Z_THREAD_ਢੇਰ_OBJ_ALIGN(ਮਾਪ) + K_THREAD_ਢੇਰ_LEN(ਮਾਪ))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ELEMENT_ਅੰਕੜੇ_ਮਾਪ(ਮਾਪ) (ਆਕਾਰ(ਬਣਤਰ z_ਢੇਰ_ਅੰਕੜੇ) + \
	K_THREAD_ਢੇਰ_LEN(ਮਾਪ))
#ਜੇ_ਅੰਤ /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ELEMENT_ਅੰਕੜੇ_ਮਾਪ(ਮਾਪ) K_THREAD_ਢੇਰ_LEN(ਮਾਪ)
#ਜੇ_ਅੰਤ /* CONFIG_GEN_PRIV_STACKS */

#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS */
ਸਥਿਰ ਬਣਤਰ k_spinlock obj_ਤਾਲਾ;         /* kobj struct data */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_THREAD_ਬਿੱਟ (CONFIG_MAX_THREAD_ਬਾਈਟਾਂ * ਬਿੱਟ_ਪ੍ਰਤੀ_BYTE)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS
ਬਾਹਰੀ uint8_t _thread_idx_map[CONFIG_MAX_THREAD_ਬਾਈਟਾਂ];

ਬਾਹਰੀ ਪੂਰਨ z_msgq_cleanup(ਬਣਤਰ k_msgq *q, bool locked);
ਬਾਹਰੀ ਪੂਰਨ z_ਢੇਰ_cleanup(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ, bool locked);
ਬਾਹਰੀ ਪੂਰਨ z_timer_cleanup(ਬਣਤਰ k_timer *timer, bool locked);
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS */

ਸਥਿਰ ਖਾਲੀ unref_check(ਬਣਤਰ k_object *ko, uintptr_t ਸੂਚਕਾਂਕ, bool locked);

ਅਚਲ ਅੱਖਰ *otype_ਮੰਜ਼ਿਲ_str(ਗਿਣਤੀ k_objects otype)
{
	ਅਚਲ ਅੱਖਰ *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LOG
	ਚੋਣ (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	ਹਾਲਤ K_OBJ_ANY:
		ret = "generic";
		ਰੋਕੋ;
	ਹਾਲਤ K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		ਰੋਕੋ;
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/otype-to-str.h>
	ਮੂਲ:
		ret = "?";
		ਰੋਕੋ;
	}
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(otype);
	ret = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_LOG */
	ਵਾਪਸ ret;
}

ਬਣਤਰ perm_ctx {
	ਪੂਰਨ ਮਾਪਾ_id;
	ਪੂਰਨ ਬੱਚਾ_id;
	ਬਣਤਰ k_thread *ਮਾਪਾ;
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ਢੇਰ_ਲੱਭੋ(k_thread_ਢੇਰ_t *ਢੇਰ)
{
	ਬਣਤਰ k_object *obj = k_object_ਲੱਭੋ(ਢੇਰ);

	__ASSERT(obj != ਨਿਰਾ, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ਢੇਰ_ELEMENT,
		 "bad stack object");

	ਵਾਪਸ obj->ਅੰਕੜੇ.ਢੇਰ_ਅੰਕੜੇ->priv;
}
#ਜੇ_ਅੰਤ /* CONFIG_GEN_PRIV_STACKS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DYN_OBJ_ਅੰਕੜੇ_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DYN_OBJ_ਅੰਕੜੇ_ALIGN_K_THREAD	(ਆਕਾਰ(ਖਾਲੀ *))
#ਜੇ_ਅੰਤ /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_THREAD_ਢੇਰ_ਮਾਪ
#ਜੇ defined(CONFIG_MPU_ਢੇਰ_GUARD) || defined(CONFIG_PMP_ਢੇਰ_GUARD)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DYN_OBJ_ਅੰਕੜੇ_ALIGN_K_THREAD_ਢੇਰ \
	Z_THREAD_ਢੇਰ_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ਢੇਰ_ਮਾਪ)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DYN_OBJ_ਅੰਕੜੇ_ALIGN_K_THREAD_ਢੇਰ \
	Z_THREAD_ਢੇਰ_OBJ_ALIGN(CONFIG_PRIVILEGED_ਢੇਰ_ਮਾਪ)
#ਜੇ_ਅੰਤ /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DYN_OBJ_ਅੰਕੜੇ_ALIGN_K_THREAD_ਢੇਰ \
	Z_THREAD_ਢੇਰ_OBJ_ALIGN(ARCH_ਢੇਰ_PTR_ALIGN)
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DYN_OBJ_ਅੰਕੜੇ_ALIGN		\
	MAX(DYN_OBJ_ਅੰਕੜੇ_ALIGN_K_THREAD, (ਆਕਾਰ(ਖਾਲੀ *)))

ਬਣਤਰ dyn_obj {
	ਬਣਤਰ k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	ਖਾਲੀ *ਅੰਕੜੇ;
};

ਬਾਹਰੀ ਬਣਤਰ k_object *z_object_gperf_ਲੱਭੋ(ਅਚਲ ਖਾਲੀ *obj);
ਬਾਹਰੀ ਖਾਲੀ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     ਖਾਲੀ *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
ਸਥਿਰ sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

ਸਥਿਰ size_t obj_ਮਾਪ_get(ਗਿਣਤੀ k_objects otype)
{
	size_t ret;

	ਚੋਣ (otype) {
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/otype-to-size.h>
	ਮੂਲ:
		ret = ਆਕਾਰ(ਅਚਲ ਬਣਤਰ device);
		ਰੋਕੋ;
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ size_t obj_align_get(ਗਿਣਤੀ k_objects otype)
{
	size_t ret;

	ਚੋਣ (otype) {
	ਹਾਲਤ K_OBJ_THREAD:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#ਨਹੀਂਤਾਂ
		ret = __alignof(ਬਣਤਰ dyn_obj);
#ਜੇ_ਅੰਤ /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		ਰੋਕੋ;
	ਮੂਲ:
		ret = __alignof(ਬਣਤਰ dyn_obj);
		ਰੋਕੋ;
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਬਣਤਰ dyn_obj *dyn_object_ਲੱਭੋ_locked(ਅਚਲ ਖਾਲੀ *obj)
{
	ਬਣਤਰ dyn_obj *ਗੰਢ;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, ਗੰਢ, dobj_list) {
		ਜੇ (ਗੰਢ->kobj.name == obj) {
			ਵਾਪਸ ਗੰਢ;
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਬਣਤਰ dyn_obj *dyn_object_ਲੱਭੋ(ਅਚਲ ਖਾਲੀ *obj)
{
	ਬਣਤਰ dyn_obj *ਗੰਢ;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&lists_ਤਾਲਾ);
	ਗੰਢ = dyn_object_ਲੱਭੋ_locked(obj);
	k_spin_unlock(&lists_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ਗੰਢ;
}

ਸਥਿਰ ਖਾਲੀ clear_perms_cb(ਬਣਤਰ k_object *ko, ਖਾਲੀ *ctx_ptr)
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
ਸਥਿਰ bool thread_idx_alloc(uintptr_t *tidx)
{
	ਪੂਰਨ i;
	ਪੂਰਨ idx;
	ਪੂਰਨ ਅਧਾਰ;
	bool ret = false;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	ਅਧਾਰ = 0;
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&lists_ਤਾਲਾ);
	ਲਈ (i = 0; i < CONFIG_MAX_THREAD_ਬਾਈਟਾਂ; i++) {
		idx = ਲੱਭੋ_lsb_ਸੈੱਟ(_thread_idx_map[i]);

		ਜੇ (idx != 0) {
			ਬਣਤਰ dyn_obj *obj, *ਅਗਲਾ;

			*tidx = ਅਧਾਰ + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(ਦੋਆਧਾਰੀ_ਅੰਕ(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (ਖਾਲੀ *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_ਸੁਰੱਖਿਅਤ(&obj_list, obj, ਅਗਲਾ, dobj_list) {
				clear_perms_cb(&obj->kobj, (ਖਾਲੀ *)*tidx);
			}

			ret = true;
			ਰੋਕੋ;
		}

		ਅਧਾਰ += 8;
	}

	k_spin_unlock(&lists_ਤਾਲਾ, ਕੁੰਜੀ);
	ਵਾਪਸ ret;
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
ਸਥਿਰ ਖਾਲੀ thread_idx_free(uintptr_t tidx)
{
	ਬਣਤਰ dyn_obj *obj, *ਅਗਲਾ;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (ਖਾਲੀ *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&lists_ਤਾਲਾ);

	SYS_DLIST_FOR_EACH_CONTAINER_ਸੁਰੱਖਿਅਤ(&obj_list, obj, ਅਗਲਾ, dobj_list) {
		clear_perms_cb(&obj->kobj, (ਖਾਲੀ *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	ਪੂਰਨ ਅਧਾਰ = tidx / ਅੰਕ_ਬਿੱਟ_2(_thread_idx_map[0]);
	ਪੂਰਨ ਵਿੱਥ = tidx % ਅੰਕ_ਬਿੱਟ_2(_thread_idx_map[0]);

	_thread_idx_map[ਅਧਾਰ] |= ਦੋਆਧਾਰੀ_ਅੰਕ(ਵਿੱਥ);

	k_spin_unlock(&lists_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਸਥਿਰ ਬਣਤਰ k_object *dynamic_object_create(ਗਿਣਤੀ k_objects otype, size_t align,
					      size_t ਮਾਪ)
{
	ਬਣਤਰ dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, ਆਕਾਰ(ਬਣਤਰ dyn_obj));
	ਜੇ (dyn == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (otype == K_OBJ_THREAD_ਢੇਰ_ELEMENT) {
		size_t adjusted_ਮਾਪ;

		ਜੇ (ਮਾਪ == 0) {
			k_free(dyn);
			ਵਾਪਸ ਨਿਰਾ;
		}

		adjusted_ਮਾਪ = ਢੇਰ_ELEMENT_ਅੰਕੜੇ_ਮਾਪ(ਮਾਪ);
		ਜੇ (adjusted_ਮਾਪ < ਮਾਪ) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			ਵਾਪਸ ਨਿਰਾ;
		}

		dyn->ਅੰਕੜੇ = z_thread_aligned_alloc(DYN_OBJ_ਅੰਕੜੇ_ALIGN_K_THREAD_ਢੇਰ,
						     adjusted_ਮਾਪ);
		ਜੇ (dyn->ਅੰਕੜੇ == ਨਿਰਾ) {
			k_free(dyn);
			ਵਾਪਸ ਨਿਰਾ;
		}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS_FORCE_ਢੇਰ_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		ਜੇ (sys_cache_is_ptr_uncached(dyn->ਅੰਕੜੇ)) {
			dyn->ਅੰਕੜੇ = sys_cache_cached_ptr_get(dyn->ਅੰਕੜੇ);
		}
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_GEN_PRIV_STACKS
		ਬਣਤਰ z_ਢੇਰ_ਅੰਕੜੇ *ਢੇਰ_ਅੰਕੜੇ = (ਬਣਤਰ z_ਢੇਰ_ਅੰਕੜੇ *)
			((uint8_t *)dyn->ਅੰਕੜੇ + adjusted_ਮਾਪ - ਆਕਾਰ(*ਢੇਰ_ਅੰਕੜੇ));
#ਜੇ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ਢੇਰ_ਅੰਕੜੇ->priv = (ਖਾਲੀ *)ਗੇੜਾ_ਉੱਪਰ(((uint8_t *)dyn->ਅੰਕੜੇ + ਮਾਪ),
			  Z_THREAD_ਢੇਰ_OBJ_ALIGN(ਮਾਪ));
#ਨਹੀਂਤਾਂ
		ਢੇਰ_ਅੰਕੜੇ->priv = (uint8_t *)dyn->ਅੰਕੜੇ;
#ਜੇ_ਅੰਤ /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ਢੇਰ_ਅੰਕੜੇ->ਮਾਪ = adjusted_ਮਾਪ;
		dyn->kobj.ਅੰਕੜੇ.ਢੇਰ_ਅੰਕੜੇ = ਢੇਰ_ਅੰਕੜੇ;
		dyn->kobj.name = dyn->ਅੰਕੜੇ;
#ਨਹੀਂਤਾਂ
		dyn->kobj.name = dyn->ਅੰਕੜੇ;
		dyn->kobj.ਅੰਕੜੇ.ਢੇਰ_ਮਾਪ = adjusted_ਮਾਪ;
#ਜੇ_ਅੰਤ /* CONFIG_GEN_PRIV_STACKS */
	} ਨਹੀਂਤਾਂ {
		size_t ਕੁੱਲ_ਮਾਪ = obj_ਮਾਪ_get(otype) + ਮਾਪ;

		ਜੇ (ਕੁੱਲ_ਮਾਪ < ਮਾਪ) {
			k_free(dyn);
			ਵਾਪਸ ਨਿਰਾ;
		}

		dyn->ਅੰਕੜੇ = z_thread_aligned_alloc(align, ਕੁੱਲ_ਮਾਪ);
		ਜੇ (dyn->ਅੰਕੜੇ == ਨਿਰਾ) {
			k_free(dyn);
			ਵਾਪਸ ਨਿਰਾ;
		}
		dyn->kobj.name = dyn->ਅੰਕੜੇ;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(ਖਾਲੀ)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_ਬਾਈਟਾਂ);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&lists_ਤਾਲਾ);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ &dyn->kobj;
}

ਬਣਤਰ k_object *k_object_create_dynamic_aligned(size_t align, size_t ਮਾਪ)
{
	ਬਣਤਰ k_object *obj = dynamic_object_create(K_OBJ_ANY, align, ਮਾਪ);

	ਜੇ (obj == ਨਿਰਾ) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	ਵਾਪਸ obj;
}

ਸਥਿਰ ਖਾਲੀ *z_object_alloc(ਗਿਣਤੀ k_objects otype, size_t ਮਾਪ)
{
	ਬਣਤਰ k_object *zo;
	uintptr_t tidx = 0;

	ਜੇ ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_ਆਖਰੀ)) {
		LOG_ERR("bad object type %d requested", otype);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਚੋਣ (otype) {
	ਹਾਲਤ K_OBJ_THREAD:
		ਜੇ (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			ਵਾਪਸ ਨਿਰਾ;
		}
		ਰੋਕੋ;
	/* The following are currently not allowed at all */
	ਹਾਲਤ K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	ਹਾਲਤ K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_ਮੰਜ਼ਿਲ_str(otype));
		ਵਾਪਸ ਨਿਰਾ;
	ਮੂਲ:
		/* Remainder within bounds are permitted */
		ਰੋਕੋ;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), ਮਾਪ);
	ਜੇ (zo == ਨਿਰਾ) {
		ਜੇ (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (otype == K_OBJ_THREAD) {
		zo->ਅੰਕੜੇ.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_ਸੈੱਟ(zo, _current);

	/* Activates reference counting logic for automatic disposal when
	 * all permissions have been revoked
	 */
	zo->flags |= K_OBJ_FLAG_ALLOC;

	ਵਾਪਸ zo->name;
}

ਖਾਲੀ *z_impl_k_object_alloc(ਗਿਣਤੀ k_objects otype)
{
	ਵਾਪਸ z_object_alloc(otype, 0);
}

ਖਾਲੀ *z_impl_k_object_alloc_ਮਾਪ(ਗਿਣਤੀ k_objects otype, size_t ਮਾਪ)
{
	ਵਾਪਸ z_object_alloc(otype, ਮਾਪ);
}

ਖਾਲੀ k_object_free(ਖਾਲੀ *obj)
{
	ਬਣਤਰ dyn_obj *dyn = ਨਿਰਾ;
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
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&lists_ਤਾਲਾ);

	dyn = dyn_object_ਲੱਭੋ_locked(obj);

	ਜੇ (dyn != ਨਿਰਾ) {
		sys_dlist_remove(&dyn->dobj_list);

		ਜੇ (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.ਅੰਕੜੇ.thread_id;
		}
	}
	k_spin_unlock(&lists_ਤਾਲਾ, ਕੁੰਜੀ);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	ਜੇ (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	ਜੇ (dyn != ਨਿਰਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE
		ਜੇ (dyn->kobj.type != K_OBJ_THREAD_ਢੇਰ_ELEMENT) {
			k_obj_core_evict_range(obj, obj_ਮਾਪ_get(dyn->kobj.type));
		}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS_FORCE_ਢੇਰ_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		ਜੇ (dyn->kobj.type == K_OBJ_THREAD_ਢੇਰ_ELEMENT) {
			uint8_t *ਢੇਰ;

			ਜੇ (sys_cache_is_ptr_cached(dyn->ਅੰਕੜੇ)) {
				ਢੇਰ = sys_cache_uncached_ptr_get(dyn->ਅੰਕੜੇ);
			} ਨਹੀਂਤਾਂ {
				ਢੇਰ = dyn->ਅੰਕੜੇ;
			}

			k_free(ਢੇਰ);
		} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->ਅੰਕੜੇ);
		}
		k_free(dyn);
	}
}

ਬਣਤਰ k_object *k_object_ਲੱਭੋ(ਅਚਲ ਖਾਲੀ *obj)
{
	ਬਣਤਰ k_object *ret;

	ret = z_object_gperf_ਲੱਭੋ(obj);

	ਜੇ (ret == ਨਿਰਾ) {
		ਬਣਤਰ dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_ਲੱਭੋ(obj);
		ਜੇ (dyn != ਨਿਰਾ) {
			ret = &dyn->kobj;
		}
	}

	ਵਾਪਸ ret;
}

ਖਾਲੀ k_object_wordlist_foreach(_wordlist_cb_func_t func, ਖਾਲੀ *context)
{
	ਬਣਤਰ dyn_obj *obj, *ਅਗਲਾ;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&lists_ਤਾਲਾ);

	SYS_DLIST_FOR_EACH_CONTAINER_ਸੁਰੱਖਿਅਤ(&obj_list, obj, ਅਗਲਾ, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_ਤਾਲਾ, ਕੁੰਜੀ);
}
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_ਅੰਕੜੇ.ਪਾਠ.dummies)
__weak ਬਣਤਰ k_object *z_object_gperf_ਲੱਭੋ(ਅਚਲ ਖਾਲੀ *obj)
{
	ARG_ਅਣਵਰਤਿਆ(obj);
	ਵਾਪਸ ਨਿਰਾ;
}
Z_GENERIC_SECTION(.kobject_ਅੰਕੜੇ.ਪਾਠ.dummies)
__weak ਖਾਲੀ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, ਖਾਲੀ *context)
{
	ARG_ਅਣਵਰਤਿਆ(func);
	ARG_ਅਣਵਰਤਿਆ(context);
}
#ਨਹੀਂਤਾਂ
Z_GENERIC_SECTION(.kobject_ਅੰਕੜੇ.ਪਾਠ.dummies)
__weak ਬਣਤਰ k_object *k_object_ਲੱਭੋ(ਅਚਲ ਖਾਲੀ *obj)
{
	ਵਾਪਸ ਨਿਰਾ;
}
Z_GENERIC_SECTION(.kobject_ਅੰਕੜੇ.ਪਾਠ.dummies)
__weak ਖਾਲੀ k_object_wordlist_foreach(_wordlist_cb_func_t func, ਖਾਲੀ *context)
{
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ thread_ਸੂਚਕਾਂਕ_get(ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ k_object *ko;

	ko = k_object_ਲੱਭੋ(thread);

	ਜੇ (ko == ਨਿਰਾ) {
		ਵਾਪਸ -1;
	}

	ਵਾਪਸ ko->ਅੰਕੜੇ.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ਸਥਿਰ ਖਾਲੀ unref_check(ਬਣਤਰ k_object *ko, uintptr_t ਸੂਚਕਾਂਕ, bool locked)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&obj_ਤਾਲਾ);

	sys_bitfield_clear_ਦੋਆਧਾਰੀ_ਅੰਕ((mem_addr_t)&ko->perms, ਸੂਚਕਾਂਕ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS
	ਜੇ ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਖਾਲੀ *vko = ko;

	ਬਣਤਰ dyn_obj *dyn = CONTAINER_ਦਾ(vko, ਬਣਤਰ dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, ਬਣਤਰ dyn_obj), "unaligned z_object");

	ਲਈ (ਪੂਰਨ i = 0; i < CONFIG_MAX_THREAD_ਬਾਈਟਾਂ; i++) {
		ਜੇ (ko->perms[i] != 0U) {
			ਜਾਓ ਬਾਹਰਲਾ;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	ਚੋਣ (ko->type) {
	ਹਾਲਤ K_OBJ_MSGQ:
		(ਖਾਲੀ)z_msgq_cleanup((ਬਣਤਰ k_msgq *)ko->name, locked);
		ਰੋਕੋ;
	ਹਾਲਤ K_OBJ_ਢੇਰ:
		(ਖਾਲੀ)z_ਢੇਰ_cleanup((ਬਣਤਰ k_ਢੇਰ_2 *)ko->name, locked);
		ਰੋਕੋ;
	ਹਾਲਤ K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		ਜੇ ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(ਖਾਲੀ)z_timer_cleanup((ਬਣਤਰ k_timer *)ko->name, locked);
		}
		ਰੋਕੋ;
	ਮੂਲ:
		/* Nothing to do */
		ਰੋਕੋ;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->ਅੰਕੜੇ);
	k_free(dyn);
ਬਾਹਰਲਾ:
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਸਥਿਰ ਖਾਲੀ wordlist_cb(ਬਣਤਰ k_object *ko, ਖਾਲੀ *ctx_ptr)
{
	ਬਣਤਰ perm_ctx *ctx = (ਬਣਤਰ perm_ctx *)ctx_ptr;

	ਜੇ (sys_bitfield_test_ਦੋਆਧਾਰੀ_ਅੰਕ((mem_addr_t)&ko->perms, ctx->ਮਾਪਾ_id) &&
				  ((ਬਣਤਰ k_thread *)ko->name != ctx->ਮਾਪਾ)) {
		sys_bitfield_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ((mem_addr_t)&ko->perms, ctx->ਬੱਚਾ_id);
	}
}

ਖਾਲੀ k_thread_perms_inherit(ਬਣਤਰ k_thread *ਮਾਪਾ, ਬਣਤਰ k_thread *ਬੱਚਾ)
{
	ਬਣਤਰ perm_ctx ctx = {
		thread_ਸੂਚਕਾਂਕ_get(ਮਾਪਾ),
		thread_ਸੂਚਕਾਂਕ_get(ਬੱਚਾ),
		ਮਾਪਾ
	};

	ਜੇ ((ctx.ਮਾਪਾ_id != -1) && (ctx.ਬੱਚਾ_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

ਖਾਲੀ k_thread_perms_ਸੈੱਟ(ਬਣਤਰ k_object *ko, ਬਣਤਰ k_thread *thread)
{
	ਪੂਰਨ ਸੂਚਕਾਂਕ = thread_ਸੂਚਕਾਂਕ_get(thread);

	ਜੇ (ਸੂਚਕਾਂਕ != -1) {
		sys_bitfield_ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ((mem_addr_t)&ko->perms, ਸੂਚਕਾਂਕ);
	}
}

ਖਾਲੀ k_thread_perms_clear(ਬਣਤਰ k_object *ko, ਬਣਤਰ k_thread *thread)
{
	ਪੂਰਨ ਸੂਚਕਾਂਕ = thread_ਸੂਚਕਾਂਕ_get(thread);

	ਜੇ (ਸੂਚਕਾਂਕ != -1) {
		sys_bitfield_clear_ਦੋਆਧਾਰੀ_ਅੰਕ((mem_addr_t)&ko->perms, ਸੂਚਕਾਂਕ);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS
		k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&lists_ਤਾਲਾ);

		unref_check(ko, ਸੂਚਕਾਂਕ, false);
		k_spin_unlock(&lists_ਤਾਲਾ, ਕੁੰਜੀ);
#ਨਹੀਂਤਾਂ
		unref_check(ko, ਸੂਚਕਾਂਕ, false);
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS */
	}
}

ਸਥਿਰ ਖਾਲੀ clear_perms_cb_locked(ਬਣਤਰ k_object *ko, ਖਾਲੀ *ctx_ptr)
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
ਖਾਲੀ k_thread_perms_all_clear(ਬਣਤਰ k_thread *thread)
{
	uintptr_t ਸੂਚਕਾਂਕ = thread_ਸੂਚਕਾਂਕ_get(thread);

	ਜੇ ((ਪੂਰਨ)ਸੂਚਕਾਂਕ != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (ਖਾਲੀ *)ਸੂਚਕਾਂਕ);
	}
}

ਸਥਿਰ ਪੂਰਨ thread_perms_test(ਬਣਤਰ k_object *ko)
{
	ਪੂਰਨ ਸੂਚਕਾਂਕ;

	ਜੇ ((ko->flags & K_OBJ_FLAG_ਜਨਤਕ) != 0U) {
		ਵਾਪਸ 1;
	}

	ਸੂਚਕਾਂਕ = thread_ਸੂਚਕਾਂਕ_get(_current);
	ਜੇ (ਸੂਚਕਾਂਕ != -1) {
		ਵਾਪਸ sys_bitfield_test_ਦੋਆਧਾਰੀ_ਅੰਕ((mem_addr_t)&ko->perms, ਸੂਚਕਾਂਕ);
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ dump_permission_error(ਬਣਤਰ k_object *ko)
{
	ਪੂਰਨ ਸੂਚਕਾਂਕ = thread_ਸੂਚਕਾਂਕ_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, ਸੂਚਕਾਂਕ,
		otype_ਮੰਜ਼ਿਲ_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, ਆਕਾਰ(ko->perms), "permission bitmap");
}

ਖਾਲੀ k_object_dump_error(ਪੂਰਨ retval, ਅਚਲ ਖਾਲੀ *obj, ਬਣਤਰ k_object *ko,
			ਗਿਣਤੀ k_objects otype)
{
	ਚੋਣ (retval) {
	ਹਾਲਤ -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_ਮੰਜ਼ਿਲ_str(otype));
		ਜੇ (ko == ਨਿਰਾ) {
			LOG_ERR("address is not a known kernel object");
		} ਨਹੀਂਤਾਂ {
			LOG_ERR("address is actually a %s",
				otype_ਮੰਜ਼ਿਲ_str(ko->type));
		}
		ਰੋਕੋ;
	ਹਾਲਤ -EPERM:
		dump_permission_error(ko);
		ਰੋਕੋ;
	ਹਾਲਤ -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		ਰੋਕੋ;
	ਹਾਲਤ -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_ਮੰਜ਼ਿਲ_str(otype));
		ਰੋਕੋ;
	ਮੂਲ:
		/* Not handled error */
		ਰੋਕੋ;
	}
}

ਖਾਲੀ z_impl_k_object_access_grant(ਅਚਲ ਖਾਲੀ *object, ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ k_object *ko = k_object_ਲੱਭੋ(object);

	ਜੇ (ko != ਨਿਰਾ) {
		k_thread_perms_ਸੈੱਟ(ko, thread);
	}
}

ਖਾਲੀ k_object_access_revoke(ਅਚਲ ਖਾਲੀ *object, ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ k_object *ko = k_object_ਲੱਭੋ(object);

	ਜੇ (ko != ਨਿਰਾ) {
		k_thread_perms_clear(ko, thread);
	}
}

ਖਾਲੀ k_object_access_revoke_others(ਅਚਲ ਖਾਲੀ *object)
{
	ਬਣਤਰ k_object *ko = k_object_ਲੱਭੋ(object);

	ਜੇ (ko != ਨਿਰਾ) {
		(ਖਾਲੀ)memset(ko->perms, 0, ਆਕਾਰ(ko->perms));
		k_thread_perms_ਸੈੱਟ(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_ਜਨਤਕ;
	}
}

ਖਾਲੀ z_impl_k_object_release(ਅਚਲ ਖਾਲੀ *object)
{
	k_object_access_revoke(object, _current);
}

ਖਾਲੀ k_object_access_all_grant(ਅਚਲ ਖਾਲੀ *object)
{
	ਬਣਤਰ k_object *ko = k_object_ਲੱਭੋ(object);

	ਜੇ (ko != ਨਿਰਾ) {
		ko->flags |= K_OBJ_FLAG_ਜਨਤਕ;
	}
}

ਪੂਰਨ k_object_validate(ਬਣਤਰ k_object *ko, ਗਿਣਤੀ k_objects otype,
		       ਗਿਣਤੀ _obj_init_check init)
{
	ਜੇ (unlikely(ko == ਨਿਰਾ)) {
		ਵਾਪਸ -EBADF;
	}

	ਜੇ (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		ਜੇ ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_ਪਹਿਲਾ) ||
		    (ko->type > K_OBJ_DRIVER_ਆਖਰੀ)) {
			ਵਾਪਸ -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	ਜੇ (unlikely(thread_perms_test(ko) == 0)) {
		ਵਾਪਸ -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	ਜੇ (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		ਜੇ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			ਵਾਪਸ -EINVAL;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		ਜੇ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			ਵਾਪਸ -EADDRINUSE;
		}
	} ਨਹੀਂਤਾਂ {
		/* _OBJ_INIT_ANY */
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_impl_k_object_access_check(ਅਚਲ ਖਾਲੀ *object)
{
	ਵਾਪਸ k_object_validate(k_object_ਲੱਭੋ(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

ਖਾਲੀ k_object_init(ਅਚਲ ਖਾਲੀ *obj)
{
	ਬਣਤਰ k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_ਲੱਭੋ(obj);
	ਜੇ (ko == ਨਿਰਾ) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		ਵਾਪਸ;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

ਖਾਲੀ k_object_recycle(ਅਚਲ ਖਾਲੀ *obj)
{
	ਬਣਤਰ k_object *ko = k_object_ਲੱਭੋ(obj);

	ਜੇ (ko != ਨਿਰਾ) {
		(ਖਾਲੀ)memset(ko->perms, 0, ਆਕਾਰ(ko->perms));
		k_thread_perms_ਸੈੱਟ(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

ਖਾਲੀ k_object_uninit(ਅਚਲ ਖਾਲੀ *obj)
{
	ਬਣਤਰ k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_ਲੱਭੋ(obj);
	ਜੇ (ko == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
ਖਾਲੀ *k_usermode_alloc_from_copy(ਅਚਲ ਖਾਲੀ *src, size_t ਮਾਪ)
{
	ਖਾਲੀ *dst = ਨਿਰਾ;

	/* Does the caller in user mode have access to read this memory? */
	ਜੇ (K_SYSCALL_ਸਮ੍ਰਿਤੀ_READ(src, ਮਾਪ)) {
		ਜਾਓ ਬਾਹਰਲਾ_err;
	}

	dst = z_thread_malloc(ਮਾਪ);
	ਜੇ (dst == ਨਿਰਾ) {
		LOG_ERR("out of thread resource pool memory (%zu)", ਮਾਪ);
		ਜਾਓ ਬਾਹਰਲਾ_err;
	}

	(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(dst, src, ਮਾਪ);
ਬਾਹਰਲਾ_err:
	ਵਾਪਸ dst;
}

ਸਥਿਰ ਪੂਰਨ user_copy(ਖਾਲੀ *dst, ਅਚਲ ਖਾਲੀ *src, size_t ਮਾਪ, bool ਮੰਜ਼ਿਲ_user)
{
	ਪੂਰਨ ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	ਜੇ (ਮੰਜ਼ਿਲ_user ? K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(dst, ਮਾਪ) :
			K_SYSCALL_ਸਮ੍ਰਿਤੀ_READ(src, ਮਾਪ)) {
		ਜਾਓ ਬਾਹਰਲਾ_err;
	}

	(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(dst, src, ਮਾਪ);
	ret = 0;
ਬਾਹਰਲਾ_err:
	ਵਾਪਸ ret;
}

ਪੂਰਨ k_usermode_from_copy(ਖਾਲੀ *dst, ਅਚਲ ਖਾਲੀ *src, size_t ਮਾਪ)
{
	ਵਾਪਸ user_copy(dst, src, ਮਾਪ, false);
}

ਪੂਰਨ k_usermode_ਮੰਜ਼ਿਲ_copy(ਖਾਲੀ *dst, ਅਚਲ ਖਾਲੀ *src, size_t ਮਾਪ)
{
	ਵਾਪਸ user_copy(dst, src, ਮਾਪ, true);
}

ਅੱਖਰ *k_usermode_string_alloc_copy(ਅਚਲ ਅੱਖਰ *src, size_t maxlen)
{
	size_t actual_len;
	ਪੂਰਨ err;
	ਅੱਖਰ *ret = ਨਿਰਾ;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ਜੇ (err != 0) {
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ (ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	ਜੇ (ret != ਨਿਰਾ) {
		ret[actual_len - 1U] = '\0';
	}
ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

ਪੂਰਨ k_usermode_string_copy(ਅੱਖਰ *dst, ਅਚਲ ਅੱਖਰ *src, size_t maxlen)
{
	size_t actual_len;
	ਪੂਰਨ ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ਜੇ (err != 0) {
		ret = EFAULT;
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ (ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

/*
 * Application memory region initialization
 */

ਬਾਹਰੀ ਅੱਖਰ __app_shmem_regions_start[];
ਬਾਹਰੀ ਅੱਖਰ __app_shmem_regions_end[];

ਸਥਿਰ ਖਾਲੀ app_shmem_bss_zero(ਖਾਲੀ)
{
	ਬਣਤਰ z_app_region *region, *ਸਿਰਾ;


	ਸਿਰਾ = (ਬਣਤਰ z_app_region *)&__app_shmem_regions_end[0];
	region = (ਬਣਤਰ z_app_region *)&__app_shmem_regions_start[0];

	ਲਈ ( ; region < ਸਿਰਾ; region++) {
		(ਖਾਲੀ)memset(region->bss_ਸ਼ੁਰੂ, 0, region->bss_ਮਾਪ);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

ਸਥਿਰ uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     ਖਾਲੀ *ssf)
{
	ARG_ਅਣਵਰਤਿਆ(arg2);
	ARG_ਅਣਵਰਤਿਆ(arg3);
	ARG_ਅਣਵਰਤਿਆ(arg4);
	ARG_ਅਣਵਰਤਿਆ(arg5);
	ARG_ਅਣਵਰਤਿਆ(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	ਕੋਡ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ਸਥਿਰ uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, ਖਾਲੀ *ssf)
{
	ARG_ਅਣਵਰਤਿਆ(arg1);
	ARG_ਅਣਵਰਤਿਆ(arg2);
	ARG_ਅਣਵਰਤਿਆ(arg3);
	ARG_ਅਣਵਰਤਿਆ(arg4);
	ARG_ਅਣਵਰਤਿਆ(arg5);
	ARG_ਅਣਵਰਤਿਆ(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	ਕੋਡ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscall_dispatch.c>
