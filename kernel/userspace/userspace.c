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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_LIBC_ವಿಭಜನೆ_EXISTS
K_APPMEM_ವಿಭಜನೆ_DEFINE(z_libc_ವಿಭಜನೆ);
#ಆದರೆ_ಅಂತ್ಯ /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MBEDTLS
K_APPMEM_ವಿಭಜನೆ_DEFINE(k_mbedtls_ವಿಭಜನೆ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MBEDTLS */

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS
ಸ್ಥಿರ ರಚನೆ k_spinlock lists_ಬೀಗ;       /* kobj dlist */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#ಆದರೆ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ELEMENT_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಗಾತ್ರ_2) \
	(ಗಾತ್ರ(ರಚನೆ z_ರಾಶಿ_ದತ್ತಾಂಶ) + CONFIG_PRIVILEGED_ರಾಶಿ_ಗಾತ್ರ + \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(ಗಾತ್ರ_2) + K_THREAD_ರಾಶಿ_LEN(ಗಾತ್ರ_2))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ELEMENT_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಗಾತ್ರ_2) (ಗಾತ್ರ(ರಚನೆ z_ರಾಶಿ_ದತ್ತಾಂಶ) + \
	K_THREAD_ರಾಶಿ_LEN(ಗಾತ್ರ_2))
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ELEMENT_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಗಾತ್ರ_2) K_THREAD_ರಾಶಿ_LEN(ಗಾತ್ರ_2)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_GEN_PRIV_STACKS */

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS */
ಸ್ಥಿರ ರಚನೆ k_spinlock obj_ಬೀಗ;         /* kobj struct data */

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_THREAD_ಬಿಟ್ಟುಗಳು (CONFIG_MAX_THREAD_ಬೈಟುಗಳು * ಬಿಟ್ಟುಗಳು_ಪ್ರತಿ_BYTE)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS
ಬಾಹ್ಯ uint8_t _thread_idx_map[CONFIG_MAX_THREAD_ಬೈಟುಗಳು];

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ z_msgq_cleanup(ರಚನೆ k_msgq *q, bool locked);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ z_ರಾಶಿ_cleanup(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ, bool locked);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ z_timer_cleanup(ರಚನೆ k_timer *timer, bool locked);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS */

ಸ್ಥಿರ ಶೂನ್ಯ unref_check(ರಚನೆ k_object *ko, uintptr_t ಸೂಚ್ಯಂಕ, bool locked);

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *otype_ಗಮ್ಯ_str(ಎಣಿಕೆ k_objects otype)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LOG
	ಆಯ್ಕೆ (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	ಸ್ಥಿತಿ K_OBJ_ANY:
		ret = "generic";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		ನಿಲ್ಲಿಸು;
#ಅನ್ತರ್ಭಾವ <zephyr/otype-to-str.h>
	ಪೂರ್ವನಿಯೋಜಿತ:
		ret = "?";
		ನಿಲ್ಲಿಸು;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(otype);
	ret = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_LOG */
	ಹಿಂತಿರುಗಿ ret;
}

