/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#অন্তর্ভাব <zephyr/sys/math_extras.h>
#অন্তর্ভাব <zephyr/kernel.h>

#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <scheduler.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <stdbool.h>

#ifdef CONFIG_OBJ_CORE_স্তূপ
K_OBJ_TYPE_DEFINE(obj_type_স্তূপ, k_স্তূপ, K_OBJ_TYPE_স্তূপ_ID, ফাঁকা);
#endif /* CONFIG_OBJ_CORE_STACK */

শূন্য k_স্তূপ_init(কাঠামো k_স্তূপ *স্তূপ, স্তূপ_তথ্য_t *বাফার,
		  uint32_t অঙ্ক_entries)
{
	z_waitq_init(&স্তূপ->wait_q);
	স্তূপ->lock = (কাঠামো k_spinlock) {};
	স্তূপ->পরবর্তী = বাফার;
	স্তূপ->base = বাফার;
	স্তূপ->শীর্ষ_2 = স্তূপ->base + অঙ্ক_entries;
	স্তূপ->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_স্তূপ, স্তূপ);
	k_object_init(স্তূপ);

#ifdef CONFIG_OBJ_CORE_স্তূপ
	k_obj_core_init_and_link(K_OBJ_CORE(স্তূপ), &obj_type_স্তূপ);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_স্তূপ_alloc_init(কাঠামো k_স্তূপ *স্তূপ, uint32_t অঙ্ক_entries)
{
	শূন্য *বাফার;
	size_t যোগ_আকার;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, alloc_init, স্তূপ);

	/* Reject allocation sizes that cannot be represented. */
	যদি (আকার_mul_overflow(অঙ্ক_entries, আকার(স্তূপ_তথ্য_t), &যোগ_আকার)) {
		ret = -ENOMEM;
	} অন্যথা {
		বাফার = z_thread_malloc(যোগ_আকার);
		যদি (বাফার != ফাঁকা) {
			k_স্তূপ_init(স্তূপ, বাফার, অঙ্ক_entries);
			স্তূপ->flags = K_স্তূপ_FLAG_ALLOC;
			ret = 0;
		} অন্যথা {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, alloc_init, স্তূপ, ret);

	ফেরত ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ int32_t z_vrfy_k_স্তূপ_alloc_init(কাঠামো k_স্তূপ *স্তূপ,
					      uint32_t অঙ্ক_entries)
{
	size_t যোগ_আকার;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(স্তূপ, K_OBJ_স্তূপ));
	K_OOPS(K_SYSCALL_VERIFY(অঙ্ক_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!আকার_mul_overflow(অঙ্ক_entries, আকার(স্তূপ_তথ্য_t),
					&যোগ_আকার)));
	ফেরত z_impl_k_স্তূপ_alloc_init(স্তূপ, অঙ্ক_entries);
}
#অন্তর্ভাব <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_স্তূপ_cleanup(কাঠামো k_স্তূপ *স্তূপ, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, cleanup, স্তূপ);

	পূর্ণসংখ্যা ret = 0;
	bool freed = false;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&স্তূপ->lock);

	যদি (locked && (z_waitq_শীর্ষ_locked(&স্তূপ->wait_q) != ফাঁকা)) {
		ret = -EAGAIN;
		যাও out;
	}

	যদি (!locked && (z_waitq_শীর্ষ(&স্তূপ->wait_q) != ফাঁকা)) {
		ret = -EAGAIN;
		যাও out;
	}

	যদি ((স্তূপ->flags & K_স্তূপ_FLAG_ALLOC) != (uint8_t)0) {
		k_free(স্তূপ->base);
		স্তূপ->base = ফাঁকা;
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

	ফেরত ret;
}

পূর্ণসংখ্যা k_স্তূপ_cleanup(কাঠামো k_স্তূপ *স্তূপ)
{
	ফেরত z_স্তূপ_cleanup(স্তূপ, false);
}

পূর্ণসংখ্যা z_impl_k_স্তূপ_ঠেলো(কাঠামো k_স্তূপ *স্তূপ, স্তূপ_তথ্য_t তথ্য)
{
	পূর্ণসংখ্যা ret = 0;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&স্তূপ->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, ঠেলো, স্তূপ);

	যদি (স্তূপ->পরবর্তী == স্তূপ->শীর্ষ_2) {
		ret = -ENOMEM;
		যাও out;
	}

	যদি (z_sched_wake(&স্তূপ->wait_q, 0, (শূন্য *)তথ্য)) {
		z_reschedule(&স্তূপ->lock, চাবি);
		যাও end;
	} অন্যথা {
		*(স্তূপ->পরবর্তী) = তথ্য;
		স্তূপ->পরবর্তী++;
		যাও out;
	}

out:
	k_spin_unlock(&স্তূপ->lock, চাবি);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, ঠেলো, স্তূপ, ret);

	ফেরত ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_স্তূপ_ঠেলো(কাঠামো k_স্তূপ *স্তূপ, স্তূপ_তথ্য_t তথ্য)
{
	K_OOPS(K_SYSCALL_OBJ(স্তূপ, K_OBJ_স্তূপ));

	ফেরত z_impl_k_স্তূপ_ঠেলো(স্তূপ, তথ্য);
}
#অন্তর্ভাব <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_স্তূপ_বের_করো(কাঠামো k_স্তূপ *স্তূপ, স্তূপ_তথ্য_t *তথ্য,
		       k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ফলাফল;

	চাবি = k_spin_lock(&স্তূপ->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_স্তূপ, বের_করো, স্তূপ, timeout);

	যদি (likely(স্তূপ->পরবর্তী > স্তূপ->base)) {
		স্তূপ->পরবর্তী--;
		*তথ্য = *(স্তূপ->পরবর্তী);
		k_spin_unlock(&স্তূপ->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বের_করো, স্তূপ, timeout, 0);

		ফেরত 0;
	}

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&স্তূপ->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বের_করো, স্তূপ, timeout, -EBUSY);

		ফেরত -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_স্তূপ, বের_করো, স্তূপ, timeout);

	ফলাফল = z_pend_curr(&স্তূপ->lock, চাবি, &স্তূপ->wait_q, timeout);
	যদি (ফলাফল != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বের_করো, স্তূপ, timeout, ফলাফল);

		ফেরত ফলাফল;
	}

	*তথ্য = (স্তূপ_তথ্য_t)_current->base.swap_তথ্য;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_স্তূপ, বের_করো, স্তূপ, timeout, 0);

	ফেরত 0;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_স্তূপ_বের_করো(কাঠামো k_স্তূপ *স্তূপ,
				     স্তূপ_তথ্য_t *তথ্য, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(স্তূপ, K_OBJ_স্তূপ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(তথ্য, আকার(স্তূপ_তথ্য_t)));
	ফেরত z_impl_k_স্তূপ_বের_করো(স্তূপ, তথ্য, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
