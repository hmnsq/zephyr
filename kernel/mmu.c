/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#अन्तर्भाव <stdint.h>
#अन्तर्भाव <kernel_arch_interface.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <mmu.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <zephyr/linker/linker-defs.h>
#अन्तर्भाव <zephyr/sys/bitarray.h>
#अन्तर्भाव <zephyr/sys/check.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/sys/minmax.h>
#अन्तर्भाव <zephyr/timing/timing.h>
#अन्तर्भाव <zephyr/arch/common/init.h>
#अन्तर्भाव <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
#अन्तर्भाव <zephyr/kernel/mm/demand_paging.h>
#यदि_अन्तः /* CONFIG_DEMAND_PAGING */

/*
 * General terminology:
 * - A page frame is a page-sized physical memory region in RAM. It is a
 *   container where a data page may be placed. It is always referred to by
 *   physical address. We have a convention of using uintptr_t for physical
 *   addresses. We instantiate a struct k_mem_page_frame to store metadata for
 *   every page frame.
 *
 * - A data page is a page-sized region of data. It may exist in a page frame,
 *   or be paged out to some backing store. Its location can always be looked
 *   up in the CPU's page tables (or equivalent) by virtual address.
 *   The data type will always be void * or in some cases uint8_t * when we
 *   want to do pointer arithmetic.
 */

/* Spinlock to protect any globals in this file and serialize page table
 * updates in arch code
 */
संरचना k_spinlock z_mm_तालकम्;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
संरचना k_mem_page_frame k_mem_page_चौकटाः[K_MEM_अङ्कः_PAGE_चौकटाः];

#यदि_परिभाषितम् CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
स्थिर bool page_चौकटाः_initialized;
#यदि_अन्तः /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#परिभाषय COLOR_PAGE_चौकटाः	1

#यदि COLOR_PAGE_चौकटाः
#परिभाषय ANSI_DEFAULT "\x1B" "[0m"
#परिभाषय ANSI_RED     "\x1B" "[1;31m"
#परिभाषय ANSI_GREEN   "\x1B" "[1;32m"
#परिभाषय ANSI_YELLOW  "\x1B" "[1;33m"
#परिभाषय ANSI_BLUE    "\x1B" "[1;34m"
#परिभाषय ANSI_MAGENTA "\x1B" "[1;35m"
#परिभाषय ANSI_CYAN    "\x1B" "[1;36m"
#परिभाषय ANSI_GREY    "\x1B" "[1;90m"

#परिभाषय COLOR(x)	printk(_CONCAT(ANSI_, x))
#अन्यथा
#परिभाषय COLOR(x)	कुरु { } यावत् (false)
#यदि_अन्तः /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
स्थिर शून्य page_frame_dump(संरचना k_mem_page_frame *pf)
{
	यदि (k_mem_page_frame_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} अन्यथा यदि (k_mem_page_frame_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} अन्यथा यदि (k_mem_page_frame_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} अन्यथा यदि (k_mem_page_frame_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} अन्यथा यदि (k_mem_page_frame_is_उपलब्धम्(pf)) {
		COLOR(GREY);
		printk(".");
	} अन्यथा यदि (k_mem_page_frame_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} अन्यथा {
		COLOR(RED);
		printk("?");
	}
}

