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

#جے_تعریف_شدہ CONFIG_OBJ_CORE_ڈھیر
K_OBJ_TYPE_DEFINE(obj_type_ڈھیر, k_ڈھیر_2, K_OBJ_TYPE_ڈھیر_ID, عدم);
#جے_انت /* CONFIG_OBJ_CORE_STACK */

فراغ k_ڈھیر_init_2(بنية k_ڈھیر_2 *ڈھیر, ڈھیر_انکڑے_t *بفر,
		  uint32_t عدد_entries)
{
	z_waitq_init(&ڈھیر->wait_q);
	ڈھیر->جندرا = (بنية k_spinlock) {};
	ڈھیر->اگلا = بفر;
	ڈھیر->ادھار = بفر;
	ڈھیر->چوٹی = ڈھیر->ادھار + عدد_entries;
	ڈھیر->flags = 0;

	SYS_پورٹ_TRACING_OBJ_INIT(k_ڈھیر_2, ڈھیر);
	k_object_init(ڈھیر);

#جے_تعریف_شدہ CONFIG_OBJ_CORE_ڈھیر
	k_obj_core_init_and_link(K_OBJ_CORE(ڈھیر), &obj_type_ڈھیر);
#جے_انت /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ڈھیر_alloc_init(بنية k_ڈھیر_2 *ڈھیر, uint32_t عدد_entries)
{
	فراغ *بفر;
	size_t کل_ناپ;
	int32_t ret;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, alloc_init, ڈھیر);

	/* Reject allocation sizes that cannot be represented. */
	إذا (ناپ_mul_حد_توں_ودھ(عدد_entries, حجم(ڈھیر_انکڑے_t), &کل_ناپ)) {
		ret = -ENOMEM;
	} وإلا {
		بفر = z_thread_malloc(کل_ناپ);
		إذا (بفر != عدم) {
			k_ڈھیر_init_2(ڈھیر, بفر, عدد_entries);
			ڈھیر->flags = K_ڈھیر_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, alloc_init, ڈھیر, ret);

	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_ڈھیر_alloc_init(بنية k_ڈھیر_2 *ڈھیر,
					      uint32_t عدد_entries)
{
	size_t کل_ناپ;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ڈھیر, K_OBJ_ڈھیر));
	K_OOPS(K_SYSCALL_VERIFY(عدد_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!ناپ_mul_حد_توں_ودھ(عدد_entries, حجم(ڈھیر_انکڑے_t),
					&کل_ناپ)));
	أرجع z_impl_k_ڈھیر_alloc_init(ڈھیر, عدد_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_ڈھیر_cleanup(بنية k_ڈھیر_2 *ڈھیر, __maybe_unused bool locked)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, cleanup, ڈھیر);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&ڈھیر->جندرا);

	إذا (locked && (z_waitq_سر_locked(&ڈھیر->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب باہرلا;
	}

	إذا (!locked && (z_waitq_سر(&ڈھیر->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب باہرلا;
	}

	إذا ((ڈھیر->flags & K_ڈھیر_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ڈھیر->ادھار);
		ڈھیر->ادھار = عدم;
		ڈھیر->flags &= ~K_ڈھیر_FLAG_ALLOC;
		freed = true;
	}

باہرلا:
	k_spin_unlock(&ڈھیر->جندرا, کلید);

#جے_تعریف_شدہ CONFIG_OBJ_CORE_ڈھیر
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ڈھیر));
	}
#جے_انت /* CONFIG_OBJ_CORE_STACK */
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, cleanup, ڈھیر, ret);

	أرجع ret;
}

صحيح k_ڈھیر_cleanup(بنية k_ڈھیر_2 *ڈھیر)
{
	أرجع z_ڈھیر_cleanup(ڈھیر, false);
}

صحيح z_impl_k_ڈھیر_ڈالو(بنية k_ڈھیر_2 *ڈھیر, ڈھیر_انکڑے_t انکڑے)
{
	صحيح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&ڈھیر->جندرا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, ڈالو, ڈھیر);

	إذا (ڈھیر->اگلا == ڈھیر->چوٹی) {
		ret = -ENOMEM;
		اذهب باہرلا;
	}

	إذا (z_sched_wake(&ڈھیر->wait_q, 0, (فراغ *)انکڑے)) {
		z_reschedule(&ڈھیر->جندرا, کلید);
		اذهب سرا;
	} وإلا {
		*(ڈھیر->اگلا) = انکڑے;
		ڈھیر->اگلا++;
		اذهب باہرلا;
	}

باہرلا:
	k_spin_unlock(&ڈھیر->جندرا, کلید);

سرا:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, ڈالو, ڈھیر, ret);

	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ڈھیر_ڈالو(بنية k_ڈھیر_2 *ڈھیر, ڈھیر_انکڑے_t انکڑے)
{
	K_OOPS(K_SYSCALL_OBJ(ڈھیر, K_OBJ_ڈھیر));

	أرجع z_impl_k_ڈھیر_ڈالو(ڈھیر, انکڑے);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_impl_k_ڈھیر_نکالو(بنية k_ڈھیر_2 *ڈھیر, ڈھیر_انکڑے_t *انکڑے,
		       k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحيح نتیجہ;

	کلید = k_spin_جندرا(&ڈھیر->جندرا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, نکالو, ڈھیر, timeout);

	إذا (likely(ڈھیر->اگلا > ڈھیر->ادھار)) {
		ڈھیر->اگلا--;
		*انکڑے = *(ڈھیر->اگلا);
		k_spin_unlock(&ڈھیر->جندرا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ڈھیر->جندرا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_ڈھیر_2, نکالو, ڈھیر, timeout);

	نتیجہ = z_pend_curr(&ڈھیر->جندرا, کلید, &ڈھیر->wait_q, timeout);
	إذا (نتیجہ != 0) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, نتیجہ);

		أرجع نتیجہ;
	}

	*انکڑے = (ڈھیر_انکڑے_t)_current->ادھار.وٹاندرا_انکڑے;

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, 0);

	أرجع 0;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ڈھیر_نکالو(بنية k_ڈھیر_2 *ڈھیر,
				     ڈھیر_انکڑے_t *انکڑے, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ڈھیر, K_OBJ_ڈھیر));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(انکڑے, حجم(ڈھیر_انکڑے_t)));
	أرجع z_impl_k_ڈھیر_نکالو(ڈھیر, انکڑے, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */
