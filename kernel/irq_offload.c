/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/irq_offload.h>

/* Make offload_sem visible outside testing, in order to release
 * it outside when error happened.
 */
K_SEM_DEFINE(offload_sem, 1, 1);

व्योम irq_offload(irq_offload_routine_t routine, स्थिर व्योम *parameter)
{
#अगर_घोषित CONFIG_IRQ_OFFLOAD_NESTED
	arch_irq_offload(routine, parameter);
#अन्यथा
	k_sem_take(&offload_sem, K_FOREVER);
	arch_irq_offload(routine, parameter);
	k_sem_give(&offload_sem);
#पूर्ण_अगर /* CONFIG_IRQ_OFFLOAD_NESTED */
}
