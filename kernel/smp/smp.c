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

ساكن atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
ساكن atomic_t cpu_start_flag;

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
ساكن بنية cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	فراغ *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#endif /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

ساكن بنية k_spinlock cpu_start_lock;

unsigned صحيح z_smp_global_lock(فراغ)
{
	unsigned صحيح مفتاح = arch_irq_lock();

	إذا (!_current->base.global_lock_عدد) {
		طالما (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_عدد++;

	أرجع مفتاح;
}
EXPORT_رمز(z_smp_global_lock);

فراغ z_smp_global_unlock(unsigned صحيح مفتاح)
{
	إذا (_current->base.global_lock_عدد != 0U) {
		_current->base.global_lock_عدد--;

		إذا (!_current->base.global_lock_عدد) {
			(فراغ)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(مفتاح);
}
EXPORT_رمز(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
فراغ z_smp_release_global_lock(بنية k_thread *thread)
{
	إذا (!thread->base.global_lock_عدد) {
		(فراغ)atomic_clear(&global_lock);
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

ساكن فراغ wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	طالما (!atomic_get(start_flag)) {
		local_delay();
	}
}

ساكن مضمن فراغ smp_init_قمة(فراغ *arg)
{
	بنية cpu_start_cb csc = arg ? *(بنية cpu_start_cb *)arg : (بنية cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(فراغ)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	إذا ((arg == عدم) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	إذا ((arg == عدم) || csc.reinit_timer) {
		smp_timer_init();
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	إذا (csc.fn != عدم) {
		csc.fn(csc.arg);
	}

	إذا ((arg != عدم) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		أرجع;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ساكن فراغ start_cpu(صحيح id, بنية cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(فراغ)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_مكدس_SIZE,
		       smp_init_قمة, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	طالما (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

فراغ k_smp_cpu_start(صحيح id, smp_init_fn fn, فراغ *arg)
{
	k_spinlock_مفتاح_t مفتاح = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(فراغ)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, مفتاح);
}

فراغ k_smp_cpu_resume(صحيح id, smp_init_fn fn, فراغ *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_مفتاح_t مفتاح = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#ifdef CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#وإلا
	ARG_UNUSED(reinit_timer);
#endif /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(فراغ)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, مفتاح);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#إذا DT_عقدة_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#define CPU_DEFERRED_START_FLAG(عقدة_id) DT_PROP_OR(عقدة_id, zephyr_deferred_start, 0),
ساكن ثابت bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#define CPU_START_DEFERRED(i)                                                  \
	(((unsigned صحيح)(i) < ARRAY_SIZE(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#وإلا
#define CPU_START_DEFERRED(i) false
#endif

فراغ z_smp_init(فراغ)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(فراغ)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	unsigned صحيح رقم_cpus = arch_رقم_cpus();

	لكل (صحيح i = 1; i < رقم_cpus; i++) {
		إذا (CPU_START_DEFERRED(i)) {
			استمر;
		}
		z_init_cpu(i);
		start_cpu(i, عدم);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(فراغ)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(فراغ)
{
	أرجع !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ بنية k_thread *z_smp_current_get(فراغ)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	unsigned صحيح مفتاح = arch_irq_lock();
	بنية k_thread *t = _raw_current;

	arch_irq_unlock(مفتاح);
	أرجع t;
}
