/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel/smp.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/devicetree.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/llext/symbol.h>
#ଅନ୍ତର୍ଭାଵ <kswap.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>

ସ୍ଥିର atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
ସ୍ଥିର atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
ସ୍ଥିର atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
ସ୍ଥିର ସଂରଚନା cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	ଶୂନ୍ୟ *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

ସ୍ଥିର ସଂରଚନା k_spinlock cpu_start_lock;

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_smp_global_lock(ଶୂନ୍ୟ)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି = arch_irq_lock();

	ଯଦି (!_current->base.global_lock_ଗଣନା) {
		ଯେତେବେଳେ (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_ଗଣନା++;

	ଫେରନ୍ତୁ ଚାବି;
}
EXPORT_ପ୍ରତୀକ(z_smp_global_lock);

ଶୂନ୍ୟ z_smp_global_unlock(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି)
{
	ଯଦି (_current->base.global_lock_ଗଣନା != 0U) {
		_current->base.global_lock_ଗଣନା--;

		ଯଦି (!_current->base.global_lock_ଗଣନା) {
			(ଶୂନ୍ୟ)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(ଚାବି);
}
EXPORT_ପ୍ରତୀକ(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
ଶୂନ୍ୟ z_smp_release_global_lock(ସଂରଚନା k_thread *thread)
{
	ଯଦି (!thread->base.global_lock_ଗଣନା) {
		(ଶୂନ୍ୟ)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ local_delay(ଶୂନ୍ୟ)
{
	ପାଇଁ (ଅସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < 1000; i++) {
	}
}

ସ୍ଥିର ଶୂନ୍ୟ wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	ଯେତେବେଳେ (!atomic_get(start_flag)) {
		local_delay();
	}
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ smp_init_ଶୀର୍ଷ(ଶୂନ୍ୟ *arg)
{
	ସଂରଚନା cpu_start_cb csc = arg ? *(ସଂରଚନା cpu_start_cb *)arg : (ସଂରଚନା cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(ଶୂନ୍ୟ)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	ଯଦି ((arg == ଖାଲି) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	ଯଦି ((arg == ଖାଲି) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	ଯଦି (csc.fn != ଖାଲି) {
		csc.fn(csc.arg);
	}

	ଯଦି ((arg != ଖାଲି) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		ଫେରନ୍ତୁ;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ସ୍ଥିର ଶୂନ୍ୟ start_cpu(ପୂର୍ଣ୍ଣସଂଖ୍ୟା id, ସଂରଚନା cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(ଶୂନ୍ୟ)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_ଗଦା_SIZE,
		       smp_init_ଶୀର୍ଷ, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	ଯେତେବେଳେ (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

ଶୂନ୍ୟ k_smp_cpu_start(ପୂର୍ଣ୍ଣସଂଖ୍ୟା id, smp_init_fn fn, ଶୂନ୍ୟ *arg)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ଶୂନ୍ୟ)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, ଚାବି);
}

ଶୂନ୍ୟ k_smp_cpu_resume(ପୂର୍ଣ୍ଣସଂଖ୍ୟା id, smp_init_fn fn, ଶୂନ୍ୟ *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#ଅନ୍ୟଥା
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(ଶୂନ୍ୟ)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, ଚାବି);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#ଯଦି DT_ଗଣ୍ଠି_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(ଗଣ୍ଠି_id) DT_PROP_OR(ଗଣ୍ଠି_id, zephyr_deferred_start, 0),
ସ୍ଥିର ଅଚଳ bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#ଅନ୍ୟଥା
#define CPU_START_DEFERRED(i) false
#endif

ଶୂନ୍ୟ z_smp_init(ଶୂନ୍ୟ)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(ଶୂନ୍ୟ)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_cpus = arch_ଅଙ୍କ_cpus();

	ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 1; i < ଅଙ୍କ_cpus; i++) {
		ଯଦି (CPU_START_DEFERRED(i)) {
			ଜାରି;
		}
		z_init_cpu(i);
		start_cpu(i, ଖାଲି);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(ଶୂନ୍ୟ)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ ସଂରଚନା k_thread *z_smp_current_get(ଶୂନ୍ୟ)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି = arch_irq_lock();
	ସଂରଚନା k_thread *t = _raw_current;

	arch_irq_unlock(ଚାବି);
	ଫେରନ୍ତୁ t;
}
