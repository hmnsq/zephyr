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

#ifdef CONFIG_OBJ_CORE_राशिः
K_OBJ_TYPE_DEFINE(obj_type_राशिः, k_राशिः, K_OBJ_TYPE_राशिः_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_STACK */

शून्य k_राशिः_init(संरचना k_राशिः *राशिः, राशिः_data_t *buffer,
		  uint32_t अङ्कः_entries)
{
	z_waitq_init(&राशिः->wait_q);
	राशिः->lock = (संरचना k_spinlock) {};
	राशिः->next = buffer;
	राशिः->base = buffer;
	राशिः->शिखरम् = राशिः->base + अङ्कः_entries;
	राशिः->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_राशिः, राशिः);
	k_object_init(राशिः);

#ifdef CONFIG_OBJ_CORE_राशिः
	k_obj_core_init_and_link(K_OBJ_CORE(राशिः), &obj_type_राशिः);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_राशिः_alloc_init(संरचना k_राशिः *राशिः, uint32_t अङ्कः_entries)
{
	शून्य *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_राशिः, alloc_init, राशिः);

	/* Reject allocation sizes that cannot be represented. */
	सचे (size_mul_overflow(अङ्कः_entries, परिमाण(राशिः_data_t), &total_size)) {
		ret = -ENOMEM;
	} अञ्ञथा {
		buffer = z_thread_malloc(total_size);
		सचे (buffer != रिक्त) {
			k_राशिः_init(राशिः, buffer, अङ्कः_entries);
			राशिः->flags = K_राशिः_FLAG_ALLOC;
			ret = 0;
		} अञ्ञथा {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_राशिः, alloc_init, राशिः, ret);

	पटिनिवत्त ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_राशिः_alloc_init(संरचना k_राशिः *राशिः,
					      uint32_t अङ्कः_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(राशिः, K_OBJ_राशिः));
	K_OOPS(K_SYSCALL_VERIFY(अङ्कः_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(अङ्कः_entries, परिमाण(राशिः_data_t),
					&total_size)));
	पटिनिवत्त z_impl_k_राशिः_alloc_init(राशिः, अङ्कः_entries);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_राशिः_cleanup(संरचना k_राशिः *राशिः, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_राशिः, cleanup, राशिः);

	पूर्णाङ्क ret = 0;
	bool freed = false;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&राशिः->lock);

	सचे (locked && (z_waitq_head_locked(&राशिः->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
	}

	सचे (!locked && (z_waitq_head(&राशिः->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
	}

	सचे ((राशिः->flags & K_राशिः_FLAG_ALLOC) != (uint8_t)0) {
		k_free(राशिः->base);
		राशिः->base = रिक्त;
		राशिः->flags &= ~K_राशिः_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&राशिः->lock, कुञ्चिका);

#ifdef CONFIG_OBJ_CORE_राशिः
	सचे (freed) {
		k_obj_core_unlink(K_OBJ_CORE(राशिः));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_राशिः, cleanup, राशिः, ret);

	पटिनिवत्त ret;
}

पूर्णाङ्क k_राशिः_cleanup(संरचना k_राशिः *राशिः)
{
	पटिनिवत्त z_राशिः_cleanup(राशिः, false);
}

पूर्णाङ्क z_impl_k_राशिः_प्रक्षिप(संरचना k_राशिः *राशिः, राशिः_data_t data)
{
	पूर्णाङ्क ret = 0;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&राशिः->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_राशिः, प्रक्षिप, राशिः);

	सचे (राशिः->next == राशिः->शिखरम्) {
		ret = -ENOMEM;
		गच्छ out;
	}

	सचे (z_sched_wake(&राशिः->wait_q, 0, (शून्य *)data)) {
		z_reschedule(&राशिः->lock, कुञ्चिका);
		गच्छ end;
	} अञ्ञथा {
		*(राशिः->next) = data;
		राशिः->next++;
		गच्छ out;
	}

out:
	k_spin_unlock(&राशिः->lock, कुञ्चिका);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_राशिः, प्रक्षिप, राशिः, ret);

	पटिनिवत्त ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_राशिः_प्रक्षिप(संरचना k_राशिः *राशिः, राशिः_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(राशिः, K_OBJ_राशिः));

	पटिनिवत्त z_impl_k_राशिः_प्रक्षिप(राशिः, data);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_राशिः_निष्कासय(संरचना k_राशिः *राशिः, राशिः_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क result;

	कुञ्चिका = k_spin_lock(&राशिः->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_राशिः, निष्कासय, राशिः, timeout);

	सचे (likely(राशिः->next > राशिः->base)) {
		राशिः->next--;
		*data = *(राशिः->next);
		k_spin_unlock(&राशिः->lock, कुञ्चिका);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, 0);

		पटिनिवत्त 0;
	}

	सचे (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&राशिः->lock, कुञ्चिका);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, -EBUSY);

		पटिनिवत्त -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_राशिः, निष्कासय, राशिः, timeout);

	result = z_pend_curr(&राशिः->lock, कुञ्चिका, &राशिः->wait_q, timeout);
	सचे (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, result);

		पटिनिवत्त result;
	}

	*data = (राशिः_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, 0);

	पटिनिवत्त 0;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_राशिः_निष्कासय(संरचना k_राशिः *राशिः,
				     राशिः_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(राशिः, K_OBJ_राशिः));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, परिमाण(राशिः_data_t)));
	पटिनिवत्त z_impl_k_राशिः_निष्कासय(राशिः, data, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
