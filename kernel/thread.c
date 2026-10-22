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

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/clock.h>
#ضمن <ksched.h>
#ضمن <kthread.h>
#ضمن <wait_q.h>
#ضمن <timeout_q.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <kernel_internal.h>
#ضمن <kswap.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/tracing/tracing.h>
#ضمن <string.h>
#ضمن <stdbool.h>
#ضمن <zephyr/sys/check.h>
#ضمن <zephyr/random/random.h>
#ضمن <zephyr/sys/atomic.h>
#ضمن <zephyr/logging/log.h>
#ضمن <zephyr/llext/symbol.h>
#ضمن <zephyr/sys/iterable_sections.h>
#ضمن <zephyr/sys/minmax.h>

#ضمن <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_THREAD
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_STATS_THREAD
ساكن بنية k_obj_core_stats_desc  thread_stats_desc = {
	.raw_سائيز = حجم(بنية k_چڪر_stats),
	.query_سائيز = حجم(بنية k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#وإلا
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STATS_THREAD */
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_THREAD */


#وصف_ڪريو _FOREACH_STATIC_THREAD(thread_معلومات)              \
	STRUCT_SECTION_FOREACH(static_thread_معلومات, thread_معلومات)

bool k_is_اندريون_isr(فراغ)
{
	أرجع arch_is_اندريون_isr();
}
EXPORT_نشان(k_is_اندريون_isr);

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_CUSTOM_معلومات
فراغ z_impl_k_thread_custom_معلومات_سيٽ(فراغ *قدر)
{
	_current->custom_معلومات = قدر;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_custom_معلومات_سيٽ(فراغ *معلومات)
{
	z_impl_k_thread_custom_معلومات_سيٽ(معلومات);
}
#ضمن <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ *z_impl_k_thread_custom_معلومات_get(فراغ)
{
	أرجع _current->custom_معلومات;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_thread_custom_معلومات_get(فراغ)
{
	أرجع z_impl_k_thread_custom_معلومات_get();
}
#ضمن <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#جيڪڏهن_پورو /* CONFIG_USERSPACE */
#جيڪڏهن_پورو /* CONFIG_THREAD_CUSTOM_DATA */

صحيح z_impl_k_is_preempt_thread(فراغ)
{
	أرجع !arch_is_اندريون_isr() && thread_is_preemptible(_current);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_is_preempt_thread(فراغ)
{
	أرجع z_impl_k_is_preempt_thread();
}
#ضمن <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_ترجيح_get(k_tid_t thread)
{
	أرجع thread->بنياد.prio;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ترجيح_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_ترجيح_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME
ساكن فراغ سيٽ_thread_name(بنية k_thread *thread, ثابت حرف *str)
{
	إذا (str != عدم) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_سيٽ(thread, str);
#جيڪڏهن_پورو /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} وإلا {
		thread->name[0] = '\0';
	}
}
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */

صحيح z_impl_k_thread_name_سيٽ(k_tid_t thread, ثابت حرف *str)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME
	إذا (thread == عدم) {
		thread = _current;
	}

	سيٽ_thread_name(thread, str);

	SYS_پورٽ_TRACING_OBJ_FUNC(k_thread, name_سيٽ, thread, 0);

	أرجع 0;
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(str);

	SYS_پورٽ_TRACING_OBJ_FUNC(k_thread, name_سيٽ, thread, -ENOSYS);

	أرجع -ENOSYS;
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_سيٽ(k_tid_t thread, ثابت حرف *str)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME
	حرف name[CONFIG_THREAD_MAX_NAME_LEN];

	إذا (thread != عدم) {
		إذا (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			أرجع -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	إذا (k_usermode_string_copy(name, str, حجم(name)) != 0) {
		أرجع -EFAULT;
	}

	أرجع z_impl_k_thread_name_سيٽ(thread, name);
#وإلا
	أرجع -ENOSYS;
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_set_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

ثابت حرف *k_thread_name_get(k_tid_t thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME
	أرجع (ثابت حرف *)thread->name;
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	أرجع عدم;
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */
}

صحيح z_impl_k_thread_name_copy(k_tid_t thread, حرف *buf, size_t سائيز)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME
	strncpy(buf, thread->name, سائيز);
	أرجع 0;
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(buf);
	ARG_اڻ_استعمال_ٿيل(سائيز);
	أرجع -ENOSYS;
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */
}

ساكن size_t copy_بائيٽون(حرف *dest, size_t dest_سائيز, ثابت حرف *src, size_t src_سائيز)
{
	size_t  بائيٽون_منزل_copy;

	بائيٽون_منزل_copy = min(dest_سائيز, src_سائيز);
	يادگيري_نقل(dest, src, بائيٽون_منزل_copy);

	أرجع بائيٽون_منزل_copy;
}

ثابت حرف *k_thread_state_str(k_tid_t thread_id, حرف *buf, size_t buf_سائيز)
{
	size_t      off = 0;
	uint8_t     ٻٽو_انگ_2;
	uint8_t     thread_state = thread_id->بنياد.thread_state;
#وصف_ڪريو SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, حجم(Z_STATE_STR_##s) - 1 }
	ساكن ثابت بنية {
		ثابت حرف *str;
		uint16_t    ٻٽو_انگ_2;
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
#وصف_رد SS_ENT

	إذا ((buf == عدم) || (buf_سائيز == 0)) {
		أرجع "";
	}

	buf_سائيز--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	لكل (بنانشان صحيح اشاريو = 0; thread_state != 0; اشاريو++) {
		ٻٽو_انگ_2 = state_string[اشاريو].ٻٽو_انگ_2;
		إذا ((thread_state & ٻٽو_انگ_2) == 0) {
			استمر;
		}

		off += copy_بائيٽون(buf + off, buf_سائيز - off,
				  state_string[اشاريو].str,
				  state_string[اشاريو].len);

		thread_state &= ~ٻٽو_انگ_2;

		إذا (thread_state != 0) {
			off += copy_بائيٽون(buf + off, buf_سائيز - off, "+", 1);
		}
	}

	buf[off] = '\0';

	أرجع (ثابت حرف *)buf;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_copy(k_tid_t thread,
					    حرف *buf, size_t سائيز)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME
	size_t len;
	بنية k_object *ko = k_object_ڳوليو(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	إذا ((ko == عدم) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		أرجع -EINVAL;
	}
	إذا (K_SYSCALL_يادگيري_WRITE(buf, سائيز) != 0) {
		أرجع -EFAULT;
	}
	len = لڙي_ڊيگهه(thread->name);
	إذا ((len + 1) > سائيز) {
		أرجع -ENOSPC;
	}

	أرجع k_usermode_منزل_copy((فراغ *)buf, thread->name, len + 1);
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(buf);
	ARG_اڻ_استعمال_ٿيل(سائيز);
	أرجع -ENOSYS;
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_وصف_ٿيل CONFIG_ڍير_SENTINEL
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
فراغ z_check_ڍير_sentinel(فراغ)
{
	uint32_t *ڍير;

	إذا ((_current->بنياد.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}

	ڍير = (uint32_t *)_current->ڍير_info.شروعات;
	إذا (*ڍير != ڍير_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ڍير = ڍير_SENTINEL;
		z_except_reason(K_ERR_ڍير_CHK_FAIL);
	}
}
#جيڪڏهن_پورو /* CONFIG_STACK_SENTINEL */

#إذا defined(CONFIG_ڍير_اشارو_RANDOM) && (CONFIG_ڍير_اشارو_RANDOM != 0)
صحيح z_ڍير_adjust_initialized;

ساكن size_t random_وٿي(size_t ڍير_سائيز)
{
	size_t random_val;

	إذا (!z_ڍير_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, حجم(random_val));
	} وإلا {
		sys_rand_get((uint8_t *)&random_val, حجم(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ثابت size_t fuzz = random_val % CONFIG_ڍير_اشارو_RANDOM;

	إذا (unlikely(fuzz * 2 > ڍير_سائيز)) {
		أرجع 0;
	}

	أرجع fuzz;
}
#إذا defined(CONFIG_ڍير_GROWS_مٿي)
	/* This is so rare not bothering for now */
#غلطي "Stack pointer randomization not implemented for upward growing stacks"
#جيڪڏهن_پورو /* CONFIG_STACK_GROWS_UP */
#جيڪڏهن_پورو /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
بنية ڍير_geometry {
	size_t obj_سائيز;	/* size of the whole stack object */
	حرف *buf_شروعات;	/* start of the usable stack buffer */
	size_t buf_سائيز;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ساكن بنية ڍير_geometry compute_ڍير_geometry(k_thread_ڍير_t *ڍير,
						    size_t ڍير_سائيز)
{
	بنية ڍير_geometry geo;

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
	إذا (z_ڍير_is_user_capable(ڍير)) {
		geo.obj_سائيز = K_THREAD_ڍير_LEN(ڍير_سائيز);
		geo.buf_شروعات = K_THREAD_ڍير_بفر(ڍير);
		geo.buf_سائيز = geo.obj_سائيز - K_THREAD_ڍير_RESERVED;
	} وإلا
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_سائيز = K_KERNEL_ڍير_LEN(ڍير_سائيز);
		geo.buf_شروعات = K_KERNEL_ڍير_بفر(ڍير);
		geo.buf_سائيز = geo.obj_سائيز - K_KERNEL_ڍير_RESERVED;

#إذا defined(ARCH_KERNEL_ڍير_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		إذا (K_KERNEL_ڍير_RESERVED > geo.obj_سائيز) {
			k_panic();
		}
#جيڪڏهن_پورو
	}

	أرجع geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ساكن حرف *map_thread_ڍير(بنية k_thread *new_thread,
			      k_thread_ڍير_t *ڍير,
			      بنية ڍير_geometry *geo)
{
	حرف *ڍير_ptr;

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	فراغ *ڍير_mapped = k_mem_map_phys_guard((uintptr_t)ڍير, geo->obj_سائيز,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ڍير_mapped != 0);

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
	إذا (z_ڍير_is_user_capable(ڍير)) {
		geo->buf_شروعات = K_THREAD_ڍير_بفر(ڍير_mapped);
	} وإلا
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
	{
		geo->buf_شروعات = K_KERNEL_ڍير_بفر(ڍير_mapped);
	}

	ڍير_ptr = (حرف *)ڍير_mapped + geo->obj_سائيز;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ڍير_info.mapped.addr = ڍير_mapped;
	new_thread->ڍير_info.mapped.sz = geo->obj_سائيز;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_اڻ_استعمال_ٿيل(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ڍير_ptr = (حرف *)ڍير + geo->obj_سائيز;
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع ڍير_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ساكن مضمن فراغ fill_init_ڍير(حرف *buf_شروعات, size_t buf_سائيز)
{
#جيڪڏهن_وصف_ٿيل CONFIG_INIT_STACKS
	memset(buf_شروعات, 0xaa, buf_سائيز);
#وإلا
	ARG_اڻ_استعمال_ٿيل(buf_شروعات);
	ARG_اڻ_استعمال_ٿيل(buf_سائيز);
#جيڪڏهن_پورو /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ساكن مضمن فراغ سيٽ_ڍير_sentinel(حرف *buf_شروعات)
{
#جيڪڏهن_وصف_ٿيل CONFIG_ڍير_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_شروعات) = ڍير_SENTINEL;
#وإلا
	ARG_اڻ_استعمال_ٿيل(buf_شروعات);
#جيڪڏهن_پورو /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ساكن size_t reserve_ڍير_headroom(بنية k_thread *new_thread,
				     حرف *ڍير_ptr, size_t buf_سائيز)
{
	size_t delta = 0;

	ARG_اڻ_استعمال_ٿيل(new_thread);
	ARG_اڻ_استعمال_ٿيل(ڍير_ptr);
	ARG_اڻ_استعمال_ٿيل(buf_سائيز);

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ڍير_تياري(new_thread, ڍير_ptr);
#جيڪڏهن_پورو /* CONFIG_THREAD_LOCAL_STORAGE */
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_USERSPACE_LOCAL_معلومات
	size_t tls_سائيز = حجم(بنية thread_userspace_local_معلومات);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_سائيز;
	new_thread->userspace_local_معلومات =
		(بنية thread_userspace_local_معلومات *)(ڍير_ptr - delta);
#جيڪڏهن_پورو /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#إذا defined(CONFIG_ڍير_اشارو_RANDOM) && (CONFIG_ڍير_اشارو_RANDOM != 0)
	delta += random_وٿي(buf_سائيز);
#جيڪڏهن_پورو /* CONFIG_STACK_POINTER_RANDOM */

	أرجع ڦيرو_مٿي(delta, ARCH_ڍير_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ساكن مضمن فراغ سيٽ_ڍير_info(بنية k_thread *new_thread, حرف *buf_شروعات,
				  size_t buf_سائيز, size_t delta)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ڍير_info.شروعات = (uintptr_t)buf_شروعات;
	new_thread->ڍير_info.سائيز = buf_سائيز;
	new_thread->ڍير_info.delta = delta;

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_RUNTIME_ڍير_SAFETY
	new_thread->ڍير_info.usage.اڻ_استعمال_ٿيل_حد =
		(CONFIG_THREAD_RUNTIME_ڍير_SAFETY_DEFAULT_اڻ_استعمال_ٿيل_حد_PCT *
		 buf_سائيز) / 100;
#جيڪڏهن_پورو
#وإلا
	ARG_اڻ_استعمال_ٿيل(new_thread);
	ARG_اڻ_استعمال_ٿيل(buf_شروعات);
	ARG_اڻ_استعمال_ٿيل(buf_سائيز);
	ARG_اڻ_استعمال_ٿيل(delta);
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_INFO */
}

ساكن حرف *تياري_thread_ڍير(بنية k_thread *new_thread,
				k_thread_ڍير_t *ڍير, size_t ڍير_سائيز)
{
	بنية ڍير_geometry geo = compute_ڍير_geometry(ڍير, ڍير_سائيز);
	حرف *ڍير_ptr = map_thread_ڍير(new_thread, ڍير, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ڍير, new_thread, geo.obj_سائيز, (فراغ *)geo.buf_شروعات,
		geo.buf_سائيز, (فراغ *)ڍير_ptr);

	fill_init_ڍير(geo.buf_شروعات, geo.buf_سائيز);
	سيٽ_ڍير_sentinel(geo.buf_شروعات);
	delta = reserve_ڍير_headroom(new_thread, ڍير_ptr, geo.buf_سائيز);
	سيٽ_ڍير_info(new_thread, geo.buf_شروعات, geo.buf_سائيز, delta);

	أرجع ڍير_ptr - delta;
}

#جيڪڏهن_وصف_ٿيل CONFIG_HW_SHADOW_ڍير
ساكن فراغ تياري_shadow_ڍير(بنية k_thread *new_thread,
			 k_thread_ڍير_t *ڍير)
{
	صحيح ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ڍير_منزل_hw_shadow_ڍير, stk_منزل_hw_shstk) {
		إذا (stk_منزل_hw_shstk->ڍير == ڍير) {
			ret = k_thread_hw_shadow_ڍير_attach(new_thread,
							      stk_منزل_hw_shstk->shstk_addr,
							      stk_منزل_hw_shstk->سائيز);
			إذا (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_منزل_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			اكسر;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	إذا (ret != -ENOENT) {
		أرجع;
	}

	STRUCT_SECTION_FOREACH(ڍير_منزل_hw_shadow_ڍير_arr, stk_منزل_hw_shstk) {
		إذا ((uintptr_t)ڍير >= stk_منزل_hw_shstk->ڍير_addr &&
		    (uintptr_t)ڍير < stk_منزل_hw_shstk->ڍير_addr +
		    stk_منزل_hw_shstk->ڍير_سائيز * stk_منزل_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ڍير_وٿي = (uintptr_t)ڍير - stk_منزل_hw_shstk->ڍير_addr;
			uintptr_t ڍير_اشاريو = ڍير_وٿي / stk_منزل_hw_shstk->ڍير_سائيز;
			uintptr_t addr;

			إذا (ڍير_اشاريو >= stk_منزل_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ڍير);
				k_panic();
			}

			addr = stk_منزل_hw_shstk->shstk_addr +
				stk_منزل_hw_shstk->shstk_سائيز * ڍير_اشاريو;
			ret = k_thread_hw_shadow_ڍير_attach(new_thread,
							      (arch_thread_hw_shadow_ڍير_t *)addr,
							      stk_منزل_hw_shstk->shstk_سائيز);
			إذا (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_منزل_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			اكسر;
		}
	}

	إذا (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ڍير);
		k_panic();
	}
}
#وإلا
/* No-op stub used when hardware shadow stacks are not configured. */
ساكن مضمن فراغ تياري_shadow_ڍير(بنية k_thread *new_thread,
				      k_thread_ڍير_t *ڍير)
{
	ARG_اڻ_استعمال_ٿيل(new_thread);
	ARG_اڻ_استعمال_ٿيل(ڍير);
}
#جيڪڏهن_پورو

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
ساكن مضمن فراغ thread_abort_cleanup_check_reuse(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ساكن مضمن فراغ init_thread_obj_core(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_رجسٽر(K_OBJ_CORE(thread), &thread->بنياد.usage,
				  حجم(thread->بنياد.usage));
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STATS_THREAD */
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ساكن مضمن فراغ init_thread_userspace(بنية k_thread *thread,
					 k_thread_ڍير_t *ڍير,
					 uint32_t options)
{
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ڍير_is_user_capable(ڍير),
		 "user thread %p with kernel-only stack %p", thread, ڍير);
	k_object_init(thread);
	k_object_init(ڍير);
	thread->ڍير_obj = ڍير;
	thread->syscall_چوکٺو = عدم;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(ڍير);
	ARG_اڻ_استعمال_ٿيل(options);
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ساكن مضمن فراغ assert_thread_coherence(بنية k_thread *thread,
					   k_thread_ڍير_t *ڍير)
{
#جيڪڏهن_وصف_ٿيل CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#جيڪڏهن_وصف_نه_ٿيل CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ڍير));
#جيڪڏهن_پورو /* CONFIG_DYNAMIC_THREAD */
#جيڪڏهن_پورو /* CONFIG_KERNEL_COHERENCE */
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(ڍير);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ساكن مضمن فراغ assert_switch_handle(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != عدم,
		 "arch layer failed to initialize switch_handle");
#جيڪڏهن_پورو /* CONFIG_USE_SWITCH */
	ARG_اڻ_استعمال_ٿيل(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ساكن مضمن فراغ init_thread_custom_معلومات(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_CUSTOM_معلومات
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_معلومات = عدم;
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ساكن مضمن فراغ جوڙيو_thread_منزل_monitor(بنية k_thread *thread,
					 k_thread_داخلا_t داخلا,
					 فراغ *p1, فراغ *p2, فراغ *p3)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_MONITOR
	thread->داخلا.p_داخلا = داخلا;
	thread->داخلا.parameter1 = p1;
	thread->داخلا.parameter2 = p2;
	thread->داخلا.parameter3 = p3;

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&z_thread_monitor_تالو);

	thread->ايندڙ_thread = _kernel.ڌاڳا;
	_kernel.ڌاڳا = thread;
	k_spin_unlock(&z_thread_monitor_تالو, ڪنجي);
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(داخلا);
	ARG_اڻ_استعمال_ٿيل(p1);
	ARG_اڻ_استعمال_ٿيل(p2);
	ARG_اڻ_استعمال_ٿيل(p3);
#جيڪڏهن_پورو /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ساكن مضمن فراغ init_thread_name(بنية k_thread *thread, ثابت حرف *name)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME
	سيٽ_thread_name(thread, name);
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(name);
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ساكن مضمن فراغ init_thread_cpu_نقاب(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب
	إذا (IS_ENABLED(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)) {
		thread->بنياد.cpu_نقاب = 1; /* must specify only one cpu */
	} وإلا {
		thread->بنياد.cpu_نقاب = -1; /* allow all cpus */
	}
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ساكن مضمن فراغ init_thread_userspace_perms(بنية k_thread *thread,
					       uint32_t options)
{
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	إذا ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(options);
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ساكن مضمن فراغ init_thread_deadline(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_DEADLINE
	thread->بنياد.prio_deadline = 0;
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ساكن مضمن فراغ init_thread_halt_قطار(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	z_waitq_init(&thread->halt_قطار);
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ساكن مضمن فراغ init_thread_usage(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE
	thread->بنياد.usage = (بنية k_چڪر_stats) {};
	thread->بنياد.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ساكن مضمن فراغ assert_thread_not_reused(بنية k_thread *new_thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_MONITOR
#جيڪڏهن_وصف_ٿيل CONFIG_ASSERT
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&z_thread_monitor_تالو);
	bool reused = false;

	/* Check if the thread is already in the list */
	لكل (بنية k_thread *t = _kernel.ڌاڳا; t; t = t->ايندڙ_thread) {
		إذا (t == new_thread) {
			reused = true;
			اكسر;
		}
	}

	k_spin_unlock(&z_thread_monitor_تالو, ڪنجي);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#جيڪڏهن_پورو /* CONFIG_ASSERT */
#جيڪڏهن_پورو /* CONFIG_THREAD_MONITOR */
	ARG_اڻ_استعمال_ٿيل(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
حرف *z_تياري_new_thread(بنية k_thread *new_thread,
			 k_thread_ڍير_t *ڍير, size_t ڍير_سائيز,
			 k_thread_داخلا_t داخلا,
			 فراغ *p1, فراغ *p2, فراغ *p3,
			 صحيح prio, uint32_t options, ثابت حرف *name)
{
	حرف *ڍير_ptr;

	Z_ASSERT_VALID_PRIO(prio, داخلا);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ڍير, options);

	z_waitq_init(&new_thread->join_قطار);

	/* Initialize various struct k_thread members */
	z_init_thread_بنياد(&new_thread->بنياد, prio, _THREAD_SLEEPING, options);
	ڍير_ptr = تياري_thread_ڍير(new_thread, ڍير, ڍير_سائيز);

#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = عدم;
	new_thread->orig_prio = new_thread->بنياد.prio;
#جيڪڏهن_پورو /* Z_MUTEX_PI_ENABLED */

	تياري_shadow_ڍير(new_thread, ڍير);
	assert_thread_coherence(new_thread, ڍير);

	arch_new_thread(new_thread, ڍير, ڍير_ptr, داخلا, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_معلومات = عدم;

	assert_switch_handle(new_thread);
	init_thread_custom_معلومات(new_thread);
	جوڙيو_thread_منزل_monitor(new_thread, داخلا, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_نقاب(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	إذا (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_مٽا_سٽا_منزل_MAIN) &&
	    (_current == عدم)) {
		new_thread->resource_pool = عدم;
		أرجع ڍير_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_قطار(new_thread);
	init_thread_usage(new_thread);

	SYS_پورٽ_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	أرجع ڍير_ptr;
}

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_RUNTIME_ڍير_SAFETY
صحيح z_impl_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_pct_سيٽ(بنية k_thread *thread,
							   uint32_t pct)
{
	size_t اڻ_استعمال_ٿيل_حد;

	إذا (pct > 99) {
		أرجع -EINVAL;   /* 100% unused stack and up is invalid */
	}

	اڻ_استعمال_ٿيل_حد = (thread->ڍير_info.سائيز * pct) / 100;

	thread->ڍير_info.usage.اڻ_استعمال_ٿيل_حد = اڻ_استعمال_ٿيل_حد;

	أرجع 0;
}

صحيح z_impl_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_سيٽ(بنية k_thread *thread,
						       size_t حد)
{
	إذا (حد > thread->ڍير_info.سائيز) {
		أرجع -EINVAL;
	}

	thread->ڍير_info.usage.اڻ_استعمال_ٿيل_حد = حد;

	أرجع 0;
}

size_t z_impl_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_get(بنية k_thread *thread)
{
	أرجع thread->ڍير_info.usage.اڻ_استعمال_ٿيل_حد;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
صحيح z_vrfy_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_pct_سيٽ(بنية k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_pct_سيٽ(thread, pct);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

صحيح z_vrfy_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_سيٽ(بنية k_thread *thread,
						       size_t حد)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_سيٽ(thread, حد);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_get(بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ڍير_اڻ_استعمال_ٿيل_حد_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
#جيڪڏهن_پورو /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(بنية k_thread *new_thread,
			      k_thread_ڍير_t *ڍير,
			      size_t ڍير_سائيز, k_thread_داخلا_t داخلا,
			      فراغ *p1, فراغ *p2, فراغ *p3,
			      صحيح prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_اندريون_isr(), "Threads may not be created in ISRs");

	z_تياري_new_thread(new_thread, ڍير, ڍير_سائيز, داخلا, p1, p2, p3,
			  prio, options, عدم);

	thread_وقت_جدول_new(new_thread, delay);

	أرجع new_thread;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
bool z_ڍير_is_user_capable(k_thread_ڍير_t *ڍير)
{
	أرجع k_object_ڳوليو(ڍير) != عدم;
}

k_tid_t z_vrfy_k_thread_create(بنية k_thread *new_thread,
			       k_thread_ڍير_t *ڍير,
			       size_t ڍير_سائيز, k_thread_داخلا_t داخلا,
			       فراغ *p1, فراغ *p2, فراغ *p3,
			       صحيح prio, uint32_t options, k_timeout_t delay)
{
	size_t ڪل_سائيز, ڍير_obj_سائيز;
	بنية k_object *ڍير_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ڍير_object = k_object_ڳوليو(ڍير);
	K_OOPS(K_SYSCALL_VERIFY_نياپو(k_object_validation_check(ڍير_object, ڍير,
						K_OBJ_THREAD_ڍير_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_نياپو(!سائيز_جوڙيو_حد_کان_مٿي(K_THREAD_ڍير_RESERVED,
						       ڍير_سائيز, &ڪل_سائيز),
				    "stack size overflow (%zu+%zu)",
				    ڍير_سائيز,
				    K_THREAD_ڍير_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#جيڪڏهن_وصف_ٿيل CONFIG_GEN_PRIV_STACKS
	ڍير_obj_سائيز = ڍير_object->معلومات.ڍير_معلومات->سائيز;
#وإلا
	ڍير_obj_سائيز = ڍير_object->معلومات.ڍير_سائيز;
#جيڪڏهن_پورو /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_نياپو(ڪل_سائيز <= ڍير_obj_سائيز,
				    "stack size %zu is too big, max is %zu",
				    ڪل_سائيز, ڍير_obj_سائيز));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, عدم)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_هيٺاهون_or_equal(prio,
							_current->بنياد.prio)));

	z_تياري_new_thread(new_thread, ڍير, ڍير_سائيز,
			   داخلا, p1, p2, p3, prio, options, عدم);

	thread_وقت_جدول_new(new_thread, delay);

	أرجع new_thread;
}
#ضمن <zephyr/syscalls/k_thread_create_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ z_init_thread_بنياد(بنية thread_بنياد *thread_بنياد_2, صحيح ترجيح,
		       uint32_t initial_state, بنانشان صحيح options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_بنياد_2->pended_on = عدم;
	thread_بنياد_2->user_options = (uint16_t)options;
	thread_بنياد_2->thread_state = (uint8_t)initial_state;

	thread_بنياد_2->prio = ترجيح;

	thread_بنياد_2->sched_locked = 0U;

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	thread_بنياد_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_بنياد_2->cpu = 0;
#جيڪڏهن_پورو /* CONFIG_SMP */

#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICE_في_THREAD
	thread_بنياد_2->ٽڪرو_ticks = 0;
	thread_بنياد_2->ٽڪرو_expired = عدم;
#جيڪڏهن_پورو /* CONFIG_TIMESLICE_PER_THREAD */

#جيڪڏهن_وصف_ٿيل CONFIG_SPIN_VALIDATE
	thread_بنياد_2->مٽا_سٽا_معلومات = عدم;
	/* otherwise swap_data does not need to be initialized */
#جيڪڏهن_پورو

	z_init_thread_timeout(thread_بنياد_2);
}

FUNC_NORETURN فراغ k_thread_user_گهڻائي_enter(k_thread_داخلا_t داخلا,
					    فراغ *p1, فراغ *p2, فراغ *p3)
{
	SYS_پورٽ_TRACING_FUNC(k_thread, user_گهڻائي_enter);

	_current->بنياد.user_options |= K_USER;
	z_thread_essential_clear(_current);
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_MONITOR
	_current->داخلا.p_داخلا = داخلا;
	_current->داخلا.parameter1 = p1;
	_current->داخلا.parameter2 = p2;
	_current->داخلا.parameter3 = p3;
#جيڪڏهن_پورو /* CONFIG_THREAD_MONITOR */
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
	__ASSERT(z_ڍير_is_user_capable(_current->ڍير_obj),
		 "dropping to user mode with kernel-only stack object");
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_USERSPACE_LOCAL_معلومات
	memset(_current->userspace_local_معلومات, 0,
	       حجم(بنية thread_userspace_local_معلومات));
#جيڪڏهن_پورو /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ڍير_تياري(_current,
			     (حرف *)(_current->ڍير_info.شروعات +
				      _current->ڍير_info.سائيز));
#جيڪڏهن_پورو /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_گهڻائي_enter(داخلا, p1, p2, p3);
#وإلا
	/* XXX In this case we do not reset the stack */
	z_thread_داخلا(داخلا, p1, p2, p3);
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
}

#إذا defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ڍير_INFO)
#جيڪڏهن_وصف_ٿيل CONFIG_ڍير_GROWS_مٿي
#غلطي "Unsupported configuration for stack analysis"
#جيڪڏهن_پورو /* CONFIG_STACK_GROWS_UP */

صحيح z_ڍير_space_get(ثابت uint8_t *ڍير_شروعات, size_t سائيز, size_t *اڻ_استعمال_ٿيل_ptr)
{
	size_t اڻ_استعمال_ٿيل = 0;
	ثابت uint8_t *checked_ڍير = ڍير_شروعات;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ثابت uint8_t *ڍير_اشارو = (ثابت uint8_t *)&ڍير_شروعات;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	إذا ((ڍير_اشارو > ڍير_شروعات) && (ڍير_اشارو <= (ڍير_شروعات + سائيز)) &&
	    IS_ENABLED(CONFIG_NO_اڻ_استعمال_ٿيل_ڍير_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		أرجع -ENOTSUP;
	}

	إذا (IS_ENABLED(CONFIG_ڍير_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ڍير += 4;
		سائيز -= 4;
	}

	لكل (size_t i = 0; i < سائيز; i++) {
		إذا ((checked_ڍير[i]) == 0xaaU) {
			اڻ_استعمال_ٿيل++;
		} وإلا {
			اكسر;
		}
	}

	*اڻ_استعمال_ٿيل_ptr = اڻ_استعمال_ٿيل;

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_RUNTIME_ڍير_SAFETY
صحيح k_thread_runtime_ڍير_safety_ڀريل_check(ثابت بنية k_thread *thread,
					     size_t *اڻ_استعمال_ٿيل_ptr,
					     k_thread_ڍير_safety_handler_t handler,
					     فراغ *arg)
{
	صحيح    rv;
	size_t اڻ_استعمال_ٿيل_space;

	__ASSERT_NO_MSG(thread != عدم);

	rv = z_ڍير_space_get((ثابت uint8_t *)thread->ڍير_info.شروعات,
			       thread->ڍير_info.سائيز, &اڻ_استعمال_ٿيل_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (اڻ_استعمال_ٿيل_ptr != عدم) {
		*اڻ_استعمال_ٿيل_ptr = اڻ_استعمال_ٿيل_space;
	}

	إذا ((اڻ_استعمال_ٿيل_space < thread->ڍير_info.usage.اڻ_استعمال_ٿيل_حد) &&
	    (handler != عدم)) {
		handler(thread, اڻ_استعمال_ٿيل_space, arg);
	}

	أرجع 0;
}

صحيح k_thread_runtime_ڍير_safety_حد_check(ثابت بنية k_thread *thread,
						  size_t *اڻ_استعمال_ٿيل_ptr,
						  k_thread_ڍير_safety_handler_t handler,
						  فراغ *arg)
{
	صحيح    rv;
	size_t اڻ_استعمال_ٿيل_space;
	size_t حد;
	size_t جاچ_سائيز;

	__ASSERT_NO_MSG(thread != عدم);

	حد = thread->ڍير_info.usage.اڻ_استعمال_ٿيل_حد;
	جاچ_سائيز = حد;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	إذا (IS_ENABLED(CONFIG_ڍير_SENTINEL) && (حد != 0U)) {
		جاچ_سائيز += حجم(uint32_t);
	}

	rv = z_ڍير_space_get((ثابت uint8_t *)thread->ڍير_info.شروعات,
			       جاچ_سائيز, &اڻ_استعمال_ٿيل_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (اڻ_استعمال_ٿيل_ptr != عدم) {
		*اڻ_استعمال_ٿيل_ptr = اڻ_استعمال_ٿيل_space;
	}

	إذا ((اڻ_استعمال_ٿيل_space < حد) && (handler != عدم)) {
		handler(thread, اڻ_استعمال_ٿيل_space, arg);
	}

	أرجع 0;
}
#جيڪڏهن_پورو /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

صحيح z_impl_k_thread_ڍير_space_get(ثابت بنية k_thread *thread,
				    size_t *اڻ_استعمال_ٿيل_ptr)
{
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_MEM_MAPPED
	إذا (thread->ڍير_info.mapped.addr == عدم) {
		أرجع -EINVAL;
	}
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع z_ڍير_space_get((ثابت uint8_t *)thread->ڍير_info.شروعات,
				 thread->ڍير_info.سائيز, اڻ_استعمال_ٿيل_ptr);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
صحيح z_vrfy_k_thread_ڍير_space_get(ثابت بنية k_thread *thread,
				    size_t *اڻ_استعمال_ٿيل_ptr)
{
	size_t اڻ_استعمال_ٿيل;
	صحيح ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ڍير_space_get(thread, &اڻ_استعمال_ٿيل);
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	ret = k_usermode_منزل_copy(اڻ_استعمال_ٿيل_ptr, &اڻ_استعمال_ٿيل, حجم(size_t));
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	أرجع 0;
}
#ضمن <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
#جيڪڏهن_پورو /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_بچيل_ticks(
						    ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_بچيل_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_expires_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_وصف_ٿيل CONFIG_INSTRUMENT_THREAD_SWITCHING
فراغ z_thread_mark_switched_اندريون(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_شروعات(_current);
#جيڪڏهن_پورو /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#جيڪڏهن_وصف_ٿيل CONFIG_TRACING
	SYS_پورٽ_TRACING_FUNC(k_thread, switched_اندريون);
#جيڪڏهن_پورو /* CONFIG_TRACING */
}

فراغ z_thread_mark_switched_ٻاهريون(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#جيڪڏهن_پورو /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#جيڪڏهن_وصف_ٿيل CONFIG_TRACING
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	إذا (!_current ||
	    (_current->بنياد.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}
#جيڪڏهن_پورو /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_پورٽ_TRACING_FUNC(k_thread, switched_ٻاهريون);
#جيڪڏهن_پورو /* CONFIG_TRACING */
}
#جيڪڏهن_پورو /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

صحيح k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	إذا ((thread == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#وإلا
	*stats = (k_thread_runtime_stats_t) {};
#جيڪڏهن_پورو /* CONFIG_SCHED_THREAD_USAGE */

	أرجع 0;
}

صحيح k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#جيڪڏهن_پورو /* CONFIG_SCHED_THREAD_USAGE_ALL */

	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	بنانشان صحيح انگ_cpus = arch_انگ_cpus();

	لكل (بنانشان صحيح i = 0; i < انگ_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_چڪرن += tmp_stats.execution_چڪرن;
		stats->ڪل_چڪرن     += tmp_stats.ڪل_چڪرن;
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_چڪرن   += tmp_stats.current_چڪرن;
		stats->peak_چڪرن      += tmp_stats.peak_چڪرن;
		stats->اوسط_چڪرن   += tmp_stats.اوسط_چڪرن;
#جيڪڏهن_پورو /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_چڪرن      += tmp_stats.idle_چڪرن;
	}
#جيڪڏهن_پورو /* CONFIG_SCHED_THREAD_USAGE_ALL */

	أرجع 0;
}

صحيح k_thread_runtime_stats_cpu_get(صحيح cpu, k_thread_runtime_stats_t *stats)
{
	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((بنانشان صحيح)cpu >= arch_انگ_cpus())) {
		أرجع -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#وإلا
	ARG_اڻ_استعمال_ٿيل(cpu);
#جيڪڏهن_پورو

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ساكن بنية k_thread *thread_منزل_cleanup;

/** Spinlock for thread abort cleanup. */
ساكن بنية k_spinlock thread_cleanup_تالو;

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_MEM_MAPPED
ساكن فراغ *thread_cleanup_ڍير_addr;
ساكن size_t thread_cleanup_ڍير_sz;

ساكن فراغ unmap_thread_ڍير(فراغ *addr, size_t sz)
{
	إذا (addr != عدم) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_MEM_MAPPED */

فراغ defer_thread_cleanup(بنية k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_منزل_cleanup = thread;

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ڍير_addr = thread->ڍير_info.mapped.addr;
	thread_cleanup_ڍير_sz = thread->ڍير_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ڍير_info.mapped.addr = عدم;
	thread->ڍير_info.mapped.sz = 0;
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_deferred_thread_cleanup(فراغ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_MEM_MAPPED
	unmap_thread_ڍير(thread_cleanup_ڍير_addr, thread_cleanup_ڍير_sz);

	thread_cleanup_ڍير_addr = عدم;
	thread_cleanup_ڍير_sz = 0;
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_thread_cleanup(بنية k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_MEM_MAPPED
	unmap_thread_ڍير(thread->ڍير_info.mapped.addr,
			   thread->ڍير_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ڍير_info.mapped.addr = عدم;
	thread->ڍير_info.mapped.sz = 0;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ k_thread_abort_cleanup(بنية k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_تالو) {
		إذا (thread_منزل_cleanup != عدم) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_منزل_cleanup = عدم;
		}

		إذا (thread == _current) {
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
		} وإلا {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

فراغ k_thread_abort_cleanup_check_reuse(بنية k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_تالو) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		إذا (thread_منزل_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_منزل_cleanup = عدم;
		}
	}
}

#جيڪڏهن_پورو /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

فراغ z_dummy_thread_init(بنية k_thread *dummy_thread)
{
	dummy_thread->بنياد.thread_state = _THREAD_DUMMY;
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب
	dummy_thread->بنياد.cpu_نقاب = -1;
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->بنياد.user_options = K_ESSENTIAL;
#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = عدم;
	dummy_thread->orig_prio = dummy_thread->بنياد.prio;
#جيڪڏهن_پورو /* Z_MUTEX_PI_ENABLED */
#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_INFO
	dummy_thread->ڍير_info.شروعات = 0U;
	dummy_thread->ڍير_info.سائيز = 0U;
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_INFO */
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
#إذا (K_ڍير_MEM_POOL_سائيز > 0)
	k_thread_system_pool_سونپيو(dummy_thread);
#وإلا
	dummy_thread->resource_pool = عدم;
#جيڪڏهن_پورو /* K_HEAP_MEM_POOL_SIZE */

#جيڪڏهن_وصف_ٿيل CONFIG_USE_SWITCH
	dummy_thread->switch_handle = عدم;
#جيڪڏهن_پورو /* CONFIG_USE_SWITCH */

#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICE_في_THREAD
	dummy_thread->بنياد.ٽڪرو_ticks = 0;
#جيڪڏهن_پورو /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_سيٽ(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

فراغ z_impl_k_thread_ترجيح_سيٽ(k_tid_t thread, صحيح prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, عدم);

	bool need_sched = z_thread_prio_سيٽ((بنية k_thread *)thread, prio);

	إذا ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->بنياد.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_ترجيح_سيٽ(k_tid_t thread, صحيح prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_نياپو(_is_valid_prio(prio, عدم),
				    "invalid thread priority %d", prio));
#جيڪڏهن_وصف_نه_ٿيل CONFIG_USERSPACE_THREAD_MAY_RAISE_ترجيح
	K_OOPS(K_SYSCALL_VERIFY_نياپو((int8_t)prio >= thread->بنياد.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->بنياد.prio));
#جيڪڏهن_پورو /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_ترجيح_سيٽ(thread, prio);
}
#ضمن <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	إذا (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_اندريون_isr())) {
		z_thread_suspend_current(thread);
		أرجع;
	}

	k_spinlock_ڪنجي_t ڪنجي = z_sched_spinlock_تالو();

	إذا (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(ڪنجي);
		أرجع;
	}

	z_thread_halt(thread, ڪنجي, false);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ضمن <zephyr/syscalls/k_thread_suspend_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_ڪنجي_t ڪنجي = z_sched_spinlock_تالو();

	/* Do not try to resume a thread that was not suspended */
	إذا (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(ڪنجي);
		أرجع;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(ڪنجي);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ضمن <zephyr/syscalls/k_thread_resume_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_پورٽ_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_ڪنجي_t ڪنجي = z_sched_spinlock_تالو();

	إذا (z_is_thread_sleeping(thread)) {
		(فراغ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(ڪنجي);
	} وإلا {
		z_sched_spinlock_unlock(ڪنجي);
	}
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ضمن <zephyr/syscalls/k_wakeup_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ z_thread_abort(بنية k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_ڪنجي_t ڪنجي = z_sched_spinlock_تالو();

	إذا (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(ڪنجي);
		أرجع;
	}

	z_thread_halt(thread, ڪنجي, true);

	إذا (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#إذا !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
فراغ z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#جيڪڏهن_پورو /* !CONFIG_ARCH_HAS_THREAD_ABORT */

صحيح z_impl_k_thread_join(بنية k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_ڪنجي_t ڪنجي = z_sched_spinlock_تالو();
	صحيح ret;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	إذا (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} وإلا إذا ((thread == _current) ||
		   (thread->بنياد.pended_on == &_current->join_قطار)) {
		ret = -EDEADLK;
	} وإلا {
		__ASSERT(!arch_is_اندريون_isr(), "cannot join in ISR");
		z_sched_جوڙيو_منزل_waitq_locked(_current, &thread->join_قطار);
		z_جوڙيو_thread_timeout(_current, timeout);

		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_مٽا_سٽا_locked(ڪنجي);
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		أرجع ret;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(ڪنجي);
	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
ساكن bool thread_obj_validate(بنية k_thread *thread)
{
	بنية k_object *ko = k_object_ڳوليو(thread);
	صحيح ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	بدل (ret) {
	حالة 0:
		أرجع false;
	حالة -EINVAL:
		أرجع true;
	افتراضي:
#جيڪڏهن_وصف_ٿيل CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#جيڪڏهن_پورو /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_نياپو(ret == 0, "access denied"));
	}
	ڪوڊ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساكن مضمن صحيح z_vrfy_k_thread_join(بنية k_thread *thread,
				       k_timeout_t timeout)
{
	إذا (thread_obj_validate(thread)) {
		أرجع 0;
	}

	أرجع z_impl_k_thread_join(thread, timeout);
}
#ضمن <zephyr/syscalls/k_thread_join_mrsh.c>

ساكن مضمن فراغ z_vrfy_k_thread_abort(k_tid_t thread)
{
	إذا (thread_obj_validate(thread)) {
		أرجع;
	}

	K_OOPS(K_SYSCALL_VERIFY_نياپو(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((بنية k_thread *)thread);
}
#ضمن <zephyr/syscalls/k_thread_abort_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

bool k_can_yield(فراغ)
{
	أرجع !(k_is_pre_kernel() || k_is_اندريون_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

فراغ z_impl_k_yield(فراغ)
{
	__ASSERT(!arch_is_اندريون_isr(), "");

	SYS_پورٽ_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_yield(فراغ)
{
	z_impl_k_yield();
}
#ضمن <zephyr/syscalls/k_yield_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
