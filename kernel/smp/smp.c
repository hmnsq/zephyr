/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/smp.h>
#ضمن <zephyr/devicetree.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/llext/symbol.h>
#ضمن <kswap.h>
#ضمن <kernel_internal.h>

ساكن atomic_t global_قلف;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
ساكن atomic_t cpu_پیل_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
ساكن atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
ساكن بنية cpu_پیل_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	فراغ *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_پیل_fn;

ساكن بنية k_spinlock cpu_پیل_قلف;

بینښه صحيح z_smp_global_قلف(فراغ)
{
	بینښه صحيح کلی = arch_irq_قلف();

	إذا (!_current->بنسټ.global_قلف_شمېر) {
		طالما (!atomic_cas(&global_قلف, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->بنسټ.global_قلف_شمېر++;

	أرجع کلی;
}
EXPORT_نښه(z_smp_global_قلف);

فراغ z_smp_global_unlock(بینښه صحيح کلی)
{
	إذا (_current->بنسټ.global_قلف_شمېر != 0U) {
		_current->بنسټ.global_قلف_شمېر--;

		إذا (!_current->بنسټ.global_قلف_شمېر) {
			(فراغ)atomic_clear(&global_قلف);
		}
	}

	arch_irq_unlock(کلی);
}
EXPORT_نښه(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
فراغ z_smp_release_global_قلف(بنية k_thread *thread)
{
	إذا (!thread->بنسټ.global_قلف_شمېر) {
		(فراغ)atomic_clear(&global_قلف);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
ساكن مضمن فراغ local_delay(فراغ)
{
	لكل (متطاير صحيح i = 0; i < 1000; i++) {
	}
}

ساكن فراغ wait_for_پیل_signal(atomic_t *پیل_flag)
{
	/* Wait for the signal to begin scheduling */
	طالما (!atomic_get(پیل_flag)) {
		local_delay();
	}
}

ساكن مضمن فراغ smp_init_سر(فراغ *arg)
{
	بنية cpu_پیل_cb csc = arg ? *(بنية cpu_پیل_cb *)arg : (بنية cpu_پیل_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(فراغ)atomic_سېټ(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_پیل_signal(&cpu_پیل_flag);

	إذا ((arg == عدم) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS
	إذا ((arg == عدم) || csc.reinit_timer) {
		smp_timer_init();
	}
#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	إذا (csc.fn != عدم) {
		csc.fn(csc.arg);
	}

	إذا ((arg != عدم) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		أرجع;
	}

	/* Let scheduler decide what thread to run next. */
	z_بدلون_unlocked();

	کوډ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساكن فراغ پیل_cpu(صحيح id, بنية cpu_پیل_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(فراغ)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_پیل(id, z_مداخله_stacks[id], CONFIG_ISR_ډېری_اندازه,
		       smp_init_سر, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	طالما (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

فراغ k_smp_cpu_پیل(صحيح id, smp_init_fn fn, فراغ *arg)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&cpu_پیل_قلف);

	cpu_پیل_fn.fn = fn;
	cpu_پیل_fn.arg = arg;
	cpu_پیل_fn.invoke_sched = true;

#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS
	cpu_پیل_fn.reinit_timer = true;
#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(فراغ)atomic_سېټ(&cpu_پیل_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	پیل_cpu(id, &cpu_پیل_fn);

	k_spin_unlock(&cpu_پیل_قلف, کلی);
}

فراغ k_smp_cpu_resume(صحيح id, smp_init_fn fn, فراغ *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&cpu_پیل_قلف);

	cpu_پیل_fn.fn = fn;
	cpu_پیل_fn.arg = arg;
	cpu_پیل_fn.invoke_sched = invoke_sched;

#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS
	cpu_پیل_fn.reinit_timer = reinit_timer;
#وإلا
	ARG_ناکارول_شوی(reinit_timer);
#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(فراغ)atomic_سېټ(&cpu_پیل_flag, 1);

	/* Start the CPU! */
	پیل_cpu(id, &cpu_پیل_fn);

	k_spin_unlock(&cpu_پیل_قلف, کلی);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#إذا DT_غوټه_EXISTS(DT_لار(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#تعریف_کړه CPU_DEFERRED_پیل_FLAG(غوټه_id) DT_PROP_OR(غوټه_id, zephyr_deferred_پیل, 0),
ساكن ثابت bool cpu_deferred_پیل[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_پیل_FLAG)
};
#تعریف_کړه CPU_پیل_DEFERRED(i)                                                  \
	(((بینښه صحيح)(i) < ARRAY_اندازه(cpu_deferred_پیل)) &&               \
	 cpu_deferred_پیل[i])
#وإلا
#تعریف_کړه CPU_پیل_DEFERRED(i) false
#د_که_پای

فراغ z_smp_init(فراغ)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(فراغ)atomic_clear(&cpu_پیل_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	بینښه صحيح عدد_cpus = arch_عدد_cpus();

	لكل (صحيح i = 1; i < عدد_cpus; i++) {
		إذا (CPU_پیل_DEFERRED(i)) {
			استمر;
		}
		z_init_cpu(i);
		پیل_cpu(i, عدم);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(فراغ)atomic_سېټ(&cpu_پیل_flag, 1);
}

bool z_smp_cpu_mobile(فراغ)
{
	أرجع !arch_is_دننه_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ بنية k_thread *z_smp_current_get(فراغ)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	بینښه صحيح کلی = arch_irq_قلف();
	بنية k_thread *t = _raw_current;

	arch_irq_unlock(کلی);
	أرجع t;
}
