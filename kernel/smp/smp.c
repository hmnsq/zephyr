/* Copyright (c) 2022 Intel corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/kernel/smp.h>
#समावेश <zephyr/devicetree.h>
#समावेश <zephyr/spinlock.h>
#समावेश <zephyr/llext/symbol.h>
#समावेश <kswap.h>
#समावेश <kernel_internal.h>

अटल atomic_t global_lock;

/**
 * Flag to tell recently powered up CPU to start
 * initialization routine.
 *
 * 0 to tell powered up CPU to wait.
 * 1 to tell powered up CPU to continue initialization.
 */
अटल atomic_t cpu_start_flag;

/**
 * Flag to tell caller that the target CPU is now
 * powered up and ready to be initialized.
 *
 * 0 if target CPU is not yet ready.
 * 1 if target CPU has powered up and ready to be initialized.
 */
अटल atomic_t ready_flag;

/**
 * Struct holding the function to be called before handing off
 * to schedule and its argument.
 */
अटल काष्ठा cpu_start_cb {
	/**
	 * Function to be called before handing off to scheduler.
	 * Can be NULL.
	 */
	smp_init_fn fn;

	/** Argument to @ref cpu_start_cb.fn. */
	व्योम *arg;

	/** Invoke scheduler after CPU has started if true. */
	bool invoke_sched;

#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS
	/** True if smp_timer_init() needs to be called. */
	bool reinit_timer;
#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */
} cpu_start_fn;

अटल काष्ठा k_spinlock cpu_start_lock;

अचिन्हित पूर्णांक z_smp_global_lock(व्योम)
{
	अचिन्हित पूर्णांक कुंजी = arch_irq_lock();

	अगर (!_current->base.global_lock_गिनती) {
		जबतक (!atomic_cas(&global_lock, 0, 1)) {
			arch_spin_relax();
		}
	}

	_current->base.global_lock_गिनती++;

	वापस कुंजी;
}
EXPORT_प्रतीक(z_smp_global_lock);

व्योम z_smp_global_unlock(अचिन्हित पूर्णांक कुंजी)
{
	अगर (_current->base.global_lock_गिनती != 0U) {
		_current->base.global_lock_गिनती--;

		अगर (!_current->base.global_lock_गिनती) {
			(व्योम)atomic_clear(&global_lock);
		}
	}

	arch_irq_unlock(कुंजी);
}
EXPORT_प्रतीक(z_smp_global_unlock);

/* Called from within z_swap(), so assumes lock already held */
व्योम z_smp_release_global_lock(काष्ठा k_thread *thread)
{
	अगर (!thread->base.global_lock_गिनती) {
		(व्योम)atomic_clear(&global_lock);
	}
}

/* Tiny delay that relaxes bus traffic to avoid spamming a shared
 * memory bus looking at an atomic variable
 */
अटल अंतरभूत व्योम local_delay(व्योम)
{
	क्रम (अस्थिर पूर्णांक i = 0; i < 1000; i++) {
	}
}

अटल व्योम wait_for_start_signal(atomic_t *start_flag)
{
	/* Wait for the signal to begin scheduling */
	जबतक (!atomic_get(start_flag)) {
		local_delay();
	}
}

अटल अंतरभूत व्योम smp_init_शीर्ष(व्योम *arg)
{
	काष्ठा cpu_start_cb csc = arg ? *(काष्ठा cpu_start_cb *)arg : (काष्ठा cpu_start_cb){0};

	/* Let start_cpu() know that this CPU has powered up. */
	(व्योम)atomic_set(&ready_flag, 1);

	/* Wait for the CPU start caller to signal that
	 * we can start initialization.
	 */
	wait_for_start_signal(&cpu_start_flag);

	अगर ((arg == शून्य) || csc.invoke_sched) {
		/* Initialize the dummy thread struct so that
		 * the scheduler can schedule actual threads to run.
		 */
		z_dummy_thread_init(&_thread_dummy);
	}

#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS
	अगर ((arg == शून्य) || csc.reinit_timer) {
		smp_timer_init();
	}
#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */

	/* Do additional initialization steps if needed. */
	अगर (csc.fn != शून्य) {
		csc.fn(csc.arg);
	}

	अगर ((arg != शून्य) && !csc.invoke_sched) {
		/* Don't invoke scheduler. */
		वापस;
	}

	/* Let scheduler decide what thread to run next. */
	z_swap_unlocked();

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

अटल व्योम start_cpu(पूर्णांक id, काष्ठा cpu_start_cb *csc)
{
	/* Clear the ready flag so the newly powered up CPU can
	 * signal that it has powered up.
	 */
	(व्योम)atomic_clear(&ready_flag);

	/* Power up the CPU */
	arch_cpu_start(id, z_interrupt_stacks[id], CONFIG_ISR_ढेर_आकार,
		       smp_init_शीर्ष, csc);

	/* Wait until the newly powered up CPU to signal that
	 * it has powered up.
	 */
	जबतक (!atomic_get(&ready_flag)) {
		local_delay();
	}
}

व्योम k_smp_cpu_start(पूर्णांक id, smp_init_fn fn, व्योम *arg)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = true;

#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = true;
#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(व्योम)atomic_set(&cpu_start_flag, 1); /* async, don't care */

	/* Initialize various CPU structs related to this CPU. */
	z_init_cpu(id);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, कुंजी);
}