शून्य k_mem_page_चौकटाः_dump(शून्य)
{
	पूर्णाङ्क column = 0;

	__ASSERT(page_चौकटाः_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_START, K_MEM_PHYS_RAM_अन्तः);

	प्रति (पूर्णाङ्क i = 0; i < K_MEM_अङ्कः_PAGE_चौकटाः; i++) {
		संरचना k_mem_page_frame *pf = &k_mem_page_चौकटाः[i];

		page_frame_dump(pf);

		column++;
		यदि (column == 64) {
			column = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	यदि (column != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#परिभाषय VIRT_FOREACH(आधारः_2, आकार_2, _pos) \
	प्रति ((_pos) = (आधारः_2); \
	     (_pos) < ((uint8_t *)(आधारः_2) + (आकार_2)); (_pos) += CONFIG_MMU_PAGE_आकार)

#परिभाषय PHYS_FOREACH(आधारः_2, आकार_2, _pos) \
	प्रति ((_pos) = (आधारः_2); \
	     (_pos) < ((uintptr_t)(आधारः_2) + (आकार_2)); (_pos) += CONFIG_MMU_PAGE_आकार)


/*
 * Virtual address space management
 *
 * Call all of these functions with z_mm_lock held.
 *
 * Overall virtual memory map: When the kernel starts, it resides in
 * virtual memory in the region K_MEM_KERNEL_VIRT_START to
 * K_MEM_KERNEL_VIRT_END. Unused virtual memory past this, up to the limit
 * noted by CONFIG_KERNEL_VM_SIZE may be used for runtime memory mappings.
 *
 * If CONFIG_ARCH_MAPS_ALL_RAM is set, we do not just map the kernel image,
 * but have a mapping for all RAM in place. This is for special architectural
 * purposes and does not otherwise affect page frame accounting or flags;
 * the only guarantee is that such RAM mapping outside of the Zephyr image
 * won't be disturbed by subsequent memory mapping calls.
 *
 * +--------------+ <- K_MEM_VIRT_RAM_START
 * | Undefined VM | <- May contain ancillary regions like x86_64's locore
 * +--------------+ <- K_MEM_KERNEL_VIRT_START (often == K_MEM_VIRT_RAM_START)
 * | Mapping for  |
 * | main kernel  |
 * | image        |
 * |		  |
 * |		  |
 * +--------------+ <- K_MEM_VM_FREE_START
 * |              |
 * | Unused,      |
 * | Available VM |
 * |              |
 * |..............| <- mapping_pos (grows downward as more mappings are made)
 * | Mapping      |
 * +--------------+
 * | Mapping      |
 * +--------------+
 * | ...          |
 * +--------------+
 * | Mapping      |
 * +--------------+ <- mappings start here
 * | Reserved     | <- special purpose virtual page(s) of size K_MEM_VM_RESERVED
 * +--------------+ <- K_MEM_VIRT_RAM_END
 */

/* Bitmap of virtual addresses where one bit corresponds to one page.
 * This is being used for virt_region_alloc() to figure out which
 * region of virtual addresses can be used for memory mapping.
 *
 * Note that bit #0 is the highest address so that allocation is
 * done in reverse from highest address.
 */
SYS_BITARRAY_DEFINE_STATIC(virt_region_bitmap,
			   CONFIG_KERNEL_VM_आकार / CONFIG_MMU_PAGE_आकार);

स्थिर bool virt_region_inited;

#परिभाषय Z_VIRT_REGION_START_ADDR	K_MEM_VM_FREE_START
#परिभाषय Z_VIRT_REGION_अन्तः_ADDR		(K_MEM_VIRT_RAM_अन्तः - K_MEM_VM_RESERVED)

स्थिर अन्तःस्थ uintptr_t virt_from_bitmap_विस्थापनम्(size_t विस्थापनम्, size_t आकार)
{
	प्रत्यागम POINTER_गन्तव्यम्_UINT(K_MEM_VIRT_RAM_अन्तः)
	       - (विस्थापनम् * CONFIG_MMU_PAGE_आकार) - आकार;
}

स्थिर अन्तःस्थ size_t virt_गन्तव्यम्_bitmap_विस्थापनम्(शून्य *vaddr, size_t आकार)
{
	प्रत्यागम (POINTER_गन्तव्यम्_UINT(K_MEM_VIRT_RAM_अन्तः)
		- POINTER_गन्तव्यम्_UINT(vaddr) - आकार) / CONFIG_MMU_PAGE_आकार;
}

स्थिर शून्य virt_region_init(शून्य)
{
	size_t विस्थापनम्, अङ्कः_द्वयङ्काः;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	यदि (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		अङ्कः_द्वयङ्काः = K_MEM_VM_RESERVED / CONFIG_MMU_PAGE_आकार;
		(शून्य)sys_bitarray_समुच्चयः_region(&virt_region_bitmap,
					      अङ्कः_द्वयङ्काः, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	अङ्कः_द्वयङ्काः = POINTER_गन्तव्यम्_UINT(K_MEM_VM_FREE_START)
		   - POINTER_गन्तव्यम्_UINT(K_MEM_VIRT_RAM_START);
	विस्थापनम् = virt_गन्तव्यम्_bitmap_विस्थापनम्(K_MEM_VIRT_RAM_START, अङ्कः_द्वयङ्काः);
	अङ्कः_द्वयङ्काः /= CONFIG_MMU_PAGE_आकार;
	(शून्य)sys_bitarray_समुच्चयः_region(&virt_region_bitmap,
				      अङ्कः_द्वयङ्काः, विस्थापनम्);

	virt_region_inited = true;
}

स्थिर शून्य virt_region_free(शून्य *vaddr, size_t आकार)
{
	size_t विस्थापनम्, अङ्कः_द्वयङ्काः;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	यदि (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#यदि_अपरिभाषितम् CONFIG_KERNEL_प्रत्यक्षम्_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_START_ADDR)
		 && ((vaddr_u8 + आकार - 1) < Z_VIRT_REGION_अन्तः_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, आकार);
	यदि (!((vaddr_u8 >= Z_VIRT_REGION_START_ADDR)
	      && ((vaddr_u8 + आकार - 1) < Z_VIRT_REGION_अन्तः_ADDR))) {
		प्रत्यागम;
	}

	विस्थापनम् = virt_गन्तव्यम्_bitmap_विस्थापनम्(vaddr, आकार);
	अङ्कः_द्वयङ्काः = आकार / CONFIG_MMU_PAGE_आकार;
	(शून्य)sys_bitarray_free(&virt_region_bitmap, अङ्कः_द्वयङ्काः, विस्थापनम्);
#अन्यथा /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	यदि (((vaddr_u8 >= Z_VIRT_REGION_START_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_अन्तः_ADDR)) ||
	    (((vaddr_u8 + आकार - 1) >= Z_VIRT_REGION_START_ADDR) &&
	     ((vaddr_u8 + आकार - 1) < Z_VIRT_REGION_अन्तः_ADDR))) {
		uint8_t *adjusted_start = max(vaddr_u8, Z_VIRT_REGION_START_ADDR);
		uint8_t *adjusted_अन्तः = min(vaddr_u8 + आकार,
					    Z_VIRT_REGION_अन्तः_ADDR);
		size_t adjusted_sz = adjusted_अन्तः - adjusted_start;

		विस्थापनम् = virt_गन्तव्यम्_bitmap_विस्थापनम्(adjusted_start, adjusted_sz);
		अङ्कः_द्वयङ्काः = adjusted_sz / CONFIG_MMU_PAGE_आकार;
		(शून्य)sys_bitarray_free(&virt_region_bitmap, अङ्कः_द्वयङ्काः, विस्थापनम्);
	}
#यदि_अन्तः /* !CONFIG_KERNEL_DIRECT_MAP */
}

स्थिर शून्य *virt_region_alloc(size_t आकार, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_आकार;
	size_t विस्थापनम्;
	size_t अङ्कः_द्वयङ्काः;
	पूर्णाङ्क ret;

	यदि (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	अङ्कः_द्वयङ्काः = (आकार + align - CONFIG_MMU_PAGE_आकार) / CONFIG_MMU_PAGE_आकार;
	alloc_आकार = अङ्कः_द्वयङ्काः * CONFIG_MMU_PAGE_आकार;
	ret = sys_bitarray_alloc(&virt_region_bitmap, अङ्कः_द्वयङ्काः, &विस्थापनम्);
	यदि (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			आकार);
		प्रत्यागम रिक्त;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_विस्थापनम्(विस्थापनम्, alloc_आकार);

	यदि (alloc_आकार > आकार) {
		uintptr_t aligned_dest_addr = आवर्तनम्_ऊर्ध्वम्(dest_addr, align);

		/* Here is the memory organization when trying to get an aligned
		 * virtual address:
		 *
		 * +--------------+ <- K_MEM_VIRT_RAM_START
		 * | Undefined VM |
		 * +--------------+ <- K_MEM_KERNEL_VIRT_START (often == K_MEM_VIRT_RAM_START)
		 * | Mapping for  |
		 * | main kernel  |
		 * | image        |
		 * |		  |
		 * |		  |
		 * +--------------+ <- K_MEM_VM_FREE_START
		 * | ...          |
		 * +==============+ <- dest_addr
		 * | Unused       |
		 * |..............| <- aligned_dest_addr
		 * |              |
		 * | Aligned      |
		 * | Mapping      |
		 * |              |
		 * |..............| <- aligned_dest_addr + size
		 * | Unused       |
		 * +==============+ <- offset from K_MEM_VIRT_RAM_END == dest_addr + alloc_size
		 * | ...          |
		 * +--------------+
		 * | Mapping      |
		 * +--------------+
		 * | Reserved     |
		 * +--------------+ <- K_MEM_VIRT_RAM_END
		 */

		/* Free the two unused regions */
		virt_region_free(UINT_गन्तव्यम्_POINTER(dest_addr),
				 aligned_dest_addr - dest_addr);
		यदि (((dest_addr + alloc_आकार) - (aligned_dest_addr + आकार)) > 0) {
			virt_region_free(UINT_गन्तव्यम्_POINTER(aligned_dest_addr + आकार),
					 (dest_addr + alloc_आकार) - (aligned_dest_addr + आकार));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	यदि (dest_addr < POINTER_गन्तव्यम्_UINT(Z_VIRT_REGION_START_ADDR)) {
		(शून्य)sys_bitarray_free(&virt_region_bitmap, अङ्कः_द्वयङ्काः, विस्थापनम्);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम UINT_गन्तव्यम्_POINTER(dest_addr);
}

/*
 * Free page frames management
 *
 * Call all of these functions with z_mm_lock held.
 */

/* Linked list of unused and available page frames.
 *
 * TODO: This is very simple and treats all free page frames as being equal.
 * However, there are use-cases to consolidate free pages such that entire
 * SRAM banks can be switched off to save power, and so obtaining free pages
 * may require a more complex ontology which prefers page frames in RAM banks
 * which are still active.
 *
 * This implies in the future there may be multiple slists managing physical
 * pages. Each page frame will still just have one snode link.
 */
BUILD_ASSERT(SYS_SFLIST_FLAG_द्वयङ्काः >= 1, "MMU free-page list needs one sflist flag bit");

स्थिर sys_sflist_t free_page_frame_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
स्थिर size_t z_free_page_गणनम्;

#परिभाषय PF_ASSERT(pf, व्यञ्जकम्, fmt, ...) \
	__ASSERT(व्यञ्जकम्, "page frame 0x%lx: " fmt, k_mem_page_frame_गन्तव्यम्_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
स्थिर संरचना k_mem_page_frame *free_page_frame_list_get(शून्य)
{
	sys_sfnode_t *ग्रन्थिः;
	संरचना k_mem_page_frame *pf = रिक्त;

	ग्रन्थिः = sys_sflist_get(&free_page_frame_list);
	यदि (ग्रन्थिः != रिक्त) {
		z_free_page_गणनम्--;
		pf = CONTAINER_सम्बन्धि(ग्रन्थिः, संरचना k_mem_page_frame, ग्रन्थिः);
		PF_ASSERT(pf, k_mem_page_frame_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	प्रत्यागम pf;
}

/* Release a page frame back into the list of free pages */
स्थिर शून्य free_page_frame_list_put(संरचना k_mem_page_frame *pf)
{
	PF_ASSERT(pf, k_mem_page_frame_is_उपलब्धम्(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->ग्रन्थिः, K_MEM_PAGE_FRAME_FREE);
	sys_sflist_append(&free_page_frame_list, &pf->ग्रन्थिः);
	z_free_page_गणनम्++;
}

स्थिर शून्य free_page_frame_list_init(शून्य)
{
	sys_sflist_init(&free_page_frame_list);
}

स्थिर शून्य page_frame_free_locked(संरचना k_mem_page_frame *pf)
{
	pf->va_and_flags = 0;
	free_page_frame_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
स्थिर शून्य frame_mapped_समुच्चयः(संरचना k_mem_page_frame *pf, शून्य *addr)
{
	PF_ASSERT(pf, !k_mem_page_frame_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_page_frame_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_page_frame_is_mapped(pf) || k_mem_page_frame_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_page_frame_गन्तव्यम्_virt(pf));

	uintptr_t flags_आवरणम् = CONFIG_MMU_PAGE_आकार - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_आवरणम्;

	pf->va_and_flags &= flags_आवरणम्;
	pf->va_and_flags |= va | K_MEM_PAGE_FRAME_MAPPED;
}

/* LCOV_EXCL_START */
/* Go through page frames to find the physical address mapped
 * by a virtual address.
 *
 * @param[in]  virt Virtual Address
 * @param[out] phys Physical address mapped to the input virtual address
 *                  if such mapping exists.
 *
 * @retval 0 if mapping is found and valid
 * @retval -EFAULT if virtual address is not mapped
 */
स्थिर पूर्णाङ्क virt_गन्तव्यम्_page_frame(शून्य *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	संरचना k_mem_page_frame *pf;
	पूर्णाङ्क ret = -EFAULT;

	K_MEM_PAGE_FRAME_FOREACH(paddr, pf) {
		यदि (k_mem_page_frame_is_mapped(pf)) {
			यदि (virt == k_mem_page_frame_गन्तव्यम्_virt(pf)) {
				ret = 0;
				यदि (phys != रिक्त) {
					*phys = k_mem_page_frame_गन्तव्यम्_phys(pf);
				}
				भङ्ग;
			}
		}
	}

	प्रत्यागम ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_गन्तव्यम्_page_frame, arch_page_phys_get, पूर्णाङ्क);

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
स्थिर पूर्णाङ्क page_frame_prepare_locked(संरचना k_mem_page_frame *pf, bool *dirty_ptr,
				     bool page_अन्तर्, uintptr_t *location_ptr);

स्थिर अन्तःस्थ शून्य do_backing_store_page_अन्तर्(uintptr_t location);
स्थिर अन्तःस्थ शून्य do_backing_store_page_बहिर्गमनम्(uintptr_t location);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING */

/* Allocate a free page frame, and map it to a specified virtual address
 *
 * TODO: Add optional support for copy-on-write mappings to a zero page instead
 * of allocating, in which case page frames will be allocated lazily as
 * the mappings to the zero page get touched. This will avoid expensive
 * page-ins as memory is mapped and physical RAM or backing store storage will
 * not be used if the mapped memory is unused. The cost is an empty physical
 * page of zeroes.
 *
 * @param addr Virtual address to map the page frame to
 * @param flags Mapping flags
 * @param[out] recovered Set to false if a failed mapping could not be undone,
 *                       which means the page may still be mapped and its page
 *                       frame cannot be reused.
 *
 * @retval 0 On success
 * @retval -errno On failure
 */
स्थिर पूर्णाङ्क map_anon_page(शून्य *addr, uint32_t flags, bool *recovered)
{
	संरचना k_mem_page_frame *pf;
	uintptr_t phys;
	bool तालकम् = (flags & K_MEM_MAP_तालकम्) != 0U;
	पूर्णाङ्क ret;

	*recovered = true;

	pf = free_page_frame_list_get();
	यदि (pf == रिक्त) {
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		यदि (pf == रिक्त) {
			/* Every evictable page frame is pinned or busy */
			प्रत्यागम -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_page_frame_गन्तव्यम्_virt(pf),
			k_mem_page_frame_गन्तव्यम्_phys(pf));
		ret = page_frame_prepare_locked(pf, &dirty, false, &location);
		यदि (ret != 0) {
			प्रत्यागम -ENOMEM;
		}
		यदि (dirty) {
			do_backing_store_page_बहिर्गमनम्(location);
		}
		pf->va_and_flags = 0;
#अन्यथा
		प्रत्यागम -ENOMEM;
#यदि_अन्तः /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_page_frame_गन्तव्यम्_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_PAGE_आकार, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	यदि (ret != 0) {
#यदि_परिभाषितम् CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		पूर्णाङ्क unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_PAGE_आकार);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		यदि (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			प्रत्यागम ret;
		}
#यदि_अन्तः

		/* Mapping failed: give the page frame back before bailing out. */
		page_frame_free_locked(pf);
		प्रत्यागम ret;
	}

	यदि (तालकम्) {
		k_mem_page_frame_समुच्चयः(pf, K_MEM_PAGE_FRAME_PINNED);
	}
	frame_mapped_समुच्चयः(pf, addr);
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
	यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!तालकम्)) {
		k_mem_paging_eviction_सङ्कलय(pf);
	}
#यदि_अन्तः

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	प्रत्यागम 0;
}

/**
 * Undo pages previously established by map_anon_page().
 *
 * Only used to roll back a partial k_mem_map_phys_guard() failure
 * via the anonymous memory path, the pages are known to be
 * currently mapped and not paged out.
 *
 * @param virt Start virtual address to unmap pages
 * @param sz Size of memory region to be unmapped
 *
 * @retval 0 On success
 * @retval -errno On failure
 */
स्थिर पूर्णाङ्क unmap_anon_पृष्ठानि(शून्य *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	संरचना k_mem_page_frame *pf;
	पूर्णाङ्क ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
		गणना arch_page_location स्थितिः;
		uintptr_t location;

		स्थितिः = arch_page_location_get(pos, &location);
		विकल्प (स्थितिः) {
		पक्ष ARCH_PAGE_LOCATION_PAGED_बहिर्गमनम्:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_PAGE_आकार);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			यदि (ret != 0) {
				गच्छ unmap_anon_बहिर्गमनम्;
			}

			अनुवर्त;
		पक्ष ARCH_PAGE_LOCATION_PAGED_अन्तर्:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			भङ्ग;
		स्वभाव:
			ret = arch_page_phys_get(pos, &phys);
			भङ्ग;
		}
#अन्यथा
		ret = arch_page_phys_get(pos, &phys);
#यदि_अन्तः
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		यदि (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			गच्छ unmap_anon_बहिर्गमनम्;
		}

		__ASSERT(k_mem_is_page_frame(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		यदि (!k_mem_is_page_frame(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			गच्छ unmap_anon_बहिर्गमनम्;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_गन्तव्यम्_page_frame(phys);

		__ASSERT(k_mem_page_frame_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		यदि (!k_mem_page_frame_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			गच्छ unmap_anon_बहिर्गमनम्;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_PAGE_आकार);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		यदि (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			गच्छ unmap_anon_बहिर्गमनम्;
		}

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
		यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_page_frame_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#यदि_अन्तः

		/* Put the page frame back into free list */
		page_frame_free_locked(pf);
	}

unmap_anon_बहिर्गमनम्:
	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_DEMAND_MAPPING
/**
 * Undo demand mapped pages previously established by map_anon_page().
 *
 * Only used to roll back a partial k_mem_map_phys_guard() failure
 * via the demand mapped path where pages are mapped with flags
 * K_MEM_MAP_UNPAGED.
 *
 * @param virt Start virtual address to umap pages
 * @param sz Size of memory region to be unmapped
 *
 * @retval true When able to unmap all pages.
 * @retval false When fails to unmap some pages.
 */
स्थिर bool undo_demand_mapped_पृष्ठानि(शून्य *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	पूर्णाङ्क ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_PAGE_आकार);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		यदि (ret != 0) {
			success = false;
		}
	}

	प्रत्यागम success;
}
#यदि_अन्तः

शून्य *k_mem_map_phys_guard(uintptr_t phys, size_t आकार, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t योग_आकार;
	पूर्णाङ्क ret;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && page_चौकटाः_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_आवरणम्) == 0U,
		 "%s does not support explicit cache settings", __func__);

	यदि (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		प्रत्यागम रिक्त;
	}
	यदि ((आकार % CONFIG_MMU_PAGE_आकार) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", आकार, __func__);
		प्रत्यागम रिक्त;
	}
	यदि (आकार == 0) {
		LOG_ERR("zero sized memory mapping");
		प्रत्यागम रिक्त;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	यदि (आकार_सङ्कलय_अतिप्रवाहः(आकार, CONFIG_MMU_PAGE_आकार * 2, &योग_आकार)) {
		LOG_ERR("too large size %zu passed to %s", आकार, __func__);
		प्रत्यागम रिक्त;
	}

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);

	region = virt_region_alloc(योग_आकार, CONFIG_MMU_PAGE_आकार);
	यदि (region == रिक्त) {
		/* Address space has no free region */
		dst = रिक्त;
		गच्छ बहिर्गमनम्;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_PAGE_आकार);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	यदि (ret != 0) {
		dst = रिक्त;
		गच्छ बहिर्गमनम्;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_PAGE_आकार + आकार, CONFIG_MMU_PAGE_आकार);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	यदि (ret != 0) {
		dst = रिक्त;
		गच्छ बहिर्गमनम्;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_PAGE_आकार;

	यदि (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#यदि_परिभाषितम् CONFIG_DEMAND_MAPPING
		यदि ((flags & K_MEM_MAP_तालकम्) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, आकार, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_PAGE_आकार, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				यदि (ret != 0) {
					size_t unmap_sz = pos - dst;

					यदि (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_PAGE_आकार;
					}

					bool success = undo_demand_mapped_पृष्ठानि(dst, unmap_sz);

					dst = रिक्त;

					यदि (success) {
						गच्छ fail;
					} अन्यथा {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						गच्छ बहिर्गमनम्;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} अन्यथा
#यदि_अन्तः
		{
			VIRT_FOREACH(dst, आकार, pos) {
				bool recovered;

				ret = map_anon_page(pos, flags, &recovered);

				यदि (ret != 0) {
					पूर्णाङ्क unmap_ret = unmap_anon_पृष्ठानि(dst, pos - dst);

					dst = रिक्त;

					यदि ((unmap_ret == 0) && recovered) {
						गच्छ fail;
					} अन्यथा {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						गच्छ बहिर्गमनम्;
					}
				}
			}
		}
	} अन्यथा {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, आकार, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		यदि (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, आकार);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = रिक्त;

			यदि (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				गच्छ बहिर्गमनम्;
			} अन्यथा {
				गच्छ fail;
			}
		}
	}

बहिर्गमनम्:
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);

	यदि (dst != रिक्त && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		स्मृतिं_पूरय(dst, 0, आकार);
	}

	प्रत्यागम dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, योग_आकार);

	गच्छ बहिर्गमनम्;
}

शून्य k_mem_unmap_phys_guard(शून्य *addr, size_t आकार, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	size_t योग_आकार;
	पूर्णाङ्क ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(POINTER_गन्तव्यम्_UINT(addr) >= CONFIG_MMU_PAGE_आकार);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_PAGE_आकार;
	k_mem_assert_virtual_region(pos, आकार + (CONFIG_MMU_PAGE_आकार * 2));

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_page_phys_get(pos - CONFIG_MMU_PAGE_आकार, रिक्त);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, आकार);
	यदि (ret == 0) {
		गच्छ बहिर्गमनम्;
	}

	ret = arch_page_phys_get(pos + आकार, रिक्त);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, आकार);
	यदि (ret == 0) {
		गच्छ बहिर्गमनम्;
	}

	यदि (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_पृष्ठानि(addr, आकार);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		यदि (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			गच्छ बहिर्गमनम्;
		}
	} अन्यथा {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, आकार);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		यदि (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			गच्छ बहिर्गमनम्;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_PAGE_आकार;
	योग_आकार = आकार + (CONFIG_MMU_PAGE_आकार * 2);
	virt_region_free(pos, योग_आकार);

बहिर्गमनम्:
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
}

पूर्णाङ्क k_mem_update_flags(शून्य *addr, size_t आकार, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret;

	k_mem_assert_virtual_region(addr, आकार);

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_page_phys_get(addr, &phys);
	यदि (ret < 0) {
		गच्छ बहिर्गमनम्;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, आकार);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	यदि (ret != 0) {
		गच्छ बहिर्गमनम्;
	}

	ret = arch_mem_map(addr, phys, आकार, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

बहिर्गमनम्:
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
	प्रत्यागम ret;
}

size_t k_mem_free_get(शून्य)
{
	size_t ret;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	__ASSERT(page_चौकटाः_initialized, "%s called too early", __func__);

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
	यदि (z_free_page_गणनम् > CONFIG_DEMAND_PAGING_PAGE_चौकटाः_RESERVE) {
		ret = z_free_page_गणनम् - CONFIG_DEMAND_PAGING_PAGE_चौकटाः_RESERVE;
	} अन्यथा {
		ret = 0;
	}
#अन्यथा
	ret = z_free_page_गणनम्;
#यदि_अन्तः /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);

	प्रत्यागम ret * (size_t)CONFIG_MMU_PAGE_आकार;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
स्थिर size_t virt_region_align(uintptr_t phys, size_t आकार)
{
	ARG_अप्रयुक्तम्(phys);
	ARG_अप्रयुक्तम्(आकार);

	प्रत्यागम CONFIG_MMU_PAGE_आकार;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
शून्य k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t आकार, uint32_t flags)
{
	uintptr_t aligned_phys, addr_विस्थापनम्;
	size_t aligned_आकार, align_boundary;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	uint8_t *dest_addr;
	size_t अङ्कः_द्वयङ्काः;
	size_t विस्थापनम्;
	पूर्णाङ्क ret;

#यदि_अपरिभाषितम् CONFIG_KERNEL_प्रत्यक्षम्_MAP
	__ASSERT(!(flags & K_MEM_प्रत्यक्षम्_MAP), "The direct-map is not enabled");
#यदि_अन्तः /* CONFIG_KERNEL_DIRECT_MAP */
	addr_विस्थापनम् = k_mem_region_align(&aligned_phys, &aligned_आकार,
					 phys, आकार,
					 CONFIG_MMU_PAGE_आकार);
	__ASSERT(aligned_आकार != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_आकार - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_आकार);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_आकार);

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);

	यदि (IS_ENABLED(CONFIG_KERNEL_प्रत्यक्षम्_MAP) &&
	    (flags & K_MEM_प्रत्यक्षम्_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		यदि (अन्तर्_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_अन्तः - 1)) ||
		    अन्तर्_RANGE(aligned_phys + aligned_आकार - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_अन्तः - 1))) {
			uint8_t *adjusted_start = max(dest_addr, K_MEM_VIRT_RAM_START);
			uint8_t *adjusted_अन्तः = min(dest_addr + aligned_आकार,
						    K_MEM_VIRT_RAM_अन्तः);
			size_t adjusted_sz = adjusted_अन्तः - adjusted_start;

			अङ्कः_द्वयङ्काः = adjusted_sz / CONFIG_MMU_PAGE_आकार;
			विस्थापनम् = virt_गन्तव्यम्_bitmap_विस्थापनम्(adjusted_start, adjusted_sz);
			यदि (sys_bitarray_test_and_समुच्चयः_region(
			    &virt_region_bitmap, अङ्कः_द्वयङ्काः, विस्थापनम्, true)) {
				गच्छ fail;
			}
		}
	} अन्यथा {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_आकार, align_boundary);
		यदि (!dest_addr) {
			गच्छ fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (आकार - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, आकार);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (शून्य *)dest_addr,
		aligned_phys, aligned_आकार, flags, addr_विस्थापनम्);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_आकार, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	यदि (ret != 0) {
		गच्छ fail;
	}
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);

	*virt_ptr = dest_addr + addr_विस्थापनम्;
	प्रत्यागम;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, आकार, flags);
	k_panic();
}

शून्य k_mem_unmap_phys_bare(uint8_t *virt, size_t आकार)
{
	uintptr_t aligned_virt, addr_विस्थापनम्;
	size_t aligned_आकार;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret;

	addr_विस्थापनम् = k_mem_region_align(&aligned_virt, &aligned_आकार,
					 POINTER_गन्तव्यम्_UINT(virt), आकार,
					 CONFIG_MMU_PAGE_आकार);
	__ASSERT(aligned_आकार != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_आकार - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_आकार);

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_आकार, addr_विस्थापनम्);

	ret = arch_mem_unmap(UINT_गन्तव्यम्_POINTER(aligned_virt), aligned_आकार);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	यदि (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		गच्छ बहिर्गमनम्;
	}

	virt_region_free(UINT_गन्तव्यम्_POINTER(aligned_virt), aligned_आकार);

बहिर्गमनम्:
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_आकार,
			  uintptr_t addr, size_t आकार, size_t align)
{
	size_t addr_विस्थापनम्;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = आवर्तनम्_अधस्तात्(addr, align);
	addr_विस्थापनम् = addr - *aligned_addr;
	*aligned_आकार = आवर्तनम्_ऊर्ध्वम्(आकार + addr_विस्थापनम्, align);

	प्रत्यागम addr_विस्थापनम्;
}

#यदि_परिभाषितम् CONFIG_LINKER_USE_BOOT_SECTION
स्थिर शून्य mark_linker_section_pinned(शून्य *start_addr, शून्य *अन्तः_addr,
				       bool pin)
{
	संरचना k_mem_page_frame *pf;
	uint8_t *addr;

	uintptr_t pinned_start = आवर्तनम्_अधस्तात्(POINTER_गन्तव्यम्_UINT(start_addr),
					    CONFIG_MMU_PAGE_आकार);
	uintptr_t pinned_अन्तः = आवर्तनम्_ऊर्ध्वम्(POINTER_गन्तव्यम्_UINT(अन्तः_addr),
					CONFIG_MMU_PAGE_आकार);
	size_t pinned_आकार = pinned_अन्तः - pinned_start;

	VIRT_FOREACH(UINT_गन्तव्यम्_POINTER(pinned_start), pinned_आकार, addr)
	{
		pf = k_mem_phys_गन्तव्यम्_page_frame(K_MEM_BOOT_VIRT_गन्तव्यम्_PHYS(addr));
		frame_mapped_समुच्चयः(pf, addr);

		यदि (pin) {
			k_mem_page_frame_समुच्चयः(pf, K_MEM_PAGE_FRAME_PINNED);
		} अन्यथा {
			k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_PINNED);
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
			यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_page_frame_is_evictable(pf)) {
				k_mem_paging_eviction_सङ्कलय(pf);
			}
#यदि_अन्तः
		}
	}
}
#यदि_अन्तः /* CONFIG_LINKER_USE_BOOT_SECTION */

#यदि_परिभाषितम् CONFIG_LINKER_USE_ONDEMAND_SECTION
स्थिर शून्य z_paging_ondemand_section_map(शून्य)
{
	uint8_t *addr;
	size_t आकार;
	uintptr_t location;
	uint32_t flags;
	पूर्णाङ्क ret;

	आकार = (uintptr_t)lnkr_ondemand_text_आकार;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_text_start, आकार, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_PAGE_आकार, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		यदि (ret != 0) {
			k_panic();
		}

		sys_bitarray_समुच्चयः_region(&virt_region_bitmap, 1,
					virt_गन्तव्यम्_bitmap_विस्थापनम्(addr, CONFIG_MMU_PAGE_आकार));
	}

	आकार = (uintptr_t)lnkr_ondemand_rodata_आकार;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_start, आकार, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_PAGE_आकार, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		यदि (ret != 0) {
			k_panic();
		}

		sys_bitarray_समुच्चयः_region(&virt_region_bitmap, 1,
					virt_गन्तव्यम्_bitmap_विस्थापनम्(addr, CONFIG_MMU_PAGE_आकार));
	}
}
#यदि_अन्तः /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

शून्य z_mem_manage_init(शून्य)
{
	uintptr_t phys;
	uint8_t *addr;
	संरचना k_mem_page_frame *pf;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);

	free_page_frame_list_init();

	ARG_अप्रयुक्तम्(addr);

#यदि_परिभाषितम् CONFIG_ARCH_HAS_RESERVED_PAGE_चौकटाः
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_पृष्ठानि_update();
#यदि_अन्तः /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_START, K_MEM_KERNEL_VIRT_आकार, addr)
	{
		pf = k_mem_phys_गन्तव्यम्_page_frame(K_MEM_BOOT_VIRT_गन्तव्यम्_PHYS(addr));
		frame_mapped_समुच्चयः(pf, addr);
		k_mem_page_frame_समुच्चयः(pf, K_MEM_PAGE_FRAME_PINNED);
	}

#यदि_परिभाषितम् CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_start, lnkr_boot_अन्तः, true);
#यदि_अन्तः /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_PAGE_FRAME_FOREACH(phys, pf) {
		यदि (k_mem_page_frame_is_उपलब्धम्(pf)) {
			free_page_frame_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_page_गणनम्);

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_PAGE_FRAME_FOREACH(phys, pf) {
			यदि (k_mem_page_frame_is_evictable(pf)) {
				k_mem_paging_eviction_सङ्कलय(pf);
			}
		}
	}
#यदि_अन्तः /* CONFIG_DEMAND_PAGING */

#यदि_परिभाषितम् CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_ASSERT
	page_चौकटाः_initialized = true;
#यदि_अन्तः /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
}

