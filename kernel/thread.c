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

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_THREAD
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
ساکن ساخت k_obj_core_stats_desc  thread_stats_desc = {
	.raw_سائز = حجم(ساخت k_چکر_stats),
	.query_سائز = حجم(ساخت k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#ورنہ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_THREAD */
#اگر_ختم /* CONFIG_OBJ_CORE_THREAD */


#تعریف_کرو _FOREACH_STATIC_THREAD(thread_ڈیٹا)              \
	STRUCT_SECTION_FOREACH(static_thread_ڈیٹا, thread_ڈیٹا)

bool k_is_اندر_isr(خالی)
{
	واپس arch_is_اندر_isr();
}
EXPORT_علامت(k_is_اندر_isr);

#اگر_تعریف_شدہ CONFIG_THREAD_CUSTOM_ڈیٹا
خالی z_impl_k_thread_custom_ڈیٹا_سیٹ(خالی *قدر)
{
	_current->custom_ڈیٹا = قدر;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_thread_custom_ڈیٹا_سیٹ(خالی *ڈیٹا)
{
	z_impl_k_thread_custom_ڈیٹا_سیٹ(ڈیٹا);
}
#شامل <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی *z_impl_k_thread_custom_ڈیٹا_get(خالی)
{
	واپس _current->custom_ڈیٹا;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی *z_vrfy_k_thread_custom_ڈیٹا_get(خالی)
{
	واپس z_impl_k_thread_custom_ڈیٹا_get();
}
#شامل <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#اگر_ختم /* CONFIG_USERSPACE */
#اگر_ختم /* CONFIG_THREAD_CUSTOM_DATA */

صحیح z_impl_k_is_preempt_thread(خالی)
{
	واپس !arch_is_اندر_isr() && thread_is_preemptible(_current);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_is_preempt_thread(خالی)
{
	واپس z_impl_k_is_preempt_thread();
}
#شامل <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_thread_ترجیح_get(k_tid_t thread)
{
	واپس thread->بنیاد.prio;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_thread_ترجیح_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	واپس z_impl_k_thread_ترجیح_get(thread);
}
#شامل <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_تعریف_شدہ CONFIG_THREAD_NAME
ساکن خالی سیٹ_thread_name(ساخت k_thread *thread, ثابت حرف *str)
{
	اگر (str != تہی) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#اگر_تعریف_شدہ CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_سیٹ(thread, str);
#اگر_ختم /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} ورنہ {
		thread->name[0] = '\0';
	}
}
#اگر_ختم /* CONFIG_THREAD_NAME */

صحیح z_impl_k_thread_name_سیٹ(k_tid_t thread, ثابت حرف *str)
{
#اگر_تعریف_شدہ CONFIG_THREAD_NAME
	اگر (thread == تہی) {
		thread = _current;
	}

	سیٹ_thread_name(thread, str);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, name_سیٹ, thread, 0);

	واپس 0;
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(str);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, name_سیٹ, thread, -ENOSYS);

	واپس -ENOSYS;
#اگر_ختم /* CONFIG_THREAD_NAME */
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_thread_name_سیٹ(k_tid_t thread, ثابت حرف *str)
{
#اگر_تعریف_شدہ CONFIG_THREAD_NAME
	حرف name[CONFIG_THREAD_MAX_NAME_LEN];

	اگر (thread != تہی) {
		اگر (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			واپس -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	اگر (k_usermode_string_copy(name, str, حجم(name)) != 0) {
		واپس -EFAULT;
	}

	واپس z_impl_k_thread_name_سیٹ(thread, name);
#ورنہ
	واپس -ENOSYS;
#اگر_ختم /* CONFIG_THREAD_NAME */
}
#شامل <zephyr/syscalls/k_thread_name_set_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

ثابت حرف *k_thread_name_get(k_tid_t thread)
{
#اگر_تعریف_شدہ CONFIG_THREAD_NAME
	واپس (ثابت حرف *)thread->name;
#ورنہ
	ARG_غیر_مستعمل(thread);
	واپس تہی;
#اگر_ختم /* CONFIG_THREAD_NAME */
}

صحیح z_impl_k_thread_name_copy(k_tid_t thread, حرف *buf, size_t سائز)
{
#اگر_تعریف_شدہ CONFIG_THREAD_NAME
	strncpy(buf, thread->name, سائز);
	واپس 0;
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(buf);
	ARG_غیر_مستعمل(سائز);
	واپس -ENOSYS;
#اگر_ختم /* CONFIG_THREAD_NAME */
}

ساکن size_t copy_بائٹیں(حرف *dest, size_t dest_سائز, ثابت حرف *src, size_t src_سائز)
{
	size_t  بائٹیں_مقصد_copy;

	بائٹیں_مقصد_copy = min(dest_سائز, src_سائز);
	حافظہ_نقل(dest, src, بائٹیں_مقصد_copy);

	واپس بائٹیں_مقصد_copy;
}

ثابت حرف *k_thread_state_str(k_tid_t thread_id, حرف *buf, size_t buf_سائز)
{
	size_t      off = 0;
	uint8_t     ثنائی_ہندسہ_2;
	uint8_t     thread_state = thread_id->بنیاد.thread_state;
#تعریف_کرو SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, حجم(Z_STATE_STR_##s) - 1 }
	ساکن ثابت ساخت {
		ثابت حرف *str;
		uint16_t    ثنائی_ہندسہ_2;
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
#تعریف_منسوخ SS_ENT

	اگر ((buf == تہی) || (buf_سائز == 0)) {
		واپس "";
	}

	buf_سائز--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	برائے (غیرعلامتی صحیح اشاریہ = 0; thread_state != 0; اشاریہ++) {
		ثنائی_ہندسہ_2 = state_string[اشاریہ].ثنائی_ہندسہ_2;
		اگر ((thread_state & ثنائی_ہندسہ_2) == 0) {
			جاری;
		}

		off += copy_بائٹیں(buf + off, buf_سائز - off,
				  state_string[اشاریہ].str,
				  state_string[اشاریہ].len);

		thread_state &= ~ثنائی_ہندسہ_2;

		اگر (thread_state != 0) {
			off += copy_بائٹیں(buf + off, buf_سائز - off, "+", 1);
		}
	}

	buf[off] = '\0';

	واپس (ثابت حرف *)buf;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_thread_name_copy(k_tid_t thread,
					    حرف *buf, size_t سائز)
{
#اگر_تعریف_شدہ CONFIG_THREAD_NAME
	size_t len;
	ساخت k_object *ko = k_object_ڈھونڈو(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	اگر ((ko == تہی) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		واپس -EINVAL;
	}
	اگر (K_SYSCALL_حافظہ_WRITE(buf, سائز) != 0) {
		واپس -EFAULT;
	}
	len = لڑی_لمبائی(thread->name);
	اگر ((len + 1) > سائز) {
		واپس -ENOSPC;
	}

	واپس k_usermode_مقصد_copy((خالی *)buf, thread->name, len + 1);
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(buf);
	ARG_غیر_مستعمل(سائز);
	واپس -ENOSYS;
#اگر_ختم /* CONFIG_THREAD_NAME */
}
#شامل <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_تعریف_شدہ CONFIG_ڈھیر_SENTINEL
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
خالی z_check_ڈھیر_sentinel(خالی)
{
	uint32_t *ڈھیر;

	اگر ((_current->بنیاد.thread_state & _THREAD_DUMMY) != 0) {
		واپس;
	}

	ڈھیر = (uint32_t *)_current->ڈھیر_info.آغاز;
	اگر (*ڈھیر != ڈھیر_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ڈھیر = ڈھیر_SENTINEL;
		z_except_reason(K_ERR_ڈھیر_CHK_FAIL);
	}
}
#اگر_ختم /* CONFIG_STACK_SENTINEL */

#اگر defined(CONFIG_ڈھیر_اشارہ_گر_RANDOM) && (CONFIG_ڈھیر_اشارہ_گر_RANDOM != 0)
صحیح z_ڈھیر_adjust_initialized;

ساکن size_t random_انحراف(size_t ڈھیر_سائز)
{
	size_t random_val;

	اگر (!z_ڈھیر_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, حجم(random_val));
	} ورنہ {
		sys_rand_get((uint8_t *)&random_val, حجم(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ثابت size_t fuzz = random_val % CONFIG_ڈھیر_اشارہ_گر_RANDOM;

	اگر (unlikely(fuzz * 2 > ڈھیر_سائز)) {
		واپس 0;
	}

	واپس fuzz;
}
#اگر defined(CONFIG_ڈھیر_GROWS_اوپر)
	/* This is so rare not bothering for now */
#غلطی "Stack pointer randomization not implemented for upward growing stacks"
#اگر_ختم /* CONFIG_STACK_GROWS_UP */
#اگر_ختم /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
ساخت ڈھیر_geometry {
	size_t obj_سائز;	/* size of the whole stack object */
	حرف *buf_آغاز;	/* start of the usable stack buffer */
	size_t buf_سائز;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ساکن ساخت ڈھیر_geometry compute_ڈھیر_geometry(k_thread_ڈھیر_t *ڈھیر,
						    size_t ڈھیر_سائز)
{
	ساخت ڈھیر_geometry geo;

#اگر_تعریف_شدہ CONFIG_USERSPACE
	اگر (z_ڈھیر_is_user_capable(ڈھیر)) {
		geo.obj_سائز = K_THREAD_ڈھیر_LEN(ڈھیر_سائز);
		geo.buf_آغاز = K_THREAD_ڈھیر_بفر(ڈھیر);
		geo.buf_سائز = geo.obj_سائز - K_THREAD_ڈھیر_RESERVED;
	} ورنہ
#اگر_ختم /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_سائز = K_KERNEL_ڈھیر_LEN(ڈھیر_سائز);
		geo.buf_آغاز = K_KERNEL_ڈھیر_بفر(ڈھیر);
		geo.buf_سائز = geo.obj_سائز - K_KERNEL_ڈھیر_RESERVED;

#اگر defined(ARCH_KERNEL_ڈھیر_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		اگر (K_KERNEL_ڈھیر_RESERVED > geo.obj_سائز) {
			k_panic();
		}
#اگر_ختم
	}

	واپس geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ساکن حرف *map_thread_ڈھیر(ساخت k_thread *new_thread,
			      k_thread_ڈھیر_t *ڈھیر,
			      ساخت ڈھیر_geometry *geo)
{
	حرف *ڈھیر_ptr;

#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	خالی *ڈھیر_mapped = k_mem_map_phys_guard((uintptr_t)ڈھیر, geo->obj_سائز,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ڈھیر_mapped != 0);

#اگر_تعریف_شدہ CONFIG_USERSPACE
	اگر (z_ڈھیر_is_user_capable(ڈھیر)) {
		geo->buf_آغاز = K_THREAD_ڈھیر_بفر(ڈھیر_mapped);
	} ورنہ
#اگر_ختم /* CONFIG_USERSPACE */
	{
		geo->buf_آغاز = K_KERNEL_ڈھیر_بفر(ڈھیر_mapped);
	}

	ڈھیر_ptr = (حرف *)ڈھیر_mapped + geo->obj_سائز;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ڈھیر_info.mapped.addr = ڈھیر_mapped;
	new_thread->ڈھیر_info.mapped.sz = geo->obj_سائز;
#ورنہ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_غیر_مستعمل(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ڈھیر_ptr = (حرف *)ڈھیر + geo->obj_سائز;
#اگر_ختم /* CONFIG_THREAD_STACK_MEM_MAPPED */

	واپس ڈھیر_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ساکن اندرونی خالی fill_init_ڈھیر(حرف *buf_آغاز, size_t buf_سائز)
{
#اگر_تعریف_شدہ CONFIG_INIT_STACKS
	memset(buf_آغاز, 0xaa, buf_سائز);
#ورنہ
	ARG_غیر_مستعمل(buf_آغاز);
	ARG_غیر_مستعمل(buf_سائز);
#اگر_ختم /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ساکن اندرونی خالی سیٹ_ڈھیر_sentinel(حرف *buf_آغاز)
{
#اگر_تعریف_شدہ CONFIG_ڈھیر_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_آغاز) = ڈھیر_SENTINEL;
#ورنہ
	ARG_غیر_مستعمل(buf_آغاز);
#اگر_ختم /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ساکن size_t reserve_ڈھیر_headroom(ساخت k_thread *new_thread,
				     حرف *ڈھیر_ptr, size_t buf_سائز)
{
	size_t delta = 0;

	ARG_غیر_مستعمل(new_thread);
	ARG_غیر_مستعمل(ڈھیر_ptr);
	ARG_غیر_مستعمل(buf_سائز);

#اگر_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ڈھیر_تیاری(new_thread, ڈھیر_ptr);
#اگر_ختم /* CONFIG_THREAD_LOCAL_STORAGE */
#اگر_تعریف_شدہ CONFIG_THREAD_USERSPACE_LOCAL_ڈیٹا
	size_t tls_سائز = حجم(ساخت thread_userspace_local_ڈیٹا);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_سائز;
	new_thread->userspace_local_ڈیٹا =
		(ساخت thread_userspace_local_ڈیٹا *)(ڈھیر_ptr - delta);
#اگر_ختم /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#اگر defined(CONFIG_ڈھیر_اشارہ_گر_RANDOM) && (CONFIG_ڈھیر_اشارہ_گر_RANDOM != 0)
	delta += random_انحراف(buf_سائز);
#اگر_ختم /* CONFIG_STACK_POINTER_RANDOM */

	واپس باری_اوپر(delta, ARCH_ڈھیر_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ساکن اندرونی خالی سیٹ_ڈھیر_info(ساخت k_thread *new_thread, حرف *buf_آغاز,
				  size_t buf_سائز, size_t delta)
{
#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ڈھیر_info.آغاز = (uintptr_t)buf_آغاز;
	new_thread->ڈھیر_info.سائز = buf_سائز;
	new_thread->ڈھیر_info.delta = delta;

#اگر_تعریف_شدہ CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY
	new_thread->ڈھیر_info.usage.غیر_مستعمل_حد =
		(CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY_DEFAULT_غیر_مستعمل_حد_PCT *
		 buf_سائز) / 100;
#اگر_ختم
#ورنہ
	ARG_غیر_مستعمل(new_thread);
	ARG_غیر_مستعمل(buf_آغاز);
	ARG_غیر_مستعمل(buf_سائز);
	ARG_غیر_مستعمل(delta);
#اگر_ختم /* CONFIG_THREAD_STACK_INFO */
}

ساکن حرف *تیاری_thread_ڈھیر(ساخت k_thread *new_thread,
				k_thread_ڈھیر_t *ڈھیر, size_t ڈھیر_سائز)
{
	ساخت ڈھیر_geometry geo = compute_ڈھیر_geometry(ڈھیر, ڈھیر_سائز);
	حرف *ڈھیر_ptr = map_thread_ڈھیر(new_thread, ڈھیر, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ڈھیر, new_thread, geo.obj_سائز, (خالی *)geo.buf_آغاز,
		geo.buf_سائز, (خالی *)ڈھیر_ptr);

	fill_init_ڈھیر(geo.buf_آغاز, geo.buf_سائز);
	سیٹ_ڈھیر_sentinel(geo.buf_آغاز);
	delta = reserve_ڈھیر_headroom(new_thread, ڈھیر_ptr, geo.buf_سائز);
	سیٹ_ڈھیر_info(new_thread, geo.buf_آغاز, geo.buf_سائز, delta);

	واپس ڈھیر_ptr - delta;
}

#اگر_تعریف_شدہ CONFIG_HW_SHADOW_ڈھیر
ساکن خالی تیاری_shadow_ڈھیر(ساخت k_thread *new_thread,
			 k_thread_ڈھیر_t *ڈھیر)
{
	صحیح ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ڈھیر_مقصد_hw_shadow_ڈھیر, stk_مقصد_hw_shstk) {
		اگر (stk_مقصد_hw_shstk->ڈھیر == ڈھیر) {
			ret = k_thread_hw_shadow_ڈھیر_attach(new_thread,
							      stk_مقصد_hw_shstk->shstk_addr,
							      stk_مقصد_hw_shstk->سائز);
			اگر (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_مقصد_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			توڑو;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	اگر (ret != -ENOENT) {
		واپس;
	}

	STRUCT_SECTION_FOREACH(ڈھیر_مقصد_hw_shadow_ڈھیر_arr, stk_مقصد_hw_shstk) {
		اگر ((uintptr_t)ڈھیر >= stk_مقصد_hw_shstk->ڈھیر_addr &&
		    (uintptr_t)ڈھیر < stk_مقصد_hw_shstk->ڈھیر_addr +
		    stk_مقصد_hw_shstk->ڈھیر_سائز * stk_مقصد_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ڈھیر_انحراف = (uintptr_t)ڈھیر - stk_مقصد_hw_shstk->ڈھیر_addr;
			uintptr_t ڈھیر_اشاریہ = ڈھیر_انحراف / stk_مقصد_hw_shstk->ڈھیر_سائز;
			uintptr_t addr;

			اگر (ڈھیر_اشاریہ >= stk_مقصد_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ڈھیر);
				k_panic();
			}

			addr = stk_مقصد_hw_shstk->shstk_addr +
				stk_مقصد_hw_shstk->shstk_سائز * ڈھیر_اشاریہ;
			ret = k_thread_hw_shadow_ڈھیر_attach(new_thread,
							      (arch_thread_hw_shadow_ڈھیر_t *)addr,
							      stk_مقصد_hw_shstk->shstk_سائز);
			اگر (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_مقصد_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			توڑو;
		}
	}

	اگر (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ڈھیر);
		k_panic();
	}
}
#ورنہ
/* No-op stub used when hardware shadow stacks are not configured. */
ساکن اندرونی خالی تیاری_shadow_ڈھیر(ساخت k_thread *new_thread,
				      k_thread_ڈھیر_t *ڈھیر)
{
	ARG_غیر_مستعمل(new_thread);
	ARG_غیر_مستعمل(ڈھیر);
}
#اگر_ختم

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
ساکن اندرونی خالی thread_abort_cleanup_check_reuse(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ساکن اندرونی خالی init_thread_obj_core(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(thread), &thread->بنیاد.usage,
				  حجم(thread->بنیاد.usage));
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_THREAD */
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ساکن اندرونی خالی init_thread_userspace(ساخت k_thread *thread,
					 k_thread_ڈھیر_t *ڈھیر,
					 uint32_t options)
{
#اگر_تعریف_شدہ CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ڈھیر_is_user_capable(ڈھیر),
		 "user thread %p with kernel-only stack %p", thread, ڈھیر);
	k_object_init(thread);
	k_object_init(ڈھیر);
	thread->ڈھیر_obj = ڈھیر;
	thread->syscall_چوکھٹا = تہی;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(ڈھیر);
	ARG_غیر_مستعمل(options);
#اگر_ختم /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ساکن اندرونی خالی assert_thread_coherence(ساخت k_thread *thread,
					   k_thread_ڈھیر_t *ڈھیر)
{
#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#اگر_غیر_تعریف_شدہ CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ڈھیر));
#اگر_ختم /* CONFIG_DYNAMIC_THREAD */
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(ڈھیر);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ساکن اندرونی خالی assert_switch_handle(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != تہی,
		 "arch layer failed to initialize switch_handle");
#اگر_ختم /* CONFIG_USE_SWITCH */
	ARG_غیر_مستعمل(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ساکن اندرونی خالی init_thread_custom_ڈیٹا(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_THREAD_CUSTOM_ڈیٹا
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_ڈیٹا = تہی;
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ساکن اندرونی خالی جوڑو_thread_مقصد_monitor(ساخت k_thread *thread,
					 k_thread_اندراج_t اندراج_2,
					 خالی *p1, خالی *p2, خالی *p3)
{
#اگر_تعریف_شدہ CONFIG_THREAD_MONITOR
	thread->اندراج_2.p_اندراج = اندراج_2;
	thread->اندراج_2.parameter1 = p1;
	thread->اندراج_2.parameter2 = p2;
	thread->اندراج_2.parameter3 = p3;

	k_spinlock_کلید_t کلید = k_spin_تالا(&z_thread_monitor_تالا);

	thread->اگلا_thread = _kernel.دھاگے;
	_kernel.دھاگے = thread;
	k_spin_unlock(&z_thread_monitor_تالا, کلید);
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(اندراج_2);
	ARG_غیر_مستعمل(p1);
	ARG_غیر_مستعمل(p2);
	ARG_غیر_مستعمل(p3);
#اگر_ختم /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ساکن اندرونی خالی init_thread_name(ساخت k_thread *thread, ثابت حرف *name)
{
#اگر_تعریف_شدہ CONFIG_THREAD_NAME
	سیٹ_thread_name(thread, name);
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(name);
#اگر_ختم /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ساکن اندرونی خالی init_thread_cpu_نقاب(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
	اگر (IS_ENABLED(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)) {
		thread->بنیاد.cpu_نقاب = 1; /* must specify only one cpu */
	} ورنہ {
		thread->بنیاد.cpu_نقاب = -1; /* allow all cpus */
	}
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ساکن اندرونی خالی init_thread_userspace_perms(ساخت k_thread *thread,
					       uint32_t options)
{
#اگر_تعریف_شدہ CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	اگر ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(options);
#اگر_ختم /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ساکن اندرونی خالی init_thread_deadline(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SCHED_DEADLINE
	thread->بنیاد.prio_deadline = 0;
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ساکن اندرونی خالی init_thread_halt_قطار(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SMP
	z_waitq_init(&thread->halt_قطار);
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ساکن اندرونی خالی init_thread_usage(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE
	thread->بنیاد.usage = (ساخت k_چکر_stats) {};
	thread->بنیاد.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ساکن اندرونی خالی assert_thread_not_reused(ساخت k_thread *new_thread)
{
#اگر_تعریف_شدہ CONFIG_THREAD_MONITOR
#اگر_تعریف_شدہ CONFIG_ASSERT
	k_spinlock_کلید_t کلید = k_spin_تالا(&z_thread_monitor_تالا);
	bool reused = false;

	/* Check if the thread is already in the list */
	برائے (ساخت k_thread *t = _kernel.دھاگے; t; t = t->اگلا_thread) {
		اگر (t == new_thread) {
			reused = true;
			توڑو;
		}
	}

	k_spin_unlock(&z_thread_monitor_تالا, کلید);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#اگر_ختم /* CONFIG_ASSERT */
#اگر_ختم /* CONFIG_THREAD_MONITOR */
	ARG_غیر_مستعمل(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
حرف *z_تیاری_new_thread(ساخت k_thread *new_thread,
			 k_thread_ڈھیر_t *ڈھیر, size_t ڈھیر_سائز,
			 k_thread_اندراج_t اندراج_2,
			 خالی *p1, خالی *p2, خالی *p3,
			 صحیح prio, uint32_t options, ثابت حرف *name)
{
	حرف *ڈھیر_ptr;

	Z_ASSERT_VALID_PRIO(prio, اندراج_2);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ڈھیر, options);

	z_waitq_init(&new_thread->join_قطار);

	/* Initialize various struct k_thread members */
	z_init_thread_بنیاد(&new_thread->بنیاد, prio, _THREAD_SLEEPING, options);
	ڈھیر_ptr = تیاری_thread_ڈھیر(new_thread, ڈھیر, ڈھیر_سائز);

#اگر Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = تہی;
	new_thread->orig_prio = new_thread->بنیاد.prio;
#اگر_ختم /* Z_MUTEX_PI_ENABLED */

	تیاری_shadow_ڈھیر(new_thread, ڈھیر);
	assert_thread_coherence(new_thread, ڈھیر);

	arch_new_thread(new_thread, ڈھیر, ڈھیر_ptr, اندراج_2, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_ڈیٹا = تہی;

	assert_switch_handle(new_thread);
	init_thread_custom_ڈیٹا(new_thread);
	جوڑو_thread_مقصد_monitor(new_thread, اندراج_2, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_نقاب(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	اگر (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_تبادلہ_مقصد_MAIN) &&
	    (_current == تہی)) {
		new_thread->resource_pool = تہی;
		واپس ڈھیر_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_قطار(new_thread);
	init_thread_usage(new_thread);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	واپس ڈھیر_ptr;
}

#اگر_تعریف_شدہ CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY
صحیح z_impl_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_pct_سیٹ(ساخت k_thread *thread,
							   uint32_t pct)
{
	size_t غیر_مستعمل_حد;

	اگر (pct > 99) {
		واپس -EINVAL;   /* 100% unused stack and up is invalid */
	}

	غیر_مستعمل_حد = (thread->ڈھیر_info.سائز * pct) / 100;

	thread->ڈھیر_info.usage.غیر_مستعمل_حد = غیر_مستعمل_حد;

	واپس 0;
}

صحیح z_impl_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_سیٹ(ساخت k_thread *thread,
						       size_t حد)
{
	اگر (حد > thread->ڈھیر_info.سائز) {
		واپس -EINVAL;
	}

	thread->ڈھیر_info.usage.غیر_مستعمل_حد = حد;

	واپس 0;
}

size_t z_impl_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_get(ساخت k_thread *thread)
{
	واپس thread->ڈھیر_info.usage.غیر_مستعمل_حد;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_pct_سیٹ(ساخت k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	واپس z_impl_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_pct_سیٹ(thread, pct);
}
#شامل <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

صحیح z_vrfy_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_سیٹ(ساخت k_thread *thread,
						       size_t حد)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	واپس z_impl_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_سیٹ(thread, حد);
}
#شامل <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_get(ساخت k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	واپس z_impl_k_thread_runtime_ڈھیر_غیر_مستعمل_حد_get(thread);
}
#شامل <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
#اگر_ختم /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(ساخت k_thread *new_thread,
			      k_thread_ڈھیر_t *ڈھیر,
			      size_t ڈھیر_سائز, k_thread_اندراج_t اندراج_2,
			      خالی *p1, خالی *p2, خالی *p3,
			      صحیح prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_اندر_isr(), "Threads may not be created in ISRs");

	z_تیاری_new_thread(new_thread, ڈھیر, ڈھیر_سائز, اندراج_2, p1, p2, p3,
			  prio, options, تہی);

	thread_جدول_وقت_new(new_thread, delay);

	واپس new_thread;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
bool z_ڈھیر_is_user_capable(k_thread_ڈھیر_t *ڈھیر)
{
	واپس k_object_ڈھونڈو(ڈھیر) != تہی;
}

k_tid_t z_vrfy_k_thread_create(ساخت k_thread *new_thread,
			       k_thread_ڈھیر_t *ڈھیر,
			       size_t ڈھیر_سائز, k_thread_اندراج_t اندراج_2,
			       خالی *p1, خالی *p2, خالی *p3,
			       صحیح prio, uint32_t options, k_timeout_t delay)
{
	size_t کل_سائز, ڈھیر_obj_سائز;
	ساخت k_object *ڈھیر_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ڈھیر_object = k_object_ڈھونڈو(ڈھیر);
	K_OOPS(K_SYSCALL_VERIFY_پیغام(k_object_validation_check(ڈھیر_object, ڈھیر,
						K_OBJ_THREAD_ڈھیر_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_پیغام(!سائز_جوڑو_حد_سے_تجاوز(K_THREAD_ڈھیر_RESERVED,
						       ڈھیر_سائز, &کل_سائز),
				    "stack size overflow (%zu+%zu)",
				    ڈھیر_سائز,
				    K_THREAD_ڈھیر_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#اگر_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
	ڈھیر_obj_سائز = ڈھیر_object->ڈیٹا.ڈھیر_ڈیٹا->سائز;
#ورنہ
	ڈھیر_obj_سائز = ڈھیر_object->ڈیٹا.ڈھیر_سائز;
#اگر_ختم /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_پیغام(کل_سائز <= ڈھیر_obj_سائز,
				    "stack size %zu is too big, max is %zu",
				    کل_سائز, ڈھیر_obj_سائز));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, تہی)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_زیریں_or_equal(prio,
							_current->بنیاد.prio)));

	z_تیاری_new_thread(new_thread, ڈھیر, ڈھیر_سائز,
			   اندراج_2, p1, p2, p3, prio, options, تہی);

	thread_جدول_وقت_new(new_thread, delay);

	واپس new_thread;
}
#شامل <zephyr/syscalls/k_thread_create_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_init_thread_بنیاد(ساخت thread_بنیاد *thread_بنیاد_2, صحیح ترجیح,
		       uint32_t initial_state, غیرعلامتی صحیح options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_بنیاد_2->pended_on = تہی;
	thread_بنیاد_2->user_options = (uint16_t)options;
	thread_بنیاد_2->thread_state = (uint8_t)initial_state;

	thread_بنیاد_2->prio = ترجیح;

	thread_بنیاد_2->sched_locked = 0U;

#اگر_تعریف_شدہ CONFIG_SMP
	thread_بنیاد_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_بنیاد_2->cpu = 0;
#اگر_ختم /* CONFIG_SMP */

#اگر_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	thread_بنیاد_2->قاش_ticks = 0;
	thread_بنیاد_2->قاش_expired = تہی;
#اگر_ختم /* CONFIG_TIMESLICE_PER_THREAD */

#اگر_تعریف_شدہ CONFIG_SPIN_VALIDATE
	thread_بنیاد_2->تبادلہ_ڈیٹا = تہی;
	/* otherwise swap_data does not need to be initialized */
#اگر_ختم

	z_init_thread_timeout(thread_بنیاد_2);
}

FUNC_NORETURN خالی k_thread_user_منوال_enter(k_thread_اندراج_t اندراج_2,
					    خالی *p1, خالی *p2, خالی *p3)
{
	SYS_پورٹ_TRACING_FUNC(k_thread, user_منوال_enter);

	_current->بنیاد.user_options |= K_USER;
	z_thread_essential_clear(_current);
#اگر_تعریف_شدہ CONFIG_THREAD_MONITOR
	_current->اندراج_2.p_اندراج = اندراج_2;
	_current->اندراج_2.parameter1 = p1;
	_current->اندراج_2.parameter2 = p2;
	_current->اندراج_2.parameter3 = p3;
#اگر_ختم /* CONFIG_THREAD_MONITOR */
#اگر_تعریف_شدہ CONFIG_USERSPACE
	__ASSERT(z_ڈھیر_is_user_capable(_current->ڈھیر_obj),
		 "dropping to user mode with kernel-only stack object");
#اگر_تعریف_شدہ CONFIG_THREAD_USERSPACE_LOCAL_ڈیٹا
	memset(_current->userspace_local_ڈیٹا, 0,
	       حجم(ساخت thread_userspace_local_ڈیٹا));
#اگر_ختم /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#اگر_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ڈھیر_تیاری(_current,
			     (حرف *)(_current->ڈھیر_info.آغاز +
				      _current->ڈھیر_info.سائز));
#اگر_ختم /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_منوال_enter(اندراج_2, p1, p2, p3);
#ورنہ
	/* XXX In this case we do not reset the stack */
	z_thread_اندراج(اندراج_2, p1, p2, p3);
#اگر_ختم /* CONFIG_USERSPACE */
}

#اگر defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ڈھیر_INFO)
#اگر_تعریف_شدہ CONFIG_ڈھیر_GROWS_اوپر
#غلطی "Unsupported configuration for stack analysis"
#اگر_ختم /* CONFIG_STACK_GROWS_UP */

صحیح z_ڈھیر_space_get(ثابت uint8_t *ڈھیر_آغاز, size_t سائز, size_t *غیر_مستعمل_ptr)
{
	size_t غیر_مستعمل = 0;
	ثابت uint8_t *checked_ڈھیر = ڈھیر_آغاز;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ثابت uint8_t *ڈھیر_اشارہ_گر = (ثابت uint8_t *)&ڈھیر_آغاز;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	اگر ((ڈھیر_اشارہ_گر > ڈھیر_آغاز) && (ڈھیر_اشارہ_گر <= (ڈھیر_آغاز + سائز)) &&
	    IS_ENABLED(CONFIG_NO_غیر_مستعمل_ڈھیر_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		واپس -ENOTSUP;
	}

	اگر (IS_ENABLED(CONFIG_ڈھیر_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ڈھیر += 4;
		سائز -= 4;
	}

	برائے (size_t i = 0; i < سائز; i++) {
		اگر ((checked_ڈھیر[i]) == 0xaaU) {
			غیر_مستعمل++;
		} ورنہ {
			توڑو;
		}
	}

	*غیر_مستعمل_ptr = غیر_مستعمل;

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY
صحیح k_thread_runtime_ڈھیر_safety_بھرا_check(ثابت ساخت k_thread *thread,
					     size_t *غیر_مستعمل_ptr,
					     k_thread_ڈھیر_safety_handler_t handler,
					     خالی *arg)
{
	صحیح    rv;
	size_t غیر_مستعمل_space;

	__ASSERT_NO_MSG(thread != تہی);

	rv = z_ڈھیر_space_get((ثابت uint8_t *)thread->ڈھیر_info.آغاز,
			       thread->ڈھیر_info.سائز, &غیر_مستعمل_space);

	اگر (rv != 0) {
		واپس rv;
	}

	اگر (غیر_مستعمل_ptr != تہی) {
		*غیر_مستعمل_ptr = غیر_مستعمل_space;
	}

	اگر ((غیر_مستعمل_space < thread->ڈھیر_info.usage.غیر_مستعمل_حد) &&
	    (handler != تہی)) {
		handler(thread, غیر_مستعمل_space, arg);
	}

	واپس 0;
}

صحیح k_thread_runtime_ڈھیر_safety_حد_check(ثابت ساخت k_thread *thread,
						  size_t *غیر_مستعمل_ptr,
						  k_thread_ڈھیر_safety_handler_t handler,
						  خالی *arg)
{
	صحیح    rv;
	size_t غیر_مستعمل_space;
	size_t حد;
	size_t جائزہ_سائز;

	__ASSERT_NO_MSG(thread != تہی);

	حد = thread->ڈھیر_info.usage.غیر_مستعمل_حد;
	جائزہ_سائز = حد;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	اگر (IS_ENABLED(CONFIG_ڈھیر_SENTINEL) && (حد != 0U)) {
		جائزہ_سائز += حجم(uint32_t);
	}

	rv = z_ڈھیر_space_get((ثابت uint8_t *)thread->ڈھیر_info.آغاز,
			       جائزہ_سائز, &غیر_مستعمل_space);

	اگر (rv != 0) {
		واپس rv;
	}

	اگر (غیر_مستعمل_ptr != تہی) {
		*غیر_مستعمل_ptr = غیر_مستعمل_space;
	}

	اگر ((غیر_مستعمل_space < حد) && (handler != تہی)) {
		handler(thread, غیر_مستعمل_space, arg);
	}

	واپس 0;
}
#اگر_ختم /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

صحیح z_impl_k_thread_ڈھیر_space_get(ثابت ساخت k_thread *thread,
				    size_t *غیر_مستعمل_ptr)
{
#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	اگر (thread->ڈھیر_info.mapped.addr == تہی) {
		واپس -EINVAL;
	}
#اگر_ختم /* CONFIG_THREAD_STACK_MEM_MAPPED */

	واپس z_ڈھیر_space_get((ثابت uint8_t *)thread->ڈھیر_info.آغاز,
				 thread->ڈھیر_info.سائز, غیر_مستعمل_ptr);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_thread_ڈھیر_space_get(ثابت ساخت k_thread *thread,
				    size_t *غیر_مستعمل_ptr)
{
	size_t غیر_مستعمل;
	صحیح ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ڈھیر_space_get(thread, &غیر_مستعمل);
	CHECKIF(ret != 0) {
		واپس ret;
	}

	ret = k_usermode_مقصد_copy(غیر_مستعمل_ptr, &غیر_مستعمل, حجم(size_t));
	CHECKIF(ret != 0) {
		واپس ret;
	}

	واپس 0;
}
#شامل <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
#اگر_ختم /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی k_ticks_t z_vrfy_k_thread_timeout_بقایا_ticks(
						    ثابت ساخت k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	واپس z_impl_k_thread_timeout_بقایا_ticks(thread);
}
#شامل <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ساکن اندرونی k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ثابت ساخت k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	واپس z_impl_k_thread_timeout_expires_ticks(thread);
}
#شامل <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_تعریف_شدہ CONFIG_INSTRUMENT_THREAD_SWITCHING
خالی z_thread_mark_switched_اندر(خالی)
{
#اگر defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_آغاز(_current);
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#اگر_تعریف_شدہ CONFIG_TRACING
	SYS_پورٹ_TRACING_FUNC(k_thread, switched_اندر);
#اگر_ختم /* CONFIG_TRACING */
}

خالی z_thread_mark_switched_برآمد(خالی)
{
#اگر defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#اگر_ختم /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#اگر_تعریف_شدہ CONFIG_TRACING
#اگر_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	اگر (!_current ||
	    (_current->بنیاد.thread_state & _THREAD_DUMMY) != 0) {
		واپس;
	}
#اگر_ختم /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_پورٹ_TRACING_FUNC(k_thread, switched_برآمد);
#اگر_ختم /* CONFIG_TRACING */
}
#اگر_ختم /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

صحیح k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	اگر ((thread == تہی) || (stats == تہی)) {
		واپس -EINVAL;
	}

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#ورنہ
	*stats = (k_thread_runtime_stats_t) {};
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE */

	واپس 0;
}

صحیح k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ALL */

	اگر (stats == تہی) {
		واپس -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	غیرعلامتی صحیح عدد_cpus = arch_عدد_cpus();

	برائے (غیرعلامتی صحیح i = 0; i < عدد_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_ادوار += tmp_stats.execution_ادوار;
		stats->کل_ادوار     += tmp_stats.کل_ادوار;
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_ادوار   += tmp_stats.current_ادوار;
		stats->peak_ادوار      += tmp_stats.peak_ادوار;
		stats->اوسط_قدر_ادوار   += tmp_stats.اوسط_قدر_ادوار;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_ادوار      += tmp_stats.idle_ادوار;
	}
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ALL */

	واپس 0;
}

صحیح k_thread_runtime_stats_cpu_get(صحیح cpu, k_thread_runtime_stats_t *stats)
{
	اگر (stats == تہی) {
		واپس -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((غیرعلامتی صحیح)cpu >= arch_عدد_cpus())) {
		واپس -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#ورنہ
	ARG_غیر_مستعمل(cpu);
#اگر_ختم

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ساکن ساخت k_thread *thread_مقصد_cleanup;

/** Spinlock for thread abort cleanup. */
ساکن ساخت k_spinlock thread_cleanup_تالا;

#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
ساکن خالی *thread_cleanup_ڈھیر_addr;
ساکن size_t thread_cleanup_ڈھیر_sz;

ساکن خالی unmap_thread_ڈھیر(خالی *addr, size_t sz)
{
	اگر (addr != تہی) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#اگر_ختم /* CONFIG_THREAD_STACK_MEM_MAPPED */

خالی defer_thread_cleanup(ساخت k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_مقصد_cleanup = thread;

#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ڈھیر_addr = thread->ڈھیر_info.mapped.addr;
	thread_cleanup_ڈھیر_sz = thread->ڈھیر_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ڈھیر_info.mapped.addr = تہی;
	thread->ڈھیر_info.mapped.sz = 0;
#اگر_ختم /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

خالی do_deferred_thread_cleanup(خالی)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	unmap_thread_ڈھیر(thread_cleanup_ڈھیر_addr, thread_cleanup_ڈھیر_sz);

	thread_cleanup_ڈھیر_addr = تہی;
	thread_cleanup_ڈھیر_sz = 0;
#اگر_ختم /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

خالی do_thread_cleanup(ساخت k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	unmap_thread_ڈھیر(thread->ڈھیر_info.mapped.addr,
			   thread->ڈھیر_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ڈھیر_info.mapped.addr = تہی;
	thread->ڈھیر_info.mapped.sz = 0;
#ورنہ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_غیر_مستعمل(thread);
#اگر_ختم /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

خالی k_thread_abort_cleanup(ساخت k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_تالا) {
		اگر (thread_مقصد_cleanup != تہی) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_مقصد_cleanup = تہی;
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
		} ورنہ {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

خالی k_thread_abort_cleanup_check_reuse(ساخت k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_تالا) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		اگر (thread_مقصد_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_مقصد_cleanup = تہی;
		}
	}
}

#اگر_ختم /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

خالی z_dummy_thread_init(ساخت k_thread *dummy_thread)
{
	dummy_thread->بنیاد.thread_state = _THREAD_DUMMY;
#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
	dummy_thread->بنیاد.cpu_نقاب = -1;
#اگر_ختم /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->بنیاد.user_options = K_ESSENTIAL;
#اگر Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = تہی;
	dummy_thread->orig_prio = dummy_thread->بنیاد.prio;
#اگر_ختم /* Z_MUTEX_PI_ENABLED */
#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_INFO
	dummy_thread->ڈھیر_info.آغاز = 0U;
	dummy_thread->ڈھیر_info.سائز = 0U;
#اگر_ختم /* CONFIG_THREAD_STACK_INFO */
#اگر_تعریف_شدہ CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#اگر_ختم /* CONFIG_USERSPACE */
#اگر (K_ڈھیر_MEM_POOL_سائز > 0)
	k_thread_system_pool_تفویض(dummy_thread);
#ورنہ
	dummy_thread->resource_pool = تہی;
#اگر_ختم /* K_HEAP_MEM_POOL_SIZE */

#اگر_تعریف_شدہ CONFIG_USE_SWITCH
	dummy_thread->switch_handle = تہی;
#اگر_ختم /* CONFIG_USE_SWITCH */

#اگر_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	dummy_thread->بنیاد.قاش_ticks = 0;
#اگر_ختم /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_سیٹ(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

خالی z_impl_k_thread_ترجیح_سیٹ(k_tid_t thread, صحیح prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, تہی);

	bool need_sched = z_thread_prio_سیٹ((ساخت k_thread *)thread, prio);

	اگر ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->بنیاد.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_thread_ترجیح_سیٹ(k_tid_t thread, صحیح prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_پیغام(_is_valid_prio(prio, تہی),
				    "invalid thread priority %d", prio));
#اگر_غیر_تعریف_شدہ CONFIG_USERSPACE_THREAD_MAY_RAISE_ترجیح
	K_OOPS(K_SYSCALL_VERIFY_پیغام((int8_t)prio >= thread->بنیاد.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->بنیاد.prio));
#اگر_ختم /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_ترجیح_سیٹ(thread, prio);
}
#شامل <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	اگر (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_اندر_isr())) {
		z_thread_suspend_current(thread);
		واپس;
	}

	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	اگر (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(کلید);
		واپس;
	}

	z_thread_halt(thread, کلید, false);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#شامل <zephyr/syscalls/k_thread_suspend_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	/* Do not try to resume a thread that was not suspended */
	اگر (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(کلید);
		واپس;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#شامل <zephyr/syscalls/k_thread_resume_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_impl_k_wakeup(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	اگر (z_is_thread_sleeping(thread)) {
		(خالی)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(کلید);
	} ورنہ {
		z_sched_spinlock_unlock(کلید);
	}
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#شامل <zephyr/syscalls/k_wakeup_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_thread_abort(ساخت k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	اگر (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(کلید);
		واپس;
	}

	z_thread_halt(thread, کلید, true);

	اگر (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#اگر !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
خالی z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#اگر_ختم /* !CONFIG_ARCH_HAS_THREAD_ABORT */

صحیح z_impl_k_thread_join(ساخت k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();
	صحیح ret;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	اگر (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} ورنہ اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} ورنہ اگر ((thread == _current) ||
		   (thread->بنیاد.pended_on == &_current->join_قطار)) {
		ret = -EDEADLK;
	} ورنہ {
		__ASSERT(!arch_is_اندر_isr(), "cannot join in ISR");
		z_sched_جوڑو_مقصد_waitq_locked(_current, &thread->join_قطار);
		z_جوڑو_thread_timeout(_current, timeout);

		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_تبادلہ_locked(کلید);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		واپس ret;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(کلید);
	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
ساکن bool thread_obj_validate(ساخت k_thread *thread)
{
	ساخت k_object *ko = k_object_ڈھونڈو(thread);
	صحیح ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	انتخاب (ret) {
	حالت 0:
		واپس false;
	حالت -EINVAL:
		واپس true;
	طےشدہ:
#اگر_تعریف_شدہ CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#اگر_ختم /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_پیغام(ret == 0, "access denied"));
	}
	رمز_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساکن اندرونی صحیح z_vrfy_k_thread_join(ساخت k_thread *thread,
				       k_timeout_t timeout)
{
	اگر (thread_obj_validate(thread)) {
		واپس 0;
	}

	واپس z_impl_k_thread_join(thread, timeout);
}
#شامل <zephyr/syscalls/k_thread_join_mrsh.c>

ساکن اندرونی خالی z_vrfy_k_thread_abort(k_tid_t thread)
{
	اگر (thread_obj_validate(thread)) {
		واپس;
	}

	K_OOPS(K_SYSCALL_VERIFY_پیغام(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((ساخت k_thread *)thread);
}
#شامل <zephyr/syscalls/k_thread_abort_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

bool k_can_yield(خالی)
{
	واپس !(k_is_pre_kernel() || k_is_اندر_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

خالی z_impl_k_yield(خالی)
{
	__ASSERT(!arch_is_اندر_isr(), "");

	SYS_پورٹ_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_yield(خالی)
{
	z_impl_k_yield();
}
#شامل <zephyr/syscalls/k_yield_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
