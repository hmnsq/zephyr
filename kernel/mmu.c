/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Routines for managing virtual address spaces
 */

#समावेश <stdint.h>
#समावेश <kernel_arch_interface.h>
#समावेश <zephyr/spinlock.h>
#समावेश <mmu.h>
#समावेश <zephyr/init.h>
#समावेश <kernel_internal.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/linker/linker-defs.h>
#समावेश <zephyr/sys/bitarray.h>
#समावेश <zephyr/sys/check.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/sys/minmax.h>
#समावेश <zephyr/timing/timing.h>
#समावेश <zephyr/arch/common/init.h>
#समावेश <zephyr/logging/log.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#अगर_घोषित CONFIG_DEMAND_PAGING
#समावेश <zephyr/kernel/mm/demand_paging.h>
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING */

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
काष्ठा k_spinlock z_mm_lock;

/*
 * General page frame management
 */

/* Database of all RAM page frames */
काष्ठा k_mem_page_frame k_mem_page_frames[K_MEM_अंक_PAGE_FRAMES];

#अगर_घोषित CONFIG_ASSERT
/* Indicator that k_mem_page_frames has been initialized, many of these APIs do
 * not work before POST_KERNEL
 */
अटल bool page_frames_initialized;
#पूर्ण_अगर /* CONFIG_ASSERT */

/* Add colors to page table dumps to indicate mapping type */
#घोषणा COLOR_PAGE_FRAMES	1

#अगर COLOR_PAGE_FRAMES
#घोषणा ANSI_DEFAULT "\x1B" "[0m"
#घोषणा ANSI_RED     "\x1B" "[1;31m"
#घोषणा ANSI_GREEN   "\x1B" "[1;32m"
#घोषणा ANSI_YELLOW  "\x1B" "[1;33m"
#घोषणा ANSI_BLUE    "\x1B" "[1;34m"
#घोषणा ANSI_MAGENTA "\x1B" "[1;35m"
#घोषणा ANSI_CYAN    "\x1B" "[1;36m"
#घोषणा ANSI_GREY    "\x1B" "[1;90m"

#घोषणा COLOR(x)	printk(_CONCAT(ANSI_, x))
#अन्यथा
#घोषणा COLOR(x)	करो { } जबतक (false)
#पूर्ण_अगर /* COLOR_PAGE_FRAMES */

/* LCOV_EXCL_START */
अटल व्योम page_frame_dump(काष्ठा k_mem_page_frame *pf)
{
	अगर (k_mem_page_frame_is_free(pf)) {
		COLOR(GREY);
		printk("-");
	} अन्यथा अगर (k_mem_page_frame_is_reserved(pf)) {
		COLOR(CYAN);
		printk("R");
	} अन्यथा अगर (k_mem_page_frame_is_busy(pf)) {
		COLOR(MAGENTA);
		printk("B");
	} अन्यथा अगर (k_mem_page_frame_is_pinned(pf)) {
		COLOR(YELLOW);
		printk("P");
	} अन्यथा अगर (k_mem_page_frame_is_available(pf)) {
		COLOR(GREY);
		printk(".");
	} अन्यथा अगर (k_mem_page_frame_is_mapped(pf)) {
		COLOR(DEFAULT);
		printk("M");
	} अन्यथा {
		COLOR(RED);
		printk("?");
	}
}

व्योम k_mem_page_frames_dump(व्योम)
{
	पूर्णांक column = 0;

	__ASSERT(page_frames_initialized, "%s called too early", __func__);
	printk("Physical memory from 0x%lx to 0x%lx\n",
	       K_MEM_PHYS_RAM_START, K_MEM_PHYS_RAM_END);

	क्रम (पूर्णांक i = 0; i < K_MEM_अंक_PAGE_FRAMES; i++) {
		काष्ठा k_mem_page_frame *pf = &k_mem_page_frames[i];

		page_frame_dump(pf);

		column++;
		अगर (column == 64) {
			column = 0;
			printk("\n");
		}
	}

	COLOR(DEFAULT);
	अगर (column != 0) {
		printk("\n");
	}
}
/* LCOV_EXCL_STOP */

#घोषणा VIRT_FOREACH(_base, आकार_2, _pos) \
	क्रम ((_pos) = (_base); \
	     (_pos) < ((uint8_t *)(_base) + (आकार_2)); (_pos) += CONFIG_MMU_PAGE_आकार)

#घोषणा PHYS_FOREACH(_base, आकार_2, _pos) \
	क्रम ((_pos) = (_base); \
	     (_pos) < ((uintptr_t)(_base) + (आकार_2)); (_pos) += CONFIG_MMU_PAGE_आकार)


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

अटल bool virt_region_inited;

#घोषणा Z_VIRT_REGION_START_ADDR	K_MEM_VM_FREE_START
#घोषणा Z_VIRT_REGION_END_ADDR		(K_MEM_VIRT_RAM_END - K_MEM_VM_RESERVED)

अटल अंतरभूत uintptr_t virt_from_bitmap_offset(माप_प्रकार offset, माप_प्रकार आकार)
{
	वापस POINTER_TO_UINT(K_MEM_VIRT_RAM_END)
	       - (offset * CONFIG_MMU_PAGE_आकार) - आकार;
}

अटल अंतरभूत माप_प्रकार virt_to_bitmap_offset(व्योम *vaddr, माप_प्रकार आकार)
{
	वापस (POINTER_TO_UINT(K_MEM_VIRT_RAM_END)
		- POINTER_TO_UINT(vaddr) - आकार) / CONFIG_MMU_PAGE_आकार;
}

अटल व्योम virt_region_init(व्योम)
{
	माप_प्रकार offset, अंक_bits;

	/* There are regions where we should never map via
	 * k_mem_map() and k_mem_map_phys_bare(). Mark them as
	 * already allocated so they will never be used.
	 */

	अगर (K_MEM_VM_RESERVED > 0) {
		/* Mark reserved region at end of virtual address space */
		अंक_bits = K_MEM_VM_RESERVED / CONFIG_MMU_PAGE_आकार;
		(व्योम)sys_bitarray_set_region(&virt_region_bitmap,
					      अंक_bits, 0);
	}

	/* Mark all bits up to Z_FREE_VM_START as allocated */
	अंक_bits = POINTER_TO_UINT(K_MEM_VM_FREE_START)
		   - POINTER_TO_UINT(K_MEM_VIRT_RAM_START);
	offset = virt_to_bitmap_offset(K_MEM_VIRT_RAM_START, अंक_bits);
	अंक_bits /= CONFIG_MMU_PAGE_आकार;
	(व्योम)sys_bitarray_set_region(&virt_region_bitmap,
				      अंक_bits, offset);

	virt_region_inited = true;
}

