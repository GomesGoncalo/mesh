#include <control/control.h>

struct mesh_dev_ctrl *mesh_control_init(void) {
	return NULL;
}

void mesh_control_deinit(struct mesh_dev_ctrl *ctrl) {
	if (!ctrl)
		return;

	kfree(ctrl);
}