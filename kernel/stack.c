/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#include <zephyr/sys/math_extras.h>
#include <zephyr/kernel.h>

#include <zephyr/toolchain.h>
#include <ksched.h>
#include <scheduler.h>
#include <wait_q.h>
#include <zephyr/init.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_internal.h>
#include <stdbool.h>

#ifdef CONFIG_OBJ_CORE_STACK
K_OBJ_TYPE_DEFINE(obj_type_stack, k_stack, K_OBJ_TYPE_STACK_ID, NULL);
#endif /* CONFIG_OBJ_CORE_STACK */

void k_stack_init(sa_nracanaa k_stack *stack, stack_data_t *buffer,
		  uint32_t num_entries)
{
	z_waitq_init(&stack->wait_q);
	stack->lock = (sa_nracanaa k_spinlock) {};
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

int32_t z_impl_k_stack_alloc_init(sa_nracanaa k_stack *stack, uint32_t num_entries)
{
	void *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, alloc_init, stack);

	/* Reject allocation sizes that cannot be represented. */
	නම් (size_mul_overflow(num_entries, parimaa_na(stack_data_t), &total_size)) {
		ret = -ENOMEM;
	} නැතහොත් {
		buffer = z_thread_malloc(total_size);
		නම් (buffer != NULL) {
			k_stack_init(stack, buffer, num_entries);
			stack->flags = K_STACK_FLAG_ALLOC;
			ret = 0;
		} නැතහොත් {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, alloc_init, stack, ret);

	ආපසු ret;
}

#ifdef CONFIG_USERSPACE
static anta_hstha int32_t z_vrfy_k_stack_alloc_init(sa_nracanaa k_stack *stack,
					      uint32_t num_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(stack, K_OBJ_STACK));
	K_OOPS(K_SYSCALL_VERIFY(num_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(num_entries, parimaa_na(stack_data_t),
					&total_size)));
	ආපසු z_impl_k_stack_alloc_init(stack, num_entries);
}
#include <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

පූර්ණ_සංඛ්‍යා z_stack_cleanup(sa_nracanaa k_stack *stack, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, cleanup, stack);

	පූර්ණ_සංඛ්‍යා ret = 0;
	bool freed = false;
	k_spinlock_key_t key = k_spin_lock(&stack->lock);

	නම් (locked && (z_waitq_head_locked(&stack->wait_q) != NULL)) {
		ret = -EAGAIN;
		goto out;
	}

	නම් (!locked && (z_waitq_head(&stack->wait_q) != NULL)) {
		ret = -EAGAIN;
		goto out;
	}

	නම් ((stack->flags & K_STACK_FLAG_ALLOC) != (uint8_t)0) {
		k_free(stack->base);
		stack->base = NULL;
		stack->flags &= ~K_STACK_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&stack->lock, key);

#ifdef CONFIG_OBJ_CORE_STACK
	නම් (freed) {
		k_obj_core_unlink(K_OBJ_CORE(stack));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, cleanup, stack, ret);

	ආපසු ret;
}

පූර්ණ_සංඛ්‍යා k_stack_cleanup(sa_nracanaa k_stack *stack)
{
	ආපසු z_stack_cleanup(stack, false);
}

පූර්ණ_සංඛ්‍යා z_impl_k_stack_push(sa_nracanaa k_stack *stack, stack_data_t data)
{
	පූර්ණ_සංඛ්‍යා ret = 0;
	k_spinlock_key_t key = k_spin_lock(&stack->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, push, stack);

	නම් (stack->next == stack->top) {
		ret = -ENOMEM;
		goto out;
	}

	නම් (z_sched_wake(&stack->wait_q, 0, (void *)data)) {
		z_reschedule(&stack->lock, key);
		goto end;
	} නැතහොත් {
		*(stack->next) = data;
		stack->next++;
		goto out;
	}

out:
	k_spin_unlock(&stack->lock, key);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, push, stack, ret);

	ආපසු ret;
}

#ifdef CONFIG_USERSPACE
static anta_hstha පූර්ණ_සංඛ්‍යා z_vrfy_k_stack_push(sa_nracanaa k_stack *stack, stack_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(stack, K_OBJ_STACK));

	ආපසු z_impl_k_stack_push(stack, data);
}
#include <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

පූර්ණ_සංඛ්‍යා z_impl_k_stack_pop(sa_nracanaa k_stack *stack, stack_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_key_t key;
	පූර්ණ_සංඛ්‍යා result;

	key = k_spin_lock(&stack->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, pop, stack, timeout);

	නම් (likely(stack->next > stack->base)) {
		stack->next--;
		*data = *(stack->next);
		k_spin_unlock(&stack->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, 0);

		ආපසු 0;
	}

	නම් (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&stack->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, -EBUSY);

		ආපසු -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_stack, pop, stack, timeout);

	result = z_pend_curr(&stack->lock, key, &stack->wait_q, timeout);
	නම් (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, result);

		ආපසු result;
	}

	*data = (stack_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, 0);

	ආපසු 0;
}

#ifdef CONFIG_USERSPACE
static anta_hstha පූර්ණ_සංඛ්‍යා z_vrfy_k_stack_pop(sa_nracanaa k_stack *stack,
				     stack_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(stack, K_OBJ_STACK));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, parimaa_na(stack_data_t)));
	ආපසු z_impl_k_stack_pop(stack, data, timeout);
}
#include <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
