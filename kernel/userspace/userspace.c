/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/rb.h>
#అన్తర్భావ <zephyr/sys/sys_io.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/syscall.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/device.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <zephyr/app_memory/app_memdomain.h>
#అన్తర్భావ <zephyr/sys/libc-hooks.h>
#అన్తర్భావ <zephyr/sys/mutex.h>
#అన్తర్భావ <zephyr/sys/util.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <zephyr/linker/linker-defs.h>
#అన్తర్భావ <zephyr/cache.h>
#అన్తర్భావ <kernel_internal.h>

#నిర్వచితమైతే Z_LIBC_విభజన_EXISTS
K_APPMEM_విభజన_DEFINE(z_libc_విభజన);
#అయితే_ముగింపు /* Z_LIBC_PARTITION_EXISTS */

/* TODO: Find a better place to put this. Since we pull the entire
 * lib..__modules__crypto__mbedtls.a  globals into app shared memory
 * section, we can't put this in zephyr_init.c of the mbedtls module.
 */
#నిర్వచితమైతే CONFIG_MBEDTLS
K_APPMEM_విభజన_DEFINE(k_mbedtls_విభజన);
#అయితే_ముగింపు /* CONFIG_MBEDTLS */

#అన్తర్భావ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* The originally synchronization strategy made heavy use of recursive
 * irq_locking, which ports poorly to spinlocks which are
 * non-recursive.  Rather than try to redesign as part of
 * spinlockification, this uses multiple locks to preserve the
 * original semantics exactly.  The locks are named for the data they
 * protect where possible, or just for the code that uses them where
 * not.
 */
#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS
స్థిర నిర్మాణం k_spinlock lists_తాళం;       /* kobj dlist */

#నిర్వచితమైతే CONFIG_GEN_PRIV_STACKS
/* On ARM & ARC MPU & RISC-V PMP we may have two different alignment requirement
 * when dynamically allocating thread stacks, one for the privileged
 * stack and other for the user stack, so we need to account the
 * worst alignment scenario and reserve space for that.
 */
#ఒకవేళ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
#నిర్వచించు దొంతర_ELEMENT_దత్తాంశం_కొలత(కొలత) \
	(పరిమాణం(నిర్మాణం z_దొంతర_దత్తాంశం) + CONFIG_PRIVILEGED_దొంతర_కొలత + \
	Z_THREAD_దొంతర_OBJ_ALIGN(కొలత) + K_THREAD_దొంతర_LEN(కొలత))
#లేకపోతే
#నిర్వచించు దొంతర_ELEMENT_దత్తాంశం_కొలత(కొలత) (పరిమాణం(నిర్మాణం z_దొంతర_దత్తాంశం) + \
	K_THREAD_దొంతర_LEN(కొలత))
#అయితే_ముగింపు /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
#లేకపోతే
#నిర్వచించు దొంతర_ELEMENT_దత్తాంశం_కొలత(కొలత) K_THREAD_దొంతర_LEN(కొలత)
#అయితే_ముగింపు /* CONFIG_GEN_PRIV_STACKS */

#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS */
స్థిర నిర్మాణం k_spinlock obj_తాళం;         /* kobj struct data */

#నిర్వచించు MAX_THREAD_బిట్లు (CONFIG_MAX_THREAD_బైట్లు * బిట్లు_చొప్పున_BYTE)

#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS
బాహ్య uint8_t _thread_idx_map[CONFIG_MAX_THREAD_బైట్లు];

బాహ్య పూర్ణాంకం z_msgq_cleanup(నిర్మాణం k_msgq *q, bool locked);
బాహ్య పూర్ణాంకం z_దొంతర_cleanup(నిర్మాణం k_దొంతర *దొంతర, bool locked);
బాహ్య పూర్ణాంకం z_timer_cleanup(నిర్మాణం k_timer *timer, bool locked);
#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS */

స్థిర శూన్య unref_check(నిర్మాణం k_object *ko, uintptr_t సూచీ, bool locked);

స్థిరరాశి అక్షరం *otype_గమ్యం_str(గణన k_objects otype)
{
	స్థిరరాశి అక్షరం *ret;
	/* -fdata-sections doesn't work right except in very recent
	 * GCC and these literal strings would appear in the binary even if
	 * otype_to_str was omitted by the linker
	 */
#నిర్వచితమైతే CONFIG_LOG
	ఎంపిక (otype) {
	/* otype-to-str.h is generated automatically during build by
	 * gen_kobject_list.py
	 */
	స్థితి K_OBJ_ANY:
		ret = "generic";
		ఆపు;
	స్థితి K_OBJ_DRIVER_ANY:
		ret = "generic driver";
		ఆపు;
#అన్తర్భావ <zephyr/otype-to-str.h>
	అప్రమేయ:
		ret = "?";
		ఆపు;
	}
#లేకపోతే
	ARG_వాడని(otype);
	ret = ఖాళీ;
#అయితే_ముగింపు /* CONFIG_LOG */
	తిరిగి ret;
}

నిర్మాణం perm_ctx {
	పూర్ణాంకం మాతృ_id;
	పూర్ణాంకం బిడ్డ_id;
	నిర్మాణం k_thread *మాతృ;
};

#నిర్వచితమైతే CONFIG_GEN_PRIV_STACKS
/* See write_gperf_table() in scripts/build/gen_kobject_list.py. The privilege
 * mode stacks are allocated as an array. The base of the array is
 * aligned to Z_PRIVILEGE_STACK_ALIGN, and all members must be as well.
 */
