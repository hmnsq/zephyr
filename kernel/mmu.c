/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#شامل <stdint.h>
#شامل <kernel_arch_interface.h>
#شامل <zephyr/spinlock.h>
#شامل <mmu.h>
#شامل <zephyr/init.h>
#شامل <kernel_internal.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/toolchain.h>
#شامل <zephyr/linker/linker-defs.h>
#شامل <zephyr/sys/bitarray.h>
#شامل <zephyr/sys/check.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/minmax.h>
#شامل <zephyr/timing/timing.h>
#شامل <zephyr/arch/common/init.h>
#شامل <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
#شامل <zephyr/kernel/mm/demand_paging.h>
#اگر_ختم /* CONFIG_DEMAND_PAGING */

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
ساخت k_spinlock z_mm_تالا;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
ساخت k_mem_صفحہ_چوکھٹا k_mem_صفحہ_چوکھٹے[K_MEM_عدد_صفحہ_چوکھٹے];

#اگر_تعریف_شدہ CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
ساکن bool صفحہ_چوکھٹے_initialized;
#اگر_ختم /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#تعریف_کرو COLOR_صفحہ_چوکھٹے	1

#اگر COLOR_صفحہ_چوکھٹے
#تعریف_کرو ANSI_DEFAULT "\x1B" "[0m"
#تعریف_کرو ANSI_RED     "\x1B" "[1;31m"
#تعریف_کرو ANSI_GREEN   "\x1B" "[1;32m"
#تعریف_کرو ANSI_YELLOW  "\x1B" "[1;33m"
#تعریف_کرو ANSI_BLUE    "\x1B" "[1;34m"
#تعریف_کرو ANSI_MAGENTA "\x1B" "[1;35m"
#تعریف_کرو ANSI_CYAN    "\x1B" "[1;36m"
#تعریف_کرو ANSI_GREY    "\x1B" "[1;90m"

#تعریف_کرو COLOR(x)	printk(_CONCAT(ANSI_, x))
#ورنہ
#تعریف_کرو COLOR(x)	کرو { } جب (false)
#اگر_ختم /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
ساکن خالی صفحہ_چوکھٹا_dump(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	اگر (k_mem_صفحہ_چوکھٹا_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} ورنہ اگر (k_mem_صفحہ_چوکھٹا_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} ورنہ اگر (k_mem_صفحہ_چوکھٹا_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} ورنہ اگر (k_mem_صفحہ_چوکھٹا_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} ورنہ اگر (k_mem_صفحہ_چوکھٹا_is_دستیاب(pf)) {
		COLOR(GREY);
		printk(".");
	} ورنہ اگر (k_mem_صفحہ_چوکھٹا_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} ورنہ {
		COLOR(RED);
		printk("?");
	}
}

