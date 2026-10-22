/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/rb.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/sys_io.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/syscall.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/device.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/app_memory/app_memdomain.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/libc-hooks.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/mutex.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/util.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/linker/linker-defs.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/cache.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>

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

#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log.h>
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
ସ୍ଥିର ସଂରଚନା k_spinlock lists_lock;       /* kobj dlist */

#ifdef CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#ଯଦି defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#define ଗଦା_ELEMENT_DATA_SIZE(size) \
	(ଆକାର(ସଂରଚନା z_ଗଦା_data) + CONFIG_PRIVILEGED_ଗଦା_SIZE + \
	Z_THREAD_ଗଦା_OBJ_ALIGN(size) + K_THREAD_ଗଦା_LEN(size))
#ଅନ୍ୟଥା
#define ଗଦା_ELEMENT_DATA_SIZE(size) (ଆକାର(ସଂରଚନା z_ଗଦା_data) + \
	K_THREAD_ଗଦା_LEN(size))
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#ଅନ୍ୟଥା
#define ଗଦା_ELEMENT_DATA_SIZE(size) K_THREAD_ଗଦା_LEN(size)
#endif /* CONFIG_GEN_PRIV_STACKS */

#endif /* CONFIG_DYNAMIC_OBJECTS */
ସ୍ଥିର ସଂରଚନା k_spinlock obj_lock;         /* kobj struct data */

#define MAX_THREAD_BITS (CONFIG_MAX_THREAD_BYTES * BITS_PER_BYTE)

#ifdef CONFIG_DYNAMIC_OBJECTS
ବାହ୍ୟ uint8_t _thread_idx_map[CONFIG_MAX_THREAD_BYTES];

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_msgq_cleanup(ସଂରଚନା k_msgq *q, bool locked);
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_ଗଦା_cleanup(ସଂରଚନା k_ଗଦା *ଗଦା, bool locked);
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_timer_cleanup(ସଂରଚନା k_timer *timer, bool locked);
#endif /* CONFIG_DYNAMIC_OBJECTS */

ସ୍ଥିର ଶୂନ୍ୟ unref_check(ସଂରଚନା k_object *ko, uintptr_t index, bool locked);

ଅଚଳ ଅକ୍ଷର *otype_to_str(ଗଣନା k_objects otype)
{
	ଅଚଳ ଅକ୍ଷର *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#ifdef CONFIG_LOG
	ବାଛ (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	ସ୍ଥିତି K_OBJ_ANY:
		ret = "generic";
		ଭାଙ୍ଗ;
	ସ୍ଥିତି K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		ଭାଙ୍ଗ;
#ଅନ୍ତର୍ଭାଵ <zephyr/otype-to-str.h>
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ret = "?";
		ଭାଙ୍ଗ;
	}
#ଅନ୍ୟଥା
	ARG_UNUSED(otype);
	ret = ଖାଲି;
#endif /* CONFIG_LOG */
	ଫେରନ୍ତୁ ret;
}

ସଂରଚନା perm_ctx {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା parent_id;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା child_id;
	ସଂରଚନା k_thread *parent;
};

#ifdef CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_ଗଦା_find(k_thread_ଗଦା_t *ଗଦା)
{
	ସଂରଚନା k_object *obj = k_object_find(ଗଦା);

	__ASSERT(obj != ଖାଲି, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_ଗଦା_ELEMENT,
		 "bad stack object");

	ଫେରନ୍ତୁ obj->data.ଗଦା_data->priv;
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
#ଅନ୍ୟଥା
#define DYN_OBJ_DATA_ALIGN_K_THREAD	(ଆକାର(ଶୂନ୍ୟ *))
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#ifdef CONFIG_DYNAMIC_THREAD_ଗଦା_SIZE
#ଯଦି defined(CONFIG_MPU_ଗଦା_GUARD) || defined(CONFIG_PMP_ଗଦା_GUARD)
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ଗଦା \
	Z_THREAD_ଗଦା_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_ଗଦା_SIZE)
