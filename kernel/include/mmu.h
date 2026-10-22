/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#यदि_अपरिभाषितम् KERNEL_INCLUDE_MMU_H
#परिभाषय KERNEL_INCLUDE_MMU_H

#यदि_परिभाषितम् CONFIG_MMU

#अन्तर्भाव <stdint.h>
#अन्तर्भाव <zephyr/sys/sflist.h>
#अन्तर्भाव <zephyr/sys/__assert.h>
#अन्तर्भाव <zephyr/sys/util.h>
#अन्तर्भाव <zephyr/kernel/mm.h>
#अन्तर्भाव <zephyr/linker/linker-defs.h>
#अन्तर्भाव <zephyr/devicetree.h>

/** Start address of physical memory. */
#परिभाषय K_MEM_PHYS_RAM_START	((uintptr_t)DT_CHOSEN_SRAM_ADDR)

/** Size of physical memory. */
#परिभाषय K_MEM_PHYS_RAM_आकार	DT_CHOSEN_SRAM_आकार

/** End address (exclusive) of physical memory. */
#परिभाषय K_MEM_PHYS_RAM_अन्तः	(K_MEM_PHYS_RAM_START + K_MEM_PHYS_RAM_आकार)

/** Start address of virtual memory. */
#परिभाषय K_MEM_VIRT_RAM_START	((uint8_t *)CONFIG_KERNEL_VM_आधारः)

/** Size of virtual memory. */
#परिभाषय K_MEM_VIRT_RAM_आकार	((size_t)CONFIG_KERNEL_VM_आकार)

/** End address (exclusive) of virtual memory. */
#परिभाषय K_MEM_VIRT_RAM_अन्तः	(K_MEM_VIRT_RAM_START + K_MEM_VIRT_RAM_आकार)

/** Boot-time virtual start address of the kernel image. */
#परिभाषय K_MEM_KERNEL_VIRT_START	((uint8_t *)&z_mapped_start[0])

/** Boot-time virtual end address of the kernel image. */
#परिभाषय K_MEM_KERNEL_VIRT_अन्तः	((uint8_t *)&z_mapped_अन्तः[0])

/** Boot-time virtual address space size of the kernel image. */
#परिभाषय K_MEM_KERNEL_VIRT_आकार	(K_MEM_KERNEL_VIRT_अन्तः - K_MEM_KERNEL_VIRT_START)

/**
 * @brief Offset for translating between static physical and virtual addresses.
 *
 * @note Do not use directly unless you know exactly what you are going.
 */
#परिभाषय K_MEM_VM_विस्थापनम्	\
	((CONFIG_KERNEL_VM_आधारः + CONFIG_KERNEL_VM_विस्थापनम्) - \
	 (K_MEM_PHYS_RAM_START + CONFIG_SRAM_विस्थापनम्))

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
#परिभाषय K_MEM_BOOT_VIRT_गन्तव्यम्_PHYS(virt) ((uintptr_t)(((uint8_t *)(virt)) - K_MEM_VM_विस्थापनम्))

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
#परिभाषय K_MEM_BOOT_PHYS_गन्तव्यम्_VIRT(phys) ((uint8_t *)(((uintptr_t)(phys)) + K_MEM_VM_विस्थापनम्))

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
#यदि_परिभाषितम् CONFIG_ARCH_MAPS_ALL_RAM
#परिभाषय K_MEM_VM_FREE_START	K_MEM_BOOT_PHYS_गन्तव्यम्_VIRT(K_MEM_PHYS_RAM_अन्तः)
#अन्यथा
#परिभाषय K_MEM_VM_FREE_START	K_MEM_KERNEL_VIRT_अन्तः
#यदि_अन्तः /* CONFIG_ARCH_MAPS_ALL_RAM */

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
#परिभाषय K_MEM_अङ्कः_PAGE_चौकटाः	(K_MEM_PHYS_RAM_आकार / (size_t)CONFIG_MMU_PAGE_आकार)

/*
 * k_mem_page_frame flags bits
 *
 * Requirements:
 * - K_MEM_PAGE_FRAME_FREE must be one of the possible sfnode flag bits
 * - All bit values must be lower than CONFIG_MMU_PAGE_SIZE
 */

/** This physical page is free and part of the free list */
#परिभाषय K_MEM_PAGE_FRAME_FREE		BIT(0)

/** This physical page is reserved by hardware; we will never use it */
#परिभाषय K_MEM_PAGE_FRAME_RESERVED	BIT(1)

