/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/irq_offload.h>

/* Make offload_sem visible outside testing, in order to release
 * it outside when error happened.
 */
K_SEM_DEFINE(offload_sem, 1, 1);

शून्य irq_offload(irq_offload_routine_t routine, ध्रुव शून्य *parameter)
{
#यदि_परिभाषितम् CONFIG_IRQ_OFFLOAD_NESTED
	arch_irq_offload(routine, parameter);
#अन्यथा
	k_sem_take(&offload_sem, K_FOREVER);
	arch_irq_offload(routine, parameter);
	k_sem_give(&offload_sem);
#यदि_अन्तः /* CONFIG_IRQ_OFFLOAD_NESTED */
}
