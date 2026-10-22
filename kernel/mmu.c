/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#ضمن <stdint.h>
#ضمن <kernel_arch_interface.h>
#ضمن <zephyr/spinlock.h>
#ضمن <mmu.h>
#ضمن <zephyr/init.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/sys/bitarray.h>
#ضمن <zephyr/sys/check.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/minmax.h>
#ضمن <zephyr/timing/timing.h>
#ضمن <zephyr/arch/common/init.h>
#ضمن <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#که_تعریف_شوی CONFIG_DEMAND_PAGING
#ضمن <zephyr/kernel/mm/demand_paging.h>
#د_که_پای /* CONFIG_DEMAND_PAGING */

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
بنية k_spinlock z_mm_قلف;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
بنية k_mem_پاڼه_چوکاټ k_mem_پاڼه_چوکاټونه[K_MEM_عدد_پاڼه_چوکاټونه];

#که_تعریف_شوی CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
ساكن bool پاڼه_چوکاټونه_initialized;
#د_که_پای /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#تعریف_کړه COLOR_پاڼه_چوکاټونه	1

#إذا COLOR_پاڼه_چوکاټونه
#تعریف_کړه ANSI_DEFAULT "\x1B" "[0m"
#تعریف_کړه ANSI_RED     "\x1B" "[1;31m"
#تعریف_کړه ANSI_GREEN   "\x1B" "[1;32m"
#تعریف_کړه ANSI_YELLOW  "\x1B" "[1;33m"
#تعریف_کړه ANSI_BLUE    "\x1B" "[1;34m"
#تعریف_کړه ANSI_MAGENTA "\x1B" "[1;35m"
#تعریف_کړه ANSI_CYAN    "\x1B" "[1;36m"
#تعریف_کړه ANSI_GREY    "\x1B" "[1;90m"

#تعریف_کړه COLOR(x)	printk(_CONCAT(ANSI_, x))
#وإلا
#تعریف_کړه COLOR(x)	افعل { } طالما (false)
#د_که_پای /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
ساكن فراغ پاڼه_چوکاټ_dump(بنية k_mem_پاڼه_چوکاټ *pf)
{
	إذا (k_mem_پاڼه_چوکاټ_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} وإلا إذا (k_mem_پاڼه_چوکاټ_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} وإلا إذا (k_mem_پاڼه_چوکاټ_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} وإلا إذا (k_mem_پاڼه_چوکاټ_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} وإلا إذا (k_mem_پاڼه_چوکاټ_is_شته(pf)) {
		COLOR(GREY);
		printk(".");
	} وإلا إذا (k_mem_پاڼه_چوکاټ_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} وإلا {
		COLOR(RED);
		printk("?");
	}
}

