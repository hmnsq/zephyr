/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#అన్తర్భావ <stdint.h>
#అన్తర్భావ <kernel_arch_interface.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <mmu.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/linker/linker-defs.h>
#అన్తర్భావ <zephyr/sys/bitarray.h>
#అన్తర్భావ <zephyr/sys/check.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/minmax.h>
#అన్తర్భావ <zephyr/timing/timing.h>
#అన్తర్భావ <zephyr/arch/common/init.h>
#అన్తర్భావ <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#నిర్వచితమైతే CONFIG_DEMAND_PAGING
#అన్తర్భావ <zephyr/kernel/mm/demand_paging.h>
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */

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
నిర్మాణం k_spinlock z_mm_తాళం;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
నిర్మాణం k_mem_పుట_చట్రం k_mem_పుట_చట్రాలు[K_MEM_అంకె_పుట_చట్రాలు];

#నిర్వచితమైతే CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
స్థిర bool పుట_చట్రాలు_initialized;
#అయితే_ముగింపు /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#నిర్వచించు COLOR_పుట_చట్రాలు	1

#ఒకవేళ COLOR_పుట_చట్రాలు
#నిర్వచించు ANSI_DEFAULT "\x1B" "[0m"
#నిర్వచించు ANSI_RED     "\x1B" "[1;31m"
#నిర్వచించు ANSI_GREEN   "\x1B" "[1;32m"
#నిర్వచించు ANSI_YELLOW  "\x1B" "[1;33m"
#నిర్వచించు ANSI_BLUE    "\x1B" "[1;34m"
#నిర్వచించు ANSI_MAGENTA "\x1B" "[1;35m"
#నిర్వచించు ANSI_CYAN    "\x1B" "[1;36m"
#నిర్వచించు ANSI_GREY    "\x1B" "[1;90m"

#నిర్వచించు COLOR(x)	printk(_CONCAT(ANSI_, x))
#లేకపోతే
#నిర్వచించు COLOR(x)	చేయి { } అయితే (false)
#అయితే_ముగింపు /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
స్థిర శూన్య పుట_చట్రం_dump(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	ఒకవేళ (k_mem_పుట_చట్రం_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} లేకపోతే ఒకవేళ (k_mem_పుట_చట్రం_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} లేకపోతే ఒకవేళ (k_mem_పుట_చట్రం_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} లేకపోతే ఒకవేళ (k_mem_పుట_చట్రం_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} లేకపోతే ఒకవేళ (k_mem_పుట_చట్రం_is_అందుబాటు(pf)) {
		COLOR(GREY);
		printk(".");
	} లేకపోతే ఒకవేళ (k_mem_పుట_చట్రం_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} లేకపోతే {
		COLOR(RED);
		printk("?");
	}
}

