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

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_ڍير
K_OBJ_TYPE_DEFINE(obj_type_ڍير, k_ڍير_2, K_OBJ_TYPE_ڍير_ID, عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STACK */

فراغ k_ڍير_init_2(بنية k_ڍير_2 *ڍير, ڍير_معلومات_t *بفر,
		  uint32_t انگ_entries)
{
	z_waitq_init(&ڍير->wait_q);
	ڍير->تالو = (بنية k_spinlock) {};
	ڍير->ايندڙ = بفر;
	ڍير->بنياد = بفر;
	ڍير->چوٽي = ڍير->بنياد + انگ_entries;
	ڍير->flags = 0;

	SYS_پورٽ_TRACING_OBJ_INIT(k_ڍير_2, ڍير);
	k_object_init(ڍير);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_ڍير
	k_obj_core_init_and_link(K_OBJ_CORE(ڍير), &obj_type_ڍير);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ڍير_alloc_init(بنية k_ڍير_2 *ڍير, uint32_t انگ_entries)
{
	فراغ *بفر;
	size_t ڪل_سائيز;
	int32_t ret;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_2, alloc_init, ڍير);

	/* Reject allocation sizes that cannot be represented. */
	إذا (سائيز_mul_حد_کان_مٿي(انگ_entries, حجم(ڍير_معلومات_t), &ڪل_سائيز)) {
		ret = -ENOMEM;
	} وإلا {
		بفر = z_thread_malloc(ڪل_سائيز);
		إذا (بفر != عدم) {
			k_ڍير_init_2(ڍير, بفر, انگ_entries);
			ڍير->flags = K_ڍير_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_2, alloc_init, ڍير, ret);

	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_ڍير_alloc_init(بنية k_ڍير_2 *ڍير,
					      uint32_t انگ_entries)
{
	size_t ڪل_سائيز;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ڍير, K_OBJ_ڍير));
	K_OOPS(K_SYSCALL_VERIFY(انگ_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!سائيز_mul_حد_کان_مٿي(انگ_entries, حجم(ڍير_معلومات_t),
					&ڪل_سائيز)));
	أرجع z_impl_k_ڍير_alloc_init(ڍير, انگ_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_ڍير_cleanup(بنية k_ڍير_2 *ڍير, __maybe_unused bool locked)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_2, cleanup, ڍير);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ڍير->تالو);

	إذا (locked && (z_waitq_سر_locked(&ڍير->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب ٻاهريون;
	}

	إذا (!locked && (z_waitq_سر(&ڍير->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب ٻاهريون;
	}

	إذا ((ڍير->flags & K_ڍير_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ڍير->بنياد);
		ڍير->بنياد = عدم;
		ڍير->flags &= ~K_ڍير_FLAG_ALLOC;
		freed = true;
	}

ٻاهريون:
	k_spin_unlock(&ڍير->تالو, ڪنجي);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_ڍير
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ڍير));
	}
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STACK */
	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_2, cleanup, ڍير, ret);

	أرجع ret;
}

صحيح k_ڍير_cleanup(بنية k_ڍير_2 *ڍير)
{
	أرجع z_ڍير_cleanup(ڍير, false);
}

صحيح z_impl_k_ڍير_ڌڪو(بنية k_ڍير_2 *ڍير, ڍير_معلومات_t معلومات)
{
	صحيح ret = 0;
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ڍير->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_2, ڌڪو, ڍير);

	إذا (ڍير->ايندڙ == ڍير->چوٽي) {
		ret = -ENOMEM;
		اذهب ٻاهريون;
	}

	إذا (z_sched_wake(&ڍير->wait_q, 0, (فراغ *)معلومات)) {
		z_reschedule(&ڍير->تالو, ڪنجي);
		اذهب پڇاڙي;
	} وإلا {
		*(ڍير->ايندڙ) = معلومات;
		ڍير->ايندڙ++;
		اذهب ٻاهريون;
	}

ٻاهريون:
	k_spin_unlock(&ڍير->تالو, ڪنجي);

پڇاڙي:
	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_2, ڌڪو, ڍير, ret);

	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ڍير_ڌڪو(بنية k_ڍير_2 *ڍير, ڍير_معلومات_t معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(ڍير, K_OBJ_ڍير));

	أرجع z_impl_k_ڍير_ڌڪو(ڍير, معلومات);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_ڍير_ڪڍو(بنية k_ڍير_2 *ڍير, ڍير_معلومات_t *معلومات,
		       k_timeout_t timeout)
{
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح نتيجو;

	ڪنجي = k_spin_تالو(&ڍير->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_ڍير_2, ڪڍو, ڍير, timeout);

	إذا (likely(ڍير->ايندڙ > ڍير->بنياد)) {
		ڍير->ايندڙ--;
		*معلومات = *(ڍير->ايندڙ);
		k_spin_unlock(&ڍير->تالو, ڪنجي);

		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_2, ڪڍو, ڍير, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ڍير->تالو, ڪنجي);

		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_2, ڪڍو, ڍير, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_ڍير_2, ڪڍو, ڍير, timeout);

	نتيجو = z_pend_curr(&ڍير->تالو, ڪنجي, &ڍير->wait_q, timeout);
	إذا (نتيجو != 0) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_2, ڪڍو, ڍير, timeout, نتيجو);

		أرجع نتيجو;
	}

	*معلومات = (ڍير_معلومات_t)_current->بنياد.مٽا_سٽا_معلومات;

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_ڍير_2, ڪڍو, ڍير, timeout, 0);

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ڍير_ڪڍو(بنية k_ڍير_2 *ڍير,
				     ڍير_معلومات_t *معلومات, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ڍير, K_OBJ_ڍير));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(معلومات, حجم(ڍير_معلومات_t)));
	أرجع z_impl_k_ڍير_ڪڍو(ڍير, معلومات, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
