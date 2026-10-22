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

#ifdef CONFIG_OBJ_CORE_चळत
K_OBJ_TYPE_DEFINE(obj_type_चळत, k_चळत, K_OBJ_TYPE_चळत_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_STACK */

शून्य k_चळत_init(संरचना k_चळत *चळत, चळत_data_t *buffer,
		  uint32_t अंक_entries)
{
	z_waitq_init(&चळत->wait_q);
	चळत->lock = (संरचना k_spinlock) {};
	चळत->next = buffer;
	चळत->base = buffer;
	चळत->शीर्ष = चळत->base + अंक_entries;
	चळत->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_चळत, चळत);
	k_object_init(चळत);

#ifdef CONFIG_OBJ_CORE_चळत
	k_obj_core_init_and_link(K_OBJ_CORE(चळत), &obj_type_चळत);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_चळत_alloc_init(संरचना k_चळत *चळत, uint32_t अंक_entries)
{
	शून्य *buffer;
	माप_प्रकार total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_चळत, alloc_init, चळत);

	/* Reject allocation sizes that cannot be represented. */
	जर (size_mul_overflow(अंक_entries, आकार(चळत_data_t), &total_size)) {
		ret = -ENOMEM;
	} अन्यथा {
		buffer = z_thread_malloc(total_size);
		जर (buffer != रिक्त) {
			k_चळत_init(चळत, buffer, अंक_entries);
			चळत->flags = K_चळत_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_चळत, alloc_init, चळत, ret);

	परत ret;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत int32_t z_vrfy_k_चळत_alloc_init(संरचना k_चळत *चळत,
					      uint32_t अंक_entries)
{
	माप_प्रकार total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(चळत, K_OBJ_चळत));
	K_OOPS(K_SYSCALL_VERIFY(अंक_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(अंक_entries, आकार(चळत_data_t),
					&total_size)));
	परत z_impl_k_चळत_alloc_init(चळत, अंक_entries);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_चळत_cleanup(संरचना k_चळत *चळत, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_चळत, cleanup, चळत);

	पूर्णांक ret = 0;
	bool freed = false;
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&चळत->lock);

	जर (locked && (z_waitq_head_locked(&चळत->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		जा out;
	}

	जर (!locked && (z_waitq_head(&चळत->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		जा out;
	}

	जर ((चळत->flags & K_चळत_FLAG_ALLOC) != (uint8_t)0) {
		k_free(चळत->base);
		चळत->base = रिक्त;
		चळत->flags &= ~K_चळत_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&चळत->lock, किल्ली);

#ifdef CONFIG_OBJ_CORE_चळत
	जर (freed) {
		k_obj_core_unlink(K_OBJ_CORE(चळत));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_चळत, cleanup, चळत, ret);

	परत ret;
}

पूर्णांक k_चळत_cleanup(संरचना k_चळत *चळत)
{
	परत z_चळत_cleanup(चळत, false);
}

पूर्णांक z_impl_k_चळत_ढकला(संरचना k_चळत *चळत, चळत_data_t data)
{
	पूर्णांक ret = 0;
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&चळत->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_चळत, ढकला, चळत);

	जर (चळत->next == चळत->शीर्ष) {
		ret = -ENOMEM;
		जा out;
	}

	जर (z_sched_wake(&चळत->wait_q, 0, (शून्य *)data)) {
		z_reschedule(&चळत->lock, किल्ली);
		जा end;
	} अन्यथा {
		*(चळत->next) = data;
		चळत->next++;
		जा out;
	}

out:
	k_spin_unlock(&चळत->lock, किल्ली);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_चळत, ढकला, चळत, ret);

	परत ret;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_चळत_ढकला(संरचना k_चळत *चळत, चळत_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(चळत, K_OBJ_चळत));

	परत z_impl_k_चळत_ढकला(चळत, data);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_चळत_काढा(संरचना k_चळत *चळत, चळत_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_किल्ली_t किल्ली;
	पूर्णांक result;

	किल्ली = k_spin_lock(&चळत->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_चळत, काढा, चळत, timeout);

	जर (likely(चळत->next > चळत->base)) {
		चळत->next--;
		*data = *(चळत->next);
		k_spin_unlock(&चळत->lock, किल्ली);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_चळत, काढा, चळत, timeout, 0);

		परत 0;
	}

	जर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&चळत->lock, किल्ली);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_चळत, काढा, चळत, timeout, -EBUSY);

		परत -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_चळत, काढा, चळत, timeout);

	result = z_pend_curr(&चळत->lock, किल्ली, &चळत->wait_q, timeout);
	जर (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_चळत, काढा, चळत, timeout, result);

		परत result;
	}

	*data = (चळत_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_चळत, काढा, चळत, timeout, 0);

	परत 0;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_चळत_काढा(संरचना k_चळत *चळत,
				     चळत_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(चळत, K_OBJ_चळत));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, आकार(चळत_data_t)));
	परत z_impl_k_चळत_काढा(चळत, data, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