अटल व्योम virt_region_free(व्योम *vaddr, माप_प्रकार आकार)
{
	माप_प्रकार offset, अंक_bits;
	uint8_t *vaddr_u8 = (uint8_t *)vaddr;

	अगर (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

#अगर_अघोषित CONFIG_KERNEL_DIRECT_MAP
	/* Without the need to support K_MEM_DIRECT_MAP, the region must be
	 * able to be represented in the bitmap. So this case is
	 * simple.
	 */

	__ASSERT((vaddr_u8 >= Z_VIRT_REGION_START_ADDR)
		 && ((vaddr_u8 + आकार - 1) < Z_VIRT_REGION_END_ADDR),
		 "invalid virtual address region %p (%zu)", vaddr_u8, आकार);
	अगर (!((vaddr_u8 >= Z_VIRT_REGION_START_ADDR)
	      && ((vaddr_u8 + आकार - 1) < Z_VIRT_REGION_END_ADDR))) {
		वापस;
	}

	offset = virt_to_bitmap_offset(vaddr, आकार);
	अंक_bits = आकार / CONFIG_MMU_PAGE_आकार;
	(व्योम)sys_bitarray_free(&virt_region_bitmap, अंक_bits, offset);
#अन्यथा /* !CONFIG_KERNEL_DIRECT_MAP */
	/* With K_MEM_DIRECT_MAP, the region can be outside of the virtual
	 * memory space, wholly within it, or overlap partially.
	 * So additional processing is needed to make sure we only
	 * mark the pages within the bitmap.
	 */
	अगर (((vaddr_u8 >= Z_VIRT_REGION_START_ADDR) &&
	     (vaddr_u8 < Z_VIRT_REGION_END_ADDR)) ||
	    (((vaddr_u8 + आकार - 1) >= Z_VIRT_REGION_START_ADDR) &&
	     ((vaddr_u8 + आकार - 1) < Z_VIRT_REGION_END_ADDR))) {
		uint8_t *adjusted_start = max(vaddr_u8, Z_VIRT_REGION_START_ADDR);
		uint8_t *adjusted_end = min(vaddr_u8 + आकार,
					    Z_VIRT_REGION_END_ADDR);
		माप_प्रकार adjusted_sz = adjusted_end - adjusted_start;

		offset = virt_to_bitmap_offset(adjusted_start, adjusted_sz);
		अंक_bits = adjusted_sz / CONFIG_MMU_PAGE_आकार;
		(व्योम)sys_bitarray_free(&virt_region_bitmap, अंक_bits, offset);
	}
#पूर्ण_अगर /* !CONFIG_KERNEL_DIRECT_MAP */
}

अटल व्योम *virt_region_alloc(माप_प्रकार आकार, माप_प्रकार align)
{
	uintptr_t dest_addr;
	माप_प्रकार alloc_आकार;
	माप_प्रकार offset;
	माप_प्रकार अंक_bits;
	पूर्णांक ret;

	अगर (unlikely(!virt_region_inited)) {
		virt_region_init();
	}

	/* Possibly request more pages to ensure we can get an aligned virtual address */
	अंक_bits = (आकार + align - CONFIG_MMU_PAGE_आकार) / CONFIG_MMU_PAGE_आकार;
	alloc_आकार = अंक_bits * CONFIG_MMU_PAGE_आकार;
	ret = sys_bitarray_alloc(&virt_region_bitmap, अंक_bits, &offset);
	अगर (ret != 0) {
		LOG_ERR("insufficient virtual address space (requested %zu)",
			आकार);
		वापस शून्य;
	}

	/* Remember that bit #0 in bitmap corresponds to the highest
	 * virtual address. So here we need to go downwards (backwards?)
	 * to get the starting address of the allocated region.
	 */
	dest_addr = virt_from_bitmap_offset(offset, alloc_आकार);

	अगर (alloc_आकार > आकार) {
		uintptr_t aligned_dest_addr = ROUND_UP(dest_addr, align);

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
		virt_region_free(UINT_TO_POINTER(dest_addr),
				 aligned_dest_addr - dest_addr);
		अगर (((dest_addr + alloc_आकार) - (aligned_dest_addr + आकार)) > 0) {
			virt_region_free(UINT_TO_POINTER(aligned_dest_addr + आकार),
					 (dest_addr + alloc_आकार) - (aligned_dest_addr + आकार));
		}

		dest_addr = aligned_dest_addr;
	}

	/* Need to make sure this does not step into kernel memory */
	अगर (dest_addr < POINTER_TO_UINT(Z_VIRT_REGION_START_ADDR)) {
		(व्योम)sys_bitarray_free(&virt_region_bitmap, अंक_bits, offset);
		वापस शून्य;
	}

	वापस UINT_TO_POINTER(dest_addr);
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
BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "MMU free-page list needs one sflist flag bit");

अटल sys_sflist_t free_page_frame_list;

/* Number of unused and available free page frames.
 * This information may go stale immediately.
 */
अटल माप_प्रकार z_free_page_गिनती;

#घोषणा PF_ASSERT(pf, व्यंजक, fmt, ...) \
	__ASSERT(व्यंजक, "page frame 0x%lx: " fmt, k_mem_page_frame_to_phys(pf), \
		 ##__VA_ARGS__)

/* Get an unused page frame. don't care which one, or NULL if there are none */
अटल काष्ठा k_mem_page_frame *free_page_frame_list_get(व्योम)
{
	sys_sfnode_t *गाँठ;
	काष्ठा k_mem_page_frame *pf = शून्य;

	गाँठ = sys_sflist_get(&free_page_frame_list);
	अगर (गाँठ != शून्य) {
		z_free_page_गिनती--;
		pf = CONTAINER_OF(गाँठ, काष्ठा k_mem_page_frame, गाँठ);
		PF_ASSERT(pf, k_mem_page_frame_is_free(pf),
			 "on free list but not free");
		pf->va_and_flags = 0;
	}

	वापस pf;
}

/* Release a page frame back into the list of free pages */
अटल व्योम free_page_frame_list_put(काष्ठा k_mem_page_frame *pf)
{
	PF_ASSERT(pf, k_mem_page_frame_is_available(pf),
		 "unavailable page put on free list");

	sys_sfnode_init(&pf->गाँठ, K_MEM_PAGE_FRAME_FREE);
	sys_sflist_append(&free_page_frame_list, &pf->गाँठ);
	z_free_page_गिनती++;
}

अटल व्योम free_page_frame_list_init(व्योम)
{
	sys_sflist_init(&free_page_frame_list);
}

अटल व्योम page_frame_free_locked(काष्ठा k_mem_page_frame *pf)
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
अटल व्योम frame_mapped_set(काष्ठा k_mem_page_frame *pf, व्योम *addr)
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
		 k_mem_page_frame_to_virt(pf));

	uintptr_t flags_mask = CONFIG_MMU_PAGE_आकार - 1;
	uintptr_t va = (uintptr_t)addr & ~flags_mask;

	pf->va_and_flags &= flags_mask;
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
अटल पूर्णांक virt_to_page_frame(व्योम *virt, uintptr_t *phys)
{
	uintptr_t paddr;
	काष्ठा k_mem_page_frame *pf;
	पूर्णांक ret = -EFAULT;

	K_MEM_PAGE_FRAME_FOREACH(paddr, pf) {
		अगर (k_mem_page_frame_is_mapped(pf)) {
			अगर (virt == k_mem_page_frame_to_virt(pf)) {
				ret = 0;
				अगर (phys != शून्य) {
					*phys = k_mem_page_frame_to_phys(pf);
				}
				अवरोध;
			}
		}
	}

	वापस ret;
}
/* LCOV_EXCL_STOP */

__weak FUNC_ALIAS(virt_to_page_frame, arch_page_phys_get, पूर्णांक);

#अगर_घोषित CONFIG_DEMAND_PAGING
अटल पूर्णांक page_frame_prepare_locked(काष्ठा k_mem_page_frame *pf, bool *dirty_ptr,
				     bool page_in, uintptr_t *location_ptr);

अटल अंतरभूत व्योम do_backing_store_page_in(uintptr_t location);
अटल अंतरभूत व्योम do_backing_store_page_out(uintptr_t location);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING */

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
अटल पूर्णांक map_anon_page(व्योम *addr, uint32_t flags, bool *recovered)
{
	काष्ठा k_mem_page_frame *pf;
	uintptr_t phys;
	bool lock = (flags & K_MEM_MAP_LOCK) != 0U;
	पूर्णांक ret;

	*recovered = true;

	pf = free_page_frame_list_get();
	अगर (pf == शून्य) {
#अगर_घोषित CONFIG_DEMAND_PAGING
		uintptr_t location;
		bool dirty;

		pf = k_mem_paging_eviction_select(&dirty);
		अगर (pf == शून्य) {
			/* Every evictable page frame is pinned or busy */
			वापस -ENOMEM;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_page_frame_to_virt(pf),
			k_mem_page_frame_to_phys(pf));
		ret = page_frame_prepare_locked(pf, &dirty, false, &location);
		अगर (ret != 0) {
			वापस -ENOMEM;
		}
		अगर (dirty) {
			do_backing_store_page_out(location);
		}
		pf->va_and_flags = 0;
#अन्यथा
		वापस -ENOMEM;
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING */
	}

	phys = k_mem_page_frame_to_phys(pf);
	ret = arch_mem_map(addr, phys, CONFIG_MMU_PAGE_आकार, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	अगर (ret != 0) {
#अगर_घोषित CONFIG_USERSPACE
		/* The mapping above may have already succeeded in some
		 * memory domains but failing in the middle. So we need to
		 * unmap those first before returning the page frame back
		 * to the list.
		 */
		पूर्णांक unmap_ret = arch_mem_unmap(addr, CONFIG_MMU_PAGE_आकार);

		__ASSERT(unmap_ret == 0, "%s: cannot undo failed mapping %d\n", __func__,
			 unmap_ret);

		अगर (unmap_ret != 0) {
			/* The page may still be mapped in some memory domains,
			 * so the page frame cannot be given back to the free
			 * list for reuse.
			 */
			*recovered = false;
			वापस ret;
		}
#पूर्ण_अगर

		/* Mapping failed: give the page frame back before bailing out. */
		page_frame_free_locked(pf);
		वापस ret;
	}

	अगर (lock) {
		k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
	}
	frame_mapped_set(pf, addr);
#अगर_घोषित CONFIG_DEMAND_PAGING
	अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!lock)) {
		k_mem_paging_eviction_add(pf);
	}
#पूर्ण_अगर

	LOG_DBG("memory mapping anon page %p -> 0x%lx", addr, phys);

	वापस 0;
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
अटल पूर्णांक unmap_anon_pages(व्योम *virt, माप_प्रकार sz)
{
	uint8_t *pos;
	uintptr_t phys;
	काष्ठा k_mem_page_frame *pf;
	पूर्णांक ret = 0;

	VIRT_FOREACH(virt, sz, pos) {
#अगर_घोषित CONFIG_DEMAND_PAGING
		क्रमागत arch_page_location status;
		uintptr_t location;

		status = arch_page_location_get(pos, &location);
		चयन (status) {
		हाल ARCH_PAGE_LOCATION_PAGED_OUT:
			/*
			 * No pf is associated with this mapping.
			 * Simply get rid of the MMU entry and free
			 * corresponding backing store.
			 */
			ret = arch_mem_unmap(pos, CONFIG_MMU_PAGE_आकार);
			__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__,
				 ret);

			k_mem_paging_backing_store_location_free(location);

			अगर (ret != 0) {
				जाओ unmap_anon_out;
			}

			जारी;
		हाल ARCH_PAGE_LOCATION_PAGED_IN:
			/*
			 * The page is in memory but it may not be
			 * accessible in order to manage tracking
			 * of the ARCH_DATA_PAGE_ACCESSED flag
			 * meaning arch_page_phys_get() could fail.
			 * Still, we know the actual phys address.
			 */
			phys = location;
			ret = 0;
			अवरोध;
		शेष:
			ret = arch_page_phys_get(pos, &phys);
			अवरोध;
		}
#अन्यथा
		ret = arch_page_phys_get(pos, &phys);
#पूर्ण_अगर
		__ASSERT(ret == 0, "%s: cannot unmap an unmapped address %p", __func__, pos);
		अगर (ret != 0) {
			/* Found an address not mapped. Do not continue. */
			जाओ unmap_anon_out;
		}

		__ASSERT(k_mem_is_page_frame(phys), "%s: 0x%lx is not a page frame", __func__,
			 phys);
		अगर (!k_mem_is_page_frame(phys)) {
			/* Physical address has no corresponding page frame
			 * description in the page frame array.
			 * This should not happen. Do not continue.
			 */
			जाओ unmap_anon_out;
		}

		/* Grab the corresponding page frame from physical address */
		pf = k_mem_phys_to_page_frame(phys);

		__ASSERT(k_mem_page_frame_is_mapped(pf), "%s: 0x%lx is not a mapped page frame",
			 __func__, phys);
		अगर (!k_mem_page_frame_is_mapped(pf)) {
			/* Page frame is not marked mapped.
			 * This should not happen. Do not continue.
			 */
			जाओ unmap_anon_out;
		}

		ret = arch_mem_unmap(pos, CONFIG_MMU_PAGE_आकार);
		__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
		अगर (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as the page is still mapped.
			 */
			जाओ unmap_anon_out;
		}

#अगर_घोषित CONFIG_DEMAND_PAGING
		अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
		    (!k_mem_page_frame_is_pinned(pf))) {
			k_mem_paging_eviction_remove(pf);
		}
#पूर्ण_अगर

		/* Put the page frame back into free list */
		page_frame_free_locked(pf);
	}

