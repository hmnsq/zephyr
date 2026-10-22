/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/device.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/atomic.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/iterable_sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/kobject.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/pm/device_runtime.h>

ਪੂਰਨ do_device_init(ਅਚਲ ਬਣਤਰ device *dev)
{
	ਪੂਰਨ rc = 0;

	ਜੇ (dev->ops.init != ਨਿਰਾ) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		ਜੇ (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			ਜੇ (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			ਜੇ (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	ਜੇ (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(ਖਾਲੀ)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	ਵਾਪਸ -rc;
}

ਪੂਰਨ z_impl_device_init(ਅਚਲ ਬਣਤਰ device *dev)
{
	ਜੇ (dev->state->initialized) {
		ਵਾਪਸ -EALREADY;
	}

	ਵਾਪਸ do_device_init(dev);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_device_init(ਅਚਲ ਬਣਤਰ device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ਵਾਪਸ z_impl_device_init(dev);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/device_init_mrsh.c>
#ਜੇ_ਅੰਤ


ਅਚਲ ਬਣਤਰ device *z_impl_device_get_binding(ਅਚਲ ਅੱਖਰ *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	ਜੇ ((name == ਨਿਰਾ) || (name[0] == '\0')) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		ਜੇ ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			ਵਾਪਸ z_impl_device_is_ready(dev) ? dev : ਨਿਰਾ;
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਅਚਲ ਬਣਤਰ device *z_vrfy_device_get_binding(ਅਚਲ ਅੱਖਰ *name)
{
	ਅੱਖਰ name_copy[Z_DEVICE_MAX_NAME_LEN];

	ਜੇ (k_usermode_string_copy(name_copy, name, ਆਕਾਰ(name_copy))
	    != 0) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ z_impl_device_get_binding(name_copy);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/device_get_binding_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_vrfy_device_is_ready(ਅਚਲ ਬਣਤਰ device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	ਜੇ (dev == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ਵਾਪਸ z_impl_device_is_ready(dev);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/device_is_ready_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEVICE_DT_METADATA
ਅਚਲ ਬਣਤਰ device *z_impl_device_get_by_dt_nodelabel(ਅਚਲ ਅੱਖਰ *nodelabel)
{
	/* For consistency with device_get_binding(). */
	ਜੇ ((nodelabel == ਨਿਰਾ) || (nodelabel[0] == '\0')) {
		ਵਾਪਸ ਨਿਰਾ;
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
		ਅਚਲ ਬਣਤਰ device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		ਜੇ (!z_impl_device_is_ready(dev) || nl == ਨਿਰਾ) {
			ਜਾਰੀ;
		}

		ਲਈ (size_t i = 0; i < nl->ਅੰਕ_nodelabels; i++) {
			ਅਚਲ ਅੱਖਰ *dev_nodelabel = nl->nodelabels[i];

			ਜੇ (strcmp(nodelabel, dev_nodelabel) == 0) {
				ਵਾਪਸ dev;
			}
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਅਚਲ ਬਣਤਰ device *z_vrfy_device_get_by_dt_nodelabel(ਅਚਲ ਅੱਖਰ *nodelabel)
{
	ਅੱਖਰ nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	ਜੇ (k_usermode_string_copy(nl_copy, (ਅੱਖਰ *)nodelabel, ਆਕਾਰ(nl_copy)) != 0) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
#ਜੇ_ਅੰਤ /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(ਬਣਤਰ device ਅਚਲ **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_ਗਿਣਤੀ(device, &cnt);

	ਵਾਪਸ cnt;
}

bool z_impl_device_is_ready(ਅਚਲ ਬਣਤਰ device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	ਜੇ (dev == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ dev->state->initialized;
}

ਪੂਰਨ z_impl_device_deinit(ਅਚਲ ਬਣਤਰ device *dev)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEVICE_DEINIT_SUPPORT
	ਪੂਰਨ ret;

	ਜੇ (!dev->state->initialized) {
		ਵਾਪਸ -EPERM;
	}

	ਜੇ (dev->ops.deinit == ਨਿਰਾ) {
		ਵਾਪਸ -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	ਜੇ (ret < 0) {
		ਵਾਪਸ ret;
	}

	dev->state->initialized = false;

	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(dev);
	ਵਾਪਸ -ENOTSUP;
#ਜੇ_ਅੰਤ /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_device_deinit(ਅਚਲ ਬਣਤਰ device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ਵਾਪਸ z_impl_device_deinit(dev);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/device_deinit_mrsh.c>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEVICE_DEPS

ਸਥਿਰ ਪੂਰਨ device_visitor(ਅਚਲ device_handle_t *handles,
			   size_t handle_ਗਿਣਤੀ,
			   device_visitor_callback_t visitor_cb,
			   ਖਾਲੀ *context)
{
	/* Iterate over fixed devices */
	ਲਈ (size_t i = 0; i < handle_ਗਿਣਤੀ; ++i) {
		device_handle_t dh = handles[i];
		ਅਚਲ ਬਣਤਰ device *rdev = device_from_handle(dh);
		ਪੂਰਨ rc = visitor_cb(rdev, context);

		ਜੇ (rc < 0) {
			ਵਾਪਸ rc;
		}
	}

	ਵਾਪਸ handle_ਗਿਣਤੀ;
}

ਪੂਰਨ device_required_foreach(ਅਚਲ ਬਣਤਰ device *dev,
			    device_visitor_callback_t visitor_cb,
			    ਖਾਲੀ *context)
{
	size_t handle_ਗਿਣਤੀ = 0;
	ਅਚਲ device_handle_t *handles = device_required_handles_get(dev, &handle_ਗਿਣਤੀ);

	ਵਾਪਸ device_visitor(handles, handle_ਗਿਣਤੀ, visitor_cb, context);
}

ਪੂਰਨ device_supported_foreach(ਅਚਲ ਬਣਤਰ device *dev,
			     device_visitor_callback_t visitor_cb,
			     ਖਾਲੀ *context)
{
	size_t handle_ਗਿਣਤੀ = 0;
	ਅਚਲ device_handle_t *handles = device_supported_handles_get(dev, &handle_ਗਿਣਤੀ);

	ਵਾਪਸ device_visitor(handles, handle_ਗਿਣਤੀ, visitor_cb, context);
}

#ਜੇ_ਅੰਤ /* CONFIG_DEVICE_DEPS */
