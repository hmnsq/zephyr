/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ KERNEL_INCLUDE_MMU_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KERNEL_INCLUDE_MMU_H

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MMU

#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/sflist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/__assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/util.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel/mm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/linker-defs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/devicetree.h>

/** Start address of physical memory. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_PHYS_RAM_ਸ਼ੁਰੂ	((uintptr_t)DT_CHOSEN_SRAM_ADDR)

/** Size of physical memory. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_PHYS_RAM_ਮਾਪ	DT_CHOSEN_SRAM_ਮਾਪ

/** End address (exclusive) of physical memory. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_PHYS_RAM_ਸਿਰਾ	(K_MEM_PHYS_RAM_ਸ਼ੁਰੂ + K_MEM_PHYS_RAM_ਮਾਪ)

/** Start address of virtual memory. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VIRT_RAM_ਸ਼ੁਰੂ	((uint8_t *)CONFIG_KERNEL_VM_ਅਧਾਰ)

/** Size of virtual memory. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VIRT_RAM_ਮਾਪ	((size_t)CONFIG_KERNEL_VM_ਮਾਪ)

/** End address (exclusive) of virtual memory. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VIRT_RAM_ਸਿਰਾ	(K_MEM_VIRT_RAM_ਸ਼ੁਰੂ + K_MEM_VIRT_RAM_ਮਾਪ)

/** Boot-time virtual start address of the kernel image. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_KERNEL_VIRT_ਸ਼ੁਰੂ	((uint8_t *)&z_mapped_ਸ਼ੁਰੂ[0])

/** Boot-time virtual end address of the kernel image. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_KERNEL_VIRT_ਸਿਰਾ	((uint8_t *)&z_mapped_ਸਿਰਾ[0])

/** Boot-time virtual address space size of the kernel image. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_KERNEL_VIRT_ਮਾਪ	(K_MEM_KERNEL_VIRT_ਸਿਰਾ - K_MEM_KERNEL_VIRT_ਸ਼ੁਰੂ)

/**
 * @brief Offset for translating between static physical and virtual addresses.
 *
 * @note Do not use directly unless you know exactly what you are going.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VM_ਵਿੱਥ	\
	((CONFIG_KERNEL_VM_ਅਧਾਰ + CONFIG_KERNEL_VM_ਵਿੱਥ) - \
	 (K_MEM_PHYS_RAM_ਸ਼ੁਰੂ + CONFIG_SRAM_ਵਿੱਥ))

/**
 * @brief Get physical address from virtual address for boot RAM mappings.
 *
 * @note Only applies to boot RAM mappings within the Zephyr image that have never
 *       been remapped or paged out. Never use this unless you know exactly what you
 *       are doing.
 *
 * @param virt Virtual address.
 *
 * @return Physical address.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_BOOT_VIRT_ਮੰਜ਼ਿਲ_PHYS(virt) ((uintptr_t)(((uint8_t *)(virt)) - K_MEM_VM_ਵਿੱਥ))

/**
 * @brief Get virtual address from physical address for boot RAM mappings.
 *
 * @note Only applies to boot RAM mappings within the Zephyr image that have never
 *       been remapped or paged out. Never use this unless you know exactly what you
 *       are doing.
 *
 * @param phys Physical address.
 *
 * @return Virtual address.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_BOOT_PHYS_ਮੰਜ਼ਿਲ_VIRT(phys) ((uint8_t *)(((uintptr_t)(phys)) + K_MEM_VM_ਵਿੱਥ))

/**
 * @def K_MEM_VM_FREE_START
 * @brief Start address of unused, available virtual addresses.
 *
 * This is the start address of the virtual memory region where
 * addresses can be allocated for memory mapping. This depends on whether
 * CONFIG_ARCH_MAPS_ALL_RAM is enabled:
 *
 * - If it is enabled, which means all physical memory are mapped in virtual
 *   memory address space, and it is the same as
 *   (DT_CHOSEN_SRAM_ADDR + DT_CHOSEN_SRAM_SIZE).
 *
 * - If it is disabled, K_MEM_VM_FREE_START is the same K_MEM_KERNEL_VIRT_END which
 *   is the end of the kernel image.
 *
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_MAPS_ALL_RAM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VM_FREE_ਸ਼ੁਰੂ	K_MEM_BOOT_PHYS_ਮੰਜ਼ਿਲ_VIRT(K_MEM_PHYS_RAM_ਸਿਰਾ)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VM_FREE_ਸ਼ੁਰੂ	K_MEM_KERNEL_VIRT_ਸਿਰਾ
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_MAPS_ALL_RAM */