शून्य z_mem_manage_boot_finish(शून्य)
{
#यदि_परिभाषितम् CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_start, lnkr_boot_अन्तः, false);
#यदि_अन्तः /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS
संरचना k_mem_paging_stats_t paging_stats;
बाह्य संरचना k_mem_paging_histogram_t z_paging_histogram_eviction;
बाह्य संरचना k_mem_paging_histogram_t z_paging_histogram_backing_store_page_अन्तर्;
बाह्य संरचना k_mem_paging_histogram_t z_paging_histogram_backing_store_page_बहिर्गमनम्;
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS */

स्थिर अन्तःस्थ शून्य do_backing_store_page_अन्तर्(uintptr_t location)
{
#यदि_परिभाषितम् CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	विकल्प (location) {
	पक्ष ARCH_UNPAGED_ANON_ZERO:
		स्मृतिं_पूरय(K_MEM_SCRATCH_PAGE, 0, CONFIG_MMU_PAGE_आकार);
		__fallthrough;
	पक्ष ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		प्रत्यागम;
	स्वभाव:
		भङ्ग;
	}
#यदि_अन्तः /* CONFIG_DEMAND_MAPPING */

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_अन्तः;

	time_start = timing_गणक_get();
#अन्यथा
	uint32_t time_start;

	time_start = k_आवृत्तिः_get_32();
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_page_अन्तर्(location);

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_अन्तः = timing_गणक_get();
	time_diff = (uint32_t)timing_आवृत्तयः_get(&time_start, &time_अन्तः);
#अन्यथा
	time_diff = k_आवृत्तिः_get_32() - time_start;
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_page_अन्तर्,
			       time_diff);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

