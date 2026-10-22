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

ಸ್ಥಿರ atomic_t global_ಬೀಗ;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
ಸ್ಥಿರ atomic_t cpu_ಆರಂಭ_flag;

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
ಸ್ಥಿರ ರಚನೆ cpu_ಆರಂಭ_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	ಶೂನ್ಯ *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_ಆರಂಭ_fn;

ಸ್ಥಿರ ರಚನೆ k_spinlock cpu_ಆರಂಭ_ಬೀಗ;

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ z_smp_global_ಬೀಗ(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ = arch_irq_ಬೀಗ();

	ಆದರೆ (!_current->ತಳ.global_ಬೀಗ_ಎಣಿಕೆ) {
		ಆಗ (!atomic_cas(&global_ಬೀಗ, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->ತಳ.global_ಬೀಗ_ಎಣಿಕೆ++;

	ಹಿಂತಿರುಗಿ ಕೀಲಿ;
}
EXPORT_ಚಿಹ್ನೆ(z_smp_global_ಬೀಗ);

ಶೂನ್ಯ z_smp_global_unlock(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ)
{
	ಆದರೆ (_current->ತಳ.global_ಬೀಗ_ಎಣಿಕೆ != 0U) {
		_current->ತಳ.global_ಬೀಗ_ಎಣಿಕೆ--;

		ಆದರೆ (!_current->ತಳ.global_ಬೀಗ_ಎಣಿಕೆ) {
			(ಶೂನ್ಯ)atomic_clear(&global_ಬೀಗ);
		}
	}

	arch_irq_unlock(ಕೀಲಿ);
}
EXPORT_ಚಿಹ್ನೆ(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
ಶೂನ್ಯ z_smp_release_global_ಬೀಗ(ರಚನೆ k_thread *thread)
{
	ಆದರೆ (!thread->ತಳ.global_ಬೀಗ_ಎಣಿಕೆ) {
		(ಶೂನ್ಯ)atomic_clear(&global_ಬೀಗ);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ local_delay(ಶೂನ್ಯ)
{
	ಗಾಗಿ (ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i = 0; i < 1000; i++) {
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ wait_for_ಆರಂಭ_signal(atomic_t *ಆರಂಭ_flag)
{
	/* Wait for the signal to begin scheduling */
	ಆಗ (!atomic_get(ಆರಂಭ_flag)) {
		local_delay();
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ smp_init_ತುದಿ(ಶೂನ್ಯ *arg)
{
	ರಚನೆ cpu_ಆರಂಭ_cb csc = arg ? *(ರಚನೆ cpu_ಆರಂಭ_cb *)arg : (ರಚನೆ cpu_ಆರಂಭ_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(ಶೂನ್ಯ)atomic_ಗಣ(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_ಆರಂಭ_signal(&cpu_ಆರಂಭ_flag);

	ಆದರೆ ((arg == ಖಾಲಿ) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
	ಆದರೆ ((arg == ಖಾಲಿ) || csc.reinit_timer) {
		smp_timer_init();
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	ಆದರೆ (csc.fn != ಖಾಲಿ) {
		csc.fn(csc.arg);
	}

	ಆದರೆ ((arg != ಖಾಲಿ) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		ಹಿಂತಿರುಗಿ;
	}

	/* Let scheduler decide what thread to run next. */
	z_ಅದಲುಬದಲು_unlocked();

	ಸಂಕೇತ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ಸ್ಥಿರ ಶೂನ್ಯ ಆರಂಭ_cpu(ಪೂರ್ಣಾಂಕ id, ರಚನೆ cpu_ಆರಂಭ_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(ಶೂನ್ಯ)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_ಆರಂಭ(id, z_ಅಡಚಣೆ_stacks[id], CONFIG_ISR_ರಾಶಿ_ಗಾತ್ರ,
		       smp_init_ತುದಿ, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	ಆಗ (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

ಶೂನ್ಯ k_smp_cpu_ಆರಂಭ(ಪೂರ್ಣಾಂಕ id, smp_init_fn fn, ಶೂನ್ಯ *arg)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&cpu_ಆರಂಭ_ಬೀಗ);

	cpu_ಆರಂಭ_fn.fn = fn;
	cpu_ಆರಂಭ_fn.arg = arg;
	cpu_ಆರಂಭ_fn.invoke_sched = true;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
	cpu_ಆರಂಭ_fn.reinit_timer = true;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ಶೂನ್ಯ)atomic_ಗಣ(&cpu_ಆರಂಭ_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	ಆರಂಭ_cpu(id, &cpu_ಆರಂಭ_fn);

	k_spin_unlock(&cpu_ಆರಂಭ_ಬೀಗ, ಕೀಲಿ);
}

ಶೂನ್ಯ k_smp_cpu_resume(ಪೂರ್ಣಾಂಕ id, smp_init_fn fn, ಶೂನ್ಯ *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&cpu_ಆರಂಭ_ಬೀಗ);

	cpu_ಆರಂಭ_fn.fn = fn;
	cpu_ಆರಂಭ_fn.arg = arg;
	cpu_ಆರಂಭ_fn.invoke_sched = invoke_sched;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
	cpu_ಆರಂಭ_fn.reinit_timer = reinit_timer;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(reinit_timer);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ಶೂನ್ಯ)atomic_ಗಣ(&cpu_ಆರಂಭ_flag, 1);

	/* Start the CPU! */
	ಆರಂಭ_cpu(id, &cpu_ಆರಂಭ_fn);

	k_spin_unlock(&cpu_ಆರಂಭ_ಬೀಗ, ಕೀಲಿ);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#ಆದರೆ DT_ಗಂಟು_EXISTS(DT_ಮಾರ್ಗ(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು CPU_DEFERRED_ಆರಂಭ_FLAG(ಗಂಟು_id) DT_PROP_OR(ಗಂಟು_id, zephyr_deferred_ಆರಂಭ, 0),
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ bool cpu_deferred_ಆರಂಭ[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_ಆರಂಭ_FLAG)
};
#ವ್ಯಾಖ್ಯಾನಿಸು CPU_ಆರಂಭ_DEFERRED(i)                                                  \
	(((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(i) < ARRAY_ಗಾತ್ರ(cpu_deferred_ಆರಂಭ)) &&               \
	 cpu_deferred_ಆರಂಭ[i])
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು CPU_ಆರಂಭ_DEFERRED(i) false
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ z_smp_init(ಶೂನ್ಯ)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(ಶೂನ್ಯ)atomic_clear(&cpu_ಆರಂಭ_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 1; i < ಅಂಕೆ_cpus; i++) {
		ಆದರೆ (CPU_ಆರಂಭ_DEFERRED(i)) {
			ಮುಂದುವರಿ;
		}
		z_init_cpu(i);
		ಆರಂಭ_cpu(i, ಖಾಲಿ);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(ಶೂನ್ಯ)atomic_ಗಣ(&cpu_ಆರಂಭ_flag, 1);
}

bool z_smp_cpu_mobile(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ !arch_is_ಒಳ_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ ರಚನೆ k_thread *z_smp_current_get(ಶೂನ್ಯ)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ = arch_irq_ಬೀಗ();
	ರಚನೆ k_thread *t = _raw_current;

	arch_irq_unlock(ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ t;
}
