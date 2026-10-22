/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/irq_offload.h>

/* Make offload_sem visible outside testing, in order to release
 * it outside when error happened.
 */
K_SEM_DEFINE(offload_sem, 1, 1);

శూన్య irq_offload(irq_offload_routine_t routine, స్థిరరాశి శూన్య *parameter)
{
#నిర్వచితమైతే CONFIG_IRQ_OFFLOAD_NESTED
	arch_irq_offload(routine, parameter);
#లేకపోతే
	k_sem_take(&offload_sem, K_FOREVER);
	arch_irq_offload(routine, parameter);
	k_sem_give(&offload_sem);
#అయితే_ముగింపు /* CONFIG_IRQ_OFFLOAD_NESTED */
}
