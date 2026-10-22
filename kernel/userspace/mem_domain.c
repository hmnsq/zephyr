/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <kernel_internal.h>
#समावेश <zephyr/sys/__assert.h>
#समावेश <stdbool.h>
#समावेश <zephyr/spinlock.h>
#समावेश <zephyr/sys/check.h>
#समावेश <zephyr/sys/libc-hooks.h>
#समावेश <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

काष्ठा k_spinlock z_mem_domain_lock;
अटल uint8_t max_partitions;

काष्ठा k_mem_domain k_mem_domain_default;

अटल bool check_add_partition(काष्ठा k_mem_domain *domain,
				काष्ठा k_mem_partition *part)
{

	पूर्णांक i;
	uintptr_t pstart, pend, dstart, dend;

	अगर (part == शून्य) {
		LOG_ERR("NULL k_mem_partition provided");
		वापस false;
	}

#अगर_घोषित CONFIG_EXECUTE_XOR_WRITE
	/* Arches where execution cannot be disabled should always return
	 * false to this check
	 */
	अगर (K_MEM_PARTITION_IS_EXECUTABLE(part->attr) &&
	    K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
		LOG_ERR("partition is writable and executable <start %lx>",
			part->start);
		वापस false;
	}
#पूर्ण_अगर /* CONFIG_EXECUTE_XOR_WRITE */

	अगर (part->आकार == 0U) {
		LOG_ERR("zero sized partition at %p with base 0x%lx",
			part, part->start);
		वापस false;
	}

	pstart = part->start;
	pend = part->start + part->आकार;

	अगर (pend <= pstart) {
		LOG_ERR("invalid partition %p, wraparound detected. base 0x%lx size %zu",
			part, part->start, part->आकार);
		वापस false;
	}

	/* Check that this partition doesn't overlap any existing ones already
	 * in the domain
	 */
	क्रम (i = 0; i < domain->अंक_partitions; i++) {
		काष्ठा k_mem_partition *dpart = &domain->partitions[i];

		अगर (dpart->आकार == 0U) {
			/* Unused slot */
			जारी;
		}

		dstart = dpart->start;
		dend = dstart + dpart->आकार;

		अगर (pend > dstart && dend > pstart) {
			LOG_ERR("partition %p base %lx (size %zu) overlaps existing base %lx (size %zu)",
				part, part->start, part->आकार,
				dpart->start, dpart->आकार);
			वापस false;
		}
	}

	वापस true;
}

पूर्णांक k_mem_domain_init(काष्ठा k_mem_domain *domain, uint8_t अंक_parts,
		      काष्ठा k_mem_partition *parts[])
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक ret = 0;

	CHECKIF(domain == शून्य) {
		ret = -EINVAL;
		जाओ out;
	}

	CHECKIF(!(अंक_parts == 0U || parts != शून्य)) {
		LOG_ERR("parts array is NULL and num_parts is nonzero");
		ret = -EINVAL;
		जाओ out;
	}

	CHECKIF(!(अंक_parts <= max_partitions)) {
		LOG_ERR("num_parts of %d exceeds maximum allowable partitions (%d)",
			अंक_parts, max_partitions);
		ret = -EINVAL;
		जाओ out;
	}

	कुंजी = k_spin_lock(&z_mem_domain_lock);

	domain->अंक_partitions = 0U;
	(व्योम)स_रखो(domain->partitions, 0, माप(domain->partitions));

#अगर_घोषित CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_init(&domain->thread_mem_domain_list);
#पूर्ण_अगर /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#अगर_घोषित CONFIG_ARCH_MEM_DOMAIN_तथ्य
	ret = arch_mem_domain_init(domain);

	अगर (ret != 0) {
		LOG_ERR("architecture-specific initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		जाओ unlock_out;
	}
#पूर्ण_अगर /* CONFIG_ARCH_MEM_DOMAIN_DATA */
	अगर (अंक_parts != 0U) {
		uint32_t i;

		क्रम (i = 0U; i < अंक_parts; i++) {
			CHECKIF(!check_add_partition(domain, parts[i])) {
				LOG_ERR("invalid partition index %d (%p)",
					i, parts[i]);
				ret = -EINVAL;
				जाओ unlock_out;
			}

			domain->partitions[i] = *parts[i];
			domain->अंक_partitions++;
#अगर_घोषित CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
			पूर्णांक ret2 = arch_mem_domain_partition_add(domain, i);

			ARG_UNUSED(ret2);
			CHECKIF(ret2 != 0) {
				ret = ret2;
			}
#पूर्ण_अगर /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */
		}
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, कुंजी);

out:
	वापस ret;
}

