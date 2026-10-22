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

#यदि_परिभाषितम् CONFIG_OBJ_CORE_राशिः
K_OBJ_TYPE_DEFINE(obj_type_राशिः, k_राशिः, K_OBJ_TYPE_राशिः_ID, रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_STACK */

शून्य k_राशिः_init(संरचना k_राशिः *राशिः, राशिः_दत्त_t *बफर,
		  uint32_t अङ्कः_entries)
{
	z_waitq_init(&राशिः->wait_q);
	राशिः->तालकम् = (संरचना k_spinlock) {};
	राशिः->अग्र = बफर;
	राशिः->आधारः = बफर;
	राशिः->शिखरम् = राशिः->आधारः + अङ्कः_entries;
	राशिः->flags = 0;

	SYS_पत्तनम्_TRACING_OBJ_INIT(k_राशिः, राशिः);
	k_object_init(राशिः);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_राशिः
	k_obj_core_init_and_link(K_OBJ_CORE(राशिः), &obj_type_राशिः);
#यदि_अन्तः /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_राशिः_alloc_init(संरचना k_राशिः *राशिः, uint32_t अङ्कः_entries)
{
	शून्य *बफर;
	size_t योग_आकार;
	int32_t ret;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_राशिः, alloc_init, राशिः);

	/* Reject allocation sizes that cannot be represented. */
	यदि (आकार_mul_अतिप्रवाहः(अङ्कः_entries, परिमाण(राशिः_दत्त_t), &योग_आकार)) {
		ret = -ENOMEM;
	} अन्यथा {
		बफर = z_thread_malloc(योग_आकार);
		यदि (बफर != रिक्त) {
			k_राशिः_init(राशिः, बफर, अङ्कः_entries);
			राशिः->flags = K_राशिः_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_राशिः, alloc_init, राशिः, ret);

	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_राशिः_alloc_init(संरचना k_राशिः *राशिः,
					      uint32_t अङ्कः_entries)
{
	size_t योग_आकार;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(राशिः, K_OBJ_राशिः));
	K_OOPS(K_SYSCALL_VERIFY(अङ्कः_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!आकार_mul_अतिप्रवाहः(अङ्कः_entries, परिमाण(राशिः_दत्त_t),
					&योग_आकार)));
	प्रत्यागम z_impl_k_राशिः_alloc_init(राशिः, अङ्कः_entries);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_राशिः_cleanup(संरचना k_राशिः *राशिः, __maybe_unused bool locked)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_राशिः, cleanup, राशिः);

	पूर्णाङ्क ret = 0;
	bool freed = false;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&राशिः->तालकम्);

	यदि (locked && (z_waitq_शीर्ष_locked(&राशिः->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ बहिर्गमनम्;
	}

	यदि (!locked && (z_waitq_शीर्ष(&राशिः->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ बहिर्गमनम्;
	}

	यदि ((राशिः->flags & K_राशिः_FLAG_ALLOC) != (uint8_t)0) {
		k_free(राशिः->आधारः);
		राशिः->आधारः = रिक्त;
		राशिः->flags &= ~K_राशिः_FLAG_ALLOC;
		freed = true;
	}

बहिर्गमनम्:
	k_spin_unlock(&राशिः->तालकम्, कुञ्चिका);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_राशिः
	यदि (freed) {
		k_obj_core_unlink(K_OBJ_CORE(राशिः));
	}
#यदि_अन्तः /* CONFIG_OBJ_CORE_STACK */
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_राशिः, cleanup, राशिः, ret);

	प्रत्यागम ret;
}

पूर्णाङ्क k_राशिः_cleanup(संरचना k_राशिः *राशिः)
{
	प्रत्यागम z_राशिः_cleanup(राशिः, false);
}

पूर्णाङ्क z_impl_k_राशिः_प्रक्षिप(संरचना k_राशिः *राशिः, राशिः_दत्त_t दत्त)
{
	पूर्णाङ्क ret = 0;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&राशिः->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_राशिः, प्रक्षिप, राशिः);

	यदि (राशिः->अग्र == राशिः->शिखरम्) {
		ret = -ENOMEM;
		गच्छ बहिर्गमनम्;
	}

	यदि (z_sched_wake(&राशिः->wait_q, 0, (शून्य *)दत्त)) {
		z_reschedule(&राशिः->तालकम्, कुञ्चिका);
		गच्छ अन्तः;
	} अन्यथा {
		*(राशिः->अग्र) = दत्त;
		राशिः->अग्र++;
		गच्छ बहिर्गमनम्;
	}

बहिर्गमनम्:
	k_spin_unlock(&राशिः->तालकम्, कुञ्चिका);

अन्तः:
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_राशिः, प्रक्षिप, राशिः, ret);

	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_राशिः_प्रक्षिप(संरचना k_राशिः *राशिः, राशिः_दत्त_t दत्त)
{
	K_OOPS(K_SYSCALL_OBJ(राशिः, K_OBJ_राशिः));

	प्रत्यागम z_impl_k_राशिः_प्रक्षिप(राशिः, दत्त);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_push_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_राशिः_निष्कासय(संरचना k_राशिः *राशिः, राशिः_दत्त_t *दत्त,
		       k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क परिणाम;

	कुञ्चिका = k_spin_तालकम्(&राशिः->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_राशिः, निष्कासय, राशिः, timeout);

	यदि (likely(राशिः->अग्र > राशिः->आधारः)) {
		राशिः->अग्र--;
		*दत्त = *(राशिः->अग्र);
		k_spin_unlock(&राशिः->तालकम्, कुञ्चिका);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, 0);

		प्रत्यागम 0;
	}

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&राशिः->तालकम्, कुञ्चिका);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, -EBUSY);

		प्रत्यागम -EBUSY;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_राशिः, निष्कासय, राशिः, timeout);

	परिणाम = z_pend_curr(&राशिः->तालकम्, कुञ्चिका, &राशिः->wait_q, timeout);
	यदि (परिणाम != 0) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, परिणाम);

		प्रत्यागम परिणाम;
	}

	*दत्त = (राशिः_दत्त_t)_current->आधारः.swap_दत्त;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_राशिः, निष्कासय, राशिः, timeout, 0);

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_राशिः_निष्कासय(संरचना k_राशिः *राशिः,
				     राशिः_दत्त_t *दत्त, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(राशिः, K_OBJ_राशिः));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(दत्त, परिमाण(राशिः_दत्त_t)));
	प्रत्यागम z_impl_k_राशिः_निष्कासय(राशिः, दत्त, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_stack_pop_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
