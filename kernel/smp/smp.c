/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/kernel/smp.h>
#அந்தர்பாவ <zephyr/devicetree.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <zephyr/llext/symbol.h>
#அந்தர்பாவ <kswap.h>
#அந்தர்பாவ <kernel_internal.h>

நிலையான atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
நிலையான atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
நிலையான atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
நிலையான கட்டமைப்பு cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	வெறுமை *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

நிலையான கட்டமைப்பு k_spinlock cpu_start_lock;

unsigned முழு z_smp_global_lock(வெறுமை)
{
	unsigned முழு சாவி = arch_irq_lock();

	எனில் (!_current->base.global_lock_மொத்தம்) {
		போது (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_மொத்தம்++;

	திரும்பு சாவி;
}
EXPORT_குறியீடு(z_smp_global_lock);

வெறுமை z_smp_global_unlock(unsigned முழு சாவி)
{
	எனில் (_current->base.global_lock_மொத்தம் != 0U) {
		_current->base.global_lock_மொத்தம்--;

		எனில் (!_current->base.global_lock_மொத்தம்) {
			(வெறுமை)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(சாவி);
}
EXPORT_குறியீடு(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
வெறுமை z_smp_release_global_lock(கட்டமைப்பு k_thread *thread)
{
	எனில் (!thread->base.global_lock_மொத்தம்) {
		(வெறுமை)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
நிலையான உள்வரி வெறுமை local_delay(வெறுமை)
{
	க்கு (நிலையற்ற முழு i = 0; i < 1000; i++) {
	}
}

நிலையான வெறுமை wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	போது (!atomic_get(start_flag)) {
		local_delay();
	}
}

நிலையான உள்வரி வெறுமை smp_init_உச்சி(வெறுமை *arg)
{
	கட்டமைப்பு cpu_start_cb csc = arg ? *(கட்டமைப்பு cpu_start_cb *)arg : (கட்டமைப்பு cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(வெறுமை)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	எனில் ((arg == வெற்று) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	எனில் ((arg == வெற்று) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	எனில் (csc.fn != வெற்று) {
		csc.fn(csc.arg);
	}

	எனில் ((arg != வெற்று) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		திரும்பு;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

நிலையான வெறுமை start_cpu(முழு id, கட்டமைப்பு cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(வெறுமை)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_அடுக்கு_SIZE,
		       smp_init_உச்சி, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	போது (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

வெறுமை k_smp_cpu_start(முழு id, smp_init_fn fn, வெறுமை *arg)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(வெறுமை)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, சாவி);
}

வெறுமை k_smp_cpu_resume(முழு id, smp_init_fn fn, வெறுமை *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#இல்லையெனில்
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(வெறுமை)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, சாவி);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#எனில் DT_முனை_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(முனை_id) DT_PROP_OR(முனை_id, zephyr_deferred_start, 0),
நிலையான மாறிலி bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned முழு)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#இல்லையெனில்
#define CPU_START_DEFERRED(i) false
#endif

வெறுமை z_smp_init(வெறுமை)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(வெறுமை)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned முழு இலக்கம்_cpus = arch_இலக்கம்_cpus();

	க்கு (முழு i = 1; i < இலக்கம்_cpus; i++) {
		எனில் (CPU_START_DEFERRED(i)) {
			தொடர்;
		}
		z_init_cpu(i);
		start_cpu(i, வெற்று);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(வெறுமை)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(வெறுமை)
{
	திரும்பு !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ கட்டமைப்பு k_thread *z_smp_current_get(வெறுமை)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned முழு சாவி = arch_irq_lock();
	கட்டமைப்பு k_thread *t = _raw_current;

	arch_irq_unlock(சாவி);
	திரும்பு t;
}