व्योम k_smp_cpu_resume(पूर्णांक id, smp_init_fn fn, व्योम *arg,
		      bool reinit_timer, bool invoke_sched)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&cpu_start_lock);

	cpu_start_fn.fn = fn;
	cpu_start_fn.arg = arg;
	cpu_start_fn.invoke_sched = invoke_sched;

#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS
	cpu_start_fn.reinit_timer = reinit_timer;
#अन्यथा
	ARG_UNUSED(reinit_timer);
#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */

	/* We are only starting one CPU so we do not need to synchronize
	 * across all CPUs using the start_flag. So just set it to 1.
	 */
	(व्योम)atomic_set(&cpu_start_flag, 1);

	/* Start the CPU! */
	start_cpu(id, &cpu_start_fn);

	k_spin_unlock(&cpu_start_lock, कुंजी);
}

/* Per-CPU boot deferral flags from the devicetree, indexed by logical CPU id
 * (the order of the "cpu" children of /cpus). A CPU marked with
 * zephyr,deferred-start is not started during kernel boot; it is brought up
 * at run time with k_smp_cpu_start() or k_smp_cpu_resume().
 */
#अगर DT_गाँठ_EXISTS(DT_PATH(cpus))
/* DT_PROP_OR: a cpu node without a binding covering the property simply
 * cannot be deferred, rather than failing to compile.
 */
#घोषणा CPU_DEFERRED_START_FLAG(गाँठ_id) DT_PROP_OR(गाँठ_id, zephyr_deferred_start, 0),
अटल स्थिर bool cpu_deferred_start[] = {
	DT_FOREACH_CPU(CPU_DEFERRED_START_FLAG)
};
#घोषणा CPU_START_DEFERRED(i)                                                  \
	(((अचिन्हित पूर्णांक)(i) < ARRAY_आकार(cpu_deferred_start)) &&               \
	 cpu_deferred_start[i])
#अन्यथा
#घोषणा CPU_START_DEFERRED(i) false
#पूर्ण_अगर

व्योम z_smp_init(व्योम)
{
	/* We are powering up all CPUs and we want to synchronize their
	 * entry into scheduler. So set the start flag to 0 here.
	 */
	(व्योम)atomic_clear(&cpu_start_flag);

	/* Just start CPUs one by one, skipping those the devicetree defers
	 * to a later, run-time start.
	 */
	अचिन्हित पूर्णांक अंक_cpus = arch_अंक_cpus();

	क्रम (पूर्णांक i = 1; i < अंक_cpus; i++) {
		अगर (CPU_START_DEFERRED(i)) {
			जारी;
		}
		z_init_cpu(i);
		start_cpu(i, शून्य);
	}

	/* Let loose those CPUs so they can start scheduling
	 * threads to run.
	 */
	(व्योम)atomic_set(&cpu_start_flag, 1);
}

bool z_smp_cpu_mobile(व्योम)
{
	वापस !arch_is_in_isr() && arch_cpu_irqs_are_enabled();
}

__attribute_const__ काष्ठा k_thread *z_smp_current_get(व्योम)
{
	/*
	 * _raw_current can race with preemption before it is read.  We
	 * must lock local interrupts when reading it.
	 */
	अचिन्हित पूर्णांक कुंजी = arch_irq_lock();
	काष्ठा k_thread *t = _raw_current;

	arch_irq_unlock(कुंजी);
	वापस t;
}