/** This page contains critical kernel data and will never be swapped */
#परिभाषय K_MEM_PAGE_FRAME_PINNED		BIT(2)

/**
 * This physical page is mapped to some virtual memory address
 *
 * Currently, we just support one mapping per page frame. If a page frame
 * is mapped to multiple virtual pages then it must be pinned.
 */
#परिभाषय K_MEM_PAGE_FRAME_MAPPED		BIT(3)

/**
 * This page frame is currently involved in a page-in/out operation
 */
#परिभाषय K_MEM_PAGE_FRAME_BUSY		BIT(4)

/**
 * This page frame has a clean copy in the backing store
 */
#परिभाषय K_MEM_PAGE_FRAME_BACKED		BIT(5)

/**
 * Data structure for physical page frames
 *
 * An array of these is instantiated, one element per physical RAM page.
 * Hence it's necessary to constrain its size as much as possible.
 */
संरचना k_mem_page_frame {
	सङ्घ {
		/*
		 * If mapped, K_MEM_PAGE_FRAME_* flags and virtual address
		 * this page is mapped to.
		 */
		uintptr_t va_and_flags;

		/*
		 * If unmapped and available, free pages list membership
		 * with the K_MEM_PAGE_FRAME_FREE flag.
		 */
		sys_sfnode_t ग्रन्थिः;
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
स्थिर अन्तःस्थ bool k_mem_page_frame_is_free(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (pf->va_and_flags & K_MEM_PAGE_FRAME_FREE) != 0U;
}

स्थिर अन्तःस्थ bool k_mem_page_frame_is_pinned(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (pf->va_and_flags & K_MEM_PAGE_FRAME_PINNED) != 0U;
}

स्थिर अन्तःस्थ bool k_mem_page_frame_is_reserved(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (pf->va_and_flags & K_MEM_PAGE_FRAME_RESERVED) != 0U;
}

स्थिर अन्तःस्थ bool k_mem_page_frame_is_mapped(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (pf->va_and_flags & K_MEM_PAGE_FRAME_MAPPED) != 0U;
}

स्थिर अन्तःस्थ bool k_mem_page_frame_is_busy(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (pf->va_and_flags & K_MEM_PAGE_FRAME_BUSY) != 0U;
}

स्थिर अन्तःस्थ bool k_mem_page_frame_is_backed(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (pf->va_and_flags & K_MEM_PAGE_FRAME_BACKED) != 0U;
}

स्थिर अन्तःस्थ bool k_mem_page_frame_is_evictable(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (!k_mem_page_frame_is_free(pf) &&
		!k_mem_page_frame_is_reserved(pf) &&
		k_mem_page_frame_is_mapped(pf) &&
		!k_mem_page_frame_is_pinned(pf) &&
		!k_mem_page_frame_is_busy(pf));
}

/* If true, page is not being used for anything, is not reserved, is not
 * a member of some free pages list, isn't busy, and is ready to be mapped
 * in memory
 */
स्थिर अन्तःस्थ bool k_mem_page_frame_is_उपलब्धम्(संरचना k_mem_page_frame *page)
{
	प्रत्यागम page->va_and_flags == 0U;
}

स्थिर अन्तःस्थ शून्य k_mem_page_frame_समुच्चयः(संरचना k_mem_page_frame *pf, uint8_t flags)
{
	pf->va_and_flags |= flags;
}

स्थिर अन्तःस्थ शून्य k_mem_page_frame_clear(संरचना k_mem_page_frame *pf, uint8_t flags)
{
	/* ensure bit inversion to follow is done on the proper type width */
	uintptr_t wide_flags = flags;

	pf->va_and_flags &= ~wide_flags;
}

स्थिर अन्तःस्थ शून्य k_mem_assert_phys_aligned(uintptr_t phys)
{
	__ASSERT(phys % CONFIG_MMU_PAGE_आकार == 0U,
		 "physical address 0x%lx is not page-aligned", phys);
	(शून्य)phys;
}

बाह्य संरचना k_mem_page_frame k_mem_page_चौकटाः[K_MEM_अङ्कः_PAGE_चौकटाः];

स्थिर अन्तःस्थ uintptr_t k_mem_page_frame_गन्तव्यम्_phys(संरचना k_mem_page_frame *pf)
{
	प्रत्यागम (uintptr_t)((pf - k_mem_page_चौकटाः) * CONFIG_MMU_PAGE_आकार) +
			  K_MEM_PHYS_RAM_START;
}

/* Presumes there is but one mapping in the virtual address space */
स्थिर अन्तःस्थ शून्य *k_mem_page_frame_गन्तव्यम्_virt(संरचना k_mem_page_frame *pf)
{
	uintptr_t flags_आवरणम् = CONFIG_MMU_PAGE_आकार - 1;

	प्रत्यागम (शून्य *)(pf->va_and_flags & ~flags_आवरणम्);
}

स्थिर अन्तःस्थ bool k_mem_is_page_frame(uintptr_t phys)
{
	k_mem_assert_phys_aligned(phys);
	प्रत्यागम अन्तर्_RANGE(phys, (uintptr_t)K_MEM_PHYS_RAM_START,
			(uintptr_t)(K_MEM_PHYS_RAM_अन्तः - 1));
}

स्थिर अन्तःस्थ संरचना k_mem_page_frame *k_mem_phys_गन्तव्यम्_page_frame(uintptr_t phys)
{
	__ASSERT(k_mem_is_page_frame(phys),
		 "0x%lx not an SRAM physical address", phys);

	प्रत्यागम &k_mem_page_चौकटाः[(phys - K_MEM_PHYS_RAM_START) /
				  CONFIG_MMU_PAGE_आकार];
}

स्थिर अन्तःस्थ शून्य k_mem_assert_virtual_region(uint8_t *addr, size_t आकार)
{
	__ASSERT((uintptr_t)addr % CONFIG_MMU_PAGE_आकार == 0U,
		 "unaligned addr %p", addr);
	__ASSERT(आकार % CONFIG_MMU_PAGE_आकार == 0U,
		 "unaligned size %zu", आकार);
	__ASSERT(!Z_DETECT_POINTER_अतिप्रवाहः(addr, आकार),
		 "region %p size %zu zero or wraps around", addr, आकार);
	__ASSERT(अन्तर्_RANGE((uintptr_t)addr,
			  (uintptr_t)K_MEM_VIRT_RAM_START,
			  ((uintptr_t)K_MEM_VIRT_RAM_अन्तः - 1)) &&
		 अन्तर्_RANGE(((uintptr_t)addr + आकार - 1),
			  (uintptr_t)K_MEM_VIRT_RAM_START,
			  ((uintptr_t)K_MEM_VIRT_RAM_अन्तः - 1)),
		 "invalid virtual address region %p (%zu)", addr, आकार);
}

/**
 * @brief Pretty-print page frame information for all page frames.
 *
 * Debug function, pretty-print page frame information for all frames
 * concisely to printk.
 */
शून्य k_mem_page_चौकटाः_dump(शून्य);

/* Convenience macro for iterating over all page frames */
#परिभाषय K_MEM_PAGE_FRAME_FOREACH(_phys, _pageframe) \
	प्रति ((_phys) = K_MEM_PHYS_RAM_START, (_pageframe) = k_mem_page_चौकटाः; \
	     (_phys) < K_MEM_PHYS_RAM_अन्तः; \
	     (_phys) += CONFIG_MMU_PAGE_आकार, (_pageframe)++)

/** @} */

/**
 * @def K_MEM_VM_RESERVED
 * @brief Reserve space at the end of virtual memory.
 */
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
/* We reserve a virtual page as a scratch area for page-ins/outs at the end
 * of the address space
 */
#परिभाषय K_MEM_VM_RESERVED	CONFIG_MMU_PAGE_आकार

/**
 * @brief Location of the scratch page used for demand paging.
 */
#परिभाषय K_MEM_SCRATCH_PAGE	((शून्य *)((uintptr_t)CONFIG_KERNEL_VM_आधारः + \
					  (uintptr_t)CONFIG_KERNEL_VM_आकार - \
					  CONFIG_MMU_PAGE_आकार))
#अन्यथा
#परिभाषय K_MEM_VM_RESERVED	0
#यदि_अन्तः /* CONFIG_DEMAND_PAGING */

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
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
अचिह्नित दीर्घ k_mem_अङ्कः_pagefaults_get(शून्य);

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
पूर्णाङ्क k_mem_page_frame_evict(uintptr_t phys);

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
bool k_mem_page_fault(शून्य *addr);

#यदि_अन्तः /* CONFIG_DEMAND_PAGING */
#यदि_अन्तः /* CONFIG_MMU */
#यदि_अन्तः /* KERNEL_INCLUDE_MMU_H */
