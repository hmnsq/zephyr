/*
 * Copyright (c) 2015-2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <errno.h>
#समावेश <stddef.h>
#समावेश <string.h>
#समावेश <zephyr/device.h>
#समावेश <zephyr/sys/atomic.h>
#समावेश <zephyr/sys/iterable_sections.h>
#समावेश <zephyr/sys/kobject.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/pm/device_runtime.h>

पूर्णांक do_device_init(स्थिर काष्ठा device *dev)
{
	पूर्णांक rc = 0;

	अगर (dev->ops.init != शून्य) {
		rc = dev->ops.init(dev);
		/* If initialization failed, record in dev->state->init_res
		 * the POSITIVE value of the resulting errno
		 */
		अगर (rc != 0) {
			/* device's init function should return:
			 *   0 on success
			 *   a negative value on failure (-errno)
			 * errno value maps to an uint8_t range as of now.
			 */
			__ASSERT(rc >= -UINT8_MAX && rc < 0, "device %s init: invalid error (%d)",
				 dev->name, rc);

			अगर (rc < 0) {
				rc = -rc;
			}
			/* handle error value overflow in production
			 * this is likely a bug in the device's init function. Signals it
			 */
			अगर (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
		}

		/* Always record the result: a device that failed to initialize
		 * earlier must not keep that error once it initializes
		 * successfully.
		 */
		dev->state->init_res = rc;
	}

	अगर (rc == 0) {
		/* A failed init leaves the device uninitialized, so it can be
		 * retried by device_init() and nothing can be released by
		 * device_deinit().
		 */
		dev->state->initialized = true;

		/* Run automatic device runtime enablement */
		(व्योम)pm_device_runtime_auto_enable(dev);
	}

	/* here, the value of rc is either 0 or +errno
	 * flip the sign to return a negative value on failure as expected
	 */
	वापस -rc;
}

