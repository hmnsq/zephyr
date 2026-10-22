/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/kernel/smp.h>
#अन्तर्भाव <zephyr/devicetree.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/llext/symbol.h>
#अन्तर्भाव <kswap.h>
#अन्तर्भाव <kernel_internal.h>

स्थिर atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
स्थिर atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
स्थिर atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
स्थिर संरचना cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	शून्य *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

स्थिर संरचना k_spinlock cpu_start_lock;

unsigned पूर्णाङ्क z_smp_global_lock(शून्य)
{
	unsigned पूर्णाङ्क कुञ्चिका = arch_irq_lock();

	जइ (!_current->base.global_lock_गणनम्) {
		जाव (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_गणनम्++;

	प्रत्यागम कुञ्चिका;
}
EXPORT_चिह्नम्(z_smp_global_lock);

शून्य z_smp_global_unlock(unsigned पूर्णाङ्क कुञ्चिका)
{
	जइ (_current->base.global_lock_गणनम् != 0U) {
		_current->base.global_lock_गणनम्--;

		जइ (!_current->base.global_lock_गणनम्) {
			(शून्य)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(कुञ्चिका);
}
EXPORT_चिह्नम्(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
शून्य z_smp_release_global_lock(संरचना k_thread *thread)
{
	जइ (!thread->base.global_lock_गणनम्) {
		(शून्य)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
स्थिर अन्तःस्थ शून्य local_delay(शून्य)
{
	प्रति (चञ्चल पूर्णाङ्क i = 0; i < 1000; i++) {
	}
}

स्थिर शून्य wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	जाव (!atomic_get(start_flag)) {
		local_delay();
	}
}

स्थिर अन्तःस्थ शून्य smp_init_शिखरम्(शून्य *arg)
{
	संरचना cpu_start_cb csc = arg ? *(संरचना cpu_start_cb *)arg : (संरचना cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(शून्य)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	जइ ((arg == रिक्त) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	जइ ((arg == रिक्त) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	जइ (csc.fn != रिक्त) {
		csc.fn(csc.arg);
	}

	जइ ((arg != रिक्त) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		प्रत्यागम;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

स्थिर शून्य start_cpu(पूर्णाङ्क id, संरचना cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(शून्य)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_राशिः_SIZE,
		       smp_init_शिखरम्, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	जाव (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

शून्य k_smp_cpu_start(पूर्णाङ्क id, smp_init_fn fn, शून्य *arg)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(शून्य)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, कुञ्चिका);
}

शून्य k_smp_cpu_resume(पूर्णाङ्क id, smp_init_fn fn, शून्य *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#अण्णहा
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(शून्य)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, कुञ्चिका);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#जइ DT_ग्रन्थिः_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(ग्रन्थिः_id) DT_PROP_OR(ग्रन्थिः_id, zephyr_deferred_start, 0),
स्थिर ध्रुव bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned पूर्णाङ्क)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#अण्णहा
#define CPU_START_DEFERRED(i) false
#endif

शून्य z_smp_init(शून्य)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(शून्य)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned पूर्णाङ्क अङ्कः_cpus = arch_अङ्कः_cpus();

	प्रति (पूर्णाङ्क i = 1; i < अङ्कः_cpus; i++) {
		जइ (CPU_START_DEFERRED(i)) {
			अनुवर्त;
		}
		z_init_cpu(i);
		start_cpu(i, रिक्त);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(शून्य)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(शून्य)
{
	प्रत्यागम !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ संरचना k_thread *z_smp_current_get(शून्य)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned पूर्णाङ्क कुञ्चिका = arch_irq_lock();
	संरचना k_thread *t = _raw_current;

	arch_irq_unlock(कुञ्चिका);
	प्रत्यागम t;
}
