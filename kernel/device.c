/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <errno.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <zephyr/device.h>
#అన్తర్భావ <zephyr/sys/atomic.h>
#అన్తర్భావ <zephyr/sys/iterable_sections.h>
#అన్తర్భావ <zephyr/sys/kobject.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/pm/device_runtime.h>

పూర్ణాంకం do_device_init(స్థిరరాశి నిర్మాణం device *dev)
{
	పూర్ణాంకం rc = 0;

	ఒకవేళ (dev->ops.init != ఖాళీ) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		ఒకవేళ (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			ఒకవేళ (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			ఒకవేళ (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	ఒకవేళ (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(శూన్య)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	తిరిగి -rc;
}

పూర్ణాంకం z_impl_device_init(స్థిరరాశి నిర్మాణం device *dev)
{
	ఒకవేళ (dev->state->initialized) {
		తిరిగి -EALREADY;
	}

	తిరిగి do_device_init(dev);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_device_init(స్థిరరాశి నిర్మాణం device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	తిరిగి z_impl_device_init(dev);
}
#అన్తర్భావ <zephyr/syscalls/device_init_mrsh.c>
#అయితే_ముగింపు


స్థిరరాశి నిర్మాణం device *z_impl_device_get_binding(స్థిరరాశి అక్షరం *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	ఒకవేళ ((name == ఖాళీ) || (name[0] == '\0')) {
		తిరిగి ఖాళీ;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		ఒకవేళ ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			తిరిగి z_impl_device_is_ready(dev) ? dev : ఖాళీ;
		}
	}

	తిరిగి ఖాళీ;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస స్థిరరాశి నిర్మాణం device *z_vrfy_device_get_binding(స్థిరరాశి అక్షరం *name)
{
	అక్షరం name_copy[Z_DEVICE_MAX_NAME_LEN];

	ఒకవేళ (k_usermode_string_copy(name_copy, name, పరిమాణం(name_copy))
	    != 0) {
		తిరిగి ఖాళీ;
	}

	తిరిగి z_impl_device_get_binding(name_copy);
}
#అన్తర్భావ <zephyr/syscalls/device_get_binding_mrsh.c>

స్థిర అంతఃవరుస bool z_vrfy_device_is_ready(స్థిరరాశి నిర్మాణం device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	ఒకవేళ (dev == ఖాళీ) {
		తిరిగి false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	తిరిగి z_impl_device_is_ready(dev);
}
#అన్తర్భావ <zephyr/syscalls/device_is_ready_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_DEVICE_DT_METADATA
స్థిరరాశి నిర్మాణం device *z_impl_device_get_by_dt_nodelabel(స్థిరరాశి అక్షరం *nodelabel)
{
	/* For consistency with device_get_binding(). */
	ఒకవేళ ((nodelabel == ఖాళీ) || (nodelabel[0] == '\0')) {
		తిరిగి ఖాళీ;
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
		స్థిరరాశి నిర్మాణం device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		ఒకవేళ (!z_impl_device_is_ready(dev) || nl == ఖాళీ) {
			కొనసాగు;
		}

		కొరకు (size_t i = 0; i < nl->అంకె_nodelabels; i++) {
			స్థిరరాశి అక్షరం *dev_nodelabel = nl->nodelabels[i];

			ఒకవేళ (strcmp(nodelabel, dev_nodelabel) == 0) {
				తిరిగి dev;
			}
		}
	}

	తిరిగి ఖాళీ;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస స్థిరరాశి నిర్మాణం device *z_vrfy_device_get_by_dt_nodelabel(స్థిరరాశి అక్షరం *nodelabel)
{
	అక్షరం nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	ఒకవేళ (k_usermode_string_copy(nl_copy, (అక్షరం *)nodelabel, పరిమాణం(nl_copy)) != 0) {
		తిరిగి ఖాళీ;
	}

	తిరిగి z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#అన్తర్భావ <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
#అయితే_ముగింపు /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(నిర్మాణం device స్థిరరాశి **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_లెక్క(device, &cnt);

	తిరిగి cnt;
}

bool z_impl_device_is_ready(స్థిరరాశి నిర్మాణం device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	ఒకవేళ (dev == ఖాళీ) {
		తిరిగి false;
	}

	తిరిగి dev->state->initialized;
}

పూర్ణాంకం z_impl_device_deinit(స్థిరరాశి నిర్మాణం device *dev)
{
#నిర్వచితమైతే CONFIG_DEVICE_DEINIT_SUPPORT
	పూర్ణాంకం ret;

	ఒకవేళ (!dev->state->initialized) {
		తిరిగి -EPERM;
	}

	ఒకవేళ (dev->ops.deinit == ఖాళీ) {
		తిరిగి -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	ఒకవేళ (ret < 0) {
		తిరిగి ret;
	}

	dev->state->initialized = false;

	తిరిగి 0;
#లేకపోతే
	ARG_వాడని(dev);
	తిరిగి -ENOTSUP;
#అయితే_ముగింపు /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_device_deinit(స్థిరరాశి నిర్మాణం device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	తిరిగి z_impl_device_deinit(dev);
}
#అన్తర్భావ <zephyr/syscalls/device_deinit_mrsh.c>
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_DEVICE_DEPS

స్థిర పూర్ణాంకం device_visitor(స్థిరరాశి device_handle_t *handles,
			   size_t handle_లెక్క,
			   device_visitor_callback_t visitor_cb,
			   శూన్య *context)
{
	/* Iterate over fixed devices */
	కొరకు (size_t i = 0; i < handle_లెక్క; ++i) {
		device_handle_t dh = handles[i];
		స్థిరరాశి నిర్మాణం device *rdev = device_from_handle(dh);
		పూర్ణాంకం rc = visitor_cb(rdev, context);

		ఒకవేళ (rc < 0) {
			తిరిగి rc;
		}
	}

	తిరిగి handle_లెక్క;
}

పూర్ణాంకం device_required_foreach(స్థిరరాశి నిర్మాణం device *dev,
			    device_visitor_callback_t visitor_cb,
			    శూన్య *context)
{
	size_t handle_లెక్క = 0;
	స్థిరరాశి device_handle_t *handles = device_required_handles_get(dev, &handle_లెక్క);

	తిరిగి device_visitor(handles, handle_లెక్క, visitor_cb, context);
}

పూర్ణాంకం device_supported_foreach(స్థిరరాశి నిర్మాణం device *dev,
			     device_visitor_callback_t visitor_cb,
			     శూన్య *context)
{
	size_t handle_లెక్క = 0;
	స్థిరరాశి device_handle_t *handles = device_supported_handles_get(dev, &handle_లెక్క);

	తిరిగి device_visitor(handles, handle_లెక్క, visitor_cb, context);
}

#అయితే_ముగింపు /* CONFIG_DEVICE_DEPS */
