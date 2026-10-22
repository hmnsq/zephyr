/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <scheduler.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <stdbool.h>

#ifdef CONFIG_OBJ_CORE_അട്ടി
K_OBJ_TYPE_DEFINE(obj_type_അട്ടി, k_അട്ടി, K_OBJ_TYPE_അട്ടി_ID, ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_STACK */

ശൂന്യം k_അട്ടി_init(ഘടന k_അട്ടി *അട്ടി, അട്ടി_data_t *buffer,
		  uint32_t അക്കം_entries)
{
	z_waitq_init(&അട്ടി->wait_q);
	അട്ടി->lock = (ഘടന k_spinlock) {};
	അട്ടി->next = buffer;
	അട്ടി->base = buffer;
	അട്ടി->മുകൾ = അട്ടി->base + അക്കം_entries;
	അട്ടി->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_അട്ടി, അട്ടി);
	k_object_init(അട്ടി);

#ifdef CONFIG_OBJ_CORE_അട്ടി
	k_obj_core_init_and_link(K_OBJ_CORE(അട്ടി), &obj_type_അട്ടി);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_അട്ടി_alloc_init(ഘടന k_അട്ടി *അട്ടി, uint32_t അക്കം_entries)
{
	ശൂന്യം *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_അട്ടി, alloc_init, അട്ടി);

	/* Reject allocation sizes that cannot be represented. */
	എങ്കിൽ (size_mul_overflow(അക്കം_entries, വലുപ്പം(അട്ടി_data_t), &total_size)) {
		ret = -ENOMEM;
	} അല്ലെങ്കിൽ {
		buffer = z_thread_malloc(total_size);
		എങ്കിൽ (buffer != ശൂന്യ) {
			k_അട്ടി_init(അട്ടി, buffer, അക്കം_entries);
			അട്ടി->flags = K_അട്ടി_FLAG_ALLOC;
			ret = 0;
		} അല്ലെങ്കിൽ {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_അട്ടി, alloc_init, അട്ടി, ret);

	തിരികെ ret;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ int32_t z_vrfy_k_അട്ടി_alloc_init(ഘടന k_അട്ടി *അട്ടി,
					      uint32_t അക്കം_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(അട്ടി, K_OBJ_അട്ടി));
	K_OOPS(K_SYSCALL_VERIFY(അക്കം_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(അക്കം_entries, വലുപ്പം(അട്ടി_data_t),
					&total_size)));
	തിരികെ z_impl_k_അട്ടി_alloc_init(അട്ടി, അക്കം_entries);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_അട്ടി_cleanup(ഘടന k_അട്ടി *അട്ടി, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_അട്ടി, cleanup, അട്ടി);

	പൂർണ്ണസംഖ്യ ret = 0;
	bool freed = false;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&അട്ടി->lock);

	എങ്കിൽ (locked && (z_waitq_head_locked(&അട്ടി->wait_q) != ശൂന്യ)) {
		ret = -EAGAIN;
		പോകുക out;
	}

	എങ്കിൽ (!locked && (z_waitq_head(&അട്ടി->wait_q) != ശൂന്യ)) {
		ret = -EAGAIN;
		പോകുക out;
	}

	എങ്കിൽ ((അട്ടി->flags & K_അട്ടി_FLAG_ALLOC) != (uint8_t)0) {
		k_free(അട്ടി->base);
		അട്ടി->base = ശൂന്യ;
		അട്ടി->flags &= ~K_അട്ടി_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&അട്ടി->lock, താക്കോൽ);

#ifdef CONFIG_OBJ_CORE_അട്ടി
	എങ്കിൽ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(അട്ടി));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_അട്ടി, cleanup, അട്ടി, ret);

	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_അട്ടി_cleanup(ഘടന k_അട്ടി *അട്ടി)
{
	തിരികെ z_അട്ടി_cleanup(അട്ടി, false);
}

പൂർണ്ണസംഖ്യ z_impl_k_അട്ടി_തള്ളുക(ഘടന k_അട്ടി *അട്ടി, അട്ടി_data_t data)
{
	പൂർണ്ണസംഖ്യ ret = 0;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&അട്ടി->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_അട്ടി, തള്ളുക, അട്ടി);

	എങ്കിൽ (അട്ടി->next == അട്ടി->മുകൾ) {
		ret = -ENOMEM;
		പോകുക out;
	}

	എങ്കിൽ (z_sched_wake(&അട്ടി->wait_q, 0, (ശൂന്യം *)data)) {
		z_reschedule(&അട്ടി->lock, താക്കോൽ);
		പോകുക end;
	} അല്ലെങ്കിൽ {
		*(അട്ടി->next) = data;
		അട്ടി->next++;
		പോകുക out;
	}

out:
	k_spin_unlock(&അട്ടി->lock, താക്കോൽ);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_അട്ടി, തള്ളുക, അട്ടി, ret);

	തിരികെ ret;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_അട്ടി_തള്ളുക(ഘടന k_അട്ടി *അട്ടി, അട്ടി_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(അട്ടി, K_OBJ_അട്ടി));

	തിരികെ z_impl_k_അട്ടി_തള്ളുക(അട്ടി, data);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_അട്ടി_എടുക്കുക(ഘടന k_അട്ടി *അട്ടി, അട്ടി_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ result;

	താക്കോൽ = k_spin_lock(&അട്ടി->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_അട്ടി, എടുക്കുക, അട്ടി, timeout);

	എങ്കിൽ (likely(അട്ടി->next > അട്ടി->base)) {
		അട്ടി->next--;
		*data = *(അട്ടി->next);
		k_spin_unlock(&അട്ടി->lock, താക്കോൽ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_അട്ടി, എടുക്കുക, അട്ടി, timeout, 0);

		തിരികെ 0;
	}

	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&അട്ടി->lock, താക്കോൽ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_അട്ടി, എടുക്കുക, അട്ടി, timeout, -EBUSY);

		തിരികെ -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_അട്ടി, എടുക്കുക, അട്ടി, timeout);

	result = z_pend_curr(&അട്ടി->lock, താക്കോൽ, &അട്ടി->wait_q, timeout);
	എങ്കിൽ (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_അട്ടി, എടുക്കുക, അട്ടി, timeout, result);

		തിരികെ result;
	}

	*data = (അട്ടി_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_അട്ടി, എടുക്കുക, അട്ടി, timeout, 0);

	തിരികെ 0;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_അട്ടി_എടുക്കുക(ഘടന k_അട്ടി *അട്ടി,
				     അട്ടി_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(അട്ടി, K_OBJ_അട്ടി));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, വലുപ്പം(അട്ടി_data_t)));
	തിരികെ z_impl_k_അട്ടി_എടുക്കുക(അട്ടി, data, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
