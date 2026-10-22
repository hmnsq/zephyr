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

#ifdef CONFIG_OBJ_CORE_تيندنن
K_OBJ_TYPE_DEFINE(obj_type_تيندنن, k_تيندنن, K_OBJ_TYPE_تيندنن_ID, عدم);
#endif /* CONFIG_OBJ_CORE_STACK */

فراغ k_تيندنن_init(بنية k_تيندنن *تيندنن, تيندنن_data_t *buffer,
		  uint32_t اڠک_entries)
{
	z_waitq_init(&تيندنن->wait_q);
	تيندنن->lock = (بنية k_spinlock) {};
	تيندنن->next = buffer;
	تيندنن->base = buffer;
	تيندنن->اتس = تيندنن->base + اڠک_entries;
	تيندنن->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_تيندنن, تيندنن);
	k_object_init(تيندنن);

#ifdef CONFIG_OBJ_CORE_تيندنن
	k_obj_core_init_and_link(K_OBJ_CORE(تيندنن), &obj_type_تيندنن);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_تيندنن_alloc_init(بنية k_تيندنن *تيندنن, uint32_t اڠک_entries)
{
	فراغ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تيندنن, alloc_init, تيندنن);

	/* Reject allocation sizes that cannot be represented. */
	إذا (size_mul_overflow(اڠک_entries, حجم(تيندنن_data_t), &total_size)) {
		ret = -ENOMEM;
	} وإلا {
		buffer = z_thread_malloc(total_size);
		إذا (buffer != عدم) {
			k_تيندنن_init(تيندنن, buffer, اڠک_entries);
			تيندنن->flags = K_تيندنن_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تيندنن, alloc_init, تيندنن, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_تيندنن_alloc_init(بنية k_تيندنن *تيندنن,
					      uint32_t اڠک_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(تيندنن, K_OBJ_تيندنن));
	K_OOPS(K_SYSCALL_VERIFY(اڠک_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(اڠک_entries, حجم(تيندنن_data_t),
					&total_size)));
	أرجع z_impl_k_تيندنن_alloc_init(تيندنن, اڠک_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_تيندنن_cleanup(بنية k_تيندنن *تيندنن, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تيندنن, cleanup, تيندنن);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کونچي_t کونچي = k_spin_lock(&تيندنن->lock);

	إذا (locked && (z_waitq_head_locked(&تيندنن->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا (!locked && (z_waitq_head(&تيندنن->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا ((تيندنن->flags & K_تيندنن_FLAG_ALLOC) != (uint8_t)0) {
		k_free(تيندنن->base);
		تيندنن->base = عدم;
		تيندنن->flags &= ~K_تيندنن_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&تيندنن->lock, کونچي);

#ifdef CONFIG_OBJ_CORE_تيندنن
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(تيندنن));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تيندنن, cleanup, تيندنن, ret);

	أرجع ret;
}

صحيح k_تيندنن_cleanup(بنية k_تيندنن *تيندنن)
{
	أرجع z_تيندنن_cleanup(تيندنن, false);
}

صحيح z_impl_k_تيندنن_تولق(بنية k_تيندنن *تيندنن, تيندنن_data_t data)
{
	صحيح ret = 0;
	k_spinlock_کونچي_t کونچي = k_spin_lock(&تيندنن->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تيندنن, تولق, تيندنن);

	إذا (تيندنن->next == تيندنن->اتس) {
		ret = -ENOMEM;
		اذهب out;
	}

	إذا (z_sched_wake(&تيندنن->wait_q, 0, (فراغ *)data)) {
		z_reschedule(&تيندنن->lock, کونچي);
		اذهب end;
	} وإلا {
		*(تيندنن->next) = data;
		تيندنن->next++;
		اذهب out;
	}

out:
	k_spin_unlock(&تيندنن->lock, کونچي);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تيندنن, تولق, تيندنن, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_تيندنن_تولق(بنية k_تيندنن *تيندنن, تيندنن_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(تيندنن, K_OBJ_تيندنن));

	أرجع z_impl_k_تيندنن_تولق(تيندنن, data);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_تيندنن_کلوار(بنية k_تيندنن *تيندنن, تيندنن_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_کونچي_t کونچي;
	صحيح result;

	کونچي = k_spin_lock(&تيندنن->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_تيندنن, کلوار, تيندنن, timeout);

	إذا (likely(تيندنن->next > تيندنن->base)) {
		تيندنن->next--;
		*data = *(تيندنن->next);
		k_spin_unlock(&تيندنن->lock, کونچي);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تيندنن, کلوار, تيندنن, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&تيندنن->lock, کونچي);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تيندنن, کلوار, تيندنن, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_تيندنن, کلوار, تيندنن, timeout);

	result = z_pend_curr(&تيندنن->lock, کونچي, &تيندنن->wait_q, timeout);
	إذا (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تيندنن, کلوار, تيندنن, timeout, result);

		أرجع result;
	}

	*data = (تيندنن_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_تيندنن, کلوار, تيندنن, timeout, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_تيندنن_کلوار(بنية k_تيندنن *تيندنن,
				     تيندنن_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(تيندنن, K_OBJ_تيندنن));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, حجم(تيندنن_data_t)));
	أرجع z_impl_k_تيندنن_کلوار(تيندنن, data, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