unmap_anon_out:
	वापस ret;
}

#अगर_घोषित CONFIG_DEMAND_MAPPING
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
अटल bool undo_demand_mapped_pages(व्योम *virt, माप_प्रकार sz)
{
	uint8_t *undo_pos;
	bool success = true;
	पूर्णांक ret;

	/* Undo the pages mapped before the failure. */
	VIRT_FOREACH(virt, sz, undo_pos) {
		ret = arch_mem_unmap(undo_pos, CONFIG_MMU_PAGE_आकार);
		__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

		/* Failed to undo one mapped page. Though we still
		 * want to continue unmapping the remaining mapped
		 * pages to reclaim them.
		 */
		अगर (ret != 0) {
			success = false;
		}
	}

	वापस success;
}
#पूर्ण_अगर

व्योम *k_mem_map_phys_guard(uintptr_t phys, माप_प्रकार आकार, uint32_t flags, bool is_anon)
{
	uint8_t *dst;
	uint8_t *region;
	माप_प्रकार योग_आकार;
	पूर्णांक ret;
	k_spinlock_कुंजी_t कुंजी;
	uint8_t *pos;
	bool uninit = (flags & K_MEM_MAP_UNINIT) != 0U;

	__ASSERT(!is_anon || (is_anon && page_frames_initialized),
		 "%s called too early", __func__);
	__ASSERT((flags & K_MEM_CACHE_MASK) == 0U,
		 "%s does not support explicit cache settings", __func__);

	अगर (((flags & K_MEM_PERM_USER) != 0U) &&
	    ((flags & K_MEM_MAP_UNINIT) != 0U)) {
		LOG_ERR("user access to anonymous uninitialized pages is forbidden");
		वापस शून्य;
	}
	अगर ((आकार % CONFIG_MMU_PAGE_आकार) != 0U) {
		LOG_ERR("unaligned size %zu passed to %s", आकार, __func__);
		वापस शून्य;
	}
	अगर (आकार == 0) {
		LOG_ERR("zero sized memory mapping");
		वापस शून्य;
	}

	/* Need extra for the guard pages (before and after) which we
	 * won't map.
	 */
	अगर (आकार_add_overflow(आकार, CONFIG_MMU_PAGE_आकार * 2, &योग_आकार)) {
		LOG_ERR("too large size %zu passed to %s", आकार, __func__);
		वापस शून्य;
	}

	कुंजी = k_spin_lock(&z_mm_lock);

	region = virt_region_alloc(योग_आकार, CONFIG_MMU_PAGE_आकार);
	अगर (region == शून्य) {
		/* Address space has no free region */
		dst = शून्य;
		जाओ out;
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
	अगर (ret != 0) {
		dst = शून्य;
		जाओ out;
	}

	ret = arch_mem_unmap(region + CONFIG_MMU_PAGE_आकार + आकार, CONFIG_MMU_PAGE_आकार);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	अगर (ret != 0) {
		dst = शून्य;
		जाओ out;
	}

	/* Skip over the "before" guard page in returned address. */
	dst = region + CONFIG_MMU_PAGE_आकार;

	अगर (is_anon) {
		/* Mapping from anonymous memory */
		flags |= K_MEM_CACHE_WB;
#अगर_घोषित CONFIG_DEMAND_MAPPING
		अगर ((flags & K_MEM_MAP_LOCK) == 0) {
			flags |= K_MEM_MAP_UNPAGED;
			VIRT_FOREACH(dst, आकार, pos) {
				ret = arch_mem_map(pos,
						   uninit ? ARCH_UNPAGED_ANON_UNINIT
							  : ARCH_UNPAGED_ANON_ZERO,
						   CONFIG_MMU_PAGE_आकार, flags);
				__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
				अगर (ret != 0) {
					माप_प्रकार unmap_sz = pos - dst;

					अगर (IS_ENABLED(CONFIG_USERSPACE)) {
						/* Note that it is unmapping an extra page due to
						 * the possibility that the mapping above has
						 * already updated some memory domains but fail in
						 * the middle. So we need to include the failed
						 * page to unmap those that have been mapped so
						 * far.
						 */
						unmap_sz += CONFIG_MMU_PAGE_आकार;
					}

					bool success = undo_demand_mapped_pages(dst, unmap_sz);

					dst = शून्य;

					अगर (success) {
						जाओ fail;
					} अन्यथा {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						जाओ out;
					}
				}
			}
			LOG_DBG("memory mapping anon pages %p to %p unpaged", dst, pos-1);
			/* skip the memset() below */
			uninit = true;
		} अन्यथा
#पूर्ण_अगर
		{
			VIRT_FOREACH(dst, आकार, pos) {
				bool recovered;

				ret = map_anon_page(pos, flags, &recovered);

				अगर (ret != 0) {
					पूर्णांक unmap_ret = unmap_anon_pages(dst, pos - dst);

					dst = शून्य;

					अगर ((unmap_ret == 0) && recovered) {
						जाओ fail;
					} अन्यथा {
						/* Cannot fully recover from mapping
						 * failure. We cannot free the allocated
						 * virtual memory region since some of
						 * the page may still be mapped.
						 */
						जाओ out;
					}
				}
			}
		}
	} अन्यथा {
		/* Mapping known physical memory. */
		ret = arch_mem_map(dst, phys, आकार, flags);
		__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
		अगर (ret != 0) {
			/* Clean up any partial mapping left behind. */
			ret = arch_mem_unmap(dst, आकार);
			__ASSERT(ret == 0, "%s: cannot undo mapped pages %d", __func__, ret);

			dst = शून्य;

			अगर (ret != 0) {
				/* Cannot fully undo all the mappings. There might still be pages
				 * mapped. So we cannot free the allocated virtual memory region.
				 */
				जाओ out;
			} अन्यथा {
				जाओ fail;
			}
		}
	}

out:
	k_spin_unlock(&z_mm_lock, कुंजी);

	अगर (dst != शून्य && !uninit) {
		/* If we later implement mappings to a copy-on-write
		 * zero page, won't need this step
		 */
		स_रखो(dst, 0, आकार);
	}

	वापस dst;

fail:
	/* Release the whole region, including the guard pages. */
	virt_region_free(region, योग_आकार);

	जाओ out;
}

व्योम k_mem_unmap_phys_guard(व्योम *addr, माप_प्रकार आकार, bool is_anon)
{
	uint8_t *pos;
	k_spinlock_कुंजी_t कुंजी;
	माप_प्रकार योग_आकार;
	पूर्णांक ret;

	/* Need space for the "before" guard page */
	__ASSERT_NO_MSG(POINTER_TO_UINT(addr) >= CONFIG_MMU_PAGE_आकार);

	/* Make sure address range is still valid after accounting
	 * for two guard pages.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_PAGE_आकार;
	k_mem_assert_virtual_region(pos, आकार + (CONFIG_MMU_PAGE_आकार * 2));

	कुंजी = k_spin_lock(&z_mm_lock);

	/* Check if both guard pages are unmapped.
	 * Bail if not, as this is probably a region not mapped
	 * using k_mem_map().
	 */
	pos = addr;
	ret = arch_page_phys_get(pos - CONFIG_MMU_PAGE_आकार, शून्य);
	__ASSERT(ret != 0,
		 "%s: cannot find preceding guard page for (%p, %zu)",
		 __func__, addr, आकार);
	अगर (ret == 0) {
		जाओ out;
	}

	ret = arch_page_phys_get(pos + आकार, शून्य);
	__ASSERT(ret != 0,
		 "%s: cannot find succeeding guard page for (%p, %zu)",
		 __func__, addr, आकार);
	अगर (ret == 0) {
		जाओ out;
	}

	अगर (is_anon) {
		/* Unmapping anonymous memory */
		ret = unmap_anon_pages(addr, आकार);
		__ASSERT(ret == 0, "%s: unmap_anon_pages() returned %d", __func__, ret);
		अगर (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			जाओ out;
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
		अगर (ret != 0) {
			/* Fail to unmap. Must bail without releasing any resources
			 * as some pages are still be mapped.
			 */
			जाओ out;
		}
	}

	/* There are guard pages just before and after the mapped
	 * region. So we also need to free them from the bitmap.
	 */
	pos = (uint8_t *)addr - CONFIG_MMU_PAGE_आकार;
	योग_आकार = आकार + (CONFIG_MMU_PAGE_आकार * 2);
	virt_region_free(pos, योग_आकार);

out:
	k_spin_unlock(&z_mm_lock, कुंजी);
}

पूर्णांक k_mem_update_flags(व्योम *addr, माप_प्रकार आकार, uint32_t flags)
{
	uintptr_t phys;
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक ret;

	k_mem_assert_virtual_region(addr, आकार);

	कुंजी = k_spin_lock(&z_mm_lock);

	/*
	 * We can achieve desired result without explicit architecture support
	 * by unmapping and remapping the same physical memory using new flags.
	 */

	ret = arch_page_phys_get(addr, &phys);
	अगर (ret < 0) {
		जाओ out;
	}

	/* TODO: detect and handle paged-out memory as well */

	ret = arch_mem_unmap(addr, आकार);
	__ASSERT(ret == 0, "%s: arch_mem_unmap failed %d\n", __func__, ret);
	अगर (ret != 0) {
		जाओ out;
	}

	ret = arch_mem_map(addr, phys, आकार, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);

out:
	k_spin_unlock(&z_mm_lock, कुंजी);
	वापस ret;
}

माप_प्रकार k_mem_free_get(व्योम)
{
	माप_प्रकार ret;
	k_spinlock_कुंजी_t कुंजी;

	__ASSERT(page_frames_initialized, "%s called too early", __func__);

	कुंजी = k_spin_lock(&z_mm_lock);
#अगर_घोषित CONFIG_DEMAND_PAGING
	अगर (z_free_page_गिनती > CONFIG_DEMAND_PAGING_PAGE_FRAMES_RESERVE) {
		ret = z_free_page_गिनती - CONFIG_DEMAND_PAGING_PAGE_FRAMES_RESERVE;
	} अन्यथा {
		ret = 0;
	}
#अन्यथा
	ret = z_free_page_गिनती;
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING */
	k_spin_unlock(&z_mm_lock, कुंजी);

	वापस ret * (माप_प्रकार)CONFIG_MMU_PAGE_आकार;
}

/* Get the default virtual region alignment, here the default MMU page size
 *
 * @param[in] phys Physical address of region to be mapped, aligned to MMU_PAGE_SIZE
 * @param[in] size Size of region to be mapped, aligned to MMU_PAGE_SIZE
 *
 * @return alignment to apply on the virtual address of this region
 */
अटल माप_प्रकार virt_region_align(uintptr_t phys, माप_प्रकार आकार)
{
	ARG_UNUSED(phys);
	ARG_UNUSED(आकार);

	वापस CONFIG_MMU_PAGE_आकार;
}

__weak FUNC_ALIAS(virt_region_align, arch_virt_region_align, माप_प्रकार);

/* This may be called from arch early boot code before z_cstart() is invoked.
 * Data will be copied and BSS zeroed, but this must not rely on any
 * initialization functions being called prior to work correctly.
 */
व्योम k_mem_map_phys_bare(uint8_t **virt_ptr, uintptr_t phys, माप_प्रकार आकार, uint32_t flags)
{
	uintptr_t aligned_phys, addr_offset;
	माप_प्रकार aligned_आकार, align_boundary;
	k_spinlock_कुंजी_t कुंजी;
	uint8_t *dest_addr;
	माप_प्रकार अंक_bits;
	माप_प्रकार offset;
	पूर्णांक ret;

#अगर_अघोषित CONFIG_KERNEL_DIRECT_MAP
	__ASSERT(!(flags & K_MEM_DIRECT_MAP), "The direct-map is not enabled");
#पूर्ण_अगर /* CONFIG_KERNEL_DIRECT_MAP */
	addr_offset = k_mem_region_align(&aligned_phys, &aligned_आकार,
					 phys, आकार,
					 CONFIG_MMU_PAGE_आकार);
	__ASSERT(aligned_आकार != 0U, "0-length mapping at 0x%lx", aligned_phys);
	__ASSERT(aligned_आकार - 1 <= (UINTPTR_MAX - aligned_phys),
		 "wraparound for physical address 0x%lx (size %zu)",
		 aligned_phys, aligned_आकार);

	align_boundary = arch_virt_region_align(aligned_phys, aligned_आकार);

	कुंजी = k_spin_lock(&z_mm_lock);

	अगर (IS_ENABLED(CONFIG_KERNEL_DIRECT_MAP) &&
	    (flags & K_MEM_DIRECT_MAP)) {
		dest_addr = (uint8_t *)aligned_phys;

		/* Mark the region of virtual memory bitmap as used
		 * if the region overlaps the virtual memory space.
		 *
		 * Basically if either end of region is within
		 * virtual memory space, we need to mark the bits.
		 */

		अगर (IN_RANGE(aligned_phys,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_END - 1)) ||
		    IN_RANGE(aligned_phys + aligned_आकार - 1,
			      (uintptr_t)K_MEM_VIRT_RAM_START,
			      (uintptr_t)(K_MEM_VIRT_RAM_END - 1))) {
			uint8_t *adjusted_start = max(dest_addr, K_MEM_VIRT_RAM_START);
			uint8_t *adjusted_end = min(dest_addr + aligned_आकार,
						    K_MEM_VIRT_RAM_END);
			माप_प्रकार adjusted_sz = adjusted_end - adjusted_start;

			अंक_bits = adjusted_sz / CONFIG_MMU_PAGE_आकार;
			offset = virt_to_bitmap_offset(adjusted_start, adjusted_sz);
			अगर (sys_bitarray_test_and_set_region(
			    &virt_region_bitmap, अंक_bits, offset, true)) {
				जाओ fail;
			}
		}
	} अन्यथा {
		/* Obtain an appropriately sized chunk of virtual memory */
		dest_addr = virt_region_alloc(aligned_आकार, align_boundary);
		अगर (!dest_addr) {
			जाओ fail;
		}
	}

	/* If this fails there's something amiss with virt_region_get */
	__ASSERT((uintptr_t)dest_addr <
		 ((uintptr_t)dest_addr + (आकार - 1)),
		 "wraparound for virtual address %p (size %zu)",
		 dest_addr, आकार);

	LOG_DBG("arch_mem_map(%p, 0x%lx, %zu, %x) offset %lu", (व्योम *)dest_addr,
		aligned_phys, aligned_आकार, flags, addr_offset);

	ret = arch_mem_map(dest_addr, aligned_phys, aligned_आकार, flags);
	__ASSERT(ret == 0, "%s: arch_mem_map failed %d\n", __func__, ret);
	अगर (ret != 0) {
		जाओ fail;
	}
	k_spin_unlock(&z_mm_lock, कुंजी);

	*virt_ptr = dest_addr + addr_offset;
	वापस;
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

व्योम k_mem_unmap_phys_bare(uint8_t *virt, माप_प्रकार आकार)
{
	uintptr_t aligned_virt, addr_offset;
	माप_प्रकार aligned_आकार;
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक ret;

	addr_offset = k_mem_region_align(&aligned_virt, &aligned_आकार,
					 POINTER_TO_UINT(virt), आकार,
					 CONFIG_MMU_PAGE_आकार);
	__ASSERT(aligned_आकार != 0U, "0-length mapping at 0x%lx", aligned_virt);
	__ASSERT(aligned_आकार - 1 <= (UINTPTR_MAX - aligned_virt),
		 "wraparound for virtual address 0x%lx (size %zu)",
		 aligned_virt, aligned_आकार);

	कुंजी = k_spin_lock(&z_mm_lock);

	LOG_DBG("arch_mem_unmap(0x%lx, %zu) offset %lu",
		aligned_virt, aligned_आकार, addr_offset);

	ret = arch_mem_unmap(UINT_TO_POINTER(aligned_virt), aligned_आकार);
	__ASSERT(ret == 0, "%s: arch_mem_unmap() returned %d", __func__, ret);
	अगर (ret != 0) {
		/* Fail to unmap. We cannot free the virtual memory region
		 * as some pages are still be mapped.
		 */
		जाओ out;
	}

	virt_region_free(UINT_TO_POINTER(aligned_virt), aligned_आकार);

out:
	k_spin_unlock(&z_mm_lock, कुंजी);
}

/*
 * Miscellaneous
 */

माप_प्रकार k_mem_region_align(uintptr_t *aligned_addr, माप_प्रकार *aligned_आकार,
			  uintptr_t addr, माप_प्रकार आकार, माप_प्रकार align)
{
	माप_प्रकार addr_offset;

	/* The actual mapped region must be page-aligned. Round down the
	 * physical address and pad the region size appropriately
	 */
	*aligned_addr = ROUND_DOWN(addr, align);
	addr_offset = addr - *aligned_addr;
	*aligned_आकार = ROUND_UP(आकार + addr_offset, align);

	वापस addr_offset;
}

#अगर_घोषित CONFIG_LINKER_USE_BOOT_SECTION
अटल व्योम mark_linker_section_pinned(व्योम *start_addr, व्योम *end_addr,
				       bool pin)
{
	काष्ठा k_mem_page_frame *pf;
	uint8_t *addr;

	uintptr_t pinned_start = ROUND_DOWN(POINTER_TO_UINT(start_addr),
					    CONFIG_MMU_PAGE_आकार);
	uintptr_t pinned_end = ROUND_UP(POINTER_TO_UINT(end_addr),
					CONFIG_MMU_PAGE_आकार);
	माप_प्रकार pinned_आकार = pinned_end - pinned_start;

	VIRT_FOREACH(UINT_TO_POINTER(pinned_start), pinned_आकार, addr)
	{
		pf = k_mem_phys_to_page_frame(K_MEM_BOOT_VIRT_TO_PHYS(addr));
		frame_mapped_set(pf, addr);

		अगर (pin) {
			k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
		} अन्यथा {
			k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_PINNED);
#अगर_घोषित CONFIG_DEMAND_PAGING
			अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING) &&
			    k_mem_page_frame_is_evictable(pf)) {
				k_mem_paging_eviction_add(pf);
			}
#पूर्ण_अगर
		}
	}
}
#पूर्ण_अगर /* CONFIG_LINKER_USE_BOOT_SECTION */

