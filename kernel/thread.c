/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel thread support
 *
 * This module provides general purpose thread support.
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/clock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <timeout_q.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/tracing/tracing.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <zephyr/sys/check.h>
#అన్తర్భావ <zephyr/random/random.h>
#అన్తర్భావ <zephyr/sys/atomic.h>
#అన్తర్భావ <zephyr/logging/log.h>
#అన్తర్భావ <zephyr/llext/symbol.h>
#అన్తర్భావ <zephyr/sys/iterable_sections.h>
#అన్తర్భావ <zephyr/sys/minmax.h>

#అన్తర్భావ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#నిర్వచితమైతే CONFIG_OBJ_CORE_THREAD
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_THREAD
స్థిర నిర్మాణం k_obj_core_stats_desc  thread_stats_desc = {
	.raw_కొలత = పరిమాణం(నిర్మాణం k_చక్రం_stats),
	.query_కొలత = పరిమాణం(నిర్మాణం k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#లేకపోతే
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_THREAD */
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_THREAD */


#నిర్వచించు _FOREACH_STATIC_THREAD(thread_దత్తాంశం)              \
	STRUCT_SECTION_FOREACH(static_thread_దత్తాంశం, thread_దత్తాంశం)

bool k_is_లోపల_isr(శూన్య)
{
	తిరిగి arch_is_లోపల_isr();
}
EXPORT_చిహ్నం(k_is_లోపల_isr);

#నిర్వచితమైతే CONFIG_THREAD_CUSTOM_దత్తాంశం
శూన్య z_impl_k_thread_custom_దత్తాంశం_సమితి(శూన్య *విలువ)
{
	_current->custom_దత్తాంశం = విలువ;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_thread_custom_దత్తాంశం_సమితి(శూన్య *దత్తాంశం)
{
	z_impl_k_thread_custom_దత్తాంశం_సమితి(దత్తాంశం);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య *z_impl_k_thread_custom_దత్తాంశం_get(శూన్య)
{
	తిరిగి _current->custom_దత్తాంశం;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య *z_vrfy_k_thread_custom_దత్తాంశం_get(శూన్య)
{
	తిరిగి z_impl_k_thread_custom_దత్తాంశం_get();
}
#అన్తర్భావ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#అయితే_ముగింపు /* CONFIG_USERSPACE */
#అయితే_ముగింపు /* CONFIG_THREAD_CUSTOM_DATA */

పూర్ణాంకం z_impl_k_is_preempt_thread(శూన్య)
{
	తిరిగి !arch_is_లోపల_isr() && thread_is_preemptible(_current);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_is_preempt_thread(శూన్య)
{
	తిరిగి z_impl_k_is_preempt_thread();
}
#అన్తర్భావ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_thread_ప్రాధాన్యత_get(k_tid_t thread)
{
	తిరిగి thread->పునాది.prio;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_thread_ప్రాధాన్యత_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	తిరిగి z_impl_k_thread_ప్రాధాన్యత_get(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_THREAD_NAME
స్థిర శూన్య సమితి_thread_name(నిర్మాణం k_thread *thread, స్థిరరాశి అక్షరం *str)
{
	ఒకవేళ (str != ఖాళీ) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#నిర్వచితమైతే CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_సమితి(thread, str);
#అయితే_ముగింపు /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} లేకపోతే {
		thread->name[0] = '\0';
	}
}
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */

పూర్ణాంకం z_impl_k_thread_name_సమితి(k_tid_t thread, స్థిరరాశి అక్షరం *str)
{
#నిర్వచితమైతే CONFIG_THREAD_NAME
	ఒకవేళ (thread == ఖాళీ) {
		thread = _current;
	}

	సమితి_thread_name(thread, str);

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_thread, name_సమితి, thread, 0);

	తిరిగి 0;
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(str);

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_thread, name_సమితి, thread, -ENOSYS);

	తిరిగి -ENOSYS;
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_thread_name_సమితి(k_tid_t thread, స్థిరరాశి అక్షరం *str)
{
#నిర్వచితమైతే CONFIG_THREAD_NAME
	అక్షరం name[CONFIG_THREAD_MAX_NAME_LEN];

	ఒకవేళ (thread != ఖాళీ) {
		ఒకవేళ (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			తిరిగి -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	ఒకవేళ (k_usermode_string_copy(name, str, పరిమాణం(name)) != 0) {
		తిరిగి -EFAULT;
	}

	తిరిగి z_impl_k_thread_name_సమితి(thread, name);
#లేకపోతే
	తిరిగి -ENOSYS;
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */
}
#అన్తర్భావ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

స్థిరరాశి అక్షరం *k_thread_name_get(k_tid_t thread)
{
#నిర్వచితమైతే CONFIG_THREAD_NAME
	తిరిగి (స్థిరరాశి అక్షరం *)thread->name;
#లేకపోతే
	ARG_వాడని(thread);
	తిరిగి ఖాళీ;
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */
}

పూర్ణాంకం z_impl_k_thread_name_copy(k_tid_t thread, అక్షరం *buf, size_t కొలత)
{
#నిర్వచితమైతే CONFIG_THREAD_NAME
	strncpy(buf, thread->name, కొలత);
	తిరిగి 0;
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(buf);
	ARG_వాడని(కొలత);
	తిరిగి -ENOSYS;
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */
}

స్థిర size_t copy_బైట్లు(అక్షరం *dest, size_t dest_కొలత, స్థిరరాశి అక్షరం *src, size_t src_కొలత)
{
	size_t  బైట్లు_గమ్యం_copy;

	బైట్లు_గమ్యం_copy = min(dest_కొలత, src_కొలత);
	జ్ఞాపకం_నకలు(dest, src, బైట్లు_గమ్యం_copy);

	తిరిగి బైట్లు_గమ్యం_copy;
}

స్థిరరాశి అక్షరం *k_thread_state_str(k_tid_t thread_id, అక్షరం *buf, size_t buf_కొలత)
{
	size_t      off = 0;
	uint8_t     బిట్టు_2;
	uint8_t     thread_state = thread_id->పునాది.thread_state;
#నిర్వచించు SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, పరిమాణం(Z_STATE_STR_##s) - 1 }
	స్థిర స్థిరరాశి నిర్మాణం {
		స్థిరరాశి అక్షరం *str;
		uint16_t    బిట్టు_2;
		uint16_t    len;
	} state_string[] = {
		SS_ENT(DUMMY),
		SS_ENT(PENDING),
		SS_ENT(SLEEPING),
		SS_ENT(DEAD),
		SS_ENT(SUSPENDED),
		SS_ENT(ABORTING),
		SS_ENT(SUSPENDING),
		SS_ENT(QUEUED),
	};
#నిర్వచనం_రద్దు SS_ENT

	ఒకవేళ ((buf == ఖాళీ) || (buf_కొలత == 0)) {
		తిరిగి "";
	}

	buf_కొలత--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	కొరకు (గుర్తులేని పూర్ణాంకం సూచీ = 0; thread_state != 0; సూచీ++) {
		బిట్టు_2 = state_string[సూచీ].బిట్టు_2;
		ఒకవేళ ((thread_state & బిట్టు_2) == 0) {
			కొనసాగు;
		}

		off += copy_బైట్లు(buf + off, buf_కొలత - off,
				  state_string[సూచీ].str,
				  state_string[సూచీ].len);

		thread_state &= ~బిట్టు_2;

		ఒకవేళ (thread_state != 0) {
			off += copy_బైట్లు(buf + off, buf_కొలత - off, "+", 1);
		}
	}

	buf[off] = '\0';

	తిరిగి (స్థిరరాశి అక్షరం *)buf;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_thread_name_copy(k_tid_t thread,
					    అక్షరం *buf, size_t కొలత)
{
#నిర్వచితమైతే CONFIG_THREAD_NAME
	size_t len;
	నిర్మాణం k_object *ko = k_object_కనుగొను(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	ఒకవేళ ((ko == ఖాళీ) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		తిరిగి -EINVAL;
	}
	ఒకవేళ (K_SYSCALL_జ్ఞాపకం_WRITE(buf, కొలత) != 0) {
		తిరిగి -EFAULT;
	}
	len = అక్షరశ్రేణి_పొడవు(thread->name);
	ఒకవేళ ((len + 1) > కొలత) {
		తిరిగి -ENOSPC;
	}

	తిరిగి k_usermode_గమ్యం_copy((శూన్య *)buf, thread->name, len + 1);
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(buf);
	ARG_వాడని(కొలత);
	తిరిగి -ENOSYS;
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */
}
#అన్తర్భావ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_దొంతర_SENTINEL
/* Check that the stack sentinel is still present
 *
 * The stack sentinel feature writes a magic value to the lowest 4 bytes of
 * the thread's stack when the thread is initialized. This value gets checked
 * in a few places:
 *
 * 1) In k_yield() if the current thread is not swapped out
 * 2) After servicing a non-nested interrupt
 * 3) In z_swap(), check the sentinel in the outgoing thread
 *
 * Item 2 requires support in arch/ code.
 *
 * If the check fails, the thread will be terminated appropriately through
 * the system fatal error handler.
 */
శూన్య z_check_దొంతర_sentinel(శూన్య)
{
	uint32_t *దొంతర;

	ఒకవేళ ((_current->పునాది.thread_state & _THREAD_DUMMY) != 0) {
		తిరిగి;
	}

	దొంతర = (uint32_t *)_current->దొంతర_info.ప్రారంభం;
	ఒకవేళ (*దొంతర != దొంతర_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*దొంతర = దొంతర_SENTINEL;
		z_except_reason(K_ERR_దొంతర_CHK_FAIL);
	}
}
#అయితే_ముగింపు /* CONFIG_STACK_SENTINEL */

#ఒకవేళ defined(CONFIG_దొంతర_సూచిక_RANDOM) && (CONFIG_దొంతర_సూచిక_RANDOM != 0)
పూర్ణాంకం z_దొంతర_adjust_initialized;

స్థిర size_t random_స్థానభ్రంశం(size_t దొంతర_కొలత)
{
	size_t random_val;

	ఒకవేళ (!z_దొంతర_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, పరిమాణం(random_val));
	} లేకపోతే {
		sys_rand_get((uint8_t *)&random_val, పరిమాణం(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	స్థిరరాశి size_t fuzz = random_val % CONFIG_దొంతర_సూచిక_RANDOM;

	ఒకవేళ (unlikely(fuzz * 2 > దొంతర_కొలత)) {
		తిరిగి 0;
	}

	తిరిగి fuzz;
}
#ఒకవేళ defined(CONFIG_దొంతర_GROWS_పైకి)
	/* This is so rare not bothering for now */
#దోషం "Stack pointer randomization not implemented for upward growing stacks"
#అయితే_ముగింపు /* CONFIG_STACK_GROWS_UP */
#అయితే_ముగింపు /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
నిర్మాణం దొంతర_geometry {
	size_t obj_కొలత;	/* size of the whole stack object */
	అక్షరం *buf_ప్రారంభం;	/* start of the usable stack buffer */
	size_t buf_కొలత;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
స్థిర నిర్మాణం దొంతర_geometry compute_దొంతర_geometry(k_thread_దొంతర_t *దొంతర,
						    size_t దొంతర_కొలత)
{
	నిర్మాణం దొంతర_geometry geo;

#నిర్వచితమైతే CONFIG_USERSPACE
	ఒకవేళ (z_దొంతర_is_user_capable(దొంతర)) {
		geo.obj_కొలత = K_THREAD_దొంతర_LEN(దొంతర_కొలత);
		geo.buf_ప్రారంభం = K_THREAD_దొంతర_బఫరు(దొంతర);
		geo.buf_కొలత = geo.obj_కొలత - K_THREAD_దొంతర_RESERVED;
	} లేకపోతే
#అయితే_ముగింపు /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_కొలత = K_KERNEL_దొంతర_LEN(దొంతర_కొలత);
		geo.buf_ప్రారంభం = K_KERNEL_దొంతర_బఫరు(దొంతర);
		geo.buf_కొలత = geo.obj_కొలత - K_KERNEL_దొంతర_RESERVED;

#ఒకవేళ defined(ARCH_KERNEL_దొంతర_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		ఒకవేళ (K_KERNEL_దొంతర_RESERVED > geo.obj_కొలత) {
			k_panic();
		}
#అయితే_ముగింపు
	}

	తిరిగి geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
స్థిర అక్షరం *map_thread_దొంతర(నిర్మాణం k_thread *new_thread,
			      k_thread_దొంతర_t *దొంతర,
			      నిర్మాణం దొంతర_geometry *geo)
{
	అక్షరం *దొంతర_ptr;

#నిర్వచితమైతే CONFIG_THREAD_దొంతర_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	శూన్య *దొంతర_mapped = k_mem_map_phys_guard((uintptr_t)దొంతర, geo->obj_కొలత,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)దొంతర_mapped != 0);

#నిర్వచితమైతే CONFIG_USERSPACE
	ఒకవేళ (z_దొంతర_is_user_capable(దొంతర)) {
		geo->buf_ప్రారంభం = K_THREAD_దొంతర_బఫరు(దొంతర_mapped);
	} లేకపోతే
#అయితే_ముగింపు /* CONFIG_USERSPACE */
	{
		geo->buf_ప్రారంభం = K_KERNEL_దొంతర_బఫరు(దొంతర_mapped);
	}

	దొంతర_ptr = (అక్షరం *)దొంతర_mapped + geo->obj_కొలత;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->దొంతర_info.mapped.addr = దొంతర_mapped;
	new_thread->దొంతర_info.mapped.sz = geo->obj_కొలత;
#లేకపోతే /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_వాడని(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	దొంతర_ptr = (అక్షరం *)దొంతర + geo->obj_కొలత;
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_MEM_MAPPED */

	తిరిగి దొంతర_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
స్థిర అంతఃవరుస శూన్య fill_init_దొంతర(అక్షరం *buf_ప్రారంభం, size_t buf_కొలత)
{
#నిర్వచితమైతే CONFIG_INIT_STACKS
	memset(buf_ప్రారంభం, 0xaa, buf_కొలత);
#లేకపోతే
	ARG_వాడని(buf_ప్రారంభం);
	ARG_వాడని(buf_కొలత);
#అయితే_ముగింపు /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
స్థిర అంతఃవరుస శూన్య సమితి_దొంతర_sentinel(అక్షరం *buf_ప్రారంభం)
{
#నిర్వచితమైతే CONFIG_దొంతర_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_ప్రారంభం) = దొంతర_SENTINEL;
#లేకపోతే
	ARG_వాడని(buf_ప్రారంభం);
#అయితే_ముగింపు /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
స్థిర size_t reserve_దొంతర_headroom(నిర్మాణం k_thread *new_thread,
				     అక్షరం *దొంతర_ptr, size_t buf_కొలత)
{
	size_t delta = 0;

	ARG_వాడని(new_thread);
	ARG_వాడని(దొంతర_ptr);
	ARG_వాడని(buf_కొలత);

#నిర్వచితమైతే CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_దొంతర_అమరిక(new_thread, దొంతర_ptr);
#అయితే_ముగింపు /* CONFIG_THREAD_LOCAL_STORAGE */
#నిర్వచితమైతే CONFIG_THREAD_USERSPACE_LOCAL_దత్తాంశం
	size_t tls_కొలత = పరిమాణం(నిర్మాణం thread_userspace_local_దత్తాంశం);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_కొలత;
	new_thread->userspace_local_దత్తాంశం =
		(నిర్మాణం thread_userspace_local_దత్తాంశం *)(దొంతర_ptr - delta);
#అయితే_ముగింపు /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ఒకవేళ defined(CONFIG_దొంతర_సూచిక_RANDOM) && (CONFIG_దొంతర_సూచిక_RANDOM != 0)
	delta += random_స్థానభ్రంశం(buf_కొలత);
#అయితే_ముగింపు /* CONFIG_STACK_POINTER_RANDOM */

	తిరిగి విడత_పైకి(delta, ARCH_దొంతర_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
స్థిర అంతఃవరుస శూన్య సమితి_దొంతర_info(నిర్మాణం k_thread *new_thread, అక్షరం *buf_ప్రారంభం,
				  size_t buf_కొలత, size_t delta)
{
#నిర్వచితమైతే CONFIG_THREAD_దొంతర_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->దొంతర_info.ప్రారంభం = (uintptr_t)buf_ప్రారంభం;
	new_thread->దొంతర_info.కొలత = buf_కొలత;
	new_thread->దొంతర_info.delta = delta;

#నిర్వచితమైతే CONFIG_THREAD_RUNTIME_దొంతర_SAFETY
	new_thread->దొంతర_info.usage.వాడని_పరిమితి =
		(CONFIG_THREAD_RUNTIME_దొంతర_SAFETY_DEFAULT_వాడని_పరిమితి_PCT *
		 buf_కొలత) / 100;
#అయితే_ముగింపు
#లేకపోతే
	ARG_వాడని(new_thread);
	ARG_వాడని(buf_ప్రారంభం);
	ARG_వాడని(buf_కొలత);
	ARG_వాడని(delta);
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_INFO */
}

స్థిర అక్షరం *అమరిక_thread_దొంతర(నిర్మాణం k_thread *new_thread,
				k_thread_దొంతర_t *దొంతర, size_t దొంతర_కొలత)
{
	నిర్మాణం దొంతర_geometry geo = compute_దొంతర_geometry(దొంతర, దొంతర_కొలత);
	అక్షరం *దొంతర_ptr = map_thread_దొంతర(new_thread, దొంతర, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		దొంతర, new_thread, geo.obj_కొలత, (శూన్య *)geo.buf_ప్రారంభం,
		geo.buf_కొలత, (శూన్య *)దొంతర_ptr);

	fill_init_దొంతర(geo.buf_ప్రారంభం, geo.buf_కొలత);
	సమితి_దొంతర_sentinel(geo.buf_ప్రారంభం);
	delta = reserve_దొంతర_headroom(new_thread, దొంతర_ptr, geo.buf_కొలత);
	సమితి_దొంతర_info(new_thread, geo.buf_ప్రారంభం, geo.buf_కొలత, delta);

	తిరిగి దొంతర_ptr - delta;
}

#నిర్వచితమైతే CONFIG_HW_SHADOW_దొంతర
స్థిర శూన్య అమరిక_shadow_దొంతర(నిర్మాణం k_thread *new_thread,
			 k_thread_దొంతర_t *దొంతర)
{
	పూర్ణాంకం ret = -ENOENT;

	STRUCT_SECTION_FOREACH(దొంతర_గమ్యం_hw_shadow_దొంతర, stk_గమ్యం_hw_shstk) {
		ఒకవేళ (stk_గమ్యం_hw_shstk->దొంతర == దొంతర) {
			ret = k_thread_hw_shadow_దొంతర_attach(new_thread,
							      stk_గమ్యం_hw_shstk->shstk_addr,
							      stk_గమ్యం_hw_shstk->కొలత);
			ఒకవేళ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_గమ్యం_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ఆపు;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	ఒకవేళ (ret != -ENOENT) {
		తిరిగి;
	}

	STRUCT_SECTION_FOREACH(దొంతర_గమ్యం_hw_shadow_దొంతర_arr, stk_గమ్యం_hw_shstk) {
		ఒకవేళ ((uintptr_t)దొంతర >= stk_గమ్యం_hw_shstk->దొంతర_addr &&
		    (uintptr_t)దొంతర < stk_గమ్యం_hw_shstk->దొంతర_addr +
		    stk_గమ్యం_hw_shstk->దొంతర_కొలత * stk_గమ్యం_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t దొంతర_స్థానభ్రంశం = (uintptr_t)దొంతర - stk_గమ్యం_hw_shstk->దొంతర_addr;
			uintptr_t దొంతర_సూచీ = దొంతర_స్థానభ్రంశం / stk_గమ్యం_hw_shstk->దొంతర_కొలత;
			uintptr_t addr;

			ఒకవేళ (దొంతర_సూచీ >= stk_గమ్యం_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, దొంతర);
				k_panic();
			}

			addr = stk_గమ్యం_hw_shstk->shstk_addr +
				stk_గమ్యం_hw_shstk->shstk_కొలత * దొంతర_సూచీ;
			ret = k_thread_hw_shadow_దొంతర_attach(new_thread,
							      (arch_thread_hw_shadow_దొంతర_t *)addr,
							      stk_గమ్యం_hw_shstk->shstk_కొలత);
			ఒకవేళ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_గమ్యం_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ఆపు;
		}
	}

	ఒకవేళ (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, దొంతర);
		k_panic();
	}
}
#లేకపోతే
/* No-op stub used when hardware shadow stacks are not configured. */
స్థిర అంతఃవరుస శూన్య అమరిక_shadow_దొంతర(నిర్మాణం k_thread *new_thread,
				      k_thread_దొంతర_t *దొంతర)
{
	ARG_వాడని(new_thread);
	ARG_వాడని(దొంతర);
}
#అయితే_ముగింపు

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
స్థిర అంతఃవరుస శూన్య thread_abort_cleanup_check_reuse(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
స్థిర అంతఃవరుస శూన్య init_thread_obj_core(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_రిజిస్టరు(K_OBJ_CORE(thread), &thread->పునాది.usage,
				  పరిమాణం(thread->పునాది.usage));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_THREAD */
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
స్థిర అంతఃవరుస శూన్య init_thread_userspace(నిర్మాణం k_thread *thread,
					 k_thread_దొంతర_t *దొంతర,
					 uint32_t options)
{
#నిర్వచితమైతే CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_దొంతర_is_user_capable(దొంతర),
		 "user thread %p with kernel-only stack %p", thread, దొంతర);
	k_object_init(thread);
	k_object_init(దొంతర);
	thread->దొంతర_obj = దొంతర;
	thread->syscall_చట్రం = ఖాళీ;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(దొంతర);
	ARG_వాడని(options);
#అయితే_ముగింపు /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
స్థిర అంతఃవరుస శూన్య assert_thread_coherence(నిర్మాణం k_thread *thread,
					   k_thread_దొంతర_t *దొంతర)
{
#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#నిర్వచితం_కాకపోతే CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(దొంతర));
#అయితే_ముగింపు /* CONFIG_DYNAMIC_THREAD */
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */
	ARG_వాడని(thread);
	ARG_వాడని(దొంతర);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
స్థిర అంతఃవరుస శూన్య assert_switch_handle(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != ఖాళీ,
		 "arch layer failed to initialize switch_handle");
#అయితే_ముగింపు /* CONFIG_USE_SWITCH */
	ARG_వాడని(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
స్థిర అంతఃవరుస శూన్య init_thread_custom_దత్తాంశం(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_THREAD_CUSTOM_దత్తాంశం
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_దత్తాంశం = ఖాళీ;
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
స్థిర అంతఃవరుస శూన్య కూడు_thread_గమ్యం_monitor(నిర్మాణం k_thread *thread,
					 k_thread_నమోదు_t నమోదు_2,
					 శూన్య *p1, శూన్య *p2, శూన్య *p3)
{
#నిర్వచితమైతే CONFIG_THREAD_MONITOR
	thread->నమోదు_2.p_నమోదు = నమోదు_2;
	thread->నమోదు_2.parameter1 = p1;
	thread->నమోదు_2.parameter2 = p2;
	thread->నమోదు_2.parameter3 = p3;

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&z_thread_monitor_తాళం);

	thread->తదుపరి_thread = _kernel.దారాలు;
	_kernel.దారాలు = thread;
	k_spin_unlock(&z_thread_monitor_తాళం, కీలకం);
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(నమోదు_2);
	ARG_వాడని(p1);
	ARG_వాడని(p2);
	ARG_వాడని(p3);
#అయితే_ముగింపు /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
స్థిర అంతఃవరుస శూన్య init_thread_name(నిర్మాణం k_thread *thread, స్థిరరాశి అక్షరం *name)
{
#నిర్వచితమైతే CONFIG_THREAD_NAME
	సమితి_thread_name(thread, name);
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(name);
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
స్థిర అంతఃవరుస శూన్య init_thread_cpu_ముసుగు(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు
	ఒకవేళ (IS_ENABLED(CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY)) {
		thread->పునాది.cpu_ముసుగు = 1; /* must specify only one cpu */
	} లేకపోతే {
		thread->పునాది.cpu_ముసుగు = -1; /* allow all cpus */
	}
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
స్థిర అంతఃవరుస శూన్య init_thread_userspace_perms(నిర్మాణం k_thread *thread,
					       uint32_t options)
{
#నిర్వచితమైతే CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	ఒకవేళ ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(options);
#అయితే_ముగింపు /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
స్థిర అంతఃవరుస శూన్య init_thread_deadline(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SCHED_DEADLINE
	thread->పునాది.prio_deadline = 0;
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
స్థిర అంతఃవరుస శూన్య init_thread_halt_వరుస(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SMP
	z_waitq_init(&thread->halt_వరుస);
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
స్థిర అంతఃవరుస శూన్య init_thread_usage(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE
	thread->పునాది.usage = (నిర్మాణం k_చక్రం_stats) {};
	thread->పునాది.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
స్థిర అంతఃవరుస శూన్య assert_thread_not_reused(నిర్మాణం k_thread *new_thread)
{
#నిర్వచితమైతే CONFIG_THREAD_MONITOR
#నిర్వచితమైతే CONFIG_ASSERT
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&z_thread_monitor_తాళం);
	bool reused = false;

	/* Check if the thread is already in the list */
	కొరకు (నిర్మాణం k_thread *t = _kernel.దారాలు; t; t = t->తదుపరి_thread) {
		ఒకవేళ (t == new_thread) {
			reused = true;
			ఆపు;
		}
	}

	k_spin_unlock(&z_thread_monitor_తాళం, కీలకం);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#అయితే_ముగింపు /* CONFIG_ASSERT */
#అయితే_ముగింపు /* CONFIG_THREAD_MONITOR */
	ARG_వాడని(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
అక్షరం *z_అమరిక_new_thread(నిర్మాణం k_thread *new_thread,
			 k_thread_దొంతర_t *దొంతర, size_t దొంతర_కొలత,
			 k_thread_నమోదు_t నమోదు_2,
			 శూన్య *p1, శూన్య *p2, శూన్య *p3,
			 పూర్ణాంకం prio, uint32_t options, స్థిరరాశి అక్షరం *name)
{
	అక్షరం *దొంతర_ptr;

	Z_ASSERT_VALID_PRIO(prio, నమోదు_2);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, దొంతర, options);

	z_waitq_init(&new_thread->join_వరుస);

	/* Initialize various struct k_thread members */
	z_init_thread_పునాది(&new_thread->పునాది, prio, _THREAD_SLEEPING, options);
	దొంతర_ptr = అమరిక_thread_దొంతర(new_thread, దొంతర, దొంతర_కొలత);

#ఒకవేళ Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = ఖాళీ;
	new_thread->orig_prio = new_thread->పునాది.prio;
#అయితే_ముగింపు /* Z_MUTEX_PI_ENABLED */

	అమరిక_shadow_దొంతర(new_thread, దొంతర);
	assert_thread_coherence(new_thread, దొంతర);

	arch_new_thread(new_thread, దొంతర, దొంతర_ptr, నమోదు_2, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_దత్తాంశం = ఖాళీ;

	assert_switch_handle(new_thread);
	init_thread_custom_దత్తాంశం(new_thread);
	కూడు_thread_గమ్యం_monitor(new_thread, నమోదు_2, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_ముసుగు(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	ఒకవేళ (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_మార్పిడి_గమ్యం_MAIN) &&
	    (_current == ఖాళీ)) {
		new_thread->resource_pool = ఖాళీ;
		తిరిగి దొంతర_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_వరుస(new_thread);
	init_thread_usage(new_thread);

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	తిరిగి దొంతర_ptr;
}

#నిర్వచితమైతే CONFIG_THREAD_RUNTIME_దొంతర_SAFETY
పూర్ణాంకం z_impl_k_thread_runtime_దొంతర_వాడని_పరిమితి_pct_సమితి(నిర్మాణం k_thread *thread,
							   uint32_t pct)
{
	size_t వాడని_పరిమితి;

	ఒకవేళ (pct > 99) {
		తిరిగి -EINVAL;   /* 100% unused stack and up is invalid */
	}

	వాడని_పరిమితి = (thread->దొంతర_info.కొలత * pct) / 100;

	thread->దొంతర_info.usage.వాడని_పరిమితి = వాడని_పరిమితి;

	తిరిగి 0;
}

పూర్ణాంకం z_impl_k_thread_runtime_దొంతర_వాడని_పరిమితి_సమితి(నిర్మాణం k_thread *thread,
						       size_t పరిమితి)
{
	ఒకవేళ (పరిమితి > thread->దొంతర_info.కొలత) {
		తిరిగి -EINVAL;
	}

	thread->దొంతర_info.usage.వాడని_పరిమితి = పరిమితి;

	తిరిగి 0;
}

size_t z_impl_k_thread_runtime_దొంతర_వాడని_పరిమితి_get(నిర్మాణం k_thread *thread)
{
	తిరిగి thread->దొంతర_info.usage.వాడని_పరిమితి;
}

#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_thread_runtime_దొంతర_వాడని_పరిమితి_pct_సమితి(నిర్మాణం k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	తిరిగి z_impl_k_thread_runtime_దొంతర_వాడని_పరిమితి_pct_సమితి(thread, pct);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

పూర్ణాంకం z_vrfy_k_thread_runtime_దొంతర_వాడని_పరిమితి_సమితి(నిర్మాణం k_thread *thread,
						       size_t పరిమితి)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	తిరిగి z_impl_k_thread_runtime_దొంతర_వాడని_పరిమితి_సమితి(thread, పరిమితి);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_దొంతర_వాడని_పరిమితి_get(నిర్మాణం k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	తిరిగి z_impl_k_thread_runtime_దొంతర_వాడని_పరిమితి_get(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
#అయితే_ముగింపు /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(నిర్మాణం k_thread *new_thread,
			      k_thread_దొంతర_t *దొంతర,
			      size_t దొంతర_కొలత, k_thread_నమోదు_t నమోదు_2,
			      శూన్య *p1, శూన్య *p2, శూన్య *p3,
			      పూర్ణాంకం prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_లోపల_isr(), "Threads may not be created in ISRs");

	z_అమరిక_new_thread(new_thread, దొంతర, దొంతర_కొలత, నమోదు_2, p1, p2, p3,
			  prio, options, ఖాళీ);

	thread_కాలపట్టిక_new(new_thread, delay);

	తిరిగి new_thread;
}

#నిర్వచితమైతే CONFIG_USERSPACE
bool z_దొంతర_is_user_capable(k_thread_దొంతర_t *దొంతర)
{
	తిరిగి k_object_కనుగొను(దొంతర) != ఖాళీ;
}

k_tid_t z_vrfy_k_thread_create(నిర్మాణం k_thread *new_thread,
			       k_thread_దొంతర_t *దొంతర,
			       size_t దొంతర_కొలత, k_thread_నమోదు_t నమోదు_2,
			       శూన్య *p1, శూన్య *p2, శూన్య *p3,
			       పూర్ణాంకం prio, uint32_t options, k_timeout_t delay)
{
	size_t మొత్తం_కొలత, దొంతర_obj_కొలత;
	నిర్మాణం k_object *దొంతర_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	దొంతర_object = k_object_కనుగొను(దొంతర);
	K_OOPS(K_SYSCALL_VERIFY_సందేశం(k_object_validation_check(దొంతర_object, దొంతర,
						K_OBJ_THREAD_దొంతర_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_సందేశం(!కొలత_కూడు_అధికప్రవాహం(K_THREAD_దొంతర_RESERVED,
						       దొంతర_కొలత, &మొత్తం_కొలత),
				    "stack size overflow (%zu+%zu)",
				    దొంతర_కొలత,
				    K_THREAD_దొంతర_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#నిర్వచితమైతే CONFIG_GEN_PRIV_STACKS
	దొంతర_obj_కొలత = దొంతర_object->దత్తాంశం.దొంతర_దత్తాంశం->కొలత;
#లేకపోతే
	దొంతర_obj_కొలత = దొంతర_object->దత్తాంశం.దొంతర_కొలత;
#అయితే_ముగింపు /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_సందేశం(మొత్తం_కొలత <= దొంతర_obj_కొలత,
				    "stack size %zu is too big, max is %zu",
				    మొత్తం_కొలత, దొంతర_obj_కొలత));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, ఖాళీ)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_క్రింది_or_equal(prio,
							_current->పునాది.prio)));

	z_అమరిక_new_thread(new_thread, దొంతర, దొంతర_కొలత,
			   నమోదు_2, p1, p2, p3, prio, options, ఖాళీ);

	thread_కాలపట్టిక_new(new_thread, delay);

	తిరిగి new_thread;
}
#అన్తర్భావ <zephyr/syscalls/k_thread_create_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_init_thread_పునాది(నిర్మాణం thread_పునాది *thread_పునాది_2, పూర్ణాంకం ప్రాధాన్యత,
		       uint32_t initial_state, గుర్తులేని పూర్ణాంకం options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_పునాది_2->pended_on = ఖాళీ;
	thread_పునాది_2->user_options = (uint16_t)options;
	thread_పునాది_2->thread_state = (uint8_t)initial_state;

	thread_పునాది_2->prio = ప్రాధాన్యత;

	thread_పునాది_2->sched_locked = 0U;

#నిర్వచితమైతే CONFIG_SMP
	thread_పునాది_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_పునాది_2->cpu = 0;
#అయితే_ముగింపు /* CONFIG_SMP */

#నిర్వచితమైతే CONFIG_TIMESLICE_చొప్పున_THREAD
	thread_పునాది_2->ముక్క_ticks = 0;
	thread_పునాది_2->ముక్క_expired = ఖాళీ;
#అయితే_ముగింపు /* CONFIG_TIMESLICE_PER_THREAD */

#నిర్వచితమైతే CONFIG_SPIN_VALIDATE
	thread_పునాది_2->మార్పిడి_దత్తాంశం = ఖాళీ;
	/* otherwise swap_data does not need to be initialized */
#అయితే_ముగింపు

	z_init_thread_timeout(thread_పునాది_2);
}

FUNC_NORETURN శూన్య k_thread_user_బహుళకం_enter(k_thread_నమోదు_t నమోదు_2,
					    శూన్య *p1, శూన్య *p2, శూన్య *p3)
{
	SYS_పోర్టు_TRACING_FUNC(k_thread, user_బహుళకం_enter);

	_current->పునాది.user_options |= K_USER;
	z_thread_essential_clear(_current);
#నిర్వచితమైతే CONFIG_THREAD_MONITOR
	_current->నమోదు_2.p_నమోదు = నమోదు_2;
	_current->నమోదు_2.parameter1 = p1;
	_current->నమోదు_2.parameter2 = p2;
	_current->నమోదు_2.parameter3 = p3;
#అయితే_ముగింపు /* CONFIG_THREAD_MONITOR */
#నిర్వచితమైతే CONFIG_USERSPACE
	__ASSERT(z_దొంతర_is_user_capable(_current->దొంతర_obj),
		 "dropping to user mode with kernel-only stack object");
#నిర్వచితమైతే CONFIG_THREAD_USERSPACE_LOCAL_దత్తాంశం
	memset(_current->userspace_local_దత్తాంశం, 0,
	       పరిమాణం(నిర్మాణం thread_userspace_local_దత్తాంశం));
#అయితే_ముగింపు /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#నిర్వచితమైతే CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_దొంతర_అమరిక(_current,
			     (అక్షరం *)(_current->దొంతర_info.ప్రారంభం +
				      _current->దొంతర_info.కొలత));
#అయితే_ముగింపు /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_బహుళకం_enter(నమోదు_2, p1, p2, p3);
#లేకపోతే
	/* XXX In this case we do not reset the stack */
	z_thread_నమోదు(నమోదు_2, p1, p2, p3);
#అయితే_ముగింపు /* CONFIG_USERSPACE */
}

#ఒకవేళ defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_దొంతర_INFO)
#నిర్వచితమైతే CONFIG_దొంతర_GROWS_పైకి
#దోషం "Unsupported configuration for stack analysis"
#అయితే_ముగింపు /* CONFIG_STACK_GROWS_UP */

పూర్ణాంకం z_దొంతర_space_get(స్థిరరాశి uint8_t *దొంతర_ప్రారంభం, size_t కొలత, size_t *వాడని_ptr)
{
	size_t వాడని = 0;
	స్థిరరాశి uint8_t *checked_దొంతర = దొంతర_ప్రారంభం;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	స్థిరరాశి uint8_t *దొంతర_సూచిక = (స్థిరరాశి uint8_t *)&దొంతర_ప్రారంభం;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	ఒకవేళ ((దొంతర_సూచిక > దొంతర_ప్రారంభం) && (దొంతర_సూచిక <= (దొంతర_ప్రారంభం + కొలత)) &&
	    IS_ENABLED(CONFIG_NO_వాడని_దొంతర_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		తిరిగి -ENOTSUP;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_దొంతర_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_దొంతర += 4;
		కొలత -= 4;
	}

	కొరకు (size_t i = 0; i < కొలత; i++) {
		ఒకవేళ ((checked_దొంతర[i]) == 0xaaU) {
			వాడని++;
		} లేకపోతే {
			ఆపు;
		}
	}

	*వాడని_ptr = వాడని;

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_THREAD_RUNTIME_దొంతర_SAFETY
పూర్ణాంకం k_thread_runtime_దొంతర_safety_నిండిన_check(స్థిరరాశి నిర్మాణం k_thread *thread,
					     size_t *వాడని_ptr,
					     k_thread_దొంతర_safety_handler_t handler,
					     శూన్య *arg)
{
	పూర్ణాంకం    rv;
	size_t వాడని_space;

	__ASSERT_NO_MSG(thread != ఖాళీ);

	rv = z_దొంతర_space_get((స్థిరరాశి uint8_t *)thread->దొంతర_info.ప్రారంభం,
			       thread->దొంతర_info.కొలత, &వాడని_space);

	ఒకవేళ (rv != 0) {
		తిరిగి rv;
	}

	ఒకవేళ (వాడని_ptr != ఖాళీ) {
		*వాడని_ptr = వాడని_space;
	}

	ఒకవేళ ((వాడని_space < thread->దొంతర_info.usage.వాడని_పరిమితి) &&
	    (handler != ఖాళీ)) {
		handler(thread, వాడని_space, arg);
	}

	తిరిగి 0;
}

పూర్ణాంకం k_thread_runtime_దొంతర_safety_పరిమితి_check(స్థిరరాశి నిర్మాణం k_thread *thread,
						  size_t *వాడని_ptr,
						  k_thread_దొంతర_safety_handler_t handler,
						  శూన్య *arg)
{
	పూర్ణాంకం    rv;
	size_t వాడని_space;
	size_t పరిమితి;
	size_t పరిశీలన_కొలత;

	__ASSERT_NO_MSG(thread != ఖాళీ);

	పరిమితి = thread->దొంతర_info.usage.వాడని_పరిమితి;
	పరిశీలన_కొలత = పరిమితి;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_దొంతర_SENTINEL) && (పరిమితి != 0U)) {
		పరిశీలన_కొలత += పరిమాణం(uint32_t);
	}

	rv = z_దొంతర_space_get((స్థిరరాశి uint8_t *)thread->దొంతర_info.ప్రారంభం,
			       పరిశీలన_కొలత, &వాడని_space);

	ఒకవేళ (rv != 0) {
		తిరిగి rv;
	}

	ఒకవేళ (వాడని_ptr != ఖాళీ) {
		*వాడని_ptr = వాడని_space;
	}

	ఒకవేళ ((వాడని_space < పరిమితి) && (handler != ఖాళీ)) {
		handler(thread, వాడని_space, arg);
	}

	తిరిగి 0;
}
#అయితే_ముగింపు /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

పూర్ణాంకం z_impl_k_thread_దొంతర_space_get(స్థిరరాశి నిర్మాణం k_thread *thread,
				    size_t *వాడని_ptr)
{
#నిర్వచితమైతే CONFIG_THREAD_దొంతర_MEM_MAPPED
	ఒకవేళ (thread->దొంతర_info.mapped.addr == ఖాళీ) {
		తిరిగి -EINVAL;
	}
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_MEM_MAPPED */

	తిరిగి z_దొంతర_space_get((స్థిరరాశి uint8_t *)thread->దొంతర_info.ప్రారంభం,
				 thread->దొంతర_info.కొలత, వాడని_ptr);
}

#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_thread_దొంతర_space_get(స్థిరరాశి నిర్మాణం k_thread *thread,
				    size_t *వాడని_ptr)
{
	size_t వాడని;
	పూర్ణాంకం ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_దొంతర_space_get(thread, &వాడని);
	CHECKIF(ret != 0) {
		తిరిగి ret;
	}

	ret = k_usermode_గమ్యం_copy(వాడని_ptr, &వాడని, పరిమాణం(size_t));
	CHECKIF(ret != 0) {
		తిరిగి ret;
	}

	తిరిగి 0;
}
#అన్తర్భావ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
#అయితే_ముగింపు /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస k_ticks_t z_vrfy_k_thread_timeout_మిగిలిన_ticks(
						    స్థిరరాశి నిర్మాణం k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	తిరిగి z_impl_k_thread_timeout_మిగిలిన_ticks(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

స్థిర అంతఃవరుస k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  స్థిరరాశి నిర్మాణం k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	తిరిగి z_impl_k_thread_timeout_expires_ticks(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_INSTRUMENT_THREAD_SWITCHING
శూన్య z_thread_mark_switched_లోపల(శూన్య)
{
#ఒకవేళ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_ప్రారంభం(_current);
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#నిర్వచితమైతే CONFIG_TRACING
	SYS_పోర్టు_TRACING_FUNC(k_thread, switched_లోపల);
#అయితే_ముగింపు /* CONFIG_TRACING */
}

శూన్య z_thread_mark_switched_బహిర్గతం(శూన్య)
{
#ఒకవేళ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#అయితే_ముగింపు /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#నిర్వచితమైతే CONFIG_TRACING
#నిర్వచితమైతే CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	ఒకవేళ (!_current ||
	    (_current->పునాది.thread_state & _THREAD_DUMMY) != 0) {
		తిరిగి;
	}
#అయితే_ముగింపు /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_పోర్టు_TRACING_FUNC(k_thread, switched_బహిర్గతం);
#అయితే_ముగింపు /* CONFIG_TRACING */
}
#అయితే_ముగింపు /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

పూర్ణాంకం k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	ఒకవేళ ((thread == ఖాళీ) || (stats == ఖాళీ)) {
		తిరిగి -EINVAL;
	}

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#లేకపోతే
	*stats = (k_thread_runtime_stats_t) {};
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE */

	తిరిగి 0;
}

పూర్ణాంకం k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ఒకవేళ (stats == ఖాళీ) {
		తిరిగి -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	గుర్తులేని పూర్ణాంకం అంకె_cpus = arch_అంకె_cpus();

	కొరకు (గుర్తులేని పూర్ణాంకం i = 0; i < అంకె_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_చక్రాలు += tmp_stats.execution_చక్రాలు;
		stats->మొత్తం_చక్రాలు     += tmp_stats.మొత్తం_చక్రాలు;
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_చక్రాలు   += tmp_stats.current_చక్రాలు;
		stats->peak_చక్రాలు      += tmp_stats.peak_చక్రాలు;
		stats->సరాసరి_చక్రాలు   += tmp_stats.సరాసరి_చక్రాలు;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_చక్రాలు      += tmp_stats.idle_చక్రాలు;
	}
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ALL */

	తిరిగి 0;
}

పూర్ణాంకం k_thread_runtime_stats_cpu_get(పూర్ణాంకం cpu, k_thread_runtime_stats_t *stats)
{
	ఒకవేళ (stats == ఖాళీ) {
		తిరిగి -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((గుర్తులేని పూర్ణాంకం)cpu >= arch_అంకె_cpus())) {
		తిరిగి -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#లేకపోతే
	ARG_వాడని(cpu);
#అయితే_ముగింపు

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
స్థిర నిర్మాణం k_thread *thread_గమ్యం_cleanup;

/** Spinlock for thread abort cleanup. */
స్థిర నిర్మాణం k_spinlock thread_cleanup_తాళం;

#నిర్వచితమైతే CONFIG_THREAD_దొంతర_MEM_MAPPED
స్థిర శూన్య *thread_cleanup_దొంతర_addr;
స్థిర size_t thread_cleanup_దొంతర_sz;

స్థిర శూన్య unmap_thread_దొంతర(శూన్య *addr, size_t sz)
{
	ఒకవేళ (addr != ఖాళీ) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_MEM_MAPPED */

శూన్య defer_thread_cleanup(నిర్మాణం k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_గమ్యం_cleanup = thread;

#నిర్వచితమైతే CONFIG_THREAD_దొంతర_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_దొంతర_addr = thread->దొంతర_info.mapped.addr;
	thread_cleanup_దొంతర_sz = thread->దొంతర_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->దొంతర_info.mapped.addr = ఖాళీ;
	thread->దొంతర_info.mapped.sz = 0;
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

శూన్య do_deferred_thread_cleanup(శూన్య)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#నిర్వచితమైతే CONFIG_THREAD_దొంతర_MEM_MAPPED
	unmap_thread_దొంతర(thread_cleanup_దొంతర_addr, thread_cleanup_దొంతర_sz);

	thread_cleanup_దొంతర_addr = ఖాళీ;
	thread_cleanup_దొంతర_sz = 0;
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

శూన్య do_thread_cleanup(నిర్మాణం k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#నిర్వచితమైతే CONFIG_THREAD_దొంతర_MEM_MAPPED
	unmap_thread_దొంతర(thread->దొంతర_info.mapped.addr,
			   thread->దొంతర_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->దొంతర_info.mapped.addr = ఖాళీ;
	thread->దొంతర_info.mapped.sz = 0;
#లేకపోతే /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_వాడని(thread);
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

శూన్య k_thread_abort_cleanup(నిర్మాణం k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_తాళం) {
		ఒకవేళ (thread_గమ్యం_cleanup != ఖాళీ) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_గమ్యం_cleanup = ఖాళీ;
		}

		ఒకవేళ (thread == _current) {
			/* Need to defer for current running thread as the cleanup
			 * might result in exception. Actual cleanup will be done
			 * at the next time k_thread_abort() is called, or at thread
			 * creation if the same thread object is being reused. This
			 * is to make sure the cleanup code no longer needs this
			 * thread's stack. This is not exactly ideal as the stack
			 * may still be memory mapped for a while. However, this is
			 * a simple solution without a) the need to workaround
			 * the schedule lock during k_thread_abort(), b) creating
			 * another thread to perform the cleanup, and c) does not
			 * require architecture code support (e.g. via exception).
			 */
			defer_thread_cleanup(thread);
		} లేకపోతే {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

శూన్య k_thread_abort_cleanup_check_reuse(నిర్మాణం k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_తాళం) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		ఒకవేళ (thread_గమ్యం_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_గమ్యం_cleanup = ఖాళీ;
		}
	}
}

#అయితే_ముగింపు /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

శూన్య z_dummy_thread_init(నిర్మాణం k_thread *dummy_thread)
{
	dummy_thread->పునాది.thread_state = _THREAD_DUMMY;
#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు
	dummy_thread->పునాది.cpu_ముసుగు = -1;
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->పునాది.user_options = K_ESSENTIAL;
#ఒకవేళ Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = ఖాళీ;
	dummy_thread->orig_prio = dummy_thread->పునాది.prio;
#అయితే_ముగింపు /* Z_MUTEX_PI_ENABLED */
#నిర్వచితమైతే CONFIG_THREAD_దొంతర_INFO
	dummy_thread->దొంతర_info.ప్రారంభం = 0U;
	dummy_thread->దొంతర_info.కొలత = 0U;
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_INFO */
#నిర్వచితమైతే CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#అయితే_ముగింపు /* CONFIG_USERSPACE */
#ఒకవేళ (K_కుప్ప_MEM_POOL_కొలత > 0)
	k_thread_system_pool_కేటాయించు(dummy_thread);
#లేకపోతే
	dummy_thread->resource_pool = ఖాళీ;
#అయితే_ముగింపు /* K_HEAP_MEM_POOL_SIZE */

#నిర్వచితమైతే CONFIG_USE_SWITCH
	dummy_thread->switch_handle = ఖాళీ;
#అయితే_ముగింపు /* CONFIG_USE_SWITCH */

#నిర్వచితమైతే CONFIG_TIMESLICE_చొప్పున_THREAD
	dummy_thread->పునాది.ముక్క_ticks = 0;
#అయితే_ముగింపు /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_సమితి(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

శూన్య z_impl_k_thread_ప్రాధాన్యత_సమితి(k_tid_t thread, పూర్ణాంకం prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, ఖాళీ);

	bool need_sched = z_thread_prio_సమితి((నిర్మాణం k_thread *)thread, prio);

	ఒకవేళ ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->పునాది.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_thread_ప్రాధాన్యత_సమితి(k_tid_t thread, పూర్ణాంకం prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_సందేశం(_is_valid_prio(prio, ఖాళీ),
				    "invalid thread priority %d", prio));
#నిర్వచితం_కాకపోతే CONFIG_USERSPACE_THREAD_MAY_RAISE_ప్రాధాన్యత
	K_OOPS(K_SYSCALL_VERIFY_సందేశం((int8_t)prio >= thread->పునాది.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->పునాది.prio));
#అయితే_ముగింపు /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_ప్రాధాన్యత_సమితి(thread, prio);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	ఒకవేళ (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_లోపల_isr())) {
		z_thread_suspend_current(thread);
		తిరిగి;
	}

	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	ఒకవేళ (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(కీలకం);
		తిరిగి;
	}

	z_thread_halt(thread, కీలకం, false);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	/* Do not try to resume a thread that was not suspended */
	ఒకవేళ (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(కీలకం);
		తిరిగి;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_resume_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_impl_k_wakeup(k_tid_t thread)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	ఒకవేళ (z_is_thread_sleeping(thread)) {
		(శూన్య)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(కీలకం);
	} లేకపోతే {
		z_sched_spinlock_unlock(కీలకం);
	}
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_wakeup_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_thread_abort(నిర్మాణం k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	ఒకవేళ (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(కీలకం);
		తిరిగి;
	}

	z_thread_halt(thread, కీలకం, true);

	ఒకవేళ (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#ఒకవేళ !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
శూన్య z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#అయితే_ముగింపు /* !CONFIG_ARCH_HAS_THREAD_ABORT */

పూర్ణాంకం z_impl_k_thread_join(నిర్మాణం k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();
	పూర్ణాంకం ret;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	ఒకవేళ (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} లేకపోతే ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} లేకపోతే ఒకవేళ ((thread == _current) ||
		   (thread->పునాది.pended_on == &_current->join_వరుస)) {
		ret = -EDEADLK;
	} లేకపోతే {
		__ASSERT(!arch_is_లోపల_isr(), "cannot join in ISR");
		z_sched_కూడు_గమ్యం_waitq_locked(_current, &thread->join_వరుస);
		z_కూడు_thread_timeout(_current, timeout);

		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_మార్పిడి_locked(కీలకం);
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		తిరిగి ret;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(కీలకం);
	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
స్థిర bool thread_obj_validate(నిర్మాణం k_thread *thread)
{
	నిర్మాణం k_object *ko = k_object_కనుగొను(thread);
	పూర్ణాంకం ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	ఎంపిక (ret) {
	స్థితి 0:
		తిరిగి false;
	స్థితి -EINVAL:
		తిరిగి true;
	అప్రమేయ:
#నిర్వచితమైతే CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#అయితే_ముగింపు /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_సందేశం(ret == 0, "access denied"));
	}
	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_thread_join(నిర్మాణం k_thread *thread,
				       k_timeout_t timeout)
{
	ఒకవేళ (thread_obj_validate(thread)) {
		తిరిగి 0;
	}

	తిరిగి z_impl_k_thread_join(thread, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_join_mrsh.c>

స్థిర అంతఃవరుస శూన్య z_vrfy_k_thread_abort(k_tid_t thread)
{
	ఒకవేళ (thread_obj_validate(thread)) {
		తిరిగి;
	}

	K_OOPS(K_SYSCALL_VERIFY_సందేశం(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((నిర్మాణం k_thread *)thread);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_abort_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

bool k_can_yield(శూన్య)
{
	తిరిగి !(k_is_pre_kernel() || k_is_లోపల_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

శూన్య z_impl_k_yield(శూన్య)
{
	__ASSERT(!arch_is_లోపల_isr(), "");

	SYS_పోర్టు_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_yield(శూన్య)
{
	z_impl_k_yield();
}
#అన్తర్భావ <zephyr/syscalls/k_yield_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
