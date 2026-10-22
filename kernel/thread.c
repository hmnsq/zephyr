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

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/spinlock.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/sys/clock.h>
#समावेश <ksched.h>
#समावेश <kthread.h>
#समावेश <wait_q.h>
#समावेश <timeout_q.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <kernel_internal.h>
#समावेश <kswap.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/tracing/tracing.h>
#समावेश <string.h>
#समावेश <stdbool.h>
#समावेश <zephyr/sys/check.h>
#समावेश <zephyr/random/random.h>
#समावेश <zephyr/sys/atomic.h>
#समावेश <zephyr/logging/log.h>
#समावेश <zephyr/llext/symbol.h>
#समावेश <zephyr/sys/iterable_sections.h>
#समावेश <zephyr/sys/minmax.h>

#समावेश <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#अगर_घोषित CONFIG_OBJ_CORE_THREAD
#अगर_घोषित CONFIG_OBJ_CORE_STATS_THREAD
अटल काष्ठा k_obj_core_stats_desc  thread_stats_desc = {
	.raw_आकार = माप(काष्ठा k_cycle_stats),
	.query_आकार = माप(काष्ठा k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#अगर_घोषित CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#अन्यथा
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_THREAD */
#पूर्ण_अगर /* CONFIG_OBJ_CORE_THREAD */


#घोषणा _FOREACH_STATIC_THREAD(thread_तथ्य)              \
	STRUCT_SECTION_FOREACH(static_thread_तथ्य, thread_तथ्य)

bool k_is_in_isr(व्योम)
{
	वापस arch_is_in_isr();
}
EXPORT_प्रतीक(k_is_in_isr);

#अगर_घोषित CONFIG_THREAD_CUSTOM_तथ्य
व्योम z_impl_k_thread_custom_तथ्य_set(व्योम *मूल्य)
{
	_current->custom_तथ्य = मूल्य;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_thread_custom_तथ्य_set(व्योम *तथ्य)
{
	z_impl_k_thread_custom_तथ्य_set(तथ्य);
}
#समावेश <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम *z_impl_k_thread_custom_तथ्य_get(व्योम)
{
	वापस _current->custom_तथ्य;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम *z_vrfy_k_thread_custom_तथ्य_get(व्योम)
{
	वापस z_impl_k_thread_custom_तथ्य_get();
}
#समावेश <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#पूर्ण_अगर /* CONFIG_USERSPACE */
#पूर्ण_अगर /* CONFIG_THREAD_CUSTOM_DATA */

पूर्णांक z_impl_k_is_preempt_thread(व्योम)
{
	वापस !arch_is_in_isr() && thread_is_preemptible(_current);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_is_preempt_thread(व्योम)
{
	वापस z_impl_k_is_preempt_thread();
}
#समावेश <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_thread_priority_get(k_tid_t thread)
{
	वापस thread->base.prio;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	वापस z_impl_k_thread_priority_get(thread);
}
#समावेश <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

#अगर_घोषित CONFIG_THREAD_NAME
अटल व्योम set_thread_name(काष्ठा k_thread *thread, स्थिर अक्षर *str)
{
	अगर (str != शून्य) {
		म_नकलन(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#अगर_घोषित CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#पूर्ण_अगर /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} अन्यथा {
		thread->name[0] = '\0';
	}
}
#पूर्ण_अगर /* CONFIG_THREAD_NAME */

पूर्णांक z_impl_k_thread_name_set(k_tid_t thread, स्थिर अक्षर *str)
{
#अगर_घोषित CONFIG_THREAD_NAME
	अगर (thread == शून्य) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	वापस 0;
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	वापस -ENOSYS;
#पूर्ण_अगर /* CONFIG_THREAD_NAME */
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_thread_name_set(k_tid_t thread, स्थिर अक्षर *str)
{
#अगर_घोषित CONFIG_THREAD_NAME
	अक्षर name[CONFIG_THREAD_MAX_NAME_LEN];

	अगर (thread != शून्य) {
		अगर (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			वापस -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	अगर (k_usermode_string_copy(name, str, माप(name)) != 0) {
		वापस -EFAULT;
	}

	वापस z_impl_k_thread_name_set(thread, name);
#अन्यथा
	वापस -ENOSYS;
#पूर्ण_अगर /* CONFIG_THREAD_NAME */
}
#समावेश <zephyr/syscalls/k_thread_name_set_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

स्थिर अक्षर *k_thread_name_get(k_tid_t thread)
{
#अगर_घोषित CONFIG_THREAD_NAME
	वापस (स्थिर अक्षर *)thread->name;
#अन्यथा
	ARG_UNUSED(thread);
	वापस शून्य;
#पूर्ण_अगर /* CONFIG_THREAD_NAME */
}

पूर्णांक z_impl_k_thread_name_copy(k_tid_t thread, अक्षर *buf, माप_प्रकार आकार)
{
#अगर_घोषित CONFIG_THREAD_NAME
	म_नकलन(buf, thread->name, आकार);
	वापस 0;
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(आकार);
	वापस -ENOSYS;
#पूर्ण_अगर /* CONFIG_THREAD_NAME */
}

अटल माप_प्रकार copy_bytes(अक्षर *dest, माप_प्रकार dest_आकार, स्थिर अक्षर *src, माप_प्रकार src_आकार)
{
	माप_प्रकार  bytes_to_copy;

	bytes_to_copy = min(dest_आकार, src_आकार);
	स_नकल(dest, src, bytes_to_copy);

	वापस bytes_to_copy;
}

स्थिर अक्षर *k_thread_state_str(k_tid_t thread_id, अक्षर *buf, माप_प्रकार buf_आकार)
{
	माप_प्रकार      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#घोषणा SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, माप(Z_STATE_STR_##s) - 1 }
	अटल स्थिर काष्ठा {
		स्थिर अक्षर *str;
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
#अघोषित SS_ENT

	अगर ((buf == शून्य) || (buf_आकार == 0)) {
		वापस "";
	}

	buf_आकार--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	क्रम (अचिन्हित पूर्णांक सूचकांक = 0; thread_state != 0; सूचकांक++) {
		bit = state_string[सूचकांक].bit;
		अगर ((thread_state & bit) == 0) {
			जारी;
		}

		off += copy_bytes(buf + off, buf_आकार - off,
				  state_string[सूचकांक].str,
				  state_string[सूचकांक].len);

		thread_state &= ~bit;

		अगर (thread_state != 0) {
			off += copy_bytes(buf + off, buf_आकार - off, "+", 1);
		}
	}

	buf[off] = '\0';

	वापस (स्थिर अक्षर *)buf;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_thread_name_copy(k_tid_t thread,
					    अक्षर *buf, माप_प्रकार आकार)
{
#अगर_घोषित CONFIG_THREAD_NAME
	माप_प्रकार len;
	काष्ठा k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	अगर ((ko == शून्य) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		वापस -EINVAL;
	}
	अगर (K_SYSCALL_MEMORY_WRITE(buf, आकार) != 0) {
		वापस -EFAULT;
	}
	len = म_माप(thread->name);
	अगर ((len + 1) > आकार) {
		वापस -ENOSPC;
	}

	वापस k_usermode_to_copy((व्योम *)buf, thread->name, len + 1);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(आकार);
	वापस -ENOSYS;
#पूर्ण_अगर /* CONFIG_THREAD_NAME */
}
#समावेश <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

#अगर_घोषित CONFIG_ढेर_SENTINEL
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
व्योम z_check_ढेर_sentinel(व्योम)
{
	uint32_t *ढेर;

	अगर ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		वापस;
	}

	ढेर = (uint32_t *)_current->ढेर_info.start;
	अगर (*ढेर != ढेर_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ढेर = ढेर_SENTINEL;
		z_except_reason(K_ERR_ढेर_CHK_FAIL);
	}
}
#पूर्ण_अगर /* CONFIG_STACK_SENTINEL */

#अगर defined(CONFIG_ढेर_POINTER_RANDOM) && (CONFIG_ढेर_POINTER_RANDOM != 0)
पूर्णांक z_ढेर_adjust_initialized;

अटल माप_प्रकार random_offset(माप_प्रकार ढेर_आकार)
{
	माप_प्रकार random_val;

	अगर (!z_ढेर_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, माप(random_val));
	} अन्यथा {
		sys_rand_get((uint8_t *)&random_val, माप(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	स्थिर माप_प्रकार fuzz = random_val % CONFIG_ढेर_POINTER_RANDOM;

	अगर (unlikely(fuzz * 2 > ढेर_आकार)) {
		वापस 0;
	}

	वापस fuzz;
}
#अगर defined(CONFIG_ढेर_GROWS_UP)
	/* This is so rare not bothering for now */
#त्रुटि "Stack pointer randomization not implemented for upward growing stacks"
#पूर्ण_अगर /* CONFIG_STACK_GROWS_UP */
#पूर्ण_अगर /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
काष्ठा ढेर_geometry {
	माप_प्रकार obj_आकार;	/* size of the whole stack object */
	अक्षर *buf_start;	/* start of the usable stack buffer */
	माप_प्रकार buf_आकार;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
अटल काष्ठा ढेर_geometry compute_ढेर_geometry(k_thread_ढेर_t *ढेर,
						    माप_प्रकार ढेर_आकार)
{
	काष्ठा ढेर_geometry geo;

#अगर_घोषित CONFIG_USERSPACE
	अगर (z_ढेर_is_user_capable(ढेर)) {
		geo.obj_आकार = K_THREAD_ढेर_LEN(ढेर_आकार);
		geo.buf_start = K_THREAD_ढेर_बफर(ढेर);
		geo.buf_आकार = geo.obj_आकार - K_THREAD_ढेर_RESERVED;
	} अन्यथा
#पूर्ण_अगर /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_आकार = K_KERNEL_ढेर_LEN(ढेर_आकार);
		geo.buf_start = K_KERNEL_ढेर_बफर(ढेर);
		geo.buf_आकार = geo.obj_आकार - K_KERNEL_ढेर_RESERVED;

#अगर defined(ARCH_KERNEL_ढेर_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		अगर (K_KERNEL_ढेर_RESERVED > geo.obj_आकार) {
			k_panic();
		}
#पूर्ण_अगर
	}

	वापस geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
अटल अक्षर *map_thread_ढेर(काष्ठा k_thread *new_thread,
			      k_thread_ढेर_t *ढेर,
			      काष्ठा ढेर_geometry *geo)
{
	अक्षर *ढेर_ptr;

#अगर_घोषित CONFIG_THREAD_ढेर_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	व्योम *ढेर_mapped = k_mem_map_phys_guard((uintptr_t)ढेर, geo->obj_आकार,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ढेर_mapped != 0);

#अगर_घोषित CONFIG_USERSPACE
	अगर (z_ढेर_is_user_capable(ढेर)) {
		geo->buf_start = K_THREAD_ढेर_बफर(ढेर_mapped);
	} अन्यथा
#पूर्ण_अगर /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_ढेर_बफर(ढेर_mapped);
	}

	ढेर_ptr = (अक्षर *)ढेर_mapped + geo->obj_आकार;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ढेर_info.mapped.addr = ढेर_mapped;
	new_thread->ढेर_info.mapped.sz = geo->obj_आकार;
#अन्यथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ढेर_ptr = (अक्षर *)ढेर + geo->obj_आकार;
#पूर्ण_अगर /* CONFIG_THREAD_STACK_MEM_MAPPED */

	वापस ढेर_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
अटल अंतरभूत व्योम fill_init_ढेर(अक्षर *buf_start, माप_प्रकार buf_आकार)
{
#अगर_घोषित CONFIG_INIT_STACKS
	स_रखो(buf_start, 0xaa, buf_आकार);
#अन्यथा
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_आकार);
#पूर्ण_अगर /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
अटल अंतरभूत व्योम set_ढेर_sentinel(अक्षर *buf_start)
{
#अगर_घोषित CONFIG_ढेर_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = ढेर_SENTINEL;
#अन्यथा
	ARG_UNUSED(buf_start);
#पूर्ण_अगर /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
अटल माप_प्रकार reserve_ढेर_headroom(काष्ठा k_thread *new_thread,
				     अक्षर *ढेर_ptr, माप_प्रकार buf_आकार)
{
	माप_प्रकार delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(ढेर_ptr);
	ARG_UNUSED(buf_आकार);

#अगर_घोषित CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ढेर_setup(new_thread, ढेर_ptr);
#पूर्ण_अगर /* CONFIG_THREAD_LOCAL_STORAGE */
#अगर_घोषित CONFIG_THREAD_USERSPACE_LOCAL_तथ्य
	माप_प्रकार tls_आकार = माप(काष्ठा thread_userspace_local_तथ्य);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_आकार;
	new_thread->userspace_local_तथ्य =
		(काष्ठा thread_userspace_local_तथ्य *)(ढेर_ptr - delta);
#पूर्ण_अगर /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#अगर defined(CONFIG_ढेर_POINTER_RANDOM) && (CONFIG_ढेर_POINTER_RANDOM != 0)
	delta += random_offset(buf_आकार);
#पूर्ण_अगर /* CONFIG_STACK_POINTER_RANDOM */

	वापस ROUND_UP(delta, ARCH_ढेर_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
अटल अंतरभूत व्योम set_ढेर_info(काष्ठा k_thread *new_thread, अक्षर *buf_start,
				  माप_प्रकार buf_आकार, माप_प्रकार delta)
{
#अगर_घोषित CONFIG_THREAD_ढेर_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ढेर_info.start = (uintptr_t)buf_start;
	new_thread->ढेर_info.आकार = buf_आकार;
	new_thread->ढेर_info.delta = delta;

#अगर_घोषित CONFIG_THREAD_RUNTIME_ढेर_SAFETY
	new_thread->ढेर_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_ढेर_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_आकार) / 100;
#पूर्ण_अगर
#अन्यथा
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_आकार);
	ARG_UNUSED(delta);
#पूर्ण_अगर /* CONFIG_THREAD_STACK_INFO */
}

अटल अक्षर *setup_thread_ढेर(काष्ठा k_thread *new_thread,
				k_thread_ढेर_t *ढेर, माप_प्रकार ढेर_आकार)
{
	काष्ठा ढेर_geometry geo = compute_ढेर_geometry(ढेर, ढेर_आकार);
	अक्षर *ढेर_ptr = map_thread_ढेर(new_thread, ढेर, &geo);
	माप_प्रकार delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ढेर, new_thread, geo.obj_आकार, (व्योम *)geo.buf_start,
		geo.buf_आकार, (व्योम *)ढेर_ptr);

	fill_init_ढेर(geo.buf_start, geo.buf_आकार);
	set_ढेर_sentinel(geo.buf_start);
	delta = reserve_ढेर_headroom(new_thread, ढेर_ptr, geo.buf_आकार);
	set_ढेर_info(new_thread, geo.buf_start, geo.buf_आकार, delta);

	वापस ढेर_ptr - delta;
}

#अगर_घोषित CONFIG_HW_SHADOW_ढेर
अटल व्योम setup_shadow_ढेर(काष्ठा k_thread *new_thread,
			 k_thread_ढेर_t *ढेर)
{
	पूर्णांक ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ढेर_to_hw_shadow_ढेर, stk_to_hw_shstk) {
		अगर (stk_to_hw_shstk->ढेर == ढेर) {
			ret = k_thread_hw_shadow_ढेर_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->आकार);
			अगर (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			अवरोध;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	अगर (ret != -ENOENT) {
		वापस;
	}

	STRUCT_SECTION_FOREACH(ढेर_to_hw_shadow_ढेर_arr, stk_to_hw_shstk) {
		अगर ((uintptr_t)ढेर >= stk_to_hw_shstk->ढेर_addr &&
		    (uintptr_t)ढेर < stk_to_hw_shstk->ढेर_addr +
		    stk_to_hw_shstk->ढेर_आकार * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ढेर_offset = (uintptr_t)ढेर - stk_to_hw_shstk->ढेर_addr;
			uintptr_t ढेर_सूचकांक = ढेर_offset / stk_to_hw_shstk->ढेर_आकार;
			uintptr_t addr;

			अगर (ढेर_सूचकांक >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ढेर);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_आकार * ढेर_सूचकांक;
			ret = k_thread_hw_shadow_ढेर_attach(new_thread,
							      (arch_thread_hw_shadow_ढेर_t *)addr,
							      stk_to_hw_shstk->shstk_आकार);
			अगर (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			अवरोध;
		}
	}

	अगर (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ढेर);
		k_panic();
	}
}
#अन्यथा
/* No-op stub used when hardware shadow stacks are not configured. */
अटल अंतरभूत व्योम setup_shadow_ढेर(काष्ठा k_thread *new_thread,
				      k_thread_ढेर_t *ढेर)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(ढेर);
}
#पूर्ण_अगर

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
अटल अंतरभूत व्योम thread_abort_cleanup_check_reuse(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
अटल अंतरभूत व्योम init_thread_obj_core(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#अगर_घोषित CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  माप(thread->base.usage));
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_THREAD */
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
अटल अंतरभूत व्योम init_thread_userspace(काष्ठा k_thread *thread,
					 k_thread_ढेर_t *ढेर,
					 uint32_t options)
{
#अगर_घोषित CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ढेर_is_user_capable(ढेर),
		 "user thread %p with kernel-only stack %p", thread, ढेर);
	k_object_init(thread);
	k_object_init(ढेर);
	thread->ढेर_obj = ढेर;
	thread->syscall_frame = शून्य;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(ढेर);
	ARG_UNUSED(options);
#पूर्ण_अगर /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
अटल अंतरभूत व्योम assert_thread_coherence(काष्ठा k_thread *thread,
					   k_thread_ढेर_t *ढेर)
{
#अगर_घोषित CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#अगर_अघोषित CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ढेर));
#पूर्ण_अगर /* CONFIG_DYNAMIC_THREAD */
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(ढेर);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
अटल अंतरभूत व्योम assert_switch_handle(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != शून्य,
		 "arch layer failed to initialize switch_handle");
#पूर्ण_अगर /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
अटल अंतरभूत व्योम init_thread_custom_तथ्य(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_THREAD_CUSTOM_तथ्य
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_तथ्य = शून्य;
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
अटल अंतरभूत व्योम add_thread_to_monitor(काष्ठा k_thread *thread,
					 k_thread_entry_t entry,
					 व्योम *p1, व्योम *p2, व्योम *p3)
{
#अगर_घोषित CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&z_thread_monitor_lock);

	thread->अग्र_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, कुंजी);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#पूर्ण_अगर /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
अटल अंतरभूत व्योम init_thread_name(काष्ठा k_thread *thread, स्थिर अक्षर *name)
{
#अगर_घोषित CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#पूर्ण_अगर /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
अटल अंतरभूत व्योम init_thread_cpu_mask(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_SCHED_CPU_MASK
	अगर (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} अन्यथा {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
अटल अंतरभूत व्योम init_thread_userspace_perms(काष्ठा k_thread *thread,
					       uint32_t options)
{
#अगर_घोषित CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	अगर ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#पूर्ण_अगर /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
अटल अंतरभूत व्योम init_thread_deadline(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
अटल अंतरभूत व्योम init_thread_halt_queue(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
अटल अंतरभूत व्योम init_thread_usage(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (काष्ठा k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
अटल अंतरभूत व्योम assert_thread_not_reused(काष्ठा k_thread *new_thread)
{
#अगर_घोषित CONFIG_THREAD_MONITOR
#अगर_घोषित CONFIG_ASSERT
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	क्रम (काष्ठा k_thread *t = _kernel.threads; t; t = t->अग्र_thread) {
		अगर (t == new_thread) {
			reused = true;
			अवरोध;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, कुंजी);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#पूर्ण_अगर /* CONFIG_ASSERT */
#पूर्ण_अगर /* CONFIG_THREAD_MONITOR */
	ARG_UNUSED(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
अक्षर *z_setup_new_thread(काष्ठा k_thread *new_thread,
			 k_thread_ढेर_t *ढेर, माप_प्रकार ढेर_आकार,
			 k_thread_entry_t entry,
			 व्योम *p1, व्योम *p2, व्योम *p3,
			 पूर्णांक prio, uint32_t options, स्थिर अक्षर *name)
{
	अक्षर *ढेर_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ढेर, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	ढेर_ptr = setup_thread_ढेर(new_thread, ढेर, ढेर_आकार);

#अगर Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = शून्य;
	new_thread->orig_prio = new_thread->base.prio;
#पूर्ण_अगर /* Z_MUTEX_PI_ENABLED */

	setup_shadow_ढेर(new_thread, ढेर);
	assert_thread_coherence(new_thread, ढेर);

	arch_new_thread(new_thread, ढेर, ढेर_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_तथ्य = शून्य;

	assert_switch_handle(new_thread);
	init_thread_custom_तथ्य(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	अगर (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == शून्य)) {
		new_thread->resource_pool = शून्य;
		वापस ढेर_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	वापस ढेर_ptr;
}

#अगर_घोषित CONFIG_THREAD_RUNTIME_ढेर_SAFETY
पूर्णांक z_impl_k_thread_runtime_ढेर_unused_threshold_pct_set(काष्ठा k_thread *thread,
							   uint32_t pct)
{
	माप_प्रकार unused_threshold;

	अगर (pct > 99) {
		वापस -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->ढेर_info.आकार * pct) / 100;

	thread->ढेर_info.usage.unused_threshold = unused_threshold;

	वापस 0;
}

पूर्णांक z_impl_k_thread_runtime_ढेर_unused_threshold_set(काष्ठा k_thread *thread,
						       माप_प्रकार threshold)
{
	अगर (threshold > thread->ढेर_info.आकार) {
		वापस -EINVAL;
	}

	thread->ढेर_info.usage.unused_threshold = threshold;

	वापस 0;
}

माप_प्रकार z_impl_k_thread_runtime_ढेर_unused_threshold_get(काष्ठा k_thread *thread)
{
	वापस thread->ढेर_info.usage.unused_threshold;
}

#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_thread_runtime_ढेर_unused_threshold_pct_set(काष्ठा k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	वापस z_impl_k_thread_runtime_ढेर_unused_threshold_pct_set(thread, pct);
}
#समावेश <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

पूर्णांक z_vrfy_k_thread_runtime_ढेर_unused_threshold_set(काष्ठा k_thread *thread,
						       माप_प्रकार threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	वापस z_impl_k_thread_runtime_ढेर_unused_threshold_set(thread, threshold);
}
#समावेश <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

माप_प्रकार z_vrfy_k_thread_runtime_ढेर_unused_threshold_get(काष्ठा k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	वापस z_impl_k_thread_runtime_ढेर_unused_threshold_get(thread);
}
#समावेश <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
#पूर्ण_अगर /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(काष्ठा k_thread *new_thread,
			      k_thread_ढेर_t *ढेर,
			      माप_प्रकार ढेर_आकार, k_thread_entry_t entry,
			      व्योम *p1, व्योम *p2, व्योम *p3,
			      पूर्णांक prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, ढेर, ढेर_आकार, entry, p1, p2, p3,
			  prio, options, शून्य);

	thread_schedule_new(new_thread, delay);

	वापस new_thread;
}

#अगर_घोषित CONFIG_USERSPACE
bool z_ढेर_is_user_capable(k_thread_ढेर_t *ढेर)
{
	वापस k_object_find(ढेर) != शून्य;
}

k_tid_t z_vrfy_k_thread_create(काष्ठा k_thread *new_thread,
			       k_thread_ढेर_t *ढेर,
			       माप_प्रकार ढेर_आकार, k_thread_entry_t entry,
			       व्योम *p1, व्योम *p2, व्योम *p3,
			       पूर्णांक prio, uint32_t options, k_timeout_t delay)
{
	माप_प्रकार योग_आकार, ढेर_obj_आकार;
	काष्ठा k_object *ढेर_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ढेर_object = k_object_find(ढेर);
	K_OOPS(K_SYSCALL_VERIFY_संदेश(k_object_validation_check(ढेर_object, ढेर,
						K_OBJ_THREAD_ढेर_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_संदेश(!आकार_add_overflow(K_THREAD_ढेर_RESERVED,
						       ढेर_आकार, &योग_आकार),
				    "stack size overflow (%zu+%zu)",
				    ढेर_आकार,
				    K_THREAD_ढेर_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#अगर_घोषित CONFIG_GEN_PRIV_STACKS
	ढेर_obj_आकार = ढेर_object->तथ्य.ढेर_तथ्य->आकार;
#अन्यथा
	ढेर_obj_आकार = ढेर_object->तथ्य.ढेर_आकार;
#पूर्ण_अगर /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_संदेश(योग_आकार <= ढेर_obj_आकार,
				    "stack size %zu is too big, max is %zu",
				    योग_आकार, ढेर_obj_आकार));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, शून्य)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, ढेर, ढेर_आकार,
			   entry, p1, p2, p3, prio, options, शून्य);

	thread_schedule_new(new_thread, delay);

	वापस new_thread;
}
#समावेश <zephyr/syscalls/k_thread_create_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_init_thread_base(काष्ठा _thread_base *thread_base, पूर्णांक priority,
		       uint32_t initial_state, अचिन्हित पूर्णांक options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = शून्य;
	thread_base->user_options = (uint16_t)options;
	thread_base->thread_state = (uint8_t)initial_state;

	thread_base->prio = priority;

	thread_base->sched_locked = 0U;

#अगर_घोषित CONFIG_SMP
	thread_base->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_base->cpu = 0;
#पूर्ण_अगर /* CONFIG_SMP */

#अगर_घोषित CONFIG_TIMESLICE_PER_THREAD
	thread_base->slice_ticks = 0;
	thread_base->slice_expired = शून्य;
#पूर्ण_अगर /* CONFIG_TIMESLICE_PER_THREAD */

#अगर_घोषित CONFIG_SPIN_VALIDATE
	thread_base->swap_तथ्य = शून्य;
	/* otherwise swap_data does not need to be initialized */
#पूर्ण_अगर

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN व्योम k_thread_user_mode_enter(k_thread_entry_t entry,
					    व्योम *p1, व्योम *p2, व्योम *p3)
{
	SYS_PORT_TRACING_FUNC(k_thread, user_mode_enter);

	_current->base.user_options |= K_USER;
	z_thread_essential_clear(_current);
#अगर_घोषित CONFIG_THREAD_MONITOR
	_current->entry.pEntry = entry;
	_current->entry.parameter1 = p1;
	_current->entry.parameter2 = p2;
	_current->entry.parameter3 = p3;
#पूर्ण_अगर /* CONFIG_THREAD_MONITOR */
#अगर_घोषित CONFIG_USERSPACE
	__ASSERT(z_ढेर_is_user_capable(_current->ढेर_obj),
		 "dropping to user mode with kernel-only stack object");
#अगर_घोषित CONFIG_THREAD_USERSPACE_LOCAL_तथ्य
	स_रखो(_current->userspace_local_तथ्य, 0,
	       माप(काष्ठा thread_userspace_local_तथ्य));
#पूर्ण_अगर /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#अगर_घोषित CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ढेर_setup(_current,
			     (अक्षर *)(_current->ढेर_info.start +
				      _current->ढेर_info.आकार));
#पूर्ण_अगर /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#अन्यथा
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#पूर्ण_अगर /* CONFIG_USERSPACE */
}

#अगर defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ढेर_INFO)
#अगर_घोषित CONFIG_ढेर_GROWS_UP
#त्रुटि "Unsupported configuration for stack analysis"
#पूर्ण_अगर /* CONFIG_STACK_GROWS_UP */

पूर्णांक z_ढेर_space_get(स्थिर uint8_t *ढेर_start, माप_प्रकार आकार, माप_प्रकार *unused_ptr)
{
	माप_प्रकार unused = 0;
	स्थिर uint8_t *checked_ढेर = ढेर_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	स्थिर uint8_t *ढेर_pointer = (स्थिर uint8_t *)&ढेर_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	अगर ((ढेर_pointer > ढेर_start) && (ढेर_pointer <= (ढेर_start + आकार)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_ढेर_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		वापस -ENOTSUP;
	}

	अगर (IS_ENABLED(CONFIG_ढेर_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ढेर += 4;
		आकार -= 4;
	}

	क्रम (माप_प्रकार i = 0; i < आकार; i++) {
		अगर ((checked_ढेर[i]) == 0xaaU) {
			unused++;
		} अन्यथा {
			अवरोध;
		}
	}

	*unused_ptr = unused;

	वापस 0;
}

#अगर_घोषित CONFIG_THREAD_RUNTIME_ढेर_SAFETY
पूर्णांक k_thread_runtime_ढेर_safety_full_check(स्थिर काष्ठा k_thread *thread,
					     माप_प्रकार *unused_ptr,
					     k_thread_ढेर_safety_handler_t handler,
					     व्योम *arg)
{
	पूर्णांक    rv;
	माप_प्रकार unused_space;

	__ASSERT_NO_MSG(thread != शून्य);

	rv = z_ढेर_space_get((स्थिर uint8_t *)thread->ढेर_info.start,
			       thread->ढेर_info.आकार, &unused_space);

	अगर (rv != 0) {
		वापस rv;
	}

	अगर (unused_ptr != शून्य) {
		*unused_ptr = unused_space;
	}

	अगर ((unused_space < thread->ढेर_info.usage.unused_threshold) &&
	    (handler != शून्य)) {
		handler(thread, unused_space, arg);
	}

	वापस 0;
}

पूर्णांक k_thread_runtime_ढेर_safety_threshold_check(स्थिर काष्ठा k_thread *thread,
						  माप_प्रकार *unused_ptr,
						  k_thread_ढेर_safety_handler_t handler,
						  व्योम *arg)
{
	पूर्णांक    rv;
	माप_प्रकार unused_space;
	माप_प्रकार threshold;
	माप_प्रकार scan_आकार;

	__ASSERT_NO_MSG(thread != शून्य);

	threshold = thread->ढेर_info.usage.unused_threshold;
	scan_आकार = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	अगर (IS_ENABLED(CONFIG_ढेर_SENTINEL) && (threshold != 0U)) {
		scan_आकार += माप(uint32_t);
	}

	rv = z_ढेर_space_get((स्थिर uint8_t *)thread->ढेर_info.start,
			       scan_आकार, &unused_space);

	अगर (rv != 0) {
		वापस rv;
	}

	अगर (unused_ptr != शून्य) {
		*unused_ptr = unused_space;
	}

	अगर ((unused_space < threshold) && (handler != शून्य)) {
		handler(thread, unused_space, arg);
	}

	वापस 0;
}
#पूर्ण_अगर /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

पूर्णांक z_impl_k_thread_ढेर_space_get(स्थिर काष्ठा k_thread *thread,
				    माप_प्रकार *unused_ptr)
{
#अगर_घोषित CONFIG_THREAD_ढेर_MEM_MAPPED
	अगर (thread->ढेर_info.mapped.addr == शून्य) {
		वापस -EINVAL;
	}
#पूर्ण_अगर /* CONFIG_THREAD_STACK_MEM_MAPPED */

	वापस z_ढेर_space_get((स्थिर uint8_t *)thread->ढेर_info.start,
				 thread->ढेर_info.आकार, unused_ptr);
}

#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_thread_ढेर_space_get(स्थिर काष्ठा k_thread *thread,
				    माप_प्रकार *unused_ptr)
{
	माप_प्रकार unused;
	पूर्णांक ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ढेर_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		वापस ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, माप(माप_प्रकार));
	CHECKIF(ret != 0) {
		वापस ret;
	}

	वापस 0;
}
#समावेश <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
#पूर्ण_अगर /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    स्थिर काष्ठा k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	वापस z_impl_k_thread_timeout_remaining_ticks(thread);
}
#समावेश <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

अटल अंतरभूत k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  स्थिर काष्ठा k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	वापस z_impl_k_thread_timeout_expires_ticks(thread);
}
#समावेश <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

#अगर_घोषित CONFIG_INSTRUMENT_THREAD_SWITCHING
व्योम z_thread_mark_switched_in(व्योम)
{
#अगर defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#अगर_घोषित CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#पूर्ण_अगर /* CONFIG_TRACING */
}

व्योम z_thread_mark_switched_out(व्योम)
{
#अगर defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#पूर्ण_अगर /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#अगर_घोषित CONFIG_TRACING
#अगर_घोषित CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	अगर (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		वापस;
	}
#पूर्ण_अगर /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#पूर्ण_अगर /* CONFIG_TRACING */
}
#पूर्ण_अगर /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

पूर्णांक k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	अगर ((thread == शून्य) || (stats == शून्य)) {
		वापस -EINVAL;
	}

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#अन्यथा
	*stats = (k_thread_runtime_stats_t) {};
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE */

	वापस 0;
}

पूर्णांक k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ALL */

	अगर (stats == शून्य) {
		वापस -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	अचिन्हित पूर्णांक अंक_cpus = arch_अंक_cpus();

	क्रम (अचिन्हित पूर्णांक i = 0; i < अंक_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_cycles += tmp_stats.execution_cycles;
		stats->योग_cycles     += tmp_stats.योग_cycles;
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_cycles   += tmp_stats.current_cycles;
		stats->peak_cycles      += tmp_stats.peak_cycles;
		stats->average_cycles   += tmp_stats.average_cycles;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_cycles      += tmp_stats.idle_cycles;
	}
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ALL */

	वापस 0;
}

पूर्णांक k_thread_runtime_stats_cpu_get(पूर्णांक cpu, k_thread_runtime_stats_t *stats)
{
	अगर (stats == शून्य) {
		वापस -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((अचिन्हित पूर्णांक)cpu >= arch_अंक_cpus())) {
		वापस -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#अन्यथा
	ARG_UNUSED(cpu);
#पूर्ण_अगर

	वापस 0;
}

#अगर_घोषित CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
अटल काष्ठा k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
अटल काष्ठा k_spinlock thread_cleanup_lock;

#अगर_घोषित CONFIG_THREAD_ढेर_MEM_MAPPED
अटल व्योम *thread_cleanup_ढेर_addr;
अटल माप_प्रकार thread_cleanup_ढेर_sz;

अटल व्योम unmap_thread_ढेर(व्योम *addr, माप_प्रकार sz)
{
	अगर (addr != शून्य) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#पूर्ण_अगर /* CONFIG_THREAD_STACK_MEM_MAPPED */

व्योम defer_thread_cleanup(काष्ठा k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#अगर_घोषित CONFIG_THREAD_ढेर_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ढेर_addr = thread->ढेर_info.mapped.addr;
	thread_cleanup_ढेर_sz = thread->ढेर_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ढेर_info.mapped.addr = शून्य;
	thread->ढेर_info.mapped.sz = 0;
#पूर्ण_अगर /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

व्योम do_deferred_thread_cleanup(व्योम)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#अगर_घोषित CONFIG_THREAD_ढेर_MEM_MAPPED
	unmap_thread_ढेर(thread_cleanup_ढेर_addr, thread_cleanup_ढेर_sz);

	thread_cleanup_ढेर_addr = शून्य;
	thread_cleanup_ढेर_sz = 0;
#पूर्ण_अगर /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

व्योम do_thread_cleanup(काष्ठा k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#अगर_घोषित CONFIG_THREAD_ढेर_MEM_MAPPED
	unmap_thread_ढेर(thread->ढेर_info.mapped.addr,
			   thread->ढेर_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ढेर_info.mapped.addr = शून्य;
	thread->ढेर_info.mapped.sz = 0;
#अन्यथा /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

व्योम k_thread_abort_cleanup(काष्ठा k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		अगर (thread_to_cleanup != शून्य) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = शून्य;
		}

		अगर (thread == _current) {
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

व्योम k_thread_abort_cleanup_check_reuse(काष्ठा k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		अगर (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = शून्य;
		}
	}
}

#पूर्ण_अगर /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

व्योम z_dummy_thread_init(काष्ठा k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#अगर_घोषित CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#अगर Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = शून्य;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#पूर्ण_अगर /* Z_MUTEX_PI_ENABLED */
#अगर_घोषित CONFIG_THREAD_ढेर_INFO
	dummy_thread->ढेर_info.start = 0U;
	dummy_thread->ढेर_info.आकार = 0U;
#पूर्ण_अगर /* CONFIG_THREAD_STACK_INFO */
#अगर_घोषित CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#पूर्ण_अगर /* CONFIG_USERSPACE */
#अगर (K_HEAP_MEM_POOL_आकार > 0)
	k_thread_system_pool_assign(dummy_thread);
#अन्यथा
	dummy_thread->resource_pool = शून्य;
#पूर्ण_अगर /* K_HEAP_MEM_POOL_SIZE */

#अगर_घोषित CONFIG_USE_SWITCH
	dummy_thread->switch_handle = शून्य;
#पूर्ण_अगर /* CONFIG_USE_SWITCH */

#अगर_घोषित CONFIG_TIMESLICE_PER_THREAD
	dummy_thread->base.slice_ticks = 0;
#पूर्ण_अगर /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_set(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

व्योम z_impl_k_thread_priority_set(k_tid_t thread, पूर्णांक prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, शून्य);

	bool need_sched = z_thread_prio_set((काष्ठा k_thread *)thread, prio);

	अगर ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_thread_priority_set(k_tid_t thread, पूर्णांक prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_संदेश(_is_valid_prio(prio, शून्य),
				    "invalid thread priority %d", prio));
#अगर_अघोषित CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_संदेश((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#पूर्ण_अगर /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#समावेश <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	अगर (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		वापस;
	}

	k_spinlock_कुंजी_t कुंजी = z_sched_spinlock_lock();

	अगर (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(कुंजी);
		वापस;
	}

	z_thread_halt(thread, कुंजी, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#समावेश <zephyr/syscalls/k_thread_suspend_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_कुंजी_t कुंजी = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	अगर (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(कुंजी);
		वापस;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#समावेश <zephyr/syscalls/k_thread_resume_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_कुंजी_t कुंजी = z_sched_spinlock_lock();

	अगर (z_is_thread_sleeping(thread)) {
		(व्योम)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(कुंजी);
	} अन्यथा {
		z_sched_spinlock_unlock(कुंजी);
	}
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#समावेश <zephyr/syscalls/k_wakeup_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_thread_abort(काष्ठा k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_कुंजी_t कुंजी = z_sched_spinlock_lock();

	अगर (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(कुंजी);
		वापस;
	}

	z_thread_halt(thread, कुंजी, true);

	अगर (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#अगर !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
व्योम z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, पात, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, पात, thread);
}
#पूर्ण_अगर /* !CONFIG_ARCH_HAS_THREAD_ABORT */

पूर्णांक z_impl_k_thread_join(काष्ठा k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी = z_sched_spinlock_lock();
	पूर्णांक ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	अगर (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} अन्यथा अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} अन्यथा अगर ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} अन्यथा {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(कुंजी);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		वापस ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(कुंजी);
	वापस ret;
}

#अगर_घोषित CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
अटल bool thread_obj_validate(काष्ठा k_thread *thread)
{
	काष्ठा k_object *ko = k_object_find(thread);
	पूर्णांक ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	चयन (ret) {
	हाल 0:
		वापस false;
	हाल -EINVAL:
		वापस true;
	शेष:
#अगर_घोषित CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#पूर्ण_अगर /* CONFIG_LOG */
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

अटल अंतरभूत पूर्णांक z_vrfy_k_thread_join(काष्ठा k_thread *thread,
				       k_timeout_t timeout)
{
	अगर (thread_obj_validate(thread)) {
		वापस 0;
	}

	वापस z_impl_k_thread_join(thread, timeout);
}
#समावेश <zephyr/syscalls/k_thread_join_mrsh.c>

अटल अंतरभूत व्योम z_vrfy_k_thread_abort(k_tid_t thread)
{
	अगर (thread_obj_validate(thread)) {
		वापस;
	}

	K_OOPS(K_SYSCALL_VERIFY_संदेश(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((काष्ठा k_thread *)thread);
}
#समावेश <zephyr/syscalls/k_thread_abort_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

bool k_can_yield(व्योम)
{
	वापस !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

व्योम z_impl_k_yield(व्योम)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_yield(व्योम)
{
	z_impl_k_yield();
}
#समावेश <zephyr/syscalls/k_yield_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