uint8_t *z_priv_దొంతర_కనుగొను(k_thread_దొంతర_t *దొంతర)
{
	నిర్మాణం k_object *obj = k_object_కనుగొను(దొంతర);

	__ASSERT(obj != ఖాళీ, "stack object not found");
	__ASSERT(obj->type == K_OBJ_THREAD_దొంతర_ELEMENT,
		 "bad stack object");

	తిరిగి obj->దత్తాంశం.దొంతర_దత్తాంశం->priv;
}
#అయితే_ముగింపు /* CONFIG_GEN_PRIV_STACKS */

#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS

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
#నిర్వచితమైతే ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
#నిర్వచించు DYN_OBJ_దత్తాంశం_ALIGN_K_THREAD	(ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT)
#లేకపోతే
#నిర్వచించు DYN_OBJ_దత్తాంశం_ALIGN_K_THREAD	(పరిమాణం(శూన్య *))
#అయితే_ముగింపు /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */

#నిర్వచితమైతే CONFIG_DYNAMIC_THREAD_దొంతర_కొలత
#ఒకవేళ defined(CONFIG_MPU_దొంతర_GUARD) || defined(CONFIG_PMP_దొంతర_GUARD)
#నిర్వచించు DYN_OBJ_దత్తాంశం_ALIGN_K_THREAD_దొంతర \
	Z_THREAD_దొంతర_OBJ_ALIGN(CONFIG_DYNAMIC_THREAD_దొంతర_కొలత)
#లేకపోతే
#నిర్వచించు DYN_OBJ_దత్తాంశం_ALIGN_K_THREAD_దొంతర \
	Z_THREAD_దొంతర_OBJ_ALIGN(CONFIG_PRIVILEGED_దొంతర_కొలత)
#అయితే_ముగింపు /* CONFIG_MPU_STACK_GUARD || CONFIG_PMP_STACK_GUARD */
#లేకపోతే
#నిర్వచించు DYN_OBJ_దత్తాంశం_ALIGN_K_THREAD_దొంతర \
	Z_THREAD_దొంతర_OBJ_ALIGN(ARCH_దొంతర_PTR_ALIGN)
#అయితే_ముగింపు /* CONFIG_DYNAMIC_THREAD_STACK_SIZE */

#నిర్వచించు DYN_OBJ_దత్తాంశం_ALIGN		\
	MAX(DYN_OBJ_దత్తాంశం_ALIGN_K_THREAD, (పరిమాణం(శూన్య *)))

నిర్మాణం dyn_obj {
	నిర్మాణం k_object kobj;
	sys_dnode_t dobj_list;

	/* The object itself */
	శూన్య *దత్తాంశం;
};

బాహ్య నిర్మాణం k_object *z_object_gperf_కనుగొను(స్థిరరాశి శూన్య *obj);
బాహ్య శూన్య z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func,
					     శూన్య *context);

/*
 * Linked list of allocated kernel objects, for iteration over all allocated
 * objects (and potentially deleting them during iteration).
 */
స్థిర sys_dlist_t obj_list = SYS_DLIST_STATIC_INIT(&obj_list);

/*
 * TODO: Write some hash table code that will replace obj_list.
 */

స్థిర size_t obj_కొలత_get(గణన k_objects otype)
{
	size_t ret;

	ఎంపిక (otype) {
#అన్తర్భావ <zephyr/otype-to-size.h>
	అప్రమేయ:
		ret = పరిమాణం(స్థిరరాశి నిర్మాణం device);
		ఆపు;
	}

	తిరిగి ret;
}

స్థిర size_t obj_align_get(గణన k_objects otype)
{
	size_t ret;

	ఎంపిక (otype) {
	స్థితి K_OBJ_THREAD:
#నిర్వచితమైతే ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT
		ret = ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT;
#లేకపోతే
		ret = __alignof(నిర్మాణం dyn_obj);
#అయితే_ముగింపు /* ARCH_DYNAMIC_OBJ_K_THREAD_ALIGNMENT */
		ఆపు;
	అప్రమేయ:
		ret = __alignof(నిర్మాణం dyn_obj);
		ఆపు;
	}

	తిరిగి ret;
}

స్థిర నిర్మాణం dyn_obj *dyn_object_కనుగొను_locked(స్థిరరాశి శూన్య *obj)
{
	నిర్మాణం dyn_obj *కణుపు;

	/* For any dynamically allocated kernel object, the object
	 * pointer is just a member of the containing struct dyn_obj,
	 * so just a little arithmetic is necessary to locate the
	 * corresponding struct rbnode
	 */
	SYS_DLIST_FOR_EACH_CONTAINER(&obj_list, కణుపు, dobj_list) {
		ఒకవేళ (కణుపు->kobj.name == obj) {
			తిరిగి కణుపు;
		}
	}

	తిరిగి ఖాళీ;
}

స్థిర నిర్మాణం dyn_obj *dyn_object_కనుగొను(స్థిరరాశి శూన్య *obj)
{
	నిర్మాణం dyn_obj *కణుపు;
	k_spinlock_కీలకం_t కీలకం;

	కీలకం = k_spin_తాళం(&lists_తాళం);
	కణుపు = dyn_object_కనుగొను_locked(obj);
	k_spin_unlock(&lists_తాళం, కీలకం);

	తిరిగి కణుపు;
}

