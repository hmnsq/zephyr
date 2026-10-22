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

पूर्णांक do_device_init(स्थिरांक संरचना device *dev)
{
	पूर्णांक rc = 0;

	जर (dev->ops.init != रिक्त) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		जर (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			जर (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			जर (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	जर (rc == 0) {
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
	परत -rc;
}

पूर्णांक z_impl_device_init(स्थिरांक संरचना device *dev)
{
	जर (dev->state->initialized) {
		परत -EALREADY;
	}

	परत do_device_init(dev);
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_device_init(स्थिरांक संरचना device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	परत z_impl_device_init(dev);
}
#अन्तर्भाव <zephyr/syscalls/device_init_mrsh.c>
#endif


स्थिरांक संरचना device *z_impl_device_get_binding(स्थिरांक अक्षर *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	जर ((name == रिक्त) || (name[0] == '\0')) {
		परत रिक्त;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		जर ((dev->name == name) || (म_भेद(name, dev->name) == 0)) {
			परत z_impl_device_is_ready(dev) ? dev : रिक्त;
		}
	}

	परत रिक्त;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत स्थिरांक संरचना device *z_vrfy_device_get_binding(स्थिरांक अक्षर *name)
{
	अक्षर name_copy[Z_DEVICE_MAX_NAME_LEN];

	जर (k_usermode_string_copy(name_copy, name, आकार(name_copy))
	    != 0) {
		परत रिक्त;
	}

	परत z_impl_device_get_binding(name_copy);
}
#अन्तर्भाव <zephyr/syscalls/device_get_binding_mrsh.c>

स्थिर ओळीत bool z_vrfy_device_is_ready(स्थिरांक संरचना device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	जर (dev == रिक्त) {
		परत false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	परत z_impl_device_is_ready(dev);
}
#अन्तर्भाव <zephyr/syscalls/device_is_ready_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_DEVICE_DT_METADATA
स्थिरांक संरचना device *z_impl_device_get_by_dt_nodelabel(स्थिरांक अक्षर *nodelabel)
{
	/* For consistency with device_get_binding(). */
	जर ((nodelabel == रिक्त) || (nodelabel[0] == '\0')) {
		परत रिक्त;
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
		स्थिरांक संरचना device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		जर (!z_impl_device_is_ready(dev) || nl == रिक्त) {
			सुरूठेवा;
		}

		साठी (माप_प्रकार i = 0; i < nl->अंक_nodelabels; i++) {
			स्थिरांक अक्षर *dev_nodelabel = nl->nodelabels[i];

			जर (म_भेद(nodelabel, dev_nodelabel) == 0) {
				परत dev;
			}
		}
	}

	परत रिक्त;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत स्थिरांक संरचना device *z_vrfy_device_get_by_dt_nodelabel(स्थिरांक अक्षर *nodelabel)
{
	अक्षर nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	जर (k_usermode_string_copy(nl_copy, (अक्षर *)nodelabel, आकार(nl_copy)) != 0) {
		परत रिक्त;
	}

	परत z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#अन्तर्भाव <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_DEVICE_DT_METADATA */

माप_प्रकार z_device_get_all_static(संरचना device स्थिरांक **devices)
{
	माप_प्रकार cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_मोजणी(device, &cnt);

	परत cnt;
}

bool z_impl_device_is_ready(स्थिरांक संरचना device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	जर (dev == रिक्त) {
		परत false;
	}

	परत dev->state->initialized;
}

पूर्णांक z_impl_device_deinit(स्थिरांक संरचना device *dev)
{
#ifdef CONFIG_DEVICE_DEINIT_SUPPORT
	पूर्णांक ret;

	जर (!dev->state->initialized) {
		परत -EPERM;
	}

	जर (dev->ops.deinit == रिक्त) {
		परत -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	जर (ret < 0) {
		परत ret;
	}

	dev->state->initialized = false;

	परत 0;
#अन्यथा
	ARG_UNUSED(dev);
	परत -ENOTSUP;
#endif /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_device_deinit(स्थिरांक संरचना device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	परत z_impl_device_deinit(dev);
}
#अन्तर्भाव <zephyr/syscalls/device_deinit_mrsh.c>
#endif

#ifdef CONFIG_DEVICE_DEPS

स्थिर पूर्णांक device_visitor(स्थिरांक device_handle_t *handles,
			   माप_प्रकार handle_मोजणी,
			   device_visitor_callback_t visitor_cb,
			   शून्य *context)
{
	/* Iterate over fixed devices */
	साठी (माप_प्रकार i = 0; i < handle_मोजणी; ++i) {
		device_handle_t dh = handles[i];
		स्थिरांक संरचना device *rdev = device_from_handle(dh);
		पूर्णांक rc = visitor_cb(rdev, context);

		जर (rc < 0) {
			परत rc;
		}
	}

	परत handle_मोजणी;
}

पूर्णांक device_required_foreach(स्थिरांक संरचना device *dev,
			    device_visitor_callback_t visitor_cb,
			    शून्य *context)
{
	माप_प्रकार handle_मोजणी = 0;
	स्थिरांक device_handle_t *handles = device_required_handles_get(dev, &handle_मोजणी);

	परत device_visitor(handles, handle_मोजणी, visitor_cb, context);
}

पूर्णांक device_supported_foreach(स्थिरांक संरचना device *dev,
			     device_visitor_callback_t visitor_cb,
			     शून्य *context)
{
	माप_प्रकार handle_मोजणी = 0;
	स्थिरांक device_handle_t *handles = device_supported_handles_get(dev, &handle_मोजणी);

	परत device_visitor(handles, handle_मोजणी, visitor_cb, context);
}

#endif /* CONFIG_DEVICE_DEPS */