خالی k_mem_صفحہ_چوکھٹے_dump(خالی)
{
	صحیح عمود = 0;

	__ASSERT(صفحہ_چوکھٹے_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_آغاز, K_MEM_PHYS_RAM_انتہا);

	برائے (صحیح i = 0; i < K_MEM_عدد_صفحہ_چوکھٹے; i++) {
		ساخت k_mem_صفحہ_چوکھٹا *pf = &k_mem_صفحہ_چوکھٹے[i];

		صفحہ_چوکھٹا_dump(pf);

		عمود++;
		اگر (عمود == 64) {
			عمود = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	اگر (عمود != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#تعریف_کرو VIRT_FOREACH(بنیاد_2, سائز_2, _pos) \
	برائے ((_pos) = (بنیاد_2); \
	     (_pos) < ((uint8_t *)(بنیاد_2) + (سائز_2)); (_pos) += CONFIG_MMU_صفحہ_سائز)

#تعریف_کرو PHYS_FOREACH(بنیاد_2, سائز_2, _pos) \
	برائے ((_pos) = (بنیاد_2); \
	     (_pos) < ((uintptr_t)(بنیاد_2) + (سائز_2)); (_pos) += CONFIG_MMU_صفحہ_سائز)


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
			   CONFIG_KERNEL_VM_سائز / CONFIG_MMU_صفحہ_سائز);

ساکن bool virt_region_inited;

#تعریف_کرو Z_VIRT_REGION_آغاز_ADDR	K_MEM_VM_FREE_آغاز
#تعریف_کرو Z_VIRT_REGION_انتہا_ADDR		(K_MEM_VIRT_RAM_انتہا - K_MEM_VM_RESERVED)

ساکن اندرونی uintptr_t virt_from_bitmap_انحراف(size_t انحراف, size_t سائز)
{
	واپس اشارہ_گر_مقصد_UINT(K_MEM_VIRT_RAM_انتہا)
	       - (انحراف * CONFIG_MMU_صفحہ_سائز) - سائز;
}

ساکن اندرونی size_t virt_مقصد_bitmap_انحراف(خالی *vaddr, size_t سائز)
{
	واپس (اشارہ_گر_مقصد_UINT(K_MEM_VIRT_RAM_انتہا)
		- اشارہ_گر_مقصد_UINT(vaddr) - سائز) / CONFIG_MMU_صفحہ_سائز;
}

ساکن خالی virt_region_init(خالی)
{
	size_t انحراف, عدد_بٹ;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	اگر (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		عدد_بٹ = K_MEM_VM_RESERVED / CONFIG_MMU_صفحہ_سائز;
		(خالی)sys_bitarray_سیٹ_region(&virt_region_bitmap,
					      عدد_بٹ, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	عدد_بٹ = اشارہ_گر_مقصد_UINT(K_MEM_VM_FREE_آغاز)
		   - اشارہ_گر_مقصد_UINT(K_MEM_VIRT_RAM_آغاز);
	انحراف = virt_مقصد_bitmap_انحراف(K_MEM_VIRT_RAM_آغاز, عدد_بٹ);
	عدد_بٹ /= CONFIG_MMU_صفحہ_سائز;
	(خالی)sys_bitarray_سیٹ_region(&virt_region_bitmap,
				      عدد_بٹ, انحراف);

	virt_region_inited = true;
}

ساکن خالی virt_region_free(خالی *vaddr, size_t سائز)
{
	size_t انحراف, عدد_بٹ;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	اگر (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#اگر_غیر_تعریف_شدہ CONFIG_KERNEL_براہ_راست_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_آغاز_ADDR)
		 && ((vaddr_u8 + سائز - 1) < Z_VIRT_REGION_انتہا_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, سائز);
	اگر (!((vaddr_u8 >= Z_VIRT_REGION_آغاز_ADDR)
	      && ((vaddr_u8 + سائز - 1) < Z_VIRT_REGION_انتہا_ADDR))) {
		واپس;
	}

	انحراف = virt_مقصد_bitmap_انحراف(vaddr, سائز);
	عدد_بٹ = سائز / CONFIG_MMU_صفحہ_سائز;
	(خالی)sys_bitarray_free(&virt_region_bitmap, عدد_بٹ, انحراف);
#ورنہ /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	اگر (((vaddr_u8 >= Z_VIRT_REGION_آغاز_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_انتہا_ADDR)) ||
	    (((vaddr_u8 + سائز - 1) >= Z_VIRT_REGION_آغاز_ADDR) &&
	     ((vaddr_u8 + سائز - 1) < Z_VIRT_REGION_انتہا_ADDR))) {
		uint8_t *adjusted_آغاز = max(vaddr_u8, Z_VIRT_REGION_آغاز_ADDR);
		uint8_t *adjusted_انتہا = min(vaddr_u8 + سائز,
					    Z_VIRT_REGION_انتہا_ADDR);
		size_t adjusted_sz = adjusted_انتہا - adjusted_آغاز;

		انحراف = virt_مقصد_bitmap_انحراف(adjusted_آغاز, adjusted_sz);
		عدد_بٹ = adjusted_sz / CONFIG_MMU_صفحہ_سائز;
		(خالی)sys_bitarray_free(&virt_region_bitmap, عدد_بٹ, انحراف);
	}
#اگر_ختم /* !CONFIG_KERNEL_DIRECT_MAP */
}

ساکن خالی *virt_region_alloc(size_t سائز, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_سائز;
	size_t انحراف;
	size_t عدد_بٹ;
	صحیح ret;

	اگر (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	عدد_بٹ = (سائز + align - CONFIG_MMU_صفحہ_سائز) / CONFIG_MMU_صفحہ_سائز;
	alloc_سائز = عدد_بٹ * CONFIG_MMU_صفحہ_سائز;
	ret = sys_bitarray_alloc(&virt_region_bitmap, عدد_بٹ, &انحراف);
	اگر (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			سائز);
		واپس تہی;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_انحراف(انحراف, alloc_سائز);

	اگر (alloc_سائز > سائز) {
		uintptr_t aligned_dest_addr = باری_اوپر(dest_addr, align);

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
		virt_region_free(UINT_مقصد_اشارہ_گر(dest_addr),
				 aligned_dest_addr - dest_addr);
		اگر (((dest_addr + alloc_سائز) - (aligned_dest_addr + سائز)) > 0) {
			virt_region_free(UINT_مقصد_اشارہ_گر(aligned_dest_addr + سائز),
					 (dest_addr + alloc_سائز) - (aligned_dest_addr + سائز));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	اگر (dest_addr < اشارہ_گر_مقصد_UINT(Z_VIRT_REGION_آغاز_ADDR)) {
		(خالی)sys_bitarray_free(&virt_region_bitmap, عدد_بٹ, انحراف);
		واپس تہی;
	}

	واپس UINT_مقصد_اشارہ_گر(dest_addr);
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
BUILD_ASSERT(SYS_SFLIST_FLAG_بٹ >= 1, "MMU free-page list needs one sflist flag bit");

ساکن sys_sflist_t free_صفحہ_چوکھٹا_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
ساکن size_t z_free_صفحہ_گنتی;

#تعریف_کرو PF_ASSERT(pf, اظہاریہ, fmt, ...) \
	__ASSERT(اظہاریہ, "page frame 0x%lx: " fmt, k_mem_صفحہ_چوکھٹا_مقصد_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
ساکن ساخت k_mem_صفحہ_چوکھٹا *free_صفحہ_چوکھٹا_list_get(خالی)
{
	sys_sfnode_t *گرہ;
	ساخت k_mem_صفحہ_چوکھٹا *pf = تہی;

	گرہ = sys_sflist_get(&free_صفحہ_چوکھٹا_list);
	اگر (گرہ != تہی) {
		z_free_صفحہ_گنتی--;
		pf = CONTAINER_کا(گرہ, ساخت k_mem_صفحہ_چوکھٹا, گرہ);
		PF_ASSERT(pf, k_mem_صفحہ_چوکھٹا_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	واپس pf;
}

/* Release a page frame back into the list of free pages */
ساکن خالی free_صفحہ_چوکھٹا_list_put(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	PF_ASSERT(pf, k_mem_صفحہ_چوکھٹا_is_دستیاب(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->گرہ, K_MEM_صفحہ_چوکھٹا_FREE);
	sys_sflist_append(&free_صفحہ_چوکھٹا_list, &pf->گرہ);
	z_free_صفحہ_گنتی++;
}

ساکن خالی free_صفحہ_چوکھٹا_list_init(خالی)
{
	sys_sflist_init(&free_صفحہ_چوکھٹا_list);
}

ساکن خالی صفحہ_چوکھٹا_free_locked(ساخت k_mem_صفحہ_چوکھٹا *pf)
{
	pf->va_and_flags = 0;
	free_صفحہ_چوکھٹا_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
ساکن خالی چوکھٹا_mapped_سیٹ(ساخت k_mem_صفحہ_چوکھٹا *pf, خالی *addr)
{
	PF_ASSERT(pf, !k_mem_صفحہ_چوکھٹا_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_صفحہ_چوکھٹا_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_صفحہ_چوکھٹا_is_mapped(pf) || k_mem_صفحہ_چوکھٹا_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_صفحہ_چوکھٹا_مقصد_virt(pf));

	uintptr_t flags_نقاب = CONFIG_MMU_صفحہ_سائز - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_نقاب;

	pf->va_and_flags &= flags_نقاب;
	pf->va_and_flags |= va | K_MEM_صفحہ_چوکھٹا_MAPPED;
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
ساکن صحیح virt_مقصد_صفحہ_چوکھٹا(خالی *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	صحیح ret = -EFAULT;

	K_MEM_صفحہ_چوکھٹا_FOREACH(paddr, pf) {
		اگر (k_mem_صفحہ_چوکھٹا_is_mapped(pf)) {
			اگر (virt == k_mem_صفحہ_چوکھٹا_مقصد_virt(pf)) {
				ret = 0;
				اگر (phys != تہی) {
					*phys = k_mem_صفحہ_چوکھٹا_مقصد_phys(pf);
				}
				توڑو;
			}
		}
	}

	واپس ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_مقصد_صفحہ_چوکھٹا, arch_صفحہ_phys_get, صحیح);

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
ساکن صحیح صفحہ_چوکھٹا_prepare_locked(ساخت k_mem_صفحہ_چوکھٹا *pf, bool *dirty_ptr,
				     bool صفحہ_اندر, uintptr_t *location_ptr);

ساکن اندرونی خالی do_backing_store_صفحہ_اندر(uintptr_t location);
ساکن اندرونی خالی do_backing_store_صفحہ_برآمد(uintptr_t location);
#اگر_ختم /* CONFIG_DEMAND_PAGING */

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
ساکن صحیح map_anon_صفحہ(خالی *addr, uint32_t flags, bool *recovered)
{
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	uintptr_t phys;
	bool تالا = (flags & K_MEM_MAP_تالا) != 0U;
	صحیح ret;

	*recovered = true;

	pf = free_صفحہ_چوکھٹا_list_get();
	اگر (pf == تہی) {
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		اگر (pf == تہی) {
			/* Every evictable page frame is pinned or busy */
			واپس -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_صفحہ_چوکھٹا_مقصد_virt(pf),
			k_mem_صفحہ_چوکھٹا_مقصد_phys(pf));
		ret = صفحہ_چوکھٹا_prepare_locked(pf, &dirty, false, &location);
		اگر (ret != 0) {
			واپس -ENOMEM;
		}
		اگر (dirty) {
			do_backing_store_صفحہ_برآمد(location);
		}
		pf->va_and_flags = 0;
#ورنہ
		واپس -ENOMEM;
#اگر_ختم /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_صفحہ_چوکھٹا_مقصد_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_صفحہ_سائز, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	اگر (ret != 0) {
#اگر_تعریف_شدہ CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		صحیح unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_صفحہ_سائز);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		اگر (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			واپس ret;
		}
#اگر_ختم

		/* Mapping failed: give the page frame back before bailing out. */
		صفحہ_چوکھٹا_free_locked(pf);
		واپس ret;
	}

	اگر (تالا) {
		k_mem_صفحہ_چوکھٹا_سیٹ(pf, K_MEM_صفحہ_چوکھٹا_PINNED);
	}
	چوکھٹا_mapped_سیٹ(pf, addr);
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
	اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!تالا)) {
		k_mem_paging_eviction_جوڑو(pf);
	}
#اگر_ختم

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	واپس 0;
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
ساکن صحیح unmap_anon_صفحات(خالی *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	صحیح ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
		شمار arch_صفحہ_location حالت_2;
		uintptr_t location;

		حالت_2 = arch_صفحہ_location_get(pos, &location);
		انتخاب (حالت_2) {
		حالت ARCH_صفحہ_LOCATION_PAGED_برآمد:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_صفحہ_سائز);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			اگر (ret != 0) {
				جاؤ unmap_anon_برآمد;
			}

			جاری;
		حالت ARCH_صفحہ_LOCATION_PAGED_اندر:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			توڑو;
		طےشدہ:
			ret = arch_صفحہ_phys_get(pos, &phys);
			توڑو;
		}
#ورنہ
		ret = arch_صفحہ_phys_get(pos, &phys);
#اگر_ختم
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		اگر (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			جاؤ unmap_anon_برآمد;
		}

		__ASSERT(k_mem_is_صفحہ_چوکھٹا(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		اگر (!k_mem_is_صفحہ_چوکھٹا(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			جاؤ unmap_anon_برآمد;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_مقصد_صفحہ_چوکھٹا(phys);

		__ASSERT(k_mem_صفحہ_چوکھٹا_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		اگر (!k_mem_صفحہ_چوکھٹا_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			جاؤ unmap_anon_برآمد;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_صفحہ_سائز);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		اگر (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			جاؤ unmap_anon_برآمد;
		}

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
		اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_صفحہ_چوکھٹا_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#اگر_ختم

		/* Put the page frame back into free list */
		صفحہ_چوکھٹا_free_locked(pf);
	}

unmap_anon_برآمد:
	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_DEMAND_MAPPING
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
ساکن bool undo_demand_mapped_صفحات(خالی *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	صحیح ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_صفحہ_سائز);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		اگر (ret != 0) {
			success = false;
		}
	}

	واپس success;
}
#اگر_ختم

خالی *k_mem_map_phys_guard(uintptr_t phys, size_t سائز, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t کل_سائز;
	صحیح ret;
	k_spinlock_کلید_t کلید;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && صفحہ_چوکھٹے_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_نقاب) == 0U,
		 "%s does not support explicit cache settings", __func__);

	اگر (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		واپس تہی;
	}
	اگر ((سائز % CONFIG_MMU_صفحہ_سائز) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", سائز, __func__);
		واپس تہی;
	}
	اگر (سائز == 0) {
		LOG_ERR("zero sized memory mapping");
		واپس تہی;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	اگر (سائز_جوڑو_حد_سے_تجاوز(سائز, CONFIG_MMU_صفحہ_سائز * 2, &کل_سائز)) {
		LOG_ERR("too large size %zu passed to %s", سائز, __func__);
		واپس تہی;
	}

	کلید = k_spin_تالا(&z_mm_تالا);

	region = virt_region_alloc(کل_سائز, CONFIG_MMU_صفحہ_سائز);
	اگر (region == تہی) {
		/* Address space has no free region */
		dst = تہی;
		جاؤ برآمد;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_صفحہ_سائز);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	اگر (ret != 0) {
		dst = تہی;
		جاؤ برآمد;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_صفحہ_سائز + سائز, CONFIG_MMU_صفحہ_سائز);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	اگر (ret != 0) {
		dst = تہی;
		جاؤ برآمد;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_صفحہ_سائز;

	اگر (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#اگر_تعریف_شدہ CONFIG_DEMAND_MAPPING
		اگر ((flags & K_MEM_MAP_تالا) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, سائز, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_صفحہ_سائز, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				اگر (ret != 0) {
					size_t unmap_sz = pos - dst;

					اگر (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_صفحہ_سائز;
					}

					bool success = undo_demand_mapped_صفحات(dst, unmap_sz);

					dst = تہی;

					اگر (success) {
						جاؤ fail;
					} ورنہ {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						جاؤ برآمد;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} ورنہ
#اگر_ختم
		{
			VIRT_FOREACH(dst, سائز, pos) {
				bool recovered;

				ret = map_anon_صفحہ(pos, flags, &recovered);

				اگر (ret != 0) {
					صحیح unmap_ret = unmap_anon_صفحات(dst, pos - dst);

					dst = تہی;

					اگر ((unmap_ret == 0) && recovered) {
						جاؤ fail;
					} ورنہ {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						جاؤ برآمد;
					}
				}
			}
		}
	} ورنہ {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, سائز, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		اگر (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, سائز);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = تہی;

			اگر (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				جاؤ برآمد;
			} ورنہ {
				جاؤ fail;
			}
		}
	}

برآمد:
	k_spin_unlock(&z_mm_تالا, کلید);

	اگر (dst != تہی && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		memset(dst, 0, سائز);
	}

	واپس dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, کل_سائز);

	جاؤ برآمد;
}

خالی k_mem_unmap_phys_guard(خالی *addr, size_t سائز, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_کلید_t کلید;
	size_t کل_سائز;
	صحیح ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(اشارہ_گر_مقصد_UINT(addr) >= CONFIG_MMU_صفحہ_سائز);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_صفحہ_سائز;
	k_mem_assert_virtual_region(pos, سائز + (CONFIG_MMU_صفحہ_سائز * 2));

	کلید = k_spin_تالا(&z_mm_تالا);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_صفحہ_phys_get(pos - CONFIG_MMU_صفحہ_سائز, تہی);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, سائز);
	اگر (ret == 0) {
		جاؤ برآمد;
	}

	ret = arch_صفحہ_phys_get(pos + سائز, تہی);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, سائز);
	اگر (ret == 0) {
		جاؤ برآمد;
	}

	اگر (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_صفحات(addr, سائز);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		اگر (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			جاؤ برآمد;
		}
	} ورنہ {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, سائز);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		اگر (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			جاؤ برآمد;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_صفحہ_سائز;
	کل_سائز = سائز + (CONFIG_MMU_صفحہ_سائز * 2);
	virt_region_free(pos, کل_سائز);

برآمد:
	k_spin_unlock(&z_mm_تالا, کلید);
}

صحیح k_mem_update_flags(خالی *addr, size_t سائز, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_کلید_t کلید;
	صحیح ret;

	k_mem_assert_virtual_region(addr, سائز);

	کلید = k_spin_تالا(&z_mm_تالا);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_صفحہ_phys_get(addr, &phys);
	اگر (ret < 0) {
		جاؤ برآمد;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, سائز);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	اگر (ret != 0) {
		جاؤ برآمد;
	}

	ret = arch_mem_map(addr, phys, سائز, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

برآمد:
	k_spin_unlock(&z_mm_تالا, کلید);
	واپس ret;
}

size_t k_mem_free_get(خالی)
{
	size_t ret;
	k_spinlock_کلید_t کلید;

	__ASSERT(صفحہ_چوکھٹے_initialized, "%s called too early", __func__);

	کلید = k_spin_تالا(&z_mm_تالا);
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
	اگر (z_free_صفحہ_گنتی > CONFIG_DEMAND_PAGING_صفحہ_چوکھٹے_RESERVE) {
		ret = z_free_صفحہ_گنتی - CONFIG_DEMAND_PAGING_صفحہ_چوکھٹے_RESERVE;
	} ورنہ {
		ret = 0;
	}
#ورنہ
	ret = z_free_صفحہ_گنتی;
#اگر_ختم /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_تالا, کلید);

	واپس ret * (size_t)CONFIG_MMU_صفحہ_سائز;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
ساکن size_t virt_region_align(uintptr_t phys, size_t سائز)
{
	ARG_غیر_مستعمل(phys);
	ARG_غیر_مستعمل(سائز);

	واپس CONFIG_MMU_صفحہ_سائز;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
خالی k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t سائز, uint32_t flags)
{
	uintptr_t aligned_phys, addr_انحراف;
	size_t aligned_سائز, align_boundary;
	k_spinlock_کلید_t کلید;
	uint8_t *dest_addr;
	size_t عدد_بٹ;
	size_t انحراف;
	صحیح ret;

#اگر_غیر_تعریف_شدہ CONFIG_KERNEL_براہ_راست_MAP
	__ASSERT(!(flags & K_MEM_براہ_راست_MAP), "The direct-map is not enabled");
#اگر_ختم /* CONFIG_KERNEL_DIRECT_MAP */
	addr_انحراف = k_mem_region_align(&aligned_phys, &aligned_سائز,
					 phys, سائز,
					 CONFIG_MMU_صفحہ_سائز);
	__ASSERT(aligned_سائز != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_سائز - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_سائز);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_سائز);

	کلید = k_spin_تالا(&z_mm_تالا);

	اگر (IS_ENABLED(CONFIG_KERNEL_براہ_راست_MAP) &&
	    (flags & K_MEM_براہ_راست_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		اگر (اندر_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_آغاز,
			      (uintptr_t)(K_MEM_VIRT_RAM_انتہا - 1)) ||
		    اندر_RANGE(aligned_phys + aligned_سائز - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_آغاز,
			      (uintptr_t)(K_MEM_VIRT_RAM_انتہا - 1))) {
			uint8_t *adjusted_آغاز = max(dest_addr, K_MEM_VIRT_RAM_آغاز);
			uint8_t *adjusted_انتہا = min(dest_addr + aligned_سائز,
						    K_MEM_VIRT_RAM_انتہا);
			size_t adjusted_sz = adjusted_انتہا - adjusted_آغاز;

			عدد_بٹ = adjusted_sz / CONFIG_MMU_صفحہ_سائز;
			انحراف = virt_مقصد_bitmap_انحراف(adjusted_آغاز, adjusted_sz);
			اگر (sys_bitarray_test_and_سیٹ_region(
			    &virt_region_bitmap, عدد_بٹ, انحراف, true)) {
				جاؤ fail;
			}
		}
	} ورنہ {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_سائز, align_boundary);
		اگر (!dest_addr) {
			جاؤ fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (سائز - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, سائز);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (خالی *)dest_addr,
		aligned_phys, aligned_سائز, flags, addr_انحراف);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_سائز, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	اگر (ret != 0) {
		جاؤ fail;
	}
	k_spin_unlock(&z_mm_تالا, کلید);

	*virt_ptr = dest_addr + addr_انحراف;
	واپس;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, سائز, flags);
	k_panic();
}

خالی k_mem_unmap_phys_bare(uint8_t *virt, size_t سائز)
{
	uintptr_t aligned_virt, addr_انحراف;
	size_t aligned_سائز;
	k_spinlock_کلید_t کلید;
	صحیح ret;

	addr_انحراف = k_mem_region_align(&aligned_virt, &aligned_سائز,
					 اشارہ_گر_مقصد_UINT(virt), سائز,
					 CONFIG_MMU_صفحہ_سائز);
	__ASSERT(aligned_سائز != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_سائز - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_سائز);

	کلید = k_spin_تالا(&z_mm_تالا);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_سائز, addr_انحراف);

	ret = arch_mem_unmap(UINT_مقصد_اشارہ_گر(aligned_virt), aligned_سائز);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	اگر (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		جاؤ برآمد;
	}

	virt_region_free(UINT_مقصد_اشارہ_گر(aligned_virt), aligned_سائز);

برآمد:
	k_spin_unlock(&z_mm_تالا, کلید);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_سائز,
			  uintptr_t addr, size_t سائز, size_t align)
{
	size_t addr_انحراف;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = باری_نیچے_کو(addr, align);
	addr_انحراف = addr - *aligned_addr;
	*aligned_سائز = باری_اوپر(سائز + addr_انحراف, align);

	واپس addr_انحراف;
}

#اگر_تعریف_شدہ CONFIG_LINKER_USE_BOOT_SECTION
ساکن خالی mark_linker_section_pinned(خالی *آغاز_addr, خالی *انتہا_addr,
				       bool pin)
{
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	uint8_t *addr;

	uintptr_t pinned_آغاز = باری_نیچے_کو(اشارہ_گر_مقصد_UINT(آغاز_addr),
					    CONFIG_MMU_صفحہ_سائز);
	uintptr_t pinned_انتہا = باری_اوپر(اشارہ_گر_مقصد_UINT(انتہا_addr),
					CONFIG_MMU_صفحہ_سائز);
	size_t pinned_سائز = pinned_انتہا - pinned_آغاز;

	VIRT_FOREACH(UINT_مقصد_اشارہ_گر(pinned_آغاز), pinned_سائز, addr)
	{
		pf = k_mem_phys_مقصد_صفحہ_چوکھٹا(K_MEM_BOOT_VIRT_مقصد_PHYS(addr));
		چوکھٹا_mapped_سیٹ(pf, addr);

		اگر (pin) {
			k_mem_صفحہ_چوکھٹا_سیٹ(pf, K_MEM_صفحہ_چوکھٹا_PINNED);
		} ورنہ {
			k_mem_صفحہ_چوکھٹا_clear(pf, K_MEM_صفحہ_چوکھٹا_PINNED);
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
			اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_صفحہ_چوکھٹا_is_evictable(pf)) {
				k_mem_paging_eviction_جوڑو(pf);
			}
#اگر_ختم
		}
	}
}
#اگر_ختم /* CONFIG_LINKER_USE_BOOT_SECTION */

#اگر_تعریف_شدہ CONFIG_LINKER_USE_ONDEMAND_SECTION
ساکن خالی z_paging_ondemand_section_map(خالی)
{
	uint8_t *addr;
	size_t سائز;
	uintptr_t location;
	uint32_t flags;
	صحیح ret;

	سائز = (uintptr_t)lnkr_ondemand_متن_سائز;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_متن_آغاز, سائز, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_صفحہ_سائز, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		اگر (ret != 0) {
			k_panic();
		}

		sys_bitarray_سیٹ_region(&virt_region_bitmap, 1,
					virt_مقصد_bitmap_انحراف(addr, CONFIG_MMU_صفحہ_سائز));
	}

	سائز = (uintptr_t)lnkr_ondemand_rodata_سائز;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_آغاز, سائز, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_صفحہ_سائز, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		اگر (ret != 0) {
			k_panic();
		}

		sys_bitarray_سیٹ_region(&virt_region_bitmap, 1,
					virt_مقصد_bitmap_انحراف(addr, CONFIG_MMU_صفحہ_سائز));
	}
}
#اگر_ختم /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

خالی z_mem_manage_init(خالی)
{
	uintptr_t phys;
	uint8_t *addr;
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	k_spinlock_کلید_t کلید = k_spin_تالا(&z_mm_تالا);

	free_صفحہ_چوکھٹا_list_init();

	ARG_غیر_مستعمل(addr);

#اگر_تعریف_شدہ CONFIG_ARCH_HAS_RESERVED_صفحہ_چوکھٹے
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_صفحات_update();
#اگر_ختم /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_آغاز, K_MEM_KERNEL_VIRT_سائز, addr)
	{
		pf = k_mem_phys_مقصد_صفحہ_چوکھٹا(K_MEM_BOOT_VIRT_مقصد_PHYS(addr));
		چوکھٹا_mapped_سیٹ(pf, addr);
		k_mem_صفحہ_چوکھٹا_سیٹ(pf, K_MEM_صفحہ_چوکھٹا_PINNED);
	}

#اگر_تعریف_شدہ CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_آغاز, lnkr_boot_انتہا, true);
#اگر_ختم /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_صفحہ_چوکھٹا_FOREACH(phys, pf) {
		اگر (k_mem_صفحہ_چوکھٹا_is_دستیاب(pf)) {
			free_صفحہ_چوکھٹا_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_صفحہ_گنتی);

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_صفحہ_چوکھٹا_FOREACH(phys, pf) {
			اگر (k_mem_صفحہ_چوکھٹا_is_evictable(pf)) {
				k_mem_paging_eviction_جوڑو(pf);
			}
		}
	}
#اگر_ختم /* CONFIG_DEMAND_PAGING */

#اگر_تعریف_شدہ CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_ASSERT
	صفحہ_چوکھٹے_initialized = true;
#اگر_ختم /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_تالا, کلید);
}

خالی z_mem_manage_boot_اختتام(خالی)
{
#اگر_تعریف_شدہ CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_آغاز, lnkr_boot_انتہا, false);
#اگر_ختم /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS
ساخت k_mem_paging_stats_t paging_stats;
خارجی ساخت k_mem_paging_histogram_t z_paging_histogram_eviction;
خارجی ساخت k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحہ_اندر;
خارجی ساخت k_mem_paging_histogram_t z_paging_histogram_backing_store_صفحہ_برآمد;
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS */

ساکن اندرونی خالی do_backing_store_صفحہ_اندر(uintptr_t location)
{
#اگر_تعریف_شدہ CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	انتخاب (location) {
	حالت ARCH_UNPAGED_ANON_ZERO:
		memset(K_MEM_SCRATCH_صفحہ, 0, CONFIG_MMU_صفحہ_سائز);
		__fallthrough;
	حالت ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		واپس;
	طےشدہ:
		توڑو;
	}
#اگر_ختم /* CONFIG_DEMAND_MAPPING */

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t وقت_diff;

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t وقت_آغاز, وقت_انتہا;

	وقت_آغاز = timing_شمار_get();
#ورنہ
	uint32_t وقت_آغاز;

	وقت_آغاز = k_چکر_get_32();
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_صفحہ_اندر(location);

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	وقت_انتہا = timing_شمار_get();
	وقت_diff = (uint32_t)timing_ادوار_get(&وقت_آغاز, &وقت_انتہا);
#ورنہ
	وقت_diff = k_چکر_get_32() - وقت_آغاز;
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_صفحہ_اندر,
			       وقت_diff);
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

ساکن اندرونی خالی do_backing_store_صفحہ_برآمد(uintptr_t location)
{
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t وقت_diff;

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t وقت_آغاز, وقت_انتہا;

	وقت_آغاز = timing_شمار_get();
#ورنہ
	uint32_t وقت_آغاز;

	وقت_آغاز = k_چکر_get_32();
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_صفحہ_برآمد(location);

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	وقت_انتہا = timing_شمار_get();
	وقت_diff = (uint32_t)timing_ادوار_get(&وقت_آغاز, &وقت_انتہا);
#ورنہ
	وقت_diff = k_چکر_get_32() - وقت_آغاز;
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_صفحہ_برآمد,
			       وقت_diff);
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#اگر defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
ساکن K_MUTEX_DEFINE(z_mm_paging_تالا);
#اگر_ختم

ساکن خالی virt_region_foreach(خالی *addr, size_t سائز,
				خالی (*func)(خالی *vaddr))
{
	k_mem_assert_virtual_region(addr, سائز);

	برائے (size_t انحراف = 0; انحراف < سائز; انحراف += CONFIG_MMU_صفحہ_سائز) {
		func((uint8_t *)addr + انحراف);
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
ساکن صحیح صفحہ_چوکھٹا_prepare_locked(ساخت k_mem_صفحہ_چوکھٹا *pf, bool *dirty_ptr,
				     bool صفحہ_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	صحیح ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_صفحہ_چوکھٹا_مقصد_phys(pf);
	__ASSERT(!k_mem_صفحہ_چوکھٹا_is_pinned(pf), "page frame 0x%lx is pinned",
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
	اگر (k_mem_صفحہ_چوکھٹا_is_mapped(pf)) {
		dirty = dirty || !k_mem_صفحہ_چوکھٹا_is_backed(pf);
	}

	اگر (dirty || صفحہ_fault) {
		arch_mem_scratch(phys);
	}

	اگر (k_mem_صفحہ_چوکھٹا_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      صفحہ_fault);
		اگر (ret != 0) {
			LOG_ERR("out of backing store memory");
			واپس -ENOMEM;
		}
		arch_mem_صفحہ_برآمد(k_mem_صفحہ_چوکھٹا_مقصد_virt(pf), *location_ptr);

		اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} ورنہ {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_صفحہ_چوکھٹا_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_صفحہ_چوکھٹا_سیٹ(pf, K_MEM_صفحہ_چوکھٹا_BUSY);
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	واپس 0;
}

ساکن صحیح do_mem_evict(خالی *addr)
{
	bool dirty;
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	uintptr_t location;
	k_spinlock_کلید_t کلید;
	uintptr_t flags, phys;
	صحیح ret;

#اگر CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_اندر_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#اگر_تعریف_شدہ CONFIG_SMP
	k_mutex_تالا(&z_mm_paging_تالا, K_FOREVER);
#ورنہ
	k_sched_تالا();
#اگر_ختم
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	کلید = k_spin_تالا(&z_mm_تالا);
	flags = arch_صفحہ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_ڈیٹا_صفحہ_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	اگر ((flags & ARCH_ڈیٹا_صفحہ_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		جاؤ برآمد;
	}

	dirty = (flags & ARCH_ڈیٹا_صفحہ_DIRTY) != 0;
	pf = k_mem_phys_مقصد_صفحہ_چوکھٹا(phys);
	__ASSERT(k_mem_صفحہ_چوکھٹا_مقصد_virt(pf) == addr, "page frame address mismatch");
	ret = صفحہ_چوکھٹا_prepare_locked(pf, &dirty, false, &location);
	اگر (ret != 0) {
		جاؤ برآمد;
	}

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_تالا, کلید);
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	اگر (dirty) {
		do_backing_store_صفحہ_برآمد(location);
	}
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلید = k_spin_تالا(&z_mm_تالا);
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	صفحہ_چوکھٹا_free_locked(pf);
برآمد:
	k_spin_unlock(&z_mm_تالا, کلید);
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#اگر_تعریف_شدہ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_تالا);
#ورنہ
	k_sched_unlock();
#اگر_ختم
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	واپس ret;
}

صحیح k_mem_صفحہ_برآمد(خالی *addr, size_t سائز)
{
	__ASSERT(صفحہ_چوکھٹے_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, سائز);

	برائے (size_t انحراف = 0; انحراف < سائز; انحراف += CONFIG_MMU_صفحہ_سائز) {
		خالی *pos = (uint8_t *)addr + انحراف;
		صحیح ret;

		ret = do_mem_evict(pos);
		اگر (ret != 0) {
			واپس ret;
		}
	}

	واپس 0;
}

صحیح k_mem_صفحہ_چوکھٹا_evict(uintptr_t phys)
{
	k_spinlock_کلید_t کلید;
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	صحیح ret;

	__ASSERT(صفحہ_چوکھٹے_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_اندر_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#اگر_تعریف_شدہ CONFIG_SMP
	k_mutex_تالا(&z_mm_paging_تالا, K_FOREVER);
#ورنہ
	k_sched_تالا();
#اگر_ختم
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	کلید = k_spin_تالا(&z_mm_تالا);
	pf = k_mem_phys_مقصد_صفحہ_چوکھٹا(phys);
	اگر (!k_mem_صفحہ_چوکھٹا_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		جاؤ برآمد;
	}
	flags = arch_صفحہ_info_get(k_mem_صفحہ_چوکھٹا_مقصد_virt(pf), تہی, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_ڈیٹا_صفحہ_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_ڈیٹا_صفحہ_DIRTY) != 0;
	ret = صفحہ_چوکھٹا_prepare_locked(pf, &dirty, false, &location);
	اگر (ret != 0) {
		جاؤ برآمد;
	}

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_تالا, کلید);
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	اگر (dirty) {
		do_backing_store_صفحہ_برآمد(location);
	}
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلید = k_spin_تالا(&z_mm_تالا);
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	صفحہ_چوکھٹا_free_locked(pf);
برآمد:
	k_spin_unlock(&z_mm_تالا, کلید);
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#اگر_تعریف_شدہ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_تالا);
#ورنہ
	k_sched_unlock();
#اگر_ختم
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	واپس ret;
}

ساکن اندرونی خالی paging_stats_خرابیاں_inc(ساخت k_thread *faulting_thread,
					   صحیح کلید)
{
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(کلید);

	paging_stats.pagefaults.cnt++;

	اگر (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} ورنہ {
		paging_stats.pagefaults.irq_locked++;
	}

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	اگر (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} ورنہ {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#ورنہ
	ARG_غیر_مستعمل(faulting_thread);
#اگر_ختم /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#اگر_غیر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	اگر (k_is_اندر_isr()) {
		paging_stats.pagefaults.اندر_isr++;

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.اندر_isr++;
#اگر_ختم /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS */
}

ساکن اندرونی خالی paging_stats_eviction_inc(ساخت k_thread *faulting_thread,
					     bool dirty)
{
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS
	اگر (dirty) {
		paging_stats.eviction.dirty++;
	} ورنہ {
		paging_stats.eviction.clean++;
	}
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_THREAD_STATS
	اگر (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} ورنہ {
		faulting_thread->paging_stats.eviction.clean++;
	}
#ورنہ
	ARG_غیر_مستعمل(faulting_thread);
#اگر_ختم /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS */
}

ساکن اندرونی ساخت k_mem_صفحہ_چوکھٹا *do_eviction_select(bool *dirty)
{
	ساخت k_mem_صفحہ_چوکھٹا *pf;

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t وقت_diff;

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t وقت_آغاز, وقت_انتہا;

	وقت_آغاز = timing_شمار_get();
#ورنہ
	uint32_t وقت_آغاز;

	وقت_آغاز = k_چکر_get_32();
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	وقت_انتہا = timing_شمار_get();
	وقت_diff = (uint32_t)timing_ادوار_get(&وقت_آغاز, &وقت_انتہا);
#ورنہ
	وقت_diff = k_چکر_get_32() - وقت_آغاز;
#اگر_ختم /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, وقت_diff);
#اگر_ختم /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	واپس pf;
}

ساکن bool do_صفحہ_fault(خالی *addr, bool pin)
{
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	k_spinlock_کلید_t کلید;
	uintptr_t صفحہ_اندر_location, صفحہ_برآمد_location;
	شمار arch_صفحہ_location حالت_2;
	bool نتیجہ;
	bool dirty = false;
	ساخت k_thread *faulting_thread;
	صحیح ret;

	__ASSERT(صفحہ_چوکھٹے_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
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
	__ASSERT(!k_is_اندر_isr(), "ISR page faults are forbidden");
#اگر_تعریف_شدہ CONFIG_SMP
	k_mutex_تالا(&z_mm_paging_تالا, K_FOREVER);
#ورنہ
	k_sched_تالا();
#اگر_ختم
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	کلید = k_spin_تالا(&z_mm_تالا);
	faulting_thread = _current;

	حالت_2 = arch_صفحہ_location_get(addr, &صفحہ_اندر_location);
	اگر (حالت_2 == ARCH_صفحہ_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		نتیجہ = false;
		جاؤ برآمد;
	}
	نتیجہ = true;

	اگر (حالت_2 == ARCH_صفحہ_LOCATION_PAGED_اندر) {
		اگر (pin) {
			/* It's a physical memory address */
			uintptr_t phys = صفحہ_اندر_location;

			pf = k_mem_phys_مقصد_صفحہ_چوکھٹا(phys);
			اگر (!k_mem_صفحہ_چوکھٹا_is_pinned(pf)) {
				اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_صفحہ_چوکھٹا_سیٹ(pf, K_MEM_صفحہ_چوکھٹا_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		جاؤ برآمد;
	}
	__ASSERT(حالت_2 == ARCH_صفحہ_LOCATION_PAGED_برآمد,
		 "unexpected status value %d", حالت_2);

	paging_stats_خرابیاں_inc(faulting_thread, کلید.کلید);

	pf = free_صفحہ_چوکھٹا_list_get();
	اگر (pf == تہی) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		اگر (pf == تہی) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			نتیجہ = false;
			جاؤ برآمد;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_صفحہ_چوکھٹا_مقصد_virt(pf),
			k_mem_صفحہ_چوکھٹا_مقصد_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = صفحہ_چوکھٹا_prepare_locked(pf, &dirty, true, &صفحہ_برآمد_location);
	اگر (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		نتیجہ = false;
		جاؤ برآمد;
	}

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_تالا, کلید);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	اگر (dirty) {
		do_backing_store_صفحہ_برآمد(صفحہ_برآمد_location);
	}
	do_backing_store_صفحہ_اندر(صفحہ_اندر_location);

#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلید = k_spin_تالا(&z_mm_تالا);
	k_mem_صفحہ_چوکھٹا_clear(pf, K_MEM_صفحہ_چوکھٹا_BUSY);
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_صفحہ_چوکھٹا_clear(pf, K_MEM_صفحہ_چوکھٹا_MAPPED);
	چوکھٹا_mapped_سیٹ(pf, addr);
	اگر (pin) {
		k_mem_صفحہ_چوکھٹا_سیٹ(pf, K_MEM_صفحہ_چوکھٹا_PINNED);
	}

	arch_mem_صفحہ_اندر(addr, k_mem_صفحہ_چوکھٹا_مقصد_phys(pf));
	k_mem_paging_backing_store_صفحہ_finalize(pf, صفحہ_اندر_location);
	اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_جوڑو(pf);
	}
برآمد:
	k_spin_unlock(&z_mm_تالا, کلید);
#اگر_تعریف_شدہ CONFIG_DEMAND_PAGING_ALLOW_IRQ
#اگر_تعریف_شدہ CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_تالا);
#ورنہ
	k_sched_unlock();
#اگر_ختم
#اگر_ختم /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	واپس نتیجہ;
}

ساکن خالی do_صفحہ_اندر(خالی *addr)
{
	bool ret;

	ret = do_صفحہ_fault(addr, false);
	اگر (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

خالی k_mem_صفحہ_اندر(خالی *addr, size_t سائز)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_اندر_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, سائز, do_صفحہ_اندر);
}

ساکن خالی do_mem_pin(خالی *addr)
{
	bool ret;

	ret = do_صفحہ_fault(addr, true);
	اگر (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

خالی k_mem_pin(خالی *addr, size_t سائز)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_اندر_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, سائز, do_mem_pin);
}

bool k_mem_صفحہ_fault(خالی *addr)
{
	واپس do_صفحہ_fault(addr, false);
}

ساکن خالی do_mem_unpin(خالی *addr)
{
	ساخت k_mem_صفحہ_چوکھٹا *pf;
	k_spinlock_کلید_t کلید;
	uintptr_t flags, phys;

	کلید = k_spin_تالا(&z_mm_تالا);
	flags = arch_صفحہ_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_ڈیٹا_صفحہ_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	اگر ((flags & ARCH_ڈیٹا_صفحہ_LOADED) != 0) {
		pf = k_mem_phys_مقصد_صفحہ_چوکھٹا(phys);
		اگر (k_mem_صفحہ_چوکھٹا_is_pinned(pf)) {
			k_mem_صفحہ_چوکھٹا_clear(pf, K_MEM_صفحہ_چوکھٹا_PINNED);

			اگر (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_جوڑو(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_تالا, کلید);
}

خالی k_mem_unpin(خالی *addr, size_t سائز)
{
	__ASSERT(صفحہ_چوکھٹے_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, سائز, do_mem_unpin);
}

#اگر_ختم /* CONFIG_DEMAND_PAGING */
