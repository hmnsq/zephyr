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

#اگر_تعریف_شده CONFIG_OBJ_CORE_THREAD
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_THREAD
ساكن بنية k_obj_core_stats_desc  thread_stats_desc = {
	.raw_اندازه = حجم(بنية k_چرخه_stats),
	.query_اندازه = حجم(بنية k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#وإلا
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    عدم);
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_THREAD */
#پایان_اگر /* CONFIG_OBJ_CORE_THREAD */


#تعریف_کن _FOREACH_STATIC_THREAD(thread_داده)              \
	STRUCT_SECTION_FOREACH(static_thread_داده, thread_داده)

bool k_is_درون_isr(فراغ)
{
	أرجع arch_is_درون_isr();
}
EXPORT_نماد(k_is_درون_isr);

#اگر_تعریف_شده CONFIG_THREAD_CUSTOM_داده
فراغ z_impl_k_thread_custom_داده_مجموعه(فراغ *مقدار)
{
	_current->custom_داده = مقدار;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_custom_داده_مجموعه(فراغ *داده)
{
	z_impl_k_thread_custom_داده_مجموعه(داده);
}
#ضمن <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ *z_impl_k_thread_custom_داده_get(فراغ)
{
	أرجع _current->custom_داده;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_thread_custom_داده_get(فراغ)
{
	أرجع z_impl_k_thread_custom_داده_get();
}
#ضمن <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#پایان_اگر /* CONFIG_USERSPACE */
#پایان_اگر /* CONFIG_THREAD_CUSTOM_DATA */

صحيح z_impl_k_is_preempt_thread(فراغ)
{
	أرجع !arch_is_درون_isr() && thread_is_preemptible(_current);
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_is_preempt_thread(فراغ)
{
	أرجع z_impl_k_is_preempt_thread();
}
#ضمن <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_اولویت_get(k_tid_t thread)
{
	أرجع thread->پایه.prio;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_اولویت_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_اولویت_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

#اگر_تعریف_شده CONFIG_THREAD_NAME
ساكن فراغ مجموعه_thread_name(بنية k_thread *thread, ثابت حرف *str)
{
	إذا (str != عدم) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#اگر_تعریف_شده CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_مجموعه(thread, str);
#پایان_اگر /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} وإلا {
		thread->name[0] = '\0';
	}
}
#پایان_اگر /* CONFIG_THREAD_NAME */

صحيح z_impl_k_thread_name_مجموعه(k_tid_t thread, ثابت حرف *str)
{
#اگر_تعریف_شده CONFIG_THREAD_NAME
	إذا (thread == عدم) {
		thread = _current;
	}

	مجموعه_thread_name(thread, str);

	SYS_پورت_TRACING_OBJ_FUNC(k_thread, name_مجموعه, thread, 0);

	أرجع 0;
#وإلا
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(str);

	SYS_پورت_TRACING_OBJ_FUNC(k_thread, name_مجموعه, thread, -ENOSYS);

	أرجع -ENOSYS;
#پایان_اگر /* CONFIG_THREAD_NAME */
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_مجموعه(k_tid_t thread, ثابت حرف *str)
{
#اگر_تعریف_شده CONFIG_THREAD_NAME
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

	أرجع z_impl_k_thread_name_مجموعه(thread, name);
#وإلا
	أرجع -ENOSYS;
#پایان_اگر /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_set_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

ثابت حرف *k_thread_name_get(k_tid_t thread)
{
#اگر_تعریف_شده CONFIG_THREAD_NAME
	أرجع (ثابت حرف *)thread->name;
#وإلا
	ARG_استفادهنشده(thread);
	أرجع عدم;
#پایان_اگر /* CONFIG_THREAD_NAME */
}

صحيح z_impl_k_thread_name_copy(k_tid_t thread, حرف *buf, size_t اندازه)
{
#اگر_تعریف_شده CONFIG_THREAD_NAME
	strncpy(buf, thread->name, اندازه);
	أرجع 0;
#وإلا
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(buf);
	ARG_استفادهنشده(اندازه);
	أرجع -ENOSYS;
#پایان_اگر /* CONFIG_THREAD_NAME */
}

ساكن size_t copy_بایتها(حرف *dest, size_t dest_اندازه, ثابت حرف *src, size_t src_اندازه)
{
	size_t  بایتها_مقصد_copy;

	بایتها_مقصد_copy = min(dest_اندازه, src_اندازه);
	کپی_حافظه(dest, src, بایتها_مقصد_copy);

	أرجع بایتها_مقصد_copy;
}

ثابت حرف *k_thread_state_str(k_tid_t thread_id, حرف *buf, size_t buf_اندازه)
{
	size_t      off = 0;
	uint8_t     بیت_2;
	uint8_t     thread_state = thread_id->پایه.thread_state;
#تعریف_کن SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, حجم(Z_STATE_STR_##s) - 1 }
	ساكن ثابت بنية {
		ثابت حرف *str;
		uint16_t    بیت_2;
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
#لغو_تعریف SS_ENT

	إذا ((buf == عدم) || (buf_اندازه == 0)) {
		أرجع "";
	}

	buf_اندازه--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	لكل (بیعلامت صحيح شاخص = 0; thread_state != 0; شاخص++) {
		بیت_2 = state_string[شاخص].بیت_2;
		إذا ((thread_state & بیت_2) == 0) {
			استمر;
		}

		off += copy_بایتها(buf + off, buf_اندازه - off,
				  state_string[شاخص].str,
				  state_string[شاخص].len);

		thread_state &= ~بیت_2;

		إذا (thread_state != 0) {
			off += copy_بایتها(buf + off, buf_اندازه - off, "+", 1);
		}
	}

	buf[off] = '\0';

	أرجع (ثابت حرف *)buf;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_name_copy(k_tid_t thread,
					    حرف *buf, size_t اندازه)
{
#اگر_تعریف_شده CONFIG_THREAD_NAME
	size_t len;
	بنية k_object *ko = k_object_پیدا_کن(thread);

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
	len = طول_رشته(thread->name);
	إذا ((len + 1) > اندازه) {
		أرجع -ENOSPC;
	}

	أرجع k_usermode_مقصد_copy((فراغ *)buf, thread->name, len + 1);
#وإلا
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(buf);
	ARG_استفادهنشده(اندازه);
	أرجع -ENOSYS;
#پایان_اگر /* CONFIG_THREAD_NAME */
}
#ضمن <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

#اگر_تعریف_شده CONFIG_پشته_SENTINEL
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
فراغ z_check_پشته_sentinel(فراغ)
{
	uint32_t *پشته;

	إذا ((_current->پایه.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}

	پشته = (uint32_t *)_current->پشته_info.شروع;
	إذا (*پشته != پشته_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*پشته = پشته_SENTINEL;
		z_except_reason(K_ERR_پشته_CHK_FAIL);
	}
}
#پایان_اگر /* CONFIG_STACK_SENTINEL */

#إذا defined(CONFIG_پشته_اشاره_گر_RANDOM) && (CONFIG_پشته_اشاره_گر_RANDOM != 0)
صحيح z_پشته_adjust_initialized;

ساكن size_t random_جابه_جایی(size_t پشته_اندازه)
{
	size_t random_val;

	إذا (!z_پشته_adjust_initialized) {
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
	ثابت size_t fuzz = random_val % CONFIG_پشته_اشاره_گر_RANDOM;

	إذا (unlikely(fuzz * 2 > پشته_اندازه)) {
		أرجع 0;
	}

	أرجع fuzz;
}
#إذا defined(CONFIG_پشته_GROWS_به_بالا)
	/* This is so rare not bothering for now */
#خطا "Stack pointer randomization not implemented for upward growing stacks"
#پایان_اگر /* CONFIG_STACK_GROWS_UP */
#پایان_اگر /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
بنية پشته_geometry {
	size_t obj_اندازه;	/* size of the whole stack object */
	حرف *buf_شروع;	/* start of the usable stack buffer */
	size_t buf_اندازه;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ساكن بنية پشته_geometry compute_پشته_geometry(k_thread_پشته_t *پشته,
						    size_t پشته_اندازه)
{
	بنية پشته_geometry geo;

#اگر_تعریف_شده CONFIG_USERSPACE
	إذا (z_پشته_is_user_capable(پشته)) {
		geo.obj_اندازه = K_THREAD_پشته_LEN(پشته_اندازه);
		geo.buf_شروع = K_THREAD_پشته_میانگیر(پشته);
		geo.buf_اندازه = geo.obj_اندازه - K_THREAD_پشته_RESERVED;
	} وإلا
#پایان_اگر /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_اندازه = K_KERNEL_پشته_LEN(پشته_اندازه);
		geo.buf_شروع = K_KERNEL_پشته_میانگیر(پشته);
		geo.buf_اندازه = geo.obj_اندازه - K_KERNEL_پشته_RESERVED;

#إذا defined(ARCH_KERNEL_پشته_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		إذا (K_KERNEL_پشته_RESERVED > geo.obj_اندازه) {
			k_panic();
		}
#پایان_اگر
	}

	أرجع geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ساكن حرف *map_thread_پشته(بنية k_thread *new_thread,
			      k_thread_پشته_t *پشته,
			      بنية پشته_geometry *geo)
{
	حرف *پشته_ptr;

#اگر_تعریف_شده CONFIG_THREAD_پشته_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	فراغ *پشته_mapped = k_mem_map_phys_guard((uintptr_t)پشته, geo->obj_اندازه,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)پشته_mapped != 0);

#اگر_تعریف_شده CONFIG_USERSPACE
	إذا (z_پشته_is_user_capable(پشته)) {
		geo->buf_شروع = K_THREAD_پشته_میانگیر(پشته_mapped);
	} وإلا
#پایان_اگر /* CONFIG_USERSPACE */
	{
		geo->buf_شروع = K_KERNEL_پشته_میانگیر(پشته_mapped);
	}

	پشته_ptr = (حرف *)پشته_mapped + geo->obj_اندازه;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->پشته_info.mapped.addr = پشته_mapped;
	new_thread->پشته_info.mapped.sz = geo->obj_اندازه;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_استفادهنشده(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	پشته_ptr = (حرف *)پشته + geo->obj_اندازه;
#پایان_اگر /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع پشته_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ساكن مضمن فراغ fill_init_پشته(حرف *buf_شروع, size_t buf_اندازه)
{
#اگر_تعریف_شده CONFIG_INIT_STACKS
	memset(buf_شروع, 0xaa, buf_اندازه);
#وإلا
	ARG_استفادهنشده(buf_شروع);
	ARG_استفادهنشده(buf_اندازه);
#پایان_اگر /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ساكن مضمن فراغ مجموعه_پشته_sentinel(حرف *buf_شروع)
{
#اگر_تعریف_شده CONFIG_پشته_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_شروع) = پشته_SENTINEL;
#وإلا
	ARG_استفادهنشده(buf_شروع);
#پایان_اگر /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ساكن size_t reserve_پشته_headroom(بنية k_thread *new_thread,
				     حرف *پشته_ptr, size_t buf_اندازه)
{
	size_t delta = 0;

	ARG_استفادهنشده(new_thread);
	ARG_استفادهنشده(پشته_ptr);
	ARG_استفادهنشده(buf_اندازه);

#اگر_تعریف_شده CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_پشته_راهاندازی(new_thread, پشته_ptr);
#پایان_اگر /* CONFIG_THREAD_LOCAL_STORAGE */
#اگر_تعریف_شده CONFIG_THREAD_USERSPACE_LOCAL_داده
	size_t tls_اندازه = حجم(بنية thread_userspace_local_داده);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_اندازه;
	new_thread->userspace_local_داده =
		(بنية thread_userspace_local_داده *)(پشته_ptr - delta);
#پایان_اگر /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#إذا defined(CONFIG_پشته_اشاره_گر_RANDOM) && (CONFIG_پشته_اشاره_گر_RANDOM != 0)
	delta += random_جابه_جایی(buf_اندازه);
#پایان_اگر /* CONFIG_STACK_POINTER_RANDOM */

	أرجع دور_به_بالا(delta, ARCH_پشته_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ساكن مضمن فراغ مجموعه_پشته_info(بنية k_thread *new_thread, حرف *buf_شروع,
				  size_t buf_اندازه, size_t delta)
{
#اگر_تعریف_شده CONFIG_THREAD_پشته_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->پشته_info.شروع = (uintptr_t)buf_شروع;
	new_thread->پشته_info.اندازه = buf_اندازه;
	new_thread->پشته_info.delta = delta;

#اگر_تعریف_شده CONFIG_THREAD_RUNTIME_پشته_SAFETY
	new_thread->پشته_info.usage.استفادهنشده_آستانه =
		(CONFIG_THREAD_RUNTIME_پشته_SAFETY_DEFAULT_استفادهنشده_آستانه_PCT *
		 buf_اندازه) / 100;
#پایان_اگر
#وإلا
	ARG_استفادهنشده(new_thread);
	ARG_استفادهنشده(buf_شروع);
	ARG_استفادهنشده(buf_اندازه);
	ARG_استفادهنشده(delta);
#پایان_اگر /* CONFIG_THREAD_STACK_INFO */
}

ساكن حرف *راهاندازی_thread_پشته(بنية k_thread *new_thread,
				k_thread_پشته_t *پشته, size_t پشته_اندازه)
{
	بنية پشته_geometry geo = compute_پشته_geometry(پشته, پشته_اندازه);
	حرف *پشته_ptr = map_thread_پشته(new_thread, پشته, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		پشته, new_thread, geo.obj_اندازه, (فراغ *)geo.buf_شروع,
		geo.buf_اندازه, (فراغ *)پشته_ptr);

	fill_init_پشته(geo.buf_شروع, geo.buf_اندازه);
	مجموعه_پشته_sentinel(geo.buf_شروع);
	delta = reserve_پشته_headroom(new_thread, پشته_ptr, geo.buf_اندازه);
	مجموعه_پشته_info(new_thread, geo.buf_شروع, geo.buf_اندازه, delta);

	أرجع پشته_ptr - delta;
}

#اگر_تعریف_شده CONFIG_HW_SHADOW_پشته
ساكن فراغ راهاندازی_shadow_پشته(بنية k_thread *new_thread,
			 k_thread_پشته_t *پشته)
{
	صحيح ret = -ENOENT;

	STRUCT_SECTION_FOREACH(پشته_مقصد_hw_shadow_پشته, stk_مقصد_hw_shstk) {
		إذا (stk_مقصد_hw_shstk->پشته == پشته) {
			ret = k_thread_hw_shadow_پشته_attach(new_thread,
							      stk_مقصد_hw_shstk->shstk_addr,
							      stk_مقصد_hw_shstk->اندازه);
			إذا (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_مقصد_hw_shstk->shstk_addr, ret);
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

	STRUCT_SECTION_FOREACH(پشته_مقصد_hw_shadow_پشته_arr, stk_مقصد_hw_shstk) {
		إذا ((uintptr_t)پشته >= stk_مقصد_hw_shstk->پشته_addr &&
		    (uintptr_t)پشته < stk_مقصد_hw_shstk->پشته_addr +
		    stk_مقصد_hw_shstk->پشته_اندازه * stk_مقصد_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t پشته_جابه_جایی = (uintptr_t)پشته - stk_مقصد_hw_shstk->پشته_addr;
			uintptr_t پشته_شاخص = پشته_جابه_جایی / stk_مقصد_hw_shstk->پشته_اندازه;
			uintptr_t addr;

			إذا (پشته_شاخص >= stk_مقصد_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, پشته);
				k_panic();
			}

			addr = stk_مقصد_hw_shstk->shstk_addr +
				stk_مقصد_hw_shstk->shstk_اندازه * پشته_شاخص;
			ret = k_thread_hw_shadow_پشته_attach(new_thread,
							      (arch_thread_hw_shadow_پشته_t *)addr,
							      stk_مقصد_hw_shstk->shstk_اندازه);
			إذا (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_مقصد_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			اكسر;
		}
	}

	إذا (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, پشته);
		k_panic();
	}
}
#وإلا
/* No-op stub used when hardware shadow stacks are not configured. */
ساكن مضمن فراغ راهاندازی_shadow_پشته(بنية k_thread *new_thread,
				      k_thread_پشته_t *پشته)
{
	ARG_استفادهنشده(new_thread);
	ARG_استفادهنشده(پشته);
}
#پایان_اگر

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
#اگر_تعریف_شده CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ساكن مضمن فراغ init_thread_obj_core(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_ثبات(K_OBJ_CORE(thread), &thread->پایه.usage,
				  حجم(thread->پایه.usage));
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_THREAD */
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ساكن مضمن فراغ init_thread_userspace(بنية k_thread *thread,
					 k_thread_پشته_t *پشته,
					 uint32_t options)
{
#اگر_تعریف_شده CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_پشته_is_user_capable(پشته),
		 "user thread %p with kernel-only stack %p", thread, پشته);
	k_object_init(thread);
	k_object_init(پشته);
	thread->پشته_obj = پشته;
	thread->syscall_قاب = عدم;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#وإلا
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(پشته);
	ARG_استفادهنشده(options);
#پایان_اگر /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ساكن مضمن فراغ assert_thread_coherence(بنية k_thread *thread,
					   k_thread_پشته_t *پشته)
{
#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#اگر_تعریف_نشده CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(پشته));
#پایان_اگر /* CONFIG_DYNAMIC_THREAD */
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(پشته);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ساكن مضمن فراغ assert_switch_handle(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != عدم,
		 "arch layer failed to initialize switch_handle");
#پایان_اگر /* CONFIG_USE_SWITCH */
	ARG_استفادهنشده(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ساكن مضمن فراغ init_thread_custom_داده(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_THREAD_CUSTOM_داده
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_داده = عدم;
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ساكن مضمن فراغ جمع_کن_thread_مقصد_monitor(بنية k_thread *thread,
					 k_thread_مدخل_t مدخل,
					 فراغ *p1, فراغ *p2, فراغ *p3)
{
#اگر_تعریف_شده CONFIG_THREAD_MONITOR
	thread->مدخل.p_مدخل = مدخل;
	thread->مدخل.parameter1 = p1;
	thread->مدخل.parameter2 = p2;
	thread->مدخل.parameter3 = p3;

	k_spinlock_کلید_t کلید = k_spin_قفل(&z_thread_monitor_قفل);

	thread->بعدی_thread = _kernel.نخها;
	_kernel.نخها = thread;
	k_spin_unlock(&z_thread_monitor_قفل, کلید);
#وإلا
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(مدخل);
	ARG_استفادهنشده(p1);
	ARG_استفادهنشده(p2);
	ARG_استفادهنشده(p3);
#پایان_اگر /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ساكن مضمن فراغ init_thread_name(بنية k_thread *thread, ثابت حرف *name)
{
#اگر_تعریف_شده CONFIG_THREAD_NAME
	مجموعه_thread_name(thread, name);
#وإلا
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(name);
#پایان_اگر /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ساكن مضمن فراغ init_thread_cpu_ماسک(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک
	إذا (IS_ENABLED(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)) {
		thread->پایه.cpu_ماسک = 1; /* must specify only one cpu */
	} وإلا {
		thread->پایه.cpu_ماسک = -1; /* allow all cpus */
	}
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ساكن مضمن فراغ init_thread_userspace_perms(بنية k_thread *thread,
					       uint32_t options)
{
#اگر_تعریف_شده CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	إذا ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#وإلا
	ARG_استفادهنشده(thread);
	ARG_استفادهنشده(options);
#پایان_اگر /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ساكن مضمن فراغ init_thread_deadline(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_SCHED_DEADLINE
	thread->پایه.prio_deadline = 0;
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ساكن مضمن فراغ init_thread_halt_صف(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_SMP
	z_waitq_init(&thread->halt_صف);
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ساكن مضمن فراغ init_thread_usage(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_SCHED_THREAD_USAGE
	thread->پایه.usage = (بنية k_چرخه_stats) {};
	thread->پایه.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ساكن مضمن فراغ assert_thread_not_reused(بنية k_thread *new_thread)
{
#اگر_تعریف_شده CONFIG_THREAD_MONITOR
#اگر_تعریف_شده CONFIG_ASSERT
	k_spinlock_کلید_t کلید = k_spin_قفل(&z_thread_monitor_قفل);
	bool reused = false;

	/* Check if the thread is already in the list */
	لكل (بنية k_thread *t = _kernel.نخها; t; t = t->بعدی_thread) {
		إذا (t == new_thread) {
			reused = true;
			اكسر;
		}
	}

	k_spin_unlock(&z_thread_monitor_قفل, کلید);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#پایان_اگر /* CONFIG_ASSERT */
#پایان_اگر /* CONFIG_THREAD_MONITOR */
	ARG_استفادهنشده(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
حرف *z_راهاندازی_new_thread(بنية k_thread *new_thread,
			 k_thread_پشته_t *پشته, size_t پشته_اندازه,
			 k_thread_مدخل_t مدخل,
			 فراغ *p1, فراغ *p2, فراغ *p3,
			 صحيح prio, uint32_t options, ثابت حرف *name)
{
	حرف *پشته_ptr;

	Z_ASSERT_VALID_PRIO(prio, مدخل);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, پشته, options);

	z_waitq_init(&new_thread->join_صف);

	/* Initialize various struct k_thread members */
	z_init_thread_پایه(&new_thread->پایه, prio, _THREAD_SLEEPING, options);
	پشته_ptr = راهاندازی_thread_پشته(new_thread, پشته, پشته_اندازه);

#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = عدم;
	new_thread->orig_prio = new_thread->پایه.prio;
#پایان_اگر /* Z_MUTEX_PI_ENABLED */

	راهاندازی_shadow_پشته(new_thread, پشته);
	assert_thread_coherence(new_thread, پشته);

	arch_new_thread(new_thread, پشته, پشته_ptr, مدخل, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_داده = عدم;

	assert_switch_handle(new_thread);
	init_thread_custom_داده(new_thread);
	جمع_کن_thread_مقصد_monitor(new_thread, مدخل, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_ماسک(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	إذا (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_جابجا_مقصد_MAIN) &&
	    (_current == عدم)) {
		new_thread->resource_pool = عدم;
		أرجع پشته_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_صف(new_thread);
	init_thread_usage(new_thread);

	SYS_پورت_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	أرجع پشته_ptr;
}

#اگر_تعریف_شده CONFIG_THREAD_RUNTIME_پشته_SAFETY
صحيح z_impl_k_thread_runtime_پشته_استفادهنشده_آستانه_pct_مجموعه(بنية k_thread *thread,
							   uint32_t pct)
{
	size_t استفادهنشده_آستانه;

	إذا (pct > 99) {
		أرجع -EINVAL;   /* 100% unused stack and up is invalid */
	}

	استفادهنشده_آستانه = (thread->پشته_info.اندازه * pct) / 100;

	thread->پشته_info.usage.استفادهنشده_آستانه = استفادهنشده_آستانه;

	أرجع 0;
}

صحيح z_impl_k_thread_runtime_پشته_استفادهنشده_آستانه_مجموعه(بنية k_thread *thread,
						       size_t آستانه)
{
	إذا (آستانه > thread->پشته_info.اندازه) {
		أرجع -EINVAL;
	}

	thread->پشته_info.usage.استفادهنشده_آستانه = آستانه;

	أرجع 0;
}

size_t z_impl_k_thread_runtime_پشته_استفادهنشده_آستانه_get(بنية k_thread *thread)
{
	أرجع thread->پشته_info.usage.استفادهنشده_آستانه;
}

#اگر_تعریف_شده CONFIG_USERSPACE
صحيح z_vrfy_k_thread_runtime_پشته_استفادهنشده_آستانه_pct_مجموعه(بنية k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_پشته_استفادهنشده_آستانه_pct_مجموعه(thread, pct);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

صحيح z_vrfy_k_thread_runtime_پشته_استفادهنشده_آستانه_مجموعه(بنية k_thread *thread,
						       size_t آستانه)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_پشته_استفادهنشده_آستانه_مجموعه(thread, آستانه);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_پشته_استفادهنشده_آستانه_get(بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	أرجع z_impl_k_thread_runtime_پشته_استفادهنشده_آستانه_get(thread);
}
#ضمن <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
#پایان_اگر /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(بنية k_thread *new_thread,
			      k_thread_پشته_t *پشته,
			      size_t پشته_اندازه, k_thread_مدخل_t مدخل,
			      فراغ *p1, فراغ *p2, فراغ *p3,
			      صحيح prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_درون_isr(), "Threads may not be created in ISRs");

	z_راهاندازی_new_thread(new_thread, پشته, پشته_اندازه, مدخل, p1, p2, p3,
			  prio, options, عدم);

	thread_زمانبندی_new(new_thread, delay);

	أرجع new_thread;
}

#اگر_تعریف_شده CONFIG_USERSPACE
bool z_پشته_is_user_capable(k_thread_پشته_t *پشته)
{
	أرجع k_object_پیدا_کن(پشته) != عدم;
}

k_tid_t z_vrfy_k_thread_create(بنية k_thread *new_thread,
			       k_thread_پشته_t *پشته,
			       size_t پشته_اندازه, k_thread_مدخل_t مدخل,
			       فراغ *p1, فراغ *p2, فراغ *p3,
			       صحيح prio, uint32_t options, k_timeout_t delay)
{
	size_t مجموع_اندازه, پشته_obj_اندازه;
	بنية k_object *پشته_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	پشته_object = k_object_پیدا_کن(پشته);
	K_OOPS(K_SYSCALL_VERIFY_پیام(k_object_validation_check(پشته_object, پشته,
						K_OBJ_THREAD_پشته_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_پیام(!اندازه_جمع_کن_سرریز(K_THREAD_پشته_RESERVED,
						       پشته_اندازه, &مجموع_اندازه),
				    "stack size overflow (%zu+%zu)",
				    پشته_اندازه,
				    K_THREAD_پشته_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#اگر_تعریف_شده CONFIG_GEN_PRIV_STACKS
	پشته_obj_اندازه = پشته_object->داده.پشته_داده->اندازه;
#وإلا
	پشته_obj_اندازه = پشته_object->داده.پشته_اندازه;
#پایان_اگر /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_پیام(مجموع_اندازه <= پشته_obj_اندازه,
				    "stack size %zu is too big, max is %zu",
				    مجموع_اندازه, پشته_obj_اندازه));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, عدم)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_پایینی_or_equal(prio,
							_current->پایه.prio)));

	z_راهاندازی_new_thread(new_thread, پشته, پشته_اندازه,
			   مدخل, p1, p2, p3, prio, options, عدم);

	thread_زمانبندی_new(new_thread, delay);

	أرجع new_thread;
}
#ضمن <zephyr/syscalls/k_thread_create_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ z_init_thread_پایه(بنية thread_پایه *thread_پایه_2, صحيح اولویت,
		       uint32_t initial_state, بیعلامت صحيح options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_پایه_2->pended_on = عدم;
	thread_پایه_2->user_options = (uint16_t)options;
	thread_پایه_2->thread_state = (uint8_t)initial_state;

	thread_پایه_2->prio = اولویت;

	thread_پایه_2->sched_locked = 0U;

#اگر_تعریف_شده CONFIG_SMP
	thread_پایه_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_پایه_2->cpu = 0;
#پایان_اگر /* CONFIG_SMP */

#اگر_تعریف_شده CONFIG_TIMESLICE_در_هر_THREAD
	thread_پایه_2->برش_ticks = 0;
	thread_پایه_2->برش_expired = عدم;
#پایان_اگر /* CONFIG_TIMESLICE_PER_THREAD */

#اگر_تعریف_شده CONFIG_SPIN_VALIDATE
	thread_پایه_2->جابجا_داده = عدم;
	/* otherwise swap_data does not need to be initialized */
#پایان_اگر

	z_init_thread_timeout(thread_پایه_2);
}

FUNC_NORETURN فراغ k_thread_user_نما_enter(k_thread_مدخل_t مدخل,
					    فراغ *p1, فراغ *p2, فراغ *p3)
{
	SYS_پورت_TRACING_FUNC(k_thread, user_نما_enter);

	_current->پایه.user_options |= K_USER;
	z_thread_essential_clear(_current);
#اگر_تعریف_شده CONFIG_THREAD_MONITOR
	_current->مدخل.p_مدخل = مدخل;
	_current->مدخل.parameter1 = p1;
	_current->مدخل.parameter2 = p2;
	_current->مدخل.parameter3 = p3;
#پایان_اگر /* CONFIG_THREAD_MONITOR */
#اگر_تعریف_شده CONFIG_USERSPACE
	__ASSERT(z_پشته_is_user_capable(_current->پشته_obj),
		 "dropping to user mode with kernel-only stack object");
#اگر_تعریف_شده CONFIG_THREAD_USERSPACE_LOCAL_داده
	memset(_current->userspace_local_داده, 0,
	       حجم(بنية thread_userspace_local_داده));
#پایان_اگر /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#اگر_تعریف_شده CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_پشته_راهاندازی(_current,
			     (حرف *)(_current->پشته_info.شروع +
				      _current->پشته_info.اندازه));
#پایان_اگر /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_نما_enter(مدخل, p1, p2, p3);
#وإلا
	/* XXX In this case we do not reset the stack */
	z_thread_مدخل(مدخل, p1, p2, p3);
#پایان_اگر /* CONFIG_USERSPACE */
}

#إذا defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_پشته_INFO)
#اگر_تعریف_شده CONFIG_پشته_GROWS_به_بالا
#خطا "Unsupported configuration for stack analysis"
#پایان_اگر /* CONFIG_STACK_GROWS_UP */

صحيح z_پشته_space_get(ثابت uint8_t *پشته_شروع, size_t اندازه, size_t *استفادهنشده_ptr)
{
	size_t استفادهنشده = 0;
	ثابت uint8_t *checked_پشته = پشته_شروع;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ثابت uint8_t *پشته_اشاره_گر = (ثابت uint8_t *)&پشته_شروع;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	إذا ((پشته_اشاره_گر > پشته_شروع) && (پشته_اشاره_گر <= (پشته_شروع + اندازه)) &&
	    IS_ENABLED(CONFIG_NO_استفادهنشده_پشته_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		أرجع -ENOTSUP;
	}

	إذا (IS_ENABLED(CONFIG_پشته_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_پشته += 4;
		اندازه -= 4;
	}

	لكل (size_t i = 0; i < اندازه; i++) {
		إذا ((checked_پشته[i]) == 0xaaU) {
			استفادهنشده++;
		} وإلا {
			اكسر;
		}
	}

	*استفادهنشده_ptr = استفادهنشده;

	أرجع 0;
}

#اگر_تعریف_شده CONFIG_THREAD_RUNTIME_پشته_SAFETY
صحيح k_thread_runtime_پشته_safety_پر_check(ثابت بنية k_thread *thread,
					     size_t *استفادهنشده_ptr,
					     k_thread_پشته_safety_handler_t handler,
					     فراغ *arg)
{
	صحيح    rv;
	size_t استفادهنشده_space;

	__ASSERT_NO_MSG(thread != عدم);

	rv = z_پشته_space_get((ثابت uint8_t *)thread->پشته_info.شروع,
			       thread->پشته_info.اندازه, &استفادهنشده_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (استفادهنشده_ptr != عدم) {
		*استفادهنشده_ptr = استفادهنشده_space;
	}

	إذا ((استفادهنشده_space < thread->پشته_info.usage.استفادهنشده_آستانه) &&
	    (handler != عدم)) {
		handler(thread, استفادهنشده_space, arg);
	}

	أرجع 0;
}

صحيح k_thread_runtime_پشته_safety_آستانه_check(ثابت بنية k_thread *thread,
						  size_t *استفادهنشده_ptr,
						  k_thread_پشته_safety_handler_t handler,
						  فراغ *arg)
{
	صحيح    rv;
	size_t استفادهنشده_space;
	size_t آستانه;
	size_t پیمایش_اندازه;

	__ASSERT_NO_MSG(thread != عدم);

	آستانه = thread->پشته_info.usage.استفادهنشده_آستانه;
	پیمایش_اندازه = آستانه;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	إذا (IS_ENABLED(CONFIG_پشته_SENTINEL) && (آستانه != 0U)) {
		پیمایش_اندازه += حجم(uint32_t);
	}

	rv = z_پشته_space_get((ثابت uint8_t *)thread->پشته_info.شروع,
			       پیمایش_اندازه, &استفادهنشده_space);

	إذا (rv != 0) {
		أرجع rv;
	}

	إذا (استفادهنشده_ptr != عدم) {
		*استفادهنشده_ptr = استفادهنشده_space;
	}

	إذا ((استفادهنشده_space < آستانه) && (handler != عدم)) {
		handler(thread, استفادهنشده_space, arg);
	}

	أرجع 0;
}
#پایان_اگر /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

صحيح z_impl_k_thread_پشته_space_get(ثابت بنية k_thread *thread,
				    size_t *استفادهنشده_ptr)
{
#اگر_تعریف_شده CONFIG_THREAD_پشته_MEM_MAPPED
	إذا (thread->پشته_info.mapped.addr == عدم) {
		أرجع -EINVAL;
	}
#پایان_اگر /* CONFIG_THREAD_STACK_MEM_MAPPED */

	أرجع z_پشته_space_get((ثابت uint8_t *)thread->پشته_info.شروع,
				 thread->پشته_info.اندازه, استفادهنشده_ptr);
}

#اگر_تعریف_شده CONFIG_USERSPACE
صحيح z_vrfy_k_thread_پشته_space_get(ثابت بنية k_thread *thread,
				    size_t *استفادهنشده_ptr)
{
	size_t استفادهنشده;
	صحيح ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_پشته_space_get(thread, &استفادهنشده);
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	ret = k_usermode_مقصد_copy(استفادهنشده_ptr, &استفادهنشده, حجم(size_t));
	CHECKIF(ret != 0) {
		أرجع ret;
	}

	أرجع 0;
}
#ضمن <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
#پایان_اگر /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_مانده_ticks(
						    ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_مانده_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ساكن مضمن k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ثابت بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	أرجع z_impl_k_thread_timeout_expires_ticks(thread);
}
#ضمن <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

#اگر_تعریف_شده CONFIG_INSTRUMENT_THREAD_SWITCHING
فراغ z_thread_mark_switched_درون(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_شروع(_current);
#پایان_اگر /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#اگر_تعریف_شده CONFIG_TRACING
	SYS_پورت_TRACING_FUNC(k_thread, switched_درون);
#پایان_اگر /* CONFIG_TRACING */
}

فراغ z_thread_mark_switched_بیرون(فراغ)
{
#إذا defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#پایان_اگر /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#اگر_تعریف_شده CONFIG_TRACING
#اگر_تعریف_شده CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	إذا (!_current ||
	    (_current->پایه.thread_state & _THREAD_DUMMY) != 0) {
		أرجع;
	}
#پایان_اگر /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_پورت_TRACING_FUNC(k_thread, switched_بیرون);
#پایان_اگر /* CONFIG_TRACING */
}
#پایان_اگر /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

صحيح k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	إذا ((thread == عدم) || (stats == عدم)) {
		أرجع -EINVAL;
	}

#اگر_تعریف_شده CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#وإلا
	*stats = (k_thread_runtime_stats_t) {};
#پایان_اگر /* CONFIG_SCHED_THREAD_USAGE */

	أرجع 0;
}

صحيح k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#اگر_تعریف_شده CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#پایان_اگر /* CONFIG_SCHED_THREAD_USAGE_ALL */

	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#اگر_تعریف_شده CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	بیعلامت صحيح رقم_cpus = arch_رقم_cpus();

	لكل (بیعلامت صحيح i = 0; i < رقم_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_چرخهها += tmp_stats.execution_چرخهها;
		stats->مجموع_چرخهها     += tmp_stats.مجموع_چرخهها;
#اگر_تعریف_شده CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_چرخهها   += tmp_stats.current_چرخهها;
		stats->peak_چرخهها      += tmp_stats.peak_چرخهها;
		stats->معدل_چرخهها   += tmp_stats.معدل_چرخهها;
#پایان_اگر /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_چرخهها      += tmp_stats.idle_چرخهها;
	}
#پایان_اگر /* CONFIG_SCHED_THREAD_USAGE_ALL */

	أرجع 0;
}

صحيح k_thread_runtime_stats_cpu_get(صحيح cpu, k_thread_runtime_stats_t *stats)
{
	إذا (stats == عدم) {
		أرجع -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#اگر_تعریف_شده CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((بیعلامت صحيح)cpu >= arch_رقم_cpus())) {
		أرجع -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#وإلا
	ARG_استفادهنشده(cpu);
#پایان_اگر

	أرجع 0;
}

#اگر_تعریف_شده CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ساكن بنية k_thread *thread_مقصد_cleanup;

/** Spinlock for thread abort cleanup. */
ساكن بنية k_spinlock thread_cleanup_قفل;

#اگر_تعریف_شده CONFIG_THREAD_پشته_MEM_MAPPED
ساكن فراغ *thread_cleanup_پشته_addr;
ساكن size_t thread_cleanup_پشته_sz;

ساكن فراغ unmap_thread_پشته(فراغ *addr, size_t sz)
{
	إذا (addr != عدم) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#پایان_اگر /* CONFIG_THREAD_STACK_MEM_MAPPED */

فراغ defer_thread_cleanup(بنية k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_مقصد_cleanup = thread;

#اگر_تعریف_شده CONFIG_THREAD_پشته_MEM_MAPPED
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
	thread->پشته_info.mapped.addr = عدم;
	thread->پشته_info.mapped.sz = 0;
#پایان_اگر /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_deferred_thread_cleanup(فراغ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#اگر_تعریف_شده CONFIG_THREAD_پشته_MEM_MAPPED
	unmap_thread_پشته(thread_cleanup_پشته_addr, thread_cleanup_پشته_sz);

	thread_cleanup_پشته_addr = عدم;
	thread_cleanup_پشته_sz = 0;
#پایان_اگر /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ do_thread_cleanup(بنية k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#اگر_تعریف_شده CONFIG_THREAD_پشته_MEM_MAPPED
	unmap_thread_پشته(thread->پشته_info.mapped.addr,
			   thread->پشته_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->پشته_info.mapped.addr = عدم;
	thread->پشته_info.mapped.sz = 0;
#وإلا /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_استفادهنشده(thread);
#پایان_اگر /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

فراغ k_thread_abort_cleanup(بنية k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_قفل) {
		إذا (thread_مقصد_cleanup != عدم) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_مقصد_cleanup = عدم;
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
	K_SPINLOCK(&thread_cleanup_قفل) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		إذا (thread_مقصد_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_مقصد_cleanup = عدم;
		}
	}
}

#پایان_اگر /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

فراغ z_dummy_thread_init(بنية k_thread *dummy_thread)
{
	dummy_thread->پایه.thread_state = _THREAD_DUMMY;
#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک
	dummy_thread->پایه.cpu_ماسک = -1;
#پایان_اگر /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->پایه.user_options = K_ESSENTIAL;
#إذا Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = عدم;
	dummy_thread->orig_prio = dummy_thread->پایه.prio;
#پایان_اگر /* Z_MUTEX_PI_ENABLED */
#اگر_تعریف_شده CONFIG_THREAD_پشته_INFO
	dummy_thread->پشته_info.شروع = 0U;
	dummy_thread->پشته_info.اندازه = 0U;
#پایان_اگر /* CONFIG_THREAD_STACK_INFO */
#اگر_تعریف_شده CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#پایان_اگر /* CONFIG_USERSPACE */
#إذا (K_انباشته_MEM_POOL_اندازه > 0)
	k_thread_system_pool_تخصیص_بده(dummy_thread);
#وإلا
	dummy_thread->resource_pool = عدم;
#پایان_اگر /* K_HEAP_MEM_POOL_SIZE */

#اگر_تعریف_شده CONFIG_USE_SWITCH
	dummy_thread->switch_handle = عدم;
#پایان_اگر /* CONFIG_USE_SWITCH */

#اگر_تعریف_شده CONFIG_TIMESLICE_در_هر_THREAD
	dummy_thread->پایه.برش_ticks = 0;
#پایان_اگر /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_مجموعه(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

فراغ z_impl_k_thread_اولویت_مجموعه(k_tid_t thread, صحيح prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, عدم);

	bool need_sched = z_thread_prio_مجموعه((بنية k_thread *)thread, prio);

	إذا ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->پایه.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_اولویت_مجموعه(k_tid_t thread, صحيح prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_پیام(_is_valid_prio(prio, عدم),
				    "invalid thread priority %d", prio));
#اگر_تعریف_نشده CONFIG_USERSPACE_THREAD_MAY_RAISE_اولویت
	K_OOPS(K_SYSCALL_VERIFY_پیام((int8_t)prio >= thread->پایه.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->پایه.prio));
#پایان_اگر /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_اولویت_مجموعه(thread, prio);
}
#ضمن <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	إذا (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_درون_isr())) {
		z_thread_suspend_current(thread);
		أرجع;
	}

	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

	إذا (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(کلید);
		أرجع;
	}

	z_thread_halt(thread, کلید, false);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ضمن <zephyr/syscalls/k_thread_suspend_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

	/* Do not try to resume a thread that was not suspended */
	إذا (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(کلید);
		أرجع;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ضمن <zephyr/syscalls/k_thread_resume_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_پورت_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

	إذا (z_is_thread_sleeping(thread)) {
		(فراغ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(کلید);
	} وإلا {
		z_sched_spinlock_unlock(کلید);
	}
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ضمن <zephyr/syscalls/k_wakeup_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ z_thread_abort(بنية k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

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
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#پایان_اگر /* !CONFIG_ARCH_HAS_THREAD_ABORT */

صحيح z_impl_k_thread_join(بنية k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();
	صحيح ret;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	إذا (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} وإلا إذا ((thread == _current) ||
		   (thread->پایه.pended_on == &_current->join_صف)) {
		ret = -EDEADLK;
	} وإلا {
		__ASSERT(!arch_is_درون_isr(), "cannot join in ISR");
		z_sched_جمع_کن_مقصد_waitq_locked(_current, &thread->join_صف);
		z_جمع_کن_thread_timeout(_current, timeout);

		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_جابجا_locked(کلید);
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		أرجع ret;
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(کلید);
	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USERSPACE
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
	بنية k_object *ko = k_object_پیدا_کن(thread);
	صحيح ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	بدل (ret) {
	حالة 0:
		أرجع false;
	حالة -EINVAL:
		أرجع true;
	افتراضي:
#اگر_تعریف_شده CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#پایان_اگر /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_پیام(ret == 0, "access denied"));
	}
	کد_UNREACHABLE; /* LCOV_EXCL_LINE */
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

	K_OOPS(K_SYSCALL_VERIFY_پیام(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((بنية k_thread *)thread);
}
#ضمن <zephyr/syscalls/k_thread_abort_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

bool k_can_yield(فراغ)
{
	أرجع !(k_is_pre_kernel() || k_is_درون_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

فراغ z_impl_k_yield(فراغ)
{
	__ASSERT(!arch_is_درون_isr(), "");

	SYS_پورت_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_yield(فراغ)
{
	z_impl_k_yield();
}
#ضمن <zephyr/syscalls/k_yield_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
