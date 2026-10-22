/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <stdbool.h>

#ifdef CONFIG_OBJ_CORE_थाक
K_OBJ_TYPE_DEFINE(obj_type_थाक, k_थाक, K_OBJ_TYPE_थाक_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_STACK */

शून्य k_थाक_init(संरचना k_थाक *थाक, थाक_data_t *buffer,
		  uint32_t अङ्क_entries)
{
	z_waitq_init(&थाक->wait_q);
	थाक->lock = (संरचना k_spinlock) {};
	थाक->next = buffer;
	थाक->base = buffer;
	थाक->शीर्ष = थाक->base + अङ्क_entries;
	थाक->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_थाक, थाक);
	k_object_init(थाक);

#ifdef CONFIG_OBJ_CORE_थाक
	k_obj_core_init_and_link(K_OBJ_CORE(थाक), &obj_type_थाक);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_थाक_alloc_init(संरचना k_थाक *थाक, uint32_t अङ्क_entries)
{
	शून्य *buffer;
	माप_प्रकार total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_थाक, alloc_init, थाक);

	/* Reject allocation sizes that cannot be represented. */
	यदि (size_mul_overflow(अङ्क_entries, परिमाण(थाक_data_t), &total_size)) {
		ret = -ENOMEM;
	} अन्यथा {
		buffer = z_thread_malloc(total_size);
		यदि (buffer != रिक्त) {
			k_थाक_init(थाक, buffer, अङ्क_entries);
			थाक->flags = K_थाक_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_थाक, alloc_init, थाक, ret);

	फर्काऊ ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_थाक_alloc_init(संरचना k_थाक *थाक,
					      uint32_t अङ्क_entries)
{
	माप_प्रकार total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(थाक, K_OBJ_थाक));
	K_OOPS(K_SYSCALL_VERIFY(अङ्क_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(अङ्क_entries, परिमाण(थाक_data_t),
					&total_size)));
	फर्काऊ z_impl_k_थाक_alloc_init(थाक, अङ्क_entries);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_थाक_cleanup(संरचना k_थाक *थाक, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_थाक, cleanup, थाक);

	पूर्णाङ्क ret = 0;
	bool freed = false;
	k_spinlock_साँचो_t साँचो = k_spin_lock(&थाक->lock);

	यदि (locked && (z_waitq_head_locked(&थाक->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
	}

	यदि (!locked && (z_waitq_head(&थाक->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
	}

	यदि ((थाक->flags & K_थाक_FLAG_ALLOC) != (uint8_t)0) {
		k_free(थाक->base);
		थाक->base = रिक्त;
		थाक->flags &= ~K_थाक_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&थाक->lock, साँचो);

#ifdef CONFIG_OBJ_CORE_थाक
	यदि (freed) {
		k_obj_core_unlink(K_OBJ_CORE(थाक));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_थाक, cleanup, थाक, ret);

	फर्काऊ ret;
}

पूर्णाङ्क k_थाक_cleanup(संरचना k_थाक *थाक)
{
	फर्काऊ z_थाक_cleanup(थाक, false);
}

पूर्णाङ्क z_impl_k_थाक_थप(संरचना k_थाक *थाक, थाक_data_t data)
{
	पूर्णाङ्क ret = 0;
	k_spinlock_साँचो_t साँचो = k_spin_lock(&थाक->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_थाक, थप, थाक);

	यदि (थाक->next == थाक->शीर्ष) {
		ret = -ENOMEM;
		गच्छ out;
	}

	यदि (z_sched_wake(&थाक->wait_q, 0, (शून्य *)data)) {
		z_reschedule(&थाक->lock, साँचो);
		गच्छ end;
	} अन्यथा {
		*(थाक->next) = data;
		थाक->next++;
		गच्छ out;
	}

out:
	k_spin_unlock(&थाक->lock, साँचो);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_थाक, थप, थाक, ret);

	फर्काऊ ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_थाक_थप(संरचना k_थाक *थाक, थाक_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(थाक, K_OBJ_थाक));

	फर्काऊ z_impl_k_थाक_थप(थाक, data);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_थाक_झिक(संरचना k_थाक *थाक, थाक_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_साँचो_t साँचो;
	पूर्णाङ्क result;

	साँचो = k_spin_lock(&थाक->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_थाक, झिक, थाक, timeout);

	यदि (likely(थाक->next > थाक->base)) {
		थाक->next--;
		*data = *(थाक->next);
		k_spin_unlock(&थाक->lock, साँचो);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_थाक, झिक, थाक, timeout, 0);

		फर्काऊ 0;
	}

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&थाक->lock, साँचो);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_थाक, झिक, थाक, timeout, -EBUSY);

		फर्काऊ -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_थाक, झिक, थाक, timeout);

	result = z_pend_curr(&थाक->lock, साँचो, &थाक->wait_q, timeout);
	यदि (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_थाक, झिक, थाक, timeout, result);

		फर्काऊ result;
	}

	*data = (थाक_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_थाक, झिक, थाक, timeout, 0);

	फर्काऊ 0;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_थाक_झिक(संरचना k_थाक *थाक,
				     थाक_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(थाक, K_OBJ_थाक));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, परिमाण(थाक_data_t)));
	फर्काऊ z_impl_k_थाक_झिक(थाक, data, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
