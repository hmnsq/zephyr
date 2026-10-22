/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ifdef CONFIG_OBJ_CORE_ರಾಶಿ
K_OBJ_TYPE_DEFINE(obj_type_ರಾಶಿ, k_ರಾಶಿ, K_OBJ_TYPE_ರಾಶಿ_ID, ರಿಕ್ತ);
#endif /* CONFIG_OBJ_CORE_STACK */

ಶೂನ್ಯ k_ರಾಶಿ_init(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ, ರಾಶಿ_data_t *buffer,
		  uint32_t ಅಂಕೆ_entries)
{
	z_waitq_init(&ರಾಶಿ->wait_q);
	ರಾಶಿ->lock = (ಸಂರಚನಾ k_spinlock) {};
	ರಾಶಿ->next = buffer;
	ರಾಶಿ->base = buffer;
	ರಾಶಿ->ತುದಿ = ರಾಶಿ->base + ಅಂಕೆ_entries;
	ರಾಶಿ->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_ರಾಶಿ, ರಾಶಿ);
	k_object_init(ರಾಶಿ);

#ifdef CONFIG_OBJ_CORE_ರಾಶಿ
	k_obj_core_init_and_link(K_OBJ_CORE(ರಾಶಿ), &obj_type_ರಾಶಿ);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ರಾಶಿ_alloc_init(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ, uint32_t ಅಂಕೆ_entries)
{
	ಶೂನ್ಯ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, alloc_init, ರಾಶಿ);

	/* Reject allocation sizes that cannot be represented. */
	ಯದಿ (size_mul_overflow(ಅಂಕೆ_entries, ಪರಿಮಾಣ(ರಾಶಿ_data_t), &total_size)) {
		ret = -ENOMEM;
	} ಅನ್ಯಥಾ {
		buffer = z_thread_malloc(total_size);
		ಯದಿ (buffer != ರಿಕ್ತ) {
			k_ರಾಶಿ_init(ರಾಶಿ, buffer, ಅಂಕೆ_entries);
			ರಾಶಿ->flags = K_ರಾಶಿ_FLAG_ALLOC;
			ret = 0;
		} ಅನ್ಯಥಾ {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, alloc_init, ರಾಶಿ, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ int32_t z_vrfy_k_ರಾಶಿ_alloc_init(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ,
					      uint32_t ಅಂಕೆ_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ರಾಶಿ, K_OBJ_ರಾಶಿ));
	K_OOPS(K_SYSCALL_VERIFY(ಅಂಕೆ_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(ಅಂಕೆ_entries, ಪರಿಮಾಣ(ರಾಶಿ_data_t),
					&total_size)));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_ರಾಶಿ_alloc_init(ರಾಶಿ, ಅಂಕೆ_entries);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_ರಾಶಿ_cleanup(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, cleanup, ರಾಶಿ);

	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	bool freed = false;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&ರಾಶಿ->lock);

	ಯದಿ (locked && (z_waitq_head_locked(&ರಾಶಿ->wait_q) != ರಿಕ್ತ)) {
		ret = -EAGAIN;
		ಗಚ್ಛ out;
	}

	ಯದಿ (!locked && (z_waitq_head(&ರಾಶಿ->wait_q) != ರಿಕ್ತ)) {
		ret = -EAGAIN;
		ಗಚ್ಛ out;
	}

	ಯದಿ ((ರಾಶಿ->flags & K_ರಾಶಿ_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ರಾಶಿ->base);
		ರಾಶಿ->base = ರಿಕ್ತ;
		ರಾಶಿ->flags &= ~K_ರಾಶಿ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&ರಾಶಿ->lock, ಕೀಲಿ);

#ifdef CONFIG_OBJ_CORE_ರಾಶಿ
	ಯದಿ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ರಾಶಿ));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, cleanup, ರಾಶಿ, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_ರಾಶಿ_cleanup(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ)
{
	ಪ್ರತ್ಯಾಗಮ z_ರಾಶಿ_cleanup(ರಾಶಿ, false);
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_ರಾಶಿ_ತಳ್ಳು(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ, ರಾಶಿ_data_t data)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&ರಾಶಿ->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, ತಳ್ಳು, ರಾಶಿ);

	ಯದಿ (ರಾಶಿ->next == ರಾಶಿ->ತುದಿ) {
		ret = -ENOMEM;
		ಗಚ್ಛ out;
	}

	ಯದಿ (z_sched_wake(&ರಾಶಿ->wait_q, 0, (ಶೂನ್ಯ *)data)) {
		z_reschedule(&ರಾಶಿ->lock, ಕೀಲಿ);
		ಗಚ್ಛ end;
	} ಅನ್ಯಥಾ {
		*(ರಾಶಿ->next) = data;
		ರಾಶಿ->next++;
		ಗಚ್ಛ out;
	}

out:
	k_spin_unlock(&ರಾಶಿ->lock, ಕೀಲಿ);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, ತಳ್ಳು, ರಾಶಿ, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_ರಾಶಿ_ತಳ್ಳು(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ, ರಾಶಿ_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(ರಾಶಿ, K_OBJ_ರಾಶಿ));

	ಪ್ರತ್ಯಾಗಮ z_impl_k_ರಾಶಿ_ತಳ್ಳು(ರಾಶಿ, data);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_ರಾಶಿ_ತೆಗೆ(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ, ರಾಶಿ_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ result;

	ಕೀಲಿ = k_spin_lock(&ರಾಶಿ->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ, ತೆಗೆ, ರಾಶಿ, timeout);

	ಯದಿ (likely(ರಾಶಿ->next > ರಾಶಿ->base)) {
		ರಾಶಿ->next--;
		*data = *(ರಾಶಿ->next);
		k_spin_unlock(&ರಾಶಿ->lock, ಕೀಲಿ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, ತೆಗೆ, ರಾಶಿ, timeout, 0);

		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ರಾಶಿ->lock, ಕೀಲಿ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, ತೆಗೆ, ರಾಶಿ, timeout, -EBUSY);

		ಪ್ರತ್ಯಾಗಮ -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_ರಾಶಿ, ತೆಗೆ, ರಾಶಿ, timeout);

	result = z_pend_curr(&ರಾಶಿ->lock, ಕೀಲಿ, &ರಾಶಿ->wait_q, timeout);
	ಯದಿ (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, ತೆಗೆ, ರಾಶಿ, timeout, result);

		ಪ್ರತ್ಯಾಗಮ result;
	}

	*data = (ರಾಶಿ_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ, ತೆಗೆ, ರಾಶಿ, timeout, 0);

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_ರಾಶಿ_ತೆಗೆ(ಸಂರಚನಾ k_ರಾಶಿ *ರಾಶಿ,
				     ರಾಶಿ_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ರಾಶಿ, K_OBJ_ರಾಶಿ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, ಪರಿಮಾಣ(ರಾಶಿ_data_t)));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_ರಾಶಿ_ತೆಗೆ(ರಾಶಿ, data, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