#ଅନ୍ୟଥା
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ଗଦା \
	Z_THREAD_ଗଦା_OBJ_ALIGN(CONFIG_PRIVILEGED_ଗଦା_SIZE)
#endif /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#ଅନ୍ୟଥା
#define DYN_OBJ_DATA_ALIGN_K_THREAD_ଗଦା \
	Z_THREAD_ଗଦା_OBJ_ALIGN(ARCH_ଗଦା_PTR_ALIGN)
#endif /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#define DYN_OBJ_DATA_ALIGN		\
	MAX(DYN_OBJ_DATA_ALIGN_K_THREAD, (ଆକାର(ଶୂନ୍ୟ *)))

ସଂରଚନା dyn_obj {
	ସଂରଚନା k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	ଶୂନ୍ୟ *data;
};

ବାହ୍ୟ ସଂରଚନା k_object *z_object_gperf_find(ଅଚଳ ଶୂନ୍ୟ *obj);
ବାହ୍ୟ ଶୂନ୍ୟ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     ଶୂନ୍ୟ *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
ସ୍ଥିର sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

ସ୍ଥିର size_t obj_size_get(ଗଣନା k_objects otype)
{
	size_t ret;

	ବାଛ (otype) {
#ଅନ୍ତର୍ଭାଵ <zephyr/otype-to-size.h>
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ret = ଆକାର(ଅଚଳ ସଂରଚନା device);
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର size_t obj_align_get(ଗଣନା k_objects otype)
{
	size_t ret;

	ବାଛ (otype) {
	ସ୍ଥିତି K_OBJ_THREAD:
#ifdef ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#ଅନ୍ୟଥା
		ret = __alignof(ସଂରଚନା dyn_obj);
#endif /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ret = __alignof(ସଂରଚନା dyn_obj);
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ସଂରଚନା dyn_obj *dyn_object_find_locked(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ସଂରଚନା dyn_obj *ଗଣ୍ଠି;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, ଗଣ୍ଠି, dobj_list) {
		ଯଦି (ଗଣ୍ଠି->kobj.name == obj) {
			ଫେରନ୍ତୁ ଗଣ୍ଠି;
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ସଂରଚନା dyn_obj *dyn_object_find(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ସଂରଚନା dyn_obj *ଗଣ୍ଠି;
	k_spinlock_ଚାବି_t ଚାବି;

	ଚାବି = k_spin_lock(&lists_lock);
	ଗଣ୍ଠି = dyn_object_find_locked(obj);
	k_spin_unlock(&lists_lock, ଚାବି);

	ଫେରନ୍ତୁ ଗଣ୍ଠି;
}

ସ୍ଥିର ଶୂନ୍ୟ clear_perms_cb(ସଂରଚନା k_object *ko, ଶୂନ୍ୟ *ctx_ptr)
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
ସ୍ଥିର bool thread_idx_alloc(uintptr_t *tidx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା base;
	bool ret = false;
	k_spinlock_ଚାବି_t ଚାବି;

	base = 0;
	ଚାବି = k_spin_lock(&lists_lock);
	ପାଇଁ (i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		idx = find_lsb_set(_thread_idx_map[i]);

		ଯଦି (idx != 0) {
			ସଂରଚନା dyn_obj *obj, *next;

			*tidx = base + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(BIT(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (ଶୂନ୍ୟ *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
				clear_perms_cb(&obj->kobj, (ଶୂନ୍ୟ *)*tidx);
			}

			ret = true;
			ଭାଙ୍ଗ;
		}

		base += 8;
	}

	k_spin_unlock(&lists_lock, ଚାବି);
	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର ଶୂନ୍ୟ thread_idx_free(uintptr_t tidx)
{
	ସଂରଚନା dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (ଶୂନ୍ୟ *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		clear_perms_cb(&obj->kobj, (ଶୂନ୍ୟ *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା base = tidx / ଅଙ୍କ_BITS(_thread_idx_map[0]);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = tidx % ଅଙ୍କ_BITS(_thread_idx_map[0]);

	_thread_idx_map[base] |= BIT(offset);

	k_spin_unlock(&lists_lock, ଚାବି);
}

ସ୍ଥିର ସଂରଚନା k_object *dynamic_object_create(ଗଣନା k_objects otype, size_t align,
					      size_t size)
{
	ସଂରଚନା dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, ଆକାର(ସଂରଚନା dyn_obj));
	ଯଦି (dyn == ଖାଲି) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (otype == K_OBJ_THREAD_ଗଦା_ELEMENT) {
		size_t adjusted_size;

		ଯଦି (size == 0) {
			k_free(dyn);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		adjusted_size = ଗଦା_ELEMENT_DATA_SIZE(size);
		ଯଦି (adjusted_size < size) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		dyn->data = z_thread_aligned_alloc(DYN_OBJ_DATA_ALIGN_K_THREAD_ଗଦା,
						     adjusted_size);
		ଯଦି (dyn->data == ଖାଲି) {
			k_free(dyn);
			ଫେରନ୍ତୁ ଖାଲି;
		}

#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ଗଦା_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		ଯଦି (sys_cache_is_ptr_uncached(dyn->data)) {
			dyn->data = sys_cache_cached_ptr_get(dyn->data);
		}
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#ifdef CONFIG_GEN_PRIV_STACKS
		ସଂରଚନା z_ଗଦା_data *ଗଦା_data = (ସଂରଚନା z_ଗଦା_data *)
			((uint8_t *)dyn->data + adjusted_size - ଆକାର(*ଗଦା_data));
#ଯଦି defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		ଗଦା_data->priv = (ଶୂନ୍ୟ *)ROUND_UP(((uint8_t *)dyn->data + size),
			  Z_THREAD_ଗଦା_OBJ_ALIGN(size));
#ଅନ୍ୟଥା
		ଗଦା_data->priv = (uint8_t *)dyn->data;
#endif /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		ଗଦା_data->size = adjusted_size;
		dyn->kobj.data.ଗଦା_data = ଗଦା_data;
		dyn->kobj.name = dyn->data;
#ଅନ୍ୟଥା
		dyn->kobj.name = dyn->data;
		dyn->kobj.data.ଗଦା_size = adjusted_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	} ଅନ୍ୟଥା {
		size_t total_size = obj_size_get(otype) + size;

		ଯଦି (total_size < size) {
			k_free(dyn);
			ଫେରନ୍ତୁ ଖାଲି;
		}

		dyn->data = z_thread_aligned_alloc(align, total_size);
		ଯଦି (dyn->data == ଖାଲି) {
			k_free(dyn);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		dyn->kobj.name = dyn->data;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(ଶୂନ୍ୟ)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_BYTES);

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&lists_lock);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_lock, ଚାବି);

	ଫେରନ୍ତୁ &dyn->kobj;
}

ସଂରଚନା k_object *k_object_create_dynamic_aligned(size_t align, size_t size)
{
	ସଂରଚନା k_object *obj = dynamic_object_create(K_OBJ_ANY, align, size);

	ଯଦି (obj == ଖାଲି) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	ଫେରନ୍ତୁ obj;
}

ସ୍ଥିର ଶୂନ୍ୟ *z_object_alloc(ଗଣନା k_objects otype, size_t size)
{
	ସଂରଚନା k_object *zo;
	uintptr_t tidx = 0;

	ଯଦି ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_LAST)) {
		LOG_ERR("bad object type %d requested", otype);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ବାଛ (otype) {
	ସ୍ଥିତି K_OBJ_THREAD:
		ଯଦି (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			ଫେରନ୍ତୁ ଖାଲି;
		}
		ଭାଙ୍ଗ;
	/* The following are currently not allowed at all */
	ସ୍ଥିତି K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	ସ୍ଥିତି K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_to_str(otype));
		ଫେରନ୍ତୁ ଖାଲି;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Remainder within bounds are permitted */
		ଭାଙ୍ଗ;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), size);
	ଯଦି (zo == ଖାଲି) {
		ଯଦି (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (otype == K_OBJ_THREAD) {
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

	ଫେରନ୍ତୁ zo->name;
}

ଶୂନ୍ୟ *z_impl_k_object_alloc(ଗଣନା k_objects otype)
{
	ଫେରନ୍ତୁ z_object_alloc(otype, 0);
}

ଶୂନ୍ୟ *z_impl_k_object_alloc_size(ଗଣନା k_objects otype, size_t size)
{
	ଫେରନ୍ତୁ z_object_alloc(otype, size);
}

ଶୂନ୍ୟ k_object_free(ଶୂନ୍ୟ *obj)
{
	ସଂରଚନା dyn_obj *dyn = ଖାଲି;
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
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&lists_lock);

	dyn = dyn_object_find_locked(obj);

	ଯଦି (dyn != ଖାଲି) {
		sys_dlist_remove(&dyn->dobj_list);

		ଯଦି (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.data.thread_id;
		}
	}
	k_spin_unlock(&lists_lock, ଚାବି);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	ଯଦି (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	ଯଦି (dyn != ଖାଲି) {
#ifdef CONFIG_OBJ_CORE
		ଯଦି (dyn->kobj.type != K_OBJ_THREAD_ଗଦା_ELEMENT) {
			k_obj_core_evict_range(obj, obj_size_get(dyn->kobj.type));
		}
#endif /* CONFIG_OBJ_CORE */
#ifdef CONFIG_DYNAMIC_OBJECTS_FORCE_ଗଦା_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		ଯଦି (dyn->kobj.type == K_OBJ_THREAD_ଗଦା_ELEMENT) {
			uint8_t *ଗଦା;

			ଯଦି (sys_cache_is_ptr_cached(dyn->data)) {
				ଗଦା = sys_cache_uncached_ptr_get(dyn->data);
			} ଅନ୍ୟଥା {
				ଗଦା = dyn->data;
			}

			k_free(ଗଦା);
		} ଅନ୍ୟଥା
#endif /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->data);
		}
		k_free(dyn);
	}
}

ସଂରଚନା k_object *k_object_find(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ସଂରଚନା k_object *ret;

	ret = z_object_gperf_find(obj);

	ଯଦି (ret == ଖାଲି) {
		ସଂରଚନା dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_find(obj);
		ଯଦି (dyn != ଖାଲି) {
			ret = &dyn->kobj;
		}
	}

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ k_object_wordlist_foreach(_wordlist_cb_func_t func, ଶୂନ୍ୟ *context)
{
	ସଂରଚନା dyn_obj *obj, *next;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&lists_lock);

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&obj_list, obj, next, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_lock, ଚାବି);
}
#endif /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#ifdef CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ସଂରଚନା k_object *z_object_gperf_find(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ARG_UNUSED(obj);
	ଫେରନ୍ତୁ ଖାଲି;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ଶୂନ୍ୟ z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, ଶୂନ୍ୟ *context)
{
	ARG_UNUSED(func);
	ARG_UNUSED(context);
}
#ଅନ୍ୟଥା
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ସଂରଚନା k_object *k_object_find(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ଫେରନ୍ତୁ ଖାଲି;
}
Z_GENERIC_SECTION(.kobject_data.text.dummies)
__weak ଶୂନ୍ୟ k_object_wordlist_foreach(_wordlist_cb_func_t func, ଶୂନ୍ୟ *context)
{
}
#endif

ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା thread_index_get(ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା k_object *ko;

	ko = k_object_find(thread);

	ଯଦି (ko == ଖାଲି) {
		ଫେରନ୍ତୁ -1;
	}

	ଫେରନ୍ତୁ ko->data.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
ସ୍ଥିର ଶୂନ୍ୟ unref_check(ସଂରଚନା k_object *ko, uintptr_t index, bool locked)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&obj_lock);

	sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);

#ifdef CONFIG_DYNAMIC_OBJECTS
	ଯଦି ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		ଯାଅ out;
	}

	ଶୂନ୍ୟ *vko = ko;

	ସଂରଚନା dyn_obj *dyn = CONTAINER_OF(vko, ସଂରଚନା dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, ସଂରଚନା dyn_obj), "unaligned z_object");

	ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < CONFIG_MAX_THREAD_BYTES; i++) {
		ଯଦି (ko->perms[i] != 0U) {
			ଯାଅ out;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	ବାଛ (ko->type) {
	ସ୍ଥିତି K_OBJ_MSGQ:
		(ଶୂନ୍ୟ)z_msgq_cleanup((ସଂରଚନା k_msgq *)ko->name, locked);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି K_OBJ_ଗଦା:
		(ଶୂନ୍ୟ)z_ଗଦା_cleanup((ସଂରଚନା k_ଗଦା *)ko->name, locked);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		ଯଦି ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(ଶୂନ୍ୟ)z_timer_cleanup((ସଂରଚନା k_timer *)ko->name, locked);
		}
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Nothing to do */
		ଭାଙ୍ଗ;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->data);
	k_free(dyn);
out:
#endif /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_lock, ଚାବି);
}

ସ୍ଥିର ଶୂନ୍ୟ wordlist_cb(ସଂରଚନା k_object *ko, ଶୂନ୍ୟ *ctx_ptr)
{
	ସଂରଚନା perm_ctx *ctx = (ସଂରଚନା perm_ctx *)ctx_ptr;

	ଯଦି (sys_bitfield_test_bit((mem_addr_t)&ko->perms, ctx->parent_id) &&
				  ((ସଂରଚନା k_thread *)ko->name != ctx->parent)) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, ctx->child_id);
	}
}