స్థిర శూన్య clear_perms_cb(నిర్మాణం k_object *ko, శూన్య *ctx_ptr)
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
స్థిర bool thread_idx_alloc(uintptr_t *tidx)
{
	పూర్ణాంకం i;
	పూర్ణాంకం idx;
	పూర్ణాంకం పునాది;
	bool ret = false;
	k_spinlock_కీలకం_t కీలకం;

	పునాది = 0;
	కీలకం = k_spin_తాళం(&lists_తాళం);
	కొరకు (i = 0; i < CONFIG_MAX_THREAD_బైట్లు; i++) {
		idx = కనుగొను_lsb_సమితి(_thread_idx_map[i]);

		ఒకవేళ (idx != 0) {
			నిర్మాణం dyn_obj *obj, *తదుపరి;

			*tidx = పునాది + (idx - 1);

			/* Clear the bit. We already know the array index,
			 * and the bit to be cleared.
			 */
			_thread_idx_map[i] &= ~(బిట్టు(idx - 1));

			/* Clear permission from all objects */

			z_object_gperf_wordlist_foreach(clear_perms_cb, (శూన్య *)*tidx);

			SYS_DLIST_FOR_EACH_CONTAINER_సురక్షిత(&obj_list, obj, తదుపరి, dobj_list) {
				clear_perms_cb(&obj->kobj, (శూన్య *)*tidx);
			}

			ret = true;
			ఆపు;
		}

		పునాది += 8;
	}

	k_spin_unlock(&lists_తాళం, కీలకం);
	తిరిగి ret;
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
స్థిర శూన్య thread_idx_free(uintptr_t tidx)
{
	నిర్మాణం dyn_obj *obj, *తదుపరి;

	z_object_gperf_wordlist_foreach(clear_perms_cb, (శూన్య *)tidx);

	/* Hold lists_lock so two concurrent k_object_free() calls on thread indices
	 * within the same byte cannot lose a bit in the non-atomic |=.
	 */
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&lists_తాళం);

	SYS_DLIST_FOR_EACH_CONTAINER_సురక్షిత(&obj_list, obj, తదుపరి, dobj_list) {
		clear_perms_cb(&obj->kobj, (శూన్య *)tidx);
	}

	/* Figure out which bits to set in _thread_idx_map[] and set it. */
	పూర్ణాంకం పునాది = tidx / అంకె_బిట్లు_2(_thread_idx_map[0]);
	పూర్ణాంకం స్థానభ్రంశం = tidx % అంకె_బిట్లు_2(_thread_idx_map[0]);

	_thread_idx_map[పునాది] |= బిట్టు(స్థానభ్రంశం);

	k_spin_unlock(&lists_తాళం, కీలకం);
}

స్థిర నిర్మాణం k_object *dynamic_object_create(గణన k_objects otype, size_t align,
					      size_t కొలత)
{
	నిర్మాణం dyn_obj *dyn;

	dyn = z_thread_aligned_alloc(align, పరిమాణం(నిర్మాణం dyn_obj));
	ఒకవేళ (dyn == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (otype == K_OBJ_THREAD_దొంతర_ELEMENT) {
		size_t adjusted_కొలత;

		ఒకవేళ (కొలత == 0) {
			k_free(dyn);
			తిరిగి ఖాళీ;
		}

		adjusted_కొలత = దొంతర_ELEMENT_దత్తాంశం_కొలత(కొలత);
		ఒకవేళ (adjusted_కొలత < కొలత) {
			/* The size adjustment above overflowed, which would
			 * hand out an allocation smaller than requested.
			 */
			k_free(dyn);
			తిరిగి ఖాళీ;
		}

		dyn->దత్తాంశం = z_thread_aligned_alloc(DYN_OBJ_దత్తాంశం_ALIGN_K_THREAD_దొంతర,
						     adjusted_కొలత);
		ఒకవేళ (dyn->దత్తాంశం == ఖాళీ) {
			k_free(dyn);
			తిరిగి ఖాళీ;
		}

#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS_FORCE_దొంతర_CACHED
		/* With kernel coherence enabled, it is possible that the stack
		 * has been allocated on uncached area. This has an implications on
		 * performance as memory access is not cached.
		 * This simply nudges the indicated stack pointer to be inside
		 * cached area such that the thread object will have its stack
		 * inside cached area.
		 */
		ఒకవేళ (sys_cache_is_ptr_uncached(dyn->దత్తాంశం)) {
			dyn->దత్తాంశం = sys_cache_cached_ptr_get(dyn->దత్తాంశం);
		}
#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */

#నిర్వచితమైతే CONFIG_GEN_PRIV_STACKS
		నిర్మాణం z_దొంతర_దత్తాంశం *దొంతర_దత్తాంశం = (నిర్మాణం z_దొంతర_దత్తాంశం *)
			((uint8_t *)dyn->దత్తాంశం + adjusted_కొలత - పరిమాణం(*దొంతర_దత్తాంశం));
#ఒకవేళ defined(CONFIG_ARM_MPU) || defined(CONFIG_ARC_MPU) || defined(CONFIG_RISCV_PMP)
		దొంతర_దత్తాంశం->priv = (శూన్య *)విడత_పైకి(((uint8_t *)dyn->దత్తాంశం + కొలత),
			  Z_THREAD_దొంతర_OBJ_ALIGN(కొలత));
#లేకపోతే
		దొంతర_దత్తాంశం->priv = (uint8_t *)dyn->దత్తాంశం;
#అయితే_ముగింపు /* CONFIG_ARM_MPU || CONFIG_ARC_MPU || CONFIG_RISCV_PMP */
		దొంతర_దత్తాంశం->కొలత = adjusted_కొలత;
		dyn->kobj.దత్తాంశం.దొంతర_దత్తాంశం = దొంతర_దత్తాంశం;
		dyn->kobj.name = dyn->దత్తాంశం;
#లేకపోతే
		dyn->kobj.name = dyn->దత్తాంశం;
		dyn->kobj.దత్తాంశం.దొంతర_కొలత = adjusted_కొలత;
#అయితే_ముగింపు /* CONFIG_GEN_PRIV_STACKS */
	} లేకపోతే {
		size_t మొత్తం_కొలత = obj_కొలత_get(otype) + కొలత;

		ఒకవేళ (మొత్తం_కొలత < కొలత) {
			k_free(dyn);
			తిరిగి ఖాళీ;
		}

		dyn->దత్తాంశం = z_thread_aligned_alloc(align, మొత్తం_కొలత);
		ఒకవేళ (dyn->దత్తాంశం == ఖాళీ) {
			k_free(dyn);
			తిరిగి ఖాళీ;
		}
		dyn->kobj.name = dyn->దత్తాంశం;
	}

	dyn->kobj.type = otype;
	dyn->kobj.flags = 0;
	(శూన్య)memset(dyn->kobj.perms, 0, CONFIG_MAX_THREAD_బైట్లు);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&lists_తాళం);

	sys_dlist_append(&obj_list, &dyn->dobj_list);
	k_spin_unlock(&lists_తాళం, కీలకం);

	తిరిగి &dyn->kobj;
}

