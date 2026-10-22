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

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
स्थिर संरचना k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = परिमाण(संरचना k_cycle_stats),
	.query_size = परिमाण(संरचना k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#अञ्ञथा
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    रिक्त);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(शून्य)
{
	पटिनिवत्त arch_is_in_isr();
}
EXPORT_चिह्नम्(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
शून्य z_impl_k_thread_custom_data_set(शून्य *मानम्)
{
	_current->custom_data = मानम्;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_custom_data_set(शून्य *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य *z_impl_k_thread_custom_data_get(शून्य)
{
	पटिनिवत्त _current->custom_data;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य *z_vrfy_k_thread_custom_data_get(शून्य)
{
	पटिनिवत्त z_impl_k_thread_custom_data_get();
}
#अन्तर्भाव <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

पूर्णाङ्क z_impl_k_is_preempt_thread(शून्य)
{
	पटिनिवत्त !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_is_preempt_thread(शून्य)
{
	पटिनिवत्त z_impl_k_is_preempt_thread();
}
#अन्तर्भाव <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_thread_priority_get(k_tid_t thread)
{
	पटिनिवत्त thread->base.prio;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	पटिनिवत्त z_impl_k_thread_priority_get(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
स्थिर शून्य set_thread_name(संरचना k_thread *thread, ध्रुव वर्ण *str)
{
	सचे (str != रिक्त) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} अञ्ञथा {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

पूर्णाङ्क z_impl_k_thread_name_set(k_tid_t thread, ध्रुव वर्ण *str)
{
#ifdef CONFIG_THREAD_NAME
	सचे (thread == रिक्त) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	पटिनिवत्त 0;
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	पटिनिवत्त -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_name_set(k_tid_t thread, ध्रुव वर्ण *str)
{
#ifdef CONFIG_THREAD_NAME
	वर्ण name[CONFIG_THREAD_MAX_NAME_LEN];

	सचे (thread != रिक्त) {
		सचे (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			पटिनिवत्त -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	सचे (k_usermode_string_copy(name, str, परिमाण(name)) != 0) {
		पटिनिवत्त -EFAULT;
	}

	पटिनिवत्त z_impl_k_thread_name_set(thread, name);
#अञ्ञथा
	पटिनिवत्त -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#अन्तर्भाव <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ध्रुव वर्ण *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	पटिनिवत्त (ध्रुव वर्ण *)thread->name;
#अञ्ञथा
	ARG_UNUSED(thread);
	पटिनिवत्त रिक्त;
#endif /* CONFIG_THREAD_NAME */
}

पूर्णाङ्क z_impl_k_thread_name_copy(k_tid_t thread, वर्ण *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	पटिनिवत्त 0;
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	पटिनिवत्त -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

स्थिर size_t copy_bytes(वर्ण *dest, size_t dest_size, ध्रुव वर्ण *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	पटिनिवत्त bytes_to_copy;
}

ध्रुव वर्ण *k_thread_state_str(k_tid_t thread_id, वर्ण *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, परिमाण(Z_STATE_STR_##s) - 1 }
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
#undef SS_ENT

	सचे ((buf == रिक्त) || (buf_size == 0)) {
		पटिनिवत्त "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	अत्थाय (unsigned पूर्णाङ्क index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		सचे ((thread_state & bit) == 0) {
			अनुवर्त;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		सचे (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	पटिनिवत्त (ध्रुव वर्ण *)buf;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_name_copy(k_tid_t thread,
					    वर्ण *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	संरचना k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	सचे ((ko == रिक्त) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		पटिनिवत्त -EINVAL;
	}
	सचे (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		पटिनिवत्त -EFAULT;
	}
	len = strlen(thread->name);
	सचे ((len + 1) > size) {
		पटिनिवत्त -ENOSPC;
	}

	पटिनिवत्त k_usermode_to_copy((शून्य *)buf, thread->name, len + 1);
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	पटिनिवत्त -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#अन्तर्भाव <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_राशिः_SENTINEL
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

	सचे ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		पटिनिवत्त;
	}

	राशिः = (uint32_t *)_current->राशिः_info.start;
	सचे (*राशिः != राशिः_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*राशिः = राशिः_SENTINEL;
		z_except_reason(K_ERR_राशिः_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#सचे defined(CONFIG_राशिः_POINTER_RANDOM) && (CONFIG_राशिः_POINTER_RANDOM != 0)
पूर्णाङ्क z_राशिः_adjust_initialized;

स्थिर size_t random_offset(size_t राशिः_size)
{
	size_t random_val;

	सचे (!z_राशिः_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, परिमाण(random_val));
	} अञ्ञथा {
		sys_rand_get((uint8_t *)&random_val, परिमाण(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ध्रुव size_t fuzz = random_val % CONFIG_राशिः_POINTER_RANDOM;

	सचे (unlikely(fuzz * 2 > राशिः_size)) {
		पटिनिवत्त 0;
	}

	पटिनिवत्त fuzz;
}
#सचे defined(CONFIG_राशिः_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
संरचना राशिः_geometry {
	size_t obj_size;	/* size of the whole stack object */
	वर्ण *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
स्थिर संरचना राशिः_geometry compute_राशिः_geometry(k_thread_राशिः_t *राशिः,
						    size_t राशिः_size)
{
	संरचना राशिः_geometry geo;

#ifdef CONFIG_USERSPACE
	सचे (z_राशिः_is_user_capable(राशिः)) {
		geo.obj_size = K_THREAD_राशिः_LEN(राशिः_size);
		geo.buf_start = K_THREAD_राशिः_BUFFER(राशिः);
		geo.buf_size = geo.obj_size - K_THREAD_राशिः_RESERVED;
	} अञ्ञथा
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_राशिः_LEN(राशिः_size);
		geo.buf_start = K_KERNEL_राशिः_BUFFER(राशिः);
		geo.buf_size = geo.obj_size - K_KERNEL_राशिः_RESERVED;

#सचे defined(ARCH_KERNEL_राशिः_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		सचे (K_KERNEL_राशिः_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	पटिनिवत्त geo;
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

#ifdef CONFIG_THREAD_राशिः_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	शून्य *राशिः_mapped = k_mem_map_phys_guard((uintptr_t)राशिः, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)राशिः_mapped != 0);

#ifdef CONFIG_USERSPACE
	सचे (z_राशिः_is_user_capable(राशिः)) {
		geo->buf_start = K_THREAD_राशिः_BUFFER(राशिः_mapped);
	} अञ्ञथा
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_राशिः_BUFFER(राशिः_mapped);
	}

	राशिः_ptr = (वर्ण *)राशिः_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->राशिः_info.mapped.addr = राशिः_mapped;
	new_thread->राशिः_info.mapped.sz = geo->obj_size;
#अञ्ञथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	राशिः_ptr = (वर्ण *)राशिः + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	पटिनिवत्त राशिः_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
स्थिर अन्तःस्थ शून्य fill_init_राशिः(वर्ण *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#अञ्ञथा
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
स्थिर अन्तःस्थ शून्य set_राशिः_sentinel(वर्ण *buf_start)
{
#ifdef CONFIG_राशिः_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = राशिः_SENTINEL;
#अञ्ञथा
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
स्थिर size_t reserve_राशिः_headroom(संरचना k_thread *new_thread,
				     वर्ण *राशिः_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(राशिः_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_राशिः_setup(new_thread, राशिः_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = परिमाण(संरचना _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(संरचना _thread_userspace_local_data *)(राशिः_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#सचे defined(CONFIG_राशिः_POINTER_RANDOM) && (CONFIG_राशिः_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	पटिनिवत्त ROUND_UP(delta, ARCH_राशिः_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
स्थिर अन्तःस्थ शून्य set_राशिः_info(संरचना k_thread *new_thread, वर्ण *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_राशिः_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->राशिः_info.start = (uintptr_t)buf_start;
	new_thread->राशिः_info.size = buf_size;
	new_thread->राशिः_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_राशिः_SAFETY
	new_thread->राशिः_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_राशिः_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#अञ्ञथा
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

स्थिर वर्ण *setup_thread_राशिः(संरचना k_thread *new_thread,
				k_thread_राशिः_t *राशिः, size_t राशिः_size)
{
	संरचना राशिः_geometry geo = compute_राशिः_geometry(राशिः, राशिः_size);
	वर्ण *राशिः_ptr = map_thread_राशिः(new_thread, राशिः, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		राशिः, new_thread, geo.obj_size, (शून्य *)geo.buf_start,
		geo.buf_size, (शून्य *)राशिः_ptr);

	fill_init_राशिः(geo.buf_start, geo.buf_size);
	set_राशिः_sentinel(geo.buf_start);
	delta = reserve_राशिः_headroom(new_thread, राशिः_ptr, geo.buf_size);
	set_राशिः_info(new_thread, geo.buf_start, geo.buf_size, delta);

	पटिनिवत्त राशिः_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_राशिः
स्थिर शून्य setup_shadow_राशिः(संरचना k_thread *new_thread,
			 k_thread_राशिः_t *राशिः)
{
	पूर्णाङ्क ret = -ENOENT;

	STRUCT_SECTION_FOREACH(राशिः_to_hw_shadow_राशिः, stk_to_hw_shstk) {
		सचे (stk_to_hw_shstk->राशिः == राशिः) {
			ret = k_thread_hw_shadow_राशिः_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			सचे (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			भङ्ग;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	सचे (ret != -ENOENT) {
		पटिनिवत्त;
	}

	STRUCT_SECTION_FOREACH(राशिः_to_hw_shadow_राशिः_arr, stk_to_hw_shstk) {
		सचे ((uintptr_t)राशिः >= stk_to_hw_shstk->राशिः_addr &&
		    (uintptr_t)राशिः < stk_to_hw_shstk->राशिः_addr +
		    stk_to_hw_shstk->राशिः_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t राशिः_offset = (uintptr_t)राशिः - stk_to_hw_shstk->राशिः_addr;
			uintptr_t राशिः_index = राशिः_offset / stk_to_hw_shstk->राशिः_size;
			uintptr_t addr;

			सचे (राशिः_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, राशिः);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * राशिः_index;
			ret = k_thread_hw_shadow_राशिः_attach(new_thread,
							      (arch_thread_hw_shadow_राशिः_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			सचे (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			भङ्ग;
		}
	}

	सचे (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, राशिः);
		k_panic();
	}
}
#अञ्ञथा
/* No-op stub used when hardware shadow stacks are not configured. */
स्थिर अन्तःस्थ शून्य setup_shadow_राशिः(संरचना k_thread *new_thread,
				      k_thread_राशिः_t *राशिः)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(राशिः);
}
#endif

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
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#अञ्ञथा
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
स्थिर अन्तःस्थ शून्य init_thread_obj_core(संरचना k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  परिमाण(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#अञ्ञथा
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
स्थिर अन्तःस्थ शून्य init_thread_userspace(संरचना k_thread *thread,
					 k_thread_राशिः_t *राशिः,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_राशिः_is_user_capable(राशिः),
		 "user thread %p with kernel-only stack %p", thread, राशिः);
	k_object_init(thread);
	k_object_init(राशिः);
	thread->राशिः_obj = राशिः;
	thread->syscall_frame = रिक्त;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(राशिः);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
स्थिर अन्तःस्थ शून्य assert_thread_coherence(संरचना k_thread *thread,
					   k_thread_राशिः_t *राशिः)
{
#ifdef CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#ifndef CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(राशिः));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(राशिः);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
स्थिर अन्तःस्थ शून्य assert_switch_handle(संरचना k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != रिक्त,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
स्थिर अन्तःस्थ शून्य init_thread_custom_data(संरचना k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = रिक्त;
#अञ्ञथा
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
स्थिर अन्तःस्थ शून्य add_thread_to_monitor(संरचना k_thread *thread,
					 k_thread_entry_t entry,
					 शून्य *p1, शून्य *p2, शून्य *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, कुञ्चिका);
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
स्थिर अन्तःस्थ शून्य init_thread_name(संरचना k_thread *thread, ध्रुव वर्ण *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
स्थिर अन्तःस्थ शून्य init_thread_cpu_mask(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	सचे (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} अञ्ञथा {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#अञ्ञथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
स्थिर अन्तःस्थ शून्य init_thread_userspace_perms(संरचना k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	सचे ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
स्थिर अन्तःस्थ शून्य init_thread_deadline(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#अञ्ञथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
स्थिर अन्तःस्थ शून्य init_thread_halt_queue(संरचना k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#अञ्ञथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
स्थिर अन्तःस्थ शून्य init_thread_usage(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (संरचना k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#अञ्ञथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
स्थिर अन्तःस्थ शून्य assert_thread_not_reused(संरचना k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	अत्थाय (संरचना k_thread *t = _kernel.threads; t; t = t->next_thread) {
		सचे (t == new_thread) {
			reused = true;
			भङ्ग;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, कुञ्चिका);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#endif /* CONFIG_ASSERT */
#endif /* CONFIG_THREAD_MONITOR */
	ARG_UNUSED(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
वर्ण *z_setup_new_thread(संरचना k_thread *new_thread,
			 k_thread_राशिः_t *राशिः, size_t राशिः_size,
			 k_thread_entry_t entry,
			 शून्य *p1, शून्य *p2, शून्य *p3,
			 पूर्णाङ्क prio, uint32_t options, ध्रुव वर्ण *name)
{
	वर्ण *राशिः_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, राशिः, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	राशिः_ptr = setup_thread_राशिः(new_thread, राशिः, राशिः_size);

#सचे Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = रिक्त;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_राशिः(new_thread, राशिः);
	assert_thread_coherence(new_thread, राशिः);

	arch_new_thread(new_thread, राशिः, राशिः_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = रिक्त;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	सचे (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == रिक्त)) {
		new_thread->resource_pool = रिक्त;
		पटिनिवत्त राशिः_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	पटिनिवत्त राशिः_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_राशिः_SAFETY
पूर्णाङ्क z_impl_k_thread_runtime_राशिः_unused_threshold_pct_set(संरचना k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	सचे (pct > 99) {
		पटिनिवत्त -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->राशिः_info.size * pct) / 100;

	thread->राशिः_info.usage.unused_threshold = unused_threshold;

	पटिनिवत्त 0;
}

पूर्णाङ्क z_impl_k_thread_runtime_राशिः_unused_threshold_set(संरचना k_thread *thread,
						       size_t threshold)
{
	सचे (threshold > thread->राशिः_info.size) {
		पटिनिवत्त -EINVAL;
	}

	thread->राशिः_info.usage.unused_threshold = threshold;

	पटिनिवत्त 0;
}

size_t z_impl_k_thread_runtime_राशिः_unused_threshold_get(संरचना k_thread *thread)
{
	पटिनिवत्त thread->राशिः_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_thread_runtime_राशिः_unused_threshold_pct_set(संरचना k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	पटिनिवत्त z_impl_k_thread_runtime_राशिः_unused_threshold_pct_set(thread, pct);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

पूर्णाङ्क z_vrfy_k_thread_runtime_राशिः_unused_threshold_set(संरचना k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	पटिनिवत्त z_impl_k_thread_runtime_राशिः_unused_threshold_set(thread, threshold);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_राशिः_unused_threshold_get(संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	पटिनिवत्त z_impl_k_thread_runtime_राशिः_unused_threshold_get(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(संरचना k_thread *new_thread,
			      k_thread_राशिः_t *राशिः,
			      size_t राशिः_size, k_thread_entry_t entry,
			      शून्य *p1, शून्य *p2, शून्य *p3,
			      पूर्णाङ्क prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, राशिः, राशिः_size, entry, p1, p2, p3,
			  prio, options, रिक्त);

	thread_schedule_new(new_thread, delay);

	पटिनिवत्त new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_राशिः_is_user_capable(k_thread_राशिः_t *राशिः)
{
	पटिनिवत्त k_object_find(राशिः) != रिक्त;
}

k_tid_t z_vrfy_k_thread_create(संरचना k_thread *new_thread,
			       k_thread_राशिः_t *राशिः,
			       size_t राशिः_size, k_thread_entry_t entry,
			       शून्य *p1, शून्य *p2, शून्य *p3,
			       पूर्णाङ्क prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, राशिः_obj_size;
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
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(!size_add_overflow(K_THREAD_राशिः_RESERVED,
						       राशिः_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    राशिः_size,
				    K_THREAD_राशिः_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	राशिः_obj_size = राशिः_object->data.राशिः_data->size;
#अञ्ञथा
	राशिः_obj_size = राशिः_object->data.राशिः_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(total_size <= राशिः_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, राशिः_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, रिक्त)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, राशिः, राशिः_size,
			   entry, p1, p2, p3, prio, options, रिक्त);

	thread_schedule_new(new_thread, delay);

	पटिनिवत्त new_thread;
}
#अन्तर्भाव <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_init_thread_base(संरचना _thread_base *thread_base, पूर्णाङ्क priority,
		       uint32_t initial_state, unsigned पूर्णाङ्क options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = रिक्त;
	thread_base->user_options = (uint16_t)options;
	thread_base->thread_state = (uint8_t)initial_state;

	thread_base->prio = priority;

	thread_base->sched_locked = 0U;

#ifdef CONFIG_SMP
	thread_base->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_base->cpu = 0;
#endif /* CONFIG_SMP */

#ifdef CONFIG_TIMESLICE_PER_THREAD
	thread_base->slice_ticks = 0;
	thread_base->slice_expired = रिक्त;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = रिक्त;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN शून्य k_thread_user_mode_enter(k_thread_entry_t entry,
					    शून्य *p1, शून्य *p2, शून्य *p3)
{
	SYS_PORT_TRACING_FUNC(k_thread, user_mode_enter);

	_current->base.user_options |= K_USER;
	z_thread_essential_clear(_current);
#ifdef CONFIG_THREAD_MONITOR
	_current->entry.pEntry = entry;
	_current->entry.parameter1 = p1;
	_current->entry.parameter2 = p2;
	_current->entry.parameter3 = p3;
#endif /* CONFIG_THREAD_MONITOR */
#ifdef CONFIG_USERSPACE
	__ASSERT(z_राशिः_is_user_capable(_current->राशिः_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       परिमाण(संरचना _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_राशिः_setup(_current,
			     (वर्ण *)(_current->राशिः_info.start +
				      _current->राशिः_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#अञ्ञथा
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#सचे defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_राशिः_INFO)
#ifdef CONFIG_राशिः_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

पूर्णाङ्क z_राशिः_space_get(ध्रुव uint8_t *राशिः_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
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
	सचे ((राशिः_pointer > राशिः_start) && (राशिः_pointer <= (राशिः_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_राशिः_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		पटिनिवत्त -ENOTSUP;
	}

	सचे (IS_ENABLED(CONFIG_राशिः_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_राशिः += 4;
		size -= 4;
	}

	अत्थाय (size_t i = 0; i < size; i++) {
		सचे ((checked_राशिः[i]) == 0xaaU) {
			unused++;
		} अञ्ञथा {
			भङ्ग;
		}
	}

	*unused_ptr = unused;

	पटिनिवत्त 0;
}

#ifdef CONFIG_THREAD_RUNTIME_राशिः_SAFETY
पूर्णाङ्क k_thread_runtime_राशिः_safety_full_check(ध्रुव संरचना k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_राशिः_safety_handler_t handler,
					     शून्य *arg)
{
	पूर्णाङ्क    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != रिक्त);

	rv = z_राशिः_space_get((ध्रुव uint8_t *)thread->राशिः_info.start,
			       thread->राशिः_info.size, &unused_space);

	सचे (rv != 0) {
		पटिनिवत्त rv;
	}

	सचे (unused_ptr != रिक्त) {
		*unused_ptr = unused_space;
	}

	सचे ((unused_space < thread->राशिः_info.usage.unused_threshold) &&
	    (handler != रिक्त)) {
		handler(thread, unused_space, arg);
	}

	पटिनिवत्त 0;
}

पूर्णाङ्क k_thread_runtime_राशिः_safety_threshold_check(ध्रुव संरचना k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_राशिः_safety_handler_t handler,
						  शून्य *arg)
{
	पूर्णाङ्क    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != रिक्त);

	threshold = thread->राशिः_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	सचे (IS_ENABLED(CONFIG_राशिः_SENTINEL) && (threshold != 0U)) {
		scan_size += परिमाण(uint32_t);
	}

	rv = z_राशिः_space_get((ध्रुव uint8_t *)thread->राशिः_info.start,
			       scan_size, &unused_space);

	सचे (rv != 0) {
		पटिनिवत्त rv;
	}

	सचे (unused_ptr != रिक्त) {
		*unused_ptr = unused_space;
	}

	सचे ((unused_space < threshold) && (handler != रिक्त)) {
		handler(thread, unused_space, arg);
	}

	पटिनिवत्त 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

पूर्णाङ्क z_impl_k_thread_राशिः_space_get(ध्रुव संरचना k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_राशिः_MEM_MAPPED
	सचे (thread->राशिः_info.mapped.addr == रिक्त) {
		पटिनिवत्त -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	पटिनिवत्त z_राशिः_space_get((ध्रुव uint8_t *)thread->राशिः_info.start,
				 thread->राशिः_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_thread_राशिः_space_get(ध्रुव संरचना k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	पूर्णाङ्क ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_राशिः_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		पटिनिवत्त ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, परिमाण(size_t));
	CHECKIF(ret != 0) {
		पटिनिवत्त ret;
	}

	पटिनिवत्त 0;
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    ध्रुव संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	पटिनिवत्त z_impl_k_thread_timeout_remaining_ticks(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

स्थिर अन्तःस्थ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ध्रुव संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	पटिनिवत्त z_impl_k_thread_timeout_expires_ticks(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
शून्य z_thread_mark_switched_in(शून्य)
{
#सचे defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

शून्य z_thread_mark_switched_out(शून्य)
{
#सचे defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	सचे (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		पटिनिवत्त;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

पूर्णाङ्क k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	सचे ((thread == रिक्त) || (stats == रिक्त)) {
		पटिनिवत्त -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#अञ्ञथा
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	पटिनिवत्त 0;
}

पूर्णाङ्क k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	सचे (stats == रिक्त) {
		पटिनिवत्त -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned पूर्णाङ्क अङ्कः_cpus = arch_अङ्कः_cpus();

	अत्थाय (unsigned पूर्णाङ्क i = 0; i < अङ्कः_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_cycles += tmp_stats.execution_cycles;
		stats->total_cycles     += tmp_stats.total_cycles;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_cycles   += tmp_stats.current_cycles;
		stats->peak_cycles      += tmp_stats.peak_cycles;
		stats->average_cycles   += tmp_stats.average_cycles;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_cycles      += tmp_stats.idle_cycles;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	पटिनिवत्त 0;
}

पूर्णाङ्क k_thread_runtime_stats_cpu_get(पूर्णाङ्क cpu, k_thread_runtime_stats_t *stats)
{
	सचे (stats == रिक्त) {
		पटिनिवत्त -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned पूर्णाङ्क)cpu >= arch_अङ्कः_cpus())) {
		पटिनिवत्त -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#अञ्ञथा
	ARG_UNUSED(cpu);
#endif

	पटिनिवत्त 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
स्थिर संरचना k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
स्थिर संरचना k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_राशिः_MEM_MAPPED
स्थिर शून्य *thread_cleanup_राशिः_addr;
स्थिर size_t thread_cleanup_राशिः_sz;

स्थिर शून्य unmap_thread_राशिः(शून्य *addr, size_t sz)
{
	सचे (addr != रिक्त) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

शून्य defer_thread_cleanup(संरचना k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_राशिः_MEM_MAPPED
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
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य do_deferred_thread_cleanup(शून्य)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_राशिः_MEM_MAPPED
	unmap_thread_राशिः(thread_cleanup_राशिः_addr, thread_cleanup_राशिः_sz);

	thread_cleanup_राशिः_addr = रिक्त;
	thread_cleanup_राशिः_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य do_thread_cleanup(संरचना k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_राशिः_MEM_MAPPED
	unmap_thread_राशिः(thread->राशिः_info.mapped.addr,
			   thread->राशिः_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->राशिः_info.mapped.addr = रिक्त;
	thread->राशिः_info.mapped.sz = 0;
#अञ्ञथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य k_thread_abort_cleanup(संरचना k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		सचे (thread_to_cleanup != रिक्त) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = रिक्त;
		}

		सचे (thread == _current) {
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
		} अञ्ञथा {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

शून्य k_thread_abort_cleanup_check_reuse(संरचना k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		सचे (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = रिक्त;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

शून्य z_dummy_thread_init(संरचना k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#सचे Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = रिक्त;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_राशिः_INFO
	dummy_thread->राशिः_info.start = 0U;
	dummy_thread->राशिः_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#सचे (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#अञ्ञथा
	dummy_thread->resource_pool = रिक्त;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = रिक्त;
#endif /* CONFIG_USE_SWITCH */

#ifdef CONFIG_TIMESLICE_PER_THREAD
	dummy_thread->base.slice_ticks = 0;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_set(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

शून्य z_impl_k_thread_priority_set(k_tid_t thread, पूर्णाङ्क prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, रिक्त);

	bool need_sched = z_thread_prio_set((संरचना k_thread *)thread, prio);

	सचे ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_priority_set(k_tid_t thread, पूर्णाङ्क prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(_is_valid_prio(prio, रिक्त),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	सचे (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		पटिनिवत्त;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_lock();

	सचे (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(कुञ्चिका);
		पटिनिवत्त;
	}

	z_thread_halt(thread, कुञ्चिका, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	सचे (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(कुञ्चिका);
		पटिनिवत्त;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(कुञ्चिका);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_lock();

	सचे (z_is_thread_sleeping(thread)) {
		(शून्य)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(कुञ्चिका);
	} अञ्ञथा {
		z_sched_spinlock_unlock(कुञ्चिका);
	}
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_thread_abort(संरचना k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_lock();

	सचे (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(कुञ्चिका);
		पटिनिवत्त;
	}

	z_thread_halt(thread, कुञ्चिका, true);

	सचे (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#सचे !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
शून्य z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

पूर्णाङ्क z_impl_k_thread_join(संरचना k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_lock();
	पूर्णाङ्क ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	सचे (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} अञ्ञथा सचे (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} अञ्ञथा सचे ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} अञ्ञथा {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(कुञ्चिका);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		पटिनिवत्त ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(कुञ्चिका);
	पटिनिवत्त ret;
}

#ifdef CONFIG_USERSPACE
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
		पटिनिवत्त false;
	पक्ष -EINVAL:
		पटिनिवत्त true;
	स्वभाव:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_सन्देशः(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_join(संरचना k_thread *thread,
				       k_timeout_t timeout)
{
	सचे (thread_obj_validate(thread)) {
		पटिनिवत्त 0;
	}

	पटिनिवत्त z_impl_k_thread_join(thread, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_join_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_thread_abort(k_tid_t thread)
{
	सचे (thread_obj_validate(thread)) {
		पटिनिवत्त;
	}

	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((संरचना k_thread *)thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(शून्य)
{
	पटिनिवत्त !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

शून्य z_impl_k_yield(शून्य)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_yield(शून्य)
{
	z_impl_k_yield();
}
#अन्तर्भाव <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
