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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_ರಾಶಿ
K_OBJ_TYPE_DEFINE(obj_type_ರಾಶಿ, k_ರಾಶಿ_2, K_OBJ_TYPE_ರಾಶಿ_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STACK */

ಶೂನ್ಯ k_ರಾಶಿ_init_2(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ, ರಾಶಿ_ದತ್ತಾಂಶ_t *ಬಫರ್,
		  uint32_t ಅಂಕೆ_entries)
{
	z_waitq_init(&ರಾಶಿ->wait_q);
	ರಾಶಿ->ಬೀಗ = (ರಚನೆ k_spinlock) {};
	ರಾಶಿ->ಮುಂದಿನ = ಬಫರ್;
	ರಾಶಿ->ತಳ = ಬಫರ್;
	ರಾಶಿ->ತುದಿ = ರಾಶಿ->ತಳ + ಅಂಕೆ_entries;
	ರಾಶಿ->flags = 0;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_ರಾಶಿ_2, ರಾಶಿ);
	k_object_init(ರಾಶಿ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_ರಾಶಿ
	k_obj_core_init_and_link(K_OBJ_CORE(ರಾಶಿ), &obj_type_ರಾಶಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ರಾಶಿ_alloc_init(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ, uint32_t ಅಂಕೆ_entries)
{
	ಶೂನ್ಯ *ಬಫರ್;
	size_t ಒಟ್ಟು_ಗಾತ್ರ;
	int32_t ret;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_2, alloc_init, ರಾಶಿ);

	/* Reject allocation sizes that cannot be represented. */
	ಆದರೆ (ಗಾತ್ರ_mul_ಅತಿಪ್ರವಾಹ(ಅಂಕೆ_entries, ಗಾತ್ರ(ರಾಶಿ_ದತ್ತಾಂಶ_t), &ಒಟ್ಟು_ಗಾತ್ರ)) {
		ret = -ENOMEM;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಬಫರ್ = z_thread_malloc(ಒಟ್ಟು_ಗಾತ್ರ);
		ಆದರೆ (ಬಫರ್ != ಖಾಲಿ) {
			k_ರಾಶಿ_init_2(ರಾಶಿ, ಬಫರ್, ಅಂಕೆ_entries);
			ರಾಶಿ->flags = K_ರಾಶಿ_FLAG_ALLOC;
			ret = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ret = -ENOMEM;
		}
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_2, alloc_init, ರಾಶಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ int32_t z_vrfy_k_ರಾಶಿ_alloc_init(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ,
					      uint32_t ಅಂಕೆ_entries)
{
	size_t ಒಟ್ಟು_ಗಾತ್ರ;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ರಾಶಿ, K_OBJ_ರಾಶಿ));
	K_OOPS(K_SYSCALL_VERIFY(ಅಂಕೆ_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!ಗಾತ್ರ_mul_ಅತಿಪ್ರವಾಹ(ಅಂಕೆ_entries, ಗಾತ್ರ(ರಾಶಿ_ದತ್ತಾಂಶ_t),
					&ಒಟ್ಟು_ಗಾತ್ರ)));
	ಹಿಂತಿರುಗಿ z_impl_k_ರಾಶಿ_alloc_init(ರಾಶಿ, ಅಂಕೆ_entries);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_ರಾಶಿ_cleanup(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ, __maybe_unused bool locked)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_2, cleanup, ರಾಶಿ);

	ಪೂರ್ಣಾಂಕ ret = 0;
	bool freed = false;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ->ಬೀಗ);

	ಆದರೆ (locked && (z_waitq_ತಲೆ_locked(&ರಾಶಿ->wait_q) != ಖಾಲಿ)) {
		ret = -EAGAIN;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (!locked && (z_waitq_ತಲೆ(&ರಾಶಿ->wait_q) != ಖಾಲಿ)) {
		ret = -EAGAIN;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ ((ರಾಶಿ->flags & K_ರಾಶಿ_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ರಾಶಿ->ತಳ);
		ರಾಶಿ->ತಳ = ಖಾಲಿ;
		ರಾಶಿ->flags &= ~K_ರಾಶಿ_FLAG_ALLOC;
		freed = true;
	}

ಹೊರಹೋಗು:
	k_spin_unlock(&ರಾಶಿ->ಬೀಗ, ಕೀಲಿ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_ರಾಶಿ
	ಆದರೆ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ರಾಶಿ));
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STACK */
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_2, cleanup, ರಾಶಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_ರಾಶಿ_cleanup(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ)
{
	ಹಿಂತಿರುಗಿ z_ರಾಶಿ_cleanup(ರಾಶಿ, false);
}

ಪೂರ್ಣಾಂಕ z_impl_k_ರಾಶಿ_ತಳ್ಳು(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ, ರಾಶಿ_ದತ್ತಾಂಶ_t ದತ್ತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_2, ತಳ್ಳು, ರಾಶಿ);

	ಆದರೆ (ರಾಶಿ->ಮುಂದಿನ == ರಾಶಿ->ತುದಿ) {
		ret = -ENOMEM;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (z_sched_wake(&ರಾಶಿ->wait_q, 0, (ಶೂನ್ಯ *)ದತ್ತಾಂಶ)) {
		z_reschedule(&ರಾಶಿ->ಬೀಗ, ಕೀಲಿ);
		ಹೋಗು ಕೊನೆ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		*(ರಾಶಿ->ಮುಂದಿನ) = ದತ್ತಾಂಶ;
		ರಾಶಿ->ಮುಂದಿನ++;
		ಹೋಗು ಹೊರಹೋಗು;
	}

ಹೊರಹೋಗು:
	k_spin_unlock(&ರಾಶಿ->ಬೀಗ, ಕೀಲಿ);

ಕೊನೆ:
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_2, ತಳ್ಳು, ರಾಶಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_ರಾಶಿ_ತಳ್ಳು(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ, ರಾಶಿ_ದತ್ತಾಂಶ_t ದತ್ತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(ರಾಶಿ, K_OBJ_ರಾಶಿ));

	ಹಿಂತಿರುಗಿ z_impl_k_ರಾಶಿ_ತಳ್ಳು(ರಾಶಿ, ದತ್ತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_stack_push_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_ರಾಶಿ_ತೆಗೆ(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ, ರಾಶಿ_ದತ್ತಾಂಶ_t *ದತ್ತಾಂಶ,
		       k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&ರಾಶಿ->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_ರಾಶಿ_2, ತೆಗೆ, ರಾಶಿ, timeout);

	ಆದರೆ (likely(ರಾಶಿ->ಮುಂದಿನ > ರಾಶಿ->ತಳ)) {
		ರಾಶಿ->ಮುಂದಿನ--;
		*ದತ್ತಾಂಶ = *(ರಾಶಿ->ಮುಂದಿನ);
		k_spin_unlock(&ರಾಶಿ->ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_2, ತೆಗೆ, ರಾಶಿ, timeout, 0);

		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ರಾಶಿ->ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_2, ತೆಗೆ, ರಾಶಿ, timeout, -EBUSY);

		ಹಿಂತಿರುಗಿ -EBUSY;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_ರಾಶಿ_2, ತೆಗೆ, ರಾಶಿ, timeout);

	ಫಲಿತಾಂಶ = z_pend_curr(&ರಾಶಿ->ಬೀಗ, ಕೀಲಿ, &ರಾಶಿ->wait_q, timeout);
	ಆದರೆ (ಫಲಿತಾಂಶ != 0) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_2, ತೆಗೆ, ರಾಶಿ, timeout, ಫಲಿತಾಂಶ);

		ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
	}

	*ದತ್ತಾಂಶ = (ರಾಶಿ_ದತ್ತಾಂಶ_t)_current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_ರಾಶಿ_2, ತೆಗೆ, ರಾಶಿ, timeout, 0);

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_ರಾಶಿ_ತೆಗೆ(ರಚನೆ k_ರಾಶಿ_2 *ರಾಶಿ,
				     ರಾಶಿ_ದತ್ತಾಂಶ_t *ದತ್ತಾಂಶ, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ರಾಶಿ, K_OBJ_ರಾಶಿ));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(ದತ್ತಾಂಶ, ಗಾತ್ರ(ರಾಶಿ_ದತ್ತಾಂಶ_t)));
	ಹಿಂತಿರುಗಿ z_impl_k_ರಾಶಿ_ತೆಗೆ(ರಾಶಿ, ದತ್ತಾಂಶ, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_stack_pop_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