నిర్మాణం k_object *k_object_create_dynamic_aligned(size_t align, size_t కొలత)
{
	నిర్మాణం k_object *obj = dynamic_object_create(K_OBJ_ANY, align, కొలత);

	ఒకవేళ (obj == ఖాళీ) {
		LOG_ERR("could not allocate kernel object, out of memory");
	}

	తిరిగి obj;
}

స్థిర శూన్య *z_object_alloc(గణన k_objects otype, size_t కొలత)
{
	నిర్మాణం k_object *zo;
	uintptr_t tidx = 0;

	ఒకవేళ ((otype <= K_OBJ_ANY) || (otype >= K_OBJ_చివరి)) {
		LOG_ERR("bad object type %d requested", otype);
		తిరిగి ఖాళీ;
	}

	ఎంపిక (otype) {
	స్థితి K_OBJ_THREAD:
		ఒకవేళ (!thread_idx_alloc(&tidx)) {
			LOG_ERR("out of free thread indexes");
			తిరిగి ఖాళీ;
		}
		ఆపు;
	/* The following are currently not allowed at all */
	స్థితి K_OBJ_SYS_MUTEX:			/* Lives in user memory */
	స్థితి K_OBJ_NET_SOCKET:			/* Indeterminate size */
		LOG_ERR("forbidden object type '%s' requested",
			otype_గమ్యం_str(otype));
		తిరిగి ఖాళీ;
	అప్రమేయ:
		/* Remainder within bounds are permitted */
		ఆపు;
	}

	zo = dynamic_object_create(otype, obj_align_get(otype), కొలత);
	ఒకవేళ (zo == ఖాళీ) {
		ఒకవేళ (otype == K_OBJ_THREAD) {
			thread_idx_free(tidx);
		}
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (otype == K_OBJ_THREAD) {
		zo->దత్తాంశం.thread_id = tidx;
	}

	/* The allocating thread implicitly gets permission on kernel objects
	 * that it allocates
	 */
	k_thread_perms_సమితి(zo, _current);

	/* Activates reference counting logic for automatic disposal when
	 * all permissions have been revoked
	 */
	zo->flags |= K_OBJ_FLAG_ALLOC;

	తిరిగి zo->name;
}

శూన్య *z_impl_k_object_alloc(గణన k_objects otype)
{
	తిరిగి z_object_alloc(otype, 0);
}

శూన్య *z_impl_k_object_alloc_కొలత(గణన k_objects otype, size_t కొలత)
{
	తిరిగి z_object_alloc(otype, కొలత);
}

శూన్య k_object_free(శూన్య *obj)
{
	నిర్మాణం dyn_obj *dyn = ఖాళీ;
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
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&lists_తాళం);

	dyn = dyn_object_కనుగొను_locked(obj);

	ఒకవేళ (dyn != ఖాళీ) {
		sys_dlist_remove(&dyn->dobj_list);

		ఒకవేళ (dyn->kobj.type == K_OBJ_THREAD) {
			free_thread_idx = true;
			thread_idx = dyn->kobj.దత్తాంశం.thread_id;
		}
	}
	k_spin_unlock(&lists_తాళం, కీలకం);

	/* thread_idx_free() acquires lists_lock itself to serialize the
	 * non-atomic bitmap update against concurrent frees, so it must
	 * run with the lock dropped here.
	 */
	ఒకవేళ (free_thread_idx) {
		thread_idx_free(thread_idx);
	}

	ఒకవేళ (dyn != ఖాళీ) {
#నిర్వచితమైతే CONFIG_OBJ_CORE
		ఒకవేళ (dyn->kobj.type != K_OBJ_THREAD_దొంతర_ELEMENT) {
			k_obj_core_evict_range(obj, obj_కొలత_get(dyn->kobj.type));
		}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE */
#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS_FORCE_దొంతర_CACHED
		/* We may have nudged the pointer to point to the cached area
		 * in dynamic_object_create() when we first created the thread
		 * stack object. So we need to restore the uncached one before
		 * freeing it.
		 */
		ఒకవేళ (dyn->kobj.type == K_OBJ_THREAD_దొంతర_ELEMENT) {
			uint8_t *దొంతర;

			ఒకవేళ (sys_cache_is_ptr_cached(dyn->దత్తాంశం)) {
				దొంతర = sys_cache_uncached_ptr_get(dyn->దత్తాంశం);
			} లేకపోతే {
				దొంతర = dyn->దత్తాంశం;
			}

			k_free(దొంతర);
		} లేకపోతే
#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS_FORCE_STACK_CACHED */
		{
			k_free(dyn->దత్తాంశం);
		}
		k_free(dyn);
	}
}

