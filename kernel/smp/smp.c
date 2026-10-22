/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/kernel/smp.h>
#અન્તર્ભાવ <zephyr/devicetree.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <zephyr/llext/symbol.h>
#અન્તર્ભાવ <kswap.h>
#અન્તર્ભાવ <kernel_internal.h>

સ્થિર atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
સ્થિર atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
સ્થિર atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
સ્થિર સંરચના cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	શૂન્ય *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

સ્થિર સંરચના k_spinlock cpu_start_lock;

unsigned પૂર્ણાંક z_smp_global_lock(શૂન્ય)
{
	unsigned પૂર્ણાંક ચાવી = arch_irq_lock();

	જો (!_current->base.global_lock_ગણતરી) {
		જ્યારે (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_ગણતરી++;

	પાછા ચાવી;
}
EXPORT_પ્રતીક(z_smp_global_lock);

શૂન્ય z_smp_global_unlock(unsigned પૂર્ણાંક ચાવી)
{
	જો (_current->base.global_lock_ગણતરી != 0U) {
		_current->base.global_lock_ગણતરી--;

		જો (!_current->base.global_lock_ગણતરી) {
			(શૂન્ય)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(ચાવી);
}
EXPORT_પ્રતીક(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
શૂન્ય z_smp_release_global_lock(સંરચના k_thread *thread)
{
	જો (!thread->base.global_lock_ગણતરી) {
		(શૂન્ય)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
સ્થિર અંતઃરેખ શૂન્ય local_delay(શૂન્ય)
{
	માટે (અસ્થિર પૂર્ણાંક i = 0; i < 1000; i++) {
	}
}

સ્થિર શૂન્ય wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	જ્યારે (!atomic_get(start_flag)) {
		local_delay();
	}
}

સ્થિર અંતઃરેખ શૂન્ય smp_init_ટોચ(શૂન્ય *arg)
{
	સંરચના cpu_start_cb csc = arg ? *(સંરચના cpu_start_cb *)arg : (સંરચના cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(શૂન્ય)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	જો ((arg == ખાલી) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	જો ((arg == ખાલી) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	જો (csc.fn != ખાલી) {
		csc.fn(csc.arg);
	}

	જો ((arg != ખાલી) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		પાછા;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

સ્થિર શૂન્ય start_cpu(પૂર્ણાંક id, સંરચના cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(શૂન્ય)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_ઢગલો_SIZE,
		       smp_init_ટોચ, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	જ્યારે (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

શૂન્ય k_smp_cpu_start(પૂર્ણાંક id, smp_init_fn fn, શૂન્ય *arg)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(શૂન્ય)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, ચાવી);
}

શૂન્ય k_smp_cpu_resume(પૂર્ણાંક id, smp_init_fn fn, શૂન્ય *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#નહીંતર
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(શૂન્ય)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, ચાવી);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#જો DT_ગાંઠ_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(ગાંઠ_id) DT_PROP_OR(ગાંઠ_id, zephyr_deferred_start, 0),
સ્થિર અચલ bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned પૂર્ણાંક)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#નહીંતર
#define CPU_START_DEFERRED(i) false
#endif

શૂન્ય z_smp_init(શૂન્ય)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(શૂન્ય)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned પૂર્ણાંક અંક_cpus = arch_અંક_cpus();

	માટે (પૂર્ણાંક i = 1; i < અંક_cpus; i++) {
		જો (CPU_START_DEFERRED(i)) {
			ચાલુ;
		}
		z_init_cpu(i);
		start_cpu(i, ખાલી);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(શૂન્ય)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(શૂન્ય)
{
	પાછા !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ સંરચના k_thread *z_smp_current_get(શૂન્ય)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned પૂર્ણાંક ચાવી = arch_irq_lock();
	સંરચના k_thread *t = _raw_current;

	arch_irq_unlock(ચાવી);
	પાછા t;
}