#अगर_घोषित CONFIG_LINKER_USE_ONDEMAND_SECTION
अटल व्योम z_paging_ondemand_section_map(व्योम)
{
	uint8_t *addr;
	माप_प्रकार आकार;
	uintptr_t location;
	uint32_t flags;
	पूर्णांक ret;

	आकार = (uintptr_t)lnkr_ondemand_text_आकार;
	flags = K_MEM_MAP_UNPAGED | K_MEM_PERM_EXEC | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_text_start, आकार, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_PAGE_आकार, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		अगर (ret != 0) {
			k_panic();
		}

		sys_bitarray_set_region(&virt_region_bitmap, 1,
					virt_to_bitmap_offset(addr, CONFIG_MMU_PAGE_आकार));
	}

	आकार = (uintptr_t)lnkr_ondemand_rodata_आकार;
	flags = K_MEM_MAP_UNPAGED | K_MEM_CACHE_WB;
	VIRT_FOREACH(lnkr_ondemand_rodata_start, आकार, addr) {
		k_mem_paging_backing_store_location_query(addr, &location);

		ret = arch_mem_map(addr, location, CONFIG_MMU_PAGE_आकार, flags);
		__ASSERT(ret == 0, "arch_mem_map() returned %d", ret);
		अगर (ret != 0) {
			k_panic();
		}

		sys_bitarray_set_region(&virt_region_bitmap, 1,
					virt_to_bitmap_offset(addr, CONFIG_MMU_PAGE_आकार));
	}
}
#पूर्ण_अगर /* CONFIG_LINKER_USE_ONDEMAND_SECTION */

