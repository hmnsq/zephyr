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

#ifdef CONFIG_OBJ_CORE_ڈھیر
K_OBJ_TYPE_DEFINE(obj_type_ڈھیر, k_ڈھیر, K_OBJ_TYPE_ڈھیر_ID, عدم);
#endif /* CONFIG_OBJ_CORE_STACK */

فراغ k_ڈھیر_init(بنية k_ڈھیر *ڈھیر, ڈھیر_data_t *buffer,
		  uint32_t عدد_entries)
{
	z_waitq_init(&ڈھیر->wait_q);
	ڈھیر->lock = (بنية k_spinlock) {};
	ڈھیر->next = buffer;
	ڈھیر->base = buffer;
	ڈھیر->چوٹی = ڈھیر->base + عدد_entries;
	ڈھیر->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_ڈھیر, ڈھیر);
	k_object_init(ڈھیر);

#ifdef CONFIG_OBJ_CORE_ڈھیر
	k_obj_core_init_and_link(K_OBJ_CORE(ڈھیر), &obj_type_ڈھیر);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ڈھیر_alloc_init(بنية k_ڈھیر *ڈھیر, uint32_t عدد_entries)
{
	فراغ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, alloc_init, ڈھیر);

	/* Reject allocation sizes that cannot be represented. */
	إذا (size_mul_overflow(عدد_entries, حجم(ڈھیر_data_t), &total_size)) {
		ret = -ENOMEM;
	} وإلا {
		buffer = z_thread_malloc(total_size);
		إذا (buffer != عدم) {
			k_ڈھیر_init(ڈھیر, buffer, عدد_entries);
			ڈھیر->flags = K_ڈھیر_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, alloc_init, ڈھیر, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_ڈھیر_alloc_init(بنية k_ڈھیر *ڈھیر,
					      uint32_t عدد_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ڈھیر, K_OBJ_ڈھیر));
	K_OOPS(K_SYSCALL_VERIFY(عدد_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(عدد_entries, حجم(ڈھیر_data_t),
					&total_size)));
	أرجع z_impl_k_ڈھیر_alloc_init(ڈھیر, عدد_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_ڈھیر_cleanup(بنية k_ڈھیر *ڈھیر, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, cleanup, ڈھیر);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_lock(&ڈھیر->lock);

	إذا (locked && (z_waitq_head_locked(&ڈھیر->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا (!locked && (z_waitq_head(&ڈھیر->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا ((ڈھیر->flags & K_ڈھیر_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ڈھیر->base);
		ڈھیر->base = عدم;
		ڈھیر->flags &= ~K_ڈھیر_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&ڈھیر->lock, کلید);

#ifdef CONFIG_OBJ_CORE_ڈھیر
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ڈھیر));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, cleanup, ڈھیر, ret);

	أرجع ret;
}

صحيح k_ڈھیر_cleanup(بنية k_ڈھیر *ڈھیر)
{
	أرجع z_ڈھیر_cleanup(ڈھیر, false);
}

صحيح z_impl_k_ڈھیر_ڈالو(بنية k_ڈھیر *ڈھیر, ڈھیر_data_t data)
{
	صحيح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_lock(&ڈھیر->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, ڈالو, ڈھیر);

	إذا (ڈھیر->next == ڈھیر->چوٹی) {
		ret = -ENOMEM;
		اذهب out;
	}

	إذا (z_sched_wake(&ڈھیر->wait_q, 0, (فراغ *)data)) {
		z_reschedule(&ڈھیر->lock, کلید);
		اذهب end;
	} وإلا {
		*(ڈھیر->next) = data;
		ڈھیر->next++;
		اذهب out;
	}

out:
	k_spin_unlock(&ڈھیر->lock, کلید);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, ڈالو, ڈھیر, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ڈھیر_ڈالو(بنية k_ڈھیر *ڈھیر, ڈھیر_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(ڈھیر, K_OBJ_ڈھیر));

	أرجع z_impl_k_ڈھیر_ڈالو(ڈھیر, data);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_ڈھیر_نکالو(بنية k_ڈھیر *ڈھیر, ڈھیر_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحيح result;

	کلید = k_spin_lock(&ڈھیر->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ڈھیر, نکالو, ڈھیر, timeout);

	إذا (likely(ڈھیر->next > ڈھیر->base)) {
		ڈھیر->next--;
		*data = *(ڈھیر->next);
		k_spin_unlock(&ڈھیر->lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, نکالو, ڈھیر, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ڈھیر->lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, نکالو, ڈھیر, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_ڈھیر, نکالو, ڈھیر, timeout);

	result = z_pend_curr(&ڈھیر->lock, کلید, &ڈھیر->wait_q, timeout);
	إذا (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, نکالو, ڈھیر, timeout, result);

		أرجع result;
	}

	*data = (ڈھیر_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ڈھیر, نکالو, ڈھیر, timeout, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_ڈھیر_نکالو(بنية k_ڈھیر *ڈھیر,
				     ڈھیر_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ڈھیر, K_OBJ_ڈھیر));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, حجم(ڈھیر_data_t)));
	أرجع z_impl_k_ڈھیر_نکالو(ڈھیر, data, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
