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

#شامل <zephyr/kernel.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/clock.h>
#شامل <ksched.h>
#شامل <kthread.h>
#شامل <wait_q.h>
#شامل <timeout_q.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <kernel_internal.h>
#شامل <kswap.h>
#شامل <zephyr/init.h>
#شامل <zephyr/tracing/tracing.h>
#شامل <string.h>
#شامل <stdbool.h>
#شامل <zephyr/sys/check.h>
#شامل <zephyr/random/random.h>
#شامل <zephyr/sys/atomic.h>
#شامل <zephyr/logging/log.h>
#شامل <zephyr/llext/symbol.h>
#شامل <zephyr/sys/iterable_sections.h>
#شامل <zephyr/sys/minmax.h>

#شامل <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
ایستا ساختار k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = اندازه(ساختار k_cycle_stats),
	.query_size = اندازه(ساختار k_thread_runtime_stats),
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
#وگرنه
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    پوچ);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(تهی)
{
	بازگشت arch_is_in_isr();
}
EXPORT_نماد(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
تهی z_impl_k_thread_custom_data_set(تهی *مقدار)
{
	_current->custom_data = مقدار;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_thread_custom_data_set(تهی *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#شامل <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی *z_impl_k_thread_custom_data_get(تهی)
{
	بازگشت _current->custom_data;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی *z_vrfy_k_thread_custom_data_get(تهی)
{
	بازگشت z_impl_k_thread_custom_data_get();
}
#شامل <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

صحیح z_impl_k_is_preempt_thread(تهی)
{
	بازگشت !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_is_preempt_thread(تهی)
{
	بازگشت z_impl_k_is_preempt_thread();
}
#شامل <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_thread_priority_get(k_tid_t thread)
{
	بازگشت thread->base.prio;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	بازگشت z_impl_k_thread_priority_get(thread);
}
#شامل <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
ایستا تهی set_thread_name(ساختار k_thread *thread, ثابت نویسه *str)
{
	اگر (str != پوچ) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} وگرنه {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

صحیح z_impl_k_thread_name_set(k_tid_t thread, ثابت نویسه *str)
{
#ifdef CONFIG_THREAD_NAME
	اگر (thread == پوچ) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	بازگشت 0;
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	بازگشت -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_thread_name_set(k_tid_t thread, ثابت نویسه *str)
{
#ifdef CONFIG_THREAD_NAME
	نویسه name[CONFIG_THREAD_MAX_NAME_LEN];

	اگر (thread != پوچ) {
		اگر (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			بازگشت -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	اگر (k_usermode_string_copy(name, str, اندازه(name)) != 0) {
		بازگشت -EFAULT;
	}

	بازگشت z_impl_k_thread_name_set(thread, name);
#وگرنه
	بازگشت -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#شامل <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ثابت نویسه *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	بازگشت (ثابت نویسه *)thread->name;
#وگرنه
	ARG_UNUSED(thread);
	بازگشت پوچ;
#endif /* CONFIG_THREAD_NAME */
}

صحیح z_impl_k_thread_name_copy(k_tid_t thread, نویسه *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	بازگشت 0;
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	بازگشت -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

ایستا size_t copy_bytes(نویسه *dest, size_t dest_size, ثابت نویسه *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	بازگشت bytes_to_copy;
}

ثابت نویسه *k_thread_state_str(k_tid_t thread_id, نویسه *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, اندازه(Z_STATE_STR_##s) - 1 }
	ایستا ثابت ساختار {
		ثابت نویسه *str;
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

	اگر ((buf == پوچ) || (buf_size == 0)) {
		بازگشت "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	برای (unsigned صحیح index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		اگر ((thread_state & bit) == 0) {
			ادامه;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		اگر (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	بازگشت (ثابت نویسه *)buf;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_thread_name_copy(k_tid_t thread,
					    نویسه *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	ساختار k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	اگر ((ko == پوچ) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		بازگشت -EINVAL;
	}
	اگر (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		بازگشت -EFAULT;
	}
	len = strlen(thread->name);
	اگر ((len + 1) > size) {
		بازگشت -ENOSPC;
	}

	بازگشت k_usermode_to_copy((تهی *)buf, thread->name, len + 1);
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	بازگشت -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#شامل <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_پشته_SENTINEL
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
تهی z_check_پشته_sentinel(تهی)
{
	uint32_t *پشته;

	اگر ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		بازگشت;
	}

	پشته = (uint32_t *)_current->پشته_info.start;
	اگر (*پشته != پشته_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*پشته = پشته_SENTINEL;
		z_except_reason(K_ERR_پشته_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#اگر defined(CONFIG_پشته_POINTER_RANDOM) && (CONFIG_پشته_POINTER_RANDOM != 0)
صحیح z_پشته_adjust_initialized;

ایستا size_t random_offset(size_t پشته_size)
{
	size_t random_val;

	اگر (!z_پشته_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, اندازه(random_val));
	} وگرنه {
		sys_rand_get((uint8_t *)&random_val, اندازه(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ثابت size_t fuzz = random_val % CONFIG_پشته_POINTER_RANDOM;

	اگر (unlikely(fuzz * 2 > پشته_size)) {
		بازگشت 0;
	}

	بازگشت fuzz;
}
#اگر defined(CONFIG_پشته_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
ساختار پشته_geometry {
	size_t obj_size;	/* size of the whole stack object */
	نویسه *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ایستا ساختار پشته_geometry compute_پشته_geometry(k_thread_پشته_t *پشته,
						    size_t پشته_size)
{
	ساختار پشته_geometry geo;

#ifdef CONFIG_USERSPACE
	اگر (z_پشته_is_user_capable(پشته)) {
		geo.obj_size = K_THREAD_پشته_LEN(پشته_size);
		geo.buf_start = K_THREAD_پشته_BUFFER(پشته);
		geo.buf_size = geo.obj_size - K_THREAD_پشته_RESERVED;
	} وگرنه
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_پشته_LEN(پشته_size);
		geo.buf_start = K_KERNEL_پشته_BUFFER(پشته);
		geo.buf_size = geo.obj_size - K_KERNEL_پشته_RESERVED;

#اگر defined(ARCH_KERNEL_پشته_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		اگر (K_KERNEL_پشته_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	بازگشت geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ایستا نویسه *map_thread_پشته(ساختار k_thread *new_thread,
			      k_thread_پشته_t *پشته,
			      ساختار پشته_geometry *geo)
{
	نویسه *پشته_ptr;

#ifdef CONFIG_THREAD_پشته_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	تهی *پشته_mapped = k_mem_map_phys_guard((uintptr_t)پشته, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)پشته_mapped != 0);

#ifdef CONFIG_USERSPACE
	اگر (z_پشته_is_user_capable(پشته)) {
		geo->buf_start = K_THREAD_پشته_BUFFER(پشته_mapped);
	} وگرنه
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_پشته_BUFFER(پشته_mapped);
	}

	پشته_ptr = (نویسه *)پشته_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->پشته_info.mapped.addr = پشته_mapped;
	new_thread->پشته_info.mapped.sz = geo->obj_size;
#وگرنه /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	پشته_ptr = (نویسه *)پشته + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	بازگشت پشته_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ایستا درونخطی تهی fill_init_پشته(نویسه *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#وگرنه
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ایستا درونخطی تهی set_پشته_sentinel(نویسه *buf_start)
{
#ifdef CONFIG_پشته_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = پشته_SENTINEL;
#وگرنه
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ایستا size_t reserve_پشته_headroom(ساختار k_thread *new_thread,
				     نویسه *پشته_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(پشته_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_پشته_setup(new_thread, پشته_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = اندازه(ساختار _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(ساختار _thread_userspace_local_data *)(پشته_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#اگر defined(CONFIG_پشته_POINTER_RANDOM) && (CONFIG_پشته_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	بازگشت ROUND_UP(delta, ARCH_پشته_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ایستا درونخطی تهی set_پشته_info(ساختار k_thread *new_thread, نویسه *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_پشته_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->پشته_info.start = (uintptr_t)buf_start;
	new_thread->پشته_info.size = buf_size;
	new_thread->پشته_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_پشته_SAFETY
	new_thread->پشته_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_پشته_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#وگرنه
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

ایستا نویسه *setup_thread_پشته(ساختار k_thread *new_thread,
				k_thread_پشته_t *پشته, size_t پشته_size)
{
	ساختار پشته_geometry geo = compute_پشته_geometry(پشته, پشته_size);
	نویسه *پشته_ptr = map_thread_پشته(new_thread, پشته, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		پشته, new_thread, geo.obj_size, (تهی *)geo.buf_start,
		geo.buf_size, (تهی *)پشته_ptr);

	fill_init_پشته(geo.buf_start, geo.buf_size);
	set_پشته_sentinel(geo.buf_start);
	delta = reserve_پشته_headroom(new_thread, پشته_ptr, geo.buf_size);
	set_پشته_info(new_thread, geo.buf_start, geo.buf_size, delta);

	بازگشت پشته_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_پشته
ایستا تهی setup_shadow_پشته(ساختار k_thread *new_thread,
			 k_thread_پشته_t *پشته)
{
	صحیح ret = -ENOENT;

	STRUCT_SECTION_FOREACH(پشته_to_hw_shadow_پشته, stk_to_hw_shstk) {
		اگر (stk_to_hw_shstk->پشته == پشته) {
			ret = k_thread_hw_shadow_پشته_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			اگر (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			بشکن;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	اگر (ret != -ENOENT) {
		بازگشت;
	}

	STRUCT_SECTION_FOREACH(پشته_to_hw_shadow_پشته_arr, stk_to_hw_shstk) {
		اگر ((uintptr_t)پشته >= stk_to_hw_shstk->پشته_addr &&
		    (uintptr_t)پشته < stk_to_hw_shstk->پشته_addr +
		    stk_to_hw_shstk->پشته_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t پشته_offset = (uintptr_t)پشته - stk_to_hw_shstk->پشته_addr;
			uintptr_t پشته_index = پشته_offset / stk_to_hw_shstk->پشته_size;
			uintptr_t addr;

			اگر (پشته_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, پشته);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * پشته_index;
			ret = k_thread_hw_shadow_پشته_attach(new_thread,
							      (arch_thread_hw_shadow_پشته_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			اگر (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			بشکن;
		}
	}

	اگر (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, پشته);
		k_panic();
	}
}
#وگرنه
/* No-op stub used when hardware shadow stacks are not configured. */
ایستا درونخطی تهی setup_shadow_پشته(ساختار k_thread *new_thread,
				      k_thread_پشته_t *پشته)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(پشته);
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
ایستا درونخطی تهی thread_abort_cleanup_check_reuse(ساختار k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ایستا درونخطی تهی init_thread_obj_core(ساختار k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  اندازه(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ایستا درونخطی تهی init_thread_userspace(ساختار k_thread *thread,
					 k_thread_پشته_t *پشته,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_پشته_is_user_capable(پشته),
		 "user thread %p with kernel-only stack %p", thread, پشته);
	k_object_init(thread);
	k_object_init(پشته);
	thread->پشته_obj = پشته;
	thread->syscall_frame = پوچ;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(پشته);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ایستا درونخطی تهی assert_thread_coherence(ساختار k_thread *thread,
					   k_thread_پشته_t *پشته)
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
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(پشته));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(پشته);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ایستا درونخطی تهی assert_switch_handle(ساختار k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != پوچ,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ایستا درونخطی تهی init_thread_custom_data(ساختار k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = پوچ;
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ایستا درونخطی تهی add_thread_to_monitor(ساختار k_thread *thread,
					 k_thread_entry_t entry,
					 تهی *p1, تهی *p2, تهی *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_کلید_t کلید = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, کلید);
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ایستا درونخطی تهی init_thread_name(ساختار k_thread *thread, ثابت نویسه *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ایستا درونخطی تهی init_thread_cpu_mask(ساختار k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	اگر (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} وگرنه {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ایستا درونخطی تهی init_thread_userspace_perms(ساختار k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	اگر ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ایستا درونخطی تهی init_thread_deadline(ساختار k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ایستا درونخطی تهی init_thread_halt_queue(ساختار k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ایستا درونخطی تهی init_thread_usage(ساختار k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (ساختار k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ایستا درونخطی تهی assert_thread_not_reused(ساختار k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_کلید_t کلید = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	برای (ساختار k_thread *t = _kernel.threads; t; t = t->next_thread) {
		اگر (t == new_thread) {
			reused = true;
			بشکن;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, کلید);

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
نویسه *z_setup_new_thread(ساختار k_thread *new_thread,
			 k_thread_پشته_t *پشته, size_t پشته_size,
			 k_thread_entry_t entry,
			 تهی *p1, تهی *p2, تهی *p3,
			 صحیح prio, uint32_t options, ثابت نویسه *name)
{
	نویسه *پشته_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, پشته, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	پشته_ptr = setup_thread_پشته(new_thread, پشته, پشته_size);

#اگر Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = پوچ;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_پشته(new_thread, پشته);
	assert_thread_coherence(new_thread, پشته);

	arch_new_thread(new_thread, پشته, پشته_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = پوچ;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	اگر (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == پوچ)) {
		new_thread->resource_pool = پوچ;
		بازگشت پشته_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	بازگشت پشته_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_پشته_SAFETY
صحیح z_impl_k_thread_runtime_پشته_unused_threshold_pct_set(ساختار k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	اگر (pct > 99) {
		بازگشت -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->پشته_info.size * pct) / 100;

	thread->پشته_info.usage.unused_threshold = unused_threshold;

	بازگشت 0;
}

صحیح z_impl_k_thread_runtime_پشته_unused_threshold_set(ساختار k_thread *thread,
						       size_t threshold)
{
	اگر (threshold > thread->پشته_info.size) {
		بازگشت -EINVAL;
	}

	thread->پشته_info.usage.unused_threshold = threshold;

	بازگشت 0;
}

size_t z_impl_k_thread_runtime_پشته_unused_threshold_get(ساختار k_thread *thread)
{
	بازگشت thread->پشته_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_thread_runtime_پشته_unused_threshold_pct_set(ساختار k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	بازگشت z_impl_k_thread_runtime_پشته_unused_threshold_pct_set(thread, pct);
}
#شامل <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

صحیح z_vrfy_k_thread_runtime_پشته_unused_threshold_set(ساختار k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	بازگشت z_impl_k_thread_runtime_پشته_unused_threshold_set(thread, threshold);
}
#شامل <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_پشته_unused_threshold_get(ساختار k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	بازگشت z_impl_k_thread_runtime_پشته_unused_threshold_get(thread);
}
#شامل <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(ساختار k_thread *new_thread,
			      k_thread_پشته_t *پشته,
			      size_t پشته_size, k_thread_entry_t entry,
			      تهی *p1, تهی *p2, تهی *p3,
			      صحیح prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, پشته, پشته_size, entry, p1, p2, p3,
			  prio, options, پوچ);

	thread_schedule_new(new_thread, delay);

	بازگشت new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_پشته_is_user_capable(k_thread_پشته_t *پشته)
{
	بازگشت k_object_find(پشته) != پوچ;
}

k_tid_t z_vrfy_k_thread_create(ساختار k_thread *new_thread,
			       k_thread_پشته_t *پشته,
			       size_t پشته_size, k_thread_entry_t entry,
			       تهی *p1, تهی *p2, تهی *p3,
			       صحیح prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, پشته_obj_size;
	ساختار k_object *پشته_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	پشته_object = k_object_find(پشته);
	K_OOPS(K_SYSCALL_VERIFY_پیام(k_object_validation_check(پشته_object, پشته,
						K_OBJ_THREAD_پشته_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_پیام(!size_add_overflow(K_THREAD_پشته_RESERVED,
						       پشته_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    پشته_size,
				    K_THREAD_پشته_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	پشته_obj_size = پشته_object->data.پشته_data->size;
#وگرنه
	پشته_obj_size = پشته_object->data.پشته_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_پیام(total_size <= پشته_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, پشته_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, پوچ)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, پشته, پشته_size,
			   entry, p1, p2, p3, prio, options, پوچ);

	thread_schedule_new(new_thread, delay);

	بازگشت new_thread;
}
#شامل <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_init_thread_base(ساختار _thread_base *thread_base, صحیح priority,
		       uint32_t initial_state, unsigned صحیح options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = پوچ;
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
	thread_base->slice_expired = پوچ;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = پوچ;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN تهی k_thread_user_mode_enter(k_thread_entry_t entry,
					    تهی *p1, تهی *p2, تهی *p3)
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
	__ASSERT(z_پشته_is_user_capable(_current->پشته_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       اندازه(ساختار _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_پشته_setup(_current,
			     (نویسه *)(_current->پشته_info.start +
				      _current->پشته_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#وگرنه
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#اگر defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_پشته_INFO)
#ifdef CONFIG_پشته_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

صحیح z_پشته_space_get(ثابت uint8_t *پشته_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
	ثابت uint8_t *checked_پشته = پشته_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ثابت uint8_t *پشته_pointer = (ثابت uint8_t *)&پشته_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	اگر ((پشته_pointer > پشته_start) && (پشته_pointer <= (پشته_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_پشته_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		بازگشت -ENOTSUP;
	}

	اگر (IS_ENABLED(CONFIG_پشته_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_پشته += 4;
		size -= 4;
	}

	برای (size_t i = 0; i < size; i++) {
		اگر ((checked_پشته[i]) == 0xaaU) {
			unused++;
		} وگرنه {
			بشکن;
		}
	}

	*unused_ptr = unused;

	بازگشت 0;
}

#ifdef CONFIG_THREAD_RUNTIME_پشته_SAFETY
صحیح k_thread_runtime_پشته_safety_full_check(ثابت ساختار k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_پشته_safety_handler_t handler,
					     تهی *arg)
{
	صحیح    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != پوچ);

	rv = z_پشته_space_get((ثابت uint8_t *)thread->پشته_info.start,
			       thread->پشته_info.size, &unused_space);

	اگر (rv != 0) {
		بازگشت rv;
	}

	اگر (unused_ptr != پوچ) {
		*unused_ptr = unused_space;
	}

	اگر ((unused_space < thread->پشته_info.usage.unused_threshold) &&
	    (handler != پوچ)) {
		handler(thread, unused_space, arg);
	}

	بازگشت 0;
}

صحیح k_thread_runtime_پشته_safety_threshold_check(ثابت ساختار k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_پشته_safety_handler_t handler,
						  تهی *arg)
{
	صحیح    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != پوچ);

	threshold = thread->پشته_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	اگر (IS_ENABLED(CONFIG_پشته_SENTINEL) && (threshold != 0U)) {
		scan_size += اندازه(uint32_t);
	}

	rv = z_پشته_space_get((ثابت uint8_t *)thread->پشته_info.start,
			       scan_size, &unused_space);

	اگر (rv != 0) {
		بازگشت rv;
	}

	اگر (unused_ptr != پوچ) {
		*unused_ptr = unused_space;
	}

	اگر ((unused_space < threshold) && (handler != پوچ)) {
		handler(thread, unused_space, arg);
	}

	بازگشت 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

صحیح z_impl_k_thread_پشته_space_get(ثابت ساختار k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_پشته_MEM_MAPPED
	اگر (thread->پشته_info.mapped.addr == پوچ) {
		بازگشت -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	بازگشت z_پشته_space_get((ثابت uint8_t *)thread->پشته_info.start,
				 thread->پشته_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_thread_پشته_space_get(ثابت ساختار k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	صحیح ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_پشته_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		بازگشت ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, اندازه(size_t));
	CHECKIF(ret != 0) {
		بازگشت ret;
	}

	بازگشت 0;
}
#شامل <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
ایستا درونخطی k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    ثابت ساختار k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	بازگشت z_impl_k_thread_timeout_remaining_ticks(thread);
}
#شامل <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ایستا درونخطی k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ثابت ساختار k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	بازگشت z_impl_k_thread_timeout_expires_ticks(thread);
}
#شامل <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
تهی z_thread_mark_switched_in(تهی)
{
#اگر defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

تهی z_thread_mark_switched_out(تهی)
{
#اگر defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	اگر (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		بازگشت;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

صحیح k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	اگر ((thread == پوچ) || (stats == پوچ)) {
		بازگشت -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#وگرنه
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	بازگشت 0;
}

صحیح k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	اگر (stats == پوچ) {
		بازگشت -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned صحیح رقم_cpus = arch_رقم_cpus();

	برای (unsigned صحیح i = 0; i < رقم_cpus; i++) {
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

	بازگشت 0;
}

صحیح k_thread_runtime_stats_cpu_get(صحیح cpu, k_thread_runtime_stats_t *stats)
{
	اگر (stats == پوچ) {
		بازگشت -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned صحیح)cpu >= arch_رقم_cpus())) {
		بازگشت -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#وگرنه
	ARG_UNUSED(cpu);
#endif

	بازگشت 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ایستا ساختار k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
ایستا ساختار k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_پشته_MEM_MAPPED
ایستا تهی *thread_cleanup_پشته_addr;
ایستا size_t thread_cleanup_پشته_sz;

ایستا تهی unmap_thread_پشته(تهی *addr, size_t sz)
{
	اگر (addr != پوچ) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

تهی defer_thread_cleanup(ساختار k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_پشته_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_پشته_addr = thread->پشته_info.mapped.addr;
	thread_cleanup_پشته_sz = thread->پشته_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->پشته_info.mapped.addr = پوچ;
	thread->پشته_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

تهی do_deferred_thread_cleanup(تهی)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_پشته_MEM_MAPPED
	unmap_thread_پشته(thread_cleanup_پشته_addr, thread_cleanup_پشته_sz);

	thread_cleanup_پشته_addr = پوچ;
	thread_cleanup_پشته_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

تهی do_thread_cleanup(ساختار k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_پشته_MEM_MAPPED
	unmap_thread_پشته(thread->پشته_info.mapped.addr,
			   thread->پشته_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->پشته_info.mapped.addr = پوچ;
	thread->پشته_info.mapped.sz = 0;
#وگرنه /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

تهی k_thread_abort_cleanup(ساختار k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		اگر (thread_to_cleanup != پوچ) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = پوچ;
		}

		اگر (thread == _current) {
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
		} وگرنه {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

تهی k_thread_abort_cleanup_check_reuse(ساختار k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		اگر (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = پوچ;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

تهی z_dummy_thread_init(ساختار k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#اگر Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = پوچ;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_پشته_INFO
	dummy_thread->پشته_info.start = 0U;
	dummy_thread->پشته_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#اگر (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#وگرنه
	dummy_thread->resource_pool = پوچ;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = پوچ;
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

تهی z_impl_k_thread_priority_set(k_tid_t thread, صحیح prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, پوچ);

	bool need_sched = z_thread_prio_set((ساختار k_thread *)thread, prio);

	اگر ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_thread_priority_set(k_tid_t thread, صحیح prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_پیام(_is_valid_prio(prio, پوچ),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_پیام((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#شامل <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	اگر (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		بازگشت;
	}

	k_spinlock_کلید_t کلید = z_sched_spinlock_lock();

	اگر (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(کلید);
		بازگشت;
	}

	z_thread_halt(thread, کلید, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#شامل <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	اگر (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(کلید);
		بازگشت;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(کلید);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#شامل <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_lock();

	اگر (z_is_thread_sleeping(thread)) {
		(تهی)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(کلید);
	} وگرنه {
		z_sched_spinlock_unlock(کلید);
	}
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#شامل <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_thread_abort(ساختار k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_کلید_t کلید = z_sched_spinlock_lock();

	اگر (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(کلید);
		بازگشت;
	}

	z_thread_halt(thread, کلید, true);

	اگر (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#اگر !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
تهی z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

صحیح z_impl_k_thread_join(ساختار k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_lock();
	صحیح ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	اگر (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} وگرنه اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} وگرنه اگر ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} وگرنه {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(کلید);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		بازگشت ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(کلید);
	بازگشت ret;
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
ایستا bool thread_obj_validate(ساختار k_thread *thread)
{
	ساختار k_object *ko = k_object_find(thread);
	صحیح ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	انتخاب (ret) {
	حالت 0:
		بازگشت false;
	حالت -EINVAL:
		بازگشت true;
	پیشفرض:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_پیام(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ایستا درونخطی صحیح z_vrfy_k_thread_join(ساختار k_thread *thread,
				       k_timeout_t timeout)
{
	اگر (thread_obj_validate(thread)) {
		بازگشت 0;
	}

	بازگشت z_impl_k_thread_join(thread, timeout);
}
#شامل <zephyr/syscalls/k_thread_join_mrsh.c>

ایستا درونخطی تهی z_vrfy_k_thread_abort(k_tid_t thread)
{
	اگر (thread_obj_validate(thread)) {
		بازگشت;
	}

	K_OOPS(K_SYSCALL_VERIFY_پیام(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((ساختار k_thread *)thread);
}
#شامل <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(تهی)
{
	بازگشت !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

تهی z_impl_k_yield(تهی)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_yield(تهی)
{
	z_impl_k_yield();
}
#شامل <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