व्योम z_mem_manage_init(व्योम)
{
	uintptr_t phys;
	uint8_t *addr;
	काष्ठा k_mem_page_frame *pf;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&z_mm_lock);

	free_page_frame_list_init();

	ARG_UNUSED(addr);

#अगर_घोषित CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES
	/* If some page frames are unavailable for use as memory, arch
	 * code will mark K_MEM_PAGE_FRAME_RESERVED in their flags
	 */
	arch_reserved_pages_update();
#पूर्ण_अगर /* CONFIG_ARCH_HAS_RESERVED_PAGE_FRAMES */

	/* The entire Zephyr image is mapped and pinned at boot. Demand
	 * paging applies only to anonymous mappings created via k_mem_map()
	 * and to memory placed in explicit __ondemand_* linker sections;
	 * the kernel image itself is never a candidate for eviction.
	 */
	VIRT_FOREACH(K_MEM_KERNEL_VIRT_START, K_MEM_KERNEL_VIRT_आकार, addr)
	{
		pf = k_mem_phys_to_page_frame(K_MEM_BOOT_VIRT_TO_PHYS(addr));
		frame_mapped_set(pf, addr);
		k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
	}

#अगर_घोषित CONFIG_LINKER_USE_BOOT_SECTION
	/* Pin the boot section to prevent it from being swapped out during
	 * boot process. Will be un-pinned once boot process completes.
	 */
	mark_linker_section_pinned(lnkr_boot_start, lnkr_boot_end, true);
