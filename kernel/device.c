/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/device.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/iterable_sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/kobject.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/pm/device_runtime.h>

ಪೂರ್ಣಾಂಕ do_device_init(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev)
{
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (dev->ops.init != ಖಾಲಿ) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		ಆದರೆ (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			ಆದರೆ (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			ಆದರೆ (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	ಆದರೆ (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(ಶೂನ್ಯ)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	ಹಿಂತಿರುಗಿ -rc;
}

ಪೂರ್ಣಾಂಕ z_impl_device_init(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev)
{
	ಆದರೆ (dev->state->initialized) {
		ಹಿಂತಿರುಗಿ -EALREADY;
	}

	ಹಿಂತಿರುಗಿ do_device_init(dev);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_device_init(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ಹಿಂತಿರುಗಿ z_impl_device_init(dev);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರಾಂಕ ರಚನೆ device *z_impl_device_get_binding(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	ಆದರೆ ((name == ಖಾಲಿ) || (name[0] == '\0')) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		ಆದರೆ ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			ಹಿಂತಿರುಗಿ z_impl_device_is_ready(dev) ? dev : ಖಾಲಿ;
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಸ್ಥಿರಾಂಕ ರಚನೆ device *z_vrfy_device_get_binding(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಅಕ್ಷರ name_copy[Z_DEVICE_MAX_NAME_LEN];

	ಆದರೆ (k_usermode_string_copy(name_copy, name, ಗಾತ್ರ(name_copy))
	    != 0) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ z_impl_device_get_binding(name_copy);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_get_binding_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_vrfy_device_is_ready(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	ಆದರೆ (dev == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ಹಿಂತಿರುಗಿ z_impl_device_is_ready(dev);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_is_ready_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEVICE_DT_METADATA
ಸ್ಥಿರಾಂಕ ರಚನೆ device *z_impl_device_get_by_dt_nodelabel(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nodelabel)
{
	/* For consistency with device_get_binding(). */
	ಆದರೆ ((nodelabel == ಖಾಲಿ) || (nodelabel[0] == '\0')) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
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
		ಸ್ಥಿರಾಂಕ ರಚನೆ device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		ಆದರೆ (!z_impl_device_is_ready(dev) || nl == ಖಾಲಿ) {
			ಮುಂದುವರಿ;
		}

		ಗಾಗಿ (size_t i = 0; i < nl->ಅಂಕೆ_nodelabels; i++) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dev_nodelabel = nl->nodelabels[i];

			ಆದರೆ (strcmp(nodelabel, dev_nodelabel) == 0) {
				ಹಿಂತಿರುಗಿ dev;
			}
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಸ್ಥಿರಾಂಕ ರಚನೆ device *z_vrfy_device_get_by_dt_nodelabel(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nodelabel)
{
	ಅಕ್ಷರ nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	ಆದರೆ (k_usermode_string_copy(nl_copy, (ಅಕ್ಷರ *)nodelabel, ಗಾತ್ರ(nl_copy)) != 0) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(ರಚನೆ device ಸ್ಥಿರಾಂಕ **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_ಎಣಿಕೆ(device, &cnt);

	ಹಿಂತಿರುಗಿ cnt;
}

bool z_impl_device_is_ready(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	ಆದರೆ (dev == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ dev->state->initialized;
}

ಪೂರ್ಣಾಂಕ z_impl_device_deinit(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEVICE_DEINIT_SUPPORT
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (!dev->state->initialized) {
		ಹಿಂತಿರುಗಿ -EPERM;
	}

	ಆದರೆ (dev->ops.deinit == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	ಆದರೆ (ret < 0) {
		ಹಿಂತಿರುಗಿ ret;
	}

	dev->state->initialized = false;

	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(dev);
	ಹಿಂತಿರುಗಿ -ENOTSUP;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_device_deinit(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	ಹಿಂತಿರುಗಿ z_impl_device_deinit(dev);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/device_deinit_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEVICE_DEPS

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ device_visitor(ಸ್ಥಿರಾಂಕ device_handle_t *handles,
			   size_t handle_ಎಣಿಕೆ,
			   device_visitor_callback_t visitor_cb,
			   ಶೂನ್ಯ *context)
{
	/* Iterate over fixed devices */
	ಗಾಗಿ (size_t i = 0; i < handle_ಎಣಿಕೆ; ++i) {
		device_handle_t dh = handles[i];
		ಸ್ಥಿರಾಂಕ ರಚನೆ device *rdev = device_from_handle(dh);
		ಪೂರ್ಣಾಂಕ rc = visitor_cb(rdev, context);

		ಆದರೆ (rc < 0) {
			ಹಿಂತಿರುಗಿ rc;
		}
	}

	ಹಿಂತಿರುಗಿ handle_ಎಣಿಕೆ;
}

ಪೂರ್ಣಾಂಕ device_required_foreach(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev,
			    device_visitor_callback_t visitor_cb,
			    ಶೂನ್ಯ *context)
{
	size_t handle_ಎಣಿಕೆ = 0;
	ಸ್ಥಿರಾಂಕ device_handle_t *handles = device_required_handles_get(dev, &handle_ಎಣಿಕೆ);

	ಹಿಂತಿರುಗಿ device_visitor(handles, handle_ಎಣಿಕೆ, visitor_cb, context);
}

ಪೂರ್ಣಾಂಕ device_supported_foreach(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev,
			     device_visitor_callback_t visitor_cb,
			     ಶೂನ್ಯ *context)
{
	size_t handle_ಎಣಿಕೆ = 0;
	ಸ್ಥಿರಾಂಕ device_handle_t *handles = device_supported_handles_get(dev, &handle_ಎಣಿಕೆ);

	ಹಿಂತಿರುಗಿ device_visitor(handles, handle_ಎಣಿಕೆ, visitor_cb, context);
}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEVICE_DEPS */
