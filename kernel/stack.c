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

#اگر_تعریف_شده CONFIG_OBJ_CORE_پشته
K_OBJ_TYPE_DEFINE(obj_type_پشته, k_پشته, K_OBJ_TYPE_پشته_ID, عدم);
#پایان_اگر /* CONFIG_OBJ_CORE_STACK */

فراغ k_پشته_init(بنية k_پشته *پشته, پشته_داده_t *میانگیر,
		  uint32_t رقم_entries)
{
	z_waitq_init(&پشته->wait_q);
	پشته->قفل = (بنية k_spinlock) {};
	پشته->بعدی = میانگیر;
	پشته->پایه = میانگیر;
	پشته->بالاترین = پشته->پایه + رقم_entries;
	پشته->flags = 0;

	SYS_پورت_TRACING_OBJ_INIT(k_پشته, پشته);
	k_object_init(پشته);

#اگر_تعریف_شده CONFIG_OBJ_CORE_پشته
	k_obj_core_init_and_link(K_OBJ_CORE(پشته), &obj_type_پشته);
#پایان_اگر /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_پشته_alloc_init(بنية k_پشته *پشته, uint32_t رقم_entries)
{
	فراغ *میانگیر;
	size_t مجموع_اندازه;
	int32_t ret;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_پشته, alloc_init, پشته);

	/* Reject allocation sizes that cannot be represented. */
	إذا (اندازه_mul_سرریز(رقم_entries, حجم(پشته_داده_t), &مجموع_اندازه)) {
		ret = -ENOMEM;
	} وإلا {
		میانگیر = z_thread_malloc(مجموع_اندازه);
		إذا (میانگیر != عدم) {
			k_پشته_init(پشته, میانگیر, رقم_entries);
			پشته->flags = K_پشته_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_پشته, alloc_init, پشته, ret);

	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_پشته_alloc_init(بنية k_پشته *پشته,
					      uint32_t رقم_entries)
{
	size_t مجموع_اندازه;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(پشته, K_OBJ_پشته));
	K_OOPS(K_SYSCALL_VERIFY(رقم_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!اندازه_mul_سرریز(رقم_entries, حجم(پشته_داده_t),
					&مجموع_اندازه)));
	أرجع z_impl_k_پشته_alloc_init(پشته, رقم_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_پشته_cleanup(بنية k_پشته *پشته, __maybe_unused bool locked)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_پشته, cleanup, پشته);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_قفل(&پشته->قفل);

	إذا (locked && (z_waitq_سر_locked(&پشته->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب بیرون;
	}

	إذا (!locked && (z_waitq_سر(&پشته->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب بیرون;
	}

	إذا ((پشته->flags & K_پشته_FLAG_ALLOC) != (uint8_t)0) {
		k_free(پشته->پایه);
		پشته->پایه = عدم;
		پشته->flags &= ~K_پشته_FLAG_ALLOC;
		freed = true;
	}

بیرون:
	k_spin_unlock(&پشته->قفل, کلید);

#اگر_تعریف_شده CONFIG_OBJ_CORE_پشته
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(پشته));
	}
#پایان_اگر /* CONFIG_OBJ_CORE_STACK */
	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_پشته, cleanup, پشته, ret);

	أرجع ret;
}

صحيح k_پشته_cleanup(بنية k_پشته *پشته)
{
	أرجع z_پشته_cleanup(پشته, false);
}

صحيح z_impl_k_پشته_افزودن(بنية k_پشته *پشته, پشته_داده_t داده)
{
	صحيح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_قفل(&پشته->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_پشته, افزودن, پشته);

	إذا (پشته->بعدی == پشته->بالاترین) {
		ret = -ENOMEM;
		اذهب بیرون;
	}

	إذا (z_sched_wake(&پشته->wait_q, 0, (فراغ *)داده)) {
		z_reschedule(&پشته->قفل, کلید);
		اذهب انتها;
	} وإلا {
		*(پشته->بعدی) = داده;
		پشته->بعدی++;
		اذهب بیرون;
	}

بیرون:
	k_spin_unlock(&پشته->قفل, کلید);

انتها:
	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_پشته, افزودن, پشته, ret);

	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_پشته_افزودن(بنية k_پشته *پشته, پشته_داده_t داده)
{
	K_OOPS(K_SYSCALL_OBJ(پشته, K_OBJ_پشته));

	أرجع z_impl_k_پشته_افزودن(پشته, داده);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_impl_k_پشته_برداشت(بنية k_پشته *پشته, پشته_داده_t *داده,
		       k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحيح نتیجه;

	کلید = k_spin_قفل(&پشته->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_پشته, برداشت, پشته, timeout);

	إذا (likely(پشته->بعدی > پشته->پایه)) {
		پشته->بعدی--;
		*داده = *(پشته->بعدی);
		k_spin_unlock(&پشته->قفل, کلید);

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&پشته->قفل, کلید);

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_پشته, برداشت, پشته, timeout);

	نتیجه = z_pend_curr(&پشته->قفل, کلید, &پشته->wait_q, timeout);
	إذا (نتیجه != 0) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, نتیجه);

		أرجع نتیجه;
	}

	*داده = (پشته_داده_t)_current->پایه.جابجا_داده;

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, 0);

	أرجع 0;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_پشته_برداشت(بنية k_پشته *پشته,
				     پشته_داده_t *داده, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(پشته, K_OBJ_پشته));
	K_OOPS(K_SYSCALL_حافظه_WRITE(داده, حجم(پشته_داده_t)));
	أرجع z_impl_k_پشته_برداشت(پشته, داده, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
