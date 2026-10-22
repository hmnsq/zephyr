/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <kernel_internal.h>
#شامل <zephyr/sys/__assert.h>
#شامل <stdbool.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/sys/check.h>
#شامل <zephyr/sys/libc-hooks.h>
#شامل <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ساخت k_spinlock z_mem_domain_تالا;
ساکن uint8_t max_partitions;

ساخت k_mem_domain k_mem_domain_default;

ساکن bool check_جوڑو_تقسیم(ساخت k_mem_domain *domain,
				ساخت k_mem_تقسیم *part)
{

	صحیح i;
	uintptr_t pstart, pend, dstart, dend;

	اگر (part == تہی) {
		LOG_ERR("NULL k_mem_partition provided");
		واپس false;
	}

#اگر_تعریف_شدہ CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	اگر (K_MEM_تقسیم_IS_EXECUTABLE(part->attr) &&
	    K_MEM_تقسیم_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->آغاز);
		واپس false;
	}
#اگر_ختم /* CONFIG_EXECUTE_XOR_WRITE */

	اگر (part->سائز == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->آغاز);
		واپس false;
	}

	pstart = part->آغاز;
	pend = part->آغاز + part->سائز;

	اگر (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->آغاز, part->سائز);
		واپس false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	برائے (i = 0; i < domain->عدد_partitions; i++) {
		ساخت k_mem_تقسیم *dpart = &domain->partitions[i];

		اگر (dpart->سائز == 0U) {
			/* Unused slot */
			جاری;
		}

		dstart = dpart->آغاز;
		dend = dstart + dpart->سائز;

		اگر (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->آغاز, part->سائز,
				dpart->آغاز, dpart->سائز);
			واپس false;
		}
	}

	واپس true;
}

