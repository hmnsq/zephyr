/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <errno.h>
#شامل <stddef.h>
#شامل <string.h>
#شامل <zephyr/device.h>
#شامل <zephyr/sys/atomic.h>
#شامل <zephyr/sys/iterable_sections.h>
#شامل <zephyr/sys/kobject.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/toolchain.h>
#شامل <zephyr/pm/device_runtime.h>

صحیح do_device_init(ثابت ساخت device *dev)
{
	صحیح rc = 0;

	اگر (dev->ops.init != تہی) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		اگر (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			اگر (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			اگر (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	اگر (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(خالی)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	واپس -rc;
}

صحیح z_impl_device_init(ثابت ساخت device *dev)
{
	اگر (dev->state->initialized) {
		واپس -EALREADY;
	}

	واپس do_device_init(dev);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_device_init(ثابت ساخت device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	واپس z_impl_device_init(dev);
}
#شامل <zephyr/syscalls/device_init_mrsh.c>
#اگر_ختم


ثابت ساخت device *z_impl_device_get_binding(ثابت حرف *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	اگر ((name == تہی) || (name[0] == '\0')) {
		واپس تہی;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		اگر ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			واپس z_impl_device_is_ready(dev) ? dev : تہی;
		}
	}

	واپس تہی;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی ثابت ساخت device *z_vrfy_device_get_binding(ثابت حرف *name)
{
	حرف name_copy[Z_DEVICE_MAX_NAME_LEN];

	اگر (k_usermode_string_copy(name_copy, name, حجم(name_copy))
	    != 0) {
		واپس تہی;
	}

	واپس z_impl_device_get_binding(name_copy);
}
#شامل <zephyr/syscalls/device_get_binding_mrsh.c>

ساکن اندرونی bool z_vrfy_device_is_ready(ثابت ساخت device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	اگر (dev == تہی) {
		واپس false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	واپس z_impl_device_is_ready(dev);
}
#شامل <zephyr/syscalls/device_is_ready_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_تعریف_شدہ CONFIG_DEVICE_DT_METADATA
ثابت ساخت device *z_impl_device_get_by_dt_nodelabel(ثابت حرف *nodelabel)
{
	/* For consistency with device_get_binding(). */
	اگر ((nodelabel == تہی) || (nodelabel[0] == '\0')) {
		واپس تہی;
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
		ثابت ساخت device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		اگر (!z_impl_device_is_ready(dev) || nl == تہی) {
			جاری;
		}

		برائے (size_t i = 0; i < nl->عدد_nodelabels; i++) {
			ثابت حرف *dev_nodelabel = nl->nodelabels[i];

			اگر (strcmp(nodelabel, dev_nodelabel) == 0) {
				واپس dev;
			}
		}
	}

	واپس تہی;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی ثابت ساخت device *z_vrfy_device_get_by_dt_nodelabel(ثابت حرف *nodelabel)
{
	حرف nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	اگر (k_usermode_string_copy(nl_copy, (حرف *)nodelabel, حجم(nl_copy)) != 0) {
		واپس تہی;
	}

	واپس z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#شامل <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
#اگر_ختم /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(ساخت device ثابت **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_گنتی(device, &cnt);

	واپس cnt;
}

bool z_impl_device_is_ready(ثابت ساخت device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	اگر (dev == تہی) {
		واپس false;
	}

	واپس dev->state->initialized;
}

صحیح z_impl_device_deinit(ثابت ساخت device *dev)
{
#اگر_تعریف_شدہ CONFIG_DEVICE_DEINIT_SUPPORT
	صحیح ret;

	اگر (!dev->state->initialized) {
		واپس -EPERM;
	}

	اگر (dev->ops.deinit == تہی) {
		واپس -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	اگر (ret < 0) {
		واپس ret;
	}

	dev->state->initialized = false;

	واپس 0;
#ورنہ
	ARG_غیر_مستعمل(dev);
	واپس -ENOTSUP;
#اگر_ختم /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_device_deinit(ثابت ساخت device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	واپس z_impl_device_deinit(dev);
}
#شامل <zephyr/syscalls/device_deinit_mrsh.c>
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_DEVICE_DEPS

ساکن صحیح device_visitor(ثابت device_handle_t *handles,
			   size_t handle_گنتی,
			   device_visitor_callback_t visitor_cb,
			   خالی *context)
{
	/* Iterate over fixed devices */
	برائے (size_t i = 0; i < handle_گنتی; ++i) {
		device_handle_t dh = handles[i];
		ثابت ساخت device *rdev = device_from_handle(dh);
		صحیح rc = visitor_cb(rdev, context);

		اگر (rc < 0) {
			واپس rc;
		}
	}

	واپس handle_گنتی;
}

صحیح device_required_foreach(ثابت ساخت device *dev,
			    device_visitor_callback_t visitor_cb,
			    خالی *context)
{
	size_t handle_گنتی = 0;
	ثابت device_handle_t *handles = device_required_handles_get(dev, &handle_گنتی);

	واپس device_visitor(handles, handle_گنتی, visitor_cb, context);
}

صحیح device_supported_foreach(ثابت ساخت device *dev,
			     device_visitor_callback_t visitor_cb,
			     خالی *context)
{
	size_t handle_گنتی = 0;
	ثابت device_handle_t *handles = device_supported_handles_get(dev, &handle_گنتی);

	واپس device_visitor(handles, handle_گنتی, visitor_cb, context);
}

#اگر_ختم /* CONFIG_DEVICE_DEPS */
