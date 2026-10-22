/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/kernel/smp.h>
#అన్తర్భావ <zephyr/devicetree.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/llext/symbol.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <kernel_internal.h>

స్థిర atomic_t global_తాళం;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
స్థిర atomic_t cpu_ప్రారంభం_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
స్థిర atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
స్థిర నిర్మాణం cpu_ప్రారంభం_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	శూన్య *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_ప్రారంభం_fn;

స్థిర నిర్మాణం k_spinlock cpu_ప్రారంభం_తాళం;

గుర్తులేని పూర్ణాంకం z_smp_global_తాళం(శూన్య)
{
	గుర్తులేని పూర్ణాంకం కీలకం = arch_irq_తాళం();

	ఒకవేళ (!_current->పునాది.global_తాళం_లెక్క) {
		అయితే (!atomic_cas(&global_తాళం, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->పునాది.global_తాళం_లెక్క++;

	తిరిగి కీలకం;
}
EXPORT_చిహ్నం(z_smp_global_తాళం);

శూన్య z_smp_global_unlock(గుర్తులేని పూర్ణాంకం కీలకం)
{
	ఒకవేళ (_current->పునాది.global_తాళం_లెక్క != 0U) {
		_current->పునాది.global_తాళం_లెక్క--;

		ఒకవేళ (!_current->పునాది.global_తాళం_లెక్క) {
			(శూన్య)atomic_clear(&global_తాళం);
		}
	}

	arch_irq_unlock(కీలకం);
}
EXPORT_చిహ్నం(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
శూన్య z_smp_release_global_తాళం(నిర్మాణం k_thread *thread)
{
	ఒకవేళ (!thread->పునాది.global_తాళం_లెక్క) {
		(శూన్య)atomic_clear(&global_తాళం);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
స్థిర అంతఃవరుస శూన్య local_delay(శూన్య)
{
	కొరకు (అస్థిర పూర్ణాంకం i = 0; i < 1000; i++) {
	}
}

స్థిర శూన్య wait_for_ప్రారంభం_signal(atomic_t *ప్రారంభం_flag)
{
	/* Wait for the signal to begin scheduling */
	అయితే (!atomic_get(ప్రారంభం_flag)) {
		local_delay();
	}
}

స్థిర అంతఃవరుస శూన్య smp_init_శిఖరం(శూన్య *arg)
{
	నిర్మాణం cpu_ప్రారంభం_cb csc = arg ? *(నిర్మాణం cpu_ప్రారంభం_cb *)arg : (నిర్మాణం cpu_ప్రారంభం_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(శూన్య)atomic_సమితి(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_ప్రారంభం_signal(&cpu_ప్రారంభం_flag);

	ఒకవేళ ((arg == ఖాళీ) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
	ఒకవేళ ((arg == ఖాళీ) || csc.reinit_timer) {
		smp_timer_init();
	}
#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	ఒకవేళ (csc.fn != ఖాళీ) {
		csc.fn(csc.arg);
	}

	ఒకవేళ ((arg != ఖాళీ) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		తిరిగి;
	}

	/* Let scheduler decide what thread to run next. */
	z_మార్పిడి_unlocked();

	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}

స్థిర శూన్య ప్రారంభం_cpu(పూర్ణాంకం id, నిర్మాణం cpu_ప్రారంభం_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(శూన్య)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_ప్రారంభం(id, z_అంతరాయం_stacks[id], CONFIG_ISR_దొంతర_కొలత,
		       smp_init_శిఖరం, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	అయితే (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

శూన్య k_smp_cpu_ప్రారంభం(పూర్ణాంకం id, smp_init_fn fn, శూన్య *arg)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&cpu_ప్రారంభం_తాళం);

	cpu_ప్రారంభం_fn.fn = fn;
	cpu_ప్రారంభం_fn.arg = arg;
	cpu_ప్రారంభం_fn.invoke_sched = true;

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
	cpu_ప్రారంభం_fn.reinit_timer = true;
#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(శూన్య)atomic_సమితి(&cpu_ప్రారంభం_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	ప్రారంభం_cpu(id, &cpu_ప్రారంభం_fn);

	k_spin_unlock(&cpu_ప్రారంభం_తాళం, కీలకం);
}

శూన్య k_smp_cpu_resume(పూర్ణాంకం id, smp_init_fn fn, శూన్య *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&cpu_ప్రారంభం_తాళం);

	cpu_ప్రారంభం_fn.fn = fn;
	cpu_ప్రారంభం_fn.arg = arg;
	cpu_ప్రారంభం_fn.invoke_sched = invoke_sched;

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
	cpu_ప్రారంభం_fn.reinit_timer = reinit_timer;
#లేకపోతే
	ARG_వాడని(reinit_timer);
#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(శూన్య)atomic_సమితి(&cpu_ప్రారంభం_flag, 1);

	/* Start the CPU! */
	ప్రారంభం_cpu(id, &cpu_ప్రారంభం_fn);

	k_spin_unlock(&cpu_ప్రారంభం_తాళం, కీలకం);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#ఒకవేళ DT_కణుపు_EXISTS(DT_మార్గం(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#నిర్వచించు CPU_DEFERRED_ప్రారంభం_FLAG(కణుపు_id) DT_PROP_OR(కణుపు_id, zephyr_deferred_ప్రారంభం, 0),
స్థిర స్థిరరాశి bool cpu_deferred_ప్రారంభం[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_ప్రారంభం_FLAG)
};
#నిర్వచించు CPU_ప్రారంభం_DEFERRED(i)                                                  \
	(((గుర్తులేని పూర్ణాంకం)(i) < ARRAY_కొలత(cpu_deferred_ప్రారంభం)) &&               \
	 cpu_deferred_ప్రారంభం[i])
#లేకపోతే
#నిర్వచించు CPU_ప్రారంభం_DEFERRED(i) false
#అయితే_ముగింపు

శూన్య z_smp_init(శూన్య)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(శూన్య)atomic_clear(&cpu_ప్రారంభం_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	గుర్తులేని పూర్ణాంకం అంకె_cpus = arch_అంకె_cpus();

	కొరకు (పూర్ణాంకం i = 1; i < అంకె_cpus; i++) {
		ఒకవేళ (CPU_ప్రారంభం_DEFERRED(i)) {
			కొనసాగు;
		}
		z_init_cpu(i);
		ప్రారంభం_cpu(i, ఖాళీ);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(శూన్య)atomic_సమితి(&cpu_ప్రారంభం_flag, 1);
}

bool z_smp_cpu_mobile(శూన్య)
{
	తిరిగి !arch_is_లోపల_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ నిర్మాణం k_thread *z_smp_current_get(శూన్య)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	గుర్తులేని పూర్ణాంకం కీలకం = arch_irq_తాళం();
	నిర్మాణం k_thread *t = _raw_current;

	arch_irq_unlock(కీలకం);
	తిరిగి t;
}
