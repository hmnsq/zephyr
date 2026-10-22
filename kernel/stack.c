/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <scheduler.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <stdbool.h>

#ifdef CONFIG_OBJ_CORE_அடுக்கு
K_OBJ_TYPE_DEFINE(obj_type_அடுக்கு, k_அடுக்கு, K_OBJ_TYPE_அடுக்கு_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_STACK */

வெறுமை k_அடுக்கு_init(கட்டமைப்பு k_அடுக்கு *அடுக்கு, அடுக்கு_data_t *buffer,
		  uint32_t இலக்கம்_entries)
{
	z_waitq_init(&அடுக்கு->wait_q);
	அடுக்கு->lock = (கட்டமைப்பு k_spinlock) {};
	அடுக்கு->next = buffer;
	அடுக்கு->base = buffer;
	அடுக்கு->உச்சி = அடுக்கு->base + இலக்கம்_entries;
	அடுக்கு->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_அடுக்கு, அடுக்கு);
	k_object_init(அடுக்கு);

#ifdef CONFIG_OBJ_CORE_அடுக்கு
	k_obj_core_init_and_link(K_OBJ_CORE(அடுக்கு), &obj_type_அடுக்கு);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_அடுக்கு_alloc_init(கட்டமைப்பு k_அடுக்கு *அடுக்கு, uint32_t இலக்கம்_entries)
{
	வெறுமை *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_அடுக்கு, alloc_init, அடுக்கு);

	/* Reject allocation sizes that cannot be represented. */
	எனில் (size_mul_overflow(இலக்கம்_entries, அளவு(அடுக்கு_data_t), &total_size)) {
		ret = -ENOMEM;
	} இல்லையெனில் {
		buffer = z_thread_malloc(total_size);
		எனில் (buffer != வெற்று) {
			k_அடுக்கு_init(அடுக்கு, buffer, இலக்கம்_entries);
			அடுக்கு->flags = K_அடுக்கு_FLAG_ALLOC;
			ret = 0;
		} இல்லையெனில் {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_அடுக்கு, alloc_init, அடுக்கு, ret);

	திரும்பு ret;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி int32_t z_vrfy_k_அடுக்கு_alloc_init(கட்டமைப்பு k_அடுக்கு *அடுக்கு,
					      uint32_t இலக்கம்_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(அடுக்கு, K_OBJ_அடுக்கு));
	K_OOPS(K_SYSCALL_VERIFY(இலக்கம்_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(இலக்கம்_entries, அளவு(அடுக்கு_data_t),
					&total_size)));
	திரும்பு z_impl_k_அடுக்கு_alloc_init(அடுக்கு, இலக்கம்_entries);
}
#அந்தர்பாவ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_அடுக்கு_cleanup(கட்டமைப்பு k_அடுக்கு *அடுக்கு, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_அடுக்கு, cleanup, அடுக்கு);

	முழு ret = 0;
	bool freed = false;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&அடுக்கு->lock);

	எனில் (locked && (z_waitq_head_locked(&அடுக்கு->wait_q) != வெற்று)) {
		ret = -EAGAIN;
		செல் out;
	}

	எனில் (!locked && (z_waitq_head(&அடுக்கு->wait_q) != வெற்று)) {
		ret = -EAGAIN;
		செல் out;
	}

	எனில் ((அடுக்கு->flags & K_அடுக்கு_FLAG_ALLOC) != (uint8_t)0) {
		k_free(அடுக்கு->base);
		அடுக்கு->base = வெற்று;
		அடுக்கு->flags &= ~K_அடுக்கு_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&அடுக்கு->lock, சாவி);

#ifdef CONFIG_OBJ_CORE_அடுக்கு
	எனில் (freed) {
		k_obj_core_unlink(K_OBJ_CORE(அடுக்கு));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_அடுக்கு, cleanup, அடுக்கு, ret);

	திரும்பு ret;
}

முழு k_அடுக்கு_cleanup(கட்டமைப்பு k_அடுக்கு *அடுக்கு)
{
	திரும்பு z_அடுக்கு_cleanup(அடுக்கு, false);
}

முழு z_impl_k_அடுக்கு_தள்ளு(கட்டமைப்பு k_அடுக்கு *அடுக்கு, அடுக்கு_data_t data)
{
	முழு ret = 0;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&அடுக்கு->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_அடுக்கு, தள்ளு, அடுக்கு);

	எனில் (அடுக்கு->next == அடுக்கு->உச்சி) {
		ret = -ENOMEM;
		செல் out;
	}

	எனில் (z_sched_wake(&அடுக்கு->wait_q, 0, (வெறுமை *)data)) {
		z_reschedule(&அடுக்கு->lock, சாவி);
		செல் end;
	} இல்லையெனில் {
		*(அடுக்கு->next) = data;
		அடுக்கு->next++;
		செல் out;
	}

out:
	k_spin_unlock(&அடுக்கு->lock, சாவி);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_அடுக்கு, தள்ளு, அடுக்கு, ret);

	திரும்பு ret;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_அடுக்கு_தள்ளு(கட்டமைப்பு k_அடுக்கு *அடுக்கு, அடுக்கு_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(அடுக்கு, K_OBJ_அடுக்கு));

	திரும்பு z_impl_k_அடுக்கு_தள்ளு(அடுக்கு, data);
}
#அந்தர்பாவ <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_அடுக்கு_எடு(கட்டமைப்பு k_அடுக்கு *அடுக்கு, அடுக்கு_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_சாவி_t சாவி;
	முழு result;

	சாவி = k_spin_lock(&அடுக்கு->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_அடுக்கு, எடு, அடுக்கு, timeout);

	எனில் (likely(அடுக்கு->next > அடுக்கு->base)) {
		அடுக்கு->next--;
		*data = *(அடுக்கு->next);
		k_spin_unlock(&அடுக்கு->lock, சாவி);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_அடுக்கு, எடு, அடுக்கு, timeout, 0);

		திரும்பு 0;
	}

	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&அடுக்கு->lock, சாவி);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_அடுக்கு, எடு, அடுக்கு, timeout, -EBUSY);

		திரும்பு -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_அடுக்கு, எடு, அடுக்கு, timeout);

	result = z_pend_curr(&அடுக்கு->lock, சாவி, &அடுக்கு->wait_q, timeout);
	எனில் (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_அடுக்கு, எடு, அடுக்கு, timeout, result);

		திரும்பு result;
	}

	*data = (அடுக்கு_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_அடுக்கு, எடு, அடுக்கு, timeout, 0);

	திரும்பு 0;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_அடுக்கு_எடு(கட்டமைப்பு k_அடுக்கு *அடுக்கு,
				     அடுக்கு_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(அடுக்கு, K_OBJ_அடுக்கு));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, அளவு(அடுக்கு_data_t)));
	திரும்பு z_impl_k_அடுக்கு_எடு(அடுக்கு, data, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
