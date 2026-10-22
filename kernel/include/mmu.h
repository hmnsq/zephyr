/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ KERNEL_INCLUDE_MMU_H
#ವ್ಯಾಖ್ಯಾನಿಸು KERNEL_INCLUDE_MMU_H

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MMU

#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/sflist.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/__assert.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/util.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/mm.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/linker-defs.h>
#ಅನ್ತರ್ಭಾವ <zephyr/devicetree.h>

/** Start address of physical memory. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_PHYS_RAM_ಆರಂಭ	((uintptr_t)DT_CHOSEN_SRAM_ADDR)

/** Size of physical memory. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_PHYS_RAM_ಗಾತ್ರ	DT_CHOSEN_SRAM_ಗಾತ್ರ

/** End address (exclusive) of physical memory. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_PHYS_RAM_ಕೊನೆ	(K_MEM_PHYS_RAM_ಆರಂಭ + K_MEM_PHYS_RAM_ಗಾತ್ರ)

/** Start address of virtual memory. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VIRT_RAM_ಆರಂಭ	((uint8_t *)CONFIG_KERNEL_VM_ತಳ)

/** Size of virtual memory. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VIRT_RAM_ಗಾತ್ರ	((size_t)CONFIG_KERNEL_VM_ಗಾತ್ರ)

/** End address (exclusive) of virtual memory. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VIRT_RAM_ಕೊನೆ	(K_MEM_VIRT_RAM_ಆರಂಭ + K_MEM_VIRT_RAM_ಗಾತ್ರ)

/** Boot-time virtual start address of the kernel image. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_KERNEL_VIRT_ಆರಂಭ	((uint8_t *)&z_mapped_ಆರಂಭ[0])

/** Boot-time virtual end address of the kernel image. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_KERNEL_VIRT_ಕೊನೆ	((uint8_t *)&z_mapped_ಕೊನೆ[0])

/** Boot-time virtual address space size of the kernel image. */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_KERNEL_VIRT_ಗಾತ್ರ	(K_MEM_KERNEL_VIRT_ಕೊನೆ - K_MEM_KERNEL_VIRT_ಆರಂಭ)

/**
 * @brief Offset for translating between static physical and virtual addresses.
 *
 * @note Do not use directly unless you know exactly what you are going.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VM_ಸ್ಥಾನಾಂತರ	\
	((CONFIG_KERNEL_VM_ತಳ + CONFIG_KERNEL_VM_ಸ್ಥಾನಾಂತರ) - \
	 (K_MEM_PHYS_RAM_ಆರಂಭ + CONFIG_SRAM_ಸ್ಥಾನಾಂತರ))

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
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_BOOT_VIRT_ಗಮ್ಯ_PHYS(virt) ((uintptr_t)(((uint8_t *)(virt)) - K_MEM_VM_ಸ್ಥಾನಾಂತರ))

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
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_BOOT_PHYS_ಗಮ್ಯ_VIRT(phys) ((uint8_t *)(((uintptr_t)(phys)) + K_MEM_VM_ಸ್ಥಾನಾಂತರ))

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_MAPS_ALL_RAM
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VM_FREE_ಆರಂಭ	K_MEM_BOOT_PHYS_ಗಮ್ಯ_VIRT(K_MEM_PHYS_RAM_ಕೊನೆ)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VM_FREE_ಆರಂಭ	K_MEM_KERNEL_VIRT_ಕೊನೆ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_MAPS_ALL_RAM */

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
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಅಂಕೆ_ಪುಟ_ಚೌಕಟ್ಟುಗಳು	(K_MEM_PHYS_RAM_ಗಾತ್ರ / (size_t)CONFIG_MMU_ಪುಟ_ಗಾತ್ರ)

/*
 * k_mem_page_frame flags bits
 *
 * Requirements:
 * - K_MEM_PAGE_FRAME_FREE must be one of the possible sfnode flag bits
 * - All bit values must be lower than CONFIG_MMU_PAGE_SIZE
 */

/** This physical page is free and part of the free list */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_FREE		ಬಿಟ್(0)

