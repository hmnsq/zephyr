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

#ifdef CONFIG_OBJ_CORE_ढेर
K_OBJ_TYPE_DEFINE(obj_type_ढेर, k_ढेर, K_OBJ_TYPE_ढेर_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_STACK */

शून्य k_ढेर_init(संरचना k_ढेर *ढेर, ढेर_data_t *buffer,
		  uint32_t अंक_entries)
{
	z_waitq_init(&ढेर->wait_q);
	ढेर->lock = (संरचना k_spinlock) {};
	ढेर->next = buffer;
	ढेर->base = buffer;
	ढेर->शीर्ष = ढेर->base + अंक_entries;
	ढेर->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_ढेर, ढेर);
	k_object_init(ढेर);

#ifdef CONFIG_OBJ_CORE_ढेर
	k_obj_core_init_and_link(K_OBJ_CORE(ढेर), &obj_type_ढेर);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ढेर_alloc_init(संरचना k_ढेर *ढेर, uint32_t अंक_entries)
{
	शून्य *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, alloc_init, ढेर);

	/* Reject allocation sizes that cannot be represented. */
	यदि (size_mul_overflow(अंक_entries, परिमाण(ढेर_data_t), &total_size)) {
		ret = -ENOMEM;
	} अन्यथा {
		buffer = z_thread_malloc(total_size);
		यदि (buffer != रिक्त) {
			k_ढेर_init(ढेर, buffer, अंक_entries);
			ढेर->flags = K_ढेर_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, alloc_init, ढेर, ret);

	प्रत्यागम ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_ढेर_alloc_init(संरचना k_ढेर *ढेर,
					      uint32_t अंक_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ढेर, K_OBJ_ढेर));
	K_OOPS(K_SYSCALL_VERIFY(अंक_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(अंक_entries, परिमाण(ढेर_data_t),
					&total_size)));
	प्रत्यागम z_impl_k_ढेर_alloc_init(ढेर, अंक_entries);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_ढेर_cleanup(संरचना k_ढेर *ढेर, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, cleanup, ढेर);

	पूर्णाङ्क ret = 0;
	bool freed = false;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&ढेर->lock);

	यदि (locked && (z_waitq_head_locked(&ढेर->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
	}

	यदि (!locked && (z_waitq_head(&ढेर->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
	}

	यदि ((ढेर->flags & K_ढेर_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ढेर->base);
		ढेर->base = रिक्त;
		ढेर->flags &= ~K_ढेर_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&ढेर->lock, कुंजी);

#ifdef CONFIG_OBJ_CORE_ढेर
	यदि (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ढेर));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, cleanup, ढेर, ret);

	प्रत्यागम ret;
}

पूर्णाङ्क k_ढेर_cleanup(संरचना k_ढेर *ढेर)
{
	प्रत्यागम z_ढेर_cleanup(ढेर, false);
}

पूर्णाङ्क z_impl_k_ढेर_धकेलो(संरचना k_ढेर *ढेर, ढेर_data_t data)
{
	पूर्णाङ्क ret = 0;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&ढेर->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, धकेलो, ढेर);

	यदि (ढेर->next == ढेर->शीर्ष) {
		ret = -ENOMEM;
		गच्छ out;
	}

	यदि (z_sched_wake(&ढेर->wait_q, 0, (शून्य *)data)) {
		z_reschedule(&ढेर->lock, कुंजी);
		गच्छ end;
	} अन्यथा {
		*(ढेर->next) = data;
		ढेर->next++;
		गच्छ out;
	}

out:
	k_spin_unlock(&ढेर->lock, कुंजी);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, धकेलो, ढेर, ret);

	प्रत्यागम ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_ढेर_धकेलो(संरचना k_ढेर *ढेर, ढेर_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(ढेर, K_OBJ_ढेर));

	प्रत्यागम z_impl_k_ढेर_धकेलो(ढेर, data);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_ढेर_निकालो(संरचना k_ढेर *ढेर, ढेर_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णाङ्क result;

	कुंजी = k_spin_lock(&ढेर->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, निकालो, ढेर, timeout);

	यदि (likely(ढेर->next > ढेर->base)) {
		ढेर->next--;
		*data = *(ढेर->next);
		k_spin_unlock(&ढेर->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, 0);

		प्रत्यागम 0;
	}

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ढेर->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, -EBUSY);

		प्रत्यागम -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_ढेर, निकालो, ढेर, timeout);

	result = z_pend_curr(&ढेर->lock, कुंजी, &ढेर->wait_q, timeout);
	यदि (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, result);

		प्रत्यागम result;
	}

	*data = (ढेर_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, 0);

	प्रत्यागम 0;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_ढेर_निकालो(संरचना k_ढेर *ढेर,
				     ढेर_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ढेर, K_OBJ_ढेर));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, परिमाण(ढेर_data_t)));
	प्रत्यागम z_impl_k_ढेर_निकालो(ढेर, data, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
