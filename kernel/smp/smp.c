/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/kernel/smp.h>
#അന്തര്ഭാവ <zephyr/devicetree.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <zephyr/llext/symbol.h>
#അന്തര്ഭാവ <kswap.h>
#അന്തര്ഭാവ <kernel_internal.h>

സ്ഥിതം atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
സ്ഥിതം atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
സ്ഥിതം atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
സ്ഥിതം ഘടന cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	ശൂന്യം *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

സ്ഥിതം ഘടന k_spinlock cpu_start_lock;

unsigned പൂർണ്ണസംഖ്യ z_smp_global_lock(ശൂന്യം)
{
	unsigned പൂർണ്ണസംഖ്യ താക്കോൽ = arch_irq_lock();

	എങ്കിൽ (!_current->base.global_lock_എണ്ണം) {
		ആകുമ്പോൾ (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_എണ്ണം++;

	തിരികെ താക്കോൽ;
}
EXPORT_ചിഹ്നം(z_smp_global_lock);

ശൂന്യം z_smp_global_unlock(unsigned പൂർണ്ണസംഖ്യ താക്കോൽ)
{
	എങ്കിൽ (_current->base.global_lock_എണ്ണം != 0U) {
		_current->base.global_lock_എണ്ണം--;

		എങ്കിൽ (!_current->base.global_lock_എണ്ണം) {
			(ശൂന്യം)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(താക്കോൽ);
}
EXPORT_ചിഹ്നം(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
ശൂന്യം z_smp_release_global_lock(ഘടന k_thread *thread)
{
	എങ്കിൽ (!thread->base.global_lock_എണ്ണം) {
		(ശൂന്യം)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
സ്ഥിതം വരിയിൽ ശൂന്യം local_delay(ശൂന്യം)
{
	വേണ്ടി (അസ്ഥിര പൂർണ്ണസംഖ്യ i = 0; i < 1000; i++) {
	}
}

സ്ഥിതം ശൂന്യം wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	ആകുമ്പോൾ (!atomic_get(start_flag)) {
		local_delay();
	}
}

സ്ഥിതം വരിയിൽ ശൂന്യം smp_init_മുകൾ(ശൂന്യം *arg)
{
	ഘടന cpu_start_cb csc = arg ? *(ഘടന cpu_start_cb *)arg : (ഘടന cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(ശൂന്യം)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	എങ്കിൽ ((arg == ശൂന്യ) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	എങ്കിൽ ((arg == ശൂന്യ) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	എങ്കിൽ (csc.fn != ശൂന്യ) {
		csc.fn(csc.arg);
	}

	എങ്കിൽ ((arg != ശൂന്യ) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		തിരികെ;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

സ്ഥിതം ശൂന്യം start_cpu(പൂർണ്ണസംഖ്യ id, ഘടന cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(ശൂന്യം)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_അട്ടി_SIZE,
		       smp_init_മുകൾ, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	ആകുമ്പോൾ (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

ശൂന്യം k_smp_cpu_start(പൂർണ്ണസംഖ്യ id, smp_init_fn fn, ശൂന്യം *arg)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ശൂന്യം)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, താക്കോൽ);
}

ശൂന്യം k_smp_cpu_resume(പൂർണ്ണസംഖ്യ id, smp_init_fn fn, ശൂന്യം *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#അല്ലെങ്കിൽ
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ശൂന്യം)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, താക്കോൽ);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#എങ്കിൽ DT_കണ്ണി_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(കണ്ണി_id) DT_PROP_OR(കണ്ണി_id, zephyr_deferred_start, 0),
സ്ഥിതം സ്ഥിരം bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned പൂർണ്ണസംഖ്യ)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#അല്ലെങ്കിൽ
#define CPU_START_DEFERRED(i) false
#endif

ശൂന്യം z_smp_init(ശൂന്യം)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(ശൂന്യം)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned പൂർണ്ണസംഖ്യ അക്കം_cpus = arch_അക്കം_cpus();

	വേണ്ടി (പൂർണ്ണസംഖ്യ i = 1; i < അക്കം_cpus; i++) {
		എങ്കിൽ (CPU_START_DEFERRED(i)) {
			തുടരുക;
		}
		z_init_cpu(i);
		start_cpu(i, ശൂന്യ);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(ശൂന്യം)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(ശൂന്യം)
{
	തിരികെ !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ ഘടന k_thread *z_smp_current_get(ശൂന്യം)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned പൂർണ്ണസംഖ്യ താക്കോൽ = arch_irq_lock();
	ഘടന k_thread *t = _raw_current;

	arch_irq_unlock(താക്കോൽ);
	തിരികെ t;
}