صحیح k_mem_domain_init(ساخت k_mem_domain *domain, uint8_t عدد_parts,
		      ساخت k_mem_تقسیم *parts[])
{
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF(domain == تہی) {
		ret = -EINVAL;
		جاؤ برآمد;
	}

	CHECKIF(!(عدد_parts == 0U || parts != تہی)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		جاؤ برآمد;
	}

	CHECKIF(!(عدد_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			عدد_parts, max_partitions);
		ret = -EINVAL;
		جاؤ برآمد;
	}

	کلید = k_spin_تالا(&z_mem_domain_تالا);

	domain->عدد_partitions = 0U;
	(خالی)memset(domain->partitions, 0, حجم(domain->partitions));

#اگر_تعریف_شدہ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#اگر_ختم /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#اگر_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_ڈیٹا
	ret = arch_mem_domain_init(domain);

	اگر (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		جاؤ unlock_برآمد;
	}
#اگر_ختم /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	اگر (عدد_parts != 0U) {
		uint32_t i;

		برائے (i = 0U; i < عدد_parts; i++) {
			CHECKIF(!check_جوڑو_تقسیم(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				جاؤ unlock_برآمد;
			}

			domain->partitions[i] = *parts[i];
			domain->عدد_partitions++;
#اگر_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			صحیح ret2 = arch_mem_domain_تقسیم_جوڑو(domain, i);

			ARG_غیر_مستعمل(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#اگر_ختم /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_برآمد:
	k_spin_unlock(&z_mem_domain_تالا, کلید);

برآمد:
	واپس ret;
}

صحیح k_mem_domain_deinit(ساخت k_mem_domain *domain)
{
#اگر defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF(domain == تہی) {
		ret = -EINVAL;
		جاؤ برآمد;
	}

	اگر (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		جاؤ برآمد;
	}

	کلید = k_spin_تالا(&z_mem_domain_تالا);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	اگر (!sys_dlist_is_خالی_خانے(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		جاؤ unlock_برآمد;
	}

	ret = arch_mem_domain_deinit(domain);
	اگر (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		جاؤ unlock_برآمد;
	}

unlock_برآمد:
	k_spin_unlock(&z_mem_domain_تالا, کلید);

برآمد:
	واپس ret;
#ورنہ  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_غیر_مستعمل(domain);
	واپس -ENOTSUP;
#اگر_ختم /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

صحیح k_mem_domain_جوڑو_تقسیم(ساخت k_mem_domain *domain,
			       ساخت k_mem_تقسیم *part)
{
	صحیح p_idx;
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF(domain == تہی) {
		ret = -EINVAL;
		جاؤ برآمد;
	}

	CHECKIF(!check_جوڑو_تقسیم(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		جاؤ برآمد;
	}

	کلید = k_spin_تالا(&z_mem_domain_تالا);

	برائے (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		اگر (domain->partitions[p_idx].سائز == 0U) {
			توڑو;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		جاؤ unlock_برآمد;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->آغاز, part->سائز, domain);

	domain->partitions[p_idx].آغاز = part->آغاز;
	domain->partitions[p_idx].سائز = part->سائز;
	domain->partitions[p_idx].attr = part->attr;

	domain->عدد_partitions++;

#اگر_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_تقسیم_جوڑو(domain, p_idx);
#اگر_ختم /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_برآمد:
	k_spin_unlock(&z_mem_domain_تالا, کلید);

برآمد:
	واپس ret;
}

صحیح k_mem_domain_remove_تقسیم(ساخت k_mem_domain *domain,
				  ساخت k_mem_تقسیم *part)
{
	صحیح p_idx;
	k_spinlock_کلید_t کلید;
	صحیح ret = 0;

	CHECKIF((domain == تہی) || (part == تہی)) {
		ret = -EINVAL;
		جاؤ برآمد;
	}

	کلید = k_spin_تالا(&z_mem_domain_تالا);

	/* find a partition that matches the given start and size */
	برائے (p_idx = 0; p_idx < max_partitions; p_idx++) {
		اگر ((domain->partitions[p_idx].آغاز == part->آغاز) &&
		    (domain->partitions[p_idx].سائز == part->سائز)) {
			توڑو;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		جاؤ unlock_برآمد;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->آغاز, part->سائز, domain);

#اگر_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_تقسیم_remove(domain, p_idx);
#اگر_ختم /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].سائز = 0U;

	domain->عدد_partitions--;

unlock_برآمد:
	k_spin_unlock(&z_mem_domain_تالا, کلید);

برآمد:
	واپس ret;
}

ساکن صحیح جوڑو_thread_locked(ساخت k_mem_domain *domain,
			     k_tid_t thread)
{
	صحیح ret = 0;

	__ASSERT_NO_MSG(domain != تہی);
	__ASSERT_NO_MSG(thread != تہی);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#اگر_تعریف_شدہ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_گرہ);
#اگر_ختم /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#اگر_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_جوڑو(thread);
#اگر_ختم /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	واپس ret;
}

ساکن صحیح remove_thread_locked(ساخت k_thread *thread)
{
	صحیح ret = 0;

	__ASSERT_NO_MSG(thread != تہی);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#اگر_تعریف_شدہ CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_گرہ);
#اگر_ختم /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#اگر_تعریف_شدہ CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#اگر_ختم /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	واپس ret;
}

/* Called from thread object initialization */
خالی z_mem_domain_init_thread(ساخت k_thread *thread)
{
	صحیح ret;
	k_spinlock_کلید_t کلید = k_spin_تالا(&z_mem_domain_تالا);

	/* New threads inherit memory domain configuration from parent */
	ret = جوڑو_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_غیر_مستعمل(ret);

	k_spin_unlock(&z_mem_domain_تالا, کلید);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
خالی z_mem_domain_exit_thread(ساخت k_thread *thread)
{
	صحیح ret;

	k_spinlock_کلید_t کلید = k_spin_تالا(&z_mem_domain_تالا);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_غیر_مستعمل(ret);

	k_spin_unlock(&z_mem_domain_تالا, کلید);
}

صحیح k_mem_domain_جوڑو_thread(ساخت k_mem_domain *domain, k_tid_t thread)
{
	صحیح ret = 0;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_تالا(&z_mem_domain_تالا);
	اگر (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		اگر (ret == 0) {
			ret = جوڑو_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_تالا, کلید);

	واپس ret;
}

ساکن خالی init_mem_domain_module(خالی)
{
	صحیح ret;

	ARG_غیر_مستعمل(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, تہی);
	__ASSERT(ret == 0, "failed to init default mem domain");

#اگر_تعریف_شدہ Z_LIBC_تقسیم_EXISTS
	ret = k_mem_domain_جوڑو_تقسیم(&k_mem_domain_default,
					 &z_libc_تقسیم);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#اگر_ختم /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
