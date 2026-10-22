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
	.raw_size = आकार(संरचना k_cycle_stats),
	.query_size = आकार(संरचना k_thread_runtime_stats),
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
#अन्यथा
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    रिक्त);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(शून्य)
{
	परत arch_is_in_isr();
}
EXPORT_चिन्ह(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
शून्य z_impl_k_thread_custom_data_set(शून्य *मूल्य)
{
	_current->custom_data = मूल्य;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_thread_custom_data_set(शून्य *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य *z_impl_k_thread_custom_data_get(शून्य)
{
	परत _current->custom_data;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य *z_vrfy_k_thread_custom_data_get(शून्य)
{
	परत z_impl_k_thread_custom_data_get();
}
#अन्तर्भाव <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

पूर्णांक z_impl_k_is_preempt_thread(शून्य)
{
	परत !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_is_preempt_thread(शून्य)
{
	परत z_impl_k_is_preempt_thread();
}
#अन्तर्भाव <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_thread_priority_get(k_tid_t thread)
{
	परत thread->base.prio;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	परत z_impl_k_thread_priority_get(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
स्थिर शून्य set_thread_name(संरचना k_thread *thread, स्थिरांक अक्षर *str)
{
	जर (str != रिक्त) {
		म_नकलन(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} अन्यथा {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

पूर्णांक z_impl_k_thread_name_set(k_tid_t thread, स्थिरांक अक्षर *str)
{
#ifdef CONFIG_THREAD_NAME
	जर (thread == रिक्त) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	परत 0;
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	परत -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_thread_name_set(k_tid_t thread, स्थिरांक अक्षर *str)
{
#ifdef CONFIG_THREAD_NAME
	अक्षर name[CONFIG_THREAD_MAX_NAME_LEN];

	जर (thread != रिक्त) {
		जर (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			परत -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	जर (k_usermode_string_copy(name, str, आकार(name)) != 0) {
		परत -EFAULT;
	}

	परत z_impl_k_thread_name_set(thread, name);
#अन्यथा
	परत -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#अन्तर्भाव <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

स्थिरांक अक्षर *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	परत (स्थिरांक अक्षर *)thread->name;
#अन्यथा
	ARG_UNUSED(thread);
	परत रिक्त;
#endif /* CONFIG_THREAD_NAME */
}

पूर्णांक z_impl_k_thread_name_copy(k_tid_t thread, अक्षर *buf, माप_प्रकार size)
{
#ifdef CONFIG_THREAD_NAME
	म_नकलन(buf, thread->name, size);
	परत 0;
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	परत -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

स्थिर माप_प्रकार copy_bytes(अक्षर *dest, माप_प्रकार dest_size, स्थिरांक अक्षर *src, माप_प्रकार src_size)
{
	माप_प्रकार  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	स_नकल(dest, src, bytes_to_copy);

	परत bytes_to_copy;
}

स्थिरांक अक्षर *k_thread_state_str(k_tid_t thread_id, अक्षर *buf, माप_प्रकार buf_size)
{
	माप_प्रकार      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, आकार(Z_STATE_STR_##s) - 1 }
	स्थिर स्थिरांक संरचना {
		स्थिरांक अक्षर *str;
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

	जर ((buf == रिक्त) || (buf_size == 0)) {
		परत "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	साठी (अचिन्हित पूर्णांक index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		जर ((thread_state & bit) == 0) {
			सुरूठेवा;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		जर (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	परत (स्थिरांक अक्षर *)buf;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_thread_name_copy(k_tid_t thread,
					    अक्षर *buf, माप_प्रकार size)
{
#ifdef CONFIG_THREAD_NAME
	माप_प्रकार len;
	संरचना k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	जर ((ko == रिक्त) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		परत -EINVAL;
	}
	जर (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		परत -EFAULT;
	}
	len = म_माप(thread->name);
	जर ((len + 1) > size) {
		परत -ENOSPC;
	}

	परत k_usermode_to_copy((शून्य *)buf, thread->name, len + 1);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	परत -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#अन्तर्भाव <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_चळत_SENTINEL
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
शून्य z_check_चळत_sentinel(शून्य)
{
	uint32_t *चळत;

	जर ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		परत;
	}

	चळत = (uint32_t *)_current->चळत_info.start;
	जर (*चळत != चळत_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*चळत = चळत_SENTINEL;
		z_except_reason(K_ERR_चळत_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#जर defined(CONFIG_चळत_POINTER_RANDOM) && (CONFIG_चळत_POINTER_RANDOM != 0)
पूर्णांक z_चळत_adjust_initialized;

स्थिर माप_प्रकार random_offset(माप_प्रकार चळत_size)
{
	माप_प्रकार random_val;

	जर (!z_चळत_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, आकार(random_val));
	} अन्यथा {
		sys_rand_get((uint8_t *)&random_val, आकार(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	स्थिरांक माप_प्रकार fuzz = random_val % CONFIG_चळत_POINTER_RANDOM;

	जर (unlikely(fuzz * 2 > चळत_size)) {
		परत 0;
	}

	परत fuzz;
}
#जर defined(CONFIG_चळत_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
संरचना चळत_geometry {
	माप_प्रकार obj_size;	/* size of the whole stack object */
	अक्षर *buf_start;	/* start of the usable stack buffer */
	माप_प्रकार buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
स्थिर संरचना चळत_geometry compute_चळत_geometry(k_thread_चळत_t *चळत,
						    माप_प्रकार चळत_size)
{
	संरचना चळत_geometry geo;

#ifdef CONFIG_USERSPACE
	जर (z_चळत_is_user_capable(चळत)) {
		geo.obj_size = K_THREAD_चळत_LEN(चळत_size);
		geo.buf_start = K_THREAD_चळत_BUFFER(चळत);
		geo.buf_size = geo.obj_size - K_THREAD_चळत_RESERVED;
	} अन्यथा
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_चळत_LEN(चळत_size);
		geo.buf_start = K_KERNEL_चळत_BUFFER(चळत);
		geo.buf_size = geo.obj_size - K_KERNEL_चळत_RESERVED;

#जर defined(ARCH_KERNEL_चळत_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		जर (K_KERNEL_चळत_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	परत geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
स्थिर अक्षर *map_thread_चळत(संरचना k_thread *new_thread,
			      k_thread_चळत_t *चळत,
			      संरचना चळत_geometry *geo)
{
	अक्षर *चळत_ptr;

#ifdef CONFIG_THREAD_चळत_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	शून्य *चळत_mapped = k_mem_map_phys_guard((uintptr_t)चळत, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)चळत_mapped != 0);

#ifdef CONFIG_USERSPACE
	जर (z_चळत_is_user_capable(चळत)) {
		geo->buf_start = K_THREAD_चळत_BUFFER(चळत_mapped);
	} अन्यथा
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_चळत_BUFFER(चळत_mapped);
	}

	चळत_ptr = (अक्षर *)चळत_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->चळत_info.mapped.addr = चळत_mapped;
	new_thread->चळत_info.mapped.sz = geo->obj_size;
#अन्यथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	चळत_ptr = (अक्षर *)चळत + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	परत चळत_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
स्थिर ओळीत शून्य fill_init_चळत(अक्षर *buf_start, माप_प्रकार buf_size)
{
#ifdef CONFIG_INIT_STACKS
	स_रखो(buf_start, 0xaa, buf_size);
#अन्यथा
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
स्थिर ओळीत शून्य set_चळत_sentinel(अक्षर *buf_start)
{
#ifdef CONFIG_चळत_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = चळत_SENTINEL;
#अन्यथा
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
स्थिर माप_प्रकार reserve_चळत_headroom(संरचना k_thread *new_thread,
				     अक्षर *चळत_ptr, माप_प्रकार buf_size)
{
	माप_प्रकार delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(चळत_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_चळत_setup(new_thread, चळत_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	माप_प्रकार tls_size = आकार(संरचना _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(संरचना _thread_userspace_local_data *)(चळत_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#जर defined(CONFIG_चळत_POINTER_RANDOM) && (CONFIG_चळत_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	परत ROUND_UP(delta, ARCH_चळत_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
स्थिर ओळीत शून्य set_चळत_info(संरचना k_thread *new_thread, अक्षर *buf_start,
				  माप_प्रकार buf_size, माप_प्रकार delta)
{
#ifdef CONFIG_THREAD_चळत_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->चळत_info.start = (uintptr_t)buf_start;
	new_thread->चळत_info.size = buf_size;
	new_thread->चळत_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_चळत_SAFETY
	new_thread->चळत_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_चळत_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#अन्यथा
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

स्थिर अक्षर *setup_thread_चळत(संरचना k_thread *new_thread,
				k_thread_चळत_t *चळत, माप_प्रकार चळत_size)
{
	संरचना चळत_geometry geo = compute_चळत_geometry(चळत, चळत_size);
	अक्षर *चळत_ptr = map_thread_चळत(new_thread, चळत, &geo);
	माप_प्रकार delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		चळत, new_thread, geo.obj_size, (शून्य *)geo.buf_start,
		geo.buf_size, (शून्य *)चळत_ptr);

	fill_init_चळत(geo.buf_start, geo.buf_size);
	set_चळत_sentinel(geo.buf_start);
	delta = reserve_चळत_headroom(new_thread, चळत_ptr, geo.buf_size);
	set_चळत_info(new_thread, geo.buf_start, geo.buf_size, delta);

	परत चळत_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_चळत
स्थिर शून्य setup_shadow_चळत(संरचना k_thread *new_thread,
			 k_thread_चळत_t *चळत)
{
	पूर्णांक ret = -ENOENT;

	STRUCT_SECTION_FOREACH(चळत_to_hw_shadow_चळत, stk_to_hw_shstk) {
		जर (stk_to_hw_shstk->चळत == चळत) {
			ret = k_thread_hw_shadow_चळत_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			जर (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			थांबा;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	जर (ret != -ENOENT) {
		परत;
	}

	STRUCT_SECTION_FOREACH(चळत_to_hw_shadow_चळत_arr, stk_to_hw_shstk) {
		जर ((uintptr_t)चळत >= stk_to_hw_shstk->चळत_addr &&
		    (uintptr_t)चळत < stk_to_hw_shstk->चळत_addr +
		    stk_to_hw_shstk->चळत_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t चळत_offset = (uintptr_t)चळत - stk_to_hw_shstk->चळत_addr;
			uintptr_t चळत_index = चळत_offset / stk_to_hw_shstk->चळत_size;
			uintptr_t addr;

			जर (चळत_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, चळत);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * चळत_index;
			ret = k_thread_hw_shadow_चळत_attach(new_thread,
							      (arch_thread_hw_shadow_चळत_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			जर (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			थांबा;
		}
	}

	जर (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, चळत);
		k_panic();
	}
}
#अन्यथा
/* No-op stub used when hardware shadow stacks are not configured. */
स्थिर ओळीत शून्य setup_shadow_चळत(संरचना k_thread *new_thread,
				      k_thread_चळत_t *चळत)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(चळत);
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
स्थिर ओळीत शून्य thread_abort_cleanup_check_reuse(संरचना k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#अन्यथा
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
स्थिर ओळीत शून्य init_thread_obj_core(संरचना k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  आकार(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#अन्यथा
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
स्थिर ओळीत शून्य init_thread_userspace(संरचना k_thread *thread,
					 k_thread_चळत_t *चळत,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_चळत_is_user_capable(चळत),
		 "user thread %p with kernel-only stack %p", thread, चळत);
	k_object_init(thread);
	k_object_init(चळत);
	thread->चळत_obj = चळत;
	thread->syscall_frame = रिक्त;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(चळत);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
स्थिर ओळीत शून्य assert_thread_coherence(संरचना k_thread *thread,
					   k_thread_चळत_t *चळत)
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
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(चळत));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(चळत);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
स्थिर ओळीत शून्य assert_switch_handle(संरचना k_thread *thread)
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
स्थिर ओळीत शून्य init_thread_custom_data(संरचना k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = रिक्त;
#अन्यथा
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
स्थिर ओळीत शून्य add_thread_to_monitor(संरचना k_thread *thread,
					 k_thread_entry_t entry,
					 शून्य *p1, शून्य *p2, शून्य *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, किल्ली);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
स्थिर ओळीत शून्य init_thread_name(संरचना k_thread *thread, स्थिरांक अक्षर *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
स्थिर ओळीत शून्य init_thread_cpu_mask(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	जर (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} अन्यथा {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#अन्यथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
स्थिर ओळीत शून्य init_thread_userspace_perms(संरचना k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	जर ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
स्थिर ओळीत शून्य init_thread_deadline(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#अन्यथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
स्थिर ओळीत शून्य init_thread_halt_queue(संरचना k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#अन्यथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
स्थिर ओळीत शून्य init_thread_usage(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (संरचना k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#अन्यथा
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
स्थिर ओळीत शून्य assert_thread_not_reused(संरचना k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	साठी (संरचना k_thread *t = _kernel.threads; t; t = t->next_thread) {
		जर (t == new_thread) {
			reused = true;
			थांबा;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, किल्ली);

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
अक्षर *z_setup_new_thread(संरचना k_thread *new_thread,
			 k_thread_चळत_t *चळत, माप_प्रकार चळत_size,
			 k_thread_entry_t entry,
			 शून्य *p1, शून्य *p2, शून्य *p3,
			 पूर्णांक prio, uint32_t options, स्थिरांक अक्षर *name)
{
	अक्षर *चळत_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, चळत, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	चळत_ptr = setup_thread_चळत(new_thread, चळत, चळत_size);

#जर Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = रिक्त;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_चळत(new_thread, चळत);
	assert_thread_coherence(new_thread, चळत);

	arch_new_thread(new_thread, चळत, चळत_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = रिक्त;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	जर (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == रिक्त)) {
		new_thread->resource_pool = रिक्त;
		परत चळत_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	परत चळत_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_चळत_SAFETY
पूर्णांक z_impl_k_thread_runtime_चळत_unused_threshold_pct_set(संरचना k_thread *thread,
							   uint32_t pct)
{
	माप_प्रकार unused_threshold;

	जर (pct > 99) {
		परत -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->चळत_info.size * pct) / 100;

	thread->चळत_info.usage.unused_threshold = unused_threshold;

	परत 0;
}

पूर्णांक z_impl_k_thread_runtime_चळत_unused_threshold_set(संरचना k_thread *thread,
						       माप_प्रकार threshold)
{
	जर (threshold > thread->चळत_info.size) {
		परत -EINVAL;
	}

	thread->चळत_info.usage.unused_threshold = threshold;

	परत 0;
}

माप_प्रकार z_impl_k_thread_runtime_चळत_unused_threshold_get(संरचना k_thread *thread)
{
	परत thread->चळत_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
पूर्णांक z_vrfy_k_thread_runtime_चळत_unused_threshold_pct_set(संरचना k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	परत z_impl_k_thread_runtime_चळत_unused_threshold_pct_set(thread, pct);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

पूर्णांक z_vrfy_k_thread_runtime_चळत_unused_threshold_set(संरचना k_thread *thread,
						       माप_प्रकार threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	परत z_impl_k_thread_runtime_चळत_unused_threshold_set(thread, threshold);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

माप_प्रकार z_vrfy_k_thread_runtime_चळत_unused_threshold_get(संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	परत z_impl_k_thread_runtime_चळत_unused_threshold_get(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(संरचना k_thread *new_thread,
			      k_thread_चळत_t *चळत,
			      माप_प्रकार चळत_size, k_thread_entry_t entry,
			      शून्य *p1, शून्य *p2, शून्य *p3,
			      पूर्णांक prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, चळत, चळत_size, entry, p1, p2, p3,
			  prio, options, रिक्त);

	thread_schedule_new(new_thread, delay);

	परत new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_चळत_is_user_capable(k_thread_चळत_t *चळत)
{
	परत k_object_find(चळत) != रिक्त;
}

k_tid_t z_vrfy_k_thread_create(संरचना k_thread *new_thread,
			       k_thread_चळत_t *चळत,
			       माप_प्रकार चळत_size, k_thread_entry_t entry,
			       शून्य *p1, शून्य *p2, शून्य *p3,
			       पूर्णांक prio, uint32_t options, k_timeout_t delay)
{
	माप_प्रकार total_size, चळत_obj_size;
	संरचना k_object *चळत_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	चळत_object = k_object_find(चळत);
	K_OOPS(K_SYSCALL_VERIFY_संदेश(k_object_validation_check(चळत_object, चळत,
						K_OBJ_THREAD_चळत_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_संदेश(!size_add_overflow(K_THREAD_चळत_RESERVED,
						       चळत_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    चळत_size,
				    K_THREAD_चळत_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	चळत_obj_size = चळत_object->data.चळत_data->size;
#अन्यथा
	चळत_obj_size = चळत_object->data.चळत_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_संदेश(total_size <= चळत_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, चळत_obj_size));

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

	z_setup_new_thread(new_thread, चळत, चळत_size,
			   entry, p1, p2, p3, prio, options, रिक्त);

	thread_schedule_new(new_thread, delay);

	परत new_thread;
}
#अन्तर्भाव <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_init_thread_base(संरचना _thread_base *thread_base, पूर्णांक priority,
		       uint32_t initial_state, अचिन्हित पूर्णांक options)
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
	__ASSERT(z_चळत_is_user_capable(_current->चळत_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	स_रखो(_current->userspace_local_data, 0,
	       आकार(संरचना _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_चळत_setup(_current,
			     (अक्षर *)(_current->चळत_info.start +
				      _current->चळत_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#अन्यथा
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#जर defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_चळत_INFO)
#ifdef CONFIG_चळत_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

पूर्णांक z_चळत_space_get(स्थिरांक uint8_t *चळत_start, माप_प्रकार size, माप_प्रकार *unused_ptr)
{
	माप_प्रकार unused = 0;
	स्थिरांक uint8_t *checked_चळत = चळत_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	स्थिरांक uint8_t *चळत_pointer = (स्थिरांक uint8_t *)&चळत_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	जर ((चळत_pointer > चळत_start) && (चळत_pointer <= (चळत_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_चळत_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		परत -ENOTSUP;
	}

	जर (IS_ENABLED(CONFIG_चळत_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_चळत += 4;
		size -= 4;
	}

	साठी (माप_प्रकार i = 0; i < size; i++) {
		जर ((checked_चळत[i]) == 0xaaU) {
			unused++;
		} अन्यथा {
			थांबा;
		}
	}

	*unused_ptr = unused;

	परत 0;
}

#ifdef CONFIG_THREAD_RUNTIME_चळत_SAFETY
पूर्णांक k_thread_runtime_चळत_safety_full_check(स्थिरांक संरचना k_thread *thread,
					     माप_प्रकार *unused_ptr,
					     k_thread_चळत_safety_handler_t handler,
					     शून्य *arg)
{
	पूर्णांक    rv;
	माप_प्रकार unused_space;

	__ASSERT_NO_MSG(thread != रिक्त);

	rv = z_चळत_space_get((स्थिरांक uint8_t *)thread->चळत_info.start,
			       thread->चळत_info.size, &unused_space);

	जर (rv != 0) {
		परत rv;
	}

	जर (unused_ptr != रिक्त) {
		*unused_ptr = unused_space;
	}

	जर ((unused_space < thread->चळत_info.usage.unused_threshold) &&
	    (handler != रिक्त)) {
		handler(thread, unused_space, arg);
	}

	परत 0;
}

पूर्णांक k_thread_runtime_चळत_safety_threshold_check(स्थिरांक संरचना k_thread *thread,
						  माप_प्रकार *unused_ptr,
						  k_thread_चळत_safety_handler_t handler,
						  शून्य *arg)
{
	पूर्णांक    rv;
	माप_प्रकार unused_space;
	माप_प्रकार threshold;
	माप_प्रकार scan_size;

	__ASSERT_NO_MSG(thread != रिक्त);

	threshold = thread->चळत_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	जर (IS_ENABLED(CONFIG_चळत_SENTINEL) && (threshold != 0U)) {
		scan_size += आकार(uint32_t);
	}

	rv = z_चळत_space_get((स्थिरांक uint8_t *)thread->चळत_info.start,
			       scan_size, &unused_space);

	जर (rv != 0) {
		परत rv;
	}

	जर (unused_ptr != रिक्त) {
		*unused_ptr = unused_space;
	}

	जर ((unused_space < threshold) && (handler != रिक्त)) {
		handler(thread, unused_space, arg);
	}

	परत 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

पूर्णांक z_impl_k_thread_चळत_space_get(स्थिरांक संरचना k_thread *thread,
				    माप_प्रकार *unused_ptr)
{
#ifdef CONFIG_THREAD_चळत_MEM_MAPPED
	जर (thread->चळत_info.mapped.addr == रिक्त) {
		परत -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	परत z_चळत_space_get((स्थिरांक uint8_t *)thread->चळत_info.start,
				 thread->चळत_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
पूर्णांक z_vrfy_k_thread_चळत_space_get(स्थिरांक संरचना k_thread *thread,
				    माप_प्रकार *unused_ptr)
{
	माप_प्रकार unused;
	पूर्णांक ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_चळत_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		परत ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, आकार(माप_प्रकार));
	CHECKIF(ret != 0) {
		परत ret;
	}

	परत 0;
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
स्थिर ओळीत k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    स्थिरांक संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	परत z_impl_k_thread_timeout_remaining_ticks(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

स्थिर ओळीत k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  स्थिरांक संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	परत z_impl_k_thread_timeout_expires_ticks(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
शून्य z_thread_mark_switched_in(शून्य)
{
#जर defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

शून्य z_thread_mark_switched_out(शून्य)
{
#जर defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	जर (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		परत;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

पूर्णांक k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	जर ((thread == रिक्त) || (stats == रिक्त)) {
		परत -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#अन्यथा
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	परत 0;
}

पूर्णांक k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	जर (stats == रिक्त) {
		परत -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	अचिन्हित पूर्णांक अंक_cpus = arch_अंक_cpus();

	साठी (अचिन्हित पूर्णांक i = 0; i < अंक_cpus; i++) {
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

	परत 0;
}

पूर्णांक k_thread_runtime_stats_cpu_get(पूर्णांक cpu, k_thread_runtime_stats_t *stats)
{
	जर (stats == रिक्त) {
		परत -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((अचिन्हित पूर्णांक)cpu >= arch_अंक_cpus())) {
		परत -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#अन्यथा
	ARG_UNUSED(cpu);
#endif

	परत 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
स्थिर संरचना k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
स्थिर संरचना k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_चळत_MEM_MAPPED
स्थिर शून्य *thread_cleanup_चळत_addr;
स्थिर माप_प्रकार thread_cleanup_चळत_sz;

स्थिर शून्य unmap_thread_चळत(शून्य *addr, माप_प्रकार sz)
{
	जर (addr != रिक्त) {
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

#ifdef CONFIG_THREAD_चळत_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_चळत_addr = thread->चळत_info.mapped.addr;
	thread_cleanup_चळत_sz = thread->चळत_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->चळत_info.mapped.addr = रिक्त;
	thread->चळत_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य do_deferred_thread_cleanup(शून्य)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_चळत_MEM_MAPPED
	unmap_thread_चळत(thread_cleanup_चळत_addr, thread_cleanup_चळत_sz);

	thread_cleanup_चळत_addr = रिक्त;
	thread_cleanup_चळत_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य do_thread_cleanup(संरचना k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_चळत_MEM_MAPPED
	unmap_thread_चळत(thread->चळत_info.mapped.addr,
			   thread->चळत_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->चळत_info.mapped.addr = रिक्त;
	thread->चळत_info.mapped.sz = 0;
#अन्यथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

शून्य k_thread_abort_cleanup(संरचना k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		जर (thread_to_cleanup != रिक्त) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = रिक्त;
		}

		जर (thread == _current) {
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
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		जर (thread_to_cleanup == thread) {
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
#जर Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = रिक्त;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_चळत_INFO
	dummy_thread->चळत_info.start = 0U;
	dummy_thread->चळत_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#जर (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#अन्यथा
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

शून्य z_impl_k_thread_priority_set(k_tid_t thread, पूर्णांक prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, रिक्त);

	bool need_sched = z_thread_prio_set((संरचना k_thread *)thread, prio);

	जर ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_thread_priority_set(k_tid_t thread, पूर्णांक prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_संदेश(_is_valid_prio(prio, रिक्त),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_संदेश((int8_t)prio >= thread->base.prio,
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
	जर (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		परत;
	}

	k_spinlock_किल्ली_t किल्ली = z_sched_spinlock_lock();

	जर (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(किल्ली);
		परत;
	}

	z_thread_halt(thread, किल्ली, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_किल्ली_t किल्ली = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	जर (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(किल्ली);
		परत;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(किल्ली);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_किल्ली_t किल्ली = z_sched_spinlock_lock();

	जर (z_is_thread_sleeping(thread)) {
		(शून्य)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(किल्ली);
	} अन्यथा {
		z_sched_spinlock_unlock(किल्ली);
	}
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_thread_abort(संरचना k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_किल्ली_t किल्ली = z_sched_spinlock_lock();

	जर (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(किल्ली);
		परत;
	}

	z_thread_halt(thread, किल्ली, true);

	जर (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#जर !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
शून्य z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, पात, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, पात, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

पूर्णांक z_impl_k_thread_join(संरचना k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_किल्ली_t किल्ली = z_sched_spinlock_lock();
	पूर्णांक ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	जर (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} अन्यथा जर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} अन्यथा जर ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} अन्यथा {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(किल्ली);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		परत ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(किल्ली);
	परत ret;
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
	पूर्णांक ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	निवड (ret) {
	स्थिती 0:
		परत false;
	स्थिती -EINVAL:
		परत true;
	पूर्वनिर्धारित:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_संदेश(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

स्थिर ओळीत पूर्णांक z_vrfy_k_thread_join(संरचना k_thread *thread,
				       k_timeout_t timeout)
{
	जर (thread_obj_validate(thread)) {
		परत 0;
	}

	परत z_impl_k_thread_join(thread, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_join_mrsh.c>

स्थिर ओळीत शून्य z_vrfy_k_thread_abort(k_tid_t thread)
{
	जर (thread_obj_validate(thread)) {
		परत;
	}

	K_OOPS(K_SYSCALL_VERIFY_संदेश(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((संरचना k_thread *)thread);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(शून्य)
{
	परत !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

शून्य z_impl_k_yield(शून्य)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_yield(शून्य)
{
	z_impl_k_yield();
}
#अन्तर्भाव <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