#पूर्ण_अगर /* CONFIG_LINKER_USE_BOOT_SECTION */

	/* Any remaining pages that aren't mapped, reserved, or pinned get
	 * added to the free pages list
	 */
	K_MEM_PAGE_FRAME_FOREACH(phys, pf) {
		अगर (k_mem_page_frame_is_available(pf)) {
			free_page_frame_list_put(pf);
		}
	}
	LOG_DBG("free page frames: %zu", z_free_page_गिनती);

#अगर_घोषित CONFIG_DEMAND_PAGING
#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	z_paging_histogram_init();
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
	k_mem_paging_backing_store_init();
	k_mem_paging_eviction_init();

	अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
		/* start tracking evictable page installed above if any */
		K_MEM_PAGE_FRAME_FOREACH(phys, pf) {
			अगर (k_mem_page_frame_is_evictable(pf)) {
				k_mem_paging_eviction_add(pf);
			}
		}
	}
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING */

#अगर_घोषित CONFIG_LINKER_USE_ONDEMAND_SECTION
	z_paging_ondemand_section_map();
#पूर्ण_अगर

#अगर_घोषित CONFIG_ASSERT
	page_frames_initialized = true;
#पूर्ण_अगर /* CONFIG_ASSERT */

	k_spin_unlock(&z_mm_lock, कुंजी);
}

व्योम z_mem_manage_boot_finish(व्योम)
{
#अगर_घोषित CONFIG_LINKER_USE_BOOT_SECTION
	/* At the end of boot process, unpin the boot sections
	 * as they don't need to be in memory all the time anymore.
	 */
	mark_linker_section_pinned(lnkr_boot_start, lnkr_boot_end, false);
#पूर्ण_अगर /* CONFIG_LINKER_USE_BOOT_SECTION */
}

#अगर_घोषित CONFIG_DEMAND_PAGING

#अगर_घोषित CONFIG_DEMAND_PAGING_STATS
काष्ठा k_mem_paging_stats_t paging_stats;
बाह्य काष्ठा k_mem_paging_histogram_t z_paging_histogram_eviction;
बाह्य काष्ठा k_mem_paging_histogram_t z_paging_histogram_backing_store_page_in;
बाह्य काष्ठा k_mem_paging_histogram_t z_paging_histogram_backing_store_page_out;
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS */

अटल अंतरभूत व्योम do_backing_store_page_in(uintptr_t location)
{
#अगर_घोषित CONFIG_DEMAND_MAPPING
	/* Check for special cases */
	चयन (location) {
	हाल ARCH_UNPAGED_ANON_ZERO:
		स_रखो(K_MEM_SCRATCH_PAGE, 0, CONFIG_MMU_PAGE_आकार);
		__fallthrough;
	हाल ARCH_UNPAGED_ANON_UNINIT:
		/* nothing else to do */
		वापस;
	शेष:
		अवरोध;
	}
#पूर्ण_अगर /* CONFIG_DEMAND_MAPPING */

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#अगर_घोषित CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_end;

	time_start = timing_गणक_get();
#अन्यथा
	uint32_t time_start;

	time_start = k_cycle_get_32();
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_page_in(location);

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#अगर_घोषित CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_end = timing_गणक_get();
	time_diff = (uint32_t)timing_cycles_get(&time_start, &time_end);
#अन्यथा
	time_diff = k_cycle_get_32() - time_start;
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_page_in,
			       time_diff);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

अटल अंतरभूत व्योम do_backing_store_page_out(uintptr_t location)
{
#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#अगर_घोषित CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_end;

	time_start = timing_गणक_get();
#अन्यथा
	uint32_t time_start;

	time_start = k_cycle_get_32();
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	k_mem_paging_backing_store_page_out(location);

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#अगर_घोषित CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_end = timing_गणक_get();
	time_diff = (uint32_t)timing_cycles_get(&time_start, &time_end);
#अन्यथा
	time_diff = k_cycle_get_32() - time_start;
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_backing_store_page_out,
			       time_diff);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */
}

#अगर defined(CONFIG_SMP) && defined(CONFIG_DEMAND_PAGING_ALLOW_IRQ)
/*
 * SMP support is very simple. Some resources such as the scratch page could
 * be made per CPU, backing store driver execution be confined to the faulting
 * CPU, statistics be made to cope with access concurrency, etc. But in the
 * end we're dealing with memory transfer to/from some external storage which
 * is inherently slow and whose access is most likely serialized anyway.
 * So let's simply enforce global demand paging serialization across all CPUs
 * with a mutex as there is no real gain from added parallelism here.
 */
अटल K_MUTEX_DEFINE(z_mm_paging_lock);
#पूर्ण_अगर

