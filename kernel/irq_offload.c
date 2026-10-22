/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/kernel.h>
#شامل <zephyr/irq_offload.h>

/* Make offload_sem visible outside testing, in order to release
 * it outside when error happened.
 */
K_SEM_DEFINE(offload_sem, 1, 1);

تهی irq_offload(irq_offload_routine_t routine, ثابت تهی *parameter)
{
#ifdef CONFIG_IRQ_OFFLOAD_NESTED
	arch_irq_offload(routine, parameter);
#وگرنه
	k_sem_take(&offload_sem, K_FOREVER);
	arch_irq_offload(routine, parameter);
	k_sem_give(&offload_sem);
#endif /* CONFIG_IRQ_OFFLOAD_NESTED */
}