ರಚನೆ perm_ctx {
	ಪೂರ್ಣಾಂಕ ಪೋಷಕ_id;
	ಪೂರ್ಣಾಂಕ ಮಗು_id;
	ರಚನೆ k_thread *ಪೋಷಕ;
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ರಾಶಿ_ಕಂಡುಹಿಡಿ(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ರಚನೆ k_object *obj = k_object_ಕಂಡುಹಿಡಿ(ರಾಶಿ);

	__ASSERT(obj != ಖಾಲಿ, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ರಾಶಿ_ELEMENT,
		 "bad stack object");

	ಹಿಂತಿರುಗಿ obj->ದತ್ತಾಂಶ.ರಾಶಿ_ದತ್ತಾಂಶ->priv;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_GEN_PRIV_STACKS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#ವ್ಯಾಖ್ಯಾನಿಸು DYN_OBJ_ದತ್ತಾಂಶ_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು DYN_OBJ_ದತ್ತಾಂಶ_ALIGN_K_THREAD	(ಗಾತ್ರ(ಶೂನ್ಯ *))
#ಆದರೆ_ಅಂತ್ಯ /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_THREAD_ರಾಶಿ_ಗಾತ್ರ
#ಆದರೆ defined(CONFIG_MPU_ರಾಶಿ_GUARD) || defined(CONFIG_PMP_ರಾಶಿ_GUARD)
#ವ್ಯಾಖ್ಯಾನಿಸು DYN_OBJ_ದತ್ತಾಂಶ_ALIGN_K_THREAD_ರಾಶಿ \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ರಾಶಿ_ಗಾತ್ರ)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು DYN_OBJ_ದತ್ತಾಂಶ_ALIGN_K_THREAD_ರಾಶಿ \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(CONFIG_PRIVILEGED_ರಾಶಿ_ಗಾತ್ರ)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು DYN_OBJ_ದತ್ತಾಂಶ_ALIGN_K_THREAD_ರಾಶಿ \
	Z_THREAD_ರಾಶಿ_OBJ_ALIGN(ARCH_ರಾಶಿ_PTR_ALIGN)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#ವ್ಯಾಖ್ಯಾನಿಸು DYN_OBJ_ದತ್ತಾಂಶ_ALIGN		\
	MAX(DYN_OBJ_ದತ್ತಾಂಶ_ALIGN_K_THREAD, (ಗಾತ್ರ(ಶೂನ್ಯ *)))

ರಚನೆ dyn_obj {
	ರಚನೆ k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	ಶೂನ್ಯ *ದತ್ತಾಂಶ;
};

ಬಾಹ್ಯ ರಚನೆ k_object *z_object_gperf_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj);
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

ಸ್ಥಿರ size_t obj_ಗಾತ್ರ_get(ಎಣಿಕೆ k_objects otype)
{
	size_t ret;

	ಆಯ್ಕೆ (otype) {
#ಅನ್ತರ್ಭಾವ <zephyr/otype-to-size.h>
	ಪೂರ್ವನಿಯೋಜಿತ:
		ret = ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ರಚನೆ device);
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ size_t obj_align_get(ಎಣಿಕೆ k_objects otype)
{
	size_t ret;

	ಆಯ್ಕೆ (otype) {
	ಸ್ಥಿತಿ K_OBJ_THREAD:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#ಇಲ್ಲದಿದ್ದರೆ
		ret = __alignof(ರಚನೆ dyn_obj);
#ಆದರೆ_ಅಂತ್ಯ /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ret = __alignof(ರಚನೆ dyn_obj);
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ರಚನೆ dyn_obj *dyn_object_ಕಂಡುಹಿಡಿ_locked(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ರಚನೆ dyn_obj *ಗಂಟು;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, ಗಂಟು, dobj_list) {
		ಆದರೆ (ಗಂಟು->kobj.name == obj) {
			ಹಿಂತಿರುಗಿ ಗಂಟು;
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ರಚನೆ dyn_obj *dyn_object_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ರಚನೆ dyn_obj *ಗಂಟು;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&lists_ಬೀಗ);
	ಗಂಟು = dyn_object_ಕಂಡುಹಿಡಿ_locked(obj);
	k_spin_unlock(&lists_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ಗಂಟು;
}

ಸ್ಥಿರ ಶೂನ್ಯ clear_perms_cb(ರಚನೆ k_object *ko, ಶೂನ್ಯ *ctx_ptr)
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
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ idx;
	ಪೂರ್ಣಾಂಕ ತಳ;
	bool ret = false;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ತಳ = 0;
	ಕೀಲಿ = k_spin_ಬೀಗ(&lists_ಬೀಗ);
	ಗಾಗಿ (i = 0; i < CONFIG_MAX_THREAD_ಬೈಟುಗಳು; i++) {
		idx = ಕಂಡುಹಿಡಿ_lsb_ಗಣ(_thread_idx_map[i]);

		ಆದರೆ (idx != 0) {
			ರಚನೆ dyn_obj *obj, *ಮುಂದಿನ;

			*tidx = ತಳ + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(ಬಿಟ್(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (ಶೂನ್ಯ *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_ಸುರಕ್ಷಿತ(&obj_list, obj, ಮುಂದಿನ, dobj_list) {
				clear_perms_cb(&obj->kobj, (ಶೂನ್ಯ *)*tidx);
			}

			ret = true;
			ನಿಲ್ಲಿಸು;
		}

		ತಳ += 8;
	}

	k_spin_unlock(&lists_ಬೀಗ, ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ ret;
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
	ರಚನೆ dyn_obj *obj, *ಮುಂದಿನ;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (ಶೂನ್ಯ *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&lists_ಬೀಗ);

	SYS_DLIST_FOR_EACH_CONTAINER_ಸುರಕ್ಷಿತ(&obj_list, obj, ಮುಂದಿನ, dobj_list) {
		clear_perms_cb(&obj->kobj, (ಶೂನ್ಯ *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	ಪೂರ್ಣಾಂಕ ತಳ = tidx / ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2(_thread_idx_map[0]);
	ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ = tidx % ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2(_thread_idx_map[0]);

	_thread_idx_map[ತಳ] |= ಬಿಟ್(ಸ್ಥಾನಾಂತರ);

	k_spin_unlock(&lists_ಬೀಗ, ಕೀಲಿ);
}

ಸ್ಥಿರ ರಚನೆ k_object *dynamic_object_create(ಎಣಿಕೆ k_objects otype, size_t align,
					      size_t ಗಾತ್ರ_2)
{
	ರಚನೆ dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, ಗಾತ್ರ(ರಚನೆ dyn_obj));
	ಆದರೆ (dyn == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (otype == K_OBJ_THREAD_ರಾಶಿ_ELEMENT) {
		size_t adjusted_ಗಾತ್ರ;

		ಆದರೆ (ಗಾತ್ರ_2 == 0) {
			k_free(dyn);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		adjusted_ಗಾತ್ರ = ರಾಶಿ_ELEMENT_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಗಾತ್ರ_2);
		ಆದರೆ (adjusted_ಗಾತ್ರ < ಗಾತ್ರ_2) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		dyn->ದತ್ತಾಂಶ = z_thread_aligned_alloc(DYN_OBJ_ದತ್ತಾಂಶ_ALIGN_K_THREAD_ರಾಶಿ,
						     adjusted_ಗಾತ್ರ);
		ಆದರೆ (dyn->ದತ್ತಾಂಶ == ಖಾಲಿ) {
			k_free(dyn);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS_FORCE_ರಾಶಿ_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		ಆದರೆ (sys_cache_is_ptr_uncached(dyn->ದತ್ತಾಂಶ)) {
			dyn->ದತ್ತಾಂಶ = sys_cache_cached_ptr_get(dyn->ದತ್ತಾಂಶ);
		}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_GEN_PRIV_STACKS
		ರಚನೆ z_ರಾಶಿ_ದತ್ತಾಂಶ *ರಾಶಿ_ದತ್ತಾಂಶ = (ರಚನೆ z_ರಾಶಿ_ದತ್ತಾಂಶ *)
			((uint8_t *)dyn->ದತ್ತಾಂಶ + adjusted_ಗಾತ್ರ - ಗಾತ್ರ(*ರಾಶಿ_ದತ್ತಾಂಶ));
#ಆದರೆ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ರಾಶಿ_ದತ್ತಾಂಶ->priv = (ಶೂನ್ಯ *)ಸುತ್ತು_ಮೇಲೆ(((uint8_t *)dyn->ದತ್ತಾಂಶ + ಗಾತ್ರ_2),
			  Z_THREAD_ರಾಶಿ_OBJ_ALIGN(ಗಾತ್ರ_2));
#ಇಲ್ಲದಿದ್ದರೆ
		ರಾಶಿ_ದತ್ತಾಂಶ->priv = (uint8_t *)dyn->ದತ್ತಾಂಶ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ರಾಶಿ_ದತ್ತಾಂಶ->ಗಾತ್ರ_2 = adjusted_ಗಾತ್ರ;
		dyn->kobj.ದತ್ತಾಂಶ.ರಾಶಿ_ದತ್ತಾಂಶ = ರಾಶಿ_ದತ್ತಾಂಶ;
		dyn->kobj.name = dyn->ದತ್ತಾಂಶ;
#ಇಲ್ಲದಿದ್ದರೆ
		dyn->kobj.name = dyn->ದತ್ತಾಂಶ;
		dyn->kobj.ದತ್ತಾಂಶ.ರಾಶಿ_ಗಾತ್ರ = adjusted_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_GEN_PRIV_STACKS */
	} ಇಲ್ಲದಿದ್ದರೆ {
		size_t ಒಟ್ಟು_ಗಾತ್ರ = obj_ಗಾತ್ರ_get(otype) + ಗಾತ್ರ_2;

		ಆದರೆ (ಒಟ್ಟು_ಗಾತ್ರ < ಗಾತ್ರ_2) {
			k_free(dyn);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		dyn->ದತ್ತಾಂಶ = z_thread_aligned_alloc(align, ಒಟ್ಟು_ಗಾತ್ರ);
		ಆದರೆ (dyn->ದತ್ತಾಂಶ == ಖಾಲಿ) {
			k_free(dyn);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		dyn->kobj.name = dyn->ದತ್ತಾಂಶ;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(ಶೂನ್ಯ)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_ಬೈಟುಗಳು);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&lists_ಬೀಗ);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ &dyn->kobj;
}

ರಚನೆ k_object *k_object_create_dynamic_aligned(size_t align, size_t ಗಾತ್ರ_2)
{
	ರಚನೆ k_object *obj = dynamic_object_create(K_OBJ_ANY, align, ಗಾತ್ರ_2);

	ಆದರೆ (obj == ಖಾಲಿ) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	ಹಿಂತಿರುಗಿ obj;
}

ಸ್ಥಿರ ಶೂನ್ಯ *z_object_alloc(ಎಣಿಕೆ k_objects otype, size_t ಗಾತ್ರ_2)
{
	ರಚನೆ k_object *zo;
	uintptr_t tidx = 0;

	ಆದರೆ ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_ಕೊನೆಯ)) {
		LOG_ERR("bad object type %d requested", otype);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆಯ್ಕೆ (otype) {
	ಸ್ಥಿತಿ K_OBJ_THREAD:
		ಆದರೆ (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		ನಿಲ್ಲಿಸು;
	/* The following are currently not allowed at all */
	ಸ್ಥಿತಿ K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	ಸ್ಥಿತಿ K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_ಗಮ್ಯ_str(otype));
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Remainder within bounds are permitted */
		ನಿಲ್ಲಿಸು;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), ಗಾತ್ರ_2);
	ಆದರೆ (zo == ಖಾಲಿ) {
		ಆದರೆ (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (otype == K_OBJ_THREAD) {
		zo->ದತ್ತಾಂಶ.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_ಗಣ(zo, _current);

	/* Activates reference counting logic for automatic disposal when
	 * all permissions have been revoked
	 */
	zo->flags |= K_OBJ_FLAG_ALLOC;

	ಹಿಂತಿರುಗಿ zo->name;
}

ಶೂನ್ಯ *z_impl_k_object_alloc(ಎಣಿಕೆ k_objects otype)
{
	ಹಿಂತಿರುಗಿ z_object_alloc(otype, 0);
}

ಶೂನ್ಯ *z_impl_k_object_alloc_ಗಾತ್ರ(ಎಣಿಕೆ k_objects otype, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ z_object_alloc(otype, ಗಾತ್ರ_2);
}

ಶೂನ್ಯ k_object_free(ಶೂನ್ಯ *obj)
{
	ರಚನೆ dyn_obj *dyn = ಖಾಲಿ;
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
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&lists_ಬೀಗ);

	dyn = dyn_object_ಕಂಡುಹಿಡಿ_locked(obj);

	ಆದರೆ (dyn != ಖಾಲಿ) {
		sys_dlist_remove(&dyn->dobj_list);

		ಆದರೆ (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.ದತ್ತಾಂಶ.thread_id;
		}
	}
	k_spin_unlock(&lists_ಬೀಗ, ಕೀಲಿ);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	ಆದರೆ (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	ಆದರೆ (dyn != ಖಾಲಿ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE
		ಆದರೆ (dyn->kobj.type != K_OBJ_THREAD_ರಾಶಿ_ELEMENT) {
			k_obj_core_evict_range(obj, obj_ಗಾತ್ರ_get(dyn->kobj.type));
		}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS_FORCE_ರಾಶಿ_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		ಆದರೆ (dyn->kobj.type == K_OBJ_THREAD_ರಾಶಿ_ELEMENT) {
			uint8_t *ರಾಶಿ;

			ಆದರೆ (sys_cache_is_ptr_cached(dyn->ದತ್ತಾಂಶ)) {
				ರಾಶಿ = sys_cache_uncached_ptr_get(dyn->ದತ್ತಾಂಶ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ರಾಶಿ = dyn->ದತ್ತಾಂಶ;
			}

			k_free(ರಾಶಿ);
		} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->ದತ್ತಾಂಶ);
		}
		k_free(dyn);
	}
}

ರಚನೆ k_object *k_object_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ರಚನೆ k_object *ret;

	ret = z_object_gperf_ಕಂಡುಹಿಡಿ(obj);

	ಆದರೆ (ret == ಖಾಲಿ) {
		ರಚನೆ dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_ಕಂಡುಹಿಡಿ(obj);
		ಆದರೆ (dyn != ಖಾಲಿ) {
			ret = &dyn->kobj;
		}
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ k_object_wordlist_foreach(_wordlist_cb_func_t func, ಶೂನ್ಯ *context)
{
	ರಚನೆ dyn_obj *obj, *ಮುಂದಿನ;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&lists_ಬೀಗ);

	SYS_DLIST_FOR_EACH_CONTAINER_ಸುರಕ್ಷಿತ(&obj_list, obj, ಮುಂದಿನ, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_ಬೀಗ, ಕೀಲಿ);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_ದತ್ತಾಂಶ.ಪಠ್ಯ.dummies)
__weak ರಚನೆ k_object *z_object_gperf_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ARG_ಬಳಸದ(obj);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
Z_GENERIC_SECTION(.kobject_ದತ್ತಾಂಶ.ಪಠ್ಯ.dummies)
__weak ಶೂನ್ಯ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, ಶೂನ್ಯ *context)
{
	ARG_ಬಳಸದ(func);
	ARG_ಬಳಸದ(context);
}
#ಇಲ್ಲದಿದ್ದರೆ
Z_GENERIC_SECTION(.kobject_ದತ್ತಾಂಶ.ಪಠ್ಯ.dummies)
__weak ರಚನೆ k_object *k_object_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
Z_GENERIC_SECTION(.kobject_ದತ್ತಾಂಶ.ಪಠ್ಯ.dummies)
__weak ಶೂನ್ಯ k_object_wordlist_foreach(_wordlist_cb_func_t func, ಶೂನ್ಯ *context)
{
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ thread_ಸೂಚ್ಯಂಕ_get(ರಚನೆ k_thread *thread)
{
	ರಚನೆ k_object *ko;

	ko = k_object_ಕಂಡುಹಿಡಿ(thread);

	ಆದರೆ (ko == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -1;
	}

	ಹಿಂತಿರುಗಿ ko->ದತ್ತಾಂಶ.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ಸ್ಥಿರ ಶೂನ್ಯ unref_check(ರಚನೆ k_object *ko, uintptr_t ಸೂಚ್ಯಂಕ, bool locked)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&obj_ಬೀಗ);

	sys_bitfield_clear_ಬಿಟ್((mem_addr_t)&ko->perms, ಸೂಚ್ಯಂಕ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS
	ಆದರೆ ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಶೂನ್ಯ *vko = ko;

	ರಚನೆ dyn_obj *dyn = CONTAINER_ದ(vko, ರಚನೆ dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, ರಚನೆ dyn_obj), "unaligned z_object");

	ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < CONFIG_MAX_THREAD_ಬೈಟುಗಳು; i++) {
		ಆದರೆ (ko->perms[i] != 0U) {
			ಹೋಗು ಹೊರಹೋಗು;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	ಆಯ್ಕೆ (ko->type) {
	ಸ್ಥಿತಿ K_OBJ_MSGQ:
		(ಶೂನ್ಯ)z_msgq_cleanup((ರಚನೆ k_msgq *)ko->name, locked);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_OBJ_ರಾಶಿ:
		(ಶೂನ್ಯ)z_ರಾಶಿ_cleanup((ರಚನೆ k_ರಾಶಿ_2 *)ko->name, locked);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		ಆದರೆ ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(ಶೂನ್ಯ)z_timer_cleanup((ರಚನೆ k_timer *)ko->name, locked);
		}
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Nothing to do */
		ನಿಲ್ಲಿಸು;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->ದತ್ತಾಂಶ);
	k_free(dyn);
ಹೊರಹೋಗು:
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_ಬೀಗ, ಕೀಲಿ);
}

ಸ್ಥಿರ ಶೂನ್ಯ wordlist_cb(ರಚನೆ k_object *ko, ಶೂನ್ಯ *ctx_ptr)
{
	ರಚನೆ perm_ctx *ctx = (ರಚನೆ perm_ctx *)ctx_ptr;

	ಆದರೆ (sys_bitfield_test_ಬಿಟ್((mem_addr_t)&ko->perms, ctx->ಪೋಷಕ_id) &&
				  ((ರಚನೆ k_thread *)ko->name != ctx->ಪೋಷಕ)) {
		sys_bitfield_ಗಣ_ಬಿಟ್((mem_addr_t)&ko->perms, ctx->ಮಗು_id);
	}
}

ಶೂನ್ಯ k_thread_perms_inherit(ರಚನೆ k_thread *ಪೋಷಕ, ರಚನೆ k_thread *ಮಗು)
{
	ರಚನೆ perm_ctx ctx = {
		thread_ಸೂಚ್ಯಂಕ_get(ಪೋಷಕ),
		thread_ಸೂಚ್ಯಂಕ_get(ಮಗು),
		ಪೋಷಕ
	};

	ಆದರೆ ((ctx.ಪೋಷಕ_id != -1) && (ctx.ಮಗು_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

ಶೂನ್ಯ k_thread_perms_ಗಣ(ರಚನೆ k_object *ko, ರಚನೆ k_thread *thread)
{
	ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = thread_ಸೂಚ್ಯಂಕ_get(thread);

	ಆದರೆ (ಸೂಚ್ಯಂಕ != -1) {
		sys_bitfield_ಗಣ_ಬಿಟ್((mem_addr_t)&ko->perms, ಸೂಚ್ಯಂಕ);
	}
}

ಶೂನ್ಯ k_thread_perms_clear(ರಚನೆ k_object *ko, ರಚನೆ k_thread *thread)
{
	ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = thread_ಸೂಚ್ಯಂಕ_get(thread);

	ಆದರೆ (ಸೂಚ್ಯಂಕ != -1) {
		sys_bitfield_clear_ಬಿಟ್((mem_addr_t)&ko->perms, ಸೂಚ್ಯಂಕ);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&lists_ಬೀಗ);

		unref_check(ko, ಸೂಚ್ಯಂಕ, false);
		k_spin_unlock(&lists_ಬೀಗ, ಕೀಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
		unref_check(ko, ಸೂಚ್ಯಂಕ, false);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS */
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ clear_perms_cb_locked(ರಚನೆ k_object *ko, ಶೂನ್ಯ *ctx_ptr)
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
ಶೂನ್ಯ k_thread_perms_all_clear(ರಚನೆ k_thread *thread)
{
	uintptr_t ಸೂಚ್ಯಂಕ = thread_ಸೂಚ್ಯಂಕ_get(thread);

	ಆದರೆ ((ಪೂರ್ಣಾಂಕ)ಸೂಚ್ಯಂಕ != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (ಶೂನ್ಯ *)ಸೂಚ್ಯಂಕ);
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ thread_perms_test(ರಚನೆ k_object *ko)
{
	ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ;

	ಆದರೆ ((ko->flags & K_OBJ_FLAG_ಬಹಿರಂಗ) != 0U) {
		ಹಿಂತಿರುಗಿ 1;
	}

	ಸೂಚ್ಯಂಕ = thread_ಸೂಚ್ಯಂಕ_get(_current);
	ಆದರೆ (ಸೂಚ್ಯಂಕ != -1) {
		ಹಿಂತಿರುಗಿ sys_bitfield_test_ಬಿಟ್((mem_addr_t)&ko->perms, ಸೂಚ್ಯಂಕ);
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_permission_error(ರಚನೆ k_object *ko)
{
	ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = thread_ಸೂಚ್ಯಂಕ_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, ಸೂಚ್ಯಂಕ,
		otype_ಗಮ್ಯ_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, ಗಾತ್ರ(ko->perms), "permission bitmap");
}

ಶೂನ್ಯ k_object_dump_error(ಪೂರ್ಣಾಂಕ retval, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj, ರಚನೆ k_object *ko,
			ಎಣಿಕೆ k_objects otype)
{
	ಆಯ್ಕೆ (retval) {
	ಸ್ಥಿತಿ -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_ಗಮ್ಯ_str(otype));
		ಆದರೆ (ko == ಖಾಲಿ) {
			LOG_ERR("address is not a known kernel object");
		} ಇಲ್ಲದಿದ್ದರೆ {
			LOG_ERR("address is actually a %s",
				otype_ಗಮ್ಯ_str(ko->type));
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ -EPERM:
		dump_permission_error(ko);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_ಗಮ್ಯ_str(otype));
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Not handled error */
		ನಿಲ್ಲಿಸು;
	}
}

ಶೂನ್ಯ z_impl_k_object_access_grant(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object, ರಚನೆ k_thread *thread)
{
	ರಚನೆ k_object *ko = k_object_ಕಂಡುಹಿಡಿ(object);

	ಆದರೆ (ko != ಖಾಲಿ) {
		k_thread_perms_ಗಣ(ko, thread);
	}
}

ಶೂನ್ಯ k_object_access_revoke(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object, ರಚನೆ k_thread *thread)
{
	ರಚನೆ k_object *ko = k_object_ಕಂಡುಹಿಡಿ(object);

	ಆದರೆ (ko != ಖಾಲಿ) {
		k_thread_perms_clear(ko, thread);
	}
}

ಶೂನ್ಯ k_object_access_revoke_others(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object)
{
	ರಚನೆ k_object *ko = k_object_ಕಂಡುಹಿಡಿ(object);

	ಆದರೆ (ko != ಖಾಲಿ) {
		(ಶೂನ್ಯ)memset(ko->perms, 0, ಗಾತ್ರ(ko->perms));
		k_thread_perms_ಗಣ(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_ಬಹಿರಂಗ;
	}
}

ಶೂನ್ಯ z_impl_k_object_release(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object)
{
	k_object_access_revoke(object, _current);
}

ಶೂನ್ಯ k_object_access_all_grant(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object)
{
	ರಚನೆ k_object *ko = k_object_ಕಂಡುಹಿಡಿ(object);

	ಆದರೆ (ko != ಖಾಲಿ) {
		ko->flags |= K_OBJ_FLAG_ಬಹಿರಂಗ;
	}
}

ಪೂರ್ಣಾಂಕ k_object_validate(ರಚನೆ k_object *ko, ಎಣಿಕೆ k_objects otype,
		       ಎಣಿಕೆ _obj_init_check init)
{
	ಆದರೆ (unlikely(ko == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ -EBADF;
	}

	ಆದರೆ (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		ಆದರೆ ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_ಮೊದಲ) ||
		    (ko->type > K_OBJ_DRIVER_ಕೊನೆಯ)) {
			ಹಿಂತಿರುಗಿ -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	ಆದರೆ (unlikely(thread_perms_test(ko) == 0)) {
		ಹಿಂತಿರುಗಿ -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	ಆದರೆ (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		ಆದರೆ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			ಹಿಂತಿರುಗಿ -EINVAL;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		ಆದರೆ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			ಹಿಂತಿರುಗಿ -EADDRINUSE;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* _OBJ_INIT_ANY */
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_impl_k_object_access_check(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object)
{
	ಹಿಂತಿರುಗಿ k_object_validate(k_object_ಕಂಡುಹಿಡಿ(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

ಶೂನ್ಯ k_object_init(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ರಚನೆ k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_ಕಂಡುಹಿಡಿ(obj);
	ಆದರೆ (ko == ಖಾಲಿ) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		ಹಿಂತಿರುಗಿ;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

ಶೂನ್ಯ k_object_recycle(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ರಚನೆ k_object *ko = k_object_ಕಂಡುಹಿಡಿ(obj);

	ಆದರೆ (ko != ಖಾಲಿ) {
		(ಶೂನ್ಯ)memset(ko->perms, 0, ಗಾತ್ರ(ko->perms));
		k_thread_perms_ಗಣ(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

ಶೂನ್ಯ k_object_uninit(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ರಚನೆ k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_ಕಂಡುಹಿಡಿ(obj);
	ಆದರೆ (ko == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
ಶೂನ್ಯ *k_usermode_alloc_from_copy(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *dst = ಖಾಲಿ;

	/* Does the caller in user mode have access to read this memory? */
	ಆದರೆ (K_SYSCALL_ಸ್ಮರಣೆ_READ(src, ಗಾತ್ರ_2)) {
		ಹೋಗು ಹೊರಹೋಗು_err;
	}

	dst = z_thread_malloc(ಗಾತ್ರ_2);
	ಆದರೆ (dst == ಖಾಲಿ) {
		LOG_ERR("out of thread resource pool memory (%zu)", ಗಾತ್ರ_2);
		ಹೋಗು ಹೊರಹೋಗು_err;
	}

	(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(dst, src, ಗಾತ್ರ_2);
ಹೊರಹೋಗು_err:
	ಹಿಂತಿರುಗಿ dst;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ user_copy(ಶೂನ್ಯ *dst, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, size_t ಗಾತ್ರ_2, bool ಗಮ್ಯ_user)
{
	ಪೂರ್ಣಾಂಕ ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	ಆದರೆ (ಗಮ್ಯ_user ? K_SYSCALL_ಸ್ಮರಣೆ_WRITE(dst, ಗಾತ್ರ_2) :
			K_SYSCALL_ಸ್ಮರಣೆ_READ(src, ಗಾತ್ರ_2)) {
		ಹೋಗು ಹೊರಹೋಗು_err;
	}

	(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(dst, src, ಗಾತ್ರ_2);
	ret = 0;
ಹೊರಹೋಗು_err:
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_usermode_from_copy(ಶೂನ್ಯ *dst, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ user_copy(dst, src, ಗಾತ್ರ_2, false);
}

ಪೂರ್ಣಾಂಕ k_usermode_ಗಮ್ಯ_copy(ಶೂನ್ಯ *dst, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ user_copy(dst, src, ಗಾತ್ರ_2, true);
}

ಅಕ್ಷರ *k_usermode_string_alloc_copy(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t maxlen)
{
	size_t actual_len;
	ಪೂರ್ಣಾಂಕ err;
	ಅಕ್ಷರ *ret = ಖಾಲಿ;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ಆದರೆ (err != 0) {
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ (ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	ಆದರೆ (ret != ಖಾಲಿ) {
		ret[actual_len - 1U] = '\0';
	}
ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_usermode_string_copy(ಅಕ್ಷರ *dst, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t maxlen)
{
	size_t actual_len;
	ಪೂರ್ಣಾಂಕ ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ಆದರೆ (err != 0) {
		ret = EFAULT;
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ (ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

/*
 * Application memory region initialization
 */

ಬಾಹ್ಯ ಅಕ್ಷರ __app_shmem_regions_start[];
ಬಾಹ್ಯ ಅಕ್ಷರ __app_shmem_regions_end[];

ಸ್ಥಿರ ಶೂನ್ಯ app_shmem_bss_zero(ಶೂನ್ಯ)
{
	ರಚನೆ z_app_region *region, *ಕೊನೆ;


	ಕೊನೆ = (ರಚನೆ z_app_region *)&__app_shmem_regions_end[0];
	region = (ರಚನೆ z_app_region *)&__app_shmem_regions_start[0];

	ಗಾಗಿ ( ; region < ಕೊನೆ; region++) {
		(ಶೂನ್ಯ)memset(region->bss_ಆರಂಭ, 0, region->bss_ಗಾತ್ರ);
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
	ARG_ಬಳಸದ(arg2);
	ARG_ಬಳಸದ(arg3);
	ARG_ಬಳಸದ(arg4);
	ARG_ಬಳಸದ(arg5);
	ARG_ಬಳಸದ(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	ಸಂಕೇತ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ಸ್ಥಿರ uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, ಶೂನ್ಯ *ssf)
{
	ARG_ಬಳಸದ(arg1);
	ARG_ಬಳಸದ(arg2);
	ARG_ಬಳಸದ(arg3);
	ARG_ಬಳಸದ(arg4);
	ARG_ಬಳಸದ(arg5);
	ARG_ಬಳಸದ(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	ಸಂಕೇತ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#ಅನ್ತರ್ಭಾವ <zephyr/syscall_dispatch.c>
