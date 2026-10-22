/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>

#ifdef CONFIG_OBJ_CORE_ଗଦା
K_OBJ_TYPE_DEFINE(obj_type_ଗଦା, k_ଗଦା, K_OBJ_TYPE_ଗଦା_ID, ଖାଲି);
#endif /* CONFIG_OBJ_CORE_STACK */

ଶୂନ୍ୟ k_ଗଦା_init(ସଂରଚନା k_ଗଦା *ଗଦା, ଗଦା_data_t *buffer,
		  uint32_t ଅଙ୍କ_entries)
{
	z_waitq_init(&ଗଦା->wait_q);
	ଗଦା->lock = (ସଂରଚନା k_spinlock) {};
	ଗଦା->next = buffer;
	ଗଦା->base = buffer;
	ଗଦା->ଶୀର୍ଷ = ଗଦା->base + ଅଙ୍କ_entries;
	ଗଦା->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_ଗଦା, ଗଦା);
	k_object_init(ଗଦା);

#ifdef CONFIG_OBJ_CORE_ଗଦା
	k_obj_core_init_and_link(K_OBJ_CORE(ଗଦା), &obj_type_ଗଦା);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ଗଦା_alloc_init(ସଂରଚନା k_ଗଦା *ଗଦା, uint32_t ଅଙ୍କ_entries)
{
	ଶୂନ୍ୟ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ଗଦା, alloc_init, ଗଦା);

	/* Reject allocation sizes that cannot be represented. */
	ଯଦି (size_mul_overflow(ଅଙ୍କ_entries, ଆକାର(ଗଦା_data_t), &total_size)) {
		ret = -ENOMEM;
	} ଅନ୍ୟଥା {
		buffer = z_thread_malloc(total_size);
		ଯଦି (buffer != ଖାଲି) {
			k_ଗଦା_init(ଗଦା, buffer, ଅଙ୍କ_entries);
			ଗଦା->flags = K_ଗଦା_FLAG_ALLOC;
			ret = 0;
		} ଅନ୍ୟଥା {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ଗଦା, alloc_init, ଗଦା, ret);

	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ int32_t z_vrfy_k_ଗଦା_alloc_init(ସଂରଚନା k_ଗଦା *ଗଦା,
					      uint32_t ଅଙ୍କ_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ଗଦା, K_OBJ_ଗଦା));
	K_OOPS(K_SYSCALL_VERIFY(ଅଙ୍କ_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(ଅଙ୍କ_entries, ଆକାର(ଗଦା_data_t),
					&total_size)));
	ଫେରନ୍ତୁ z_impl_k_ଗଦା_alloc_init(ଗଦା, ଅଙ୍କ_entries);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_ଗଦା_cleanup(ସଂରଚନା k_ଗଦା *ଗଦା, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ଗଦା, cleanup, ଗଦା);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	bool freed = false;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&ଗଦା->lock);

	ଯଦି (locked && (z_waitq_head_locked(&ଗଦା->wait_q) != ଖାଲି)) {
		ret = -EAGAIN;
		ଯାଅ out;
	}

	ଯଦି (!locked && (z_waitq_head(&ଗଦା->wait_q) != ଖାଲି)) {
		ret = -EAGAIN;
		ଯାଅ out;
	}

	ଯଦି ((ଗଦା->flags & K_ଗଦା_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ଗଦା->base);
		ଗଦା->base = ଖାଲି;
		ଗଦା->flags &= ~K_ଗଦା_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&ଗଦା->lock, ଚାବି);

#ifdef CONFIG_OBJ_CORE_ଗଦା
	ଯଦି (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ଗଦା));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ଗଦା, cleanup, ଗଦା, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_ଗଦା_cleanup(ସଂରଚନା k_ଗଦା *ଗଦା)
{
	ଫେରନ୍ତୁ z_ଗଦା_cleanup(ଗଦା, false);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_ଗଦା_ଠେଲ(ସଂରଚନା k_ଗଦା *ଗଦା, ଗଦା_data_t data)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&ଗଦା->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ଗଦା, ଠେଲ, ଗଦା);

	ଯଦି (ଗଦା->next == ଗଦା->ଶୀର୍ଷ) {
		ret = -ENOMEM;
		ଯାଅ out;
	}

	ଯଦି (z_sched_wake(&ଗଦା->wait_q, 0, (ଶୂନ୍ୟ *)data)) {
		z_reschedule(&ଗଦା->lock, ଚାବି);
		ଯାଅ end;
	} ଅନ୍ୟଥା {
		*(ଗଦା->next) = data;
		ଗଦା->next++;
		ଯାଅ out;
	}

out:
	k_spin_unlock(&ଗଦା->lock, ଚାବି);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ଗଦା, ଠେଲ, ଗଦା, ret);

	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_ଗଦା_ଠେଲ(ସଂରଚନା k_ଗଦା *ଗଦା, ଗଦା_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(ଗଦା, K_OBJ_ଗଦା));

	ଫେରନ୍ତୁ z_impl_k_ଗଦା_ଠେଲ(ଗଦା, data);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_ଗଦା_କାଢ଼(ସଂରଚନା k_ଗଦା *ଗଦା, ଗଦା_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	ଚାବି = k_spin_lock(&ଗଦା->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ଗଦା, କାଢ଼, ଗଦା, timeout);

	ଯଦି (likely(ଗଦା->next > ଗଦା->base)) {
		ଗଦା->next--;
		*data = *(ଗଦା->next);
		k_spin_unlock(&ଗଦା->lock, ଚାବି);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ଗଦା, କାଢ଼, ଗଦା, timeout, 0);

		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ଗଦା->lock, ଚାବି);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ଗଦା, କାଢ଼, ଗଦା, timeout, -EBUSY);

		ଫେରନ୍ତୁ -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_ଗଦା, କାଢ଼, ଗଦା, timeout);

	result = z_pend_curr(&ଗଦା->lock, ଚାବି, &ଗଦା->wait_q, timeout);
	ଯଦି (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ଗଦା, କାଢ଼, ଗଦା, timeout, result);

		ଫେରନ୍ତୁ result;
	}

	*data = (ଗଦା_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ଗଦା, କାଢ଼, ଗଦା, timeout, 0);

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_ଗଦା_କାଢ଼(ସଂରଚନା k_ଗଦା *ଗଦା,
				     ଗଦା_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ଗଦା, K_OBJ_ଗଦା));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, ଆକାର(ଗଦା_data_t)));
	ଫେରନ୍ତୁ z_impl_k_ଗଦା_କାଢ଼(ଗଦା, data, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