पूर्णांक k_mem_domain_deinit(काष्ठा k_mem_domain *domain)
{
#अगर defined(CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT)
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक ret = 0;

	CHECKIF(domain == शून्य) {
		ret = -EINVAL;
		जाओ out;
	}

	अगर (domain == &k_mem_domain_default) {
		/* Default memory domain must be there forever. */
		ret = -EINVAL;
		जाओ out;
	}

	कुंजी = k_spin_lock(&z_mem_domain_lock);

	/* Must make sure there are no threads associated with this memory
	 * domain anymore. Or else these threads will run with an invalid
	 * memory domain.
	 */
	अगर (!sys_dlist_is_empty(&domain->thread_mem_domain_list)) {
		ret = -EBUSY;
		जाओ unlock_out;
	}

	ret = arch_mem_domain_deinit(domain);
	अगर (ret != 0) {
		LOG_ERR("architecture-specific de-initialization failed for domain %p with %d",
			domain, ret);
		ret = -ENOMEM;
		जाओ unlock_out;
	}

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, कुंजी);

out:
	वापस ret;
#अन्यथा  /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
	ARG_UNUSED(domain);
	वापस -ENOTSUP;
#पूर्ण_अगर /* CONFIG_ARCH_MEM_DOMAIN_SUPPORTS_DEINIT */
}

पूर्णांक k_mem_domain_add_partition(काष्ठा k_mem_domain *domain,
			       काष्ठा k_mem_partition *part)
{
	पूर्णांक p_idx;
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक ret = 0;

	CHECKIF(domain == शून्य) {
		ret = -EINVAL;
		जाओ out;
	}

	CHECKIF(!check_add_partition(domain, part)) {
		LOG_ERR("invalid partition %p", part);
		ret = -EINVAL;
		जाओ out;
	}

	कुंजी = k_spin_lock(&z_mem_domain_lock);

	क्रम (p_idx = 0; p_idx < max_partitions; p_idx++) {
		/* A zero-sized partition denotes it's a free partition */
		अगर (domain->partitions[p_idx].आकार == 0U) {
			अवरोध;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no free partition slots available");
		ret = -ENOSPC;
		जाओ unlock_out;
	}

	LOG_DBG("add partition base %lx size %zu to domain %p",
		part->start, part->आकार, domain);

	domain->partitions[p_idx].start = part->start;
	domain->partitions[p_idx].आकार = part->आकार;
	domain->partitions[p_idx].attr = part->attr;

	domain->अंक_partitions++;

#अगर_घोषित CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_add(domain, p_idx);
#पूर्ण_अगर /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, कुंजी);

out:
	वापस ret;
}

पूर्णांक k_mem_domain_remove_partition(काष्ठा k_mem_domain *domain,
				  काष्ठा k_mem_partition *part)
{
	पूर्णांक p_idx;
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक ret = 0;

	CHECKIF((domain == शून्य) || (part == शून्य)) {
		ret = -EINVAL;
		जाओ out;
	}

	कुंजी = k_spin_lock(&z_mem_domain_lock);

	/* find a partition that matches the given start and size */
	क्रम (p_idx = 0; p_idx < max_partitions; p_idx++) {
		अगर ((domain->partitions[p_idx].start == part->start) &&
		    (domain->partitions[p_idx].आकार == part->आकार)) {
			अवरोध;
		}
	}

	CHECKIF(!(p_idx < max_partitions)) {
		LOG_ERR("no matching partition found");
		ret = -ENOENT;
		जाओ unlock_out;
	}

	LOG_DBG("remove partition base %lx size %zu from domain %p",
		part->start, part->आकार, domain);

#अगर_घोषित CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_partition_remove(domain, p_idx);
#पूर्ण_अगर /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	/* A zero-sized partition denotes it's a free partition */
	domain->partitions[p_idx].आकार = 0U;

	domain->अंक_partitions--;

unlock_out:
	k_spin_unlock(&z_mem_domain_lock, कुंजी);

out:
	वापस ret;
}

अटल पूर्णांक add_thread_locked(काष्ठा k_mem_domain *domain,
			     k_tid_t thread)
{
	पूर्णांक ret = 0;

	__ASSERT_NO_MSG(domain != शून्य);
	__ASSERT_NO_MSG(thread != शून्य);

	LOG_DBG("add thread %p to domain %p", thread, domain);

#अगर_घोषित CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_append(&domain->thread_mem_domain_list,
			 &thread->mem_domain_info.thread_mem_domain_गाँठ);
#पूर्ण_अगर /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

	thread->mem_domain_info.mem_domain = domain;

#अगर_घोषित CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_add(thread);
#पूर्ण_अगर /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	वापस ret;
}