నిర్మాణం k_object *k_object_కనుగొను(స్థిరరాశి శూన్య *obj)
{
	నిర్మాణం k_object *ret;

	ret = z_object_gperf_కనుగొను(obj);

	ఒకవేళ (ret == ఖాళీ) {
		నిర్మాణం dyn_obj *dyn;

		/* The cast to pointer-to-non-const violates MISRA
		 * 11.8 but is justified since we know dynamic objects
		 * were not declared with a const qualifier.
		 */
		dyn = dyn_object_కనుగొను(obj);
		ఒకవేళ (dyn != ఖాళీ) {
			ret = &dyn->kobj;
		}
	}

	తిరిగి ret;
}

శూన్య k_object_wordlist_foreach(_wordlist_cb_func_t func, శూన్య *context)
{
	నిర్మాణం dyn_obj *obj, *తదుపరి;

	z_object_gperf_wordlist_foreach(func, context);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&lists_తాళం);

	SYS_DLIST_FOR_EACH_CONTAINER_సురక్షిత(&obj_list, obj, తదుపరి, dobj_list) {
		func(&obj->kobj, context);
	}
	k_spin_unlock(&lists_తాళం, కీలకం);
}
#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS */

/* In the earlier linker-passes before we have the real generated
 * implementation of the lookup functions, we need some weak dummies.
 * Being __weak, they will be replaced by the generated implementations in
 * the later linker passes.
 */
#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS
Z_GENERIC_SECTION(.kobject_దత్తాంశం.పాఠ్యం.dummies)
__weak నిర్మాణం k_object *z_object_gperf_కనుగొను(స్థిరరాశి శూన్య *obj)
{
	ARG_వాడని(obj);
	తిరిగి ఖాళీ;
}
Z_GENERIC_SECTION(.kobject_దత్తాంశం.పాఠ్యం.dummies)
__weak శూన్య z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, శూన్య *context)
{
	ARG_వాడని(func);
	ARG_వాడని(context);
}
#లేకపోతే
Z_GENERIC_SECTION(.kobject_దత్తాంశం.పాఠ్యం.dummies)
__weak నిర్మాణం k_object *k_object_కనుగొను(స్థిరరాశి శూన్య *obj)
{
	తిరిగి ఖాళీ;
}
Z_GENERIC_SECTION(.kobject_దత్తాంశం.పాఠ్యం.dummies)
__weak శూన్య k_object_wordlist_foreach(_wordlist_cb_func_t func, శూన్య *context)
{
}
#అయితే_ముగింపు

స్థిర గుర్తులేని పూర్ణాంకం thread_సూచీ_get(నిర్మాణం k_thread *thread)
{
	నిర్మాణం k_object *ko;

	ko = k_object_కనుగొను(thread);

	ఒకవేళ (ko == ఖాళీ) {
		తిరిగి -1;
	}

	తిరిగి ko->దత్తాంశం.thread_id;
}

/* Caller must hold lists_lock for the duration of this call so that the
 * sys_dlist_remove() below is mutually exclusive with concurrent
 * obj_list traversal in k_object_wordlist_foreach() on another CPU.
 *
 * Note: The 'locked' parameter in this function refers to the scheduler's
 * spinlock.
 */
స్థిర శూన్య unref_check(నిర్మాణం k_object *ko, uintptr_t సూచీ, bool locked)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&obj_తాళం);

	sys_bitfield_clear_బిట్టు((mem_addr_t)&ko->perms, సూచీ);

#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS
	ఒకవేళ ((ko->flags & K_OBJ_FLAG_ALLOC) == 0U) {
		/* skip unref check for static kernel object */
		వెళ్ళు బహిర్గతం;
	}

	శూన్య *vko = ko;

	నిర్మాణం dyn_obj *dyn = CONTAINER_యొక్క(vko, నిర్మాణం dyn_obj, kobj);

	__ASSERT(IS_PTR_ALIGNED(dyn, నిర్మాణం dyn_obj), "unaligned z_object");

	కొరకు (పూర్ణాంకం i = 0; i < CONFIG_MAX_THREAD_బైట్లు; i++) {
		ఒకవేళ (ko->perms[i] != 0U) {
			వెళ్ళు బహిర్గతం;
		}
	}

	/* This object has no more references. Some objects may have
	 * dynamically allocated resources, require cleanup, or need to be
	 * marked as uninitialized when all references are gone. What
	 * specifically needs to happen depends on the object type.
	 */
	ఎంపిక (ko->type) {
	స్థితి K_OBJ_MSGQ:
		(శూన్య)z_msgq_cleanup((నిర్మాణం k_msgq *)ko->name, locked);
		ఆపు;
	స్థితి K_OBJ_దొంతర:
		(శూన్య)z_దొంతర_cleanup((నిర్మాణం k_దొంతర *)ko->name, locked);
		ఆపు;
	స్థితి K_OBJ_TIMER:
		/* k_timer_cleanup() does not check whether the timer has
		 * been initialized; calling it on an uninitialized timer
		 * would read garbage from an uninitialized dnode. Guard
		 * explicitly here.
		 */
		ఒకవేళ ((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U) {
			(శూన్య)z_timer_cleanup((నిర్మాణం k_timer *)ko->name, locked);
		}
		ఆపు;
	అప్రమేయ:
		/* Nothing to do */
		ఆపు;
	}

	sys_dlist_remove(&dyn->dobj_list);
	k_free(dyn->దత్తాంశం);
	k_free(dyn);
బహిర్గతం:
#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS */
	k_spin_unlock(&obj_తాళం, కీలకం);
}

