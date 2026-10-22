/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <zephyr/device.h>
#अन्तर्भाव <zephyr/sys/atomic.h>
#अन्तर्भाव <zephyr/sys/iterable_sections.h>
#अन्तर्भाव <zephyr/sys/kobject.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <zephyr/pm/device_runtime.h>

पूर्णाङ्क do_device_init(ध्रुव संरचना device *dev)
{
	पूर्णाङ्क rc = 0;

	जइ (dev->ops.init != रिक्त) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		जइ (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			जइ (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			जइ (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	जइ (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(शून्य)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	प्रत्यागम -rc;
}

पूर्णाङ्क z_impl_device_init(ध्रुव संरचना device *dev)
{
	जइ (dev->state->initialized) {
		प्रत्यागम -EALREADY;
	}

	प्रत्यागम do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_device_init(ध्रुव संरचना device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	प्रत्यागम z_impl_device_init(dev);
}
#अन्तर्भाव <zephyr/syscalls/device_init_mrsh.c>
#endif


ध्रुव संरचना device *z_impl_device_get_binding(ध्रुव वर्ण *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	जइ ((name == रिक्त) || (name[0] == '\0')) {
		प्रत्यागम रिक्त;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		जइ ((dev->name == name) || (strcmp(name, dev->name) == 0)) {
			प्रत्यागम z_impl_device_is_ready(dev) ? dev : रिक्त;
		}
	}

	प्रत्यागम रिक्त;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ ध्रुव संरचना device *z_vrfy_device_get_binding(ध्रुव वर्ण *name)
{
	वर्ण name_copy[Z_DEVICE_MAX_NAME_LEN];

	जइ (k_usermode_string_copy(name_copy, name, परिमाण(name_copy))
	    != 0) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम z_impl_device_get_binding(name_copy);
}
#अन्तर्भाव <zephyr/syscalls/device_get_binding_mrsh.c>

स्थिर अन्तःस्थ bool z_vrfy_device_is_ready(ध्रुव संरचना device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	जइ (dev == रिक्त) {
		प्रत्यागम false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	प्रत्यागम z_impl_device_is_ready(dev);
}
#अन्तर्भाव <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
ध्रुव संरचना device *z_impl_device_get_by_dt_nodelabel(ध्रुव वर्ण *nodelabel)
{
	/* For consistency with device_get_binding(). */
	जइ ((nodelabel == रिक्त) || (nodelabel[0] == '\0')) {
		प्रत्यागम रिक्त;
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
		ध्रुव संरचना device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		जइ (!z_impl_device_is_ready(dev) || nl == रिक्त) {
			अनुवर्त;
		}

		प्रति (size_t i = 0; i < nl->अङ्कः_nodelabels; i++) {
			ध्रुव वर्ण *dev_nodelabel = nl->nodelabels[i];

			जइ (strcmp(nodelabel, dev_nodelabel) == 0) {
				प्रत्यागम dev;
			}
		}
	}

	प्रत्यागम रिक्त;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ ध्रुव संरचना device *z_vrfy_device_get_by_dt_nodelabel(ध्रुव वर्ण *nodelabel)
{
	वर्ण nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	जइ (k_usermode_string_copy(nl_copy, (वर्ण *)nodelabel, परिमाण(nl_copy)) != 0) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#अन्तर्भाव <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

size_t z_device_get_all_static(संरचना device ध्रुव **devices)
{
	size_t cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_गणनम्(device, &cnt);

	प्रत्यागम cnt;
}

bool z_impl_device_is_ready(ध्रुव संरचना device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	जइ (dev == रिक्त) {
		प्रत्यागम false;
	}

	प्रत्यागम dev->state->initialized;
}

पूर्णाङ्क z_impl_device_deinit(ध्रुव संरचना device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	पूर्णाङ्क ret;

	जइ (!dev->state->initialized) {
		प्रत्यागम -EPERM;
	}

	जइ (dev->ops.deinit == रिक्त) {
		प्रत्यागम -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	जइ (ret < 0) {
		प्रत्यागम ret;
	}

	dev->state->initialized = false;

	प्रत्यागम 0;
#अण्णहा
	ARG_UNUSED(dev);
	प्रत्यागम -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_device_deinit(ध्रुव संरचना device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	प्रत्यागम z_impl_device_deinit(dev);
}
#अन्तर्भाव <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

स्थिर पूर्णाङ्क device_visitor(ध्रुव device_handle_t *handles,
			   size_t handle_गणनम्,
			   device_visitor_callback_t visitor_cb,
			   शून्य *context)
{
	/* Iterate over fixed devices */
	प्रति (size_t i = 0; i < handle_गणनम्; ++i) {
		device_handle_t dh = handles[i];
		ध्रुव संरचना device *rdev = device_from_handle(dh);
		पूर्णाङ्क rc = visitor_cb(rdev, context);

		जइ (rc < 0) {
			प्रत्यागम rc;
		}
	}

	प्रत्यागम handle_गणनम्;
}

पूर्णाङ्क device_required_foreach(ध्रुव संरचना device *dev,
			    device_visitor_callback_t visitor_cb,
			    शून्य *context)
{
	size_t handle_गणनम् = 0;
	ध्रुव device_handle_t *handles = device_required_handles_get(dev, &handle_गणनम्);

	प्रत्यागम device_visitor(handles, handle_गणनम्, visitor_cb, context);
}

पूर्णाङ्क device_supported_foreach(ध्रुव संरचना device *dev,
			     device_visitor_callback_t visitor_cb,
			     शून्य *context)
{
	size_t handle_गणनम् = 0;
	ध्रुव device_handle_t *handles = device_supported_handles_get(dev, &handle_गणनम्);

	प्रत्यागम device_visitor(handles, handle_गणनम्, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