अटल व्योम virt_region_foreach(व्योम *addr, माप_प्रकार आकार,
				व्योम (*func)(व्योम *vaddr))
{
	k_mem_assert_virtual_region(addr, आकार);

	क्रम (माप_प्रकार offset = 0; offset < आकार; offset += CONFIG_MMU_PAGE_आकार) {
		func((uint8_t *)addr + offset);
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
अटल पूर्णांक page_frame_prepare_locked(काष्ठा k_mem_page_frame *pf, bool *dirty_ptr,
				     bool page_fault, uintptr_t *location_ptr)
{
	uintptr_t phys;
	पूर्णांक ret;
	bool dirty = *dirty_ptr;

	phys = k_mem_page_frame_to_phys(pf);
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
	अगर (k_mem_page_frame_is_mapped(pf)) {
		dirty = dirty || !k_mem_page_frame_is_backed(pf);
	}

	अगर (dirty || page_fault) {
		arch_mem_scratch(phys);
	}

	अगर (k_mem_page_frame_is_mapped(pf)) {
		ret = k_mem_paging_backing_store_location_get(pf, location_ptr,
							      page_fault);
		अगर (ret != 0) {
			LOG_ERR("out of backing store memory");
			वापस -ENOMEM;
		}
		arch_mem_page_out(k_mem_page_frame_to_virt(pf), *location_ptr);

		अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
			k_mem_paging_eviction_remove(pf);
		}
	} अन्यथा {
		/* Shouldn't happen unless this function is mis-used */
		__ASSERT(!dirty, "un-mapped page determined to be dirty");
	}
#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	/* Mark as busy so that k_mem_page_frame_is_evictable() returns false */
	__ASSERT(!k_mem_page_frame_is_busy(pf), "page frame 0x%lx is already busy",
		 phys);
	k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_BUSY);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	/* Update dirty parameter, since we set to true if it wasn't backed
	 * even if otherwise clean
	 */
	*dirty_ptr = dirty;

	वापस 0;
}

अटल पूर्णांक do_mem_evict(व्योम *addr)
{
	bool dirty;
	काष्ठा k_mem_page_frame *pf;
	uintptr_t location;
	k_spinlock_कुंजी_t कुंजी;
	uintptr_t flags, phys;
	पूर्णांक ret;

#अगर CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_in_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#अगर_घोषित CONFIG_SMP
	k_mutex_lock(&z_mm_paging_lock, K_FOREVER);
#अन्यथा
	k_sched_lock();
#पूर्ण_अगर
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	कुंजी = k_spin_lock(&z_mm_lock);
	flags = arch_page_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_तथ्य_PAGE_NOT_MAPPED) == 0,
		 "address %p isn't mapped", addr);
	अगर ((flags & ARCH_तथ्य_PAGE_LOADED) == 0) {
		/* Un-mapped or already evicted. Nothing to do */
		ret = 0;
		जाओ out;
	}

	dirty = (flags & ARCH_तथ्य_PAGE_DIRTY) != 0;
	pf = k_mem_phys_to_page_frame(phys);
	__ASSERT(k_mem_page_frame_to_virt(pf) == addr, "page frame address mismatch");
	ret = page_frame_prepare_locked(pf, &dirty, false, &location);
	अगर (ret != 0) {
		जाओ out;
	}

#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_lock, कुंजी);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	अगर (dirty) {
		do_backing_store_page_out(location);
	}
#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	कुंजी = k_spin_lock(&z_mm_lock);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	page_frame_free_locked(pf);
out:
	k_spin_unlock(&z_mm_lock, कुंजी);
#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
#अगर_घोषित CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_lock);
#अन्यथा
	k_sched_unlock();
#पूर्ण_अगर
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	वापस ret;
}

पूर्णांक k_mem_page_out(व्योम *addr, माप_प्रकार आकार)
{
	__ASSERT(page_frames_initialized, "%s called on %p too early", __func__,
		 addr);
	k_mem_assert_virtual_region(addr, आकार);

	क्रम (माप_प्रकार offset = 0; offset < आकार; offset += CONFIG_MMU_PAGE_आकार) {
		व्योम *pos = (uint8_t *)addr + offset;
		पूर्णांक ret;

		ret = do_mem_evict(pos);
		अगर (ret != 0) {
			वापस ret;
		}
	}

	वापस 0;
}

पूर्णांक k_mem_page_frame_evict(uintptr_t phys)
{
	k_spinlock_कुंजी_t कुंजी;
	काष्ठा k_mem_page_frame *pf;
	bool dirty;
	uintptr_t flags;
	uintptr_t location;
	पूर्णांक ret;

	__ASSERT(page_frames_initialized, "%s called on 0x%lx too early",
		 __func__, phys);

	/* Implementation is similar to do_page_fault() except there is no
	 * data page to page-in, see comments in that function.
	 */

#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	__ASSERT(!k_is_in_isr(),
		 "%s is unavailable in ISRs with CONFIG_DEMAND_PAGING_ALLOW_IRQ",
		 __func__);
#अगर_घोषित CONFIG_SMP
	k_mutex_lock(&z_mm_paging_lock, K_FOREVER);
#अन्यथा
	k_sched_lock();
#पूर्ण_अगर
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	कुंजी = k_spin_lock(&z_mm_lock);
	pf = k_mem_phys_to_page_frame(phys);
	अगर (!k_mem_page_frame_is_mapped(pf)) {
		/* Nothing to do, free page */
		ret = 0;
		जाओ out;
	}
	flags = arch_page_info_get(k_mem_page_frame_to_virt(pf), शून्य, false);
	/* Shouldn't ever happen */
	__ASSERT((flags & ARCH_तथ्य_PAGE_LOADED) != 0, "data page not loaded");
	dirty = (flags & ARCH_तथ्य_PAGE_DIRTY) != 0;
	ret = page_frame_prepare_locked(pf, &dirty, false, &location);
	अगर (ret != 0) {
		जाओ out;
	}

#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_lock, कुंजी);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	अगर (dirty) {
		do_backing_store_page_out(location);
	}
#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	कुंजी = k_spin_lock(&z_mm_lock);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	page_frame_free_locked(pf);
out:
	k_spin_unlock(&z_mm_lock, कुंजी);
#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
#अगर_घोषित CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_lock);
#अन्यथा
	k_sched_unlock();
#पूर्ण_अगर
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	वापस ret;
}

अटल अंतरभूत व्योम paging_stats_faults_inc(काष्ठा k_thread *faulting_thread,
					   पूर्णांक कुंजी)
{
#अगर_घोषित CONFIG_DEMAND_PAGING_STATS
	bool is_irq_unlocked = arch_irq_unlocked(कुंजी);

	paging_stats.pagefaults.cnt++;

	अगर (is_irq_unlocked) {
		paging_stats.pagefaults.irq_unlocked++;
	} अन्यथा {
		paging_stats.pagefaults.irq_locked++;
	}

#अगर_घोषित CONFIG_DEMAND_PAGING_THREAD_STATS
	faulting_thread->paging_stats.pagefaults.cnt++;

	अगर (is_irq_unlocked) {
		faulting_thread->paging_stats.pagefaults.irq_unlocked++;
	} अन्यथा {
		faulting_thread->paging_stats.pagefaults.irq_locked++;
	}
#अन्यथा
	ARG_UNUSED(faulting_thread);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_THREAD_STATS */

#अगर_अघोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	अगर (k_is_in_isr()) {
		paging_stats.pagefaults.in_isr++;

#अगर_घोषित CONFIG_DEMAND_PAGING_THREAD_STATS
		faulting_thread->paging_stats.pagefaults.in_isr++;
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_THREAD_STATS */
	}
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS */
}

अटल अंतरभूत व्योम paging_stats_eviction_inc(काष्ठा k_thread *faulting_thread,
					     bool dirty)
{
#अगर_घोषित CONFIG_DEMAND_PAGING_STATS
	अगर (dirty) {
		paging_stats.eviction.dirty++;
	} अन्यथा {
		paging_stats.eviction.clean++;
	}
#अगर_घोषित CONFIG_DEMAND_PAGING_THREAD_STATS
	अगर (dirty) {
		faulting_thread->paging_stats.eviction.dirty++;
	} अन्यथा {
		faulting_thread->paging_stats.eviction.clean++;
	}
#अन्यथा
	ARG_UNUSED(faulting_thread);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_THREAD_STATS */
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS */
}

