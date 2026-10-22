/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_ਢੇਰ
K_OBJ_TYPE_DEFINE(obj_type_ਢੇਰ, k_ਢੇਰ_2, K_OBJ_TYPE_ਢੇਰ_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STACK */

ਖਾਲੀ k_ਢੇਰ_init_2(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ, ਢੇਰ_ਅੰਕੜੇ_t *ਬਫਰ,
		  uint32_t ਅੰਕ_entries)
{
	z_waitq_init(&ਢੇਰ->wait_q);
	ਢੇਰ->ਤਾਲਾ = (ਬਣਤਰ k_spinlock) {};
	ਢੇਰ->ਅਗਲਾ = ਬਫਰ;
	ਢੇਰ->ਅਧਾਰ = ਬਫਰ;
	ਢੇਰ->ਸਿਖਰ = ਢੇਰ->ਅਧਾਰ + ਅੰਕ_entries;
	ਢੇਰ->flags = 0;

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_ਢੇਰ_2, ਢੇਰ);
	k_object_init(ਢੇਰ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_ਢੇਰ
	k_obj_core_init_and_link(K_OBJ_CORE(ਢੇਰ), &obj_type_ਢੇਰ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ਢੇਰ_alloc_init(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ, uint32_t ਅੰਕ_entries)
{
	ਖਾਲੀ *ਬਫਰ;
	size_t ਕੁੱਲ_ਮਾਪ;
	int32_t ret;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_2, alloc_init, ਢੇਰ);

	/* Reject allocation sizes that cannot be represented. */
	ਜੇ (ਮਾਪ_mul_ਉਛਾਲ(ਅੰਕ_entries, ਆਕਾਰ(ਢੇਰ_ਅੰਕੜੇ_t), &ਕੁੱਲ_ਮਾਪ)) {
		ret = -ENOMEM;
	} ਨਹੀਂਤਾਂ {
		ਬਫਰ = z_thread_malloc(ਕੁੱਲ_ਮਾਪ);
		ਜੇ (ਬਫਰ != ਨਿਰਾ) {
			k_ਢੇਰ_init_2(ਢੇਰ, ਬਫਰ, ਅੰਕ_entries);
			ਢੇਰ->flags = K_ਢੇਰ_FLAG_ALLOC;
			ret = 0;
		} ਨਹੀਂਤਾਂ {
			ret = -ENOMEM;
		}
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_2, alloc_init, ਢੇਰ, ret);

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ int32_t z_vrfy_k_ਢੇਰ_alloc_init(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ,
					      uint32_t ਅੰਕ_entries)
{
	size_t ਕੁੱਲ_ਮਾਪ;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ਢੇਰ, K_OBJ_ਢੇਰ));
	K_OOPS(K_SYSCALL_VERIFY(ਅੰਕ_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!ਮਾਪ_mul_ਉਛਾਲ(ਅੰਕ_entries, ਆਕਾਰ(ਢੇਰ_ਅੰਕੜੇ_t),
					&ਕੁੱਲ_ਮਾਪ)));
	ਵਾਪਸ z_impl_k_ਢੇਰ_alloc_init(ਢੇਰ, ਅੰਕ_entries);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_ਢੇਰ_cleanup(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ, __maybe_unused bool locked)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_2, cleanup, ਢੇਰ);

	ਪੂਰਨ ret = 0;
	bool freed = false;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ->ਤਾਲਾ);

	ਜੇ (locked && (z_waitq_ਸਿਰ_locked(&ਢੇਰ->wait_q) != ਨਿਰਾ)) {
		ret = -EAGAIN;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (!locked && (z_waitq_ਸਿਰ(&ਢੇਰ->wait_q) != ਨਿਰਾ)) {
		ret = -EAGAIN;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ ((ਢੇਰ->flags & K_ਢੇਰ_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ਢੇਰ->ਅਧਾਰ);
		ਢੇਰ->ਅਧਾਰ = ਨਿਰਾ;
		ਢੇਰ->flags &= ~K_ਢੇਰ_FLAG_ALLOC;
		freed = true;
	}

ਬਾਹਰਲਾ:
	k_spin_unlock(&ਢੇਰ->ਤਾਲਾ, ਕੁੰਜੀ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_ਢੇਰ
	ਜੇ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ਢੇਰ));
	}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STACK */
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_2, cleanup, ਢੇਰ, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_ਢੇਰ_cleanup(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ)
{
	ਵਾਪਸ z_ਢੇਰ_cleanup(ਢੇਰ, false);
}

ਪੂਰਨ z_impl_k_ਢੇਰ_ਧੱਕੋ(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ, ਢੇਰ_ਅੰਕੜੇ_t ਅੰਕੜੇ)
{
	ਪੂਰਨ ret = 0;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_2, ਧੱਕੋ, ਢੇਰ);

	ਜੇ (ਢੇਰ->ਅਗਲਾ == ਢੇਰ->ਸਿਖਰ) {
		ret = -ENOMEM;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (z_sched_wake(&ਢੇਰ->wait_q, 0, (ਖਾਲੀ *)ਅੰਕੜੇ)) {
		z_reschedule(&ਢੇਰ->ਤਾਲਾ, ਕੁੰਜੀ);
		ਜਾਓ ਸਿਰਾ;
	} ਨਹੀਂਤਾਂ {
		*(ਢੇਰ->ਅਗਲਾ) = ਅੰਕੜੇ;
		ਢੇਰ->ਅਗਲਾ++;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

ਬਾਹਰਲਾ:
	k_spin_unlock(&ਢੇਰ->ਤਾਲਾ, ਕੁੰਜੀ);

ਸਿਰਾ:
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_2, ਧੱਕੋ, ਢੇਰ, ret);

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_ਢੇਰ_ਧੱਕੋ(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ, ਢੇਰ_ਅੰਕੜੇ_t ਅੰਕੜੇ)
{
	K_OOPS(K_SYSCALL_OBJ(ਢੇਰ, K_OBJ_ਢੇਰ));

	ਵਾਪਸ z_impl_k_ਢੇਰ_ਧੱਕੋ(ਢੇਰ, ਅੰਕੜੇ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_stack_push_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_ਢੇਰ_ਕੱਢੋ(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ, ਢੇਰ_ਅੰਕੜੇ_t *ਅੰਕੜੇ,
		       k_timeout_t timeout)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ਨਤੀਜਾ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਢੇਰ->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਢੇਰ_2, ਕੱਢੋ, ਢੇਰ, timeout);

	ਜੇ (likely(ਢੇਰ->ਅਗਲਾ > ਢੇਰ->ਅਧਾਰ)) {
		ਢੇਰ->ਅਗਲਾ--;
		*ਅੰਕੜੇ = *(ਢੇਰ->ਅਗਲਾ);
		k_spin_unlock(&ਢੇਰ->ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_2, ਕੱਢੋ, ਢੇਰ, timeout, 0);

		ਵਾਪਸ 0;
	}

	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ਢੇਰ->ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_2, ਕੱਢੋ, ਢੇਰ, timeout, -EBUSY);

		ਵਾਪਸ -EBUSY;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_ਢੇਰ_2, ਕੱਢੋ, ਢੇਰ, timeout);

	ਨਤੀਜਾ = z_pend_curr(&ਢੇਰ->ਤਾਲਾ, ਕੁੰਜੀ, &ਢੇਰ->wait_q, timeout);
	ਜੇ (ਨਤੀਜਾ != 0) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_2, ਕੱਢੋ, ਢੇਰ, timeout, ਨਤੀਜਾ);

		ਵਾਪਸ ਨਤੀਜਾ;
	}

	*ਅੰਕੜੇ = (ਢੇਰ_ਅੰਕੜੇ_t)_current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਢੇਰ_2, ਕੱਢੋ, ਢੇਰ, timeout, 0);

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_ਢੇਰ_ਕੱਢੋ(ਬਣਤਰ k_ਢੇਰ_2 *ਢੇਰ,
				     ਢੇਰ_ਅੰਕੜੇ_t *ਅੰਕੜੇ, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ਢੇਰ, K_OBJ_ਢੇਰ));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(ਅੰਕੜੇ, ਆਕਾਰ(ਢੇਰ_ਅੰਕੜੇ_t)));
	ਵਾਪਸ z_impl_k_ਢੇਰ_ਕੱਢੋ(ਢੇਰ, ਅੰਕੜੇ, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_stack_pop_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
