#pragma once

struct mesh_dev_ctrl {

};

struct mesh_dev_ctrl *mesh_dev_ctrl_init(void);
void mesh_control_deinit(struct mesh_dev_ctrl *);