فراغ k_mem_پاڼه_چوکاټونه_dump(فراغ)
{
	صحيح ستنه = 0;

	__ASSERT(پاڼه_چوکاټونه_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_پیل, K_MEM_PHYS_RAM_پای_ټکی);

	لكل (صحيح i = 0; i < K_MEM_عدد_پاڼه_چوکاټونه; i++) {
		بنية k_mem_پاڼه_چوکاټ *pf = &k_mem_پاڼه_چوکاټونه[i];

		پاڼه_چوکاټ_dump(pf);

		ستنه++;
		إذا (ستنه == 64) {
			ستنه = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	إذا (ستنه != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#تعریف_کړه VIRT_FOREACH(بنسټ_2, اندازه_2, _pos) \
	لكل ((_pos) = (بنسټ_2); \
	     (_pos) < ((uint8_t *)(بنسټ_2) + (اندازه_2)); (_pos) += CONFIG_MMU_پاڼه_اندازه)

#تعریف_کړه PHYS_FOREACH(بنسټ_2, اندازه_2, _pos) \
	لكل ((_pos) = (بنسټ_2); \
	     (_pos) < ((uintptr_t)(بنسټ_2) + (اندازه_2)); (_pos) += CONFIG_MMU_پاڼه_اندازه)


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
			   CONFIG_KERNEL_VM_اندازه / CONFIG_MMU_پاڼه_اندازه);

ساكن bool virt_region_inited;

#تعریف_کړه Z_VIRT_REGION_پیل_ADDR	K_MEM_VM_FREE_پیل
#تعریف_کړه Z_VIRT_REGION_پای_ټکی_ADDR		(K_MEM_VIRT_RAM_پای_ټکی - K_MEM_VM_RESERVED)

ساكن مضمن uintptr_t virt_from_bitmap_دننه_واټن(size_t دننه_واټن, size_t اندازه)
{
	أرجع اشاره_ګر_منزل_UINT(K_MEM_VIRT_RAM_پای_ټکی)
	       - (دننه_واټن * CONFIG_MMU_پاڼه_اندازه) - اندازه;
}

ساكن مضمن size_t virt_منزل_bitmap_دننه_واټن(فراغ *vaddr, size_t اندازه)
{
	أرجع (اشاره_ګر_منزل_UINT(K_MEM_VIRT_RAM_پای_ټکی)
		- اشاره_ګر_منزل_UINT(vaddr) - اندازه) / CONFIG_MMU_پاڼه_اندازه;
}

ساكن فراغ virt_region_init(فراغ)
{
	size_t دننه_واټن, عدد_بټونه;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	إذا (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		عدد_بټونه = K_MEM_VM_RESERVED / CONFIG_MMU_پاڼه_اندازه;
		(فراغ)sys_bitarray_سېټ_region(&virt_region_bitmap,
					      عدد_بټونه, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	عدد_بټونه = اشاره_ګر_منزل_UINT(K_MEM_VM_FREE_پیل)
		   - اشاره_ګر_منزل_UINT(K_MEM_VIRT_RAM_پیل);
	دننه_واټن = virt_منزل_bitmap_دننه_واټن(K_MEM_VIRT_RAM_پیل, عدد_بټونه);
	عدد_بټونه /= CONFIG_MMU_پاڼه_اندازه;
	(فراغ)sys_bitarray_سېټ_region(&virt_region_bitmap,
				      عدد_بټونه, دننه_واټن);

	virt_region_inited = true;
}

ساكن فراغ virt_region_free(فراغ *vaddr, size_t اندازه)
{
	size_t دننه_واټن, عدد_بټونه;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	إذا (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#که_نه_تعریف_شوی CONFIG_KERNEL_مستقیم_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_پیل_ADDR)
		 && ((vaddr_u8 + اندازه - 1) < Z_VIRT_REGION_پای_ټکی_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, اندازه);
	إذا (!((vaddr_u8 >= Z_VIRT_REGION_پیل_ADDR)
	      && ((vaddr_u8 + اندازه - 1) < Z_VIRT_REGION_پای_ټکی_ADDR))) {
		أرجع;
	}

	دننه_واټن = virt_منزل_bitmap_دننه_واټن(vaddr, اندازه);
	عدد_بټونه = اندازه / CONFIG_MMU_پاڼه_اندازه;
	(فراغ)sys_bitarray_free(&virt_region_bitmap, عدد_بټونه, دننه_واټن);
#وإلا /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	إذا (((vaddr_u8 >= Z_VIRT_REGION_پیل_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_پای_ټکی_ADDR)) ||
	    (((vaddr_u8 + اندازه - 1) >= Z_VIRT_REGION_پیل_ADDR) &&
	     ((vaddr_u8 + اندازه - 1) < Z_VIRT_REGION_پای_ټکی_ADDR))) {
		uint8_t *adjusted_پیل = max(vaddr_u8, Z_VIRT_REGION_پیل_ADDR);
		uint8_t *adjusted_پای_ټکی = min(vaddr_u8 + اندازه,
					    Z_VIRT_REGION_پای_ټکی_ADDR);
		size_t adjusted_sz = adjusted_پای_ټکی - adjusted_پیل;

		دننه_واټن = virt_منزل_bitmap_دننه_واټن(adjusted_پیل, adjusted_sz);
		عدد_بټونه = adjusted_sz / CONFIG_MMU_پاڼه_اندازه;
		(فراغ)sys_bitarray_free(&virt_region_bitmap, عدد_بټونه, دننه_واټن);
	}
#د_که_پای /* !CONFIG_KERNEL_DIRECT_MAP */
}

ساكن فراغ *virt_region_alloc(size_t اندازه, size_t align)
{
	uintptr_t dest_addr;
	size_t alloc_اندازه;
	size_t دننه_واټن;
	size_t عدد_بټونه;
	صحيح ret;

	إذا (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	عدد_بټونه = (اندازه + align - CONFIG_MMU_پاڼه_اندازه) / CONFIG_MMU_پاڼه_اندازه;
	alloc_اندازه = عدد_بټونه * CONFIG_MMU_پاڼه_اندازه;
	ret = sys_bitarray_alloc(&virt_region_bitmap, عدد_بټونه, &دننه_واټن);
	إذا (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			اندازه);
		أرجع عدم;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_دننه_واټن(دننه_واټن, alloc_اندازه);

	إذا (alloc_اندازه > اندازه) {
		uintptr_t aligned_dest_addr = پړاو_پورته(dest_addr, align);

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
		virt_region_free(UINT_منزل_اشاره_ګر(dest_addr),
				 aligned_dest_addr - dest_addr);
		إذا (((dest_addr + alloc_اندازه) - (aligned_dest_addr + اندازه)) > 0) {
			virt_region_free(UINT_منزل_اشاره_ګر(aligned_dest_addr + اندازه),
					 (dest_addr + alloc_اندازه) - (aligned_dest_addr + اندازه));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	إذا (dest_addr < اشاره_ګر_منزل_UINT(Z_VIRT_REGION_پیل_ADDR)) {
		(فراغ)sys_bitarray_free(&virt_region_bitmap, عدد_بټونه, دننه_واټن);
		أرجع عدم;
	}

	أرجع UINT_منزل_اشاره_ګر(dest_addr);
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
BUILD_ASSERT(SYS_SFLIST_FLAG_بټونه >= 1, "MMU free-page list needs one sflist flag bit");

ساكن sys_sflist_t free_پاڼه_چوکاټ_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
ساكن size_t z_free_پاڼه_شمېر;

#تعریف_کړه PF_ASSERT(pf, بیان, fmt, ...) \
	__ASSERT(بیان, "page frame 0x%lx: " fmt, k_mem_پاڼه_چوکاټ_منزل_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
ساكن بنية k_mem_پاڼه_چوکاټ *free_پاڼه_چوکاټ_list_get(فراغ)
{
	sys_sfnode_t *غوټه;
	بنية k_mem_پاڼه_چوکاټ *pf = عدم;

	غوټه = sys_sflist_get(&free_پاڼه_چوکاټ_list);
	إذا (غوټه != عدم) {
		z_free_پاڼه_شمېر--;
		pf = CONTAINER_د(غوټه, بنية k_mem_پاڼه_چوکاټ, غوټه);
		PF_ASSERT(pf, k_mem_پاڼه_چوکاټ_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	أرجع pf;
}

/* Release a page frame back into the list of free pages */
ساكن فراغ free_پاڼه_چوکاټ_list_put(بنية k_mem_پاڼه_چوکاټ *pf)
{
	PF_ASSERT(pf, k_mem_پاڼه_چوکاټ_is_شته(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->غوټه, K_MEM_پاڼه_چوکاټ_FREE);
	sys_sflist_append(&free_پاڼه_چوکاټ_list, &pf->غوټه);
	z_free_پاڼه_شمېر++;
}

ساكن فراغ free_پاڼه_چوکاټ_list_init(فراغ)
{
	sys_sflist_init(&free_پاڼه_چوکاټ_list);
}

ساكن فراغ پاڼه_چوکاټ_free_locked(بنية k_mem_پاڼه_چوکاټ *pf)
{
	pf->va_and_flags = 0;
	free_پاڼه_چوکاټ_list_put(pf);
}

/*
 * Memory Mapping
 */

/* Called after the frame is mapped in the arch layer, to update our
 * local ontology (and do some assertions while we're at it)
 */
ساكن فراغ چوکاټ_mapped_سېټ(بنية k_mem_پاڼه_چوکاټ *pf, فراغ *addr)
{
	PF_ASSERT(pf, !k_mem_پاڼه_چوکاټ_is_free(pf),
		  "attempted to map a page frame on the free list");
	PF_ASSERT(pf, !k_mem_پاڼه_چوکاټ_is_reserved(pf),
		  "attempted to map a reserved page frame");

	/* We do allow multiple mappings for pinned page frames
	 * since we will never need to reverse map them.
	 * This is uncommon, use-cases are for things like the
	 * Zephyr equivalent of VSDOs
	 */
	PF_ASSERT(pf, !k_mem_پاڼه_چوکاټ_is_mapped(pf) || k_mem_پاڼه_چوکاټ_is_pinned(pf),
		 "non-pinned and already mapped to %p",
		 k_mem_پاڼه_چوکاټ_منزل_virt(pf));

	uintptr_t flags_ماسک = CONFIG_MMU_پاڼه_اندازه - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_ماسک;

	pf->va_and_flags &= flags_ماسک;
	pf->va_and_flags |= va | K_MEM_پاڼه_چوکاټ_MAPPED;
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
ساكن صحيح virt_منزل_پاڼه_چوکاټ(فراغ *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	بنية k_mem_پاڼه_چوکاټ *pf;
	صحيح ret = -EFAULT;

	K_MEM_پاڼه_چوکاټ_FOREACH(paddr, pf) {
		إذا (k_mem_پاڼه_چوکاټ_is_mapped(pf)) {
			إذا (virt == k_mem_پاڼه_چوکاټ_منزل_virt(pf)) {
				ret = 0;
				إذا (phys != عدم) {
					*phys = k_mem_پاڼه_چوکاټ_منزل_phys(pf);
				}
				اكسر;
			}
		}
	}

	أرجع ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_منزل_پاڼه_چوکاټ, arch_پاڼه_phys_get, صحيح);

#که_تعریف_شوی CONFIG_DEMAND_PAGING
ساكن صحيح پاڼه_چوکاټ_prepare_locked(بنية k_mem_پاڼه_چوکاټ *pf, bool *dirty_ptr,
				     bool پاڼه_دننه, uintptr_t *location_ptr);

ساكن مضمن فراغ do_backing_store_پاڼه_دننه(uintptr_t location);
ساكن مضمن فراغ do_backing_store_پاڼه_بهر(uintptr_t location);
#د_که_پای /* CONFIG_DEMAND_PAGING */

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
ساكن صحيح map_anon_پاڼه(فراغ *addr, uint32_t flags, bool *recovered)
{
	بنية k_mem_پاڼه_چوکاټ *pf;
	uintptr_t phys;
	bool قلف = (flags & K_MEM_MAP_قلف) != 0U;
	صحيح ret;

	*recovered = true;

	pf = free_پاڼه_چوکاټ_list_get();
	إذا (pf == عدم) {
#که_تعریف_شوی CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		إذا (pf == عدم) {
			/* Every evictable page frame is pinned or busy */
			أرجع -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_پاڼه_چوکاټ_منزل_virt(pf),
			k_mem_پاڼه_چوکاټ_منزل_phys(pf));
		ret = پاڼه_چوکاټ_prepare_locked(pf, &dirty, false, &location);
		إذا (ret != 0) {
			أرجع -ENOMEM;
		}
		إذا (dirty) {
			do_backing_store_پاڼه_بهر(location);
		}
		pf->va_and_flags = 0;
#وإلا
		أرجع -ENOMEM;
#د_که_پای /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_پاڼه_چوکاټ_منزل_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_پاڼه_اندازه, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	إذا (ret != 0) {
#که_تعریف_شوی CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		صحيح unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_پاڼه_اندازه);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		إذا (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			أرجع ret;
		}
#د_که_پای

		/* Mapping failed: give the page frame back before bailing out. */
		پاڼه_چوکاټ_free_locked(pf);
		أرجع ret;
	}

	إذا (قلف) {
		k_mem_پاڼه_چوکاټ_سېټ(pf, K_MEM_پاڼه_چوکاټ_PINNED);
	}
	چوکاټ_mapped_سېټ(pf, addr);
#که_تعریف_شوی CONFIG_DEMAND_PAGING
	إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!قلف)) {
		k_mem_paging_eviction_جمع_کړه(pf);
	}
#د_که_پای

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	أرجع 0;
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
ساكن صحيح unmap_anon_پاڼې(فراغ *virt, size_t sz)
{
	uint8_t *pos;
	uintptr_t phys;
	بنية k_mem_پاڼه_چوکاټ *pf;
	صحيح ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#که_تعریف_شوی CONFIG_DEMAND_PAGING
		تعداد arch_پاڼه_location حالت;
		uintptr_t location;

		حالت = arch_پاڼه_location_get(pos, &location);
		بدل (حالت) {
		حالة ARCH_پاڼه_LOCATION_PAGED_بهر:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_پاڼه_اندازه);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			إذا (ret != 0) {
				اذهب unmap_anon_بهر;
			}

			استمر;
		حالة ARCH_پاڼه_LOCATION_PAGED_دننه:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			اكسر;
		افتراضي:
			ret = arch_پاڼه_phys_get(pos, &phys);
			اكسر;
		}
#وإلا
		ret = arch_پاڼه_phys_get(pos, &phys);
#د_که_پای
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		إذا (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			اذهب unmap_anon_بهر;
		}

		__ASSERT(k_mem_is_پاڼه_چوکاټ(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		إذا (!k_mem_is_پاڼه_چوکاټ(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			اذهب unmap_anon_بهر;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_منزل_پاڼه_چوکاټ(phys);

		__ASSERT(k_mem_پاڼه_چوکاټ_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		إذا (!k_mem_پاڼه_چوکاټ_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			اذهب unmap_anon_بهر;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_پاڼه_اندازه);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		إذا (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			اذهب unmap_anon_بهر;
		}

#که_تعریف_شوی CONFIG_DEMAND_PAGING
		إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_پاڼه_چوکاټ_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#د_که_پای

		/* Put the page frame back into free list */
		پاڼه_چوکاټ_free_locked(pf);
	}

unmap_anon_بهر:
	أرجع ret;
}

#که_تعریف_شوی CONFIG_DEMAND_MAPPING
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
ساكن bool undo_demand_mapped_پاڼې(فراغ *virt, size_t sz)
{
	uint8_t *undo_pos;
	bool success = true;
	صحيح ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_پاڼه_اندازه);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		إذا (ret != 0) {
			success = false;
		}
	}

	أرجع success;
}
#د_که_پای

فراغ *k_mem_map_phys_guard(uintptr_t phys, size_t اندازه, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	size_t ټول_اندازه;
	صحيح ret;
	k_spinlock_کلی_t کلی;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && پاڼه_چوکاټونه_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_ماسک) == 0U,
		 "%s does not support explicit cache settings", __func__);

	إذا (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		أرجع عدم;
	}
	إذا ((اندازه % CONFIG_MMU_پاڼه_اندازه) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", اندازه, __func__);
		أرجع عدم;
	}
	إذا (اندازه == 0) {
		LOG_ERR("zero sized memory mapping");
		أرجع عدم;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	إذا (اندازه_جمع_کړه_ډکېدنه(اندازه, CONFIG_MMU_پاڼه_اندازه * 2, &ټول_اندازه)) {
		LOG_ERR("too large size %zu passed to %s", اندازه, __func__);
		أرجع عدم;
	}

	کلی = k_spin_قلف(&z_mm_قلف);

	region = virt_region_alloc(ټول_اندازه, CONFIG_MMU_پاڼه_اندازه);
	إذا (region == عدم) {
		/* Address space has no free region */
		dst = عدم;
		اذهب بهر;
	}

	/* Unmap both guard pages to make sure accessing them
	 * will generate fault.
	 *
	 * Note that a failed unmap may have left the guard page mapped in
	 * some memory domains, so the virtual region cannot be freed for
	 * reuse.
	 */
	ret = arch_mem_unmap(region, CONFIG_MMU_پاڼه_اندازه);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	إذا (ret != 0) {
		dst = عدم;
		اذهب بهر;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_پاڼه_اندازه + اندازه, CONFIG_MMU_پاڼه_اندازه);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	إذا (ret != 0) {
		dst = عدم;
		اذهب بهر;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_پاڼه_اندازه;

	إذا (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#که_تعریف_شوی CONFIG_DEMAND_MAPPING
		إذا ((flags & K_MEM_MAP_قلف) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, اندازه, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_پاڼه_اندازه, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				إذا (ret != 0) {
					size_t unmap_sz = pos - dst;

					إذا (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_پاڼه_اندازه;
					}

					bool success = undo_demand_mapped_پاڼې(dst, unmap_sz);

					dst = عدم;

					إذا (success) {
						اذهب fail;
					} وإلا {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						اذهب بهر;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} وإلا
#د_که_پای
		{
			VIRT_FOREACH(dst, اندازه, pos) {
				bool recovered;

				ret = map_anon_پاڼه(pos, flags, &recovered);

				إذا (ret != 0) {
					صحيح unmap_ret = unmap_anon_پاڼې(dst, pos - dst);

					dst = عدم;

					إذا ((unmap_ret == 0) && recovered) {
						اذهب fail;
					} وإلا {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						اذهب بهر;
					}
				}
			}
		}
	} وإلا {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, اندازه, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		إذا (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, اندازه);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = عدم;

			إذا (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				اذهب بهر;
			} وإلا {
				اذهب fail;
			}
		}
	}

بهر:
	k_spin_unlock(&z_mm_قلف, کلی);

	إذا (dst != عدم && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		memset(dst, 0, اندازه);
	}

	أرجع dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, ټول_اندازه);

	اذهب بهر;
}

فراغ k_mem_unmap_phys_guard(فراغ *addr, size_t اندازه, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_کلی_t کلی;
	size_t ټول_اندازه;
	صحيح ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(اشاره_ګر_منزل_UINT(addr) >= CONFIG_MMU_پاڼه_اندازه);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_پاڼه_اندازه;
	k_mem_assert_virtual_region(pos, اندازه + (CONFIG_MMU_پاڼه_اندازه * 2));

	کلی = k_spin_قلف(&z_mm_قلف);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_پاڼه_phys_get(pos - CONFIG_MMU_پاڼه_اندازه, عدم);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, اندازه);
	إذا (ret == 0) {
		اذهب بهر;
	}

	ret = arch_پاڼه_phys_get(pos + اندازه, عدم);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, اندازه);
	إذا (ret == 0) {
		اذهب بهر;
	}

	إذا (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_پاڼې(addr, اندازه);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		إذا (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			اذهب بهر;
		}
	} وإلا {
		/*
		 * Unmapping previous mapped memory with specific physical address.
		 *
		 * Note that we don't have to unmap the guard pages, as they should
		 * have been unmapped. We just need to unmapped the in-between
		 * region [addr, (addr + size)).
		 */
		ret = arch_mem_unmap(addr, اندازه);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		إذا (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			اذهب بهر;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_پاڼه_اندازه;
	ټول_اندازه = اندازه + (CONFIG_MMU_پاڼه_اندازه * 2);
	virt_region_free(pos, ټول_اندازه);

بهر:
	k_spin_unlock(&z_mm_قلف, کلی);
}

صحيح k_mem_update_flags(فراغ *addr, size_t اندازه, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_کلی_t کلی;
	صحيح ret;

	k_mem_assert_virtual_region(addr, اندازه);

	کلی = k_spin_قلف(&z_mm_قلف);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_پاڼه_phys_get(addr, &phys);
	إذا (ret < 0) {
		اذهب بهر;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, اندازه);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	إذا (ret != 0) {
		اذهب بهر;
	}

	ret = arch_mem_map(addr, phys, اندازه, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

بهر:
	k_spin_unlock(&z_mm_قلف, کلی);
	أرجع ret;
}

size_t k_mem_free_get(فراغ)
{
	size_t ret;
	k_spinlock_کلی_t کلی;

	__ASSERT(پاڼه_چوکاټونه_initialized, "%s called too early", __func__);

	کلی = k_spin_قلف(&z_mm_قلف);
#که_تعریف_شوی CONFIG_DEMAND_PAGING
	إذا (z_free_پاڼه_شمېر > CONFIG_DEMAND_PAGING_پاڼه_چوکاټونه_RESERVE) {
		ret = z_free_پاڼه_شمېر - CONFIG_DEMAND_PAGING_پاڼه_چوکاټونه_RESERVE;
	} وإلا {
		ret = 0;
	}
#وإلا
	ret = z_free_پاڼه_شمېر;
#د_که_پای /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_قلف, کلی);

	أرجع ret * (size_t)CONFIG_MMU_پاڼه_اندازه;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
ساكن size_t virt_region_align(uintptr_t phys, size_t اندازه)
{
	ARG_ناکارول_شوی(phys);
	ARG_ناکارول_شوی(اندازه);

	أرجع CONFIG_MMU_پاڼه_اندازه;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, size_t);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
فراغ k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, size_t اندازه, uint32_t flags)
{
	uintptr_t aligned_phys, addr_دننه_واټن;
	size_t aligned_اندازه, align_boundary;
	k_spinlock_کلی_t کلی;
	uint8_t *dest_addr;
	size_t عدد_بټونه;
	size_t دننه_واټن;
	صحيح ret;

#که_نه_تعریف_شوی CONFIG_KERNEL_مستقیم_MAP
	__ASSERT(!(flags & K_MEM_مستقیم_MAP), "The direct-map is not enabled");
#د_که_پای /* CONFIG_KERNEL_DIRECT_MAP */
	addr_دننه_واټن = k_mem_region_align(&aligned_phys, &aligned_اندازه,
					 phys, اندازه,
					 CONFIG_MMU_پاڼه_اندازه);
	__ASSERT(aligned_اندازه != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_اندازه - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_اندازه);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_اندازه);

	کلی = k_spin_قلف(&z_mm_قلف);

	إذا (IS_ENABLED(CONFIG_KERNEL_مستقیم_MAP) &&
	    (flags & K_MEM_مستقیم_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		إذا (دننه_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_پیل,
			      (uintptr_t)(K_MEM_VIRT_RAM_پای_ټکی - 1)) ||
		    دننه_RANGE(aligned_phys + aligned_اندازه - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_پیل,
			      (uintptr_t)(K_MEM_VIRT_RAM_پای_ټکی - 1))) {
			uint8_t *adjusted_پیل = max(dest_addr, K_MEM_VIRT_RAM_پیل);
			uint8_t *adjusted_پای_ټکی = min(dest_addr + aligned_اندازه,
						    K_MEM_VIRT_RAM_پای_ټکی);
			size_t adjusted_sz = adjusted_پای_ټکی - adjusted_پیل;

			عدد_بټونه = adjusted_sz / CONFIG_MMU_پاڼه_اندازه;
			دننه_واټن = virt_منزل_bitmap_دننه_واټن(adjusted_پیل, adjusted_sz);
			إذا (sys_bitarray_test_and_سېټ_region(
			    &virt_region_bitmap, عدد_بټونه, دننه_واټن, true)) {
				اذهب fail;
			}
		}
	} وإلا {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_اندازه, align_boundary);
		إذا (!dest_addr) {
			اذهب fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (اندازه - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, اندازه);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (فراغ *)dest_addr,
		aligned_phys, aligned_اندازه, flags, addr_دننه_واټن);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_اندازه, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	إذا (ret != 0) {
		اذهب fail;
	}
	k_spin_unlock(&z_mm_قلف, کلی);

	*virt_ptr = dest_addr + addr_دننه_واټن;
	أرجع;
fail:
	/* May re-visit this in the future, but for now running out of
	 * virtual address space or failing the arch_mem_map() call is
	 * an unrecoverable situation.
	 *
	 * Other problems not related to resource exhaustion we leave as
	 * assertions since they are clearly programming mistakes.
	 */
	LOG_ERR("memory mapping 0x%lx (size %zu, flags 0x%x) failed",
		phys, اندازه, flags);
	k_panic();
}

فراغ k_mem_unmap_phys_bare(uint8_t *virt, size_t اندازه)
{
	uintptr_t aligned_virt, addr_دننه_واټن;
	size_t aligned_اندازه;
	k_spinlock_کلی_t کلی;
	صحيح ret;

	addr_دننه_واټن = k_mem_region_align(&aligned_virt, &aligned_اندازه,
					 اشاره_ګر_منزل_UINT(virt), اندازه,
					 CONFIG_MMU_پاڼه_اندازه);
	__ASSERT(aligned_اندازه != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_اندازه - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_اندازه);

	کلی = k_spin_قلف(&z_mm_قلف);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_اندازه, addr_دننه_واټن);

	ret = arch_mem_unmap(UINT_منزل_اشاره_ګر(aligned_virt), aligned_اندازه);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	إذا (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		اذهب بهر;
	}

	virt_region_free(UINT_منزل_اشاره_ګر(aligned_virt), aligned_اندازه);

بهر:
	k_spin_unlock(&z_mm_قلف, کلی);
}

/*
 * Miscellaneous
 */

size_t k_mem_region_align(uintptr_t *aligned_addr, size_t *aligned_اندازه,
			  uintptr_t addr, size_t اندازه, size_t align)
{
	size_t addr_دننه_واټن;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = پړاو_ښکته(addr, align);
	addr_دننه_واټن = addr - *aligned_addr;
	*aligned_اندازه = پړاو_پورته(اندازه + addr_دننه_واټن, align);

	أرجع addr_دننه_واټن;
}

#که_تعریف_شوی CONFIG_LINKER_USE_BOOT_SECTION
ساكن فراغ mark_linker_section_pinned(فراغ *پیل_addr, فراغ *پای_ټکی_addr,
				       bool pin)
{
	بنية k_mem_پاڼه_چوکاټ *pf;
	uint8_t *addr;

	uintptr_t pinned_پیل = پړاو_ښکته(اشاره_ګر_منزل_UINT(پیل_addr),
					    CONFIG_MMU_پاڼه_اندازه);
	uintptr_t pinned_پای_ټکی = پړاو_پورته(اشاره_ګر_منزل_UINT(پای_ټکی_addr),
					CONFIG_MMU_پاڼه_اندازه);
	size_t pinned_اندازه = pinned_پای_ټکی - pinned_پیل;

	VIRT_FOREACH(UINT_منزل_اشاره_ګر(pinned_پیل), pinned_اندازه, addr)
	{
		pf = k_mem_phys_منزل_پاڼه_چوکاټ(K_MEM_BOOT_VIRT_منزل_PHYS(addr));
		چوکاټ_mapped_سېټ(pf, addr);

		إذا (pin) {
			k_mem_پاڼه_چوکاټ_سېټ(pf, K_MEM_پاڼه_چوکاټ_PINNED);
		} وإلا {
			k_mem_پاڼه_چوکاټ_clear(pf, K_MEM_پاڼه_چوکاټ_PINNED);
#که_تعریف_شوی CONFIG_DEMAND_PAGING
			إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_پاڼه_چوکاټ_is_evictable(pf)) {
				k_mem_paging_eviction_جمع_کړه(pf);
			}
#د_که_پای
		}
	}
}
#د_که_پای /* CONFIG_LINKER_USE_BOOT_SECTION */

#که_تعریف_شوی CONFIG_LINKER_USE_ONDEMAND_SECTION
ساكن فراغ z_paging_ondemand_section_map(فراغ)
{
	uint8_t *addr;
	size_t اندازه;
	uintptr_t location;
	uint32_t flags;
	صحيح ret;

	اندازه = (uintptr_t)lnkr_ondemand_متن_اندازه;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_متن_پیل, اندازه, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_پاڼه_اندازه, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		إذا (ret != 0) {
			k_panic();
		}

		sys_bitarray_سېټ_region(&virt_region_bitmap, 1,
					virt_منزل_bitmap_دننه_واټن(addr, CONFIG_MMU_پاڼه_اندازه));
	}

	اندازه = (uintptr_t)lnkr_ondemand_rodata_اندازه;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_پیل, اندازه, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_پاڼه_اندازه, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		إذا (ret != 0) {
			k_panic();
		}

		sys_bitarray_سېټ_region(&virt_region_bitmap, 1,
					virt_منزل_bitmap_دننه_واټن(addr, CONFIG_MMU_پاڼه_اندازه));
	}
}
#د_که_پای /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

فراغ z_mem_manage_init(فراغ)
{
	uintptr_t phys;
	uint8_t *addr;
	بنية k_mem_پاڼه_چوکاټ *pf;
	k_spinlock_کلی_t کلی = k_spin_قلف(&z_mm_قلف);

	free_پاڼه_چوکاټ_list_init();

	ARG_ناکارول_شوی(addr);

#که_تعریف_شوی CONFIG_ARCH_HAS_RESERVED_پاڼه_چوکاټونه
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_پاڼې_update();
#د_که_پای /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_پیل, K_MEM_KERNEL_VIRT_اندازه, addr)
	{
		pf = k_mem_phys_منزل_پاڼه_چوکاټ(K_MEM_BOOT_VIRT_منزل_PHYS(addr));
		چوکاټ_mapped_سېټ(pf, addr);
		k_mem_پاڼه_چوکاټ_سېټ(pf, K_MEM_پاڼه_چوکاټ_PINNED);
	}

#که_تعریف_شوی CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_پیل, lnkr_boot_پای_ټکی, true);
#د_که_پای /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_پاڼه_چوکاټ_FOREACH(phys, pf) {
		إذا (k_mem_پاڼه_چوکاټ_is_شته(pf)) {
			free_پاڼه_چوکاټ_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_پاڼه_شمېر);

#که_تعریف_شوی CONFIG_DEMAND_PAGING
#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_پاڼه_چوکاټ_FOREACH(phys, pf) {
			إذا (k_mem_پاڼه_چوکاټ_is_evictable(pf)) {
				k_mem_paging_eviction_جمع_کړه(pf);
			}
		}
	}
#د_که_پای /* CONFIG_DEMAND_PAGING */

#که_تعریف_شوی CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#د_که_پای

#که_تعریف_شوی CONFIG_ASSERT
	پاڼه_چوکاټونه_initialized = true;
#د_که_پای /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_قلف, کلی);
}

فراغ z_mem_manage_boot_پای(فراغ)
{
#که_تعریف_شوی CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_پیل, lnkr_boot_پای_ټکی, false);
#د_که_پای /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#که_تعریف_شوی CONFIG_DEMAND_PAGING

#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS
بنية k_mem_paging_stats_t paging_stats;
خارجي بنية k_mem_paging_histogram_t z_paging_histogram_eviction;
خارجي بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_پاڼه_دننه;
خارجي بنية k_mem_paging_histogram_t z_paging_histogram_backing_store_پاڼه_بهر;
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS */

ساكن مضمن فراغ do_backing_store_پاڼه_دننه(uintptr_t location)
{
#که_تعریف_شوی CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	بدل (location) {
	حالة ARCH_UNPAGED_ANON_ZERO:
		memset(K_MEM_SCRATCH_پاڼه, 0, CONFIG_MMU_پاڼه_اندازه);
		__fallthrough;
	حالة ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		أرجع;
	افتراضي:
		اكسر;
	}
#د_که_پای /* CONFIG_DEMAND_MAPPING */

#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t وخت_diff;

#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t وخت_پیل, وخت_پای_ټکی;

	وخت_پیل = timing_counter_get();
#وإلا
	uint32_t وخت_پیل;

	وخت_پیل = k_دوره_get_32();
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_پاڼه_دننه(location);

#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	وخت_پای_ټکی = timing_counter_get();
	وخت_diff = (uint32_t)timing_دورې_get(&وخت_پیل, &وخت_پای_ټکی);
#وإلا
	وخت_diff = k_دوره_get_32() - وخت_پیل;
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_پاڼه_دننه,
			       وخت_diff);
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

ساكن مضمن فراغ do_backing_store_پاڼه_بهر(uintptr_t location)
{
#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t وخت_diff;

#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t وخت_پیل, وخت_پای_ټکی;

	وخت_پیل = timing_counter_get();
#وإلا
	uint32_t وخت_پیل;

	وخت_پیل = k_دوره_get_32();
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_پاڼه_بهر(location);

#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	وخت_پای_ټکی = timing_counter_get();
	وخت_diff = (uint32_t)timing_دورې_get(&وخت_پیل, &وخت_پای_ټکی);
#وإلا
	وخت_diff = k_دوره_get_32() - وخت_پیل;
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_پاڼه_بهر,
			       وخت_diff);
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#إذا defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
ساكن K_MUTEX_DEFINE(z_mm_paging_قلف);
#د_که_پای

ساكن فراغ virt_region_foreach(فراغ *addr, size_t اندازه,
				فراغ (*func)(فراغ *vaddr))
{
	k_mem_assert_virtual_region(addr, اندازه);

	لكل (size_t دننه_واټن = 0; دننه_واټن < اندازه; دننه_واټن += CONFIG_MMU_پاڼه_اندازه) {
		func((uint8_t *)addr + دننه_واټن);
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
ساكن صحيح پاڼه_چوکاټ_prepare_locked(بنية k_mem_پاڼه_چوکاټ *pf, bool *dirty_ptr,
				     bool پاڼه_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	صحيح ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_پاڼه_چوکاټ_منزل_phys(pf);
	__ASSERT(!k_mem_پاڼه_چوکاټ_is_pinned(pf), "page frame 0x%lx is pinned",
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
	إذا (k_mem_پاڼه_چوکاټ_is_mapped(pf)) {
		dirty = dirty || !k_mem_پاڼه_چوکاټ_is_backed(pf);
	}

	إذا (dirty || پاڼه_fault) {
		arch_mem_scratch(phys);
	}

	إذا (k_mem_پاڼه_چوکاټ_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      پاڼه_fault);
		إذا (ret != 0) {
			LOG_ERR("out of backing store memory");
			أرجع -ENOMEM;
		}
		arch_mem_پاڼه_بهر(k_mem_پاڼه_چوکاټ_منزل_virt(pf), *location_ptr);

		إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} وإلا {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_پاڼه_چوکاټ_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_پاڼه_چوکاټ_سېټ(pf, K_MEM_پاڼه_چوکاټ_BUSY);
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	أرجع 0;
}

ساكن صحيح do_mem_evict(فراغ *addr)
{
	bool dirty;
	بنية k_mem_پاڼه_چوکاټ *pf;
	uintptr_t location;
	k_spinlock_کلی_t کلی;
	uintptr_t flags, phys;
	صحيح ret;

#إذا CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_دننه_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#که_تعریف_شوی CONFIG_SMP
	k_mutex_قلف(&z_mm_paging_قلف, K_FOREVER);
#وإلا
	k_sched_قلف();
#د_که_پای
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	کلی = k_spin_قلف(&z_mm_قلف);
	flags = arch_پاڼه_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_معلومات_پاڼه_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	إذا ((flags & ARCH_معلومات_پاڼه_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		اذهب بهر;
	}

	dirty = (flags & ARCH_معلومات_پاڼه_DIRTY) != 0;
	pf = k_mem_phys_منزل_پاڼه_چوکاټ(phys);
	__ASSERT(k_mem_پاڼه_چوکاټ_منزل_virt(pf) == addr, "page frame address mismatch");
	ret = پاڼه_چوکاټ_prepare_locked(pf, &dirty, false, &location);
	إذا (ret != 0) {
		اذهب بهر;
	}

#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_قلف, کلی);
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	إذا (dirty) {
		do_backing_store_پاڼه_بهر(location);
	}
#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلی = k_spin_قلف(&z_mm_قلف);
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	پاڼه_چوکاټ_free_locked(pf);
بهر:
	k_spin_unlock(&z_mm_قلف, کلی);
#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
#که_تعریف_شوی CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_قلف);
#وإلا
	k_sched_unlock();
#د_که_پای
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	أرجع ret;
}

صحيح k_mem_پاڼه_بهر(فراغ *addr, size_t اندازه)
{
	__ASSERT(پاڼه_چوکاټونه_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, اندازه);

	لكل (size_t دننه_واټن = 0; دننه_واټن < اندازه; دننه_واټن += CONFIG_MMU_پاڼه_اندازه) {
		فراغ *pos = (uint8_t *)addr + دننه_واټن;
		صحيح ret;

		ret = do_mem_evict(pos);
		إذا (ret != 0) {
			أرجع ret;
		}
	}

	أرجع 0;
}

صحيح k_mem_پاڼه_چوکاټ_evict(uintptr_t phys)
{
	k_spinlock_کلی_t کلی;
	بنية k_mem_پاڼه_چوکاټ *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	صحيح ret;

	__ASSERT(پاڼه_چوکاټونه_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_دننه_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#که_تعریف_شوی CONFIG_SMP
	k_mutex_قلف(&z_mm_paging_قلف, K_FOREVER);
#وإلا
	k_sched_قلف();
#د_که_پای
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	کلی = k_spin_قلف(&z_mm_قلف);
	pf = k_mem_phys_منزل_پاڼه_چوکاټ(phys);
	إذا (!k_mem_پاڼه_چوکاټ_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		اذهب بهر;
	}
	flags = arch_پاڼه_info_get(k_mem_پاڼه_چوکاټ_منزل_virt(pf), عدم, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_معلومات_پاڼه_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_معلومات_پاڼه_DIRTY) != 0;
	ret = پاڼه_چوکاټ_prepare_locked(pf, &dirty, false, &location);
	إذا (ret != 0) {
		اذهب بهر;
	}

#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_قلف, کلی);
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	إذا (dirty) {
		do_backing_store_پاڼه_بهر(location);
	}
#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلی = k_spin_قلف(&z_mm_قلف);
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	پاڼه_چوکاټ_free_locked(pf);
بهر:
	k_spin_unlock(&z_mm_قلف, کلی);
#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
#که_تعریف_شوی CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_قلف);
#وإلا
	k_sched_unlock();
#د_که_پای
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	أرجع ret;
}

ساكن مضمن فراغ paging_stats_تېروتنې_inc(بنية k_thread *faulting_thread,
					   صحيح کلی)
{
#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(کلی);

	paging_stats.pagefaults.cnt++;

	إذا (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} وإلا {
		paging_stats.pagefaults.irq_locked++;
	}

#که_تعریف_شوی CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	إذا (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} وإلا {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#وإلا
	ARG_ناکارول_شوی(faulting_thread);
#د_که_پای /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#که_نه_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	إذا (k_is_دننه_isr()) {
		paging_stats.pagefaults.دننه_isr++;

#که_تعریف_شوی CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.دننه_isr++;
#د_که_پای /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS */
}

ساكن مضمن فراغ paging_stats_eviction_inc(بنية k_thread *faulting_thread,
					     bool dirty)
{
#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS
	إذا (dirty) {
		paging_stats.eviction.dirty++;
	} وإلا {
		paging_stats.eviction.clean++;
	}
#که_تعریف_شوی CONFIG_DEMAND_PAGING_THREAD_STATS
	إذا (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} وإلا {
		faulting_thread->paging_stats.eviction.clean++;
	}
#وإلا
	ARG_ناکارول_شوی(faulting_thread);
#د_که_پای /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS */
}

ساكن مضمن بنية k_mem_پاڼه_چوکاټ *do_eviction_select(bool *dirty)
{
	بنية k_mem_پاڼه_چوکاټ *pf;

#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t وخت_diff;

#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t وخت_پیل, وخت_پای_ټکی;

	وخت_پیل = timing_counter_get();
#وإلا
	uint32_t وخت_پیل;

	وخت_پیل = k_دوره_get_32();
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#که_تعریف_شوی CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	وخت_پای_ټکی = timing_counter_get();
	وخت_diff = (uint32_t)timing_دورې_get(&وخت_پیل, &وخت_پای_ټکی);
#وإلا
	وخت_diff = k_دوره_get_32() - وخت_پیل;
#د_که_پای /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, وخت_diff);
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	أرجع pf;
}

ساكن bool do_پاڼه_fault(فراغ *addr, bool pin)
{
	بنية k_mem_پاڼه_چوکاټ *pf;
	k_spinlock_کلی_t کلی;
	uintptr_t پاڼه_دننه_location, پاڼه_بهر_location;
	تعداد arch_پاڼه_location حالت;
	bool پایله;
	bool dirty = false;
	بنية k_thread *faulting_thread;
	صحيح ret;

	__ASSERT(پاڼه_چوکاټونه_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
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
	__ASSERT(!k_is_دننه_isr(), "ISR page faults are forbidden");
#که_تعریف_شوی CONFIG_SMP
	k_mutex_قلف(&z_mm_paging_قلف, K_FOREVER);
#وإلا
	k_sched_قلف();
#د_که_پای
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	کلی = k_spin_قلف(&z_mm_قلف);
	faulting_thread = _current;

	حالت = arch_پاڼه_location_get(addr, &پاڼه_دننه_location);
	إذا (حالت == ARCH_پاڼه_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		پایله = false;
		اذهب بهر;
	}
	پایله = true;

	إذا (حالت == ARCH_پاڼه_LOCATION_PAGED_دننه) {
		إذا (pin) {
			/* It's a physical memory address */
			uintptr_t phys = پاڼه_دننه_location;

			pf = k_mem_phys_منزل_پاڼه_چوکاټ(phys);
			إذا (!k_mem_پاڼه_چوکاټ_is_pinned(pf)) {
				إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_پاڼه_چوکاټ_سېټ(pf, K_MEM_پاڼه_چوکاټ_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		اذهب بهر;
	}
	__ASSERT(حالت == ARCH_پاڼه_LOCATION_PAGED_بهر,
		 "unexpected status value %d", حالت);

	paging_stats_تېروتنې_inc(faulting_thread, کلی.کلی);

	pf = free_پاڼه_چوکاټ_list_get();
	إذا (pf == عدم) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		إذا (pf == عدم) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			پایله = false;
			اذهب بهر;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_پاڼه_چوکاټ_منزل_virt(pf),
			k_mem_پاڼه_چوکاټ_منزل_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = پاڼه_چوکاټ_prepare_locked(pf, &dirty, true, &پاڼه_بهر_location);
	إذا (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		پایله = false;
		اذهب بهر;
	}

#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_قلف, کلی);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	إذا (dirty) {
		do_backing_store_پاڼه_بهر(پاڼه_بهر_location);
	}
	do_backing_store_پاڼه_دننه(پاڼه_دننه_location);

#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
	کلی = k_spin_قلف(&z_mm_قلف);
	k_mem_پاڼه_چوکاټ_clear(pf, K_MEM_پاڼه_چوکاټ_BUSY);
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_پاڼه_چوکاټ_clear(pf, K_MEM_پاڼه_چوکاټ_MAPPED);
	چوکاټ_mapped_سېټ(pf, addr);
	إذا (pin) {
		k_mem_پاڼه_چوکاټ_سېټ(pf, K_MEM_پاڼه_چوکاټ_PINNED);
	}

	arch_mem_پاڼه_دننه(addr, k_mem_پاڼه_چوکاټ_منزل_phys(pf));
	k_mem_paging_backing_store_پاڼه_finalize(pf, پاڼه_دننه_location);
	إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_جمع_کړه(pf);
	}
بهر:
	k_spin_unlock(&z_mm_قلف, کلی);
#که_تعریف_شوی CONFIG_DEMAND_PAGING_ALLOW_IRQ
#که_تعریف_شوی CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_قلف);
#وإلا
	k_sched_unlock();
#د_که_پای
#د_که_پای /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	أرجع پایله;
}

ساكن فراغ do_پاڼه_دننه(فراغ *addr)
{
	bool ret;

	ret = do_پاڼه_fault(addr, false);
	إذا (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

فراغ k_mem_پاڼه_دننه(فراغ *addr, size_t اندازه)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_دننه_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, اندازه, do_پاڼه_دننه);
}

ساكن فراغ do_mem_pin(فراغ *addr)
{
	bool ret;

	ret = do_پاڼه_fault(addr, true);
	إذا (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

فراغ k_mem_pin(فراغ *addr, size_t اندازه)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_دننه_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, اندازه, do_mem_pin);
}

bool k_mem_پاڼه_fault(فراغ *addr)
{
	أرجع do_پاڼه_fault(addr, false);
}

ساكن فراغ do_mem_unpin(فراغ *addr)
{
	بنية k_mem_پاڼه_چوکاټ *pf;
	k_spinlock_کلی_t کلی;
	uintptr_t flags, phys;

	کلی = k_spin_قلف(&z_mm_قلف);
	flags = arch_پاڼه_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_معلومات_پاڼه_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	إذا ((flags & ARCH_معلومات_پاڼه_LOADED) != 0) {
		pf = k_mem_phys_منزل_پاڼه_چوکاټ(phys);
		إذا (k_mem_پاڼه_چوکاټ_is_pinned(pf)) {
			k_mem_پاڼه_چوکاټ_clear(pf, K_MEM_پاڼه_چوکاټ_PINNED);

			إذا (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_جمع_کړه(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_قلف, کلی);
}

فراغ k_mem_unpin(فراغ *addr, size_t اندازه)
{
	__ASSERT(پاڼه_چوکاټونه_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, اندازه, do_mem_unpin);
}

#د_که_پای /* CONFIG_DEMAND_PAGING */
