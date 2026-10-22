/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/kernel/smp.h>
#شامل <zephyr/devicetree.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/llext/symbol.h>
#شامل <kswap.h>
#شامل <kernel_internal.h>

ایستا atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
ایستا atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
ایستا atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
ایستا ساختار cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	تهی *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

ایستا ساختار k_spinlock cpu_start_lock;

unsigned صحیح z_smp_global_lock(تهی)
{
	unsigned صحیح کلید = arch_irq_lock();

	اگر (!_current->base.global_lock_شمار) {
		وقتی (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_شمار++;

	بازگشت کلید;
}
EXPORT_نماد(z_smp_global_lock);

تهی z_smp_global_unlock(unsigned صحیح کلید)
{
	اگر (_current->base.global_lock_شمار != 0U) {
		_current->base.global_lock_شمار--;

		اگر (!_current->base.global_lock_شمار) {
			(تهی)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(کلید);
}
EXPORT_نماد(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
تهی z_smp_release_global_lock(ساختار k_thread *thread)
{
	اگر (!thread->base.global_lock_شمار) {
		(تهی)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
ایستا درونخطی تهی local_delay(تهی)
{
	برای (فرار صحیح i = 0; i < 1000; i++) {
	}
}

ایستا تهی wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	وقتی (!atomic_get(start_flag)) {
		local_delay();
	}
}

ایستا درونخطی تهی smp_init_بالاترین(تهی *arg)
{
	ساختار cpu_start_cb csc = arg ? *(ساختار cpu_start_cb *)arg : (ساختار cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(تهی)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	اگر ((arg == پوچ) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	اگر ((arg == پوچ) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	اگر (csc.fn != پوچ) {
		csc.fn(csc.arg);
	}

	اگر ((arg != پوچ) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		بازگشت;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ایستا تهی start_cpu(صحیح id, ساختار cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(تهی)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_پشته_SIZE,
		       smp_init_بالاترین, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	وقتی (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

تهی k_smp_cpu_start(صحیح id, smp_init_fn fn, تهی *arg)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(تهی)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, کلید);
}

تهی k_smp_cpu_resume(صحیح id, smp_init_fn fn, تهی *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#وگرنه
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(تهی)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, کلید);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#اگر DT_گره_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(گره_id) DT_PROP_OR(گره_id, zephyr_deferred_start, 0),
ایستا ثابت bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned صحیح)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#وگرنه
#define CPU_START_DEFERRED(i) false
#endif

تهی z_smp_init(تهی)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(تهی)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned صحیح رقم_cpus = arch_رقم_cpus();

	برای (صحیح i = 1; i < رقم_cpus; i++) {
		اگر (CPU_START_DEFERRED(i)) {
			ادامه;
		}
		z_init_cpu(i);
		start_cpu(i, پوچ);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(تهی)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(تهی)
{
	بازگشت !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ ساختار k_thread *z_smp_current_get(تهی)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned صحیح کلید = arch_irq_lock();
	ساختار k_thread *t = _raw_current;

	arch_irq_unlock(کلید);
	بازگشت t;
}