/**
 * @defgroup kernel_mm_page_frame_apis Kernel Memory Page Frame Management APIs
 * @ingroup kernel_mm_internal_apis
 * @{
 *
 * Macros and data structures for physical page frame accounting,
 * APIs for use by eviction and backing store algorithms. This code
 * is otherwise not application-facing.
 */

/**
 * @brief Number of page frames.
 *
 * At present, page frame management is only done for main system RAM,
 * and we generate paging structures based on DT_CHOSEN_SRAM_ADDR
 * and DT_CHOSEN_SRAM_SIZE.
 *
 * If we have other RAM regions (DCCM, etc) these typically have special
 * properties and shouldn't be used generically for demand paging or
 * anonymous mappings. We don't currently maintain an ontology of these in the
 * core kernel.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਅੰਕ_ਪੰਨਾ_ਚੌਖਟੇ	(K_MEM_PHYS_RAM_ਮਾਪ / (size_t)CONFIG_MMU_ਪੰਨਾ_ਮਾਪ)

/*
 * k_mem_page_frame flags bits
 *
 * Requirements:
 * - K_MEM_PAGE_FRAME_FREE must be one of the possible sfnode flag bits
 * - All bit values must be lower than CONFIG_MMU_PAGE_SIZE
 */

/** This physical page is free and part of the free list */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_FREE		ਦੋਆਧਾਰੀ_ਅੰਕ(0)

/** This physical page is reserved by hardware; we will never use it */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_RESERVED	ਦੋਆਧਾਰੀ_ਅੰਕ(1)

/** This page contains critical kernel data and will never be swapped */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED		ਦੋਆਧਾਰੀ_ਅੰਕ(2)

/**
 * This physical page is mapped to some virtual memory address
 *
 * Currently, we just support one mapping per page frame. If a page frame
 * is mapped to multiple virtual pages then it must be pinned.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_MAPPED		ਦੋਆਧਾਰੀ_ਅੰਕ(3)

/**
 * This page frame is currently involved in a page-in/out operation
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_BUSY		ਦੋਆਧਾਰੀ_ਅੰਕ(4)

/**
 * This page frame has a clean copy in the backing store
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_BACKED		ਦੋਆਧਾਰੀ_ਅੰਕ(5)

/**
 * Data structure for physical page frames
 *
 * An array of these is instantiated, one element per physical RAM page.
 * Hence it's necessary to constrain its size as much as possible.
 */
ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ {
	ਸੰਘ {
		/*
		 * If mapped, K_MEM_PAGE_FRAME_* flags and virtual address
		 * this page is mapped to.
		 */
		uintptr_t va_and_flags;

		/*
		 * If unmapped and available, free pages list membership
		 * with the K_MEM_PAGE_FRAME_FREE flag.
		 */
		sys_sfnode_t ਗੰਢ;
	};

	/* Backing store and eviction algorithms may both need to
	 * require additional per-frame custom data for accounting purposes.
	 * They should declare their own array with indices matching
	 * k_mem_page_frames[] ones whenever possible.
	 * They may also want additional flags bits that could be stored here
	 * and they shouldn't clobber each other. At all costs the total
	 * size of struct k_mem_page_frame must be minimized.
	 */
};

