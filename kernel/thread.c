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

#جے_تعریف_شدہ CONFIG_OBJ_CORE_THREAD
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
ساكن بنية k_obj_core_stats_desc  thread_stats_desc = {
	.raw_ناپ = حجم(بنية k_چکر_stats),
	.query_ناپ = حجم(بنية k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#جے_انت /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#وإلا
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    عدم);
#جے_انت /* CONFIG_OBJ_CORE_STATS_THREAD */
#جے_انت /* CONFIG_OBJ_CORE_THREAD */


#تعریف_کرو _FOREACH_STATIC_THREAD(thread_انکڑے)              \
	STRUCT_SECTION_FOREACH(static_thread_انکڑے, thread_انکڑے)

bool k_is_وچ_isr(فراغ)
{
	أرجع arch_is_وچ_isr();
}
EXPORT_علامت(k_is_وچ_isr);

#جے_تعریف_شدہ CONFIG_THREAD_CUSTOM_انکڑے
فراغ z_impl_k_thread_custom_انکڑے_سیٹ(فراغ *قدر)
{
	_current->custom_انکڑے = قدر;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_custom_انکڑے_سیٹ(فراغ *انکڑے)
{
	z_impl_k_thread_custom_انکڑے_سیٹ(انکڑے);
}
#ضمن <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ *z_impl_k_thread_custom_انکڑے_get(فراغ)
{
	أرجع _current->custom_انکڑے;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_thread_custom_انکڑے_get(فراغ)
{
	أرجع z_impl_k_thread_custom_انکڑے_get();
}
#ضمن <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#جے_انت /* CONFIG_USERSPACE */
#جے_انت /* CONFIG_THREAD_CUSTOM_DATA */

صحيح z_impl_k_is_preempt_thread(فراغ)
{
	أرجع !arch_is_وچ_isr() && thread_is_preemptible(_current);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_is_preempt_thread(فراغ)
{
	أرجع z_impl_k_is_preempt_thread();
}
#ضمن <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_ترجیح_get(k_tid_t thread)
{
	أرجع thread->ادھار.prio;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ترجیح_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_ترجیح_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

#جے_تعریف_شدہ CONFIG_THREAD_NAME
ساكن فراغ سیٹ_thread_name(بنية k_thread *thread, ثابت حرف *str)
{
	إذا (str != عدم) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#جے_تعریف_شدہ CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_سیٹ(thread, str);
#جے_انت /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} وإلا {
		thread->name[0] = '\0';
	}
}
#جے_انت /* CONFIG_THREAD_NAME */

صحيح z_impl_k_thread_name_سیٹ(k_tid_t thread, ثابت حرف *str)
{
#جے_تعریف_شدہ CONFIG_THREAD_NAME
	إذا (thread == عدم) {
		thread = _current;
	}

	سیٹ_thread_name(thread, str);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, name_سیٹ, thread, 0);

	أرجع 0;
#وإلا
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(str);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, name_سیٹ, thread, -ENOSYS);

	أرجع -ENOSYS;
#جے_انت /* CONFIG_THREAD_NAME */
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_سیٹ(k_tid_t thread, ثابت حرف *str)
{
#جے_تعریف_شدہ CONFIG_THREAD_NAME
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

	أرجع z_impl_k_thread_name_سیٹ(thread, name);
#وإلا
	أرجع -ENOSYS;
#جے_انت /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_set_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

ثابت حرف *k_thread_name_get(k_tid_t thread)
{
#جے_تعریف_شدہ CONFIG_THREAD_NAME
	أرجع (ثابت حرف *)thread->name;
#وإلا
	ARG_بن_ورتیا(thread);
	أرجع عدم;
#جے_انت /* CONFIG_THREAD_NAME */
}

صحيح z_impl_k_thread_name_copy(k_tid_t thread, حرف *buf, size_t ناپ)
{
#جے_تعریف_شدہ CONFIG_THREAD_NAME
	strncpy(buf, thread->name, ناپ);
	أرجع 0;
#وإلا
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(buf);
	ARG_بن_ورتیا(ناپ);
	أرجع -ENOSYS;
#جے_انت /* CONFIG_THREAD_NAME */
}

ساكن size_t copy_بائٹاں(حرف *dest, size_t dest_ناپ, ثابت حرف *src, size_t src_ناپ)
{
	size_t  بائٹاں_منزل_copy;

	بائٹاں_منزل_copy = min(dest_ناپ, src_ناپ);
	حافظہ_نقل(dest, src, بائٹاں_منزل_copy);

	أرجع بائٹاں_منزل_copy;
}

ثابت حرف *k_thread_state_str(k_tid_t thread_id, حرف *buf, size_t buf_ناپ)
{
	size_t      off = 0;
	uint8_t     ثنائی_ہندسہ_2;
	uint8_t     thread_state = thread_id->ادھار.thread_state;
#تعریف_کرو SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, حجم(Z_STATE_STR_##s) - 1 }
	ساكن ثابت بنية {
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
#تعریف_رد SS_ENT

	إذا ((buf == عدم) || (buf_ناپ == 0)) {
		أرجع "";
	}

	buf_ناپ--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	لكل (بنانشانی صحيح اشاریہ = 0; thread_state != 0; اشاریہ++) {
		ثنائی_ہندسہ_2 = state_string[اشاریہ].ثنائی_ہندسہ_2;
		إذا ((thread_state & ثنائی_ہندسہ_2) == 0) {
			استمر;
		}

		off += copy_بائٹاں(buf + off, buf_ناپ - off,
				  state_string[اشاریہ].str,
				  state_string[اشاریہ].len);

		thread_state &= ~ثنائی_ہندسہ_2;

		إذا (thread_state != 0) {
			off += copy_بائٹاں(buf + off, buf_ناپ - off, "+", 1);
		}
	}

	buf[off] = '\0';

	أرجع (ثابت حرف *)buf;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_copy(k_tid_t thread,
					    حرف *buf, size_t ناپ)
{
#جے_تعریف_شدہ CONFIG_THREAD_NAME
	size_t len;
	بنية k_object *ko = k_object_لبھو(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	إذا ((ko == عدم) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		أرجع -EINVAL;
	}
	إذا (K_SYSCALL_حافظہ_WRITE(buf, ناپ) != 0) {
		أرجع -EFAULT;
	}
	len = لڑی_لمبائی(thread->name);
	إذا ((len + 1) > ناپ) {
		أرجع -ENOSPC;
	}

	أرجع k_usermode_منزل_copy((فراغ *)buf, thread->name, len + 1);
#وإلا
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(buf);
	ARG_بن_ورتیا(ناپ);
	أرجع -ENOSYS;
#جے_انت /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

#جے_تعریف_شدہ CONFIG_ڈھیر_SENTINEL
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
فراغ z_check_ڈھیر_sentinel(فراغ)
{
	uint32_t *ڈھیر;

	إذا ((_current->ادھار.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}

	ڈھیر = (uint32_t *)_current->ڈھیر_info.شروع;
	إذا (*ڈھیر != ڈھیر_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ڈھیر = ڈھیر_SENTINEL;
		z_except_reason(K_ERR_ڈھیر_CHK_FAIL);
	}
}
#جے_انت /* CONFIG_STACK_SENTINEL */

#إذا defined(CONFIG_ڈھیر_اشارہ_گر_RANDOM) && (CONFIG_ڈھیر_اشارہ_گر_RANDOM != 0)
صحيح z_ڈھیر_adjust_initialized;

ساكن size_t random_وتھ(size_t ڈھیر_ناپ)
{
	size_t random_val;

	إذا (!z_ڈھیر_adjust_initialized) {
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
	ثابت size_t fuzz = random_val % CONFIG_ڈھیر_اشارہ_گر_RANDOM;

	إذا (unlikely(fuzz * 2 > ڈھیر_ناپ)) {
		أرجع 0;
	}

	أرجع fuzz;
}
#إذا defined(CONFIG_ڈھیر_GROWS_اتے)
	/* This is so rare not bothering for now */
#غلطی "Stack pointer randomization not implemented for upward growing stacks"
#جے_انت /* CONFIG_STACK_GROWS_UP */
#جے_انت /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
بنية ڈھیر_geometry {
	size_t obj_ناپ;	/* size of the whole stack object */
	حرف *buf_شروع;	/* start of the usable stack buffer */
	size_t buf_ناپ;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ساكن بنية ڈھیر_geometry compute_ڈھیر_geometry(k_thread_ڈھیر_t *ڈھیر,
						    size_t ڈھیر_ناپ)
{
	بنية ڈھیر_geometry geo;

#جے_تعریف_شدہ CONFIG_USERSPACE
	إذا (z_ڈھیر_is_user_capable(ڈھیر)) {
		geo.obj_ناپ = K_THREAD_ڈھیر_LEN(ڈھیر_ناپ);
		geo.buf_شروع = K_THREAD_ڈھیر_بفر(ڈھیر);
		geo.buf_ناپ = geo.obj_ناپ - K_THREAD_ڈھیر_RESERVED;
	} وإلا
#جے_انت /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_ناپ = K_KERNEL_ڈھیر_LEN(ڈھیر_ناپ);
		geo.buf_شروع = K_KERNEL_ڈھیر_بفر(ڈھیر);
		geo.buf_ناپ = geo.obj_ناپ - K_KERNEL_ڈھیر_RESERVED;

#إذا defined(ARCH_KERNEL_ڈھیر_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		إذا (K_KERNEL_ڈھیر_RESERVED > geo.obj_ناپ) {
			k_panic();
		}
#جے_انت
	}

	أرجع geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ساكن حرف *map_thread_ڈھیر(بنية k_thread *new_thread,
			      k_thread_ڈھیر_t *ڈھیر,
			      بنية ڈھیر_geometry *geo)
{
	حرف *ڈھیر_ptr;

#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	فراغ *ڈھیر_mapped = k_mem_map_phys_guard((uintptr_t)ڈھیر, geo->obj_ناپ,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ڈھیر_mapped != 0);

#جے_تعریف_شدہ CONFIG_USERSPACE
	إذا (z_ڈھیر_is_user_capable(ڈھیر)) {
		geo->buf_شروع = K_THREAD_ڈھیر_بفر(ڈھیر_mapped);
	} وإلا
#جے_انت /* CONFIG_USERSPACE */
	{
		geo->buf_شروع = K_KERNEL_ڈھیر_بفر(ڈھیر_mapped);
	}

	ڈھیر_ptr = (حرف *)ڈھیر_mapped + geo->obj_ناپ;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ڈھیر_info.mapped.addr = ڈھیر_mapped;
	new_thread->ڈھیر_info.mapped.sz = geo->obj_ناپ;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_بن_ورتیا(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ڈھیر_ptr = (حرف *)ڈھیر + geo->obj_ناپ;
#جے_انت /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع ڈھیر_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ساكن مضمن فراغ fill_init_ڈھیر(حرف *buf_شروع, size_t buf_ناپ)
{
#جے_تعریف_شدہ CONFIG_INIT_STACKS
	memset(buf_شروع, 0xaa, buf_ناپ);
#وإلا
	ARG_بن_ورتیا(buf_شروع);
	ARG_بن_ورتیا(buf_ناپ);
#جے_انت /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ساكن مضمن فراغ سیٹ_ڈھیر_sentinel(حرف *buf_شروع)
{
#جے_تعریف_شدہ CONFIG_ڈھیر_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_شروع) = ڈھیر_SENTINEL;
#وإلا
	ARG_بن_ورتیا(buf_شروع);
#جے_انت /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ساكن size_t reserve_ڈھیر_headroom(بنية k_thread *new_thread,
				     حرف *ڈھیر_ptr, size_t buf_ناپ)
{
	size_t delta = 0;

	ARG_بن_ورتیا(new_thread);
	ARG_بن_ورتیا(ڈھیر_ptr);
	ARG_بن_ورتیا(buf_ناپ);

#جے_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ڈھیر_تیاری(new_thread, ڈھیر_ptr);
#جے_انت /* CONFIG_THREAD_LOCAL_STORAGE */
#جے_تعریف_شدہ CONFIG_THREAD_USERSPACE_LOCAL_انکڑے
	size_t tls_ناپ = حجم(بنية thread_userspace_local_انکڑے);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_ناپ;
	new_thread->userspace_local_انکڑے =
		(بنية thread_userspace_local_انکڑے *)(ڈھیر_ptr - delta);
#جے_انت /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#إذا defined(CONFIG_ڈھیر_اشارہ_گر_RANDOM) && (CONFIG_ڈھیر_اشارہ_گر_RANDOM != 0)
	delta += random_وتھ(buf_ناپ);
#جے_انت /* CONFIG_STACK_POINTER_RANDOM */

	أرجع گیڑا_اتے(delta, ARCH_ڈھیر_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ساكن مضمن فراغ سیٹ_ڈھیر_info(بنية k_thread *new_thread, حرف *buf_شروع,
				  size_t buf_ناپ, size_t delta)
{
#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ڈھیر_info.شروع = (uintptr_t)buf_شروع;
	new_thread->ڈھیر_info.ناپ = buf_ناپ;
	new_thread->ڈھیر_info.delta = delta;

#جے_تعریف_شدہ CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY
	new_thread->ڈھیر_info.usage.بن_ورتیا_حد =
		(CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY_DEFAULT_بن_ورتیا_حد_PCT *
		 buf_ناپ) / 100;
#جے_انت
#وإلا
	ARG_بن_ورتیا(new_thread);
	ARG_بن_ورتیا(buf_شروع);
	ARG_بن_ورتیا(buf_ناپ);
	ARG_بن_ورتیا(delta);
#جے_انت /* CONFIG_THREAD_STACK_INFO */
}

ساكن حرف *تیاری_thread_ڈھیر(بنية k_thread *new_thread,
				k_thread_ڈھیر_t *ڈھیر, size_t ڈھیر_ناپ)
{
	بنية ڈھیر_geometry geo = compute_ڈھیر_geometry(ڈھیر, ڈھیر_ناپ);
	حرف *ڈھیر_ptr = map_thread_ڈھیر(new_thread, ڈھیر, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ڈھیر, new_thread, geo.obj_ناپ, (فراغ *)geo.buf_شروع,
		geo.buf_ناپ, (فراغ *)ڈھیر_ptr);

	fill_init_ڈھیر(geo.buf_شروع, geo.buf_ناپ);
	سیٹ_ڈھیر_sentinel(geo.buf_شروع);
	delta = reserve_ڈھیر_headroom(new_thread, ڈھیر_ptr, geo.buf_ناپ);
	سیٹ_ڈھیر_info(new_thread, geo.buf_شروع, geo.buf_ناپ, delta);

	أرجع ڈھیر_ptr - delta;
}

#جے_تعریف_شدہ CONFIG_HW_SHADOW_ڈھیر
ساكن فراغ تیاری_shadow_ڈھیر(بنية k_thread *new_thread,
			 k_thread_ڈھیر_t *ڈھیر)
{
	صحيح ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ڈھیر_منزل_hw_shadow_ڈھیر, stk_منزل_hw_shstk) {
		إذا (stk_منزل_hw_shstk->ڈھیر == ڈھیر) {
			ret = k_thread_hw_shadow_ڈھیر_attach(new_thread,
							      stk_منزل_hw_shstk->shstk_addr,
							      stk_منزل_hw_shstk->ناپ);
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

	STRUCT_SECTION_FOREACH(ڈھیر_منزل_hw_shadow_ڈھیر_arr, stk_منزل_hw_shstk) {
		إذا ((uintptr_t)ڈھیر >= stk_منزل_hw_shstk->ڈھیر_addr &&
		    (uintptr_t)ڈھیر < stk_منزل_hw_shstk->ڈھیر_addr +
		    stk_منزل_hw_shstk->ڈھیر_ناپ * stk_منزل_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ڈھیر_وتھ = (uintptr_t)ڈھیر - stk_منزل_hw_shstk->ڈھیر_addr;
			uintptr_t ڈھیر_اشاریہ = ڈھیر_وتھ / stk_منزل_hw_shstk->ڈھیر_ناپ;
			uintptr_t addr;

			إذا (ڈھیر_اشاریہ >= stk_منزل_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ڈھیر);
				k_panic();
			}

			addr = stk_منزل_hw_shstk->shstk_addr +
				stk_منزل_hw_shstk->shstk_ناپ * ڈھیر_اشاریہ;
			ret = k_thread_hw_shadow_ڈھیر_attach(new_thread,
							      (arch_thread_hw_shadow_ڈھیر_t *)addr,
							      stk_منزل_hw_shstk->shstk_ناپ);
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
			new_thread, ڈھیر);
		k_panic();
	}
}
#وإلا
/* No-op stub used when hardware shadow stacks are not configured. */
ساكن مضمن فراغ تیاری_shadow_ڈھیر(بنية k_thread *new_thread,
				      k_thread_ڈھیر_t *ڈھیر)
{
	ARG_بن_ورتیا(new_thread);
	ARG_بن_ورتیا(ڈھیر);
}
#جے_انت

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
#جے_تعریف_شدہ CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ساكن مضمن فراغ init_thread_obj_core(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(thread), &thread->ادھار.usage,
				  حجم(thread->ادھار.usage));
#جے_انت /* CONFIG_OBJ_CORE_STATS_THREAD */
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ساكن مضمن فراغ init_thread_userspace(بنية k_thread *thread,
					 k_thread_ڈھیر_t *ڈھیر,
					 uint32_t options)
{
#جے_تعریف_شدہ CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ڈھیر_is_user_capable(ڈھیر),
		 "user thread %p with kernel-only stack %p", thread, ڈھیر);
	k_object_init(thread);
	k_object_init(ڈھیر);
	thread->ڈھیر_obj = ڈھیر;
	thread->syscall_چوکھٹا = عدم;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#وإلا
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(ڈھیر);
	ARG_بن_ورتیا(options);
#جے_انت /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ساكن مضمن فراغ assert_thread_coherence(بنية k_thread *thread,
					   k_thread_ڈھیر_t *ڈھیر)
{
#جے_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#جے_بنا_تعریف CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ڈھیر));
#جے_انت /* CONFIG_DYNAMIC_THREAD */
#جے_انت /* CONFIG_KERNEL_COHERENCE */
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(ڈھیر);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ساكن مضمن فراغ assert_switch_handle(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != عدم,
		 "arch layer failed to initialize switch_handle");
#جے_انت /* CONFIG_USE_SWITCH */
	ARG_بن_ورتیا(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ساكن مضمن فراغ init_thread_custom_انکڑے(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_THREAD_CUSTOM_انکڑے
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_انکڑے = عدم;
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ساكن مضمن فراغ جوڑو_thread_منزل_monitor(بنية k_thread *thread,
					 k_thread_اندراج_t اندراج,
					 فراغ *p1, فراغ *p2, فراغ *p3)
{
#جے_تعریف_شدہ CONFIG_THREAD_MONITOR
	thread->اندراج.p_اندراج = اندراج;
	thread->اندراج.parameter1 = p1;
	thread->اندراج.parameter2 = p2;
	thread->اندراج.parameter3 = p3;

	k_spinlock_کلید_t کلید = k_spin_جندرا(&z_thread_monitor_جندرا);

	thread->اگلا_thread = _kernel.دھاگے;
	_kernel.دھاگے = thread;
	k_spin_unlock(&z_thread_monitor_جندرا, کلید);
#وإلا
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(اندراج);
	ARG_بن_ورتیا(p1);
	ARG_بن_ورتیا(p2);
	ARG_بن_ورتیا(p3);
#جے_انت /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ساكن مضمن فراغ init_thread_name(بنية k_thread *thread, ثابت حرف *name)
{
#جے_تعریف_شدہ CONFIG_THREAD_NAME
	سیٹ_thread_name(thread, name);
#وإلا
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(name);
#جے_انت /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ساكن مضمن فراغ init_thread_cpu_نقاب(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
	إذا (IS_ENABLED(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)) {
		thread->ادھار.cpu_نقاب = 1; /* must specify only one cpu */
	} وإلا {
		thread->ادھار.cpu_نقاب = -1; /* allow all cpus */
	}
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ساكن مضمن فراغ init_thread_userspace_perms(بنية k_thread *thread,
					       uint32_t options)
{
#جے_تعریف_شدہ CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	إذا ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#وإلا
	ARG_بن_ورتیا(thread);
	ARG_بن_ورتیا(options);
#جے_انت /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ساكن مضمن فراغ init_thread_deadline(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_SCHED_DEADLINE
	thread->ادھار.prio_deadline = 0;
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ساكن مضمن فراغ init_thread_halt_قطار(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_SMP
	z_waitq_init(&thread->halt_قطار);
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ساكن مضمن فراغ init_thread_usage(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE
	thread->ادھار.usage = (بنية k_چکر_stats) {};
	thread->ادھار.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ساكن مضمن فراغ assert_thread_not_reused(بنية k_thread *new_thread)
{
#جے_تعریف_شدہ CONFIG_THREAD_MONITOR
#جے_تعریف_شدہ CONFIG_ASSERT
	k_spinlock_کلید_t کلید = k_spin_جندرا(&z_thread_monitor_جندرا);
	bool reused = false;

	/* Check if the thread is already in the list */
	لكل (بنية k_thread *t = _kernel.دھاگے; t; t = t->اگلا_thread) {
		إذا (t == new_thread) {
			reused = true;
			اكسر;
		}
	}

	k_spin_unlock(&z_thread_monitor_جندرا, کلید);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#جے_انت /* CONFIG_ASSERT */
#جے_انت /* CONFIG_THREAD_MONITOR */
	ARG_بن_ورتیا(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
حرف *z_تیاری_new_thread(بنية k_thread *new_thread,
			 k_thread_ڈھیر_t *ڈھیر, size_t ڈھیر_ناپ,
			 k_thread_اندراج_t اندراج,
			 فراغ *p1, فراغ *p2, فراغ *p3,
			 صحيح prio, uint32_t options, ثابت حرف *name)
{
	حرف *ڈھیر_ptr;

	Z_ASSERT_VALID_PRIO(prio, اندراج);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ڈھیر, options);

	z_waitq_init(&new_thread->join_قطار);

	/* Initialize various struct k_thread members */
	z_init_thread_ادھار(&new_thread->ادھار, prio, _THREAD_SLEEPING, options);
	ڈھیر_ptr = تیاری_thread_ڈھیر(new_thread, ڈھیر, ڈھیر_ناپ);

#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = عدم;
	new_thread->orig_prio = new_thread->ادھار.prio;
#جے_انت /* Z_MUTEX_PI_ENABLED */

	تیاری_shadow_ڈھیر(new_thread, ڈھیر);
	assert_thread_coherence(new_thread, ڈھیر);

	arch_new_thread(new_thread, ڈھیر, ڈھیر_ptr, اندراج, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_انکڑے = عدم;

	assert_switch_handle(new_thread);
	init_thread_custom_انکڑے(new_thread);
	جوڑو_thread_منزل_monitor(new_thread, اندراج, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_نقاب(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	إذا (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_وٹاندرا_منزل_MAIN) &&
	    (_current == عدم)) {
		new_thread->resource_pool = عدم;
		أرجع ڈھیر_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_قطار(new_thread);
	init_thread_usage(new_thread);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	أرجع ڈھیر_ptr;
}

#جے_تعریف_شدہ CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY
صحيح z_impl_k_thread_runtime_ڈھیر_بن_ورتیا_حد_pct_سیٹ(بنية k_thread *thread,
							   uint32_t pct)
{
	size_t بن_ورتیا_حد;

	إذا (pct > 99) {
		أرجع -EINVAL;   /* 100% unused stack and up is invalid */
	}

	بن_ورتیا_حد = (thread->ڈھیر_info.ناپ * pct) / 100;

	thread->ڈھیر_info.usage.بن_ورتیا_حد = بن_ورتیا_حد;

	أرجع 0;
}

صحيح z_impl_k_thread_runtime_ڈھیر_بن_ورتیا_حد_سیٹ(بنية k_thread *thread,
						       size_t حد)
{
	إذا (حد > thread->ڈھیر_info.ناپ) {
		أرجع -EINVAL;
	}

	thread->ڈھیر_info.usage.بن_ورتیا_حد = حد;

	أرجع 0;
}

size_t z_impl_k_thread_runtime_ڈھیر_بن_ورتیا_حد_get(بنية k_thread *thread)
{
	أرجع thread->ڈھیر_info.usage.بن_ورتیا_حد;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
صحيح z_vrfy_k_thread_runtime_ڈھیر_بن_ورتیا_حد_pct_سیٹ(بنية k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ڈھیر_بن_ورتیا_حد_pct_سیٹ(thread, pct);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

صحيح z_vrfy_k_thread_runtime_ڈھیر_بن_ورتیا_حد_سیٹ(بنية k_thread *thread,
						       size_t حد)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ڈھیر_بن_ورتیا_حد_سیٹ(thread, حد);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ڈھیر_بن_ورتیا_حد_get(بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ڈھیر_بن_ورتیا_حد_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */
#جے_انت /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(بنية k_thread *new_thread,
			      k_thread_ڈھیر_t *ڈھیر,
			      size_t ڈھیر_ناپ, k_thread_اندراج_t اندراج,
			      فراغ *p1, فراغ *p2, فراغ *p3,
			      صحيح prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_وچ_isr(), "Threads may not be created in ISRs");

	z_تیاری_new_thread(new_thread, ڈھیر, ڈھیر_ناپ, اندراج, p1, p2, p3,
			  prio, options, عدم);

	thread_ویلا_سارنی_new(new_thread, delay);

	أرجع new_thread;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
bool z_ڈھیر_is_user_capable(k_thread_ڈھیر_t *ڈھیر)
{
	أرجع k_object_لبھو(ڈھیر) != عدم;
}

k_tid_t z_vrfy_k_thread_create(بنية k_thread *new_thread,
			       k_thread_ڈھیر_t *ڈھیر,
			       size_t ڈھیر_ناپ, k_thread_اندراج_t اندراج,
			       فراغ *p1, فراغ *p2, فراغ *p3,
			       صحيح prio, uint32_t options, k_timeout_t delay)
{
	size_t کل_ناپ, ڈھیر_obj_ناپ;
	بنية k_object *ڈھیر_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ڈھیر_object = k_object_لبھو(ڈھیر);
	K_OOPS(K_SYSCALL_VERIFY_پیغام(k_object_validation_check(ڈھیر_object, ڈھیر,
						K_OBJ_THREAD_ڈھیر_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_پیغام(!ناپ_جوڑو_حد_توں_ودھ(K_THREAD_ڈھیر_RESERVED,
						       ڈھیر_ناپ, &کل_ناپ),
				    "stack size overflow (%zu+%zu)",
				    ڈھیر_ناپ,
				    K_THREAD_ڈھیر_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#جے_تعریف_شدہ CONFIG_GEN_PRIV_STACKS
	ڈھیر_obj_ناپ = ڈھیر_object->انکڑے.ڈھیر_انکڑے->ناپ;
#وإلا
	ڈھیر_obj_ناپ = ڈھیر_object->انکڑے.ڈھیر_ناپ;
#جے_انت /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_پیغام(کل_ناپ <= ڈھیر_obj_ناپ,
				    "stack size %zu is too big, max is %zu",
				    کل_ناپ, ڈھیر_obj_ناپ));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, عدم)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_ہیٹھلا_or_equal(prio,
							_current->ادھار.prio)));

	z_تیاری_new_thread(new_thread, ڈھیر, ڈھیر_ناپ,
			   اندراج, p1, p2, p3, prio, options, عدم);

	thread_ویلا_سارنی_new(new_thread, delay);

	أرجع new_thread;
}
#ضمن <zephyr/syscalls/k_thread_create_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_init_thread_ادھار(بنية thread_ادھار *thread_ادھار_2, صحيح ترجیح,
		       uint32_t initial_state, بنانشانی صحيح options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_ادھار_2->pended_on = عدم;
	thread_ادھار_2->user_options = (uint16_t)options;
	thread_ادھار_2->thread_state = (uint8_t)initial_state;

	thread_ادھار_2->prio = ترجیح;

	thread_ادھار_2->sched_locked = 0U;

#جے_تعریف_شدہ CONFIG_SMP
	thread_ادھار_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_ادھار_2->cpu = 0;
#جے_انت /* CONFIG_SMP */

#جے_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	thread_ادھار_2->ٹکڑا_ticks = 0;
	thread_ادھار_2->ٹکڑا_expired = عدم;
#جے_انت /* CONFIG_TIMESLICE_PER_THREAD */

#جے_تعریف_شدہ CONFIG_SPIN_VALIDATE
	thread_ادھار_2->وٹاندرا_انکڑے = عدم;
	/* otherwise swap_data does not need to be initialized */
#جے_انت

	z_init_thread_timeout(thread_ادھار_2);
}

FUNC_NORETURN فراغ k_thread_user_منوال_enter(k_thread_اندراج_t اندراج,
					    فراغ *p1, فراغ *p2, فراغ *p3)
{
	SYS_پورٹ_TRACING_FUNC(k_thread, user_منوال_enter);

	_current->ادھار.user_options |= K_USER;
	z_thread_essential_clear(_current);
#جے_تعریف_شدہ CONFIG_THREAD_MONITOR
	_current->اندراج.p_اندراج = اندراج;
	_current->اندراج.parameter1 = p1;
	_current->اندراج.parameter2 = p2;
	_current->اندراج.parameter3 = p3;
#جے_انت /* CONFIG_THREAD_MONITOR */
#جے_تعریف_شدہ CONFIG_USERSPACE
	__ASSERT(z_ڈھیر_is_user_capable(_current->ڈھیر_obj),
		 "dropping to user mode with kernel-only stack object");
#جے_تعریف_شدہ CONFIG_THREAD_USERSPACE_LOCAL_انکڑے
	memset(_current->userspace_local_انکڑے, 0,
	       حجم(بنية thread_userspace_local_انکڑے));
#جے_انت /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#جے_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ڈھیر_تیاری(_current,
			     (حرف *)(_current->ڈھیر_info.شروع +
				      _current->ڈھیر_info.ناپ));
#جے_انت /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_منوال_enter(اندراج, p1, p2, p3);
#وإلا
	/* XXX In this case we do not reset the stack */
	z_thread_اندراج(اندراج, p1, p2, p3);
#جے_انت /* CONFIG_USERSPACE */
}

#إذا defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ڈھیر_INFO)
#جے_تعریف_شدہ CONFIG_ڈھیر_GROWS_اتے
#غلطی "Unsupported configuration for stack analysis"
#جے_انت /* CONFIG_STACK_GROWS_UP */

صحيح z_ڈھیر_space_get(ثابت uint8_t *ڈھیر_شروع, size_t ناپ, size_t *بن_ورتیا_ptr)
{
	size_t بن_ورتیا = 0;
	ثابت uint8_t *checked_ڈھیر = ڈھیر_شروع;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ثابت uint8_t *ڈھیر_اشارہ_گر = (ثابت uint8_t *)&ڈھیر_شروع;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	إذا ((ڈھیر_اشارہ_گر > ڈھیر_شروع) && (ڈھیر_اشارہ_گر <= (ڈھیر_شروع + ناپ)) &&
	    IS_ENABLED(CONFIG_NO_بن_ورتیا_ڈھیر_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		أرجع -ENOTSUP;
	}

	إذا (IS_ENABLED(CONFIG_ڈھیر_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ڈھیر += 4;
		ناپ -= 4;
	}

	لكل (size_t i = 0; i < ناپ; i++) {
		إذا ((checked_ڈھیر[i]) == 0xaaU) {
			بن_ورتیا++;
		} وإلا {
			اكسر;
		}
	}

	*بن_ورتیا_ptr = بن_ورتیا;

	أرجع 0;
}

#جے_تعریف_شدہ CONFIG_THREAD_RUNTIME_ڈھیر_SAFETY
صحيح k_thread_runtime_ڈھیر_safety_بھریا_check(ثابت بنية k_thread *thread,
					     size_t *بن_ورتیا_ptr,
					     k_thread_ڈھیر_safety_handler_t handler,
					     فراغ *arg)
{
	صحيح    rv;
	size_t بن_ورتیا_space;

	__ASSERT_NO_MSG(thread != عدم);

	rv = z_ڈھیر_space_get((ثابت uint8_t *)thread->ڈھیر_info.شروع,
			       thread->ڈھیر_info.ناپ, &بن_ورتیا_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (بن_ورتیا_ptr != عدم) {
		*بن_ورتیا_ptr = بن_ورتیا_space;
	}

	إذا ((بن_ورتیا_space < thread->ڈھیر_info.usage.بن_ورتیا_حد) &&
	    (handler != عدم)) {
		handler(thread, بن_ورتیا_space, arg);
	}

	أرجع 0;
}

صحيح k_thread_runtime_ڈھیر_safety_حد_check(ثابت بنية k_thread *thread,
						  size_t *بن_ورتیا_ptr,
						  k_thread_ڈھیر_safety_handler_t handler,
						  فراغ *arg)
{
	صحيح    rv;
	size_t بن_ورتیا_space;
	size_t حد;
	size_t جانچ_ناپ;

	__ASSERT_NO_MSG(thread != عدم);

	حد = thread->ڈھیر_info.usage.بن_ورتیا_حد;
	جانچ_ناپ = حد;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	إذا (IS_ENABLED(CONFIG_ڈھیر_SENTINEL) && (حد != 0U)) {
		جانچ_ناپ += حجم(uint32_t);
	}

	rv = z_ڈھیر_space_get((ثابت uint8_t *)thread->ڈھیر_info.شروع,
			       جانچ_ناپ, &بن_ورتیا_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (بن_ورتیا_ptr != عدم) {
		*بن_ورتیا_ptr = بن_ورتیا_space;
	}

	إذا ((بن_ورتیا_space < حد) && (handler != عدم)) {
		handler(thread, بن_ورتیا_space, arg);
	}

	أرجع 0;
}
#جے_انت /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

صحيح z_impl_k_thread_ڈھیر_space_get(ثابت بنية k_thread *thread,
				    size_t *بن_ورتیا_ptr)
{
#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	إذا (thread->ڈھیر_info.mapped.addr == عدم) {
		أرجع -EINVAL;
	}
#جے_انت /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع z_ڈھیر_space_get((ثابت uint8_t *)thread->ڈھیر_info.شروع,
				 thread->ڈھیر_info.ناپ, بن_ورتیا_ptr);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
صحيح z_vrfy_k_thread_ڈھیر_space_get(ثابت بنية k_thread *thread,
				    size_t *بن_ورتیا_ptr)
{
	size_t بن_ورتیا;
	صحيح ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ڈھیر_space_get(thread, &بن_ورتیا);
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	ret = k_usermode_منزل_copy(بن_ورتیا_ptr, &بن_ورتیا, حجم(size_t));
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	أرجع 0;
}
#ضمن <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */
#جے_انت /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_بچیا_ticks(
						    ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_بچیا_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_expires_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

#جے_تعریف_شدہ CONFIG_INSTRUMENT_THREAD_SWITCHING
فراغ z_thread_mark_switched_وچ(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_شروع(_current);
#جے_انت /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#جے_تعریف_شدہ CONFIG_TRACING
	SYS_پورٹ_TRACING_FUNC(k_thread, switched_وچ);
#جے_انت /* CONFIG_TRACING */
}

فراغ z_thread_mark_switched_باہرلا(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#جے_انت /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#جے_تعریف_شدہ CONFIG_TRACING
#جے_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	إذا (!_current ||
	    (_current->ادھار.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}
#جے_انت /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_پورٹ_TRACING_FUNC(k_thread, switched_باہرلا);
#جے_انت /* CONFIG_TRACING */
}
#جے_انت /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

صحيح k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	إذا ((thread == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#وإلا
	*stats = (k_thread_runtime_stats_t) {};
#جے_انت /* CONFIG_SCHED_THREAD_USAGE */

	أرجع 0;
}

صحيح k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ALL */

	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	بنانشانی صحيح عدد_cpus = arch_عدد_cpus();

	لكل (بنانشانی صحيح i = 0; i < عدد_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_چکراں += tmp_stats.execution_چکراں;
		stats->کل_چکراں     += tmp_stats.کل_چکراں;
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_چکراں   += tmp_stats.current_چکراں;
		stats->peak_چکراں      += tmp_stats.peak_چکراں;
		stats->اوسطن_چکراں   += tmp_stats.اوسطن_چکراں;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_چکراں      += tmp_stats.idle_چکراں;
	}
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ALL */

	أرجع 0;
}

صحيح k_thread_runtime_stats_cpu_get(صحيح cpu, k_thread_runtime_stats_t *stats)
{
	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((بنانشانی صحيح)cpu >= arch_عدد_cpus())) {
		أرجع -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#وإلا
	ARG_بن_ورتیا(cpu);
#جے_انت

	أرجع 0;
}

#جے_تعریف_شدہ CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ساكن بنية k_thread *thread_منزل_cleanup;

/** Spinlock for thread abort cleanup. */
ساكن بنية k_spinlock thread_cleanup_جندرا;

#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
ساكن فراغ *thread_cleanup_ڈھیر_addr;
ساكن size_t thread_cleanup_ڈھیر_sz;

ساكن فراغ unmap_thread_ڈھیر(فراغ *addr, size_t sz)
{
	إذا (addr != عدم) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#جے_انت /* CONFIG_THREAD_STACK_MEM_MAPPED */

فراغ defer_thread_cleanup(بنية k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_منزل_cleanup = thread;

#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
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
	thread->ڈھیر_info.mapped.addr = عدم;
	thread->ڈھیر_info.mapped.sz = 0;
#جے_انت /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_deferred_thread_cleanup(فراغ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	unmap_thread_ڈھیر(thread_cleanup_ڈھیر_addr, thread_cleanup_ڈھیر_sz);

	thread_cleanup_ڈھیر_addr = عدم;
	thread_cleanup_ڈھیر_sz = 0;
#جے_انت /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_thread_cleanup(بنية k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_MEM_MAPPED
	unmap_thread_ڈھیر(thread->ڈھیر_info.mapped.addr,
			   thread->ڈھیر_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ڈھیر_info.mapped.addr = عدم;
	thread->ڈھیر_info.mapped.sz = 0;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_بن_ورتیا(thread);
#جے_انت /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ k_thread_abort_cleanup(بنية k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_جندرا) {
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
	K_SPINLOCK(&thread_cleanup_جندرا) {
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

#جے_انت /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

فراغ z_dummy_thread_init(بنية k_thread *dummy_thread)
{
	dummy_thread->ادھار.thread_state = _THREAD_DUMMY;
#جے_تعریف_شدہ CONFIG_SCHED_CPU_نقاب
	dummy_thread->ادھار.cpu_نقاب = -1;
#جے_انت /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->ادھار.user_options = K_ESSENTIAL;
#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = عدم;
	dummy_thread->orig_prio = dummy_thread->ادھار.prio;
#جے_انت /* Z_MUTEX_PI_ENABLED */
#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_INFO
	dummy_thread->ڈھیر_info.شروع = 0U;
	dummy_thread->ڈھیر_info.ناپ = 0U;
#جے_انت /* CONFIG_THREAD_STACK_INFO */
#جے_تعریف_شدہ CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#جے_انت /* CONFIG_USERSPACE */
#إذا (K_ڈھیر_MEM_POOL_ناپ > 0)
	k_thread_system_pool_سونپو(dummy_thread);
#وإلا
	dummy_thread->resource_pool = عدم;
#جے_انت /* K_HEAP_MEM_POOL_SIZE */

#جے_تعریف_شدہ CONFIG_USE_SWITCH
	dummy_thread->switch_handle = عدم;
#جے_انت /* CONFIG_USE_SWITCH */

#جے_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	dummy_thread->ادھار.ٹکڑا_ticks = 0;
#جے_انت /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_سیٹ(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

فراغ z_impl_k_thread_ترجیح_سیٹ(k_tid_t thread, صحيح prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, عدم);

	bool need_sched = z_thread_prio_سیٹ((بنية k_thread *)thread, prio);

	إذا ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->ادھار.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_ترجیح_سیٹ(k_tid_t thread, صحيح prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_پیغام(_is_valid_prio(prio, عدم),
				    "invalid thread priority %d", prio));
#جے_بنا_تعریف CONFIG_USERSPACE_THREAD_MAY_RAISE_ترجیح
	K_OOPS(K_SYSCALL_VERIFY_پیغام((int8_t)prio >= thread->ادھار.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->ادھار.prio));
#جے_انت /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_ترجیح_سیٹ(thread, prio);
}
#ضمن <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	إذا (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_وچ_isr())) {
		z_thread_suspend_current(thread);
		أرجع;
	}

	k_spinlock_کلید_t کلید = z_sched_spinlock_جندرا();

	إذا (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(کلید);
		أرجع;
	}

	z_thread_halt(thread, کلید, false);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ضمن <zephyr/syscalls/k_thread_suspend_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_جندرا();

	/* Do not try to resume a thread that was not suspended */
	إذا (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(کلید);
		أرجع;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ضمن <zephyr/syscalls/k_thread_resume_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_جندرا();

	إذا (z_is_thread_sleeping(thread)) {
		(فراغ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(کلید);
	} وإلا {
		z_sched_spinlock_unlock(کلید);
	}
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ضمن <zephyr/syscalls/k_wakeup_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_thread_abort(بنية k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_کلید_t کلید = z_sched_spinlock_جندرا();

	إذا (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(کلید);
		أرجع;
	}

	z_thread_halt(thread, کلید, true);

	إذا (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#إذا !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
فراغ z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#جے_انت /* !CONFIG_ARCH_HAS_THREAD_ABORT */

صحيح z_impl_k_thread_join(بنية k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_جندرا();
	صحيح ret;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	إذا (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} وإلا إذا ((thread == _current) ||
		   (thread->ادھار.pended_on == &_current->join_قطار)) {
		ret = -EDEADLK;
	} وإلا {
		__ASSERT(!arch_is_وچ_isr(), "cannot join in ISR");
		z_sched_جوڑو_منزل_waitq_locked(_current, &thread->join_قطار);
		z_جوڑو_thread_timeout(_current, timeout);

		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_وٹاندرا_locked(کلید);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		أرجع ret;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(کلید);
	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
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
	بنية k_object *ko = k_object_لبھو(thread);
	صحيح ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	بدل (ret) {
	حالة 0:
		أرجع false;
	حالة -EINVAL:
		أرجع true;
	افتراضي:
#جے_تعریف_شدہ CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#جے_انت /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_پیغام(ret == 0, "access denied"));
	}
	کوڈ_UNREACHABLE; /* LCOV_EXCL_LINE */
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

	K_OOPS(K_SYSCALL_VERIFY_پیغام(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((بنية k_thread *)thread);
}
#ضمن <zephyr/syscalls/k_thread_abort_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

bool k_can_yield(فراغ)
{
	أرجع !(k_is_pre_kernel() || k_is_وچ_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

فراغ z_impl_k_yield(فراغ)
{
	__ASSERT(!arch_is_وچ_isr(), "");

	SYS_پورٹ_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_yield(فراغ)
{
	z_impl_k_yield();
}
#ضمن <zephyr/syscalls/k_yield_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */
