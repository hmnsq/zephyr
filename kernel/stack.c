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

#ifdef CONFIG_OBJ_CORE_دۆۋە
K_OBJ_TYPE_DEFINE(obj_type_دۆۋە, k_دۆۋە, K_OBJ_TYPE_دۆۋە_ID, عدم);
#endif /* CONFIG_OBJ_CORE_STACK */

فراغ k_دۆۋە_init(بنية k_دۆۋە *دۆۋە, دۆۋە_data_t *buffer,
		  uint32_t رەقەم_entries)
{
	z_waitq_init(&دۆۋە->wait_q);
	دۆۋە->lock = (بنية k_spinlock) {};
	دۆۋە->next = buffer;
	دۆۋە->base = buffer;
	دۆۋە->چوققا = دۆۋە->base + رەقەم_entries;
	دۆۋە->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_دۆۋە, دۆۋە);
	k_object_init(دۆۋە);

#ifdef CONFIG_OBJ_CORE_دۆۋە
	k_obj_core_init_and_link(K_OBJ_CORE(دۆۋە), &obj_type_دۆۋە);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_دۆۋە_alloc_init(بنية k_دۆۋە *دۆۋە, uint32_t رەقەم_entries)
{
	فراغ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_دۆۋە, alloc_init, دۆۋە);

	/* Reject allocation sizes that cannot be represented. */
	إذا (size_mul_overflow(رەقەم_entries, حجم(دۆۋە_data_t), &total_size)) {
		ret = -ENOMEM;
	} وإلا {
		buffer = z_thread_malloc(total_size);
		إذا (buffer != عدم) {
			k_دۆۋە_init(دۆۋە, buffer, رەقەم_entries);
			دۆۋە->flags = K_دۆۋە_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_دۆۋە, alloc_init, دۆۋە, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_دۆۋە_alloc_init(بنية k_دۆۋە *دۆۋە,
					      uint32_t رەقەم_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(دۆۋە, K_OBJ_دۆۋە));
	K_OOPS(K_SYSCALL_VERIFY(رەقەم_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(رەقەم_entries, حجم(دۆۋە_data_t),
					&total_size)));
	أرجع z_impl_k_دۆۋە_alloc_init(دۆۋە, رەقەم_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_دۆۋە_cleanup(بنية k_دۆۋە *دۆۋە, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_دۆۋە, cleanup, دۆۋە);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_ئاچقۇچ_t ئاچقۇچ = k_spin_lock(&دۆۋە->lock);

	إذا (locked && (z_waitq_head_locked(&دۆۋە->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا (!locked && (z_waitq_head(&دۆۋە->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا ((دۆۋە->flags & K_دۆۋە_FLAG_ALLOC) != (uint8_t)0) {
		k_free(دۆۋە->base);
		دۆۋە->base = عدم;
		دۆۋە->flags &= ~K_دۆۋە_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&دۆۋە->lock, ئاچقۇچ);

#ifdef CONFIG_OBJ_CORE_دۆۋە
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(دۆۋە));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_دۆۋە, cleanup, دۆۋە, ret);

	أرجع ret;
}

صحيح k_دۆۋە_cleanup(بنية k_دۆۋە *دۆۋە)
{
	أرجع z_دۆۋە_cleanup(دۆۋە, false);
}

صحيح z_impl_k_دۆۋە_ئىتتىرىش(بنية k_دۆۋە *دۆۋە, دۆۋە_data_t data)
{
	صحيح ret = 0;
	k_spinlock_ئاچقۇچ_t ئاچقۇچ = k_spin_lock(&دۆۋە->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_دۆۋە, ئىتتىرىش, دۆۋە);

	إذا (دۆۋە->next == دۆۋە->چوققا) {
		ret = -ENOMEM;
		اذهب out;
	}

	إذا (z_sched_wake(&دۆۋە->wait_q, 0, (فراغ *)data)) {
		z_reschedule(&دۆۋە->lock, ئاچقۇچ);
		اذهب end;
	} وإلا {
		*(دۆۋە->next) = data;
		دۆۋە->next++;
		اذهب out;
	}

out:
	k_spin_unlock(&دۆۋە->lock, ئاچقۇچ);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_دۆۋە, ئىتتىرىش, دۆۋە, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_دۆۋە_ئىتتىرىش(بنية k_دۆۋە *دۆۋە, دۆۋە_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(دۆۋە, K_OBJ_دۆۋە));

	أرجع z_impl_k_دۆۋە_ئىتتىرىش(دۆۋە, data);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_دۆۋە_چىقىرىش(بنية k_دۆۋە *دۆۋە, دۆۋە_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_ئاچقۇچ_t ئاچقۇچ;
	صحيح result;

	ئاچقۇچ = k_spin_lock(&دۆۋە->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_دۆۋە, چىقىرىش, دۆۋە, timeout);

	إذا (likely(دۆۋە->next > دۆۋە->base)) {
		دۆۋە->next--;
		*data = *(دۆۋە->next);
		k_spin_unlock(&دۆۋە->lock, ئاچقۇچ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_دۆۋە, چىقىرىش, دۆۋە, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&دۆۋە->lock, ئاچقۇچ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_دۆۋە, چىقىرىش, دۆۋە, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_دۆۋە, چىقىرىش, دۆۋە, timeout);

	result = z_pend_curr(&دۆۋە->lock, ئاچقۇچ, &دۆۋە->wait_q, timeout);
	إذا (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_دۆۋە, چىقىرىش, دۆۋە, timeout, result);

		أرجع result;
	}

	*data = (دۆۋە_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_دۆۋە, چىقىرىش, دۆۋە, timeout, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_دۆۋە_چىقىرىش(بنية k_دۆۋە *دۆۋە,
				     دۆۋە_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(دۆۋە, K_OBJ_دۆۋە));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, حجم(دۆۋە_data_t)));
	أرجع z_impl_k_دۆۋە_چىقىرىش(دۆۋە, data, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