/* Note: this must be false for the other flag bits to be valid */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_free(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (pf->va_and_flags & K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_FREE) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (pf->va_and_flags & K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_PINNED) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_reserved(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (pf->va_and_flags & K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_RESERVED) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (pf->va_and_flags & K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_MAPPED) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_busy(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (pf->va_and_flags & K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_BUSY) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_backed(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (pf->va_and_flags & K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_BACKED) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_evictable(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_free(pf) &&
		!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_reserved(pf) &&
		k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_mapped(pf) &&
		!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_pinned(pf) &&
		!k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_busy(pf));
}

/* If true, page is not being used for anything, is not reserved, is not
 * a member of some free pages list, isn't busy, and is ready to be mapped
 * in memory
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_ਪੰਨਾ_ਚੌਖਟਾ_is_ਉਪਲਬਧ(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *ਪੰਨਾ)
{
	ਵਾਪਸ ਪੰਨਾ->va_and_flags == 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਸੈੱਟ(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf, uint8_t flags)
{
	pf->va_and_flags |= flags;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ k_mem_ਪੰਨਾ_ਚੌਖਟਾ_clear(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf, uint8_t flags)
{
	/* ensure bit inversion to follow is done on the proper type width */
	uintptr_t wide_flags = flags;

	pf->va_and_flags &= ~wide_flags;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ k_mem_assert_phys_aligned(uintptr_t phys)
{
	__ASSERT(phys % CONFIG_MMU_ਪੰਨਾ_ਮਾਪ == 0U,
		 "physical address 0x%lx is not page-aligned", phys);
	(ਖਾਲੀ)phys;
}

ਬਾਹਰੀ ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ k_mem_ਪੰਨਾ_ਚੌਖਟੇ[K_MEM_ਅੰਕ_ਪੰਨਾ_ਚੌਖਟੇ];

ਸਥਿਰ ਸਤਰਅੰਦਰ uintptr_t k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_phys(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	ਵਾਪਸ (uintptr_t)((pf - k_mem_ਪੰਨਾ_ਚੌਖਟੇ) * CONFIG_MMU_ਪੰਨਾ_ਮਾਪ) +
			  K_MEM_PHYS_RAM_ਸ਼ੁਰੂ;
}

/* Presumes there is but one mapping in the virtual address space */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *k_mem_ਪੰਨਾ_ਚੌਖਟਾ_ਮੰਜ਼ਿਲ_virt(ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *pf)
{
	uintptr_t flags_ਨਕਾਬ = CONFIG_MMU_ਪੰਨਾ_ਮਾਪ - 1;

	ਵਾਪਸ (ਖਾਲੀ *)(pf->va_and_flags & ~flags_ਨਕਾਬ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool k_mem_is_ਪੰਨਾ_ਚੌਖਟਾ(uintptr_t phys)
{
	k_mem_assert_phys_aligned(phys);
	ਵਾਪਸ ਵਿੱਚ_RANGE(phys, (uintptr_t)K_MEM_PHYS_RAM_ਸ਼ੁਰੂ,
			(uintptr_t)(K_MEM_PHYS_RAM_ਸਿਰਾ - 1));
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ k_mem_ਪੰਨਾ_ਚੌਖਟਾ *k_mem_phys_ਮੰਜ਼ਿਲ_ਪੰਨਾ_ਚੌਖਟਾ(uintptr_t phys)
{
	__ASSERT(k_mem_is_ਪੰਨਾ_ਚੌਖਟਾ(phys),
		 "0x%lx not an SRAM physical address", phys);

	ਵਾਪਸ &k_mem_ਪੰਨਾ_ਚੌਖਟੇ[(phys - K_MEM_PHYS_RAM_ਸ਼ੁਰੂ) /
				  CONFIG_MMU_ਪੰਨਾ_ਮਾਪ];
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ k_mem_assert_virtual_region(uint8_t *addr, size_t ਮਾਪ)
{
	__ASSERT((uintptr_t)addr % CONFIG_MMU_ਪੰਨਾ_ਮਾਪ == 0U,
		 "unaligned addr %p", addr);
	__ASSERT(ਮਾਪ % CONFIG_MMU_ਪੰਨਾ_ਮਾਪ == 0U,
		 "unaligned size %zu", ਮਾਪ);
	__ASSERT(!Z_DETECT_ਸੂਚਕ_ਉਛਾਲ(addr, ਮਾਪ),
		 "region %p size %zu zero or wraps around", addr, ਮਾਪ);
	__ASSERT(ਵਿੱਚ_RANGE((uintptr_t)addr,
			  (uintptr_t)K_MEM_VIRT_RAM_ਸ਼ੁਰੂ,
			  ((uintptr_t)K_MEM_VIRT_RAM_ਸਿਰਾ - 1)) &&
		 ਵਿੱਚ_RANGE(((uintptr_t)addr + ਮਾਪ - 1),
			  (uintptr_t)K_MEM_VIRT_RAM_ਸ਼ੁਰੂ,
			  ((uintptr_t)K_MEM_VIRT_RAM_ਸਿਰਾ - 1)),
		 "invalid virtual address region %p (%zu)", addr, ਮਾਪ);
}

/**
 * @brief Pretty-print page frame information for all page frames.
 *
 * Debug function, pretty-print page frame information for all frames
 * concisely to printk.
 */
ਖਾਲੀ k_mem_ਪੰਨਾ_ਚੌਖਟੇ_dump(ਖਾਲੀ);

/* Convenience macro for iterating over all page frames */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_ਪੰਨਾ_ਚੌਖਟਾ_FOREACH(_phys, _pageframe) \
	ਲਈ ((_phys) = K_MEM_PHYS_RAM_ਸ਼ੁਰੂ, (_pageframe) = k_mem_ਪੰਨਾ_ਚੌਖਟੇ; \
	     (_phys) < K_MEM_PHYS_RAM_ਸਿਰਾ; \
	     (_phys) += CONFIG_MMU_ਪੰਨਾ_ਮਾਪ, (_pageframe)++)

/** @} */

/**
 * @def K_MEM_VM_RESERVED
 * @brief Reserve space at the end of virtual memory.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
/* We reserve a virtual page as a scratch area for page-ins/outs at the end
 * of the address space
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VM_RESERVED	CONFIG_MMU_ਪੰਨਾ_ਮਾਪ

/**
 * @brief Location of the scratch page used for demand paging.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_SCRATCH_ਪੰਨਾ	((ਖਾਲੀ *)((uintptr_t)CONFIG_KERNEL_VM_ਅਧਾਰ + \
					  (uintptr_t)CONFIG_KERNEL_VM_ਮਾਪ - \
					  CONFIG_MMU_ਪੰਨਾ_ਮਾਪ))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_MEM_VM_RESERVED	0
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING
/*
 * Core kernel demand paging APIs
 */

/**
 * Number of page faults since system startup
 *
 * Counts only those page faults that were handled successfully by the demand
 * paging mechanism and were not errors.
 *
 * @return Number of successful page faults
 */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ k_mem_ਅੰਕ_pagefaults_get(ਖਾਲੀ);

/**
 * Free a page frame physical address by evicting its contents
 *
 * The indicated page frame, if it contains a data page, will have that
 * data page evicted to the backing store. The page frame will then be
 * marked as available for mappings or page-ins.
 *
 * This is useful for freeing up entire memory banks so that they may be
 * deactivated to save power.
 *
 * If CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled, this function may not be
 * called by ISRs as the backing store may be in-use.
 *
 * @param phys Page frame physical address
 * @retval 0 Success
 * @retval -ENOMEM Insufficient backing store space
 */
ਪੂਰਨ k_mem_ਪੰਨਾ_ਚੌਖਟਾ_evict(uintptr_t phys);

/**
 * Handle a page fault for a virtual data page
 *
 * This is invoked from the architecture page fault handler.
 *
 * If a valid page fault, the core kernel will obtain a page frame,
 * populate it with the data page that was evicted to the backing store,
 * update page tables, and return so that the faulting instruction may be
 * re-tried.
 *
 * The architecture must not call this function if the page was mapped and
 * not paged out at the time the exception was triggered (i.e. a protection
 * violation for a mapped page).
 *
 * If the faulting context had interrupts disabled when the page fault was
 * triggered, the entire page fault handling path must have interrupts
 * disabled, including the invocation of this function.
 *
 * Otherwise, interrupts may be enabled and the page fault handler may be
 * preemptible. Races to page-in will be appropriately handled by the kernel.
 *
 * @param addr Faulting virtual address
 * @retval true Page fault successfully handled, or nothing needed to be done.
 *              The arch layer should retry the faulting instruction.
 * @retval false This page fault was from an un-mapped page, should
 *               be treated as an error, and not re-tried.
 */
bool k_mem_ਪੰਨਾ_fault(ਖਾਲੀ *addr);

#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING */
#ਜੇ_ਅੰਤ /* CONFIG_MMU */
#ਜੇ_ਅੰਤ /* KERNEL_INCLUDE_MMU_H */
