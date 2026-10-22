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

صحیح do_device_init(ثابت ساختار device *dev)
{
	صحیح rc = 0;

	اگر (dev->ops.init != پوچ) {
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
		(تهی)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	بازگشت -rc;
}

صحیح z_impl_device_init(ثابت ساختار device *dev)
{
	اگر (dev->state->initialized) {
		بازگشت -EALREADY;
	}

	بازگشت do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_device_init(ثابت ساختار device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	بازگشت z_impl_device_init(dev);
}
#شامل <zephyr/syscalls/device_init_mrsh.c>
#endif


ثابت ساختار device *z_impl_device_get_binding(ثابت نویسه *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	اگر ((name == پوچ) || (name[0] == '\0')) {
		بازگشت پوچ;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		اگر ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			بازگشت z_impl_device_is_ready(dev) ? dev : پوچ;
		}
	}

	بازگشت پوچ;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی ثابت ساختار device *z_vrfy_device_get_binding(ثابت نویسه *name)
{
	نویسه name_copy[Z_DEVICE_MAX_NAME_LEN];

	اگر (k_usermode_string_copy(name_copy, name, اندازه(name_copy))
	    != 0) {
		بازگشت پوچ;
	}

	بازگشت z_impl_device_get_binding(name_copy);
}
#شامل <zephyr/syscalls/device_get_binding_mrsh.c>

ایستا درونخطی bool z_vrfy_device_is_ready(ثابت ساختار device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	اگر (dev == پوچ) {
		بازگشت false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	بازگشت z_impl_device_is_ready(dev);
}
#شامل <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
ثابت ساختار device *z_impl_device_get_by_dt_nodelabel(ثابت نویسه *nodelabel)
{
	/* For consistency with device_get_binding(). */
	اگر ((nodelabel == پوچ) || (nodelabel[0] == '\0')) {
		بازگشت پوچ;
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
		ثابت ساختار device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		اگر (!z_impl_device_is_ready(dev) || nl == پوچ) {
			ادامه;
		}

		برای (size_t i = 0; i < nl->رقم_nodelabels; i++) {
			ثابت نویسه *dev_nodelabel = nl->nodelabels[i];

			اگر (strcmp(nodelabel, dev_nodelabel) == 0) {
				بازگشت dev;
			}
		}
	}

	بازگشت پوچ;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی ثابت ساختار device *z_vrfy_device_get_by_dt_nodelabel(ثابت نویسه *nodelabel)
{
	نویسه nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	اگر (k_usermode_string_copy(nl_copy, (نویسه *)nodelabel, اندازه(nl_copy)) != 0) {
		بازگشت پوچ;
	}

	بازگشت z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#شامل <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(ساختار device ثابت **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_شمار(device, &cnt);

	بازگشت cnt;
}

bool z_impl_device_is_ready(ثابت ساختار device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	اگر (dev == پوچ) {
		بازگشت false;
	}

	بازگشت dev->state->initialized;
}

صحیح z_impl_device_deinit(ثابت ساختار device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	صحیح ret;

	اگر (!dev->state->initialized) {
		بازگشت -EPERM;
	}

	اگر (dev->ops.deinit == پوچ) {
		بازگشت -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	اگر (ret < 0) {
		بازگشت ret;
	}

	dev->state->initialized = false;

	بازگشت 0;
#وگرنه
	ARG_UNUSED(dev);
	بازگشت -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_device_deinit(ثابت ساختار device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	بازگشت z_impl_device_deinit(dev);
}
#شامل <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

ایستا صحیح device_visitor(ثابت device_handle_t *handles,
			   size_t handle_شمار,
			   device_visitor_callback_t visitor_cb,
			   تهی *context)
{
	/* Iterate over fixed devices */
	برای (size_t i = 0; i < handle_شمار; ++i) {
		device_handle_t dh = handles[i];
		ثابت ساختار device *rdev = device_from_handle(dh);
		صحیح rc = visitor_cb(rdev, context);

		اگر (rc < 0) {
			بازگشت rc;
		}
	}

	بازگشت handle_شمار;
}

صحیح device_required_foreach(ثابت ساختار device *dev,
			    device_visitor_callback_t visitor_cb,
			    تهی *context)
{
	size_t handle_شمار = 0;
	ثابت device_handle_t *handles = device_required_handles_get(dev, &handle_شمار);

	بازگشت device_visitor(handles, handle_شمار, visitor_cb, context);
}

صحیح device_supported_foreach(ثابت ساختار device *dev,
			     device_visitor_callback_t visitor_cb,
			     تهی *context)
{
	size_t handle_شمار = 0;
	ثابت device_handle_t *handles = device_supported_handles_get(dev, &handle_شمار);

	بازگشت device_visitor(handles, handle_شمار, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