अटल पूर्णांक remove_thread_locked(काष्ठा k_thread *thread)
{
	पूर्णांक ret = 0;

	__ASSERT_NO_MSG(thread != शून्य);
	LOG_DBG("remove thread %p from memory domain %p",
		thread, thread->mem_domain_info.mem_domain);

#अगर_घोषित CONFIG_MEM_DOMAIN_HAS_THREAD_LIST
	sys_dlist_remove(&thread->mem_domain_info.thread_mem_domain_गाँठ);
#पूर्ण_अगर /* CONFIG_MEM_DOMAIN_HAS_THREAD_LIST */

#अगर_घोषित CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API
	ret = arch_mem_domain_thread_remove(thread);
#पूर्ण_अगर /* CONFIG_ARCH_MEM_DOMAIN_SYNCHRONOUS_API */

	वापस ret;
}

/* Called from thread object initialization */
व्योम z_mem_domain_init_thread(काष्ठा k_thread *thread)
{
	पूर्णांक ret;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&z_mem_domain_lock);

	/* New threads inherit memory domain configuration from parent */
	ret = add_thread_locked(_current->mem_domain_info.mem_domain, thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, कुंजी);
}

/* Called when thread aborts during teardown tasks. The scheduler's spinlock is held */
व्योम z_mem_domain_exit_thread(काष्ठा k_thread *thread)
{
	पूर्णांक ret;

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&z_mem_domain_lock);

	ret = remove_thread_locked(thread);
	__ASSERT_NO_MSG(ret == 0);
	ARG_UNUSED(ret);

	k_spin_unlock(&z_mem_domain_lock, कुंजी);
}

पूर्णांक k_mem_domain_add_thread(काष्ठा k_mem_domain *domain, k_tid_t thread)
{
	पूर्णांक ret = 0;
	k_spinlock_कुंजी_t कुंजी;

	कुंजी = k_spin_lock(&z_mem_domain_lock);
	अगर (thread->mem_domain_info.mem_domain != domain) {
		ret = remove_thread_locked(thread);

		अगर (ret == 0) {
			ret = add_thread_locked(domain, thread);
		}
	}
	k_spin_unlock(&z_mem_domain_lock, कुंजी);

	वापस ret;
}

अटल व्योम init_mem_domain_module(व्योम)
{
	पूर्णांक ret;

	ARG_UNUSED(ret);

	max_partitions = arch_mem_domain_max_partitions_get();
	/*
	 * max_partitions must be less than or equal to
	 * CONFIG_MAX_DOMAIN_PARTITIONS, or would encounter array index
	 * out of bounds error.
	 */
	__ASSERT(max_partitions <= CONFIG_MAX_DOMAIN_PARTITIONS, "");

	ret = k_mem_domain_init(&k_mem_domain_default, 0, शून्य);
	__ASSERT(ret == 0, "failed to init default mem domain");

#अगर_घोषित Z_LIBC_PARTITION_EXISTS
	ret = k_mem_domain_add_partition(&k_mem_domain_default,
					 &z_libc_partition);
	__ASSERT(ret == 0, "failed to add default libc mem partition");
#पूर्ण_अगर /* Z_LIBC_PARTITION_EXISTS */
}

K_KERNEL_INIT_PRE(init_mem_domain_module);