स्थिर अन्तःस्थ शून्य do_backing_store_page_बहिर्गमनम्(uintptr_t location)
{
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_अन्तः;

	time_start = timing_गणक_get();
#अन्यथा
	uint32_t time_start;

	time_start = k_आवृत्तिः_get_32();
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_page_बहिर्गमनम्(location);

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_अन्तः = timing_गणक_get();
	time_diff = (uint32_t)timing_आवृत्तयः_get(&time_start, &time_अन्तः);
#अन्यथा
	time_diff = k_आवृत्तिः_get_32() - time_start;
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_page_बहिर्गमनम्,
			       time_diff);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#यदि defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
स्थिर K_MUTEX_DEFINE(z_mm_paging_तालकम्);
#यदि_अन्तः

स्थिर शून्य virt_region_foreach(शून्य *addr, size_t आकार,
				शून्य (*func)(शून्य *vaddr))
{
	k_mem_assert_virtual_region(addr, आकार);

	प्रति (size_t विस्थापनम् = 0; विस्थापनम् < आकार; विस्थापनम् += CONFIG_MMU_PAGE_आकार) {
		func((uint8_t *)addr + विस्थापनम्);
	}
}

/*
 * Perform some preparatory steps before paging out. The provided page frame
 * must be evicted to the backing store immediately after this is called
 * with a call to k_mem_paging_backing_store_page_out() if it contains
 * a data page.
 *
 * - Map page frame to scratch area if requested. This always is true if we're
 *   doing a page fault, but is only set on manual evictions if the page is
 *   dirty.
 * - If mapped:
 *    - obtain backing store location and populate location parameter
 *    - Update page tables with location
 * - Mark page frame as busy
 *
 * Returns -ENOMEM if the backing store is full
 */