अटल अंतरभूत काष्ठा k_mem_page_frame *do_eviction_select(bool *dirty)
{
	काष्ठा k_mem_page_frame *pf;

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
	uint32_t time_diff;

#अगर_घोषित CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	timing_t time_start, time_end;

	time_start = timing_गणक_get();
#अन्यथा
	uint32_t time_start;

	time_start = k_cycle_get_32();
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	pf = k_mem_paging_eviction_select(dirty);

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
#अगर_घोषित CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS
	time_end = timing_गणक_get();
	time_diff = (uint32_t)timing_cycles_get(&time_start, &time_end);
#अन्यथा
	time_diff = k_cycle_get_32() - time_start;
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_STATS_USING_TIMING_FUNCTIONS */

	z_paging_histogram_inc(&z_paging_histogram_eviction, time_diff);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

	वापस pf;
}

अटल bool do_page_fault(व्योम *addr, bool pin)
{
	काष्ठा k_mem_page_frame *pf;
	k_spinlock_कुंजी_t कुंजी;
	uintptr_t page_in_location, page_out_location;
	क्रमागत arch_page_location status;
	bool परिणाम;
	bool dirty = false;
	काष्ठा k_thread *faulting_thread;
	पूर्णांक ret;

	__ASSERT(page_frames_initialized, "page fault at %p happened too early",
		 addr);

	LOG_DBG("page fault at %p", addr);

	/*
	 * TODO: Add performance accounting:
	 * - k_mem_paging_eviction_select() metrics
	 *   * periodic timer execution time histogram (if implemented)
	 */

#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
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
	__ASSERT(!k_is_in_isr(), "ISR page faults are forbidden");
#अगर_घोषित CONFIG_SMP
	k_mutex_lock(&z_mm_paging_lock, K_FOREVER);
#अन्यथा
	k_sched_lock();
#पूर्ण_अगर
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	कुंजी = k_spin_lock(&z_mm_lock);
	faulting_thread = _current;

	status = arch_page_location_get(addr, &page_in_location);
	अगर (status == ARCH_PAGE_LOCATION_BAD) {
		/* Return false to treat as a fatal error */
		परिणाम = false;
		जाओ out;
	}
	परिणाम = true;

	अगर (status == ARCH_PAGE_LOCATION_PAGED_IN) {
		अगर (pin) {
			/* It's a physical memory address */
			uintptr_t phys = page_in_location;

			pf = k_mem_phys_to_page_frame(phys);
			अगर (!k_mem_page_frame_is_pinned(pf)) {
				अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
					k_mem_paging_eviction_remove(pf);
				}
				k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
			}
		}

		/* This if-block is to pin the page if it is
		 * already present in physical memory. There is
		 * no need to go through the following code to
		 * pull in the data pages. So skip to the end.
		 */
		जाओ out;
	}
	__ASSERT(status == ARCH_PAGE_LOCATION_PAGED_OUT,
		 "unexpected status value %d", status);

	paging_stats_faults_inc(faulting_thread, कुंजी.कुंजी);

	pf = free_page_frame_list_get();
	अगर (pf == शून्य) {
		/* Need to evict a page frame */
		pf = do_eviction_select(&dirty);
		अगर (pf == शून्य) {
			/* Every evictable page frame is pinned or busy.
			 * Fail the fault so it is reported as fatal.
			 */
			परिणाम = false;
			जाओ out;
		}
		LOG_DBG("evicting %p at 0x%lx",
			k_mem_page_frame_to_virt(pf),
			k_mem_page_frame_to_phys(pf));

		paging_stats_eviction_inc(faulting_thread, dirty);
	}
	ret = page_frame_prepare_locked(pf, &dirty, true, &page_out_location);
	अगर (ret != 0) {
		/* Backing store is full. Fail the fault so it is
		 * reported as fatal.
		 */
		परिणाम = false;
		जाओ out;
	}

#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	k_spin_unlock(&z_mm_lock, कुंजी);
	/* Interrupts are now unlocked if they were not locked when we entered
	 * this function, and we may service ISRs. The scheduler is still
	 * locked.
	 */
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	अगर (dirty) {
		do_backing_store_page_out(page_out_location);
	}
	do_backing_store_page_in(page_in_location);

#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
	कुंजी = k_spin_lock(&z_mm_lock);
	k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_BUSY);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */
	k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_MAPPED);
	frame_mapped_set(pf, addr);
	अगर (pin) {
		k_mem_page_frame_set(pf, K_MEM_PAGE_FRAME_PINNED);
	}

	arch_mem_page_in(addr, k_mem_page_frame_to_phys(pf));
	k_mem_paging_backing_store_page_finalize(pf, page_in_location);
	अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING) && (!pin)) {
		k_mem_paging_eviction_add(pf);
	}
out:
	k_spin_unlock(&z_mm_lock, कुंजी);
#अगर_घोषित CONFIG_DEMAND_PAGING_ALLOW_IRQ
#अगर_घोषित CONFIG_SMP
	k_mutex_unlock(&z_mm_paging_lock);
#अन्यथा
	k_sched_unlock();
#पूर्ण_अगर
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_ALLOW_IRQ */

	वापस परिणाम;
}

अटल व्योम do_page_in(व्योम *addr)
{
	bool ret;

	ret = do_page_fault(addr, false);
	अगर (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing would defer the failure to an arbitrary later
		 * access.
		 */
		LOG_ERR("cannot page in address %p", addr);
		k_panic();
	}
}

व्योम k_mem_page_in(व्योम *addr, माप_प्रकार आकार)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_in_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, आकार, do_page_in);
}

अटल व्योम do_mem_pin(व्योम *addr)
{
	bool ret;

	ret = do_page_fault(addr, true);
	अगर (!ret) {
		/* There is no error reporting channel to the caller, and
		 * continuing with the memory not actually pinned would
		 * defer the failure to an arbitrary later access, possibly
		 * from a context that cannot handle a page fault.
		 */
		LOG_ERR("cannot pin address %p", addr);
		k_panic();
	}
}

व्योम k_mem_pin(व्योम *addr, माप_प्रकार आकार)
{
	__ASSERT(!IS_ENABLED(CONFIG_DEMAND_PAGING_ALLOW_IRQ) || !k_is_in_isr(),
		 "%s may not be called in ISRs if CONFIG_DEMAND_PAGING_ALLOW_IRQ is enabled",
		 __func__);
	virt_region_foreach(addr, आकार, do_mem_pin);
}

bool k_mem_page_fault(व्योम *addr)
{
	वापस do_page_fault(addr, false);
}

अटल व्योम do_mem_unpin(व्योम *addr)
{
	काष्ठा k_mem_page_frame *pf;
	k_spinlock_कुंजी_t कुंजी;
	uintptr_t flags, phys;

	कुंजी = k_spin_lock(&z_mm_lock);
	flags = arch_page_info_get(addr, &phys, false);
	__ASSERT((flags & ARCH_तथ्य_PAGE_NOT_MAPPED) == 0,
		 "invalid data page at %p", addr);
	अगर ((flags & ARCH_तथ्य_PAGE_LOADED) != 0) {
		pf = k_mem_phys_to_page_frame(phys);
		अगर (k_mem_page_frame_is_pinned(pf)) {
			k_mem_page_frame_clear(pf, K_MEM_PAGE_FRAME_PINNED);

			अगर (IS_ENABLED(CONFIG_EVICTION_TRACKING)) {
				k_mem_paging_eviction_add(pf);
			}
		}
	}
	k_spin_unlock(&z_mm_lock, कुंजी);
}

व्योम k_mem_unpin(व्योम *addr, माप_प्रकार आकार)
{
	__ASSERT(page_frames_initialized, "%s called on %p too early", __func__,
		 addr);
	virt_region_foreach(addr, आकार, do_mem_unpin);
}

#पूर्ण_अगर /* CONFIG_DEMAND_PAGING */
