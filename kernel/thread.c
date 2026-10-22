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

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/sys/clock.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <timeout_q.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <kswap.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/tracing/tracing.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <zephyr/sys/check.h>
#अन्तर्भाव <zephyr/random/random.h>
#अन्तर्भाव <zephyr/sys/atomic.h>
#अन्तर्भाव <zephyr/logging/log.h>
#अन्तर्भाव <zephyr/llext/symbol.h>
#अन्तर्भाव <zephyr/sys/iterable_sections.h>
#अन्तर्भाव <zephyr/sys/minmax.h>

#अन्तर्भाव <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_THREAD
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_THREAD
स्थिर संरचना k_obj_core_stats_desc  thread_stats_desc = {
	.raw_आकार = परिमाण(संरचना k_आवृत्तिः_stats),
	.query_आकार = परिमाण(संरचना k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#अन्यथा
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_THREAD */
#यदि_अन्तः /* CONFIG_OBJ_CORE_THREAD */


#परिभाषय _FOREACH_STATIC_THREAD(thread_दत्त)              \
	STRUCT_SECTION_FOREACH(static_thread_दत्त, thread_दत्त)

bool k_is_अन्तर्_isr(शून्य)
{
	प्रत्यागम arch_is_अन्तर्_isr();
}
EXPORT_चिह्नम्(k_is_अन्तर्_isr);

#यदि_परिभाषितम् CONFIG_THREAD_CUSTOM_दत्त
शून्य z_impl_k_thread_custom_दत्त_समुच्चयः(शून्य *मानम्)
{
	_current->custom_दत्त = मानम्;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_custom_दत्त_समुच्चयः(शून्य *दत्त)
{
	z_impl_k_thread_custom_दत्त_समुच्चयः(दत्त);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य *z_impl_k_thread_custom_दत्त_get(शून्य)
{
	प्रत्यागम _current->custom_दत्त;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य *z_vrfy_k_thread_custom_दत्त_get(शून्य)
{
	प्रत्यागम z_impl_k_thread_custom_दत्त_get();
}
#अन्तर्भाव <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#यदि_अन्तः /* CONFIG_USERSPACE */
#यदि_अन्तः /* CONFIG_THREAD_CUSTOM_DATA */

पूर्णाङ्क z_impl_k_is_preempt_thread(शून्य)
{
	प्रत्यागम !arch_is_अन्तर्_isr() && thread_is_preemptible(_current);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_is_preempt_thread(शून्य)
{
	प्रत्यागम z_impl_k_is_preempt_thread();
}
#अन्तर्भाव <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_thread_प्राथमिकता_get(k_tid_t thread)
{
	प्रत्यागम thread->आधारः.prio;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_प्राथमिकता_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	प्रत्यागम z_impl_k_thread_प्राथमिकता_get(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_परिभाषितम् CONFIG_THREAD_NAME
स्थिर शून्य समुच्चयः_thread_name(संरचना k_thread *thread, ध्रुव वर्ण *str)
{
	यदि (str != रिक्त) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#यदि_परिभाषितम् CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_समुच्चयः(thread, str);
#यदि_अन्तः /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} अन्यथा {
		thread->name[0] = '\0';
	}
}
#यदि_अन्तः /* CONFIG_THREAD_NAME */

पूर्णाङ्क z_impl_k_thread_name_समुच्चयः(k_tid_t thread, ध्रुव वर्ण *str)
{
#यदि_परिभाषितम् CONFIG_THREAD_NAME
	यदि (thread == रिक्त) {
		thread = _current;
	}

	समुच्चयः_thread_name(thread, str);

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_thread, name_समुच्चयः, thread, 0);

	प्रत्यागम 0;
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(str);

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_thread, name_समुच्चयः, thread, -ENOSYS);

	प्रत्यागम -ENOSYS;
#यदि_अन्तः /* CONFIG_THREAD_NAME */
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_name_समुच्चयः(k_tid_t thread, ध्रुव वर्ण *str)
{
#यदि_परिभाषितम् CONFIG_THREAD_NAME
	वर्ण name[CONFIG_THREAD_MAX_NAME_LEN];

	यदि (thread != रिक्त) {
		यदि (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			प्रत्यागम -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	यदि (k_usermode_string_copy(name, str, परिमाण(name)) != 0) {
		प्रत्यागम -EFAULT;
	}

	प्रत्यागम z_impl_k_thread_name_समुच्चयः(thread, name);
#अन्यथा
	प्रत्यागम -ENOSYS;
#यदि_अन्तः /* CONFIG_THREAD_NAME */
}
#अन्तर्भाव <zephyr/syscalls/k_thread_name_set_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

ध्रुव वर्ण *k_thread_name_get(k_tid_t thread)
{
#यदि_परिभाषितम् CONFIG_THREAD_NAME
	प्रत्यागम (ध्रुव वर्ण *)thread->name;
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	प्रत्यागम रिक्त;
#यदि_अन्तः /* CONFIG_THREAD_NAME */
}

पूर्णाङ्क z_impl_k_thread_name_copy(k_tid_t thread, वर्ण *buf, size_t आकार)
{
#यदि_परिभाषितम् CONFIG_THREAD_NAME
	strncpy(buf, thread->name, आकार);
	प्रत्यागम 0;
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(buf);
	ARG_अप्रयुक्तम्(आकार);
	प्रत्यागम -ENOSYS;
#यदि_अन्तः /* CONFIG_THREAD_NAME */
}

स्थिर size_t copy_अष्टकानि(वर्ण *dest, size_t dest_आकार, ध्रुव वर्ण *src, size_t src_आकार)
{
	size_t  अष्टकानि_गन्तव्यम्_copy;

	अष्टकानि_गन्तव्यम्_copy = min(dest_आकार, src_आकार);
	स्मृतिं_प्रतिलिख(dest, src, अष्टकानि_गन्तव्यम्_copy);

	प्रत्यागम अष्टकानि_गन्तव्यम्_copy;
}

ध्रुव वर्ण *k_thread_state_str(k_tid_t thread_id, वर्ण *buf, size_t buf_आकार)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->आधारः.thread_state;
#परिभाषय SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, परिमाण(Z_STATE_STR_##s) - 1 }
	स्थिर ध्रुव संरचना {
		ध्रुव वर्ण *str;
		uint16_t    bit;
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
#अपरिभाषय SS_ENT

	यदि ((buf == रिक्त) || (buf_आकार == 0)) {
		प्रत्यागम "";
	}

	buf_आकार--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	प्रति (अचिह्नित पूर्णाङ्क सूचकांक = 0; thread_state != 0; सूचकांक++) {
		bit = state_string[सूचकांक].bit;
		यदि ((thread_state & bit) == 0) {
			अनुवर्त;
		}

		off += copy_अष्टकानि(buf + off, buf_आकार - off,
				  state_string[सूचकांक].str,
				  state_string[सूचकांक].len);

		thread_state &= ~bit;

		यदि (thread_state != 0) {
			off += copy_अष्टकानि(buf + off, buf_आकार - off, "+", 1);
		}
	}

	buf[off] = '\0';

	प्रत्यागम (ध्रुव वर्ण *)buf;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_name_copy(k_tid_t thread,
					    वर्ण *buf, size_t आकार)
{
#यदि_परिभाषितम् CONFIG_THREAD_NAME
	size_t len;
	संरचना k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	यदि ((ko == रिक्त) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		प्रत्यागम -EINVAL;
	}
	यदि (K_SYSCALL_स्मृतिः_WRITE(buf, आकार) != 0) {
		प्रत्यागम -EFAULT;
	}
	len = वाक्यदैर्घ्यम्(thread->name);
	यदि ((len + 1) > आकार) {
		प्रत्यागम -ENOSPC;
	}

	प्रत्यागम k_usermode_गन्तव्यम्_copy((शून्य *)buf, thread->name, len + 1);
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(buf);
	ARG_अप्रयुक्तम्(आकार);
	प्रत्यागम -ENOSYS;
#यदि_अन्तः /* CONFIG_THREAD_NAME */
}
#अन्तर्भाव <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_परिभाषितम् CONFIG_राशिः_SENTINEL
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
शून्य z_check_राशिः_sentinel(शून्य)
{
	uint32_t *राशिः;

	यदि ((_current->आधारः.thread_state & _THREAD_DUMMY) != 0) {
		प्रत्यागम;
	}

	राशिः = (uint32_t *)_current->राशिः_info.start;
	यदि (*राशिः != राशिः_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*राशिः = राशिः_SENTINEL;
		z_except_reason(K_ERR_राशिः_CHK_FAIL);
	}
}
#यदि_अन्तः /* CONFIG_STACK_SENTINEL */

#यदि defined(CONFIG_राशिः_POINTER_RANDOM) && (CONFIG_राशिः_POINTER_RANDOM != 0)
पूर्णाङ्क z_राशिः_adjust_initialized;

स्थिर size_t random_विस्थापनम्(size_t राशिः_आकार)
{
	size_t random_val;

	यदि (!z_राशिः_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, परिमाण(random_val));
	} अन्यथा {
		sys_rand_get((uint8_t *)&random_val, परिमाण(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ध्रुव size_t fuzz = random_val % CONFIG_राशिः_POINTER_RANDOM;

	यदि (unlikely(fuzz * 2 > राशिः_आकार)) {
		प्रत्यागम 0;
	}

	प्रत्यागम fuzz;
}
#यदि defined(CONFIG_राशिः_GROWS_ऊर्ध्वम्)
	/* This is so rare not bothering for now */
#दोषः "Stack pointer randomization not implemented for upward growing stacks"
#यदि_अन्तः /* CONFIG_STACK_GROWS_UP */
#यदि_अन्तः /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
संरचना राशिः_geometry {
	size_t obj_आकार;	/* size of the whole stack object */
	वर्ण *buf_start;	/* start of the usable stack buffer */
	size_t buf_आकार;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
स्थिर संरचना राशिः_geometry compute_राशिः_geometry(k_thread_राशिः_t *राशिः,
						    size_t राशिः_आकार)
{
	संरचना राशिः_geometry geo;

#यदि_परिभाषितम् CONFIG_USERSPACE
	यदि (z_राशिः_is_user_capable(राशिः)) {
		geo.obj_आकार = K_THREAD_राशिः_LEN(राशिः_आकार);
		geo.buf_start = K_THREAD_राशिः_बफर(राशिः);
		geo.buf_आकार = geo.obj_आकार - K_THREAD_राशिः_RESERVED;
	} अन्यथा
#यदि_अन्तः /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_आकार = K_KERNEL_राशिः_LEN(राशिः_आकार);
		geo.buf_start = K_KERNEL_राशिः_बफर(राशिः);
		geo.buf_आकार = geo.obj_आकार - K_KERNEL_राशिः_RESERVED;

#यदि defined(ARCH_KERNEL_राशिः_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		यदि (K_KERNEL_राशिः_RESERVED > geo.obj_आकार) {
			k_panic();
		}
#यदि_अन्तः
	}

	प्रत्यागम geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
स्थिर वर्ण *map_thread_राशिः(संरचना k_thread *new_thread,
			      k_thread_राशिः_t *राशिः,
			      संरचना राशिः_geometry *geo)
{
	वर्ण *राशिः_ptr;

#यदि_परिभाषितम् CONFIG_THREAD_राशिः_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	शून्य *राशिः_mapped = k_mem_map_phys_guard((uintptr_t)राशिः, geo->obj_आकार,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)राशिः_mapped != 0);

#यदि_परिभाषितम् CONFIG_USERSPACE
	यदि (z_राशिः_is_user_capable(राशिः)) {
		geo->buf_start = K_THREAD_राशिः_बफर(राशिः_mapped);
	} अन्यथा
#यदि_अन्तः /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_राशिः_बफर(राशिः_mapped);
	}

	राशिः_ptr = (वर्ण *)राशिः_mapped + geo->obj_आकार;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->राशिः_info.mapped.addr = राशिः_mapped;
	new_thread->राशिः_info.mapped.sz = geo->obj_आकार;
#अन्यथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_अप्रयुक्तम्(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	राशिः_ptr = (वर्ण *)राशिः + geo->obj_आकार;
#यदि_अन्तः /* CONFIG_THREAD_STACK_MEM_MAPPED */

	प्रत्यागम राशिः_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
स्थिर अन्तःस्थ शून्य fill_init_राशिः(वर्ण *buf_start, size_t buf_आकार)
{
#यदि_परिभाषितम् CONFIG_INIT_STACKS
	स्मृतिं_पूरय(buf_start, 0xaa, buf_आकार);
#अन्यथा
	ARG_अप्रयुक्तम्(buf_start);
	ARG_अप्रयुक्तम्(buf_आकार);
#यदि_अन्तः /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
स्थिर अन्तःस्थ शून्य समुच्चयः_राशिः_sentinel(वर्ण *buf_start)
{
#यदि_परिभाषितम् CONFIG_राशिः_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = राशिः_SENTINEL;
#अन्यथा
	ARG_अप्रयुक्तम्(buf_start);
#यदि_अन्तः /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
स्थिर size_t reserve_राशिः_headroom(संरचना k_thread *new_thread,
				     वर्ण *राशिः_ptr, size_t buf_आकार)
{
	size_t delta = 0;

	ARG_अप्रयुक्तम्(new_thread);
	ARG_अप्रयुक्तम्(राशिः_ptr);
	ARG_अप्रयुक्तम्(buf_आकार);

#यदि_परिभाषितम् CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_राशिः_सज्जता(new_thread, राशिः_ptr);
#यदि_अन्तः /* CONFIG_THREAD_LOCAL_STORAGE */
#यदि_परिभाषितम् CONFIG_THREAD_USERSPACE_LOCAL_दत्त
	size_t tls_आकार = परिमाण(संरचना thread_userspace_local_दत्त);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_आकार;
	new_thread->userspace_local_दत्त =
		(संरचना thread_userspace_local_दत्त *)(राशिः_ptr - delta);
#यदि_अन्तः /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#यदि defined(CONFIG_राशिः_POINTER_RANDOM) && (CONFIG_राशिः_POINTER_RANDOM != 0)
	delta += random_विस्थापनम्(buf_आकार);
#यदि_अन्तः /* CONFIG_STACK_POINTER_RANDOM */

	प्रत्यागम आवर्तनम्_ऊर्ध्वम्(delta, ARCH_राशिः_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
स्थिर अन्तःस्थ शून्य समुच्चयः_राशिः_info(संरचना k_thread *new_thread, वर्ण *buf_start,
				  size_t buf_आकार, size_t delta)
{
#यदि_परिभाषितम् CONFIG_THREAD_राशिः_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->राशिः_info.start = (uintptr_t)buf_start;
	new_thread->राशिः_info.आकार = buf_आकार;
	new_thread->राशिः_info.delta = delta;

#यदि_परिभाषितम् CONFIG_THREAD_RUNTIME_राशिः_SAFETY
	new_thread->राशिः_info.usage.अप्रयुक्तम्_देहली =
		(CONFIG_THREAD_RUNTIME_राशिः_SAFETY_DEFAULT_अप्रयुक्तम्_देहली_PCT *
		 buf_आकार) / 100;
#यदि_अन्तः
#अन्यथा
	ARG_अप्रयुक्तम्(new_thread);
	ARG_अप्रयुक्तम्(buf_start);
	ARG_अप्रयुक्तम्(buf_आकार);
	ARG_अप्रयुक्तम्(delta);
#यदि_अन्तः /* CONFIG_THREAD_STACK_INFO */
}

स्थिर वर्ण *सज्जता_thread_राशिः(संरचना k_thread *new_thread,
				k_thread_राशिः_t *राशिः, size_t राशिः_आकार)
{
	संरचना राशिः_geometry geo = compute_राशिः_geometry(राशिः, राशिः_आकार);
	वर्ण *राशिः_ptr = map_thread_राशिः(new_thread, राशिः, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		राशिः, new_thread, geo.obj_आकार, (शून्य *)geo.buf_start,
		geo.buf_आकार, (शून्य *)राशिः_ptr);

	fill_init_राशिः(geo.buf_start, geo.buf_आकार);
	समुच्चयः_राशिः_sentinel(geo.buf_start);
	delta = reserve_राशिः_headroom(new_thread, राशिः_ptr, geo.buf_आकार);
	समुच्चयः_राशिः_info(new_thread, geo.buf_start, geo.buf_आकार, delta);

	प्रत्यागम राशिः_ptr - delta;
}

#यदि_परिभाषितम् CONFIG_HW_SHADOW_राशिः
स्थिर शून्य सज्जता_shadow_राशिः(संरचना k_thread *new_thread,
			 k_thread_राशिः_t *राशिः)
{
	पूर्णाङ्क ret = -ENOENT;

	STRUCT_SECTION_FOREACH(राशिः_गन्तव्यम्_hw_shadow_राशिः, stk_गन्तव्यम्_hw_shstk) {
		यदि (stk_गन्तव्यम्_hw_shstk->राशिः == राशिः) {
			ret = k_thread_hw_shadow_राशिः_attach(new_thread,
							      stk_गन्तव्यम्_hw_shstk->shstk_addr,
							      stk_गन्तव्यम्_hw_shstk->आकार);
			यदि (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_गन्तव्यम्_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			भङ्ग;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	यदि (ret != -ENOENT) {
		प्रत्यागम;
	}

	STRUCT_SECTION_FOREACH(राशिः_गन्तव्यम्_hw_shadow_राशिः_arr, stk_गन्तव्यम्_hw_shstk) {
		यदि ((uintptr_t)राशिः >= stk_गन्तव्यम्_hw_shstk->राशिः_addr &&
		    (uintptr_t)राशिः < stk_गन्तव्यम्_hw_shstk->राशिः_addr +
		    stk_गन्तव्यम्_hw_shstk->राशिः_आकार * stk_गन्तव्यम्_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t राशिः_विस्थापनम् = (uintptr_t)राशिः - stk_गन्तव्यम्_hw_shstk->राशिः_addr;
			uintptr_t राशिः_सूचकांक = राशिः_विस्थापनम् / stk_गन्तव्यम्_hw_shstk->राशिः_आकार;
			uintptr_t addr;

			यदि (राशिः_सूचकांक >= stk_गन्तव्यम्_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, राशिः);
				k_panic();
			}

			addr = stk_गन्तव्यम्_hw_shstk->shstk_addr +
				stk_गन्तव्यम्_hw_shstk->shstk_आकार * राशिः_सूचकांक;
			ret = k_thread_hw_shadow_राशिः_attach(new_thread,
							      (arch_thread_hw_shadow_राशिः_t *)addr,
							      stk_गन्तव्यम्_hw_shstk->shstk_आकार);
			यदि (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_गन्तव्यम्_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			भङ्ग;
		}
	}

	यदि (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, राशिः);
		k_panic();
	}
}
#अन्यथा
/* No-op stub used when hardware shadow stacks are not configured. */
स्थिर अन्तःस्थ शून्य सज्जता_shadow_राशिः(संरचना k_thread *new_thread,
				      k_thread_राशिः_t *राशिः)
{
	ARG_अप्रयुक्तम्(new_thread);
	ARG_अप्रयुक्तम्(राशिः);
}
#यदि_अन्तः

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
स्थिर अन्तःस्थ शून्य thread_abort_cleanup_check_reuse(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
स्थिर अन्तःस्थ शून्य init_thread_obj_core(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_पञ्जिकाकोशः(K_OBJ_CORE(thread), &thread->आधारः.usage,
				  परिमाण(thread->आधारः.usage));
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_THREAD */
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
स्थिर अन्तःस्थ शून्य init_thread_userspace(संरचना k_thread *thread,
					 k_thread_राशिः_t *राशिः,
					 uint32_t options)
{
#यदि_परिभाषितम् CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_राशिः_is_user_capable(राशिः),
		 "user thread %p with kernel-only stack %p", thread, राशिः);
	k_object_init(thread);
	k_object_init(राशिः);
	thread->राशिः_obj = राशिः;
	thread->syscall_frame = रिक्त;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(राशिः);
	ARG_अप्रयुक्तम्(options);
#यदि_अन्तः /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
स्थिर अन्तःस्थ शून्य assert_thread_coherence(संरचना k_thread *thread,
					   k_thread_राशिः_t *राशिः)
{
#यदि_परिभाषितम् CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#यदि_अपरिभाषितम् CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(राशिः));
#यदि_अन्तः /* CONFIG_DYNAMIC_THREAD */
#यदि_अन्तः /* CONFIG_KERNEL_COHERENCE */
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(राशिः);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
स्थिर अन्तःस्थ शून्य assert_switch_handle(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != रिक्त,
		 "arch layer failed to initialize switch_handle");
#यदि_अन्तः /* CONFIG_USE_SWITCH */
	ARG_अप्रयुक्तम्(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
स्थिर अन्तःस्थ शून्य init_thread_custom_दत्त(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_THREAD_CUSTOM_दत्त
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_दत्त = रिक्त;
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
स्थिर अन्तःस्थ शून्य सङ्कलय_thread_गन्तव्यम्_monitor(संरचना k_thread *thread,
					 k_thread_प्रविष्टिः_t प्रविष्टिः,
					 शून्य *p1, शून्य *p2, शून्य *p3)
{
#यदि_परिभाषितम् CONFIG_THREAD_MONITOR
	thread->प्रविष्टिः.p_प्रविष्टिः = प्रविष्टिः;
	thread->प्रविष्टिः.parameter1 = p1;
	thread->प्रविष्टिः.parameter2 = p2;
	thread->प्रविष्टिः.parameter3 = p3;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&z_thread_monitor_तालकम्);

	thread->अग्र_thread = _kernel.तन्तवः;
	_kernel.तन्तवः = thread;
	k_spin_unlock(&z_thread_monitor_तालकम्, कुञ्चिका);
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(प्रविष्टिः);
	ARG_अप्रयुक्तम्(p1);
	ARG_अप्रयुक्तम्(p2);
	ARG_अप्रयुक्तम्(p3);
#यदि_अन्तः /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
स्थिर अन्तःस्थ शून्य init_thread_name(संरचना k_thread *thread, ध्रुव वर्ण *name)
{
#यदि_परिभाषितम् CONFIG_THREAD_NAME
	समुच्चयः_thread_name(thread, name);
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(name);
#यदि_अन्तः /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
स्थिर अन्तःस्थ शून्य init_thread_cpu_आवरणम्(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्
	यदि (IS_ENABLED(CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY)) {
		thread->आधारः.cpu_आवरणम् = 1; /* must specify only one cpu */
	} अन्यथा {
		thread->आधारः.cpu_आवरणम् = -1; /* allow all cpus */
	}
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
स्थिर अन्तःस्थ शून्य init_thread_userspace_perms(संरचना k_thread *thread,
					       uint32_t options)
{
#यदि_परिभाषितम् CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	यदि ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	ARG_अप्रयुक्तम्(options);
#यदि_अन्तः /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
स्थिर अन्तःस्थ शून्य init_thread_deadline(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_SCHED_DEADLINE
	thread->आधारः.prio_deadline = 0;
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
स्थिर अन्तःस्थ शून्य init_thread_halt_queue(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
स्थिर अन्तःस्थ शून्य init_thread_usage(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE
	thread->आधारः.usage = (संरचना k_आवृत्तिः_stats) {};
	thread->आधारः.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
स्थिर अन्तःस्थ शून्य assert_thread_not_reused(संरचना k_thread *new_thread)
{
#यदि_परिभाषितम् CONFIG_THREAD_MONITOR
#यदि_परिभाषितम् CONFIG_ASSERT
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&z_thread_monitor_तालकम्);
	bool reused = false;

	/* Check if the thread is already in the list */
	प्रति (संरचना k_thread *t = _kernel.तन्तवः; t; t = t->अग्र_thread) {
		यदि (t == new_thread) {
			reused = true;
			भङ्ग;
		}
	}

	k_spin_unlock(&z_thread_monitor_तालकम्, कुञ्चिका);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#यदि_अन्तः /* CONFIG_ASSERT */
#यदि_अन्तः /* CONFIG_THREAD_MONITOR */
	ARG_अप्रयुक्तम्(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
वर्ण *z_सज्जता_new_thread(संरचना k_thread *new_thread,
			 k_thread_राशिः_t *राशिः, size_t राशिः_आकार,
			 k_thread_प्रविष्टिः_t प्रविष्टिः,
			 शून्य *p1, शून्य *p2, शून्य *p3,
			 पूर्णाङ्क prio, uint32_t options, ध्रुव वर्ण *name)
{
	वर्ण *राशिः_ptr;

	Z_ASSERT_VALID_PRIO(prio, प्रविष्टिः);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, राशिः, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_आधारः(&new_thread->आधारः, prio, _THREAD_SLEEPING, options);
	राशिः_ptr = सज्जता_thread_राशिः(new_thread, राशिः, राशिः_आकार);

#यदि Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = रिक्त;
	new_thread->orig_prio = new_thread->आधारः.prio;
#यदि_अन्तः /* Z_MUTEX_PI_ENABLED */

	सज्जता_shadow_राशिः(new_thread, राशिः);
	assert_thread_coherence(new_thread, राशिः);

	arch_new_thread(new_thread, राशिः, राशिः_ptr, प्रविष्टिः, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_दत्त = रिक्त;

	assert_switch_handle(new_thread);
	init_thread_custom_दत्त(new_thread);
	सङ्कलय_thread_गन्तव्यम्_monitor(new_thread, प्रविष्टिः, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_आवरणम्(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	यदि (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_गन्तव्यम्_MAIN) &&
	    (_current == रिक्त)) {
		new_thread->resource_pool = रिक्त;
		प्रत्यागम राशिः_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	प्रत्यागम राशिः_ptr;
}

#यदि_परिभाषितम् CONFIG_THREAD_RUNTIME_राशिः_SAFETY
पूर्णाङ्क z_impl_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_pct_समुच्चयः(संरचना k_thread *thread,
							   uint32_t pct)
{
	size_t अप्रयुक्तम्_देहली;

	यदि (pct > 99) {
		प्रत्यागम -EINVAL;   /* 100% unused stack and up is invalid */
	}

	अप्रयुक्तम्_देहली = (thread->राशिः_info.आकार * pct) / 100;

	thread->राशिः_info.usage.अप्रयुक्तम्_देहली = अप्रयुक्तम्_देहली;

	प्रत्यागम 0;
}

पूर्णाङ्क z_impl_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_समुच्चयः(संरचना k_thread *thread,
						       size_t देहली)
{
	यदि (देहली > thread->राशिः_info.आकार) {
		प्रत्यागम -EINVAL;
	}

	thread->राशिः_info.usage.अप्रयुक्तम्_देहली = देहली;

	प्रत्यागम 0;
}

size_t z_impl_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_get(संरचना k_thread *thread)
{
	प्रत्यागम thread->राशिः_info.usage.अप्रयुक्तम्_देहली;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_pct_समुच्चयः(संरचना k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	प्रत्यागम z_impl_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_pct_समुच्चयः(thread, pct);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

पूर्णाङ्क z_vrfy_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_समुच्चयः(संरचना k_thread *thread,
						       size_t देहली)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	प्रत्यागम z_impl_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_समुच्चयः(thread, देहली);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_get(संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	प्रत्यागम z_impl_k_thread_runtime_राशिः_अप्रयुक्तम्_देहली_get(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
#यदि_अन्तः /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(संरचना k_thread *new_thread,
			      k_thread_राशिः_t *राशिः,
			      size_t राशिः_आकार, k_thread_प्रविष्टिः_t प्रविष्टिः,
			      शून्य *p1, शून्य *p2, शून्य *p3,
			      पूर्णाङ्क prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_अन्तर्_isr(), "Threads may not be created in ISRs");

	z_सज्जता_new_thread(new_thread, राशिः, राशिः_आकार, प्रविष्टिः, p1, p2, p3,
			  prio, options, रिक्त);

	thread_कालसारणी_new(new_thread, delay);

	प्रत्यागम new_thread;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
bool z_राशिः_is_user_capable(k_thread_राशिः_t *राशिः)
{
	प्रत्यागम k_object_find(राशिः) != रिक्त;
}

k_tid_t z_vrfy_k_thread_create(संरचना k_thread *new_thread,
			       k_thread_राशिः_t *राशिः,
			       size_t राशिः_आकार, k_thread_प्रविष्टिः_t प्रविष्टिः,
			       शून्य *p1, शून्य *p2, शून्य *p3,
			       पूर्णाङ्क prio, uint32_t options, k_timeout_t delay)
{
	size_t योग_आकार, राशिः_obj_आकार;
	संरचना k_object *राशिः_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	राशिः_object = k_object_find(राशिः);
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(k_object_validation_check(राशिः_object, राशिः,
						K_OBJ_THREAD_राशिः_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(!आकार_सङ्कलय_अतिप्रवाहः(K_THREAD_राशिः_RESERVED,
						       राशिः_आकार, &योग_आकार),
				    "stack size overflow (%zu+%zu)",
				    राशिः_आकार,
				    K_THREAD_राशिः_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#यदि_परिभाषितम् CONFIG_GEN_PRIV_STACKS
	राशिः_obj_आकार = राशिः_object->दत्त.राशिः_दत्त->आकार;
#अन्यथा
	राशिः_obj_आकार = राशिः_object->दत्त.राशिः_आकार;
#यदि_अन्तः /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(योग_आकार <= राशिः_obj_आकार,
				    "stack size %zu is too big, max is %zu",
				    योग_आकार, राशिः_obj_आकार));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, रिक्त)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_अधस्तनम्_or_equal(prio,
							_current->आधारः.prio)));

	z_सज्जता_new_thread(new_thread, राशिः, राशिः_आकार,
			   प्रविष्टिः, p1, p2, p3, prio, options, रिक्त);

	thread_कालसारणी_new(new_thread, delay);

	प्रत्यागम new_thread;
}
#अन्तर्भाव <zephyr/syscalls/k_thread_create_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य z_init_thread_आधारः(संरचना thread_आधारः *thread_आधारः_2, पूर्णाङ्क प्राथमिकता,
		       uint32_t initial_state, अचिह्नित पूर्णाङ्क options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_आधारः_2->pended_on = रिक्त;
	thread_आधारः_2->user_options = (uint16_t)options;
	thread_आधारः_2->thread_state = (uint8_t)initial_state;

	thread_आधारः_2->prio = प्राथमिकता;

	thread_आधारः_2->sched_locked = 0U;

#यदि_परिभाषितम् CONFIG_SMP
	thread_आधारः_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_आधारः_2->cpu = 0;
#यदि_अन्तः /* CONFIG_SMP */

#यदि_परिभाषितम् CONFIG_TIMESLICE_प्रत्येकम्_THREAD
	thread_आधारः_2->खण्डिका_ticks = 0;
	thread_आधारः_2->खण्डिका_expired = रिक्त;
#यदि_अन्तः /* CONFIG_TIMESLICE_PER_THREAD */

#यदि_परिभाषितम् CONFIG_SPIN_VALIDATE
	thread_आधारः_2->swap_दत्त = रिक्त;
	/* otherwise swap_data does not need to be initialized */
#यदि_अन्तः

	z_init_thread_timeout(thread_आधारः_2);
}

FUNC_NORETURN शून्य k_thread_user_बहुलकम्_enter(k_thread_प्रविष्टिः_t प्रविष्टिः,
					    शून्य *p1, शून्य *p2, शून्य *p3)
{
	SYS_पत्तनम्_TRACING_FUNC(k_thread, user_बहुलकम्_enter);

	_current->आधारः.user_options |= K_USER;
	z_thread_essential_clear(_current);
#यदि_परिभाषितम् CONFIG_THREAD_MONITOR
	_current->प्रविष्टिः.p_प्रविष्टिः = प्रविष्टिः;
	_current->प्रविष्टिः.parameter1 = p1;
	_current->प्रविष्टिः.parameter2 = p2;
	_current->प्रविष्टिः.parameter3 = p3;
#यदि_अन्तः /* CONFIG_THREAD_MONITOR */
#यदि_परिभाषितम् CONFIG_USERSPACE
	__ASSERT(z_राशिः_is_user_capable(_current->राशिः_obj),
		 "dropping to user mode with kernel-only stack object");
#यदि_परिभाषितम् CONFIG_THREAD_USERSPACE_LOCAL_दत्त
	स्मृतिं_पूरय(_current->userspace_local_दत्त, 0,
	       परिमाण(संरचना thread_userspace_local_दत्त));
#यदि_अन्तः /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#यदि_परिभाषितम् CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_राशिः_सज्जता(_current,
			     (वर्ण *)(_current->राशिः_info.start +
				      _current->राशिः_info.आकार));
#यदि_अन्तः /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_बहुलकम्_enter(प्रविष्टिः, p1, p2, p3);
#अन्यथा
	/* XXX In this case we do not reset the stack */
	z_thread_प्रविष्टिः(प्रविष्टिः, p1, p2, p3);
#यदि_अन्तः /* CONFIG_USERSPACE */
}

#यदि defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_राशिः_INFO)
#यदि_परिभाषितम् CONFIG_राशिः_GROWS_ऊर्ध्वम्
#दोषः "Unsupported configuration for stack analysis"
#यदि_अन्तः /* CONFIG_STACK_GROWS_UP */

पूर्णाङ्क z_राशिः_space_get(ध्रुव uint8_t *राशिः_start, size_t आकार, size_t *अप्रयुक्तम्_ptr)
{
	size_t अप्रयुक्तम् = 0;
	ध्रुव uint8_t *checked_राशिः = राशिः_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ध्रुव uint8_t *राशिः_pointer = (ध्रुव uint8_t *)&राशिः_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	यदि ((राशिः_pointer > राशिः_start) && (राशिः_pointer <= (राशिः_start + आकार)) &&
	    IS_ENABLED(CONFIG_NO_अप्रयुक्तम्_राशिः_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		प्रत्यागम -ENOTSUP;
	}

	यदि (IS_ENABLED(CONFIG_राशिः_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_राशिः += 4;
		आकार -= 4;
	}

	प्रति (size_t i = 0; i < आकार; i++) {
		यदि ((checked_राशिः[i]) == 0xaaU) {
			अप्रयुक्तम्++;
		} अन्यथा {
			भङ्ग;
		}
	}

	*अप्रयुक्तम्_ptr = अप्रयुक्तम्;

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_THREAD_RUNTIME_राशिः_SAFETY
पूर्णाङ्क k_thread_runtime_राशिः_safety_पूर्णम्_check(ध्रुव संरचना k_thread *thread,
					     size_t *अप्रयुक्तम्_ptr,
					     k_thread_राशिः_safety_handler_t handler,
					     शून्य *arg)
{
	पूर्णाङ्क    rv;
	size_t अप्रयुक्तम्_space;

	__ASSERT_NO_MSG(thread != रिक्त);

	rv = z_राशिः_space_get((ध्रुव uint8_t *)thread->राशिः_info.start,
			       thread->राशिः_info.आकार, &अप्रयुक्तम्_space);

	यदि (rv != 0) {
		प्रत्यागम rv;
	}

	यदि (अप्रयुक्तम्_ptr != रिक्त) {
		*अप्रयुक्तम्_ptr = अप्रयुक्तम्_space;
	}

	यदि ((अप्रयुक्तम्_space < thread->राशिः_info.usage.अप्रयुक्तम्_देहली) &&
	    (handler != रिक्त)) {
		handler(thread, अप्रयुक्तम्_space, arg);
	}

	प्रत्यागम 0;
}

पूर्णाङ्क k_thread_runtime_राशिः_safety_देहली_check(ध्रुव संरचना k_thread *thread,
						  size_t *अप्रयुक्तम्_ptr,
						  k_thread_राशिः_safety_handler_t handler,
						  शून्य *arg)
{
	पूर्णाङ्क    rv;
	size_t अप्रयुक्तम्_space;
	size_t देहली;
	size_t अवलोकनम्_आकार;

	__ASSERT_NO_MSG(thread != रिक्त);

	देहली = thread->राशिः_info.usage.अप्रयुक्तम्_देहली;
	अवलोकनम्_आकार = देहली;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	यदि (IS_ENABLED(CONFIG_राशिः_SENTINEL) && (देहली != 0U)) {
		अवलोकनम्_आकार += परिमाण(uint32_t);
	}

	rv = z_राशिः_space_get((ध्रुव uint8_t *)thread->राशिः_info.start,
			       अवलोकनम्_आकार, &अप्रयुक्तम्_space);

	यदि (rv != 0) {
		प्रत्यागम rv;
	}

	यदि (अप्रयुक्तम्_ptr != रिक्त) {
		*अप्रयुक्तम्_ptr = अप्रयुक्तम्_space;
	}

	यदि ((अप्रयुक्तम्_space < देहली) && (handler != रिक्त)) {
		handler(thread, अप्रयुक्तम्_space, arg);
	}

	प्रत्यागम 0;
}
#यदि_अन्तः /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

पूर्णाङ्क z_impl_k_thread_राशिः_space_get(ध्रुव संरचना k_thread *thread,
				    size_t *अप्रयुक्तम्_ptr)
{
#यदि_परिभाषितम् CONFIG_THREAD_राशिः_MEM_MAPPED
	यदि (thread->राशिः_info.mapped.addr == रिक्त) {
		प्रत्यागम -EINVAL;
	}
#यदि_अन्तः /* CONFIG_THREAD_STACK_MEM_MAPPED */

	प्रत्यागम z_राशिः_space_get((ध्रुव uint8_t *)thread->राशिः_info.start,
				 thread->राशिः_info.आकार, अप्रयुक्तम्_ptr);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_thread_राशिः_space_get(ध्रुव संरचना k_thread *thread,
				    size_t *अप्रयुक्तम्_ptr)
{
	size_t अप्रयुक्तम्;
	पूर्णाङ्क ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_राशिः_space_get(thread, &अप्रयुक्तम्);
	CHECKIF(ret != 0) {
		प्रत्यागम ret;
	}

	ret = k_usermode_गन्तव्यम्_copy(अप्रयुक्तम्_ptr, &अप्रयुक्तम्, परिमाण(size_t));
	CHECKIF(ret != 0) {
		प्रत्यागम ret;
	}

	प्रत्यागम 0;
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
#यदि_अन्तः /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    ध्रुव संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	प्रत्यागम z_impl_k_thread_timeout_remaining_ticks(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

स्थिर अन्तःस्थ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ध्रुव संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	प्रत्यागम z_impl_k_thread_timeout_expires_ticks(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_परिभाषितम् CONFIG_INSTRUMENT_THREAD_SWITCHING
शून्य z_thread_mark_switched_अन्तर्(शून्य)
{
#यदि defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#यदि_परिभाषितम् CONFIG_TRACING
	SYS_पत्तनम्_TRACING_FUNC(k_thread, switched_अन्तर्);
#यदि_अन्तः /* CONFIG_TRACING */
}

शून्य z_thread_mark_switched_बहिर्गमनम्(शून्य)
{
#यदि defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#यदि_अन्तः /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#यदि_परिभाषितम् CONFIG_TRACING
#यदि_परिभाषितम् CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	यदि (!_current ||
	    (_current->आधारः.thread_state & _THREAD_DUMMY) != 0) {
		प्रत्यागम;
	}
#यदि_अन्तः /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_पत्तनम्_TRACING_FUNC(k_thread, switched_बहिर्गमनम्);
#यदि_अन्तः /* CONFIG_TRACING */
}
#यदि_अन्तः /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

पूर्णाङ्क k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	यदि ((thread == रिक्त) || (stats == रिक्त)) {
		प्रत्यागम -EINVAL;
	}

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#अन्यथा
	*stats = (k_thread_runtime_stats_t) {};
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE */

	प्रत्यागम 0;
}

पूर्णाङ्क k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ALL */

	यदि (stats == रिक्त) {
		प्रत्यागम -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	अचिह्नित पूर्णाङ्क अङ्कः_cpus = arch_अङ्कः_cpus();

	प्रति (अचिह्नित पूर्णाङ्क i = 0; i < अङ्कः_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_आवृत्तयः += tmp_stats.execution_आवृत्तयः;
		stats->योग_आवृत्तयः     += tmp_stats.योग_आवृत्तयः;
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_आवृत्तयः   += tmp_stats.current_आवृत्तयः;
		stats->peak_आवृत्तयः      += tmp_stats.peak_आवृत्तयः;
		stats->मध्यमानम्_आवृत्तयः   += tmp_stats.मध्यमानम्_आवृत्तयः;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_आवृत्तयः      += tmp_stats.idle_आवृत्तयः;
	}
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ALL */

	प्रत्यागम 0;
}

पूर्णाङ्क k_thread_runtime_stats_cpu_get(पूर्णाङ्क cpu, k_thread_runtime_stats_t *stats)
{
	यदि (stats == रिक्त) {
		प्रत्यागम -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((अचिह्नित पूर्णाङ्क)cpu >= arch_अङ्कः_cpus())) {
		प्रत्यागम -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#अन्यथा
	ARG_अप्रयुक्तम्(cpu);
#यदि_अन्तः

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
स्थिर संरचना k_thread *thread_गन्तव्यम्_cleanup;

/** Spinlock for thread abort cleanup. */
स्थिर संरचना k_spinlock thread_cleanup_तालकम्;

#यदि_परिभाषितम् CONFIG_THREAD_राशिः_MEM_MAPPED
स्थिर शून्य *thread_cleanup_राशिः_addr;
स्थिर size_t thread_cleanup_राशिः_sz;

स्थिर शून्य unmap_thread_राशिः(शून्य *addr, size_t sz)
{
	यदि (addr != रिक्त) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#यदि_अन्तः /* CONFIG_THREAD_STACK_MEM_MAPPED */

शून्य defer_thread_cleanup(संरचना k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_गन्तव्यम्_cleanup = thread;

#यदि_परिभाषितम् CONFIG_THREAD_राशिः_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_राशिः_addr = thread->राशिः_info.mapped.addr;
	thread_cleanup_राशिः_sz = thread->राशिः_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->राशिः_info.mapped.addr = रिक्त;
	thread->राशिः_info.mapped.sz = 0;
#यदि_अन्तः /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य do_deferred_thread_cleanup(शून्य)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#यदि_परिभाषितम् CONFIG_THREAD_राशिः_MEM_MAPPED
	unmap_thread_राशिः(thread_cleanup_राशिः_addr, thread_cleanup_राशिः_sz);

	thread_cleanup_राशिः_addr = रिक्त;
	thread_cleanup_राशिः_sz = 0;
#यदि_अन्तः /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य do_thread_cleanup(संरचना k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#यदि_परिभाषितम् CONFIG_THREAD_राशिः_MEM_MAPPED
	unmap_thread_राशिः(thread->राशिः_info.mapped.addr,
			   thread->राशिः_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->राशिः_info.mapped.addr = रिक्त;
	thread->राशिः_info.mapped.sz = 0;
#अन्यथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य k_thread_abort_cleanup(संरचना k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_तालकम्) {
		यदि (thread_गन्तव्यम्_cleanup != रिक्त) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_गन्तव्यम्_cleanup = रिक्त;
		}

		यदि (thread == _current) {
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
		} अन्यथा {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

शून्य k_thread_abort_cleanup_check_reuse(संरचना k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_तालकम्) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		यदि (thread_गन्तव्यम्_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_गन्तव्यम्_cleanup = रिक्त;
		}
	}
}

#यदि_अन्तः /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

शून्य z_dummy_thread_init(संरचना k_thread *dummy_thread)
{
	dummy_thread->आधारः.thread_state = _THREAD_DUMMY;
#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्
	dummy_thread->आधारः.cpu_आवरणम् = -1;
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->आधारः.user_options = K_ESSENTIAL;
#यदि Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = रिक्त;
	dummy_thread->orig_prio = dummy_thread->आधारः.prio;
#यदि_अन्तः /* Z_MUTEX_PI_ENABLED */
#यदि_परिभाषितम् CONFIG_THREAD_राशिः_INFO
	dummy_thread->राशिः_info.start = 0U;
	dummy_thread->राशिः_info.आकार = 0U;
#यदि_अन्तः /* CONFIG_THREAD_STACK_INFO */
#यदि_परिभाषितम् CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#यदि_अन्तः /* CONFIG_USERSPACE */
#यदि (K_HEAP_MEM_POOL_आकार > 0)
	k_thread_system_pool_नियोजय(dummy_thread);
#अन्यथा
	dummy_thread->resource_pool = रिक्त;
#यदि_अन्तः /* K_HEAP_MEM_POOL_SIZE */

#यदि_परिभाषितम् CONFIG_USE_SWITCH
	dummy_thread->switch_handle = रिक्त;
#यदि_अन्तः /* CONFIG_USE_SWITCH */

#यदि_परिभाषितम् CONFIG_TIMESLICE_प्रत्येकम्_THREAD
	dummy_thread->आधारः.खण्डिका_ticks = 0;
#यदि_अन्तः /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_समुच्चयः(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

शून्य z_impl_k_thread_प्राथमिकता_समुच्चयः(k_tid_t thread, पूर्णाङ्क prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, रिक्त);

	bool need_sched = z_thread_prio_समुच्चयः((संरचना k_thread *)thread, prio);

	यदि ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->आधारः.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_प्राथमिकता_समुच्चयः(k_tid_t thread, पूर्णाङ्क prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(_is_valid_prio(prio, रिक्त),
				    "invalid thread priority %d", prio));
#यदि_अपरिभाषितम् CONFIG_USERSPACE_THREAD_MAY_RAISE_प्राथमिकता
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः((int8_t)prio >= thread->आधारः.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->आधारः.prio));
#यदि_अन्तः /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_प्राथमिकता_समुच्चयः(thread, prio);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	यदि (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_अन्तर्_isr())) {
		z_thread_suspend_current(thread);
		प्रत्यागम;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_तालकम्();

	यदि (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(कुञ्चिका);
		प्रत्यागम;
	}

	z_thread_halt(thread, कुञ्चिका, false);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_suspend_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_तालकम्();

	/* Do not try to resume a thread that was not suspended */
	यदि (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(कुञ्चिका);
		प्रत्यागम;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(कुञ्चिका);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_resume_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य z_impl_k_wakeup(k_tid_t thread)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_तालकम्();

	यदि (z_is_thread_sleeping(thread)) {
		(शून्य)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(कुञ्चिका);
	} अन्यथा {
		z_sched_spinlock_unlock(कुञ्चिका);
	}
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_wakeup_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य z_thread_abort(संरचना k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_तालकम्();

	यदि (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(कुञ्चिका);
		प्रत्यागम;
	}

	z_thread_halt(thread, कुञ्चिका, true);

	यदि (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#यदि !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
शून्य z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#यदि_अन्तः /* !CONFIG_ARCH_HAS_THREAD_ABORT */

पूर्णाङ्क z_impl_k_thread_join(संरचना k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_तालकम्();
	पूर्णाङ्क ret;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	यदि (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} अन्यथा यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} अन्यथा यदि ((thread == _current) ||
		   (thread->आधारः.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} अन्यथा {
		__ASSERT(!arch_is_अन्तर्_isr(), "cannot join in ISR");
		z_sched_सङ्कलय_गन्तव्यम्_waitq_locked(_current, &thread->join_queue);
		z_सङ्कलय_thread_timeout(_current, timeout);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(कुञ्चिका);
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		प्रत्यागम ret;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(कुञ्चिका);
	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
स्थिर bool thread_obj_validate(संरचना k_thread *thread)
{
	संरचना k_object *ko = k_object_find(thread);
	पूर्णाङ्क ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	विकल्प (ret) {
	पक्ष 0:
		प्रत्यागम false;
	पक्ष -EINVAL:
		प्रत्यागम true;
	स्वभाव:
#यदि_परिभाषितम् CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#यदि_अन्तः /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_सन्देशः(ret == 0, "access denied"));
	}
	संहिता_UNREACHABLE; /* LCOV_EXCL_LINE */
}

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_join(संरचना k_thread *thread,
				       k_timeout_t timeout)
{
	यदि (thread_obj_validate(thread)) {
		प्रत्यागम 0;
	}

	प्रत्यागम z_impl_k_thread_join(thread, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_join_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_abort(k_tid_t thread)
{
	यदि (thread_obj_validate(thread)) {
		प्रत्यागम;
	}

	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((संरचना k_thread *)thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_abort_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

bool k_can_yield(शून्य)
{
	प्रत्यागम !(k_is_pre_kernel() || k_is_अन्तर्_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

शून्य z_impl_k_yield(शून्य)
{
	__ASSERT(!arch_is_अन्तर्_isr(), "");

	SYS_पत्तनम्_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_yield(शून्य)
{
	z_impl_k_yield();
}
#अन्तर्भाव <zephyr/syscalls/k_yield_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
