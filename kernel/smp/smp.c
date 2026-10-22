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

ساکن atomic_t global_تالا;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
ساکن atomic_t cpu_آغاز_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
ساکن atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
ساکن ساخت cpu_آغاز_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	خالی *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_آغاز_fn;

ساکن ساخت k_spinlock cpu_آغاز_تالا;

غیرعلامتی صحیح z_smp_global_تالا(خالی)
{
	غیرعلامتی صحیح کلید = arch_irq_تالا();

	اگر (!_current->بنیاد.global_تالا_گنتی) {
		جب (!atomic_cas(&global_تالا, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->بنیاد.global_تالا_گنتی++;

	واپس کلید;
}
EXPORT_علامت(z_smp_global_تالا);

خالی z_smp_global_unlock(غیرعلامتی صحیح کلید)
{
	اگر (_current->بنیاد.global_تالا_گنتی != 0U) {
		_current->بنیاد.global_تالا_گنتی--;

		اگر (!_current->بنیاد.global_تالا_گنتی) {
			(خالی)atomic_clear(&global_تالا);
		}
	}

	arch_irq_unlock(کلید);
}
EXPORT_علامت(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
خالی z_smp_release_global_تالا(ساخت k_thread *thread)
{
	اگر (!thread->بنیاد.global_تالا_گنتی) {
		(خالی)atomic_clear(&global_تالا);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
ساکن اندرونی خالی local_delay(خالی)
{
	برائے (متلون صحیح i = 0; i < 1000; i++) {
	}
}

ساکن خالی wait_for_آغاز_signal(atomic_t *آغاز_flag)
{
	/* Wait for the signal to begin scheduling */
	جب (!atomic_get(آغاز_flag)) {
		local_delay();
	}
}

ساکن اندرونی خالی smp_init_چوٹی(خالی *arg)
{
	ساخت cpu_آغاز_cb csc = arg ? *(ساخت cpu_آغاز_cb *)arg : (ساخت cpu_آغاز_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(خالی)atomic_سیٹ(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_آغاز_signal(&cpu_آغاز_flag);

	اگر ((arg == تہی) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
	اگر ((arg == تہی) || csc.reinit_timer) {
		smp_timer_init();
	}
#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	اگر (csc.fn != تہی) {
		csc.fn(csc.arg);
	}

	اگر ((arg != تہی) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		واپس;
	}

	/* Let scheduler decide what thread to run next. */
	z_تبادلہ_unlocked();

	رمز_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساکن خالی آغاز_cpu(صحیح id, ساخت cpu_آغاز_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(خالی)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_آغاز(id, z_مداخلت_stacks[id], CONFIG_ISR_ڈھیر_سائز,
		       smp_init_چوٹی, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	جب (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

خالی k_smp_cpu_آغاز(صحیح id, smp_init_fn fn, خالی *arg)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&cpu_آغاز_تالا);

	cpu_آغاز_fn.fn = fn;
	cpu_آغاز_fn.arg = arg;
	cpu_آغاز_fn.invoke_sched = true;

#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
	cpu_آغاز_fn.reinit_timer = true;
#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(خالی)atomic_سیٹ(&cpu_آغاز_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	آغاز_cpu(id, &cpu_آغاز_fn);

	k_spin_unlock(&cpu_آغاز_تالا, کلید);
}

خالی k_smp_cpu_resume(صحیح id, smp_init_fn fn, خالی *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&cpu_آغاز_تالا);

	cpu_آغاز_fn.fn = fn;
	cpu_آغاز_fn.arg = arg;
	cpu_آغاز_fn.invoke_sched = invoke_sched;

#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
	cpu_آغاز_fn.reinit_timer = reinit_timer;
#ورنہ
	ARG_غیر_مستعمل(reinit_timer);
#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(خالی)atomic_سیٹ(&cpu_آغاز_flag, 1);

	/* Start the CPU! */
	آغاز_cpu(id, &cpu_آغاز_fn);

	k_spin_unlock(&cpu_آغاز_تالا, کلید);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#اگر DT_گرہ_EXISTS(DT_راستہ(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#تعریف_کرو CPU_DEFERRED_آغاز_FLAG(گرہ_id) DT_PROP_OR(گرہ_id, zephyr_deferred_آغاز, 0),
ساکن ثابت bool cpu_deferred_آغاز[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_آغاز_FLAG)
};
#تعریف_کرو CPU_آغاز_DEFERRED(i)                                                  \
	(((غیرعلامتی صحیح)(i) < ARRAY_سائز(cpu_deferred_آغاز)) &&               \
	 cpu_deferred_آغاز[i])
#ورنہ
#تعریف_کرو CPU_آغاز_DEFERRED(i) false
#اگر_ختم

خالی z_smp_init(خالی)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(خالی)atomic_clear(&cpu_آغاز_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	غیرعلامتی صحیح عدد_cpus = arch_عدد_cpus();

	برائے (صحیح i = 1; i < عدد_cpus; i++) {
		اگر (CPU_آغاز_DEFERRED(i)) {
			جاری;
		}
		z_init_cpu(i);
		آغاز_cpu(i, تہی);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(خالی)atomic_سیٹ(&cpu_آغاز_flag, 1);
}

bool z_smp_cpu_mobile(خالی)
{
	واپس !arch_is_اندر_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ ساخت k_thread *z_smp_current_get(خالی)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	غیرعلامتی صحیح کلید = arch_irq_تالا();
	ساخت k_thread *t = _raw_current;

	arch_irq_unlock(کلید);
	واپس t;
}
