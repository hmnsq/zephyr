/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <wait_q.h>
#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <kernel_internal.h>
#شامل <stdbool.h>

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_ڈھیر
K_OBJ_TYPE_DEFINE(obj_type_ڈھیر, k_ڈھیر_2, K_OBJ_TYPE_ڈھیر_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_STACK */

خالی k_ڈھیر_init_2(ساخت k_ڈھیر_2 *ڈھیر, ڈھیر_ڈیٹا_t *بفر,
		  uint32_t عدد_entries)
{
	z_waitq_init(&ڈھیر->wait_q);
	ڈھیر->تالا = (ساخت k_spinlock) {};
	ڈھیر->اگلا = بفر;
	ڈھیر->بنیاد = بفر;
	ڈھیر->چوٹی = ڈھیر->بنیاد + عدد_entries;
	ڈھیر->flags = 0;

	SYS_پورٹ_TRACING_OBJ_INIT(k_ڈھیر_2, ڈھیر);
	k_object_init(ڈھیر);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_ڈھیر
	k_obj_core_init_and_link(K_OBJ_CORE(ڈھیر), &obj_type_ڈھیر);
#اگر_ختم /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ڈھیر_alloc_init(ساخت k_ڈھیر_2 *ڈھیر, uint32_t عدد_entries)
{
	خالی *بفر;
	size_t کل_سائز;
	int32_t ret;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, alloc_init, ڈھیر);

	/* Reject allocation sizes that cannot be represented. */
	اگر (سائز_mul_حد_سے_تجاوز(عدد_entries, حجم(ڈھیر_ڈیٹا_t), &کل_سائز)) {
		ret = -ENOMEM;
	} ورنہ {
		بفر = z_thread_malloc(کل_سائز);
		اگر (بفر != تہی) {
			k_ڈھیر_init_2(ڈھیر, بفر, عدد_entries);
			ڈھیر->flags = K_ڈھیر_FLAG_ALLOC;
			ret = 0;
		} ورنہ {
			ret = -ENOMEM;
		}
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, alloc_init, ڈھیر, ret);

	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی int32_t z_vrfy_k_ڈھیر_alloc_init(ساخت k_ڈھیر_2 *ڈھیر,
					      uint32_t عدد_entries)
{
	size_t کل_سائز;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ڈھیر, K_OBJ_ڈھیر));
	K_OOPS(K_SYSCALL_VERIFY(عدد_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!سائز_mul_حد_سے_تجاوز(عدد_entries, حجم(ڈھیر_ڈیٹا_t),
					&کل_سائز)));
	واپس z_impl_k_ڈھیر_alloc_init(ڈھیر, عدد_entries);
}
#شامل <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_ڈھیر_cleanup(ساخت k_ڈھیر_2 *ڈھیر, __maybe_unused bool locked)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, cleanup, ڈھیر);

	صحیح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_تالا(&ڈھیر->تالا);

	اگر (locked && (z_waitq_سر_locked(&ڈھیر->wait_q) != تہی)) {
		ret = -EAGAIN;
		جاؤ برآمد;
	}

	اگر (!locked && (z_waitq_سر(&ڈھیر->wait_q) != تہی)) {
		ret = -EAGAIN;
		جاؤ برآمد;
	}

	اگر ((ڈھیر->flags & K_ڈھیر_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ڈھیر->بنیاد);
		ڈھیر->بنیاد = تہی;
		ڈھیر->flags &= ~K_ڈھیر_FLAG_ALLOC;
		freed = true;
	}

برآمد:
	k_spin_unlock(&ڈھیر->تالا, کلید);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_ڈھیر
	اگر (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ڈھیر));
	}
#اگر_ختم /* CONFIG_OBJ_CORE_STACK */
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, cleanup, ڈھیر, ret);

	واپس ret;
}

صحیح k_ڈھیر_cleanup(ساخت k_ڈھیر_2 *ڈھیر)
{
	واپس z_ڈھیر_cleanup(ڈھیر, false);
}

صحیح z_impl_k_ڈھیر_ڈالو(ساخت k_ڈھیر_2 *ڈھیر, ڈھیر_ڈیٹا_t ڈیٹا)
{
	صحیح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_تالا(&ڈھیر->تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, ڈالو, ڈھیر);

	اگر (ڈھیر->اگلا == ڈھیر->چوٹی) {
		ret = -ENOMEM;
		جاؤ برآمد;
	}

	اگر (z_sched_wake(&ڈھیر->wait_q, 0, (خالی *)ڈیٹا)) {
		z_reschedule(&ڈھیر->تالا, کلید);
		جاؤ انتہا;
	} ورنہ {
		*(ڈھیر->اگلا) = ڈیٹا;
		ڈھیر->اگلا++;
		جاؤ برآمد;
	}

برآمد:
	k_spin_unlock(&ڈھیر->تالا, کلید);

انتہا:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, ڈالو, ڈھیر, ret);

	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_ڈھیر_ڈالو(ساخت k_ڈھیر_2 *ڈھیر, ڈھیر_ڈیٹا_t ڈیٹا)
{
	K_OOPS(K_SYSCALL_OBJ(ڈھیر, K_OBJ_ڈھیر));

	واپس z_impl_k_ڈھیر_ڈالو(ڈھیر, ڈیٹا);
}
#شامل <zephyr/syscalls/k_stack_push_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_ڈھیر_نکالو(ساخت k_ڈھیر_2 *ڈھیر, ڈھیر_ڈیٹا_t *ڈیٹا,
		       k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحیح نتیجہ;

	کلید = k_spin_تالا(&ڈھیر->تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_ڈھیر_2, نکالو, ڈھیر, timeout);

	اگر (likely(ڈھیر->اگلا > ڈھیر->بنیاد)) {
		ڈھیر->اگلا--;
		*ڈیٹا = *(ڈھیر->اگلا);
		k_spin_unlock(&ڈھیر->تالا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, 0);

		واپس 0;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ڈھیر->تالا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, -EBUSY);

		واپس -EBUSY;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_ڈھیر_2, نکالو, ڈھیر, timeout);

	نتیجہ = z_pend_curr(&ڈھیر->تالا, کلید, &ڈھیر->wait_q, timeout);
	اگر (نتیجہ != 0) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, نتیجہ);

		واپس نتیجہ;
	}

	*ڈیٹا = (ڈھیر_ڈیٹا_t)_current->بنیاد.تبادلہ_ڈیٹا;

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_ڈھیر_2, نکالو, ڈھیر, timeout, 0);

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_ڈھیر_نکالو(ساخت k_ڈھیر_2 *ڈھیر,
				     ڈھیر_ڈیٹا_t *ڈیٹا, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ڈھیر, K_OBJ_ڈھیر));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(ڈیٹا, حجم(ڈھیر_ڈیٹا_t)));
	واپس z_impl_k_ڈھیر_نکالو(ڈھیر, ڈیٹا, timeout);
}
#شامل <zephyr/syscalls/k_stack_pop_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
