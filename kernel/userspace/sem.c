/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/sem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYS_SEM_MINIMUM      0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ਸਥਿਰ ਸਤਰਅੰਦਰ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_ਮੁੱਲ, new_ਮੁੱਲ;

	ਕਰੋ {
		old_ਮੁੱਲ = atomic_get(val);
		ਜੇ (old_ਮੁੱਲ < minimum) {
			ਰੋਕੋ;
		}

		new_ਮੁੱਲ = old_ਮੁੱਲ - 1;
	} ਜਦੋਂ (atomic_cas(val, old_ਮੁੱਲ, new_ਮੁੱਲ) == 0);

	ਵਾਪਸ old_ਮੁੱਲ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t ਵੱਧ_ਤੋਂ_ਵੱਧ)
{
	atomic_t old_ਮੁੱਲ, new_ਮੁੱਲ;

	ਕਰੋ {
		old_ਮੁੱਲ = atomic_get(val);
		ਜੇ (old_ਮੁੱਲ >= ਵੱਧ_ਤੋਂ_ਵੱਧ) {
			ਰੋਕੋ;
		}

		new_ਮੁੱਲ = ((old_ਮੁੱਲ < minimum) ? minimum : old_ਮੁੱਲ) + 1;
	} ਜਦੋਂ (atomic_cas(val, old_ਮੁੱਲ, new_ਮੁੱਲ) == 0U);

	ਵਾਪਸ old_ਮੁੱਲ;
}

ਪੂਰਨ sys_sem_init(ਬਣਤਰ sys_sem *sem, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ initial_ਗਿਣਤੀ,
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਹੱਦਬੰਦੀ)
{
	ਜੇ ((sem == ਨਿਰਾ) || (ਹੱਦਬੰਦੀ == SYS_SEM_MINIMUM) ||
	    (initial_ਗਿਣਤੀ > ਹੱਦਬੰਦੀ) || (ਹੱਦਬੰਦੀ > ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)) {
		ਵਾਪਸ -EINVAL;
	}

	(ਖਾਲੀ)atomic_ਸੈੱਟ(&sem->futex.val, initial_ਗਿਣਤੀ);
	sem->ਹੱਦਬੰਦੀ = ਹੱਦਬੰਦੀ;

	ਵਾਪਸ 0;
}

ਪੂਰਨ sys_sem_give(ਬਣਤਰ sys_sem *sem)
{
	ਪੂਰਨ ret = 0;
	atomic_t old_ਮੁੱਲ;

	old_ਮੁੱਲ = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->ਹੱਦਬੰਦੀ);
	ਜੇ (old_ਮੁੱਲ < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ਜੇ (ret > 0) {
			ਵਾਪਸ 0;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (old_ਮੁੱਲ >= sem->ਹੱਦਬੰਦੀ) {
		ਵਾਪਸ -EAGAIN;
	} ਨਹੀਂਤਾਂ {
		;
	}
	ਵਾਪਸ ret;
}

ਪੂਰਨ sys_sem_take(ਬਣਤਰ sys_sem *sem, k_timeout_t timeout)
{
	ਪੂਰਨ ret = 0;
	atomic_t old_ਮੁੱਲ;

	ਕਰੋ {
		old_ਮੁੱਲ = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ਜੇ (old_ਮੁੱਲ > 0) {
			ਵਾਪਸ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ਜਦੋਂ (ret == 0 || ret == -EAGAIN);

	ਵਾਪਸ ret;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sys_sem_ਗਿਣਤੀ_get(ਬਣਤਰ sys_sem *sem)
{
	ਪੂਰਨ ਮੁੱਲ = atomic_get(&sem->futex.val);

	ਵਾਪਸ (ਮੁੱਲ > SYS_SEM_MINIMUM) ? ਮੁੱਲ : SYS_SEM_MINIMUM;
}