पूर्णांक z_impl_device_init(स्थिर काष्ठा device *dev)
{
	अगर (dev->state->initialized) {
		वापस -EALREADY;
	}

	वापस do_device_init(dev);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_device_init(स्थिर काष्ठा device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	वापस z_impl_device_init(dev);
}
#समावेश <zephyr/syscalls/device_init_mrsh.c>
#पूर्ण_अगर


स्थिर काष्ठा device *z_impl_device_get_binding(स्थिर अक्षर *name)
{
	/* A null string identifies no device.  So does an empty
	 * string.
	 */
	अगर ((name == शून्य) || (name[0] == '\0')) {
		वापस शून्य;
	}

	/* Return NULL if the device matching 'name' is not ready. */
	STRUCT_SECTION_FOREACH(device, dev) {
		अगर ((dev->name == name) || (म_भेद(name, dev->name) == 0)) {
			वापस z_impl_device_is_ready(dev) ? dev : शून्य;
		}
	}

	वापस शून्य;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत स्थिर काष्ठा device *z_vrfy_device_get_binding(स्थिर अक्षर *name)
{
	अक्षर name_copy[Z_DEVICE_MAX_NAME_LEN];

	अगर (k_usermode_string_copy(name_copy, name, माप(name_copy))
	    != 0) {
		वापस शून्य;
	}

	वापस z_impl_device_get_binding(name_copy);
}
#समावेश <zephyr/syscalls/device_get_binding_mrsh.c>

अटल अंतरभूत bool z_vrfy_device_is_ready(स्थिर काष्ठा device *dev)
{
	/* NULL is a documented argument, not an invalid kernel object */
	अगर (dev == शून्य) {
		वापस false;
	}

	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	वापस z_impl_device_is_ready(dev);
}
#समावेश <zephyr/syscalls/device_is_ready_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

#अगर_घोषित CONFIG_DEVICE_DT_METADATA
स्थिर काष्ठा device *z_impl_device_get_by_dt_nodelabel(स्थिर अक्षर *nodelabel)
{
	/* For consistency with device_get_binding(). */
	अगर ((nodelabel == शून्य) || (nodelabel[0] == '\0')) {
		वापस शून्य;
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
		स्थिर काष्ठा device_dt_nodelabels *nl = device_get_dt_nodelabels(dev);

		अगर (!z_impl_device_is_ready(dev) || nl == शून्य) {
			जारी;
		}

		क्रम (माप_प्रकार i = 0; i < nl->अंक_nodelabels; i++) {
			स्थिर अक्षर *dev_nodelabel = nl->nodelabels[i];

			अगर (म_भेद(nodelabel, dev_nodelabel) == 0) {
				वापस dev;
			}
		}
	}

	वापस शून्य;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत स्थिर काष्ठा device *z_vrfy_device_get_by_dt_nodelabel(स्थिर अक्षर *nodelabel)
{
	अक्षर nl_copy[Z_DEVICE_MAX_NODELABEL_LEN];

	अगर (k_usermode_string_copy(nl_copy, (अक्षर *)nodelabel, माप(nl_copy)) != 0) {
		वापस शून्य;
	}

	वापस z_impl_device_get_by_dt_nodelabel(nl_copy);
}
#समावेश <zephyr/syscalls/device_get_by_dt_nodelabel_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
#पूर्ण_अगर /* CONFIG_DEVICE_DT_METADATA */

माप_प्रकार z_device_get_all_static(काष्ठा device स्थिर **devices)
{
	माप_प्रकार cnt;

	STRUCT_SECTION_GET(device, 0, devices);
	STRUCT_SECTION_गिनती(device, &cnt);

	वापस cnt;
}

bool z_impl_device_is_ready(स्थिर काष्ठा device *dev)
{
	/*
	 * if an invalid device pointer is passed as argument, this call
	 * reports the `device` as not ready for usage.
	 */
	अगर (dev == शून्य) {
		वापस false;
	}

	वापस dev->state->initialized;
}

पूर्णांक z_impl_device_deinit(स्थिर काष्ठा device *dev)
{
#अगर_घोषित CONFIG_DEVICE_DEINIT_SUPPORT
	पूर्णांक ret;

	अगर (!dev->state->initialized) {
		वापस -EPERM;
	}

	अगर (dev->ops.deinit == शून्य) {
		वापस -ENOTSUP;
	}

	ret = dev->ops.deinit(dev);
	अगर (ret < 0) {
		वापस ret;
	}

	dev->state->initialized = false;

	वापस 0;
#अन्यथा
	ARG_UNUSED(dev);
	वापस -ENOTSUP;
#पूर्ण_अगर /* CONFIG_DEVICE_DEINIT_SUPPORT */
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_device_deinit(स्थिर काष्ठा device *dev)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(dev, K_OBJ_DRIVER_ANY));

	वापस z_impl_device_deinit(dev);
}
#समावेश <zephyr/syscalls/device_deinit_mrsh.c>
#पूर्ण_अगर

#अगर_घोषित CONFIG_DEVICE_DEPS

अटल पूर्णांक device_visitor(स्थिर device_handle_t *handles,
			   माप_प्रकार handle_गिनती,
			   device_visitor_callback_t visitor_cb,
			   व्योम *context)
{
	/* Iterate over fixed devices */
	क्रम (माप_प्रकार i = 0; i < handle_गिनती; ++i) {
		device_handle_t dh = handles[i];
		स्थिर काष्ठा device *rdev = device_from_handle(dh);
		पूर्णांक rc = visitor_cb(rdev, context);

		अगर (rc < 0) {
			वापस rc;
		}
	}

	वापस handle_गिनती;
}

पूर्णांक device_required_foreach(स्थिर काष्ठा device *dev,
			    device_visitor_callback_t visitor_cb,
			    व्योम *context)
{
	माप_प्रकार handle_गिनती = 0;
	स्थिर device_handle_t *handles = device_required_handles_get(dev, &handle_गिनती);

	वापस device_visitor(handles, handle_गिनती, visitor_cb, context);
}

पूर्णांक device_supported_foreach(स्थिर काष्ठा device *dev,
			     device_visitor_callback_t visitor_cb,
			     व्योम *context)
{
	माप_प्रकार handle_गिनती = 0;
	स्थिर device_handle_t *handles = device_supported_handles_get(dev, &handle_गिनती);

	वापस device_visitor(handles, handle_गिनती, visitor_cb, context);
}

#पूर्ण_अगर /* CONFIG_DEVICE_DEPS */
