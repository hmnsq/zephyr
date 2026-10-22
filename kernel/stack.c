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

#ifdef CONFIG_OBJ_CORE_STACK
K_OBJ_TYPE_DEFINE(obj_type_stack, k_stack, K_OBJ_TYPE_STACK_ID, عدم);
#endif /* CONFIG_OBJ_CORE_STACK */

فراغ k_stack_init(بنية k_stack *stack, stack_data_t *buffer,
		  uint32_t num_entries)
{
	z_waitq_init(&stack->wait_q);
	stack->lock = (بنية k_spinlock) {};
	stack->next = buffer;
	stack->base = buffer;
	stack->top = stack->base + num_entries;
	stack->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_stack, stack);
	k_object_init(stack);

#ifdef CONFIG_OBJ_CORE_STACK
	k_obj_core_init_and_link(K_OBJ_CORE(stack), &obj_type_stack);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_stack_alloc_init(بنية k_stack *stack, uint32_t num_entries)
{
	فراغ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, alloc_init, stack);

	/* Reject allocation sizes that cannot be represented. */
	אם (size_mul_overflow(num_entries, حجم(stack_data_t), &total_size)) {
		ret = -ENOMEM;
	} אחרת {
		buffer = z_thread_malloc(total_size);
		אם (buffer != عدم) {
			k_stack_init(stack, buffer, num_entries);
			stack->flags = K_STACK_FLAG_ALLOC;
			ret = 0;
		} אחרת {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, alloc_init, stack, ret);

	החזר ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_stack_alloc_init(بنية k_stack *stack,
					      uint32_t num_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(stack, K_OBJ_STACK));
	K_OOPS(K_SYSCALL_VERIFY(num_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(num_entries, حجم(stack_data_t),
					&total_size)));
	החזר z_impl_k_stack_alloc_init(stack, num_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_stack_cleanup(بنية k_stack *stack, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, cleanup, stack);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_key_t key = k_spin_lock(&stack->lock);

	אם (locked && (z_waitq_head_locked(&stack->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	אם (!locked && (z_waitq_head(&stack->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	אם ((stack->flags & K_STACK_FLAG_ALLOC) != (uint8_t)0) {
		k_free(stack->base);
		stack->base = عدم;
		stack->flags &= ~K_STACK_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&stack->lock, key);

#ifdef CONFIG_OBJ_CORE_STACK
	אם (freed) {
		k_obj_core_unlink(K_OBJ_CORE(stack));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, cleanup, stack, ret);

	החזר ret;
}

صحيح k_stack_cleanup(بنية k_stack *stack)
{
	החזר z_stack_cleanup(stack, false);
}

صحيح z_impl_k_stack_push(بنية k_stack *stack, stack_data_t data)
{
	صحيح ret = 0;
	k_spinlock_key_t key = k_spin_lock(&stack->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, push, stack);

	אם (stack->next == stack->top) {
		ret = -ENOMEM;
		اذهب out;
	}

	אם (z_sched_wake(&stack->wait_q, 0, (فراغ *)data)) {
		z_reschedule(&stack->lock, key);
		اذهب end;
	} אחרת {
		*(stack->next) = data;
		stack->next++;
		اذهب out;
	}

out:
	k_spin_unlock(&stack->lock, key);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, push, stack, ret);

	החזר ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_stack_push(بنية k_stack *stack, stack_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(stack, K_OBJ_STACK));

	החזר z_impl_k_stack_push(stack, data);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_stack_pop(بنية k_stack *stack, stack_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_key_t key;
	صحيح result;

	key = k_spin_lock(&stack->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, pop, stack, timeout);

	אם (likely(stack->next > stack->base)) {
		stack->next--;
		*data = *(stack->next);
		k_spin_unlock(&stack->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, 0);

		החזר 0;
	}

	אם (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&stack->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, -EBUSY);

		החזר -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_stack, pop, stack, timeout);

	result = z_pend_curr(&stack->lock, key, &stack->wait_q, timeout);
	אם (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, result);

		החזר result;
	}

	*data = (stack_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, 0);

	החזר 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_stack_pop(بنية k_stack *stack,
				     stack_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(stack, K_OBJ_STACK));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, حجم(stack_data_t)));
	החזר z_impl_k_stack_pop(stack, data, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