ଶୂନ୍ୟ k_thread_perms_inherit(ସଂରଚନା k_thread *parent, ସଂରଚନା k_thread *child)
{
	ସଂରଚନା perm_ctx ctx = {
		thread_index_get(parent),
		thread_index_get(child),
		parent
	};

	ଯଦି ((ctx.parent_id != -1) && (ctx.child_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

ଶୂନ୍ୟ k_thread_perms_set(ସଂରଚନା k_object *ko, ସଂରଚନା k_thread *thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = thread_index_get(thread);

	ଯଦି (index != -1) {
		sys_bitfield_set_bit((mem_addr_t)&ko->perms, index);
	}
}

ଶୂନ୍ୟ k_thread_perms_clear(ସଂରଚନା k_object *ko, ସଂରଚନା k_thread *thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = thread_index_get(thread);

	ଯଦି (index != -1) {
		sys_bitfield_clear_bit((mem_addr_t)&ko->perms, index);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#ifdef CONFIG_DYNAMIC_OBJECTS
		k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&lists_lock);

		unref_check(ko, index, false);
		k_spin_unlock(&lists_lock, ଚାବି);
#ଅନ୍ୟଥା
		unref_check(ko, index, false);
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}
}

ସ୍ଥିର ଶୂନ୍ୟ clear_perms_cb_locked(ସଂରଚନା k_object *ko, ଶୂନ୍ୟ *ctx_ptr)
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
ଶୂନ୍ୟ k_thread_perms_all_clear(ସଂରଚନା k_thread *thread)
{
	uintptr_t index = thread_index_get(thread);

	ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)index != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (ଶୂନ୍ୟ *)index);
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା thread_perms_test(ସଂରଚନା k_object *ko)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା index;

	ଯଦି ((ko->flags & K_OBJ_FLAG_PUBLIC) != 0U) {
		ଫେରନ୍ତୁ 1;
	}

	index = thread_index_get(_current);
	ଯଦି (index != -1) {
		ଫେରନ୍ତୁ sys_bitfield_test_bit((mem_addr_t)&ko->perms, index);
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ dump_permission_error(ସଂରଚନା k_object *ko)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = thread_index_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, index,
		otype_to_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, ଆକାର(ko->perms), "permission bitmap");
}

