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

#که_تعریف_شوی CONFIG_OBJ_CORE_THREAD
#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_THREAD
ساكن بنية k_obj_core_stats_desc  thread_stats_desc = {
	.raw_اندازه = حجم(بنية k_دوره_stats),
	.query_اندازه = حجم(بنية k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#د_که_پای /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#وإلا
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    عدم);
#د_که_پای /* CONFIG_OBJ_CORE_STATS_THREAD */
#د_که_پای /* CONFIG_OBJ_CORE_THREAD */


#تعریف_کړه _FOREACH_STATIC_THREAD(thread_معلومات)              \
	STRUCT_SECTION_FOREACH(static_thread_معلومات, thread_معلومات)

bool k_is_دننه_isr(فراغ)
{
	أرجع arch_is_دننه_isr();
}
EXPORT_نښه(k_is_دننه_isr);

#که_تعریف_شوی CONFIG_THREAD_CUSTOM_معلومات
فراغ z_impl_k_thread_custom_معلومات_سېټ(فراغ *ارزښت)
{
	_current->custom_معلومات = ارزښت;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_custom_معلومات_سېټ(فراغ *معلومات)
{
	z_impl_k_thread_custom_معلومات_سېټ(معلومات);
}
#ضمن <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ *z_impl_k_thread_custom_معلومات_get(فراغ)
{
	أرجع _current->custom_معلومات;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_thread_custom_معلومات_get(فراغ)
{
	أرجع z_impl_k_thread_custom_معلومات_get();
}
#ضمن <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#د_که_پای /* CONFIG_USERSPACE */
#د_که_پای /* CONFIG_THREAD_CUSTOM_DATA */

صحيح z_impl_k_is_preempt_thread(فراغ)
{
	أرجع !arch_is_دننه_isr() && thread_is_preemptible(_current);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_is_preempt_thread(فراغ)
{
	أرجع z_impl_k_is_preempt_thread();
}
#ضمن <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_لومړیتوب_get(k_tid_t thread)
{
	أرجع thread->بنسټ.prio;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_لومړیتوب_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_لومړیتوب_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

#که_تعریف_شوی CONFIG_THREAD_NAME
ساكن فراغ سېټ_thread_name(بنية k_thread *thread, ثابت حرف *str)
{
	إذا (str != عدم) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#که_تعریف_شوی CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_سېټ(thread, str);
#د_که_پای /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} وإلا {
		thread->name[0] = '\0';
	}
}
#د_که_پای /* CONFIG_THREAD_NAME */

صحيح z_impl_k_thread_name_سېټ(k_tid_t thread, ثابت حرف *str)
{
#که_تعریف_شوی CONFIG_THREAD_NAME
	إذا (thread == عدم) {
		thread = _current;
	}

	سېټ_thread_name(thread, str);

	SYS_پورټ_TRACING_OBJ_FUNC(k_thread, name_سېټ, thread, 0);

	أرجع 0;
#وإلا
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(str);

	SYS_پورټ_TRACING_OBJ_FUNC(k_thread, name_سېټ, thread, -ENOSYS);

	أرجع -ENOSYS;
#د_که_پای /* CONFIG_THREAD_NAME */
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_سېټ(k_tid_t thread, ثابت حرف *str)
{
#که_تعریف_شوی CONFIG_THREAD_NAME
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

	أرجع z_impl_k_thread_name_سېټ(thread, name);
#وإلا
	أرجع -ENOSYS;
#د_که_پای /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_set_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

ثابت حرف *k_thread_name_get(k_tid_t thread)
{
#که_تعریف_شوی CONFIG_THREAD_NAME
	أرجع (ثابت حرف *)thread->name;
#وإلا
	ARG_ناکارول_شوی(thread);
	أرجع عدم;
#د_که_پای /* CONFIG_THREAD_NAME */
}

صحيح z_impl_k_thread_name_copy(k_tid_t thread, حرف *buf, size_t اندازه)
{
#که_تعریف_شوی CONFIG_THREAD_NAME
	strncpy(buf, thread->name, اندازه);
	أرجع 0;
#وإلا
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(buf);
	ARG_ناکارول_شوی(اندازه);
	أرجع -ENOSYS;
#د_که_پای /* CONFIG_THREAD_NAME */
}

ساكن size_t copy_بایټونه(حرف *dest, size_t dest_اندازه, ثابت حرف *src, size_t src_اندازه)
{
	size_t  بایټونه_منزل_copy;

	بایټونه_منزل_copy = min(dest_اندازه, src_اندازه);
	حافظه_نقل(dest, src, بایټونه_منزل_copy);

	أرجع بایټونه_منزل_copy;
}

ثابت حرف *k_thread_state_str(k_tid_t thread_id, حرف *buf, size_t buf_اندازه)
{
	size_t      off = 0;
	uint8_t     بټ_2;
	uint8_t     thread_state = thread_id->بنسټ.thread_state;
#تعریف_کړه SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, حجم(Z_STATE_STR_##s) - 1 }
	ساكن ثابت بنية {
		ثابت حرف *str;
		uint16_t    بټ_2;
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
#تعریف_لغوه SS_ENT

	إذا ((buf == عدم) || (buf_اندازه == 0)) {
		أرجع "";
	}

	buf_اندازه--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	لكل (بینښه صحيح شاخص = 0; thread_state != 0; شاخص++) {
		بټ_2 = state_string[شاخص].بټ_2;
		إذا ((thread_state & بټ_2) == 0) {
			استمر;
		}

		off += copy_بایټونه(buf + off, buf_اندازه - off,
				  state_string[شاخص].str,
				  state_string[شاخص].len);

		thread_state &= ~بټ_2;

		إذا (thread_state != 0) {
			off += copy_بایټونه(buf + off, buf_اندازه - off, "+", 1);
		}
	}

	buf[off] = '\0';

	أرجع (ثابت حرف *)buf;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_copy(k_tid_t thread,
					    حرف *buf, size_t اندازه)
{
#که_تعریف_شوی CONFIG_THREAD_NAME
	size_t len;
	بنية k_object *ko = k_object_پیدا_کړه(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	إذا ((ko == عدم) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		أرجع -EINVAL;
	}
	إذا (K_SYSCALL_حافظه_WRITE(buf, اندازه) != 0) {
		أرجع -EFAULT;
	}
	len = مزي_اوږدوالی(thread->name);
	إذا ((len + 1) > اندازه) {
		أرجع -ENOSPC;
	}

	أرجع k_usermode_منزل_copy((فراغ *)buf, thread->name, len + 1);
#وإلا
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(buf);
	ARG_ناکارول_شوی(اندازه);
	أرجع -ENOSYS;
#د_که_پای /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

#که_تعریف_شوی CONFIG_ډېری_SENTINEL
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
فراغ z_check_ډېری_sentinel(فراغ)
{
	uint32_t *ډېری;

	إذا ((_current->بنسټ.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}

	ډېری = (uint32_t *)_current->ډېری_info.پیل;
	إذا (*ډېری != ډېری_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ډېری = ډېری_SENTINEL;
		z_except_reason(K_ERR_ډېری_CHK_FAIL);
	}
}
#د_که_پای /* CONFIG_STACK_SENTINEL */

#إذا defined(CONFIG_ډېری_اشاره_ګر_RANDOM) && (CONFIG_ډېری_اشاره_ګر_RANDOM != 0)
صحيح z_ډېری_adjust_initialized;

ساكن size_t random_دننه_واټن(size_t ډېری_اندازه)
{
	size_t random_val;

	إذا (!z_ډېری_adjust_initialized) {
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
	ثابت size_t fuzz = random_val % CONFIG_ډېری_اشاره_ګر_RANDOM;

	إذا (unlikely(fuzz * 2 > ډېری_اندازه)) {
		أرجع 0;
	}

	أرجع fuzz;
}
#إذا defined(CONFIG_ډېری_GROWS_پورته)
	/* This is so rare not bothering for now */
#تېروتنه "Stack pointer randomization not implemented for upward growing stacks"
#د_که_پای /* CONFIG_STACK_GROWS_UP */
#د_که_پای /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
بنية ډېری_geometry {
	size_t obj_اندازه;	/* size of the whole stack object */
	حرف *buf_پیل;	/* start of the usable stack buffer */
	size_t buf_اندازه;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ساكن بنية ډېری_geometry compute_ډېری_geometry(k_thread_ډېری_t *ډېری,
						    size_t ډېری_اندازه)
{
	بنية ډېری_geometry geo;

#که_تعریف_شوی CONFIG_USERSPACE
	إذا (z_ډېری_is_user_capable(ډېری)) {
		geo.obj_اندازه = K_THREAD_ډېری_LEN(ډېری_اندازه);
		geo.buf_پیل = K_THREAD_ډېری_بفر(ډېری);
		geo.buf_اندازه = geo.obj_اندازه - K_THREAD_ډېری_RESERVED;
	} وإلا
#د_که_پای /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_اندازه = K_KERNEL_ډېری_LEN(ډېری_اندازه);
		geo.buf_پیل = K_KERNEL_ډېری_بفر(ډېری);
		geo.buf_اندازه = geo.obj_اندازه - K_KERNEL_ډېری_RESERVED;

#إذا defined(ARCH_KERNEL_ډېری_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		إذا (K_KERNEL_ډېری_RESERVED > geo.obj_اندازه) {
			k_panic();
		}
#د_که_پای
	}

	أرجع geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ساكن حرف *map_thread_ډېری(بنية k_thread *new_thread,
			      k_thread_ډېری_t *ډېری,
			      بنية ډېری_geometry *geo)
{
	حرف *ډېری_ptr;

#که_تعریف_شوی CONFIG_THREAD_ډېری_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	فراغ *ډېری_mapped = k_mem_map_phys_guard((uintptr_t)ډېری, geo->obj_اندازه,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ډېری_mapped != 0);

#که_تعریف_شوی CONFIG_USERSPACE
	إذا (z_ډېری_is_user_capable(ډېری)) {
		geo->buf_پیل = K_THREAD_ډېری_بفر(ډېری_mapped);
	} وإلا
#د_که_پای /* CONFIG_USERSPACE */
	{
		geo->buf_پیل = K_KERNEL_ډېری_بفر(ډېری_mapped);
	}

	ډېری_ptr = (حرف *)ډېری_mapped + geo->obj_اندازه;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ډېری_info.mapped.addr = ډېری_mapped;
	new_thread->ډېری_info.mapped.sz = geo->obj_اندازه;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_ناکارول_شوی(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ډېری_ptr = (حرف *)ډېری + geo->obj_اندازه;
#د_که_پای /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع ډېری_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ساكن مضمن فراغ fill_init_ډېری(حرف *buf_پیل, size_t buf_اندازه)
{
#که_تعریف_شوی CONFIG_INIT_STACKS
	memset(buf_پیل, 0xaa, buf_اندازه);
#وإلا
	ARG_ناکارول_شوی(buf_پیل);
	ARG_ناکارول_شوی(buf_اندازه);
#د_که_پای /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ساكن مضمن فراغ سېټ_ډېری_sentinel(حرف *buf_پیل)
{
#که_تعریف_شوی CONFIG_ډېری_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_پیل) = ډېری_SENTINEL;
#وإلا
	ARG_ناکارول_شوی(buf_پیل);
#د_که_پای /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ساكن size_t reserve_ډېری_headroom(بنية k_thread *new_thread,
				     حرف *ډېری_ptr, size_t buf_اندازه)
{
	size_t delta = 0;

	ARG_ناکارول_شوی(new_thread);
	ARG_ناکارول_شوی(ډېری_ptr);
	ARG_ناکارول_شوی(buf_اندازه);

#که_تعریف_شوی CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ډېری_تیاری(new_thread, ډېری_ptr);
#د_که_پای /* CONFIG_THREAD_LOCAL_STORAGE */
#که_تعریف_شوی CONFIG_THREAD_USERSPACE_LOCAL_معلومات
	size_t tls_اندازه = حجم(بنية thread_userspace_local_معلومات);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_اندازه;
	new_thread->userspace_local_معلومات =
		(بنية thread_userspace_local_معلومات *)(ډېری_ptr - delta);
#د_که_پای /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#إذا defined(CONFIG_ډېری_اشاره_ګر_RANDOM) && (CONFIG_ډېری_اشاره_ګر_RANDOM != 0)
	delta += random_دننه_واټن(buf_اندازه);
#د_که_پای /* CONFIG_STACK_POINTER_RANDOM */

	أرجع پړاو_پورته(delta, ARCH_ډېری_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ساكن مضمن فراغ سېټ_ډېری_info(بنية k_thread *new_thread, حرف *buf_پیل,
				  size_t buf_اندازه, size_t delta)
{
#که_تعریف_شوی CONFIG_THREAD_ډېری_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ډېری_info.پیل = (uintptr_t)buf_پیل;
	new_thread->ډېری_info.اندازه = buf_اندازه;
	new_thread->ډېری_info.delta = delta;

#که_تعریف_شوی CONFIG_THREAD_RUNTIME_ډېری_SAFETY
	new_thread->ډېری_info.usage.ناکارول_شوی_پوله =
		(CONFIG_THREAD_RUNTIME_ډېری_SAFETY_DEFAULT_ناکارول_شوی_پوله_PCT *
		 buf_اندازه) / 100;
#د_که_پای
#وإلا
	ARG_ناکارول_شوی(new_thread);
	ARG_ناکارول_شوی(buf_پیل);
	ARG_ناکارول_شوی(buf_اندازه);
	ARG_ناکارول_شوی(delta);
#د_که_پای /* CONFIG_THREAD_STACK_INFO */
}

ساكن حرف *تیاری_thread_ډېری(بنية k_thread *new_thread,
				k_thread_ډېری_t *ډېری, size_t ډېری_اندازه)
{
	بنية ډېری_geometry geo = compute_ډېری_geometry(ډېری, ډېری_اندازه);
	حرف *ډېری_ptr = map_thread_ډېری(new_thread, ډېری, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ډېری, new_thread, geo.obj_اندازه, (فراغ *)geo.buf_پیل,
		geo.buf_اندازه, (فراغ *)ډېری_ptr);

	fill_init_ډېری(geo.buf_پیل, geo.buf_اندازه);
	سېټ_ډېری_sentinel(geo.buf_پیل);
	delta = reserve_ډېری_headroom(new_thread, ډېری_ptr, geo.buf_اندازه);
	سېټ_ډېری_info(new_thread, geo.buf_پیل, geo.buf_اندازه, delta);

	أرجع ډېری_ptr - delta;
}

#که_تعریف_شوی CONFIG_HW_SHADOW_ډېری
ساكن فراغ تیاری_shadow_ډېری(بنية k_thread *new_thread,
			 k_thread_ډېری_t *ډېری)
{
	صحيح ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ډېری_منزل_hw_shadow_ډېری, stk_منزل_hw_shstk) {
		إذا (stk_منزل_hw_shstk->ډېری == ډېری) {
			ret = k_thread_hw_shadow_ډېری_attach(new_thread,
							      stk_منزل_hw_shstk->shstk_addr,
							      stk_منزل_hw_shstk->اندازه);
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

	STRUCT_SECTION_FOREACH(ډېری_منزل_hw_shadow_ډېری_arr, stk_منزل_hw_shstk) {
		إذا ((uintptr_t)ډېری >= stk_منزل_hw_shstk->ډېری_addr &&
		    (uintptr_t)ډېری < stk_منزل_hw_shstk->ډېری_addr +
		    stk_منزل_hw_shstk->ډېری_اندازه * stk_منزل_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ډېری_دننه_واټن = (uintptr_t)ډېری - stk_منزل_hw_shstk->ډېری_addr;
			uintptr_t ډېری_شاخص = ډېری_دننه_واټن / stk_منزل_hw_shstk->ډېری_اندازه;
			uintptr_t addr;

			إذا (ډېری_شاخص >= stk_منزل_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ډېری);
				k_panic();
			}

			addr = stk_منزل_hw_shstk->shstk_addr +
				stk_منزل_hw_shstk->shstk_اندازه * ډېری_شاخص;
			ret = k_thread_hw_shadow_ډېری_attach(new_thread,
							      (arch_thread_hw_shadow_ډېری_t *)addr,
							      stk_منزل_hw_shstk->shstk_اندازه);
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
			new_thread, ډېری);
		k_panic();
	}
}
#وإلا
/* No-op stub used when hardware shadow stacks are not configured. */
ساكن مضمن فراغ تیاری_shadow_ډېری(بنية k_thread *new_thread,
				      k_thread_ډېری_t *ډېری)
{
	ARG_ناکارول_شوی(new_thread);
	ARG_ناکارول_شوی(ډېری);
}
#د_که_پای

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
#که_تعریف_شوی CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ساكن مضمن فراغ init_thread_obj_core(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_ثبتوونکی(K_OBJ_CORE(thread), &thread->بنسټ.usage,
				  حجم(thread->بنسټ.usage));
#د_که_پای /* CONFIG_OBJ_CORE_STATS_THREAD */
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ساكن مضمن فراغ init_thread_userspace(بنية k_thread *thread,
					 k_thread_ډېری_t *ډېری,
					 uint32_t options)
{
#که_تعریف_شوی CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ډېری_is_user_capable(ډېری),
		 "user thread %p with kernel-only stack %p", thread, ډېری);
	k_object_init(thread);
	k_object_init(ډېری);
	thread->ډېری_obj = ډېری;
	thread->syscall_چوکاټ = عدم;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#وإلا
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(ډېری);
	ARG_ناکارول_شوی(options);
#د_که_پای /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ساكن مضمن فراغ assert_thread_coherence(بنية k_thread *thread,
					   k_thread_ډېری_t *ډېری)
{
#که_تعریف_شوی CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#که_نه_تعریف_شوی CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ډېری));
#د_که_پای /* CONFIG_DYNAMIC_THREAD */
#د_که_پای /* CONFIG_KERNEL_COHERENCE */
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(ډېری);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ساكن مضمن فراغ assert_switch_handle(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != عدم,
		 "arch layer failed to initialize switch_handle");
#د_که_پای /* CONFIG_USE_SWITCH */
	ARG_ناکارول_شوی(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ساكن مضمن فراغ init_thread_custom_معلومات(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_THREAD_CUSTOM_معلومات
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_معلومات = عدم;
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ساكن مضمن فراغ جمع_کړه_thread_منزل_monitor(بنية k_thread *thread,
					 k_thread_ثبت_t ثبت,
					 فراغ *p1, فراغ *p2, فراغ *p3)
{
#که_تعریف_شوی CONFIG_THREAD_MONITOR
	thread->ثبت.p_ثبت = ثبت;
	thread->ثبت.parameter1 = p1;
	thread->ثبت.parameter2 = p2;
	thread->ثبت.parameter3 = p3;

	k_spinlock_کلی_t کلی = k_spin_قلف(&z_thread_monitor_قلف);

	thread->بل_thread = _kernel.تارونه;
	_kernel.تارونه = thread;
	k_spin_unlock(&z_thread_monitor_قلف, کلی);
#وإلا
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(ثبت);
	ARG_ناکارول_شوی(p1);
	ARG_ناکارول_شوی(p2);
	ARG_ناکارول_شوی(p3);
#د_که_پای /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ساكن مضمن فراغ init_thread_name(بنية k_thread *thread, ثابت حرف *name)
{
#که_تعریف_شوی CONFIG_THREAD_NAME
	سېټ_thread_name(thread, name);
#وإلا
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(name);
#د_که_پای /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ساكن مضمن فراغ init_thread_cpu_ماسک(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_SCHED_CPU_ماسک
	إذا (IS_ENABLED(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)) {
		thread->بنسټ.cpu_ماسک = 1; /* must specify only one cpu */
	} وإلا {
		thread->بنسټ.cpu_ماسک = -1; /* allow all cpus */
	}
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ساكن مضمن فراغ init_thread_userspace_perms(بنية k_thread *thread,
					       uint32_t options)
{
#که_تعریف_شوی CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	إذا ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#وإلا
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(options);
#د_که_پای /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ساكن مضمن فراغ init_thread_deadline(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_SCHED_DEADLINE
	thread->بنسټ.prio_deadline = 0;
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ساكن مضمن فراغ init_thread_halt_کتار(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_SMP
	z_waitq_init(&thread->halt_کتار);
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ساكن مضمن فراغ init_thread_usage(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE
	thread->بنسټ.usage = (بنية k_دوره_stats) {};
	thread->بنسټ.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ساكن مضمن فراغ assert_thread_not_reused(بنية k_thread *new_thread)
{
#که_تعریف_شوی CONFIG_THREAD_MONITOR
#که_تعریف_شوی CONFIG_ASSERT
	k_spinlock_کلی_t کلی = k_spin_قلف(&z_thread_monitor_قلف);
	bool reused = false;

	/* Check if the thread is already in the list */
	لكل (بنية k_thread *t = _kernel.تارونه; t; t = t->بل_thread) {
		إذا (t == new_thread) {
			reused = true;
			اكسر;
		}
	}

	k_spin_unlock(&z_thread_monitor_قلف, کلی);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#د_که_پای /* CONFIG_ASSERT */
#د_که_پای /* CONFIG_THREAD_MONITOR */
	ARG_ناکارول_شوی(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
حرف *z_تیاری_new_thread(بنية k_thread *new_thread,
			 k_thread_ډېری_t *ډېری, size_t ډېری_اندازه,
			 k_thread_ثبت_t ثبت,
			 فراغ *p1, فراغ *p2, فراغ *p3,
			 صحيح prio, uint32_t options, ثابت حرف *name)
{
	حرف *ډېری_ptr;

	Z_ASSERT_VALID_PRIO(prio, ثبت);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ډېری, options);

	z_waitq_init(&new_thread->join_کتار);

	/* Initialize various struct k_thread members */
	z_init_thread_بنسټ(&new_thread->بنسټ, prio, _THREAD_SLEEPING, options);
	ډېری_ptr = تیاری_thread_ډېری(new_thread, ډېری, ډېری_اندازه);

#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = عدم;
	new_thread->orig_prio = new_thread->بنسټ.prio;
#د_که_پای /* Z_MUTEX_PI_ENABLED */

	تیاری_shadow_ډېری(new_thread, ډېری);
	assert_thread_coherence(new_thread, ډېری);

	arch_new_thread(new_thread, ډېری, ډېری_ptr, ثبت, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_معلومات = عدم;

	assert_switch_handle(new_thread);
	init_thread_custom_معلومات(new_thread);
	جمع_کړه_thread_منزل_monitor(new_thread, ثبت, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_ماسک(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	إذا (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_بدلون_منزل_MAIN) &&
	    (_current == عدم)) {
		new_thread->resource_pool = عدم;
		أرجع ډېری_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_کتار(new_thread);
	init_thread_usage(new_thread);

	SYS_پورټ_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	أرجع ډېری_ptr;
}

#که_تعریف_شوی CONFIG_THREAD_RUNTIME_ډېری_SAFETY
صحيح z_impl_k_thread_runtime_ډېری_ناکارول_شوی_پوله_pct_سېټ(بنية k_thread *thread,
							   uint32_t pct)
{
	size_t ناکارول_شوی_پوله;

	إذا (pct > 99) {
		أرجع -EINVAL;   /* 100% unused stack and up is invalid */
	}

	ناکارول_شوی_پوله = (thread->ډېری_info.اندازه * pct) / 100;

	thread->ډېری_info.usage.ناکارول_شوی_پوله = ناکارول_شوی_پوله;

	أرجع 0;
}

صحيح z_impl_k_thread_runtime_ډېری_ناکارول_شوی_پوله_سېټ(بنية k_thread *thread,
						       size_t پوله)
{
	إذا (پوله > thread->ډېری_info.اندازه) {
		أرجع -EINVAL;
	}

	thread->ډېری_info.usage.ناکارول_شوی_پوله = پوله;

	أرجع 0;
}

size_t z_impl_k_thread_runtime_ډېری_ناکارول_شوی_پوله_get(بنية k_thread *thread)
{
	أرجع thread->ډېری_info.usage.ناکارول_شوی_پوله;
}

#که_تعریف_شوی CONFIG_USERSPACE
صحيح z_vrfy_k_thread_runtime_ډېری_ناکارول_شوی_پوله_pct_سېټ(بنية k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ډېری_ناکارول_شوی_پوله_pct_سېټ(thread, pct);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

صحيح z_vrfy_k_thread_runtime_ډېری_ناکارول_شوی_پوله_سېټ(بنية k_thread *thread,
						       size_t پوله)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ډېری_ناکارول_شوی_پوله_سېټ(thread, پوله);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ډېری_ناکارول_شوی_پوله_get(بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_ډېری_ناکارول_شوی_پوله_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
#د_که_پای /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(بنية k_thread *new_thread,
			      k_thread_ډېری_t *ډېری,
			      size_t ډېری_اندازه, k_thread_ثبت_t ثبت,
			      فراغ *p1, فراغ *p2, فراغ *p3,
			      صحيح prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_دننه_isr(), "Threads may not be created in ISRs");

	z_تیاری_new_thread(new_thread, ډېری, ډېری_اندازه, ثبت, p1, p2, p3,
			  prio, options, عدم);

	thread_مهالوېش_new(new_thread, delay);

	أرجع new_thread;
}

#که_تعریف_شوی CONFIG_USERSPACE
bool z_ډېری_is_user_capable(k_thread_ډېری_t *ډېری)
{
	أرجع k_object_پیدا_کړه(ډېری) != عدم;
}

k_tid_t z_vrfy_k_thread_create(بنية k_thread *new_thread,
			       k_thread_ډېری_t *ډېری,
			       size_t ډېری_اندازه, k_thread_ثبت_t ثبت,
			       فراغ *p1, فراغ *p2, فراغ *p3,
			       صحيح prio, uint32_t options, k_timeout_t delay)
{
	size_t ټول_اندازه, ډېری_obj_اندازه;
	بنية k_object *ډېری_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ډېری_object = k_object_پیدا_کړه(ډېری);
	K_OOPS(K_SYSCALL_VERIFY_پیغام(k_object_validation_check(ډېری_object, ډېری,
						K_OBJ_THREAD_ډېری_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_پیغام(!اندازه_جمع_کړه_ډکېدنه(K_THREAD_ډېری_RESERVED,
						       ډېری_اندازه, &ټول_اندازه),
				    "stack size overflow (%zu+%zu)",
				    ډېری_اندازه,
				    K_THREAD_ډېری_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#که_تعریف_شوی CONFIG_GEN_PRIV_STACKS
	ډېری_obj_اندازه = ډېری_object->معلومات.ډېری_معلومات->اندازه;
#وإلا
	ډېری_obj_اندازه = ډېری_object->معلومات.ډېری_اندازه;
#د_که_پای /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_پیغام(ټول_اندازه <= ډېری_obj_اندازه,
				    "stack size %zu is too big, max is %zu",
				    ټول_اندازه, ډېری_obj_اندازه));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, عدم)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_ښکتنی_or_equal(prio,
							_current->بنسټ.prio)));

	z_تیاری_new_thread(new_thread, ډېری, ډېری_اندازه,
			   ثبت, p1, p2, p3, prio, options, عدم);

	thread_مهالوېش_new(new_thread, delay);

	أرجع new_thread;
}
#ضمن <zephyr/syscalls/k_thread_create_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ z_init_thread_بنسټ(بنية thread_بنسټ *thread_بنسټ_2, صحيح لومړیتوب,
		       uint32_t initial_state, بینښه صحيح options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_بنسټ_2->pended_on = عدم;
	thread_بنسټ_2->user_options = (uint16_t)options;
	thread_بنسټ_2->thread_state = (uint8_t)initial_state;

	thread_بنسټ_2->prio = لومړیتوب;

	thread_بنسټ_2->sched_locked = 0U;

#که_تعریف_شوی CONFIG_SMP
	thread_بنسټ_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_بنسټ_2->cpu = 0;
#د_که_پای /* CONFIG_SMP */

#که_تعریف_شوی CONFIG_TIMESLICE_فی_THREAD
	thread_بنسټ_2->ټوټه_ticks = 0;
	thread_بنسټ_2->ټوټه_expired = عدم;
#د_که_پای /* CONFIG_TIMESLICE_PER_THREAD */

#که_تعریف_شوی CONFIG_SPIN_VALIDATE
	thread_بنسټ_2->بدلون_معلومات = عدم;
	/* otherwise swap_data does not need to be initialized */
#د_که_پای

	z_init_thread_timeout(thread_بنسټ_2);
}

FUNC_NORETURN فراغ k_thread_user_ډېرپېښ_enter(k_thread_ثبت_t ثبت,
					    فراغ *p1, فراغ *p2, فراغ *p3)
{
	SYS_پورټ_TRACING_FUNC(k_thread, user_ډېرپېښ_enter);

	_current->بنسټ.user_options |= K_USER;
	z_thread_essential_clear(_current);
#که_تعریف_شوی CONFIG_THREAD_MONITOR
	_current->ثبت.p_ثبت = ثبت;
	_current->ثبت.parameter1 = p1;
	_current->ثبت.parameter2 = p2;
	_current->ثبت.parameter3 = p3;
#د_که_پای /* CONFIG_THREAD_MONITOR */
#که_تعریف_شوی CONFIG_USERSPACE
	__ASSERT(z_ډېری_is_user_capable(_current->ډېری_obj),
		 "dropping to user mode with kernel-only stack object");
#که_تعریف_شوی CONFIG_THREAD_USERSPACE_LOCAL_معلومات
	memset(_current->userspace_local_معلومات, 0,
	       حجم(بنية thread_userspace_local_معلومات));
#د_که_پای /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#که_تعریف_شوی CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ډېری_تیاری(_current,
			     (حرف *)(_current->ډېری_info.پیل +
				      _current->ډېری_info.اندازه));
#د_که_پای /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_ډېرپېښ_enter(ثبت, p1, p2, p3);
#وإلا
	/* XXX In this case we do not reset the stack */
	z_thread_ثبت(ثبت, p1, p2, p3);
#د_که_پای /* CONFIG_USERSPACE */
}

#إذا defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ډېری_INFO)
#که_تعریف_شوی CONFIG_ډېری_GROWS_پورته
#تېروتنه "Unsupported configuration for stack analysis"
#د_که_پای /* CONFIG_STACK_GROWS_UP */

صحيح z_ډېری_space_get(ثابت uint8_t *ډېری_پیل, size_t اندازه, size_t *ناکارول_شوی_ptr)
{
	size_t ناکارول_شوی = 0;
	ثابت uint8_t *checked_ډېری = ډېری_پیل;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ثابت uint8_t *ډېری_اشاره_ګر = (ثابت uint8_t *)&ډېری_پیل;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	إذا ((ډېری_اشاره_ګر > ډېری_پیل) && (ډېری_اشاره_ګر <= (ډېری_پیل + اندازه)) &&
	    IS_ENABLED(CONFIG_NO_ناکارول_شوی_ډېری_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		أرجع -ENOTSUP;
	}

	إذا (IS_ENABLED(CONFIG_ډېری_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ډېری += 4;
		اندازه -= 4;
	}

	لكل (size_t i = 0; i < اندازه; i++) {
		إذا ((checked_ډېری[i]) == 0xaaU) {
			ناکارول_شوی++;
		} وإلا {
			اكسر;
		}
	}

	*ناکارول_شوی_ptr = ناکارول_شوی;

	أرجع 0;
}

#که_تعریف_شوی CONFIG_THREAD_RUNTIME_ډېری_SAFETY
صحيح k_thread_runtime_ډېری_safety_ډک_check(ثابت بنية k_thread *thread,
					     size_t *ناکارول_شوی_ptr,
					     k_thread_ډېری_safety_handler_t handler,
					     فراغ *arg)
{
	صحيح    rv;
	size_t ناکارول_شوی_space;

	__ASSERT_NO_MSG(thread != عدم);

	rv = z_ډېری_space_get((ثابت uint8_t *)thread->ډېری_info.پیل,
			       thread->ډېری_info.اندازه, &ناکارول_شوی_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (ناکارول_شوی_ptr != عدم) {
		*ناکارول_شوی_ptr = ناکارول_شوی_space;
	}

	إذا ((ناکارول_شوی_space < thread->ډېری_info.usage.ناکارول_شوی_پوله) &&
	    (handler != عدم)) {
		handler(thread, ناکارول_شوی_space, arg);
	}

	أرجع 0;
}

صحيح k_thread_runtime_ډېری_safety_پوله_check(ثابت بنية k_thread *thread,
						  size_t *ناکارول_شوی_ptr,
						  k_thread_ډېری_safety_handler_t handler,
						  فراغ *arg)
{
	صحيح    rv;
	size_t ناکارول_شوی_space;
	size_t پوله;
	size_t لټون_اندازه;

	__ASSERT_NO_MSG(thread != عدم);

	پوله = thread->ډېری_info.usage.ناکارول_شوی_پوله;
	لټون_اندازه = پوله;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	إذا (IS_ENABLED(CONFIG_ډېری_SENTINEL) && (پوله != 0U)) {
		لټون_اندازه += حجم(uint32_t);
	}

	rv = z_ډېری_space_get((ثابت uint8_t *)thread->ډېری_info.پیل,
			       لټون_اندازه, &ناکارول_شوی_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (ناکارول_شوی_ptr != عدم) {
		*ناکارول_شوی_ptr = ناکارول_شوی_space;
	}

	إذا ((ناکارول_شوی_space < پوله) && (handler != عدم)) {
		handler(thread, ناکارول_شوی_space, arg);
	}

	أرجع 0;
}
#د_که_پای /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

صحيح z_impl_k_thread_ډېری_space_get(ثابت بنية k_thread *thread,
				    size_t *ناکارول_شوی_ptr)
{
#که_تعریف_شوی CONFIG_THREAD_ډېری_MEM_MAPPED
	إذا (thread->ډېری_info.mapped.addr == عدم) {
		أرجع -EINVAL;
	}
#د_که_پای /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع z_ډېری_space_get((ثابت uint8_t *)thread->ډېری_info.پیل,
				 thread->ډېری_info.اندازه, ناکارول_شوی_ptr);
}

#که_تعریف_شوی CONFIG_USERSPACE
صحيح z_vrfy_k_thread_ډېری_space_get(ثابت بنية k_thread *thread,
				    size_t *ناکارول_شوی_ptr)
{
	size_t ناکارول_شوی;
	صحيح ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ډېری_space_get(thread, &ناکارول_شوی);
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	ret = k_usermode_منزل_copy(ناکارول_شوی_ptr, &ناکارول_شوی, حجم(size_t));
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	أرجع 0;
}
#ضمن <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
#د_که_پای /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_پاتې_شوی_ticks(
						    ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_پاتې_شوی_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_expires_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

#که_تعریف_شوی CONFIG_INSTRUMENT_THREAD_SWITCHING
فراغ z_thread_mark_switched_دننه(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_پیل(_current);
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#که_تعریف_شوی CONFIG_TRACING
	SYS_پورټ_TRACING_FUNC(k_thread, switched_دننه);
#د_که_پای /* CONFIG_TRACING */
}

فراغ z_thread_mark_switched_بهر(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#د_که_پای /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#که_تعریف_شوی CONFIG_TRACING
#که_تعریف_شوی CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	إذا (!_current ||
	    (_current->بنسټ.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}
#د_که_پای /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_پورټ_TRACING_FUNC(k_thread, switched_بهر);
#د_که_پای /* CONFIG_TRACING */
}
#د_که_پای /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

صحيح k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	إذا ((thread == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#وإلا
	*stats = (k_thread_runtime_stats_t) {};
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE */

	أرجع 0;
}

صحيح k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ALL */

	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	بینښه صحيح عدد_cpus = arch_عدد_cpus();

	لكل (بینښه صحيح i = 0; i < عدد_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_دورې += tmp_stats.execution_دورې;
		stats->ټول_دورې     += tmp_stats.ټول_دورې;
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_دورې   += tmp_stats.current_دورې;
		stats->peak_دورې      += tmp_stats.peak_دورې;
		stats->اوسط_دورې   += tmp_stats.اوسط_دورې;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_دورې      += tmp_stats.idle_دورې;
	}
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ALL */

	أرجع 0;
}

صحيح k_thread_runtime_stats_cpu_get(صحيح cpu, k_thread_runtime_stats_t *stats)
{
	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((بینښه صحيح)cpu >= arch_عدد_cpus())) {
		أرجع -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#وإلا
	ARG_ناکارول_شوی(cpu);
#د_که_پای

	أرجع 0;
}

#که_تعریف_شوی CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ساكن بنية k_thread *thread_منزل_cleanup;

/** Spinlock for thread abort cleanup. */
ساكن بنية k_spinlock thread_cleanup_قلف;

#که_تعریف_شوی CONFIG_THREAD_ډېری_MEM_MAPPED
ساكن فراغ *thread_cleanup_ډېری_addr;
ساكن size_t thread_cleanup_ډېری_sz;

ساكن فراغ unmap_thread_ډېری(فراغ *addr, size_t sz)
{
	إذا (addr != عدم) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#د_که_پای /* CONFIG_THREAD_STACK_MEM_MAPPED */

فراغ defer_thread_cleanup(بنية k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_منزل_cleanup = thread;

#که_تعریف_شوی CONFIG_THREAD_ډېری_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ډېری_addr = thread->ډېری_info.mapped.addr;
	thread_cleanup_ډېری_sz = thread->ډېری_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ډېری_info.mapped.addr = عدم;
	thread->ډېری_info.mapped.sz = 0;
#د_که_پای /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_deferred_thread_cleanup(فراغ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#که_تعریف_شوی CONFIG_THREAD_ډېری_MEM_MAPPED
	unmap_thread_ډېری(thread_cleanup_ډېری_addr, thread_cleanup_ډېری_sz);

	thread_cleanup_ډېری_addr = عدم;
	thread_cleanup_ډېری_sz = 0;
#د_که_پای /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_thread_cleanup(بنية k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#که_تعریف_شوی CONFIG_THREAD_ډېری_MEM_MAPPED
	unmap_thread_ډېری(thread->ډېری_info.mapped.addr,
			   thread->ډېری_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ډېری_info.mapped.addr = عدم;
	thread->ډېری_info.mapped.sz = 0;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_ناکارول_شوی(thread);
#د_که_پای /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ k_thread_abort_cleanup(بنية k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_قلف) {
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
	K_SPINLOCK(&thread_cleanup_قلف) {
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

#د_که_پای /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

فراغ z_dummy_thread_init(بنية k_thread *dummy_thread)
{
	dummy_thread->بنسټ.thread_state = _THREAD_DUMMY;
#که_تعریف_شوی CONFIG_SCHED_CPU_ماسک
	dummy_thread->بنسټ.cpu_ماسک = -1;
#د_که_پای /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->بنسټ.user_options = K_ESSENTIAL;
#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = عدم;
	dummy_thread->orig_prio = dummy_thread->بنسټ.prio;
#د_که_پای /* Z_MUTEX_PI_ENABLED */
#که_تعریف_شوی CONFIG_THREAD_ډېری_INFO
	dummy_thread->ډېری_info.پیل = 0U;
	dummy_thread->ډېری_info.اندازه = 0U;
#د_که_پای /* CONFIG_THREAD_STACK_INFO */
#که_تعریف_شوی CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#د_که_پای /* CONFIG_USERSPACE */
#إذا (K_ډېرۍ_MEM_POOL_اندازه > 0)
	k_thread_system_pool_ټاکل(dummy_thread);
#وإلا
	dummy_thread->resource_pool = عدم;
#د_که_پای /* K_HEAP_MEM_POOL_SIZE */

#که_تعریف_شوی CONFIG_USE_SWITCH
	dummy_thread->switch_handle = عدم;
#د_که_پای /* CONFIG_USE_SWITCH */

#که_تعریف_شوی CONFIG_TIMESLICE_فی_THREAD
	dummy_thread->بنسټ.ټوټه_ticks = 0;
#د_که_پای /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_سېټ(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

فراغ z_impl_k_thread_لومړیتوب_سېټ(k_tid_t thread, صحيح prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, عدم);

	bool need_sched = z_thread_prio_سېټ((بنية k_thread *)thread, prio);

	إذا ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->بنسټ.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_لومړیتوب_سېټ(k_tid_t thread, صحيح prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_پیغام(_is_valid_prio(prio, عدم),
				    "invalid thread priority %d", prio));
#که_نه_تعریف_شوی CONFIG_USERSPACE_THREAD_MAY_RAISE_لومړیتوب
	K_OOPS(K_SYSCALL_VERIFY_پیغام((int8_t)prio >= thread->بنسټ.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->بنسټ.prio));
#د_که_پای /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_لومړیتوب_سېټ(thread, prio);
}
#ضمن <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	إذا (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_دننه_isr())) {
		z_thread_suspend_current(thread);
		أرجع;
	}

	k_spinlock_کلی_t کلی = z_sched_spinlock_قلف();

	إذا (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(کلی);
		أرجع;
	}

	z_thread_halt(thread, کلی, false);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ضمن <zephyr/syscalls/k_thread_suspend_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_کلی_t کلی = z_sched_spinlock_قلف();

	/* Do not try to resume a thread that was not suspended */
	إذا (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(کلی);
		أرجع;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ضمن <zephyr/syscalls/k_thread_resume_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_پورټ_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_کلی_t کلی = z_sched_spinlock_قلف();

	إذا (z_is_thread_sleeping(thread)) {
		(فراغ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(کلی);
	} وإلا {
		z_sched_spinlock_unlock(کلی);
	}
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ضمن <zephyr/syscalls/k_wakeup_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ z_thread_abort(بنية k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_کلی_t کلی = z_sched_spinlock_قلف();

	إذا (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(کلی);
		أرجع;
	}

	z_thread_halt(thread, کلی, true);

	إذا (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#إذا !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
فراغ z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#د_که_پای /* !CONFIG_ARCH_HAS_THREAD_ABORT */

صحيح z_impl_k_thread_join(بنية k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_کلی_t کلی = z_sched_spinlock_قلف();
	صحيح ret;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	إذا (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} وإلا إذا ((thread == _current) ||
		   (thread->بنسټ.pended_on == &_current->join_کتار)) {
		ret = -EDEADLK;
	} وإلا {
		__ASSERT(!arch_is_دننه_isr(), "cannot join in ISR");
		z_sched_جمع_کړه_منزل_waitq_locked(_current, &thread->join_کتار);
		z_جمع_کړه_thread_timeout(_current, timeout);

		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_بدلون_locked(کلی);
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		أرجع ret;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(کلی);
	أرجع ret;
}

#که_تعریف_شوی CONFIG_USERSPACE
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
	بنية k_object *ko = k_object_پیدا_کړه(thread);
	صحيح ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	بدل (ret) {
	حالة 0:
		أرجع false;
	حالة -EINVAL:
		أرجع true;
	افتراضي:
#که_تعریف_شوی CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#د_که_پای /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_پیغام(ret == 0, "access denied"));
	}
	کوډ_UNREACHABLE; /* LCOV_EXCL_LINE */
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
#د_که_پای /* CONFIG_USERSPACE */

bool k_can_yield(فراغ)
{
	أرجع !(k_is_pre_kernel() || k_is_دننه_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

فراغ z_impl_k_yield(فراغ)
{
	__ASSERT(!arch_is_دننه_isr(), "");

	SYS_پورټ_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_yield(فراغ)
{
	z_impl_k_yield();
}
#ضمن <zephyr/syscalls/k_yield_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
