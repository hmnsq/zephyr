/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/sys/util.h>

प्रकारनिर्देश शून्य * (sys_heap_allocator_t)(संरचना sys_heap *heap, size_t align, size_t अष्टकानि);

स्थिर शून्य *z_alloc_helper(संरचना k_heap *heap, size_t align, size_t आकार,
			    sys_heap_allocator_t sys_heap_allocator)
{
	शून्य *mem;
	संरचना k_heap **heap_ref;
	size_t __align;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	/* A power of 2 as well as 0 is OK */
	__ASSERT((align & (align - 1)) == 0,
		"align must be a power of 2");

	/*
	 * Adjust the size to make room for our heap reference.
	 * Merge a rewind bit with align value (see sys_heap_aligned_alloc()).
	 * This allows for storing the heap pointer right below the aligned
	 * boundary without wasting any memory.
	 */
	यदि (आकार_सङ्कलय_अतिप्रवाहः(आकार, परिमाण(heap_ref), &आकार)) {
		प्रत्यागम रिक्त;
	}
	__align = align | परिमाण(heap_ref);

	/*
	 * No point calling k_heap_malloc/k_heap_aligned_alloc with K_NO_WAIT.
	 * Better bypass them and go directly to sys_heap_*() instead.
	 */
	कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);
	mem = sys_heap_allocator(&heap->heap, __align, आकार);
	k_spin_unlock(&heap->तालकम्, कुञ्चिका);

	यदि (mem == रिक्त) {
		प्रत्यागम रिक्त;
	}

	heap_ref = mem;
	*heap_ref = heap;
	mem = ++heap_ref;
	__ASSERT(align == 0 || ((uintptr_t)mem & (align - 1)) == 0,
		 "misaligned memory at %p (align = %zu)", mem, align);

	प्रत्यागम mem;
}

शून्य k_free(शून्य *ptr)
{
	संरचना k_heap **heap_ref;

	यदि (ptr != रिक्त) {
		heap_ref = ptr;
		--heap_ref;
		ptr = heap_ref;

		SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_free, *heap_ref, heap_ref);

		/*
		 * Bypass k_heap_free() as z_unpend_all() and its scheduler
		 * locking is unnecessary here, similar to z_alloc_helper()
		 * bypassing k_heap_alloc() to go directly to sys_heap_*().
		 */
		संरचना k_heap *heap = *heap_ref;
		k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);

		sys_heap_free(&heap->heap, ptr);
		k_spin_unlock(&heap->तालकम्, कुञ्चिका);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_free, heap, ptr);
	}
}

#यदि (K_HEAP_MEM_POOL_आकार > 0)

K_HEAP_DEFINE(_system_heap, Z_HEAP_MIN_आकार_FOR(K_HEAP_MEM_POOL_आकार));
#परिभाषय _SYSTEM_HEAP (&_system_heap)

#यदि defined(CONFIG_SYS_HEAP_KASAN_SYSTEM)
#अन्तर्भाव <zephyr/sys/heap_kasan.h>
K_HEAP_KASAN_ENABLE(_system_heap, Z_HEAP_MIN_आकार_FOR(K_HEAP_MEM_POOL_आकार));
#यदि_अन्तः

शून्य *k_aligned_alloc(size_t align, size_t आकार)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP);

	शून्य *ret = z_alloc_helper(_SYSTEM_HEAP, align, आकार, sys_heap_aligned_alloc);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_aligned_alloc, _SYSTEM_HEAP, ret);

	प्रत्यागम ret;
}

शून्य *k_malloc(size_t आकार)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_malloc, _SYSTEM_HEAP);

	शून्य *ret = z_alloc_helper(_SYSTEM_HEAP, 0, आकार, sys_heap_noalign_alloc);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_malloc, _SYSTEM_HEAP, ret);

	प्रत्यागम ret;
}

शून्य *k_calloc(size_t nmemb, size_t आकार)
{
	शून्य *ret;
	size_t bounds;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_calloc, _SYSTEM_HEAP);

	यदि (आकार_mul_अतिप्रवाहः(nmemb, आकार, &bounds)) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, रिक्त);

		प्रत्यागम रिक्त;
	}

	ret = k_malloc(bounds);
	यदि (ret != रिक्त) {
		(शून्य)स्मृतिं_पूरय(ret, 0, bounds);
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_calloc, _SYSTEM_HEAP, ret);

	प्रत्यागम ret;
}

शून्य *k_realloc(शून्य *ptr, size_t आकार)
{
	संरचना k_heap *heap, **heap_ref;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	शून्य *ret;

	यदि (आकार == 0) {
		k_free(ptr);
		प्रत्यागम रिक्त;
	}
	यदि (ptr == रिक्त) {
		प्रत्यागम k_malloc(आकार);
	}
	heap_ref = ptr;
	ptr = --heap_ref;
	heap = *heap_ref;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_heap_sys, k_realloc, heap, ptr);

	यदि (आकार_सङ्कलय_अतिप्रवाहः(आकार, परिमाण(heap_ref), &आकार)) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, रिक्त);
		प्रत्यागम रिक्त;
	}

	/*
	 * No point calling k_heap_realloc() with K_NO_WAIT here.
	 * Better bypass it and go directly to sys_heap_realloc() instead.
	 */
	कुञ्चिका = k_spin_तालकम्(&heap->तालकम्);
	ret = sys_heap_realloc(&heap->heap, ptr, आकार);
	k_spin_unlock(&heap->तालकम्, कुञ्चिका);

	यदि (ret != रिक्त) {
		heap_ref = ret;
		ret = ++heap_ref;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_heap_sys, k_realloc, heap, ptr, ret);

	प्रत्यागम ret;
}

शून्य k_thread_system_pool_नियोजय(संरचना k_thread *thread)
{
	thread->resource_pool = _SYSTEM_HEAP;
}
#अन्यथा
#परिभाषय _SYSTEM_HEAP	रिक्त
#यदि_अन्तः /* K_HEAP_MEM_POOL_SIZE */

स्थिर शून्य *z_thread_alloc_helper(size_t align, size_t आकार,
				   sys_heap_allocator_t sys_heap_allocator)
{
	शून्य *ret;
	संरचना k_heap *heap;

	यदि (k_is_अन्तर्_isr()) {
		heap = _SYSTEM_HEAP;
	} अन्यथा {
		heap = _current->resource_pool;
	}

	यदि (heap != रिक्त) {
		ret = z_alloc_helper(heap, align, आकार, sys_heap_allocator);
	} अन्यथा {
		ret = रिक्त;
	}

	प्रत्यागम ret;
}

शून्य *z_thread_aligned_alloc(size_t align, size_t आकार)
{
	प्रत्यागम z_thread_alloc_helper(align, आकार, sys_heap_aligned_alloc);
}

शून्य *z_thread_malloc(size_t आकार)
{
	प्रत्यागम z_thread_alloc_helper(0, आकार, sys_heap_noalign_alloc);
}

/*
 * There is no guarantee that functions in kheap.c are being used,
 * meaning the linker may not pull it in at all. Yet we need the
 * static heaps to be initialized and that is done in that file.
 * Make sure it is pulled in at least for that; the rest will be GC'd.
 */
स्थिर शून्य *ध्रुव __used kheap_ref = k_heap_init;