स्थिर पूर्णाङ्क page_frame_prepare_locked(संरचना k_mem_page_frame *pf, bool *dirty_ptr,
				     bool page_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	पूर्णाङ्क ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_page_frame_गन्तव्यम्_phys(pf);
	__ASSERT(!k_mem_page_frame_is_pinned(pf), "page frame 0x%lx is pinned",
		 phys);

	/* If the backing store doesn't have a copy of the page, even if it
	 * wasn't modified, treat as dirty. This can happen for a few
	 * reasons:
	 * 1) Page has never been swapped out before, and the backing store
	 *    wasn't pre-populated with this data page.
	 * 2) Page was swapped out before, but the page contents were not
	 *    preserved after swapping back in.
	 * 3) Page contents were preserved when swapped back in, but were later
	 *    evicted from the backing store to make room for other evicted
	 *    pages.
	 */
	यदि (k_mem_page_frame_is_mapped(pf)) {
		dirty = dirty || !k_mem_page_frame_is_backed(pf);
	}

	यदि (dirty || page_fault) {
		arch_mem_scratch(phys);
	}

	यदि (k_mem_page_frame_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      page_fault);
		यदि (ret != 0) {
			LOG_ERR("out of backing store memory");
			प्रत्यागम -ENOMEM;
		}
		arch_mem_page_बहिर्गमनम्(k_mem_page_frame_गन्तव्यम्_virt(pf), *location_ptr);

		यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} अन्यथा {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_page_frame_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_page_frame_समुच्चयः(pf, K_MEM_PAGE_FRAME_BUSY);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क do_mem_evict(शून्य *addr)
{
	bool dirty;
	संरचना k_mem_page_frame *pf;
	uintptr_t location;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	uintptr_t flags, phys;
	पूर्णाङ्क ret;

#यदि CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_अन्तर्_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#यदि_परिभाषितम् CONFIG_SMP
	k_mutex_तालकम्(&z_mm_paging_तालकम्, K_FOREVER);
#अन्यथा
	k_sched_तालकम्();
#यदि_अन्तः
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
	flags = arch_page_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_दत्त_PAGE_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	यदि ((flags & ARCH_दत्त_PAGE_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		गच्छ बहिर्गमनम्;
	}

	dirty = (flags & ARCH_दत्त_PAGE_DIRTY) != 0;
	pf = k_mem_phys_गन्तव्यम्_page_frame(phys);
	__ASSERT(k_mem_page_frame_गन्तव्यम्_virt(pf) == addr, "page frame address mismatch");
	ret = page_frame_prepare_locked(pf, &dirty, false, &location);
	यदि (ret != 0) {
		गच्छ बहिर्गमनम्;
	}

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	यदि (dirty) {
		do_backing_store_page_बहिर्गमनम्(location);
	}
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	page_frame_free_locked(pf);
बहिर्गमनम्:
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
#यदि_परिभाषितम् CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_तालकम्);
#अन्यथा
	k_sched_unlock();
#यदि_अन्तः
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	प्रत्यागम ret;
}

पूर्णाङ्क k_mem_page_बहिर्गमनम्(शून्य *addr, size_t आकार)
{
	__ASSERT(page_चौकटाः_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, आकार);

	प्रति (size_t विस्थापनम् = 0; विस्थापनम् < आकार; विस्थापनम् += CONFIG_MMU_PAGE_आकार) {
		शून्य *pos = (uint8_t *)addr + विस्थापनम्;
		पूर्णाङ्क ret;

		ret = do_mem_evict(pos);
		यदि (ret != 0) {
			प्रत्यागम ret;
		}
	}

	प्रत्यागम 0;
}

पूर्णाङ्क k_mem_page_frame_evict(uintptr_t phys)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	संरचना k_mem_page_frame *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	पूर्णाङ्क ret;

	__ASSERT(page_चौकटाः_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_अन्तर्_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#यदि_परिभाषितम् CONFIG_SMP
	k_mutex_तालकम्(&z_mm_paging_तालकम्, K_FOREVER);
#अन्यथा
	k_sched_तालकम्();
#यदि_अन्तः
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
	pf = k_mem_phys_गन्तव्यम्_page_frame(phys);
	यदि (!k_mem_page_frame_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		गच्छ बहिर्गमनम्;
	}
	flags = arch_page_info_get(k_mem_page_frame_गन्तव्यम्_virt(pf), रिक्त, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_दत्त_PAGE_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_दत्त_PAGE_DIRTY) != 0;
	ret = page_frame_prepare_locked(pf, &dirty, false, &location);
	यदि (ret != 0) {
		गच्छ बहिर्गमनम्;
	}

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	यदि (dirty) {
		do_backing_store_page_बहिर्गमनम्(location);
	}
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	page_frame_free_locked(pf);
बहिर्गमनम्:
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
#यदि_परिभाषितम् CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_तालकम्);
#अन्यथा
	k_sched_unlock();
#यदि_अन्तः
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	प्रत्यागम ret;
}

स्थिर अन्तःस्थ शून्य paging_stats_दोषाः_inc(संरचना k_thread *faulting_thread,
					   पूर्णाङ्क कुञ्चिका)
{
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(कुञ्चिका);

	paging_stats.pagefaults.cnt++;

	यदि (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} अन्यथा {
		paging_stats.pagefaults.irq_locked++;
	}

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	यदि (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} अन्यथा {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#अन्यथा
	ARG_अप्रयुक्तम्(faulting_thread);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#यदि_अपरिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	यदि (k_is_अन्तर्_isr()) {
		paging_stats.pagefaults.अन्तर्_isr++;

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.अन्तर्_isr++;
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS */
}

स्थिर अन्तःस्थ शून्य paging_stats_eviction_inc(संरचना k_thread *faulting_thread,
					     bool dirty)
{
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS
	यदि (dirty) {
		paging_stats.eviction.dirty++;
	} अन्यथा {
		paging_stats.eviction.clean++;
	}
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_THREAD_STATS
	यदि (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} अन्यथा {
		faulting_thread->paging_stats.eviction.clean++;
	}
#अन्यथा
	ARG_अप्रयुक्तम्(faulting_thread);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS */
}

स्थिर अन्तःस्थ संरचना k_mem_page_frame *do_eviction_select(bool *dirty)
{
	संरचना k_mem_page_frame *pf;

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_अन्तः;

	time_start = timing_गणक_get();
#अन्यथा
	uint32_t time_start;

	time_start = k_आवृत्तिः_get_32();
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_अन्तः = timing_गणक_get();
	time_diff = (uint32_t)timing_आवृत्तयः_get(&time_start, &time_अन्तः);
#अन्यथा
	time_diff = k_आवृत्तिः_get_32() - time_start;
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, time_diff);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	प्रत्यागम pf;
}

स्थिर bool do_page_fault(शून्य *addr, bool pin)
{
	संरचना k_mem_page_frame *pf;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	uintptr_t page_अन्तर्_location, page_बहिर्गमनम्_location;
	गणना arch_page_location स्थितिः;
	bool परिणाम;
	bool dirty = false;
	संरचना k_thread *faulting_thread;
	पूर्णाङ्क ret;

	__ASSERT(page_चौकटाः_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/*
	 * We do re-enable interrupts during the page-in/page-out operation
	 * if and only if interrupts were enabled when the exception was
	 * taken; in this configuration page faults in an ISR are a bug; all
	 * their code/data must be pinned.
	 *
	 * If interrupts were disabled when the exception was taken, the
	 * arch code is responsible for keeping them that way when entering
	 * this function.
	 *
	 * If this is not enabled, then interrupts are always locked for the
	 * entire operation. This is far worse for system interrupt latency
	 * but requires less pinned pages and ISRs may also take page faults.
	 *
	 * On UP we lock the scheduler so that other threads are never
	 * scheduled during the page-in/out operation. Support for
	 * allowing k_mem_paging_backing_store_page_out() and
	 * k_mem_paging_backing_store_page_in() to also sleep and allow
	 * other threads to run (such as in the case where the transfer is
	 * async DMA) is not supported on UP. Even if limited to thread
	 * context, arbitrary memory access triggering exceptions that put
	 * a thread to sleep on a contended page fault operation will break
	 * scheduling assumptions of cooperative threads or threads that
	 * implement critical sections with spinlocks or disabling IRQs.
	 *
	 * On SMP, though, exclusivity cannot be assumed solely from being
	 * a cooperative thread. Another thread with any prio may be running
	 * on another CPU so exclusion must already be enforced by other
	 * means. Therefore trying to prevent scheduling on SMP is pointless,
	 * and k_sched_lock()  is equivalent to a no-op on SMP anyway.
	 * As a result, sleeping/rescheduling in the SMP case is fine.
	 */
	__ASSERT(!k_is_अन्तर्_isr(), "ISR page faults are forbidden");
#यदि_परिभाषितम् CONFIG_SMP
	k_mutex_तालकम्(&z_mm_paging_तालकम्, K_FOREVER);
#अन्यथा
	k_sched_तालकम्();
#यदि_अन्तः
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
	faulting_thread = _current;

	स्थितिः = arch_page_location_get(addr, &page_अन्तर्_location);
	यदि (स्थितिः == ARCH_PAGE_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		परिणाम = false;
		गच्छ बहिर्गमनम्;
	}
	परिणाम = true;

	यदि (स्थितिः == ARCH_PAGE_LOCATION_PAGED_अन्तर्) {
		यदि (pin) {
			/* It's a physical memory address */
			uintptr_t phys = page_अन्तर्_location;

			pf = k_mem_phys_गन्तव्यम्_page_frame(phys);
			यदि (!k_mem_page_frame_is_pinned(pf)) {
				यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_page_frame_समुच्चयः(pf, K_MEM_PAGE_FRAME_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		गच्छ बहिर्गमनम्;
	}
	__ASSERT(स्थितिः == ARCH_PAGE_LOCATION_PAGED_बहिर्गमनम्,
		 "unexpected status value %d", स्थितिः);

	paging_stats_दोषाः_inc(faulting_thread, कुञ्चिका.कुञ्चिका);

	pf = free_page_frame_list_get();
	यदि (pf == रिक्त) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		यदि (pf == रिक्त) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			परिणाम = false;
			गच्छ बहिर्गमनम्;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_page_frame_गन्तव्यम्_virt(pf),
			k_mem_page_frame_गन्तव्यम्_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = page_frame_prepare_locked(pf, &dirty, true, &page_बहिर्गमनम्_location);
	यदि (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		परिणाम = false;
		गच्छ बहिर्गमनम्;
	}

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	यदि (dirty) {
		do_backing_store_page_बहिर्गमनम्(page_बहिर्गमनम्_location);
	}
	do_backing_store_page_अन्तर्(page_अन्तर्_location);

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
	k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_BUSY);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_MAPPED);
	frame_mapped_समुच्चयः(pf, addr);
	यदि (pin) {
		k_mem_page_frame_समुच्चयः(pf, K_MEM_PAGE_FRAME_PINNED);
	}

	arch_mem_page_अन्तर्(addr, k_mem_page_frame_गन्तव्यम्_phys(pf));
	k_mem_paging_backing_store_page_finalize(pf, page_अन्तर्_location);
	यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_सङ्कलय(pf);
	}
बहिर्गमनम्:
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_ALLOW_IRQ
#यदि_परिभाषितम् CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_तालकम्);
#अन्यथा
	k_sched_unlock();
#यदि_अन्तः
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	प्रत्यागम परिणाम;
}

स्थिर शून्य do_page_अन्तर्(शून्य *addr)
{
	bool ret;

	ret = do_page_fault(addr, false);
	यदि (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

शून्य k_mem_page_अन्तर्(शून्य *addr, size_t आकार)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_अन्तर्_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, आकार, do_page_अन्तर्);
}

स्थिर शून्य do_mem_pin(शून्य *addr)
{
	bool ret;

	ret = do_page_fault(addr, true);
	यदि (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

शून्य k_mem_pin(शून्य *addr, size_t आकार)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_अन्तर्_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, आकार, do_mem_pin);
}

bool k_mem_page_fault(शून्य *addr)
{
	प्रत्यागम do_page_fault(addr, false);
}

स्थिर शून्य do_mem_unpin(शून्य *addr)
{
	संरचना k_mem_page_frame *pf;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	uintptr_t flags, phys;

	कुञ्चिका = k_spin_तालकम्(&z_mm_तालकम्);
	flags = arch_page_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_दत्त_PAGE_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	यदि ((flags & ARCH_दत्त_PAGE_LOADED) != 0) {
		pf = k_mem_phys_गन्तव्यम्_page_frame(phys);
		यदि (k_mem_page_frame_is_pinned(pf)) {
			k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_PINNED);

			यदि (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_सङ्कलय(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_तालकम्, कुञ्चिका);
}

शून्य k_mem_unpin(शून्य *addr, size_t आकार)
{
	__ASSERT(page_चौकटाः_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, आकार, do_mem_unpin);
}

#यदि_अन्तः /* CONFIG_DEMAND_PAGING */