/** This physical page is reserved by hardware; we will never use it */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_RESERVED	ಬಿಟ್(1)

/** This page contains critical kernel data and will never be swapped */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED		ಬಿಟ್(2)

/**
 * This physical page is mapped to some virtual memory address
 *
 * Currently, we just support one mapping per page frame. If a page frame
 * is mapped to multiple virtual pages then it must be pinned.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_MAPPED		ಬಿಟ್(3)

/**
 * This page frame is currently involved in a page-in/out operation
 */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_BUSY		ಬಿಟ್(4)

/**
 * This page frame has a clean copy in the backing store
 */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_BACKED		ಬಿಟ್(5)

/**
 * Data structure for physical page frames
 *
 * An array of these is instantiated, one element per physical RAM page.
 * Hence it's necessary to constrain its size as much as possible.
 */
ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು {
	ಒಕ್ಕೂಟ {
		/*
		 * If mapped, K_MEM_PAGE_FRAME_* flags and virtual address
		 * this page is mapped to.
		 */
		uintptr_t va_and_flags;

		/*
		 * If unmapped and available, free pages list membership
		 * with the K_MEM_PAGE_FRAME_FREE flag.
		 */
		sys_sfnode_t ಗಂಟು;
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_free(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (pf->va_and_flags & K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_FREE) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (pf->va_and_flags & K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_PINNED) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_reserved(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (pf->va_and_flags & K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_RESERVED) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (pf->va_and_flags & K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_MAPPED) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_busy(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (pf->va_and_flags & K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_BUSY) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_backed(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (pf->va_and_flags & K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_BACKED) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_evictable(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_free(pf) &&
		!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_reserved(pf) &&
		k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_mapped(pf) &&
		!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_pinned(pf) &&
		!k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_busy(pf));
}

/* If true, page is not being used for anything, is not reserved, is not
 * a member of some free pages list, isn't busy, and is ready to be mapped
 * in memory
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_ಪುಟ_ಚೌಕಟ್ಟು_is_ಲಭ್ಯ(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *ಪುಟ)
{
	ಹಿಂತಿರುಗಿ ಪುಟ->va_and_flags == 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಣ(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf, uint8_t flags)
{
	pf->va_and_flags |= flags;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ k_mem_ಪುಟ_ಚೌಕಟ್ಟು_clear(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf, uint8_t flags)
{
	/* ensure bit inversion to follow is done on the proper type width */
	uintptr_t wide_flags = flags;

	pf->va_and_flags &= ~wide_flags;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ k_mem_assert_phys_aligned(uintptr_t phys)
{
	__ASSERT(phys % CONFIG_MMU_ಪುಟ_ಗಾತ್ರ == 0U,
		 "physical address 0x%lx is not page-aligned", phys);
	(ಶೂನ್ಯ)phys;
}

ಬಾಹ್ಯ ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು[K_MEM_ಅಂಕೆ_ಪುಟ_ಚೌಕಟ್ಟುಗಳು];

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uintptr_t k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_phys(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	ಹಿಂತಿರುಗಿ (uintptr_t)((pf - k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು) * CONFIG_MMU_ಪುಟ_ಗಾತ್ರ) +
			  K_MEM_PHYS_RAM_ಆರಂಭ;
}

/* Presumes there is but one mapping in the virtual address space */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *k_mem_ಪುಟ_ಚೌಕಟ್ಟು_ಗಮ್ಯ_virt(ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *pf)
{
	uintptr_t flags_ಮುಖವಾಡ = CONFIG_MMU_ಪುಟ_ಗಾತ್ರ - 1;

	ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)(pf->va_and_flags & ~flags_ಮುಖವಾಡ);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool k_mem_is_ಪುಟ_ಚೌಕಟ್ಟು(uintptr_t phys)
{
	k_mem_assert_phys_aligned(phys);
	ಹಿಂತಿರುಗಿ ಒಳ_RANGE(phys, (uintptr_t)K_MEM_PHYS_RAM_ಆರಂಭ,
			(uintptr_t)(K_MEM_PHYS_RAM_ಕೊನೆ - 1));
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ k_mem_ಪುಟ_ಚೌಕಟ್ಟು *k_mem_phys_ಗಮ್ಯ_ಪುಟ_ಚೌಕಟ್ಟು(uintptr_t phys)
{
	__ASSERT(k_mem_is_ಪುಟ_ಚೌಕಟ್ಟು(phys),
		 "0x%lx not an SRAM physical address", phys);

	ಹಿಂತಿರುಗಿ &k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು[(phys - K_MEM_PHYS_RAM_ಆರಂಭ) /
				  CONFIG_MMU_ಪುಟ_ಗಾತ್ರ];
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ k_mem_assert_virtual_region(uint8_t *addr, size_t ಗಾತ್ರ_2)
{
	__ASSERT((uintptr_t)addr % CONFIG_MMU_ಪುಟ_ಗಾತ್ರ == 0U,
		 "unaligned addr %p", addr);
	__ASSERT(ಗಾತ್ರ_2 % CONFIG_MMU_ಪುಟ_ಗಾತ್ರ == 0U,
		 "unaligned size %zu", ಗಾತ್ರ_2);
	__ASSERT(!Z_DETECT_ಸೂಚಕ_ಅತಿಪ್ರವಾಹ(addr, ಗಾತ್ರ_2),
		 "region %p size %zu zero or wraps around", addr, ಗಾತ್ರ_2);
	__ASSERT(ಒಳ_RANGE((uintptr_t)addr,
			  (uintptr_t)K_MEM_VIRT_RAM_ಆರಂಭ,
			  ((uintptr_t)K_MEM_VIRT_RAM_ಕೊನೆ - 1)) &&
		 ಒಳ_RANGE(((uintptr_t)addr + ಗಾತ್ರ_2 - 1),
			  (uintptr_t)K_MEM_VIRT_RAM_ಆರಂಭ,
			  ((uintptr_t)K_MEM_VIRT_RAM_ಕೊನೆ - 1)),
		 "invalid virtual address region %p (%zu)", addr, ಗಾತ್ರ_2);
}

/**
 * @brief Pretty-print page frame information for all page frames.
 *
 * Debug function, pretty-print page frame information for all frames
 * concisely to printk.
 */
ಶೂನ್ಯ k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು_dump(ಶೂನ್ಯ);

/* Convenience macro for iterating over all page frames */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_ಪುಟ_ಚೌಕಟ್ಟು_FOREACH(_phys, _pageframe) \
	ಗಾಗಿ ((_phys) = K_MEM_PHYS_RAM_ಆರಂಭ, (_pageframe) = k_mem_ಪುಟ_ಚೌಕಟ್ಟುಗಳು; \
	     (_phys) < K_MEM_PHYS_RAM_ಕೊನೆ; \
	     (_phys) += CONFIG_MMU_ಪುಟ_ಗಾತ್ರ, (_pageframe)++)

/** @} */

/**
 * @def K_MEM_VM_RESERVED
 * @brief Reserve space at the end of virtual memory.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
/* We reserve a virtual page as a scratch area for page-ins/outs at the end
 * of the address space
 */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VM_RESERVED	CONFIG_MMU_ಪುಟ_ಗಾತ್ರ

/**
 * @brief Location of the scratch page used for demand paging.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_SCRATCH_ಪುಟ	((ಶೂನ್ಯ *)((uintptr_t)CONFIG_KERNEL_VM_ತಳ + \
					  (uintptr_t)CONFIG_KERNEL_VM_ಗಾತ್ರ - \
					  CONFIG_MMU_ಪುಟ_ಗಾತ್ರ))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು K_MEM_VM_RESERVED	0
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEMAND_PAGING
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
ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ k_mem_ಅಂಕೆ_pagefaults_get(ಶೂನ್ಯ);

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
ಪೂರ್ಣಾಂಕ k_mem_ಪುಟ_ಚೌಕಟ್ಟು_evict(uintptr_t phys);

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
bool k_mem_ಪುಟ_fault(ಶೂನ್ಯ *addr);

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEMAND_PAGING */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MMU */
#ಆದರೆ_ಅಂತ್ಯ /* KERNEL_INCLUDE_MMU_H */
