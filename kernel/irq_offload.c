/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/irq_offload.h>

/* Make offload_sem visible outside testing, in order to release
 * it outside when error happened.
 */
K_SEM_DEFINE(offload_sem, 1, 1);

ಶೂನ್ಯ irq_offload(irq_offload_routine_t routine, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *parameter)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_IRQ_OFFLOAD_NESTED
	arch_irq_offload(routine, parameter);
#ಇಲ್ಲದಿದ್ದರೆ
	k_sem_take(&offload_sem, K_FOREVER);
	arch_irq_offload(routine, parameter);
	k_sem_give(&offload_sem);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_IRQ_OFFLOAD_NESTED */
}
