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
	ถ้า (size_mul_overflow(num_entries, parimaa_na(stack_data_t), &total_size)) {
		ret = -ENOMEM;
	} มิฉะนั้น {
		buffer = z_thread_malloc(total_size);
		ถ้า (buffer != NULL) {
			k_stack_init(stack, buffer, num_entries);
			stack->flags = K_STACK_FLAG_ALLOC;
			ret = 0;
		} มิฉะนั้น {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, alloc_init, stack, ret);

	คืน ret;
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
	คืน z_impl_k_stack_alloc_init(stack, num_entries);
}
#include <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

จำนวนเต็ม z_stack_cleanup(sa_nracanaa k_stack *stack, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, cleanup, stack);

	จำนวนเต็ม ret = 0;
	bool freed = false;
	k_spinlock_key_t key = k_spin_lock(&stack->lock);

	ถ้า (locked && (z_waitq_head_locked(&stack->wait_q) != NULL)) {
		ret = -EAGAIN;
		goto out;
	}

	ถ้า (!locked && (z_waitq_head(&stack->wait_q) != NULL)) {
		ret = -EAGAIN;
		goto out;
	}

	ถ้า ((stack->flags & K_STACK_FLAG_ALLOC) != (uint8_t)0) {
		k_free(stack->base);
		stack->base = NULL;
		stack->flags &= ~K_STACK_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&stack->lock, key);

#ifdef CONFIG_OBJ_CORE_STACK
	ถ้า (freed) {
		k_obj_core_unlink(K_OBJ_CORE(stack));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, cleanup, stack, ret);

	คืน ret;
}

จำนวนเต็ม k_stack_cleanup(sa_nracanaa k_stack *stack)
{
	คืน z_stack_cleanup(stack, false);
}

จำนวนเต็ม z_impl_k_stack_push(sa_nracanaa k_stack *stack, stack_data_t data)
{
	จำนวนเต็ม ret = 0;
	k_spinlock_key_t key = k_spin_lock(&stack->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, push, stack);

	ถ้า (stack->next == stack->top) {
		ret = -ENOMEM;
		goto out;
	}

	ถ้า (z_sched_wake(&stack->wait_q, 0, (void *)data)) {
		z_reschedule(&stack->lock, key);
		goto end;
	} มิฉะนั้น {
		*(stack->next) = data;
		stack->next++;
		goto out;
	}

out:
	k_spin_unlock(&stack->lock, key);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, push, stack, ret);

	คืน ret;
}

#ifdef CONFIG_USERSPACE
static anta_hstha จำนวนเต็ม z_vrfy_k_stack_push(sa_nracanaa k_stack *stack, stack_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(stack, K_OBJ_STACK));

	คืน z_impl_k_stack_push(stack, data);
}
#include <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

จำนวนเต็ม z_impl_k_stack_pop(sa_nracanaa k_stack *stack, stack_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_key_t key;
	จำนวนเต็ม result;

	key = k_spin_lock(&stack->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_stack, pop, stack, timeout);

	ถ้า (likely(stack->next > stack->base)) {
		stack->next--;
		*data = *(stack->next);
		k_spin_unlock(&stack->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, 0);

		คืน 0;
	}

	ถ้า (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&stack->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, -EBUSY);

		คืน -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_stack, pop, stack, timeout);

	result = z_pend_curr(&stack->lock, key, &stack->wait_q, timeout);
	ถ้า (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, result);

		คืน result;
	}

	*data = (stack_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_stack, pop, stack, timeout, 0);

	คืน 0;
}

#ifdef CONFIG_USERSPACE
static anta_hstha จำนวนเต็ม z_vrfy_k_stack_pop(sa_nracanaa k_stack *stack,
				     stack_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(stack, K_OBJ_STACK));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, parimaa_na(stack_data_t)));
	คืน z_impl_k_stack_pop(stack, data, timeout);
}
#include <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