స్థిర శూన్య wordlist_cb(నిర్మాణం k_object *ko, శూన్య *ctx_ptr)
{
	నిర్మాణం perm_ctx *ctx = (నిర్మాణం perm_ctx *)ctx_ptr;

	ఒకవేళ (sys_bitfield_test_బిట్టు((mem_addr_t)&ko->perms, ctx->మాతృ_id) &&
				  ((నిర్మాణం k_thread *)ko->name != ctx->మాతృ)) {
		sys_bitfield_సమితి_బిట్టు((mem_addr_t)&ko->perms, ctx->బిడ్డ_id);
	}
}

శూన్య k_thread_perms_inherit(నిర్మాణం k_thread *మాతృ, నిర్మాణం k_thread *బిడ్డ)
{
	నిర్మాణం perm_ctx ctx = {
		thread_సూచీ_get(మాతృ),
		thread_సూచీ_get(బిడ్డ),
		మాతృ
	};

	ఒకవేళ ((ctx.మాతృ_id != -1) && (ctx.బిడ్డ_id != -1)) {
		k_object_wordlist_foreach(wordlist_cb, &ctx);
	}
}

శూన్య k_thread_perms_సమితి(నిర్మాణం k_object *ko, నిర్మాణం k_thread *thread)
{
	పూర్ణాంకం సూచీ = thread_సూచీ_get(thread);

	ఒకవేళ (సూచీ != -1) {
		sys_bitfield_సమితి_బిట్టు((mem_addr_t)&ko->perms, సూచీ);
	}
}

శూన్య k_thread_perms_clear(నిర్మాణం k_object *ko, నిర్మాణం k_thread *thread)
{
	పూర్ణాంకం సూచీ = thread_సూచీ_get(thread);

	ఒకవేళ (సూచీ != -1) {
		sys_bitfield_clear_బిట్టు((mem_addr_t)&ko->perms, సూచీ);
		/* unref_check() requires lists_lock so its dlist remove is
		 * SMP-safe.
		 */
#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS
		k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&lists_తాళం);

		unref_check(ko, సూచీ, false);
		k_spin_unlock(&lists_తాళం, కీలకం);
#లేకపోతే
		unref_check(ko, సూచీ, false);
#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS */
	}
}

స్థిర శూన్య clear_perms_cb_locked(నిర్మాణం k_object *ko, శూన్య *ctx_ptr)
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
శూన్య k_thread_perms_all_clear(నిర్మాణం k_thread *thread)
{
	uintptr_t సూచీ = thread_సూచీ_get(thread);

	ఒకవేళ ((పూర్ణాంకం)సూచీ != -1) {
		k_object_wordlist_foreach(clear_perms_cb_locked, (శూన్య *)సూచీ);
	}
}

స్థిర పూర్ణాంకం thread_perms_test(నిర్మాణం k_object *ko)
{
	పూర్ణాంకం సూచీ;

	ఒకవేళ ((ko->flags & K_OBJ_FLAG_ప్రజా) != 0U) {
		తిరిగి 1;
	}

	సూచీ = thread_సూచీ_get(_current);
	ఒకవేళ (సూచీ != -1) {
		తిరిగి sys_bitfield_test_బిట్టు((mem_addr_t)&ko->perms, సూచీ);
	}
	తిరిగి 0;
}

స్థిర శూన్య dump_permission_error(నిర్మాణం k_object *ko)
{
	పూర్ణాంకం సూచీ = thread_సూచీ_get(_current);
	LOG_ERR("thread %p (%d) does not have permission on %s %p",
		_current, సూచీ,
		otype_గమ్యం_str(ko->type), ko->name);
	LOG_HEXDUMP_ERR(ko->perms, పరిమాణం(ko->perms), "permission bitmap");
}

శూన్య k_object_dump_error(పూర్ణాంకం retval, స్థిరరాశి శూన్య *obj, నిర్మాణం k_object *ko,
			గణన k_objects otype)
{
	ఎంపిక (retval) {
	స్థితి -EBADF:
		LOG_ERR("%p is not a valid %s", obj, otype_గమ్యం_str(otype));
		ఒకవేళ (ko == ఖాళీ) {
			LOG_ERR("address is not a known kernel object");
		} లేకపోతే {
			LOG_ERR("address is actually a %s",
				otype_గమ్యం_str(ko->type));
		}
		ఆపు;
	స్థితి -EPERM:
		dump_permission_error(ko);
		ఆపు;
	స్థితి -EINVAL:
		LOG_ERR("%p used before initialization", obj);
		ఆపు;
	స్థితి -EADDRINUSE:
		LOG_ERR("%p %s in use", obj, otype_గమ్యం_str(otype));
		ఆపు;
	అప్రమేయ:
		/* Not handled error */
		ఆపు;
	}
}

