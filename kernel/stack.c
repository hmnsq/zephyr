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

#ifdef CONFIG_OBJ_CORE_کەڵەکە
K_OBJ_TYPE_DEFINE(obj_type_کەڵەکە, k_کەڵەکە, K_OBJ_TYPE_کەڵەکە_ID, عدم);
#endif /* CONFIG_OBJ_CORE_STACK */

فراغ k_کەڵەکە_init(بنية k_کەڵەکە *کەڵەکە, کەڵەکە_data_t *buffer,
		  uint32_t ژمارە_entries)
{
	z_waitq_init(&کەڵەکە->wait_q);
	کەڵەکە->lock = (بنية k_spinlock) {};
	کەڵەکە->next = buffer;
	کەڵەکە->base = buffer;
	کەڵەکە->لووتکە = کەڵەکە->base + ژمارە_entries;
	کەڵەکە->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_کەڵەکە, کەڵەکە);
	k_object_init(کەڵەکە);

#ifdef CONFIG_OBJ_CORE_کەڵەکە
	k_obj_core_init_and_link(K_OBJ_CORE(کەڵەکە), &obj_type_کەڵەکە);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_کەڵەکە_alloc_init(بنية k_کەڵەکە *کەڵەکە, uint32_t ژمارە_entries)
{
	فراغ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_کەڵەکە, alloc_init, کەڵەکە);

	/* Reject allocation sizes that cannot be represented. */
	إذا (size_mul_overflow(ژمارە_entries, حجم(کەڵەکە_data_t), &total_size)) {
		ret = -ENOMEM;
	} وإلا {
		buffer = z_thread_malloc(total_size);
		إذا (buffer != عدم) {
			k_کەڵەکە_init(کەڵەکە, buffer, ژمارە_entries);
			کەڵەکە->flags = K_کەڵەکە_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_کەڵەکە, alloc_init, کەڵەکە, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_کەڵەکە_alloc_init(بنية k_کەڵەکە *کەڵەکە,
					      uint32_t ژمارە_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(کەڵەکە, K_OBJ_کەڵەکە));
	K_OOPS(K_SYSCALL_VERIFY(ژمارە_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(ژمارە_entries, حجم(کەڵەکە_data_t),
					&total_size)));
	أرجع z_impl_k_کەڵەکە_alloc_init(کەڵەکە, ژمارە_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_کەڵەکە_cleanup(بنية k_کەڵەکە *کەڵەکە, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_کەڵەکە, cleanup, کەڵەکە);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلیل_t کلیل = k_spin_lock(&کەڵەکە->lock);

	إذا (locked && (z_waitq_head_locked(&کەڵەکە->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا (!locked && (z_waitq_head(&کەڵەکە->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا ((کەڵەکە->flags & K_کەڵەکە_FLAG_ALLOC) != (uint8_t)0) {
		k_free(کەڵەکە->base);
		کەڵەکە->base = عدم;
		کەڵەکە->flags &= ~K_کەڵەکە_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&کەڵەکە->lock, کلیل);

#ifdef CONFIG_OBJ_CORE_کەڵەکە
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(کەڵەکە));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_کەڵەکە, cleanup, کەڵەکە, ret);

	أرجع ret;
}

صحيح k_کەڵەکە_cleanup(بنية k_کەڵەکە *کەڵەکە)
{
	أرجع z_کەڵەکە_cleanup(کەڵەکە, false);
}

صحيح z_impl_k_کەڵەکە_پاڵنان(بنية k_کەڵەکە *کەڵەکە, کەڵەکە_data_t data)
{
	صحيح ret = 0;
	k_spinlock_کلیل_t کلیل = k_spin_lock(&کەڵەکە->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_کەڵەکە, پاڵنان, کەڵەکە);

	إذا (کەڵەکە->next == کەڵەکە->لووتکە) {
		ret = -ENOMEM;
		اذهب out;
	}

	إذا (z_sched_wake(&کەڵەکە->wait_q, 0, (فراغ *)data)) {
		z_reschedule(&کەڵەکە->lock, کلیل);
		اذهب end;
	} وإلا {
		*(کەڵەکە->next) = data;
		کەڵەکە->next++;
		اذهب out;
	}

out:
	k_spin_unlock(&کەڵەکە->lock, کلیل);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_کەڵەکە, پاڵنان, کەڵەکە, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_کەڵەکە_پاڵنان(بنية k_کەڵەکە *کەڵەکە, کەڵەکە_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(کەڵەکە, K_OBJ_کەڵەکە));

	أرجع z_impl_k_کەڵەکە_پاڵنان(کەڵەکە, data);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_کەڵەکە_دەرهێنان(بنية k_کەڵەکە *کەڵەکە, کەڵەکە_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_کلیل_t کلیل;
	صحيح result;

	کلیل = k_spin_lock(&کەڵەکە->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_کەڵەکە, دەرهێنان, کەڵەکە, timeout);

	إذا (likely(کەڵەکە->next > کەڵەکە->base)) {
		کەڵەکە->next--;
		*data = *(کەڵەکە->next);
		k_spin_unlock(&کەڵەکە->lock, کلیل);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_کەڵەکە, دەرهێنان, کەڵەکە, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&کەڵەکە->lock, کلیل);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_کەڵەکە, دەرهێنان, کەڵەکە, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_کەڵەکە, دەرهێنان, کەڵەکە, timeout);

	result = z_pend_curr(&کەڵەکە->lock, کلیل, &کەڵەکە->wait_q, timeout);
	إذا (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_کەڵەکە, دەرهێنان, کەڵەکە, timeout, result);

		أرجع result;
	}

	*data = (کەڵەکە_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_کەڵەکە, دەرهێنان, کەڵەکە, timeout, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_کەڵەکە_دەرهێنان(بنية k_کەڵەکە *کەڵەکە,
				     کەڵەکە_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(کەڵەکە, K_OBJ_کەڵەکە));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, حجم(کەڵەکە_data_t)));
	أرجع z_impl_k_کەڵەکە_دەرهێنان(کەڵەکە, data, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
