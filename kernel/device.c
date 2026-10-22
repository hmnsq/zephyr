/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <zephyr/device.h>
#அந்தர்பாவ <zephyr/sys/atomic.h>
#அந்தர்பாவ <zephyr/sys/iterable_sections.h>
#அந்தர்பாவ <zephyr/sys/kobject.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <zephyr/pm/device_runtime.h>

முழு do_device_init(மாறிலி கட்டமைப்பு device *dev)
{
	முழு rc = 0;

	எனில் (dev->ops.init != வெற்று) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		எனில் (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			எனில் (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			எனில் (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	எனில் (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(வெறுமை)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	திரும்பு -rc;
}

முழு z_impl_device_init(மாறிலி கட்டமைப்பு device *dev)
{
	எனில் (dev->state->initialized) {
		திரும்பு -EALREADY;
	}

	திரும்பு do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_device_init(மாறிலி கட்டமைப்பு device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	திரும்பு z_impl_device_init(dev);
}
#அந்தர்பாவ <zephyr/syscalls/device_init_mrsh.c>
#endif


மாறிலி கட்டமைப்பு device *z_impl_device_get_binding(மாறிலி எழுத்து *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	எனில் ((name == வெற்று) || (name[0] == '\0')) {
		திரும்பு வெற்று;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		எனில் ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			திரும்பு z_impl_device_is_ready(dev) ? dev : வெற்று;
		}
	}

	திரும்பு வெற்று;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி மாறிலி கட்டமைப்பு device *z_vrfy_device_get_binding(மாறிலி எழுத்து *name)
{
	எழுத்து name_copy[Z_DEVICE_MAX_NAME_LEN];

	எனில் (k_usermode_string_copy(name_copy, name, அளவு(name_copy))
	    != 0) {
		திரும்பு வெற்று;
	}

	திரும்பு z_impl_device_get_binding(name_copy);
}
#அந்தர்பாவ <zephyr/syscalls/device_get_binding_mrsh.c>

நிலையான உள்வரி bool z_vrfy_device_is_ready(மாறிலி கட்டமைப்பு device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	எனில் (dev == வெற்று) {
		திரும்பு false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	திரும்பு z_impl_device_is_ready(dev);
}
#அந்தர்பாவ <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
மாறிலி கட்டமைப்பு device *z_impl_device_get_by_dt_nodelabel(மாறிலி எழுத்து *nodelabel)
{
	/* For consistency with device_get_binding(). */
	எனில் ((nodelabel == வெற்று) || (nodelabel[0] == '\0')) {
		திரும்பு வெற்று;
	}

	/* Unlike device_get_binding(), which has a history of being
	 * used in application code, we don't expect
	 * device_get_by_dt_nodelabel() to be used outside of
	 * scenarios where a human is in the loop. The shell is the
	 * main expected use case. Therefore, nodelabel is probably
	 * not the same pointer as any of the entry->nodelabel
	 * elements. We therefore skip the pointer comparison that
	 * device_get_binding() does.
	 */
	STRUCT_SECTION_FOREACH(device, dev) {
		மாறிலி கட்டமைப்பு device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		எனில் (!z_impl_device_is_ready(dev) || nl == வெற்று) {
			தொடர்;
		}

		க்கு (size_t i = 0; i < nl->இலக்கம்_nodelabels; i++) {
			மாறிலி எழுத்து *dev_nodelabel = nl->nodelabels[i];

			எனில் (strcmp(nodelabel, dev_nodelabel) == 0) {
				திரும்பு dev;
			}
		}
	}

	திரும்பு வெற்று;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி மாறிலி கட்டமைப்பு device *z_vrfy_device_get_by_dt_nodelabel(மாறிலி எழுத்து *nodelabel)
{
	எழுத்து nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	எனில் (k_usermode_string_copy(nl_copy, (எழுத்து *)nodelabel, அளவு(nl_copy)) != 0) {
		திரும்பு வெற்று;
	}

	திரும்பு z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#அந்தர்பாவ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(கட்டமைப்பு device மாறிலி **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_மொத்தம்(device, &cnt);

	திரும்பு cnt;
}

bool z_impl_device_is_ready(மாறிலி கட்டமைப்பு device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	எனில் (dev == வெற்று) {
		திரும்பு false;
	}

	திரும்பு dev->state->initialized;
}

முழு z_impl_device_deinit(மாறிலி கட்டமைப்பு device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	முழு ret;

	எனில் (!dev->state->initialized) {
		திரும்பு -EPERM;
	}

	எனில் (dev->ops.deinit == வெற்று) {
		திரும்பு -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	எனில் (ret < 0) {
		திரும்பு ret;
	}

	dev->state->initialized = false;

	திரும்பு 0;
#இல்லையெனில்
	ARG_UNUSED(dev);
	திரும்பு -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_device_deinit(மாறிலி கட்டமைப்பு device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	திரும்பு z_impl_device_deinit(dev);
}
#அந்தர்பாவ <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

நிலையான முழு device_visitor(மாறிலி device_handle_t *handles,
			   size_t handle_மொத்தம்,
			   device_visitor_callback_t visitor_cb,
			   வெறுமை *context)
{
	/* Iterate over fixed devices */
	க்கு (size_t i = 0; i < handle_மொத்தம்; ++i) {
		device_handle_t dh = handles[i];
		மாறிலி கட்டமைப்பு device *rdev = device_from_handle(dh);
		முழு rc = visitor_cb(rdev, context);

		எனில் (rc < 0) {
			திரும்பு rc;
		}
	}

	திரும்பு handle_மொத்தம்;
}

முழு device_required_foreach(மாறிலி கட்டமைப்பு device *dev,
			    device_visitor_callback_t visitor_cb,
			    வெறுமை *context)
{
	size_t handle_மொத்தம் = 0;
	மாறிலி device_handle_t *handles = device_required_handles_get(dev, &handle_மொத்தம்);

	திரும்பு device_visitor(handles, handle_மொத்தம், visitor_cb, context);
}

முழு device_supported_foreach(மாறிலி கட்டமைப்பு device *dev,
			     device_visitor_callback_t visitor_cb,
			     வெறுமை *context)
{
	size_t handle_மொத்தம் = 0;
	மாறிலி device_handle_t *handles = device_supported_handles_get(dev, &handle_மொத்தம்);

	திரும்பு device_visitor(handles, handle_மொத்தம், visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