శూన్య z_impl_k_object_access_grant(స్థిరరాశి శూన్య *object, నిర్మాణం k_thread *thread)
{
	నిర్మాణం k_object *ko = k_object_కనుగొను(object);

	ఒకవేళ (ko != ఖాళీ) {
		k_thread_perms_సమితి(ko, thread);
	}
}

శూన్య k_object_access_revoke(స్థిరరాశి శూన్య *object, నిర్మాణం k_thread *thread)
{
	నిర్మాణం k_object *ko = k_object_కనుగొను(object);

	ఒకవేళ (ko != ఖాళీ) {
		k_thread_perms_clear(ko, thread);
	}
}

శూన్య k_object_access_revoke_others(స్థిరరాశి శూన్య *object)
{
	నిర్మాణం k_object *ko = k_object_కనుగొను(object);

	ఒకవేళ (ko != ఖాళీ) {
		(శూన్య)memset(ko->perms, 0, పరిమాణం(ko->perms));
		k_thread_perms_సమితి(ko, _current);
		ko->flags &= ~K_OBJ_FLAG_ప్రజా;
	}
}

శూన్య z_impl_k_object_release(స్థిరరాశి శూన్య *object)
{
	k_object_access_revoke(object, _current);
}

శూన్య k_object_access_all_grant(స్థిరరాశి శూన్య *object)
{
	నిర్మాణం k_object *ko = k_object_కనుగొను(object);

	ఒకవేళ (ko != ఖాళీ) {
		ko->flags |= K_OBJ_FLAG_ప్రజా;
	}
}

పూర్ణాంకం k_object_validate(నిర్మాణం k_object *ko, గణన k_objects otype,
		       గణన _obj_init_check init)
{
	ఒకవేళ (unlikely(ko == ఖాళీ)) {
		తిరిగి -EBADF;
	}

	ఒకవేళ (unlikely((otype != K_OBJ_ANY) && (otype != ko->type))) {
		ఒకవేళ ((otype != K_OBJ_DRIVER_ANY) ||
		    (ko->type < K_OBJ_DRIVER_మొదటి) ||
		    (ko->type > K_OBJ_DRIVER_చివరి)) {
			తిరిగి -EBADF;
		}
	}

	/* Manipulation of any kernel objects by a user thread requires that
	 * thread be granted access first, even for uninitialized objects
	 */
	ఒకవేళ (unlikely(thread_perms_test(ko) == 0)) {
		తిరిగి -EPERM;
	}

	/* Initialization state checks. _OBJ_INIT_ANY, we don't care */
	ఒకవేళ (likely(init == _OBJ_INIT_TRUE)) {
		/* Object MUST be initialized */
		ఒకవేళ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0U)) {
			తిరిగి -EINVAL;
		}
	} లేకపోతే ఒకవేళ (init == _OBJ_INIT_FALSE) { /* _OBJ_INIT_FALSE case */
		/* Object MUST NOT be initialized */
		ఒకవేళ (unlikely((ko->flags & K_OBJ_FLAG_INITIALIZED) != 0U)) {
			తిరిగి -EADDRINUSE;
		}
	} లేకపోతే {
		/* _OBJ_INIT_ANY */
	}

	తిరిగి 0;
}

పూర్ణాంకం z_impl_k_object_access_check(స్థిరరాశి శూన్య *object)
{
	తిరిగి k_object_validate(k_object_కనుగొను(object), K_OBJ_ANY, _OBJ_INIT_ANY);
}

శూన్య k_object_init(స్థిరరాశి శూన్య *obj)
{
	నిర్మాణం k_object *ko;

	/* By the time we get here, if the caller was from userspace, all the
	 * necessary checks have been done in k_object_validate(), which takes
	 * place before the object is initialized.
	 *
	 * This function runs after the object has been initialized and
	 * finalizes it
	 */

	ko = k_object_కనుగొను(obj);
	ఒకవేళ (ko == ఖాళీ) {
		/* Supervisor threads can ignore rules about kernel objects
		 * and may declare them on stacks, etc. Such objects will never
		 * be usable from userspace, but we shouldn't explode.
		 */
		తిరిగి;
	}

	/* Allows non-initialization system calls to be made on this object */
	ko->flags |= K_OBJ_FLAG_INITIALIZED;
}

శూన్య k_object_recycle(స్థిరరాశి శూన్య *obj)
{
	నిర్మాణం k_object *ko = k_object_కనుగొను(obj);

	ఒకవేళ (ko != ఖాళీ) {
		(శూన్య)memset(ko->perms, 0, పరిమాణం(ko->perms));
		k_thread_perms_సమితి(ko, _current);
		ko->flags |= K_OBJ_FLAG_INITIALIZED;
	}
}

శూన్య k_object_uninit(స్థిరరాశి శూన్య *obj)
{
	నిర్మాణం k_object *ko;

	/* See comments in k_object_init() */
	ko = k_object_కనుగొను(obj);
	ఒకవేళ (ko == ఖాళీ) {
		తిరిగి;
	}

	ko->flags &= ~K_OBJ_FLAG_INITIALIZED;
}

/*
 * Copy to/from helper functions used in syscall handlers
 */