ଶୂନ୍ୟ k_object_dump_error(ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval, ଅଚଳ ଶୂନ୍ୟ *obj, ସଂରଚନା k_object *ko,
			ଗଣନା k_objects otype)
{
	ବାଛ (retval) {
	ସ୍ଥିତି -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_to_str(otype));
		ଯଦି (ko == ଖାଲି) {
			LOG_ERR("address is not a known kernel object");
		} ଅନ୍ୟଥା {
			LOG_ERR("address is actually a %s",
				otype_to_str(ko->type));
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି -EPERM:
		dump_permission_error(ko);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_to_str(otype));
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Not handled error */
		ଭାଙ୍ଗ;
	}
}

ଶୂନ୍ୟ z_impl_k_object_access_grant(ଅଚଳ ଶୂନ୍ୟ *object, ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା k_object *ko = k_object_find(object);

	ଯଦି (ko != ଖାଲି) {
		k_thread_perms_set(ko, thread);
	}
}

ଶୂନ୍ୟ k_object_access_revoke(ଅଚଳ ଶୂନ୍ୟ *object, ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା k_object *ko = k_object_find(object);

	ଯଦି (ko != ଖାଲି) {
		k_thread_perms_clear(ko, thread);
	}
}

ଶୂନ୍ୟ k_object_access_revoke_others(ଅଚଳ ଶୂନ୍ୟ *object)
{
	ସଂରଚନା k_object *ko = k_object_find(object);

	ଯଦି (ko != ଖାଲି) {
		(ଶୂନ୍ୟ)memset(ko->perms, 0, ଆକାର(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_PUBLIC;
	}
}

ଶୂନ୍ୟ z_impl_k_object_release(ଅଚଳ ଶୂନ୍ୟ *object)
{
	k_object_access_revoke(object, _current);
}

ଶୂନ୍ୟ k_object_access_all_grant(ଅଚଳ ଶୂନ୍ୟ *object)
{
	ସଂରଚନା k_object *ko = k_object_find(object);

	ଯଦି (ko != ଖାଲି) {
		ko->flags |= K_OBJ_FLAG_PUBLIC;
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_object_validate(ସଂରଚନା k_object *ko, ଗଣନା k_objects otype,
		       ଗଣନା _obj_init_check init)
{
	ଯଦି (unlikely(ko == ଖାଲି)) {
		ଫେରନ୍ତୁ -EBADF;
	}

	ଯଦି (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		ଯଦି ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_FIRST) ||
		    (ko->type > K_OBJ_DRIVER_LAST)) {
			ଫେରନ୍ତୁ -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	ଯଦି (unlikely(thread_perms_test(ko) == 0)) {
		ଫେରନ୍ତୁ -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	ଯଦି (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		ଯଦି (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			ଫେରନ୍ତୁ -EINVAL;
		}
	} ଅନ୍ୟଥା ଯଦି (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		ଯଦି (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			ଫେରନ୍ତୁ -EADDRINUSE;
		}
	} ଅନ୍ୟଥା {
		/* _OBJ_INIT_ANY */
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_object_access_check(ଅଚଳ ଶୂନ୍ୟ *object)
{
	ଫେରନ୍ତୁ k_object_validate(k_object_find(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

ଶୂନ୍ୟ k_object_init(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ସଂରଚନା k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_find(obj);
	ଯଦି (ko == ଖାଲି) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		ଫେରନ୍ତୁ;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

ଶୂନ୍ୟ k_object_recycle(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ସଂରଚନା k_object *ko = k_object_find(obj);

	ଯଦି (ko != ଖାଲି) {
		(ଶୂନ୍ୟ)memset(ko->perms, 0, ଆକାର(ko->perms));
		k_thread_perms_set(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

ଶୂନ୍ୟ k_object_uninit(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ସଂରଚନା k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_find(obj);
	ଯଦି (ko == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
ଶୂନ୍ୟ *k_usermode_alloc_from_copy(ଅଚଳ ଶୂନ୍ୟ *src, size_t size)
{
	ଶୂନ୍ୟ *dst = ଖାଲି;

	/* Does the caller in user mode have access to read this memory? */
	ଯଦି (K_SYSCALL_MEMORY_READ(src, size)) {
		ଯାଅ out_err;
	}

	dst = z_thread_malloc(size);
	ଯଦି (dst == ଖାଲି) {
		LOG_ERR("out of thread resource pool memory (%zu)", size);
		ଯାଅ out_err;
	}

	(ଶୂନ୍ୟ)memcpy(dst, src, size);
out_err:
	ଫେରନ୍ତୁ dst;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା user_copy(ଶୂନ୍ୟ *dst, ଅଚଳ ଶୂନ୍ୟ *src, size_t size, bool to_user)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	ଯଦି (to_user ? K_SYSCALL_MEMORY_WRITE(dst, size) :
			K_SYSCALL_MEMORY_READ(src, size)) {
		ଯାଅ out_err;
	}

	(ଶୂନ୍ୟ)memcpy(dst, src, size);
	ret = 0;
out_err:
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_usermode_from_copy(ଶୂନ୍ୟ *dst, ଅଚଳ ଶୂନ୍ୟ *src, size_t size)
{
	ଫେରନ୍ତୁ user_copy(dst, src, size, false);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_usermode_to_copy(ଶୂନ୍ୟ *dst, ଅଚଳ ଶୂନ୍ୟ *src, size_t size)
{
	ଫେରନ୍ତୁ user_copy(dst, src, size, true);
}

ଅକ୍ଷର *k_usermode_string_alloc_copy(ଅଚଳ ଅକ୍ଷର *src, size_t maxlen)
{
	size_t actual_len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	ଅକ୍ଷର *ret = ଖାଲି;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ଯଦି (err != 0) {
		ଯାଅ out;
	}
	ଯଦି (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ଯାଅ out;
	}
	ଯଦି (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ଯାଅ out;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	ଯଦି (ret != ଖାଲି) {
		ret[actual_len - 1U] = '\0';
	}
out:
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_usermode_string_copy(ଅକ୍ଷର *dst, ଅଚଳ ଅକ୍ଷର *src, size_t maxlen)
{
	size_t actual_len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ଯଦି (err != 0) {
		ret = EFAULT;
		ଯାଅ out;
	}
	ଯଦି (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		ଯାଅ out;
	}
	ଯଦି (size_add_overflow(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		ଯାଅ out;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
out:
	ଫେରନ୍ତୁ ret;
}

/*
 * Application memory region initialization
 */

ବାହ୍ୟ ଅକ୍ଷର __app_shmem_regions_start[];
ବାହ୍ୟ ଅକ୍ଷର __app_shmem_regions_end[];

ସ୍ଥିର ଶୂନ୍ୟ app_shmem_bss_zero(ଶୂନ୍ୟ)
{
	ସଂରଚନା z_app_region *region, *end;


	end = (ସଂରଚନା z_app_region *)&__app_shmem_regions_end[0];
	region = (ସଂରଚନା z_app_region *)&__app_shmem_regions_start[0];

	ପାଇଁ ( ; region < end; region++) {
		(ଶୂନ୍ୟ)memset(region->bss_start, 0, region->bss_size);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

ସ୍ଥିର uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     ଶୂନ୍ୟ *ssf)
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

ସ୍ଥିର uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, ଶୂନ୍ୟ *ssf)
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

#ଅନ୍ତର୍ଭାଵ <zephyr/syscall_dispatch.c>
