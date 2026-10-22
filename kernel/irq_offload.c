/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/irq_offload.h>

/* Make offload_sem visible outside testing, in order to release
 * it outside when error happened.
 */
K_SEM_DEFINE(offload_sem, 1, 1);

ਖਾਲੀ irq_offload(irq_offload_routine_t routine, ਅਚਲ ਖਾਲੀ *parameter)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_IRQ_OFFLOAD_NESTED
	arch_irq_offload(routine, parameter);
#ਨਹੀਂਤਾਂ
	k_sem_take(&offload_sem, K_FOREVER);
	arch_irq_offload(routine, parameter);
	k_sem_give(&offload_sem);
#ਜੇ_ਅੰਤ /* CONFIG_IRQ_OFFLOAD_NESTED */
}
