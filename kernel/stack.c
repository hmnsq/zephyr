/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <stdbool.h>

#నిర్వచితమైతే CONFIG_OBJ_CORE_దొంతర
K_OBJ_TYPE_DEFINE(obj_type_దొంతర, k_దొంతర, K_OBJ_TYPE_దొంతర_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STACK */

శూన్య k_దొంతర_init(నిర్మాణం k_దొంతర *దొంతర, దొంతర_దత్తాంశం_t *బఫరు,
		  uint32_t అంకె_entries)
{
	z_waitq_init(&దొంతర->wait_q);
	దొంతర->తాళం = (నిర్మాణం k_spinlock) {};
	దొంతర->తదుపరి = బఫరు;
	దొంతర->పునాది = బఫరు;
	దొంతర->శిఖరం = దొంతర->పునాది + అంకె_entries;
	దొంతర->flags = 0;

	SYS_పోర్టు_TRACING_OBJ_INIT(k_దొంతర, దొంతర);
	k_object_init(దొంతర);

#నిర్వచితమైతే CONFIG_OBJ_CORE_దొంతర
	k_obj_core_init_and_link(K_OBJ_CORE(దొంతర), &obj_type_దొంతర);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_దొంతర_alloc_init(నిర్మాణం k_దొంతర *దొంతర, uint32_t అంకె_entries)
{
	శూన్య *బఫరు;
	size_t మొత్తం_కొలత;
	int32_t ret;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_దొంతర, alloc_init, దొంతర);

	/* Reject allocation sizes that cannot be represented. */
	ఒకవేళ (కొలత_mul_అధికప్రవాహం(అంకె_entries, పరిమాణం(దొంతర_దత్తాంశం_t), &మొత్తం_కొలత)) {
		ret = -ENOMEM;
	} లేకపోతే {
		బఫరు = z_thread_malloc(మొత్తం_కొలత);
		ఒకవేళ (బఫరు != ఖాళీ) {
			k_దొంతర_init(దొంతర, బఫరు, అంకె_entries);
			దొంతర->flags = K_దొంతర_FLAG_ALLOC;
			ret = 0;
		} లేకపోతే {
			ret = -ENOMEM;
		}
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_దొంతర, alloc_init, దొంతర, ret);

	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస int32_t z_vrfy_k_దొంతర_alloc_init(నిర్మాణం k_దొంతర *దొంతర,
					      uint32_t అంకె_entries)
{
	size_t మొత్తం_కొలత;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(దొంతర, K_OBJ_దొంతర));
	K_OOPS(K_SYSCALL_VERIFY(అంకె_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!కొలత_mul_అధికప్రవాహం(అంకె_entries, పరిమాణం(దొంతర_దత్తాంశం_t),
					&మొత్తం_కొలత)));
	తిరిగి z_impl_k_దొంతర_alloc_init(దొంతర, అంకె_entries);
}
#అన్తర్భావ <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_దొంతర_cleanup(నిర్మాణం k_దొంతర *దొంతర, __maybe_unused bool locked)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_దొంతర, cleanup, దొంతర);

	పూర్ణాంకం ret = 0;
	bool freed = false;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&దొంతర->తాళం);

	ఒకవేళ (locked && (z_waitq_తల_locked(&దొంతర->wait_q) != ఖాళీ)) {
		ret = -EAGAIN;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (!locked && (z_waitq_తల(&దొంతర->wait_q) != ఖాళీ)) {
		ret = -EAGAIN;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ ((దొంతర->flags & K_దొంతర_FLAG_ALLOC) != (uint8_t)0) {
		k_free(దొంతర->పునాది);
		దొంతర->పునాది = ఖాళీ;
		దొంతర->flags &= ~K_దొంతర_FLAG_ALLOC;
		freed = true;
	}

బహిర్గతం:
	k_spin_unlock(&దొంతర->తాళం, కీలకం);

#నిర్వచితమైతే CONFIG_OBJ_CORE_దొంతర
	ఒకవేళ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(దొంతర));
	}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STACK */
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_దొంతర, cleanup, దొంతర, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_దొంతర_cleanup(నిర్మాణం k_దొంతర *దొంతర)
{
	తిరిగి z_దొంతర_cleanup(దొంతర, false);
}

పూర్ణాంకం z_impl_k_దొంతర_నెట్టు(నిర్మాణం k_దొంతర *దొంతర, దొంతర_దత్తాంశం_t దత్తాంశం)
{
	పూర్ణాంకం ret = 0;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&దొంతర->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_దొంతర, నెట్టు, దొంతర);

	ఒకవేళ (దొంతర->తదుపరి == దొంతర->శిఖరం) {
		ret = -ENOMEM;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (z_sched_wake(&దొంతర->wait_q, 0, (శూన్య *)దత్తాంశం)) {
		z_reschedule(&దొంతర->తాళం, కీలకం);
		వెళ్ళు చివర;
	} లేకపోతే {
		*(దొంతర->తదుపరి) = దత్తాంశం;
		దొంతర->తదుపరి++;
		వెళ్ళు బహిర్గతం;
	}

బహిర్గతం:
	k_spin_unlock(&దొంతర->తాళం, కీలకం);

చివర:
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_దొంతర, నెట్టు, దొంతర, ret);

	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_దొంతర_నెట్టు(నిర్మాణం k_దొంతర *దొంతర, దొంతర_దత్తాంశం_t దత్తాంశం)
{
	K_OOPS(K_SYSCALL_OBJ(దొంతర, K_OBJ_దొంతర));

	తిరిగి z_impl_k_దొంతర_నెట్టు(దొంతర, దత్తాంశం);
}
#అన్తర్భావ <zephyr/syscalls/k_stack_push_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_దొంతర_తీయి(నిర్మాణం k_దొంతర *దొంతర, దొంతర_దత్తాంశం_t *దత్తాంశం,
		       k_timeout_t timeout)
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ఫలితం;

	కీలకం = k_spin_తాళం(&దొంతర->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_దొంతర, తీయి, దొంతర, timeout);

	ఒకవేళ (likely(దొంతర->తదుపరి > దొంతర->పునాది)) {
		దొంతర->తదుపరి--;
		*దత్తాంశం = *(దొంతర->తదుపరి);
		k_spin_unlock(&దొంతర->తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_దొంతర, తీయి, దొంతర, timeout, 0);

		తిరిగి 0;
	}

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&దొంతర->తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_దొంతర, తీయి, దొంతర, timeout, -EBUSY);

		తిరిగి -EBUSY;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_దొంతర, తీయి, దొంతర, timeout);

	ఫలితం = z_pend_curr(&దొంతర->తాళం, కీలకం, &దొంతర->wait_q, timeout);
	ఒకవేళ (ఫలితం != 0) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_దొంతర, తీయి, దొంతర, timeout, ఫలితం);

		తిరిగి ఫలితం;
	}

	*దత్తాంశం = (దొంతర_దత్తాంశం_t)_current->పునాది.మార్పిడి_దత్తాంశం;

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_దొంతర, తీయి, దొంతర, timeout, 0);

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_దొంతర_తీయి(నిర్మాణం k_దొంతర *దొంతర,
				     దొంతర_దత్తాంశం_t *దత్తాంశం, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(దొంతర, K_OBJ_దొంతర));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(దత్తాంశం, పరిమాణం(దొంతర_దత్తాంశం_t)));
	తిరిగి z_impl_k_దొంతర_తీయి(దొంతర, దత్తాంశం, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_stack_pop_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
