/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <scheduler.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <stdbool.h>

#ifdef CONFIG_OBJ_CORE_ઢગલો
K_OBJ_TYPE_DEFINE(obj_type_ઢગલો, k_ઢગલો, K_OBJ_TYPE_ઢગલો_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_STACK */

શૂન્ય k_ઢગલો_init(સંરચના k_ઢગલો *ઢગલો, ઢગલો_data_t *buffer,
		  uint32_t અંક_entries)
{
	z_waitq_init(&ઢગલો->wait_q);
	ઢગલો->lock = (સંરચના k_spinlock) {};
	ઢગલો->next = buffer;
	ઢગલો->base = buffer;
	ઢગલો->ટોચ = ઢગલો->base + અંક_entries;
	ઢગલો->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_ઢગલો, ઢગલો);
	k_object_init(ઢગલો);

#ifdef CONFIG_OBJ_CORE_ઢગલો
	k_obj_core_init_and_link(K_OBJ_CORE(ઢગલો), &obj_type_ઢગલો);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ઢગલો_alloc_init(સંરચના k_ઢગલો *ઢગલો, uint32_t અંક_entries)
{
	શૂન્ય *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ઢગલો, alloc_init, ઢગલો);

	/* Reject allocation sizes that cannot be represented. */
	જો (size_mul_overflow(અંક_entries, કદ(ઢગલો_data_t), &total_size)) {
		ret = -ENOMEM;
	} નહીંતર {
		buffer = z_thread_malloc(total_size);
		જો (buffer != ખાલી) {
			k_ઢગલો_init(ઢગલો, buffer, અંક_entries);
			ઢગલો->flags = K_ઢગલો_FLAG_ALLOC;
			ret = 0;
		} નહીંતર {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ઢગલો, alloc_init, ઢગલો, ret);

	પાછા ret;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ int32_t z_vrfy_k_ઢગલો_alloc_init(સંરચના k_ઢગલો *ઢગલો,
					      uint32_t અંક_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ઢગલો, K_OBJ_ઢગલો));
	K_OOPS(K_SYSCALL_VERIFY(અંક_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(અંક_entries, કદ(ઢગલો_data_t),
					&total_size)));
	પાછા z_impl_k_ઢગલો_alloc_init(ઢગલો, અંક_entries);
}
#અન્તર્ભાવ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_ઢગલો_cleanup(સંરચના k_ઢગલો *ઢગલો, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ઢગલો, cleanup, ઢગલો);

	પૂર્ણાંક ret = 0;
	bool freed = false;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&ઢગલો->lock);

	જો (locked && (z_waitq_head_locked(&ઢગલો->wait_q) != ખાલી)) {
		ret = -EAGAIN;
		જાઓ out;
	}

	જો (!locked && (z_waitq_head(&ઢગલો->wait_q) != ખાલી)) {
		ret = -EAGAIN;
		જાઓ out;
	}

	જો ((ઢગલો->flags & K_ઢગલો_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ઢગલો->base);
		ઢગલો->base = ખાલી;
		ઢગલો->flags &= ~K_ઢગલો_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&ઢગલો->lock, ચાવી);

#ifdef CONFIG_OBJ_CORE_ઢગલો
	જો (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ઢગલો));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ઢગલો, cleanup, ઢગલો, ret);

	પાછા ret;
}

પૂર્ણાંક k_ઢગલો_cleanup(સંરચના k_ઢગલો *ઢગલો)
{
	પાછા z_ઢગલો_cleanup(ઢગલો, false);
}

પૂર્ણાંક z_impl_k_ઢગલો_ધકેલો(સંરચના k_ઢગલો *ઢગલો, ઢગલો_data_t data)
{
	પૂર્ણાંક ret = 0;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&ઢગલો->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ઢગલો, ધકેલો, ઢગલો);

	જો (ઢગલો->next == ઢગલો->ટોચ) {
		ret = -ENOMEM;
		જાઓ out;
	}

	જો (z_sched_wake(&ઢગલો->wait_q, 0, (શૂન્ય *)data)) {
		z_reschedule(&ઢગલો->lock, ચાવી);
		જાઓ end;
	} નહીંતર {
		*(ઢગલો->next) = data;
		ઢગલો->next++;
		જાઓ out;
	}

out:
	k_spin_unlock(&ઢગલો->lock, ચાવી);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ઢગલો, ધકેલો, ઢગલો, ret);

	પાછા ret;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_ઢગલો_ધકેલો(સંરચના k_ઢગલો *ઢગલો, ઢગલો_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(ઢગલો, K_OBJ_ઢગલો));

	પાછા z_impl_k_ઢગલો_ધકેલો(ઢગલો, data);
}
#અન્તર્ભાવ <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_ઢગલો_કાઢો(સંરચના k_ઢગલો *ઢગલો, ઢગલો_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક result;

	ચાવી = k_spin_lock(&ઢગલો->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ઢગલો, કાઢો, ઢગલો, timeout);

	જો (likely(ઢગલો->next > ઢગલો->base)) {
		ઢગલો->next--;
		*data = *(ઢગલો->next);
		k_spin_unlock(&ઢગલો->lock, ચાવી);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ઢગલો, કાઢો, ઢગલો, timeout, 0);

		પાછા 0;
	}

	જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ઢગલો->lock, ચાવી);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ઢગલો, કાઢો, ઢગલો, timeout, -EBUSY);

		પાછા -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_ઢગલો, કાઢો, ઢગલો, timeout);

	result = z_pend_curr(&ઢગલો->lock, ચાવી, &ઢગલો->wait_q, timeout);
	જો (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ઢગલો, કાઢો, ઢગલો, timeout, result);

		પાછા result;
	}

	*data = (ઢગલો_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ઢગલો, કાઢો, ઢગલો, timeout, 0);

	પાછા 0;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_ઢગલો_કાઢો(સંરચના k_ઢગલો *ઢગલો,
				     ઢગલો_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ઢગલો, K_OBJ_ઢગલો));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, કદ(ઢગલો_data_t)));
	પાછા z_impl_k_ઢગલો_કાઢો(ઢગલો, data, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
