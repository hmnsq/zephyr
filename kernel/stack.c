/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <wait_q.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <kernel_internal.h>
#ضمن <stdbool.h>

#که_تعریف_شوی CONFIG_OBJ_CORE_ډېری
K_OBJ_TYPE_DEFINE(obj_type_ډېری, k_ډېری, K_OBJ_TYPE_ډېری_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_STACK */

فراغ k_ډېری_init(بنية k_ډېری *ډېری, ډېری_معلومات_t *بفر,
		  uint32_t عدد_entries)
{
	z_waitq_init(&ډېری->wait_q);
	ډېری->قلف = (بنية k_spinlock) {};
	ډېری->بل = بفر;
	ډېری->بنسټ = بفر;
	ډېری->سر_2 = ډېری->بنسټ + عدد_entries;
	ډېری->flags = 0;

	SYS_پورټ_TRACING_OBJ_INIT(k_ډېری, ډېری);
	k_object_init(ډېری);

#که_تعریف_شوی CONFIG_OBJ_CORE_ډېری
	k_obj_core_init_and_link(K_OBJ_CORE(ډېری), &obj_type_ډېری);
#د_که_پای /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ډېری_alloc_init(بنية k_ډېری *ډېری, uint32_t عدد_entries)
{
	فراغ *بفر;
	size_t ټول_اندازه;
	int32_t ret;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېری, alloc_init, ډېری);

	/* Reject allocation sizes that cannot be represented. */
	إذا (اندازه_mul_ډکېدنه(عدد_entries, حجم(ډېری_معلومات_t), &ټول_اندازه)) {
		ret = -ENOMEM;
	} وإلا {
		بفر = z_thread_malloc(ټول_اندازه);
		إذا (بفر != عدم) {
			k_ډېری_init(ډېری, بفر, عدد_entries);
			ډېری->flags = K_ډېری_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېری, alloc_init, ډېری, ret);

	أرجع ret;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_ډېری_alloc_init(بنية k_ډېری *ډېری,
					      uint32_t عدد_entries)
{
	size_t ټول_اندازه;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ډېری, K_OBJ_ډېری));
	K_OOPS(K_SYSCALL_VERIFY(عدد_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!اندازه_mul_ډکېدنه(عدد_entries, حجم(ډېری_معلومات_t),
					&ټول_اندازه)));
	أرجع z_impl_k_ډېری_alloc_init(ډېری, عدد_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_ډېری_cleanup(بنية k_ډېری *ډېری, __maybe_unused bool locked)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېری, cleanup, ډېری);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلی_t کلی = k_spin_قلف(&ډېری->قلف);

	إذا (locked && (z_waitq_سر_locked(&ډېری->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب بهر;
	}

	إذا (!locked && (z_waitq_سر(&ډېری->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب بهر;
	}

	إذا ((ډېری->flags & K_ډېری_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ډېری->بنسټ);
		ډېری->بنسټ = عدم;
		ډېری->flags &= ~K_ډېری_FLAG_ALLOC;
		freed = true;
	}

بهر:
	k_spin_unlock(&ډېری->قلف, کلی);

#که_تعریف_شوی CONFIG_OBJ_CORE_ډېری
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ډېری));
	}
#د_که_پای /* CONFIG_OBJ_CORE_STACK */
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېری, cleanup, ډېری, ret);

	أرجع ret;
}

صحيح k_ډېری_cleanup(بنية k_ډېری *ډېری)
{
	أرجع z_ډېری_cleanup(ډېری, false);
}

صحيح z_impl_k_ډېری_ټېل(بنية k_ډېری *ډېری, ډېری_معلومات_t معلومات)
{
	صحيح ret = 0;
	k_spinlock_کلی_t کلی = k_spin_قلف(&ډېری->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېری, ټېل, ډېری);

	إذا (ډېری->بل == ډېری->سر_2) {
		ret = -ENOMEM;
		اذهب بهر;
	}

	إذا (z_sched_wake(&ډېری->wait_q, 0, (فراغ *)معلومات)) {
		z_reschedule(&ډېری->قلف, کلی);
		اذهب پای_ټکی;
	} وإلا {
		*(ډېری->بل) = معلومات;
		ډېری->بل++;
		اذهب بهر;
	}

بهر:
	k_spin_unlock(&ډېری->قلف, کلی);

پای_ټکی:
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېری, ټېل, ډېری, ret);

	أرجع ret;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ډېری_ټېل(بنية k_ډېری *ډېری, ډېری_معلومات_t معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(ډېری, K_OBJ_ډېری));

	أرجع z_impl_k_ډېری_ټېل(ډېری, معلومات);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_ډېری_راوباسه(بنية k_ډېری *ډېری, ډېری_معلومات_t *معلومات,
		       k_timeout_t timeout)
{
	k_spinlock_کلی_t کلی;
	صحيح پایله;

	کلی = k_spin_قلف(&ډېری->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_ډېری, راوباسه, ډېری, timeout);

	إذا (likely(ډېری->بل > ډېری->بنسټ)) {
		ډېری->بل--;
		*معلومات = *(ډېری->بل);
		k_spin_unlock(&ډېری->قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېری, راوباسه, ډېری, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ډېری->قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېری, راوباسه, ډېری, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_ډېری, راوباسه, ډېری, timeout);

	پایله = z_pend_curr(&ډېری->قلف, کلی, &ډېری->wait_q, timeout);
	إذا (پایله != 0) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېری, راوباسه, ډېری, timeout, پایله);

		أرجع پایله;
	}

	*معلومات = (ډېری_معلومات_t)_current->بنسټ.بدلون_معلومات;

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_ډېری, راوباسه, ډېری, timeout, 0);

	أرجع 0;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ډېری_راوباسه(بنية k_ډېری *ډېری,
				     ډېری_معلومات_t *معلومات, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ډېری, K_OBJ_ډېری));
	K_OOPS(K_SYSCALL_حافظه_WRITE(معلومات, حجم(ډېری_معلومات_t)));
	أرجع z_impl_k_ډېری_راوباسه(ډېری, معلومات, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
