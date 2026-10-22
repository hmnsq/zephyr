/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#অন্তর্ভাৱ <zephyr/sys/math_extras.h>
#অন্তর্ভাৱ <zephyr/kernel.h>

#অন্তর্ভাৱ <zephyr/toolchain.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <scheduler.h>
#অন্তর্ভাৱ <wait_q.h>
#অন্তর্ভাৱ <zephyr/init.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <kernel_internal.h>
#অন্তর্ভাৱ <stdbool.h>

#ifdef CONFIG_OBJ_CORE_স্তূপ
K_OBJ_TYPE_DEFINE(obj_type_স্তূপ, k_স্তূপ, K_OBJ_TYPE_স্তূপ_ID, রিক্ত);
#endif /* CONFIG_OBJ_CORE_STACK */

শূন্য k_স্তূপ_init(সংরচনা k_স্তূপ *স্তূপ, স্তূপ_data_t *buffer,
		  uint32_t অঙ্ক_entries)
{
	z_waitq_init(&স্তূপ->wait_q);
	স্তূপ->lock = (সংরচনা k_spinlock) {};
	স্তূপ->next = buffer;
	স্তূপ->base = buffer;
	স্তূপ->শীৰ্ষ = স্তূপ->base + অঙ্ক_entries;
	স্তূপ->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_স্তূপ, স্তূপ);
	k_object_init(স্তূপ);

#ifdef CONFIG_OBJ_CORE_স্তূপ
	k_obj_core_init_and_link(K_OBJ_CORE(স্তূপ), &obj_type_স্তূপ);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_স্তূপ_alloc_init(সংরচনা k_স্তূপ *স্তূপ, uint32_t অঙ্ক_entries)
{
	শূন্য *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, alloc_init, স্তূপ);

	/* Reject allocation sizes that cannot be represented. */
	যদি (size_mul_overflow(অঙ্ক_entries, পরিমাণ(স্তূপ_data_t), &total_size)) {
		ret = -ENOMEM;
	} অন্যথা {
		buffer = z_thread_malloc(total_size);
		যদি (buffer != রিক্ত) {
			k_স্তূপ_init(স্তূপ, buffer, অঙ্ক_entries);
			স্তূপ->flags = K_স্তূপ_FLAG_ALLOC;
			ret = 0;
		} অন্যথা {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, alloc_init, স্তূপ, ret);

	ঘূৰাই ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ int32_t z_vrfy_k_স্তূপ_alloc_init(সংরচনা k_স্তূপ *স্তূপ,
					      uint32_t অঙ্ক_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(স্তূপ, K_OBJ_স্তূপ));
	K_OOPS(K_SYSCALL_VERIFY(অঙ্ক_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(অঙ্ক_entries, পরিমাণ(স্তূপ_data_t),
					&total_size)));
	ঘূৰাই z_impl_k_স্তূপ_alloc_init(স্তূপ, অঙ্ক_entries);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_স্তূপ_cleanup(সংরচনা k_স্তূপ *স্তূপ, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, cleanup, স্তূপ);

	পূর্ণাঙ্ক ret = 0;
	bool freed = false;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&স্তূপ->lock);

	যদি (locked && (z_waitq_head_locked(&স্তূপ->wait_q) != রিক্ত)) {
		ret = -EAGAIN;
		গচ্ছ out;
	}

	যদি (!locked && (z_waitq_head(&স্তূপ->wait_q) != রিক্ত)) {
		ret = -EAGAIN;
		গচ্ছ out;
	}

	যদি ((স্তূপ->flags & K_স্তূপ_FLAG_ALLOC) != (uint8_t)0) {
		k_free(স্তূপ->base);
		স্তূপ->base = রিক্ত;
		স্তূপ->flags &= ~K_স্তূপ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&স্তূপ->lock, চাবি);

#ifdef CONFIG_OBJ_CORE_স্তূপ
	যদি (freed) {
		k_obj_core_unlink(K_OBJ_CORE(স্তূপ));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, cleanup, স্তূপ, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_স্তূপ_cleanup(সংরচনা k_স্তূপ *স্তূপ)
{
	ঘূৰাই z_স্তূপ_cleanup(স্তূপ, false);
}

পূর্ণাঙ্ক z_impl_k_স্তূপ_ঠেলো(সংরচনা k_স্তূপ *স্তূপ, স্তূপ_data_t data)
{
	পূর্ণাঙ্ক ret = 0;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&স্তূপ->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, ঠেলো, স্তূপ);

	যদি (স্তূপ->next == স্তূপ->শীৰ্ষ) {
		ret = -ENOMEM;
		গচ্ছ out;
	}

	যদি (z_sched_wake(&স্তূপ->wait_q, 0, (শূন্য *)data)) {
		z_reschedule(&স্তূপ->lock, চাবি);
		গচ্ছ end;
	} অন্যথা {
		*(স্তূপ->next) = data;
		স্তূপ->next++;
		গচ্ছ out;
	}

out:
	k_spin_unlock(&স্তূপ->lock, চাবি);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, ঠেলো, স্তূপ, ret);

	ঘূৰাই ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_স্তূপ_ঠেলো(সংরচনা k_স্তূপ *স্তূপ, স্তূপ_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(স্তূপ, K_OBJ_স্তূপ));

	ঘূৰাই z_impl_k_স্তূপ_ঠেলো(স্তূপ, data);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_স্তূপ_বেৰ_কৰো(সংরচনা k_স্তূপ *স্তূপ, স্তূপ_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক result;

	চাবি = k_spin_lock(&স্তূপ->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, বেৰ_কৰো, স্তূপ, timeout);

	যদি (likely(স্তূপ->next > স্তূপ->base)) {
		স্তূপ->next--;
		*data = *(স্তূপ->next);
		k_spin_unlock(&স্তূপ->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বেৰ_কৰো, স্তূপ, timeout, 0);

		ঘূৰাই 0;
	}

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&স্তূপ->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বেৰ_কৰো, স্তূপ, timeout, -EBUSY);

		ঘূৰাই -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_স্তূপ, বেৰ_কৰো, স্তূপ, timeout);

	result = z_pend_curr(&স্তূপ->lock, চাবি, &স্তূপ->wait_q, timeout);
	যদি (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বেৰ_কৰো, স্তূপ, timeout, result);

		ঘূৰাই result;
	}

	*data = (স্তূপ_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বেৰ_কৰো, স্তূপ, timeout, 0);

	ঘূৰাই 0;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_স্তূপ_বেৰ_কৰো(সংরচনা k_স্তূপ *স্তূপ,
				     স্তূপ_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(স্তূপ, K_OBJ_স্তূপ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, পরিমাণ(স্তূপ_data_t)));
	ঘূৰাই z_impl_k_স্তূপ_বেৰ_কৰো(স্তূপ, data, timeout);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