శూన్య *k_usermode_alloc_from_copy(స్థిరరాశి శూన్య *src, size_t కొలత)
{
	శూన్య *dst = ఖాళీ;

	/* Does the caller in user mode have access to read this memory? */
	ఒకవేళ (K_SYSCALL_జ్ఞాపకం_READ(src, కొలత)) {
		వెళ్ళు బహిర్గతం_err;
	}

	dst = z_thread_malloc(కొలత);
	ఒకవేళ (dst == ఖాళీ) {
		LOG_ERR("out of thread resource pool memory (%zu)", కొలత);
		వెళ్ళు బహిర్గతం_err;
	}

	(శూన్య)జ్ఞాపకం_నకలు(dst, src, కొలత);
బహిర్గతం_err:
	తిరిగి dst;
}

స్థిర పూర్ణాంకం user_copy(శూన్య *dst, స్థిరరాశి శూన్య *src, size_t కొలత, bool గమ్యం_user)
{
	పూర్ణాంకం ret = EFAULT;

	/* Does the caller in user mode have access to this memory? */
	ఒకవేళ (గమ్యం_user ? K_SYSCALL_జ్ఞాపకం_WRITE(dst, కొలత) :
			K_SYSCALL_జ్ఞాపకం_READ(src, కొలత)) {
		వెళ్ళు బహిర్గతం_err;
	}

	(శూన్య)జ్ఞాపకం_నకలు(dst, src, కొలత);
	ret = 0;
బహిర్గతం_err:
	తిరిగి ret;
}

పూర్ణాంకం k_usermode_from_copy(శూన్య *dst, స్థిరరాశి శూన్య *src, size_t కొలత)
{
	తిరిగి user_copy(dst, src, కొలత, false);
}

పూర్ణాంకం k_usermode_గమ్యం_copy(శూన్య *dst, స్థిరరాశి శూన్య *src, size_t కొలత)
{
	తిరిగి user_copy(dst, src, కొలత, true);
}

అక్షరం *k_usermode_string_alloc_copy(స్థిరరాశి అక్షరం *src, size_t maxlen)
{
	size_t actual_len;
	పూర్ణాంకం err;
	అక్షరం *ret = ఖాళీ;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ఒకవేళ (err != 0) {
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ (కొలత_కూడు_అధికప్రవాహం(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		వెళ్ళు బహిర్గతం;
	}

	ret = k_usermode_alloc_from_copy(src, actual_len);

	/* Someone may have modified the source string during the above
	 * checks. Ensure what we actually copied is still terminated
	 * properly.
	 */
	ఒకవేళ (ret != ఖాళీ) {
		ret[actual_len - 1U] = '\0';
	}
బహిర్గతం:
	తిరిగి ret;
}

పూర్ణాంకం k_usermode_string_copy(అక్షరం *dst, స్థిరరాశి అక్షరం *src, size_t maxlen)
{
	size_t actual_len;
	పూర్ణాంకం ret, err;

	actual_len = k_usermode_string_nlen(src, maxlen, &err);
	ఒకవేళ (err != 0) {
		ret = EFAULT;
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ (actual_len == maxlen) {
		/* Not NULL terminated */
		LOG_ERR("string too long %p (%zu)", src, actual_len);
		ret = EINVAL;
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ (కొలత_కూడు_అధికప్రవాహం(actual_len, 1, &actual_len)) {
		LOG_ERR("overflow");
		ret = EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	ret = k_usermode_from_copy(dst, src, actual_len);

	/* See comment above in k_usermode_string_alloc_copy() */
	dst[actual_len - 1] = '\0';
బహిర్గతం:
	తిరిగి ret;
}

/*
 * Application memory region initialization
 */

బాహ్య అక్షరం __app_shmem_regions_start[];
బాహ్య అక్షరం __app_shmem_regions_end[];

స్థిర శూన్య app_shmem_bss_zero(శూన్య)
{
	నిర్మాణం z_app_region *region, *చివర;


	చివర = (నిర్మాణం z_app_region *)&__app_shmem_regions_end[0];
	region = (నిర్మాణం z_app_region *)&__app_shmem_regions_start[0];

	కొరకు ( ; region < చివర; region++) {
		(శూన్య)memset(region->bss_ప్రారంభం, 0, region->bss_కొలత);
	}
}

K_KERNEL_INIT_PRE(app_shmem_bss_zero);

/*
 * Default handlers if otherwise unimplemented
 */

స్థిర uintptr_t handler_bad_syscall(uintptr_t bad_id, uintptr_t arg2,
				     uintptr_t arg3, uintptr_t arg4,
				     uintptr_t arg5, uintptr_t arg6,
				     శూన్య *ssf)
{
	ARG_వాడని(arg2);
	ARG_వాడని(arg3);
	ARG_వాడని(arg4);
	ARG_వాడని(arg5);
	ARG_వాడని(arg6);

	LOG_ERR("Bad system call id %" PRIuPTR " invoked", bad_id);
	arch_syscall_oops(ssf);
	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}

స్థిర uintptr_t handler_no_syscall(uintptr_t arg1, uintptr_t arg2,
				    uintptr_t arg3, uintptr_t arg4,
				    uintptr_t arg5, uintptr_t arg6, శూన్య *ssf)
{
	ARG_వాడని(arg1);
	ARG_వాడని(arg2);
	ARG_వాడని(arg3);
	ARG_వాడని(arg4);
	ARG_వాడని(arg5);
	ARG_వాడని(arg6);

	LOG_ERR("Unimplemented system call");
	arch_syscall_oops(ssf);
	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}

#అన్తర్భావ <zephyr/syscall_dispatch.c>
