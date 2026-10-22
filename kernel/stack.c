/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/kernel.h>

#समावेश <zephyr/toolchain.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>
#समावेश <wait_q.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <kernel_internal.h>
#समावेश <stdbool.h>

#अगर_घोषित CONFIG_OBJ_CORE_ढेर
K_OBJ_TYPE_DEFINE(obj_type_ढेर, k_ढेर, K_OBJ_TYPE_ढेर_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STACK */

व्योम k_ढेर_init(काष्ठा k_ढेर *ढेर, ढेर_तथ्य_t *बफर,
		  uint32_t अंक_entries)
{
	z_waitq_init(&ढेर->wait_q);
	ढेर->lock = (काष्ठा k_spinlock) {};
	ढेर->अग्र = बफर;
	ढेर->base = बफर;
	ढेर->शीर्ष_2 = ढेर->base + अंक_entries;
	ढेर->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_ढेर, ढेर);
	k_object_init(ढेर);

#अगर_घोषित CONFIG_OBJ_CORE_ढेर
	k_obj_core_init_and_link(K_OBJ_CORE(ढेर), &obj_type_ढेर);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_ढेर_alloc_init(काष्ठा k_ढेर *ढेर, uint32_t अंक_entries)
{
	व्योम *बफर;
	माप_प्रकार योग_आकार;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, alloc_init, ढेर);

	/* Reject allocation sizes that cannot be represented. */
	अगर (आकार_mul_overflow(अंक_entries, माप(ढेर_तथ्य_t), &योग_आकार)) {
		ret = -ENOMEM;
	} अन्यथा {
		बफर = z_thread_malloc(योग_आकार);
		अगर (बफर != शून्य) {
			k_ढेर_init(ढेर, बफर, अंक_entries);
			ढेर->flags = K_ढेर_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, alloc_init, ढेर, ret);

	वापस ret;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत int32_t z_vrfy_k_ढेर_alloc_init(काष्ठा k_ढेर *ढेर,
					      uint32_t अंक_entries)
{
	माप_प्रकार योग_आकार;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ढेर, K_OBJ_ढेर));
	K_OOPS(K_SYSCALL_VERIFY(अंक_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!आकार_mul_overflow(अंक_entries, माप(ढेर_तथ्य_t),
					&योग_आकार)));
	वापस z_impl_k_ढेर_alloc_init(ढेर, अंक_entries);
}
#समावेश <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_ढेर_cleanup(काष्ठा k_ढेर *ढेर, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, cleanup, ढेर);

	पूर्णांक ret = 0;
	bool freed = false;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&ढेर->lock);

	अगर (locked && (z_waitq_शीर्ष_locked(&ढेर->wait_q) != शून्य)) {
		ret = -EAGAIN;
		जाओ out;
	}

	अगर (!locked && (z_waitq_शीर्ष(&ढेर->wait_q) != शून्य)) {
		ret = -EAGAIN;
		जाओ out;
	}

	अगर ((ढेर->flags & K_ढेर_FLAG_ALLOC) != (uint8_t)0) {
		k_free(ढेर->base);
		ढेर->base = शून्य;
		ढेर->flags &= ~K_ढेर_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&ढेर->lock, कुंजी);

#अगर_घोषित CONFIG_OBJ_CORE_ढेर
	अगर (freed) {
		k_obj_core_unlink(K_OBJ_CORE(ढेर));
	}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, cleanup, ढेर, ret);

	वापस ret;
}

पूर्णांक k_ढेर_cleanup(काष्ठा k_ढेर *ढेर)
{
	वापस z_ढेर_cleanup(ढेर, false);
}

पूर्णांक z_impl_k_ढेर_धकेलो(काष्ठा k_ढेर *ढेर, ढेर_तथ्य_t तथ्य)
{
	पूर्णांक ret = 0;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&ढेर->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, धकेलो, ढेर);

	अगर (ढेर->अग्र == ढेर->शीर्ष_2) {
		ret = -ENOMEM;
		जाओ out;
	}

	अगर (z_sched_wake(&ढेर->wait_q, 0, (व्योम *)तथ्य)) {
		z_reschedule(&ढेर->lock, कुंजी);
		जाओ end;
	} अन्यथा {
		*(ढेर->अग्र) = तथ्य;
		ढेर->अग्र++;
		जाओ out;
	}

out:
	k_spin_unlock(&ढेर->lock, कुंजी);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, धकेलो, ढेर, ret);

	वापस ret;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_ढेर_धकेलो(काष्ठा k_ढेर *ढेर, ढेर_तथ्य_t तथ्य)
{
	K_OOPS(K_SYSCALL_OBJ(ढेर, K_OBJ_ढेर));

	वापस z_impl_k_ढेर_धकेलो(ढेर, तथ्य);
}
#समावेश <zephyr/syscalls/k_stack_push_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_ढेर_निकालो(काष्ठा k_ढेर *ढेर, ढेर_तथ्य_t *तथ्य,
		       k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक परिणाम;

	कुंजी = k_spin_lock(&ढेर->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_ढेर, निकालो, ढेर, timeout);

	अगर (likely(ढेर->अग्र > ढेर->base)) {
		ढेर->अग्र--;
		*तथ्य = *(ढेर->अग्र);
		k_spin_unlock(&ढेर->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, 0);

		वापस 0;
	}

	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ढेर->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, -EBUSY);

		वापस -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_ढेर, निकालो, ढेर, timeout);

	परिणाम = z_pend_curr(&ढेर->lock, कुंजी, &ढेर->wait_q, timeout);
	अगर (परिणाम != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, परिणाम);

		वापस परिणाम;
	}

	*तथ्य = (ढेर_तथ्य_t)_current->base.swap_तथ्य;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_ढेर, निकालो, ढेर, timeout, 0);

	वापस 0;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_ढेर_निकालो(काष्ठा k_ढेर *ढेर,
				     ढेर_तथ्य_t *तथ्य, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ढेर, K_OBJ_ढेर));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(तथ्य, माप(ढेर_तथ्य_t)));
	वापस z_impl_k_ढेर_निकालो(ढेर, तथ्य, timeout);
}
#समावेश <zephyr/syscalls/k_stack_pop_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
