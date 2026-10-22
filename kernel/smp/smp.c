/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/smp.h>
#ಅನ್ತರ್ಭಾವ <zephyr/devicetree.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/llext/symbol.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>

ಸ್ಥಿರ atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
ಸ್ಥಿರ atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
ಸ್ಥಿರ atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
ಸ್ಥಿರ ಸಂರಚನಾ cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	ಶೂನ್ಯ *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock cpu_start_lock;

unsigned ಪೂರ್ಣಾಙ್ಕ z_smp_global_lock(ಶೂನ್ಯ)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ = arch_irq_lock();

	ಯದಿ (!_current->base.global_lock_ಎಣಿಕೆ) {
		ಯಾವತ್ (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_ಎಣಿಕೆ++;

	ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ;
}
EXPORT_ಚಿಹ್ನೆ(z_smp_global_lock);

ಶೂನ್ಯ z_smp_global_unlock(unsigned ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ)
{
	ಯದಿ (_current->base.global_lock_ಎಣಿಕೆ != 0U) {
		_current->base.global_lock_ಎಣಿಕೆ--;

		ಯದಿ (!_current->base.global_lock_ಎಣಿಕೆ) {
			(ಶೂನ್ಯ)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(ಕೀಲಿ);
}
EXPORT_ಚಿಹ್ನೆ(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
ಶೂನ್ಯ z_smp_release_global_lock(ಸಂರಚನಾ k_thread *thread)
{
	ಯದಿ (!thread->base.global_lock_ಎಣಿಕೆ) {
		(ಶೂನ್ಯ)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ local_delay(ಶೂನ್ಯ)
{
	ಪ್ರತಿ (ಚಞ್ಚಲ ಪೂರ್ಣಾಙ್ಕ i = 0; i < 1000; i++) {
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	ಯಾವತ್ (!atomic_get(start_flag)) {
		local_delay();
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ smp_init_ತುದಿ(ಶೂನ್ಯ *arg)
{
	ಸಂರಚನಾ cpu_start_cb csc = arg ? *(ಸಂರಚನಾ cpu_start_cb *)arg : (ಸಂರಚನಾ cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(ಶೂನ್ಯ)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	ಯದಿ ((arg == ರಿಕ್ತ) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	ಯದಿ ((arg == ರಿಕ್ತ) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	ಯದಿ (csc.fn != ರಿಕ್ತ) {
		csc.fn(csc.arg);
	}

	ಯದಿ ((arg != ರಿಕ್ತ) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ಸ್ಥಿರ ಶೂನ್ಯ start_cpu(ಪೂರ್ಣಾಙ್ಕ id, ಸಂರಚನಾ cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(ಶೂನ್ಯ)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_ರಾಶಿ_SIZE,
		       smp_init_ತುದಿ, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	ಯಾವತ್ (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

ಶೂನ್ಯ k_smp_cpu_start(ಪೂರ್ಣಾಙ್ಕ id, smp_init_fn fn, ಶೂನ್ಯ *arg)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ಶೂನ್ಯ)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, ಕೀಲಿ);
}

ಶೂನ್ಯ k_smp_cpu_resume(ಪೂರ್ಣಾಙ್ಕ id, smp_init_fn fn, ಶೂನ್ಯ *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#ಅನ್ಯಥಾ
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ಶೂನ್ಯ)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, ಕೀಲಿ);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#ಯದಿ DT_ಗಂಟು_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(ಗಂಟು_id) DT_PROP_OR(ಗಂಟು_id, zephyr_deferred_start, 0),
ಸ್ಥಿರ ಧ್ರುವ bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned ಪೂರ್ಣಾಙ್ಕ)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#ಅನ್ಯಥಾ
#define CPU_START_DEFERRED(i) false
#endif

ಶೂನ್ಯ z_smp_init(ಶೂನ್ಯ)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(ಶೂನ್ಯ)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 1; i < ಅಂಕೆ_cpus; i++) {
		ಯದಿ (CPU_START_DEFERRED(i)) {
			ಅನುವರ್ತ;
		}
		z_init_cpu(i);
		start_cpu(i, ರಿಕ್ತ);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(ಶೂನ್ಯ)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ ಸಂರಚನಾ k_thread *z_smp_current_get(ಶೂನ್ಯ)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ = arch_irq_lock();
	ಸಂರಚನಾ k_thread *t = _raw_current;

	arch_irq_unlock(ಕೀಲಿ);
	ಪ್ರತ್ಯಾಗಮ t;
}