శూన్య k_mem_పుట_చట్రాలు_dump(శూన్య)
{
	పూర్ణాంకం స్తంభం = 0;

	__ASSERT(పుట_చట్రాలు_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_ప్రారంభం, K_MEM_PHYS_RAM_చివర);

	కొరకు (పూర్ణాంకం i = 0; i < K_MEM_అంకె_పుట_చట్రాలు; i++) {
		నిర్మాణం k_mem_పుట_చట్రం *pf = &k_mem_పుట_చట్రాలు[i];

		పుట_చట్రం_dump(pf);

		స్తంభం++;
		ఒకవేళ (స్తంభం == 64) {
			స్తంభం = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	ఒకవేళ (స్తంభం != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#నిర్వచించు VIRT_FOREACH(పునాది_2, కొలత_2, _pos) \
	కొరకు ((_pos) = (పునాది_2); \
	     (_pos) < ((uint8_t *)(పునాది_2) + (కొలత_2)); (_pos) += CONFIG_MMU_పుట_కొలత)

#నిర్వచించు PHYS_FOREACH(పునాది_2, కొలత_2, _pos) \
	కొరకు ((_pos) = (పునాది_2); \
	     (_pos) < ((uintptr_t)(పునాది_2) + (కొలత_2)); (_pos) += CONFIG_MMU_పుట_కొలత)


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
			   CONFIG_KERNEL_VM_కొలత / CONFIG_MMU_పుట_కొలత);

స్థిర bool virt_region_inited;

#నిర్వచించు Z_VIRT_REGION_ప్రారంభం_ADDR	K_MEM_VM_FREE_ప్రారంభం
#నిర్వచించు Z_VIRT_REGION_చివర_ADDR		(K_MEM_VIRT_RAM_చివర - K_MEM_VM_RESERVED)

స్థిర అంతఃవరుస uintptr_t virt_from_bitmap_స్థానభ్రంశం(size_t స్థానభ్రంశం, size_t కొలత)
{
	తిరిగి సూచిక_గమ్యం_UINT(K_MEM_VIRT_RAM_చివర)
	       - (స్థానభ్రంశం * CONFIG_MMU_పుట_కొలత) - కొలత;
}

స్థిర అంతఃవరుస size_t virt_గమ్యం_bitmap_స్థానభ్రంశం(శూన్య *vaddr, size_t కొలత)
{
	తిరిగి (సూచిక_గమ్యం_UINT(K_MEM_VIRT_RAM_చివర)
		- సూచిక_గమ్యం_UINT(vaddr) - కొలత) / CONFIG_MMU_పుట_కొలత;
}

స్థిర శూన్య virt_region_init(శూన్య)
{
	size_t స్థానభ్రంశం, అంకె_బిట్లు;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	ఒకవేళ (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		అంకె_బిట్లు = K_MEM_VM_RESERVED / CONFIG_MMU_పుట_కొలత;
		(శూన్య)sys_bitarray_సమితి_region(&virt_region_bitmap,
					      అంకె_బిట్లు, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	అంకె_బిట్లు = సూచిక_గమ్యం_UINT(K_MEM_VM_FREE_ప్రారంభం)
		   - సూచిక_గమ్యం_UINT(K_MEM_VIRT_RAM_ప్రారంభం);
	స్థానభ్రంశం = virt_గమ్యం_bitmap_స్థానభ్రంశం(K_MEM_VIRT_RAM_ప్రారంభం, అంకె_బిట్లు);
	అంకె_బిట్లు /= CONFIG_MMU_పుట_కొలత;
	(శూన్య)sys_bitarray_సమితి_region(&virt_region_bitmap,
				      అంకె_బిట్లు, స్థానభ్రంశం);

	virt_region_inited = true;
}

స్థిర శూన్య virt_region_free(శూన్య *vaddr, size_t కొలత)
{
	size_t స్థానభ్రంశం, అంకె_బిట్లు;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	ఒకవేళ (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#నిర్వచితం_కాకపోతే CONFIG_KERNEL_ప్రత్యక్ష_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_ప్రారంభం_ADDR)
		 && ((vaddr_u8 + కొలత - 1) < Z_VIRT_REGION_చివర_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, కొలత);
	ఒకవేళ (!((vaddr_u8 >= Z_VIRT_REGION_ప్రారంభం_ADDR)
	      && ((vaddr_u8 + కొలత - 1) < Z_VIRT_REGION_చివర_ADDR))) {
		తిరిగి;
	}

	స్థానభ్రంశం = virt_గమ్యం_bitmap_స్థానభ్రంశం(vaddr, కొలత);
	అంకె_బిట్లు = కొలత / CONFIG_MMU_పుట_కొలత;
	(శూన్య)sys_bitarray_free(&virt_region_bitmap, అంకె_బిట్లు, స్థానభ్రంశం);
#లేకపోతే /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	ఒకవేళ (((vaddr_u8 >= Z_VIRT_REGION_ప్రారంభం_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_చివర_ADDR)) ||
	    (((vaddr_u8 + కొలత - 1) >= Z_VIRT_REGION_ప్రారంభం_ADDR) &&
	     ((vaddr_u8 + కొలత - 1) < Z_VIRT_REGION_చివర_ADDR))) {
		uint8_t *adjusted_ప్రారంభం = max(vaddr_u8, Z_VIRT_REGION_ప్రారంభం_ADDR);
		uint8_t *adjusted_చివర = min(vaddr_u8 + కొలత,
					    Z_VIRT_REGION_చివర_ADDR);
		size_t adjusted_sz = adjusted_చివర - adjusted_ప్రారంభం;

		స్థానభ్రంశం = virt_గమ్యం_bitmap_స్థానభ్రంశం(adjusted_ప్రారంభం, adjusted_sz);
		అంకె_బిట్లు = adjusted_sz / CONFIG_MMU_పుట_కొలత;
		(శూన్య)sys_bitarray_free(&virt_region_bitmap, అంకె_బిట్లు, స్థానభ్రంశం);
	}
#అయితే_ముగింపు /* !CONFIG_KERNEL_DIRECT_MAP */
}

స్థిర శూన్య *virt_region_alloc(size_t కొలత, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_కొలత;
	size_t స్థానభ్రంశం;
	size_t అంకె_బిట్లు;
	పూర్ణాంకం ret;

	ఒకవేళ (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	అంకె_బిట్లు = (కొలత + align - CONFIG_MMU_పుట_కొలత) / CONFIG_MMU_పుట_కొలత;
	alloc_కొలత = అంకె_బిట్లు * CONFIG_MMU_పుట_కొలత;
	ret = sys_bitarray_alloc(&virt_region_bitmap, అంకె_బిట్లు, &స్థానభ్రంశం);
	ఒకవేళ (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			కొలత);
		తిరిగి ఖాళీ;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_స్థానభ్రంశం(స్థానభ్రంశం, alloc_కొలత);

	ఒకవేళ (alloc_కొలత > కొలత) {
		uintptr_t aligned_dest_addr = విడత_పైకి(dest_addr, align);

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
		virt_region_free(UINT_గమ్యం_సూచిక(dest_addr),
				 aligned_dest_addr - dest_addr);
		ఒకవేళ (((dest_addr + alloc_కొలత) - (aligned_dest_addr + కొలత)) > 0) {
			virt_region_free(UINT_గమ్యం_సూచిక(aligned_dest_addr + కొలత),
					 (dest_addr + alloc_కొలత) - (aligned_dest_addr + కొలత));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	ఒకవేళ (dest_addr < సూచిక_గమ్యం_UINT(Z_VIRT_REGION_ప్రారంభం_ADDR)) {
		(శూన్య)sys_bitarray_free(&virt_region_bitmap, అంకె_బిట్లు, స్థానభ్రంశం);
		తిరిగి ఖాళీ;
	}

	తిరిగి UINT_గమ్యం_సూచిక(dest_addr);
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
BUILD_ASSERT(SYS_SFLIST_FLAG_బిట్లు >= 1, "MMU free-page list needs one sflist flag bit");

స్థిర sys_sflist_t free_పుట_చట్రం_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
స్థిర size_t z_free_పుట_లెక్క;

#నిర్వచించు PF_ASSERT(pf, సమాసం, fmt, ...) \
	__ASSERT(సమాసం, "page frame 0x%lx: " fmt, k_mem_పుట_చట్రం_గమ్యం_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
స్థిర నిర్మాణం k_mem_పుట_చట్రం *free_పుట_చట్రం_list_get(శూన్య)
{
	sys_sfnode_t *కణుపు;
	నిర్మాణం k_mem_పుట_చట్రం *pf = ఖాళీ;

	కణుపు = sys_sflist_get(&free_పుట_చట్రం_list);
	ఒకవేళ (కణుపు != ఖాళీ) {
		z_free_పుట_లెక్క--;
		pf = CONTAINER_యొక్క(కణుపు, నిర్మాణం k_mem_పుట_చట్రం, కణుపు);
		PF_ASSERT(pf, k_mem_పుట_చట్రం_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	తిరిగి pf;
}

/* Release a page frame back into the list of free pages */
స్థిర శూన్య free_పుట_చట్రం_list_put(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	PF_ASSERT(pf, k_mem_పుట_చట్రం_is_అందుబాటు(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->కణుపు, K_MEM_పుట_చట్రం_FREE);
	sys_sflist_append(&free_పుట_చట్రం_list, &pf->కణుపు);
	z_free_పుట_లెక్క++;
}

స్థిర శూన్య free_పుట_చట్రం_list_init(శూన్య)
{
	sys_sflist_init(&free_పుట_చట్రం_list);
}

స్థిర శూన్య పుట_చట్రం_free_locked(నిర్మాణం k_mem_పుట_చట్రం *pf)
{
	pf->va_and_flags = 0;
	free_పుట_చట్రం_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
స్థిర శూన్య చట్రం_mapped_సమితి(నిర్మాణం k_mem_పుట_చట్రం *pf, శూన్య *addr)
{
	PF_ASSERT(pf, !k_mem_పుట_చట్రం_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_పుట_చట్రం_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_పుట_చట్రం_is_mapped(pf) || k_mem_పుట_చట్రం_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_పుట_చట్రం_గమ్యం_virt(pf));

	uintptr_t flags_ముసుగు = CONFIG_MMU_పుట_కొలత - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_ముసుగు;

	pf->va_and_flags &= flags_ముసుగు;
	pf->va_and_flags |= va | K_MEM_పుట_చట్రం_MAPPED;
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
స్థిర పూర్ణాంకం virt_గమ్యం_పుట_చట్రం(శూన్య *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	పూర్ణాంకం ret = -EFAULT;

	K_MEM_పుట_చట్రం_FOREACH(paddr, pf) {
		ఒకవేళ (k_mem_పుట_చట్రం_is_mapped(pf)) {
			ఒకవేళ (virt == k_mem_పుట_చట్రం_గమ్యం_virt(pf)) {
				ret = 0;
				ఒకవేళ (phys != ఖాళీ) {
					*phys = k_mem_పుట_చట్రం_గమ్యం_phys(pf);
				}
				ఆపు;
			}
		}
	}

	తిరిగి ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_గమ్యం_పుట_చట్రం, arch_పుట_phys_get, పూర్ణాంకం);

#నిర్వచితమైతే CONFIG_DEMAND_PAGING
స్థిర పూర్ణాంకం పుట_చట్రం_prepare_locked(నిర్మాణం k_mem_పుట_చట్రం *pf, bool *dirty_ptr,
				     bool పుట_లోపల, uintptr_t *location_ptr);

స్థిర అంతఃవరుస శూన్య do_backing_store_పుట_లోపల(uintptr_t location);
స్థిర అంతఃవరుస శూన్య do_backing_store_పుట_బహిర్గతం(uintptr_t location);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */

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
స్థిర పూర్ణాంకం map_anon_పుట(శూన్య *addr, uint32_t flags, bool *recovered)
{
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	uintptr_t phys;
	bool తాళం = (flags & K_MEM_MAP_తాళం) != 0U;
	పూర్ణాంకం ret;

	*recovered = true;

	pf = free_పుట_చట్రం_list_get();
	ఒకవేళ (pf == ఖాళీ) {
#నిర్వచితమైతే CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		ఒకవేళ (pf == ఖాళీ) {
			/* Every evictable page frame is pinned or busy */
			తిరిగి -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_పుట_చట్రం_గమ్యం_virt(pf),
			k_mem_పుట_చట్రం_గమ్యం_phys(pf));
		ret = పుట_చట్రం_prepare_locked(pf, &dirty, false, &location);
		ఒకవేళ (ret != 0) {
			తిరిగి -ENOMEM;
		}
		ఒకవేళ (dirty) {
			do_backing_store_పుట_బహిర్గతం(location);
		}
		pf->va_and_flags = 0;
#లేకపోతే
		తిరిగి -ENOMEM;
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_పుట_చట్రం_గమ్యం_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_పుట_కొలత, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	ఒకవేళ (ret != 0) {
#నిర్వచితమైతే CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		పూర్ణాంకం unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_పుట_కొలత);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		ఒకవేళ (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			తిరిగి ret;
		}
#అయితే_ముగింపు

		/* Mapping failed: give the page frame back before bailing out. */
		పుట_చట్రం_free_locked(pf);
		తిరిగి ret;
	}

	ఒకవేళ (తాళం) {
		k_mem_పుట_చట్రం_సమితి(pf, K_MEM_పుట_చట్రం_PINNED);
	}
	చట్రం_mapped_సమితి(pf, addr);
#నిర్వచితమైతే CONFIG_DEMAND_PAGING
	ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!తాళం)) {
		k_mem_paging_eviction_కూడు(pf);
	}
#అయితే_ముగింపు

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	తిరిగి 0;
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
స్థిర పూర్ణాంకం unmap_anon_పుటలు(శూన్య *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	పూర్ణాంకం ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#నిర్వచితమైతే CONFIG_DEMAND_PAGING
		గణన arch_పుట_location స్థితి_2;
		uintptr_t location;

		స్థితి_2 = arch_పుట_location_get(pos, &location);
		ఎంపిక (స్థితి_2) {
		స్థితి ARCH_పుట_LOCATION_PAGED_బహిర్గతం:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_పుట_కొలత);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			ఒకవేళ (ret != 0) {
				వెళ్ళు unmap_anon_బహిర్గతం;
			}

			కొనసాగు;
		స్థితి ARCH_పుట_LOCATION_PAGED_లోపల:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			ఆపు;
		అప్రమేయ:
			ret = arch_పుట_phys_get(pos, &phys);
			ఆపు;
		}
#లేకపోతే
		ret = arch_పుట_phys_get(pos, &phys);
#అయితే_ముగింపు
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		ఒకవేళ (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			వెళ్ళు unmap_anon_బహిర్గతం;
		}

		__ASSERT(k_mem_is_పుట_చట్రం(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		ఒకవేళ (!k_mem_is_పుట_చట్రం(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			వెళ్ళు unmap_anon_బహిర్గతం;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_గమ్యం_పుట_చట్రం(phys);

		__ASSERT(k_mem_పుట_చట్రం_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		ఒకవేళ (!k_mem_పుట_చట్రం_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			వెళ్ళు unmap_anon_బహిర్గతం;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_పుట_కొలత);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		ఒకవేళ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			వెళ్ళు unmap_anon_బహిర్గతం;
		}

#నిర్వచితమైతే CONFIG_DEMAND_PAGING
		ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_పుట_చట్రం_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#అయితే_ముగింపు

		/* Put the page frame back into free list */
		పుట_చట్రం_free_locked(pf);
	}

unmap_anon_బహిర్గతం:
	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_DEMAND_MAPPING
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
స్థిర bool undo_demand_mapped_పుటలు(శూన్య *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	పూర్ణాంకం ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_పుట_కొలత);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		ఒకవేళ (ret != 0) {
			success = false;
		}
	}

	తిరిగి success;
}
#అయితే_ముగింపు

శూన్య *k_mem_map_phys_guard(uintptr_t phys, size_t కొలత, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t మొత్తం_కొలత;
	పూర్ణాంకం ret;
	k_spinlock_కీలకం_t కీలకం;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && పుట_చట్రాలు_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_ముసుగు) == 0U,
		 "%s does not support explicit cache settings", __func__);

	ఒకవేళ (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		తిరిగి ఖాళీ;
	}
	ఒకవేళ ((కొలత % CONFIG_MMU_పుట_కొలత) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", కొలత, __func__);
		తిరిగి ఖాళీ;
	}
	ఒకవేళ (కొలత == 0) {
		LOG_ERR("zero sized memory mapping");
		తిరిగి ఖాళీ;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	ఒకవేళ (కొలత_కూడు_అధికప్రవాహం(కొలత, CONFIG_MMU_పుట_కొలత * 2, &మొత్తం_కొలత)) {
		LOG_ERR("too large size %zu passed to %s", కొలత, __func__);
		తిరిగి ఖాళీ;
	}

	కీలకం = k_spin_తాళం(&z_mm_తాళం);

	region = virt_region_alloc(మొత్తం_కొలత, CONFIG_MMU_పుట_కొలత);
	ఒకవేళ (region == ఖాళీ) {
		/* Address space has no free region */
		dst = ఖాళీ;
		వెళ్ళు బహిర్గతం;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_పుట_కొలత);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ఒకవేళ (ret != 0) {
		dst = ఖాళీ;
		వెళ్ళు బహిర్గతం;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_పుట_కొలత + కొలత, CONFIG_MMU_పుట_కొలత);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ఒకవేళ (ret != 0) {
		dst = ఖాళీ;
		వెళ్ళు బహిర్గతం;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_పుట_కొలత;

	ఒకవేళ (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#నిర్వచితమైతే CONFIG_DEMAND_MAPPING
		ఒకవేళ ((flags & K_MEM_MAP_తాళం) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, కొలత, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_పుట_కొలత, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				ఒకవేళ (ret != 0) {
					size_t unmap_sz = pos - dst;

					ఒకవేళ (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_పుట_కొలత;
					}

					bool success = undo_demand_mapped_పుటలు(dst, unmap_sz);

					dst = ఖాళీ;

					ఒకవేళ (success) {
						వెళ్ళు fail;
					} లేకపోతే {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						వెళ్ళు బహిర్గతం;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} లేకపోతే
#అయితే_ముగింపు
		{
			VIRT_FOREACH(dst, కొలత, pos) {
				bool recovered;

				ret = map_anon_పుట(pos, flags, &recovered);

				ఒకవేళ (ret != 0) {
					పూర్ణాంకం unmap_ret = unmap_anon_పుటలు(dst, pos - dst);

					dst = ఖాళీ;

					ఒకవేళ ((unmap_ret == 0) && recovered) {
						వెళ్ళు fail;
					} లేకపోతే {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						వెళ్ళు బహిర్గతం;
					}
				}
			}
		}
	} లేకపోతే {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, కొలత, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		ఒకవేళ (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, కొలత);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = ఖాళీ;

			ఒకవేళ (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				వెళ్ళు బహిర్గతం;
			} లేకపోతే {
				వెళ్ళు fail;
			}
		}
	}

బహిర్గతం:
	k_spin_unlock(&z_mm_తాళం, కీలకం);

	ఒకవేళ (dst != ఖాళీ && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		memset(dst, 0, కొలత);
	}

	తిరిగి dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, మొత్తం_కొలత);

	వెళ్ళు బహిర్గతం;
}

శూన్య k_mem_unmap_phys_guard(శూన్య *addr, size_t కొలత, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_కీలకం_t కీలకం;
	size_t మొత్తం_కొలత;
	పూర్ణాంకం ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(సూచిక_గమ్యం_UINT(addr) >= CONFIG_MMU_పుట_కొలత);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_పుట_కొలత;
	k_mem_assert_virtual_region(pos, కొలత + (CONFIG_MMU_పుట_కొలత * 2));

	కీలకం = k_spin_తాళం(&z_mm_తాళం);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_పుట_phys_get(pos - CONFIG_MMU_పుట_కొలత, ఖాళీ);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, కొలత);
	ఒకవేళ (ret == 0) {
		వెళ్ళు బహిర్గతం;
	}

	ret = arch_పుట_phys_get(pos + కొలత, ఖాళీ);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, కొలత);
	ఒకవేళ (ret == 0) {
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_పుటలు(addr, కొలత);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		ఒకవేళ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			వెళ్ళు బహిర్గతం;
		}
	} లేకపోతే {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, కొలత);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		ఒకవేళ (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			వెళ్ళు బహిర్గతం;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_పుట_కొలత;
	మొత్తం_కొలత = కొలత + (CONFIG_MMU_పుట_కొలత * 2);
	virt_region_free(pos, మొత్తం_కొలత);

బహిర్గతం:
	k_spin_unlock(&z_mm_తాళం, కీలకం);
}

పూర్ణాంకం k_mem_update_flags(శూన్య *addr, size_t కొలత, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ret;

	k_mem_assert_virtual_region(addr, కొలత);

	కీలకం = k_spin_తాళం(&z_mm_తాళం);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_పుట_phys_get(addr, &phys);
	ఒకవేళ (ret < 0) {
		వెళ్ళు బహిర్గతం;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, కొలత);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	ఒకవేళ (ret != 0) {
		వెళ్ళు బహిర్గతం;
	}

	ret = arch_mem_map(addr, phys, కొలత, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

బహిర్గతం:
	k_spin_unlock(&z_mm_తాళం, కీలకం);
	తిరిగి ret;
}

size_t k_mem_free_get(శూన్య)
{
	size_t ret;
	k_spinlock_కీలకం_t కీలకం;

	__ASSERT(పుట_చట్రాలు_initialized, "%s called too early", __func__);

	కీలకం = k_spin_తాళం(&z_mm_తాళం);
#నిర్వచితమైతే CONFIG_DEMAND_PAGING
	ఒకవేళ (z_free_పుట_లెక్క > CONFIG_DEMAND_PAGING_పుట_చట్రాలు_RESERVE) {
		ret = z_free_పుట_లెక్క - CONFIG_DEMAND_PAGING_పుట_చట్రాలు_RESERVE;
	} లేకపోతే {
		ret = 0;
	}
#లేకపోతే
	ret = z_free_పుట_లెక్క;
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_తాళం, కీలకం);

	తిరిగి ret * (size_t)CONFIG_MMU_పుట_కొలత;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
స్థిర size_t virt_region_align(uintptr_t phys, size_t కొలత)
{
	ARG_వాడని(phys);
	ARG_వాడని(కొలత);

	తిరిగి CONFIG_MMU_పుట_కొలత;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
శూన్య k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t కొలత, uint32_t flags)
{
	uintptr_t aligned_phys, addr_స్థానభ్రంశం;
	size_t aligned_కొలత, align_boundary;
	k_spinlock_కీలకం_t కీలకం;
	uint8_t *dest_addr;
	size_t అంకె_బిట్లు;
	size_t స్థానభ్రంశం;
	పూర్ణాంకం ret;

#నిర్వచితం_కాకపోతే CONFIG_KERNEL_ప్రత్యక్ష_MAP
	__ASSERT(!(flags & K_MEM_ప్రత్యక్ష_MAP), "The direct-map is not enabled");
#అయితే_ముగింపు /* CONFIG_KERNEL_DIRECT_MAP */
	addr_స్థానభ్రంశం = k_mem_region_align(&aligned_phys, &aligned_కొలత,
					 phys, కొలత,
					 CONFIG_MMU_పుట_కొలత);
	__ASSERT(aligned_కొలత != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_కొలత - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_కొలత);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_కొలత);

	కీలకం = k_spin_తాళం(&z_mm_తాళం);

	ఒకవేళ (IS_ENABLED(CONFIG_KERNEL_ప్రత్యక్ష_MAP) &&
	    (flags & K_MEM_ప్రత్యక్ష_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		ఒకవేళ (లోపల_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_ప్రారంభం,
			      (uintptr_t)(K_MEM_VIRT_RAM_చివర - 1)) ||
		    లోపల_RANGE(aligned_phys + aligned_కొలత - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_ప్రారంభం,
			      (uintptr_t)(K_MEM_VIRT_RAM_చివర - 1))) {
			uint8_t *adjusted_ప్రారంభం = max(dest_addr, K_MEM_VIRT_RAM_ప్రారంభం);
			uint8_t *adjusted_చివర = min(dest_addr + aligned_కొలత,
						    K_MEM_VIRT_RAM_చివర);
			size_t adjusted_sz = adjusted_చివర - adjusted_ప్రారంభం;

			అంకె_బిట్లు = adjusted_sz / CONFIG_MMU_పుట_కొలత;
			స్థానభ్రంశం = virt_గమ్యం_bitmap_స్థానభ్రంశం(adjusted_ప్రారంభం, adjusted_sz);
			ఒకవేళ (sys_bitarray_test_and_సమితి_region(
			    &virt_region_bitmap, అంకె_బిట్లు, స్థానభ్రంశం, true)) {
				వెళ్ళు fail;
			}
		}
	} లేకపోతే {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_కొలత, align_boundary);
		ఒకవేళ (!dest_addr) {
			వెళ్ళు fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (కొలత - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, కొలత);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (శూన్య *)dest_addr,
		aligned_phys, aligned_కొలత, flags, addr_స్థానభ్రంశం);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_కొలత, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	ఒకవేళ (ret != 0) {
		వెళ్ళు fail;
	}
	k_spin_unlock(&z_mm_తాళం, కీలకం);

	*virt_ptr = dest_addr + addr_స్థానభ్రంశం;
	తిరిగి;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, కొలత, flags);
	k_panic();
}

శూన్య k_mem_unmap_phys_bare(uint8_t *virt, size_t కొలత)
{
	uintptr_t aligned_virt, addr_స్థానభ్రంశం;
	size_t aligned_కొలత;
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ret;

	addr_స్థానభ్రంశం = k_mem_region_align(&aligned_virt, &aligned_కొలత,
					 సూచిక_గమ్యం_UINT(virt), కొలత,
					 CONFIG_MMU_పుట_కొలత);
	__ASSERT(aligned_కొలత != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_కొలత - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_కొలత);

	కీలకం = k_spin_తాళం(&z_mm_తాళం);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_కొలత, addr_స్థానభ్రంశం);

	ret = arch_mem_unmap(UINT_గమ్యం_సూచిక(aligned_virt), aligned_కొలత);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	ఒకవేళ (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		వెళ్ళు బహిర్గతం;
	}

	virt_region_free(UINT_గమ్యం_సూచిక(aligned_virt), aligned_కొలత);

బహిర్గతం:
	k_spin_unlock(&z_mm_తాళం, కీలకం);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_కొలత,
			  uintptr_t addr, size_t కొలత, size_t align)
{
	size_t addr_స్థానభ్రంశం;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = విడత_కిందికి(addr, align);
	addr_స్థానభ్రంశం = addr - *aligned_addr;
	*aligned_కొలత = విడత_పైకి(కొలత + addr_స్థానభ్రంశం, align);

	తిరిగి addr_స్థానభ్రంశం;
}

#నిర్వచితమైతే CONFIG_LINKER_USE_BOOT_SECTION
స్థిర శూన్య mark_linker_section_pinned(శూన్య *ప్రారంభం_addr, శూన్య *చివర_addr,
				       bool pin)
{
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	uint8_t *addr;

	uintptr_t pinned_ప్రారంభం = విడత_కిందికి(సూచిక_గమ్యం_UINT(ప్రారంభం_addr),
					    CONFIG_MMU_పుట_కొలత);
	uintptr_t pinned_చివర = విడత_పైకి(సూచిక_గమ్యం_UINT(చివర_addr),
					CONFIG_MMU_పుట_కొలత);
	size_t pinned_కొలత = pinned_చివర - pinned_ప్రారంభం;

	VIRT_FOREACH(UINT_గమ్యం_సూచిక(pinned_ప్రారంభం), pinned_కొలత, addr)
	{
		pf = k_mem_phys_గమ్యం_పుట_చట్రం(K_MEM_BOOT_VIRT_గమ్యం_PHYS(addr));
		చట్రం_mapped_సమితి(pf, addr);

		ఒకవేళ (pin) {
			k_mem_పుట_చట్రం_సమితి(pf, K_MEM_పుట_చట్రం_PINNED);
		} లేకపోతే {
			k_mem_పుట_చట్రం_clear(pf, K_MEM_పుట_చట్రం_PINNED);
#నిర్వచితమైతే CONFIG_DEMAND_PAGING
			ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_పుట_చట్రం_is_evictable(pf)) {
				k_mem_paging_eviction_కూడు(pf);
			}
#అయితే_ముగింపు
		}
	}
}
#అయితే_ముగింపు /* CONFIG_LINKER_USE_BOOT_SECTION */

#నిర్వచితమైతే CONFIG_LINKER_USE_ONDEMAND_SECTION
స్థిర శూన్య z_paging_ondemand_section_map(శూన్య)
{
	uint8_t *addr;
	size_t కొలత;
	uintptr_t location;
	uint32_t flags;
	పూర్ణాంకం ret;

	కొలత = (uintptr_t)lnkr_ondemand_పాఠ్యం_కొలత;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_పాఠ్యం_ప్రారంభం, కొలత, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_పుట_కొలత, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		ఒకవేళ (ret != 0) {
			k_panic();
		}

		sys_bitarray_సమితి_region(&virt_region_bitmap, 1,
					virt_గమ్యం_bitmap_స్థానభ్రంశం(addr, CONFIG_MMU_పుట_కొలత));
	}

	కొలత = (uintptr_t)lnkr_ondemand_rodata_కొలత;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_ప్రారంభం, కొలత, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_పుట_కొలత, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		ఒకవేళ (ret != 0) {
			k_panic();
		}

		sys_bitarray_సమితి_region(&virt_region_bitmap, 1,
					virt_గమ్యం_bitmap_స్థానభ్రంశం(addr, CONFIG_MMU_పుట_కొలత));
	}
}
#అయితే_ముగింపు /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

శూన్య z_mem_manage_init(శూన్య)
{
	uintptr_t phys;
	uint8_t *addr;
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&z_mm_తాళం);

	free_పుట_చట్రం_list_init();

	ARG_వాడని(addr);

#నిర్వచితమైతే CONFIG_ARCH_HAS_RESERVED_పుట_చట్రాలు
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_పుటలు_update();
#అయితే_ముగింపు /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_ప్రారంభం, K_MEM_KERNEL_VIRT_కొలత, addr)
	{
		pf = k_mem_phys_గమ్యం_పుట_చట్రం(K_MEM_BOOT_VIRT_గమ్యం_PHYS(addr));
		చట్రం_mapped_సమితి(pf, addr);
		k_mem_పుట_చట్రం_సమితి(pf, K_MEM_పుట_చట్రం_PINNED);
	}

#నిర్వచితమైతే CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_ప్రారంభం, lnkr_boot_చివర, true);
#అయితే_ముగింపు /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_పుట_చట్రం_FOREACH(phys, pf) {
		ఒకవేళ (k_mem_పుట_చట్రం_is_అందుబాటు(pf)) {
			free_పుట_చట్రం_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_పుట_లెక్క);

#నిర్వచితమైతే CONFIG_DEMAND_PAGING
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_పుట_చట్రం_FOREACH(phys, pf) {
			ఒకవేళ (k_mem_పుట_చట్రం_is_evictable(pf)) {
				k_mem_paging_eviction_కూడు(pf);
			}
		}
	}
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */

#నిర్వచితమైతే CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_ASSERT
	పుట_చట్రాలు_initialized = true;
#అయితే_ముగింపు /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_తాళం, కీలకం);
}

శూన్య z_mem_manage_boot_ముగింపు(శూన్య)
{
#నిర్వచితమైతే CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_ప్రారంభం, lnkr_boot_చివర, false);
#అయితే_ముగింపు /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#నిర్వచితమైతే CONFIG_DEMAND_PAGING

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS
నిర్మాణం k_mem_paging_stats_t paging_stats;
బాహ్య నిర్మాణం k_mem_paging_histogram_t z_paging_histogram_eviction;
బాహ్య నిర్మాణం k_mem_paging_histogram_t z_paging_histogram_backing_store_పుట_లోపల;
బాహ్య నిర్మాణం k_mem_paging_histogram_t z_paging_histogram_backing_store_పుట_బహిర్గతం;
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS */

స్థిర అంతఃవరుస శూన్య do_backing_store_పుట_లోపల(uintptr_t location)
{
#నిర్వచితమైతే CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	ఎంపిక (location) {
	స్థితి ARCH_UNPAGED_ANON_ZERO:
		memset(K_MEM_SCRATCH_పుట, 0, CONFIG_MMU_పుట_కొలత);
		__fallthrough;
	స్థితి ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		తిరిగి;
	అప్రమేయ:
		ఆపు;
	}
#అయితే_ముగింపు /* CONFIG_DEMAND_MAPPING */

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t సమయం_diff;

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t సమయం_ప్రారంభం, సమయం_చివర;

	సమయం_ప్రారంభం = timing_counter_get();
#లేకపోతే
	uint32_t సమయం_ప్రారంభం;

	సమయం_ప్రారంభం = k_చక్రం_get_32();
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_పుట_లోపల(location);

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	సమయం_చివర = timing_counter_get();
	సమయం_diff = (uint32_t)timing_చక్రాలు_get(&సమయం_ప్రారంభం, &సమయం_చివర);
#లేకపోతే
	సమయం_diff = k_చక్రం_get_32() - సమయం_ప్రారంభం;
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_పుట_లోపల,
			       సమయం_diff);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

స్థిర అంతఃవరుస శూన్య do_backing_store_పుట_బహిర్గతం(uintptr_t location)
{
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t సమయం_diff;

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t సమయం_ప్రారంభం, సమయం_చివర;

	సమయం_ప్రారంభం = timing_counter_get();
#లేకపోతే
	uint32_t సమయం_ప్రారంభం;

	సమయం_ప్రారంభం = k_చక్రం_get_32();
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_పుట_బహిర్గతం(location);

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	సమయం_చివర = timing_counter_get();
	సమయం_diff = (uint32_t)timing_చక్రాలు_get(&సమయం_ప్రారంభం, &సమయం_చివర);
#లేకపోతే
	సమయం_diff = k_చక్రం_get_32() - సమయం_ప్రారంభం;
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_పుట_బహిర్గతం,
			       సమయం_diff);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#ఒకవేళ defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
స్థిర K_MUTEX_DEFINE(z_mm_paging_తాళం);
#అయితే_ముగింపు

స్థిర శూన్య virt_region_foreach(శూన్య *addr, size_t కొలత,
				శూన్య (*func)(శూన్య *vaddr))
{
	k_mem_assert_virtual_region(addr, కొలత);

	కొరకు (size_t స్థానభ్రంశం = 0; స్థానభ్రంశం < కొలత; స్థానభ్రంశం += CONFIG_MMU_పుట_కొలత) {
		func((uint8_t *)addr + స్థానభ్రంశం);
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
స్థిర పూర్ణాంకం పుట_చట్రం_prepare_locked(నిర్మాణం k_mem_పుట_చట్రం *pf, bool *dirty_ptr,
				     bool పుట_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	పూర్ణాంకం ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_పుట_చట్రం_గమ్యం_phys(pf);
	__ASSERT(!k_mem_పుట_చట్రం_is_pinned(pf), "page frame 0x%lx is pinned",
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
	ఒకవేళ (k_mem_పుట_చట్రం_is_mapped(pf)) {
		dirty = dirty || !k_mem_పుట_చట్రం_is_backed(pf);
	}

	ఒకవేళ (dirty || పుట_fault) {
		arch_mem_scratch(phys);
	}

	ఒకవేళ (k_mem_పుట_చట్రం_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      పుట_fault);
		ఒకవేళ (ret != 0) {
			LOG_ERR("out of backing store memory");
			తిరిగి -ENOMEM;
		}
		arch_mem_పుట_బహిర్గతం(k_mem_పుట_చట్రం_గమ్యం_virt(pf), *location_ptr);

		ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} లేకపోతే {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_పుట_చట్రం_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_పుట_చట్రం_సమితి(pf, K_MEM_పుట_చట్రం_BUSY);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	తిరిగి 0;
}

స్థిర పూర్ణాంకం do_mem_evict(శూన్య *addr)
{
	bool dirty;
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	uintptr_t location;
	k_spinlock_కీలకం_t కీలకం;
	uintptr_t flags, phys;
	పూర్ణాంకం ret;

#ఒకవేళ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_లోపల_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#నిర్వచితమైతే CONFIG_SMP
	k_mutex_తాళం(&z_mm_paging_తాళం, K_FOREVER);
#లేకపోతే
	k_sched_తాళం();
#అయితే_ముగింపు
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	కీలకం = k_spin_తాళం(&z_mm_తాళం);
	flags = arch_పుట_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_దత్తాంశం_పుట_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	ఒకవేళ ((flags & ARCH_దత్తాంశం_పుట_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		వెళ్ళు బహిర్గతం;
	}

	dirty = (flags & ARCH_దత్తాంశం_పుట_DIRTY) != 0;
	pf = k_mem_phys_గమ్యం_పుట_చట్రం(phys);
	__ASSERT(k_mem_పుట_చట్రం_గమ్యం_virt(pf) == addr, "page frame address mismatch");
	ret = పుట_చట్రం_prepare_locked(pf, &dirty, false, &location);
	ఒకవేళ (ret != 0) {
		వెళ్ళు బహిర్గతం;
	}

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_తాళం, కీలకం);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ఒకవేళ (dirty) {
		do_backing_store_పుట_బహిర్గతం(location);
	}
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	కీలకం = k_spin_తాళం(&z_mm_తాళం);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	పుట_చట్రం_free_locked(pf);
బహిర్గతం:
	k_spin_unlock(&z_mm_తాళం, కీలకం);
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
#నిర్వచితమైతే CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_తాళం);
#లేకపోతే
	k_sched_unlock();
#అయితే_ముగింపు
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	తిరిగి ret;
}

పూర్ణాంకం k_mem_పుట_బహిర్గతం(శూన్య *addr, size_t కొలత)
{
	__ASSERT(పుట_చట్రాలు_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, కొలత);

	కొరకు (size_t స్థానభ్రంశం = 0; స్థానభ్రంశం < కొలత; స్థానభ్రంశం += CONFIG_MMU_పుట_కొలత) {
		శూన్య *pos = (uint8_t *)addr + స్థానభ్రంశం;
		పూర్ణాంకం ret;

		ret = do_mem_evict(pos);
		ఒకవేళ (ret != 0) {
			తిరిగి ret;
		}
	}

	తిరిగి 0;
}

పూర్ణాంకం k_mem_పుట_చట్రం_evict(uintptr_t phys)
{
	k_spinlock_కీలకం_t కీలకం;
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	పూర్ణాంకం ret;

	__ASSERT(పుట_చట్రాలు_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_లోపల_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#నిర్వచితమైతే CONFIG_SMP
	k_mutex_తాళం(&z_mm_paging_తాళం, K_FOREVER);
#లేకపోతే
	k_sched_తాళం();
#అయితే_ముగింపు
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	కీలకం = k_spin_తాళం(&z_mm_తాళం);
	pf = k_mem_phys_గమ్యం_పుట_చట్రం(phys);
	ఒకవేళ (!k_mem_పుట_చట్రం_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		వెళ్ళు బహిర్గతం;
	}
	flags = arch_పుట_info_get(k_mem_పుట_చట్రం_గమ్యం_virt(pf), ఖాళీ, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_దత్తాంశం_పుట_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_దత్తాంశం_పుట_DIRTY) != 0;
	ret = పుట_చట్రం_prepare_locked(pf, &dirty, false, &location);
	ఒకవేళ (ret != 0) {
		వెళ్ళు బహిర్గతం;
	}

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_తాళం, కీలకం);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ఒకవేళ (dirty) {
		do_backing_store_పుట_బహిర్గతం(location);
	}
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	కీలకం = k_spin_తాళం(&z_mm_తాళం);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	పుట_చట్రం_free_locked(pf);
బహిర్గతం:
	k_spin_unlock(&z_mm_తాళం, కీలకం);
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
#నిర్వచితమైతే CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_తాళం);
#లేకపోతే
	k_sched_unlock();
#అయితే_ముగింపు
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	తిరిగి ret;
}

స్థిర అంతఃవరుస శూన్య paging_stats_దోషాలు_inc(నిర్మాణం k_thread *faulting_thread,
					   పూర్ణాంకం కీలకం)
{
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(కీలకం);

	paging_stats.pagefaults.cnt++;

	ఒకవేళ (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} లేకపోతే {
		paging_stats.pagefaults.irq_locked++;
	}

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	ఒకవేళ (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} లేకపోతే {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#లేకపోతే
	ARG_వాడని(faulting_thread);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#నిర్వచితం_కాకపోతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	ఒకవేళ (k_is_లోపల_isr()) {
		paging_stats.pagefaults.లోపల_isr++;

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.లోపల_isr++;
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS */
}

స్థిర అంతఃవరుస శూన్య paging_stats_eviction_inc(నిర్మాణం k_thread *faulting_thread,
					     bool dirty)
{
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS
	ఒకవేళ (dirty) {
		paging_stats.eviction.dirty++;
	} లేకపోతే {
		paging_stats.eviction.clean++;
	}
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_THREAD_STATS
	ఒకవేళ (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} లేకపోతే {
		faulting_thread->paging_stats.eviction.clean++;
	}
#లేకపోతే
	ARG_వాడని(faulting_thread);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS */
}

స్థిర అంతఃవరుస నిర్మాణం k_mem_పుట_చట్రం *do_eviction_select(bool *dirty)
{
	నిర్మాణం k_mem_పుట_చట్రం *pf;

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t సమయం_diff;

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t సమయం_ప్రారంభం, సమయం_చివర;

	సమయం_ప్రారంభం = timing_counter_get();
#లేకపోతే
	uint32_t సమయం_ప్రారంభం;

	సమయం_ప్రారంభం = k_చక్రం_get_32();
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	సమయం_చివర = timing_counter_get();
	సమయం_diff = (uint32_t)timing_చక్రాలు_get(&సమయం_ప్రారంభం, &సమయం_చివర);
#లేకపోతే
	సమయం_diff = k_చక్రం_get_32() - సమయం_ప్రారంభం;
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, సమయం_diff);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	తిరిగి pf;
}

స్థిర bool do_పుట_fault(శూన్య *addr, bool pin)
{
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	k_spinlock_కీలకం_t కీలకం;
	uintptr_t పుట_లోపల_location, పుట_బహిర్గతం_location;
	గణన arch_పుట_location స్థితి_2;
	bool ఫలితం;
	bool dirty = false;
	నిర్మాణం k_thread *faulting_thread;
	పూర్ణాంకం ret;

	__ASSERT(పుట_చట్రాలు_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
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
	__ASSERT(!k_is_లోపల_isr(), "ISR page faults are forbidden");
#నిర్వచితమైతే CONFIG_SMP
	k_mutex_తాళం(&z_mm_paging_తాళం, K_FOREVER);
#లేకపోతే
	k_sched_తాళం();
#అయితే_ముగింపు
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	కీలకం = k_spin_తాళం(&z_mm_తాళం);
	faulting_thread = _current;

	స్థితి_2 = arch_పుట_location_get(addr, &పుట_లోపల_location);
	ఒకవేళ (స్థితి_2 == ARCH_పుట_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		ఫలితం = false;
		వెళ్ళు బహిర్గతం;
	}
	ఫలితం = true;

	ఒకవేళ (స్థితి_2 == ARCH_పుట_LOCATION_PAGED_లోపల) {
		ఒకవేళ (pin) {
			/* It's a physical memory address */
			uintptr_t phys = పుట_లోపల_location;

			pf = k_mem_phys_గమ్యం_పుట_చట్రం(phys);
			ఒకవేళ (!k_mem_పుట_చట్రం_is_pinned(pf)) {
				ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_పుట_చట్రం_సమితి(pf, K_MEM_పుట_చట్రం_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		వెళ్ళు బహిర్గతం;
	}
	__ASSERT(స్థితి_2 == ARCH_పుట_LOCATION_PAGED_బహిర్గతం,
		 "unexpected status value %d", స్థితి_2);

	paging_stats_దోషాలు_inc(faulting_thread, కీలకం.కీలకం);

	pf = free_పుట_చట్రం_list_get();
	ఒకవేళ (pf == ఖాళీ) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		ఒకవేళ (pf == ఖాళీ) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			ఫలితం = false;
			వెళ్ళు బహిర్గతం;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_పుట_చట్రం_గమ్యం_virt(pf),
			k_mem_పుట_చట్రం_గమ్యం_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = పుట_చట్రం_prepare_locked(pf, &dirty, true, &పుట_బహిర్గతం_location);
	ఒకవేళ (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		ఫలితం = false;
		వెళ్ళు బహిర్గతం;
	}

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_తాళం, కీలకం);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	ఒకవేళ (dirty) {
		do_backing_store_పుట_బహిర్గతం(పుట_బహిర్గతం_location);
	}
	do_backing_store_పుట_లోపల(పుట_లోపల_location);

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
	కీలకం = k_spin_తాళం(&z_mm_తాళం);
	k_mem_పుట_చట్రం_clear(pf, K_MEM_పుట_చట్రం_BUSY);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_పుట_చట్రం_clear(pf, K_MEM_పుట_చట్రం_MAPPED);
	చట్రం_mapped_సమితి(pf, addr);
	ఒకవేళ (pin) {
		k_mem_పుట_చట్రం_సమితి(pf, K_MEM_పుట_చట్రం_PINNED);
	}

	arch_mem_పుట_లోపల(addr, k_mem_పుట_చట్రం_గమ్యం_phys(pf));
	k_mem_paging_backing_store_పుట_finalize(pf, పుట_లోపల_location);
	ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_కూడు(pf);
	}
బహిర్గతం:
	k_spin_unlock(&z_mm_తాళం, కీలకం);
#నిర్వచితమైతే CONFIG_DEMAND_PAGING_ALLOW_IRQ
#నిర్వచితమైతే CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_తాళం);
#లేకపోతే
	k_sched_unlock();
#అయితే_ముగింపు
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	తిరిగి ఫలితం;
}

స్థిర శూన్య do_పుట_లోపల(శూన్య *addr)
{
	bool ret;

	ret = do_పుట_fault(addr, false);
	ఒకవేళ (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

శూన్య k_mem_పుట_లోపల(శూన్య *addr, size_t కొలత)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_లోపల_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, కొలత, do_పుట_లోపల);
}

స్థిర శూన్య do_mem_pin(శూన్య *addr)
{
	bool ret;

	ret = do_పుట_fault(addr, true);
	ఒకవేళ (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

శూన్య k_mem_pin(శూన్య *addr, size_t కొలత)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_లోపల_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, కొలత, do_mem_pin);
}

bool k_mem_పుట_fault(శూన్య *addr)
{
	తిరిగి do_పుట_fault(addr, false);
}

స్థిర శూన్య do_mem_unpin(శూన్య *addr)
{
	నిర్మాణం k_mem_పుట_చట్రం *pf;
	k_spinlock_కీలకం_t కీలకం;
	uintptr_t flags, phys;

	కీలకం = k_spin_తాళం(&z_mm_తాళం);
	flags = arch_పుట_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_దత్తాంశం_పుట_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	ఒకవేళ ((flags & ARCH_దత్తాంశం_పుట_LOADED) != 0) {
		pf = k_mem_phys_గమ్యం_పుట_చట్రం(phys);
		ఒకవేళ (k_mem_పుట_చట్రం_is_pinned(pf)) {
			k_mem_పుట_చట్రం_clear(pf, K_MEM_పుట_చట్రం_PINNED);

			ఒకవేళ (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_కూడు(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_తాళం, కీలకం);
}

శూన్య k_mem_unpin(శూన్య *addr, size_t కొలత)
{
	__ASSERT(పుట_చట్రాలు_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, కొలత, do_mem_unpin);
}

#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING */
