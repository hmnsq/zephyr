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

#ifdef CONFIG_OBJ_CORE_تر
K_OBJ_TYPE_DEFINE(obj_type_تر, k_تر, K_OBJ_TYPE_تر_ID, عدم);
#endif /* CONFIG_OBJ_CORE_STACK */

فراغ k_تر_init(بنية k_تر *تر, تر_data_t *buffer,
		  uint32_t لمب_entries)
{
	z_waitq_init(&تر->wait_q);
	تر->lock = (بنية k_spinlock) {};
	تر->next = buffer;
	تر->base = buffer;
	تر->كولي = تر->base + لمب_entries;
	تر->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_تر, تر);
	k_object_init(تر);

#ifdef CONFIG_OBJ_CORE_تر
	k_obj_core_init_and_link(K_OBJ_CORE(تر), &obj_type_تر);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_تر_alloc_init(بنية k_تر *تر, uint32_t لمب_entries)
{
	فراغ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تر, alloc_init, تر);

	/* Reject allocation sizes that cannot be represented. */
	إذا (size_mul_overflow(لمب_entries, حجم(تر_data_t), &total_size)) {
		ret = -ENOMEM;
	} وإلا {
		buffer = z_thread_malloc(total_size);
		إذا (buffer != عدم) {
			k_تر_init(تر, buffer, لمب_entries);
			تر->flags = K_تر_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تر, alloc_init, تر, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_تر_alloc_init(بنية k_تر *تر,
					      uint32_t لمب_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(تر, K_OBJ_تر));
	K_OOPS(K_SYSCALL_VERIFY(لمب_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(لمب_entries, حجم(تر_data_t),
					&total_size)));
	أرجع z_impl_k_تر_alloc_init(تر, لمب_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_تر_cleanup(بنية k_تر *تر, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تر, cleanup, تر);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_مبودي_t مبودي = k_spin_lock(&تر->lock);

	إذا (locked && (z_waitq_head_locked(&تر->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا (!locked && (z_waitq_head(&تر->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا ((تر->flags & K_تر_FLAG_ALLOC) != (uint8_t)0) {
		k_free(تر->base);
		تر->base = عدم;
		تر->flags &= ~K_تر_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&تر->lock, مبودي);

#ifdef CONFIG_OBJ_CORE_تر
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(تر));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تر, cleanup, تر, ret);

	أرجع ret;
}

صحيح k_تر_cleanup(بنية k_تر *تر)
{
	أرجع z_تر_cleanup(تر, false);
}

صحيح z_impl_k_تر_تور(بنية k_تر *تر, تر_data_t data)
{
	صحيح ret = 0;
	k_spinlock_مبودي_t مبودي = k_spin_lock(&تر->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تر, تور, تر);

	إذا (تر->next == تر->كولي) {
		ret = -ENOMEM;
		اذهب out;
	}

	إذا (z_sched_wake(&تر->wait_q, 0, (فراغ *)data)) {
		z_reschedule(&تر->lock, مبودي);
		اذهب end;
	} وإلا {
		*(تر->next) = data;
		تر->next++;
		اذهب out;
	}

out:
	k_spin_unlock(&تر->lock, مبودي);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تر, تور, تر, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_تر_تور(بنية k_تر *تر, تر_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(تر, K_OBJ_تر));

	أرجع z_impl_k_تر_تور(تر, data);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_تر_چرو(بنية k_تر *تر, تر_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_مبودي_t مبودي;
	صحيح result;

	مبودي = k_spin_lock(&تر->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تر, چرو, تر, timeout);

	إذا (likely(تر->next > تر->base)) {
		تر->next--;
		*data = *(تر->next);
		k_spin_unlock(&تر->lock, مبودي);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تر, چرو, تر, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&تر->lock, مبودي);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تر, چرو, تر, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_تر, چرو, تر, timeout);

	result = z_pend_curr(&تر->lock, مبودي, &تر->wait_q, timeout);
	إذا (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تر, چرو, تر, timeout, result);

		أرجع result;
	}

	*data = (تر_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تر, چرو, تر, timeout, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_تر_چرو(بنية k_تر *تر,
				     تر_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(تر, K_OBJ_تر));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, حجم(تر_data_t)));
	أرجع z_impl_k_تر_چرو(تر, data, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
