/**************************************************************************/
/*  generic_3dof_joint_2d.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "generic_3dof_joint_2d.h"
#include "scene/2d/physics/physics_body_2d.h"
#include "servers/physics_server_2d.h"

void Generic3DOFJoint2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (!is_inside_tree()) {
				break;
			}

			if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
				break;
			}

			draw_line(Point2(-10, 0), Point2(+10, 0), Color(0.7, 0.6, 0.0, 0.5), 3);
			draw_line(Point2(0, -10), Point2(0, +10), Color(0.7, 0.6, 0.0, 0.5), 3);
		} break;
	}
}

void Generic3DOFJoint2D::_configure_joint(RID p_joint, PhysicsBody2D *body_a, PhysicsBody2D *body_b) {
	PhysicsServer2D::get_singleton()->joint_make_generic_3dof(p_joint, get_global_position(), body_a->get_rid(), body_b ? body_b->get_rid() : RID());

	for (int i = 0; i < PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_LINEAR_PARAM_COUNT; i++) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_param(p_joint, Vector2::AXIS_X, PhysicsServer2D::G3DOFJointLinearAxisParam(i), params_x[i]);
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_param(p_joint, Vector2::AXIS_Y, PhysicsServer2D::G3DOFJointLinearAxisParam(i), params_y[i]);
	}

	for (int i = 0; i < PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_ANGULAR_PARAM_COUNT; i++) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_angular_param(p_joint, PhysicsServer2D::G3DOFJointAngularAxisParam(i), params_angular[i]);
	}

	for (int i = 0; i < PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_COUNT; i++) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_flag(p_joint, Vector2::AXIS_X, PhysicsServer2D::G3DOFJointAxisFlag(i), flags_x[i]);
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_flag(p_joint, Vector2::AXIS_Y, PhysicsServer2D::G3DOFJointAxisFlag(i), flags_y[i]);
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_angular_flag(p_joint, PhysicsServer2D::G3DOFJointAxisFlag(i), flags_angular[i]);
	}
}

void Generic3DOFJoint2D::set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam p_param, real_t p_value) {
	ERR_FAIL_INDEX(p_param, PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_LINEAR_PARAM_COUNT);
	params_x[p_param] = p_value;
	if (is_configured()) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_param(get_rid(), Vector2::AXIS_X, PhysicsServer2D::G3DOFJointLinearAxisParam(p_param), p_value);
	}
}

real_t Generic3DOFJoint2D::get_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam p_param) const {
	ERR_FAIL_INDEX_V(p_param, PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_LINEAR_PARAM_COUNT, 0);
	return params_x[p_param];
}

void Generic3DOFJoint2D::set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam p_param, real_t p_value) {
	ERR_FAIL_INDEX(p_param, PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_LINEAR_PARAM_COUNT);
	params_y[p_param] = p_value;
	if (is_configured()) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_param(get_rid(), Vector2::AXIS_Y, PhysicsServer2D::G3DOFJointLinearAxisParam(p_param), p_value);
	}
}

real_t Generic3DOFJoint2D::get_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam p_param) const {
	ERR_FAIL_INDEX_V(p_param, PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_LINEAR_PARAM_COUNT, 0);
	return params_y[p_param];
}

void Generic3DOFJoint2D::set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam p_param, real_t p_value) {
	ERR_FAIL_INDEX(p_param, PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_ANGULAR_PARAM_COUNT);
	params_angular[p_param] = p_value;

	queue_redraw();
	if (is_configured()) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_angular_param(get_rid(), PhysicsServer2D::G3DOFJointAngularAxisParam(p_param), p_value);
	}
}

real_t Generic3DOFJoint2D::get_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam p_param) const {
	ERR_FAIL_INDEX_V(p_param, PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_ANGULAR_PARAM_COUNT, 0);
	return params_angular[p_param];
}

void Generic3DOFJoint2D::set_linear_flag_x(PhysicsServer2D::G3DOFJointAxisFlag p_flag, bool p_enabled) {
	ERR_FAIL_INDEX(p_flag, PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_COUNT);
	flags_x[p_flag] = p_enabled;

	queue_redraw();
	if (is_configured()) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_flag(get_rid(), Vector2::AXIS_X, PhysicsServer2D::G3DOFJointAxisFlag(p_flag), p_enabled);
	}
}

bool Generic3DOFJoint2D::get_linear_flag_x(PhysicsServer2D::G3DOFJointAxisFlag p_flag) const {
	ERR_FAIL_INDEX_V(p_flag, PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_COUNT, false);
	return flags_x[p_flag];
}

void Generic3DOFJoint2D::set_linear_flag_y(PhysicsServer2D::G3DOFJointAxisFlag p_flag, bool p_enabled) {
	ERR_FAIL_INDEX(p_flag, PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_COUNT);
	flags_y[p_flag] = p_enabled;

	queue_redraw();
	if (is_configured()) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_linear_flag(get_rid(), Vector2::AXIS_Y, PhysicsServer2D::G3DOFJointAxisFlag(p_flag), p_enabled);
	}
}

bool Generic3DOFJoint2D::get_linear_flag_y(PhysicsServer2D::G3DOFJointAxisFlag p_flag) const {
	ERR_FAIL_INDEX_V(p_flag, PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_COUNT, false);
	return flags_y[p_flag];
}

void Generic3DOFJoint2D::set_angular_flag(PhysicsServer2D::G3DOFJointAxisFlag p_flag, bool p_enabled) {
	ERR_FAIL_INDEX(p_flag, PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_COUNT);
	flags_angular[p_flag] = p_enabled;

	queue_redraw();
	if (is_configured()) {
		PhysicsServer2D::get_singleton()->generic_3dof_joint_set_angular_flag(get_rid(), PhysicsServer2D::G3DOFJointAxisFlag(p_flag), p_enabled);
	}
}

bool Generic3DOFJoint2D::get_angular_flag(PhysicsServer2D::G3DOFJointAxisFlag p_flag) const {
	ERR_FAIL_INDEX_V(p_flag, PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_COUNT, false);
	return flags_angular[p_flag];
}

void Generic3DOFJoint2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linear_param_x", "param", "value"), &Generic3DOFJoint2D::set_linear_param_x);
	ClassDB::bind_method(D_METHOD("get_linear_param_x", "param"), &Generic3DOFJoint2D::get_linear_param_x);
	
	ClassDB::bind_method(D_METHOD("set_linear_param_y", "param", "value"), &Generic3DOFJoint2D::set_linear_param_y);
	ClassDB::bind_method(D_METHOD("get_linear_param_y", "param"), &Generic3DOFJoint2D::get_linear_param_y);

	ClassDB::bind_method(D_METHOD("set_angular_param", "param", "value"), &Generic3DOFJoint2D::set_angular_param);
	ClassDB::bind_method(D_METHOD("get_angular_param", "param"), &Generic3DOFJoint2D::get_angular_param);

	ClassDB::bind_method(D_METHOD("set_linear_flag_x", "flag", "value"), &Generic3DOFJoint2D::set_linear_flag_x);
	ClassDB::bind_method(D_METHOD("get_linear_flag_x", "flag"), &Generic3DOFJoint2D::get_linear_flag_x);

	ClassDB::bind_method(D_METHOD("set_linear_flag_y", "flag", "value"), &Generic3DOFJoint2D::set_linear_flag_y);
	ClassDB::bind_method(D_METHOD("get_linear_flag_y", "flag"), &Generic3DOFJoint2D::get_linear_flag_y);

	ClassDB::bind_method(D_METHOD("set_angular_flag", "flag", "value"), &Generic3DOFJoint2D::set_angular_flag);
	ClassDB::bind_method(D_METHOD("get_angular_flag", "flag"), &Generic3DOFJoint2D::get_angular_flag);

	ADD_GROUP("Linear Limit", "linear_limit_");
	// Linear X-axis
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "linear_limit_x/enabled"), "set_linear_flag_x", "get_linear_flag_x", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_x/upper_distance", PROPERTY_HINT_NONE, "suffix:px"), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_UPPER_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_x/lower_distance", PROPERTY_HINT_NONE, "suffix:px"), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LOWER_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_x/softness", PROPERTY_HINT_NONE), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LIMIT_SOFTNESS);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_x/restitution", PROPERTY_HINT_NONE), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_RESTITUTION);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_x/damping", PROPERTY_HINT_NONE), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_DAMPING);

	// Linear Y-axis
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "linear_limit_y/enabled"), "set_linear_flag_y", "get_linear_flag_y", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_y/upper_distance", PROPERTY_HINT_NONE, "suffix:px"), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_UPPER_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_y/lower_distance", PROPERTY_HINT_NONE, "suffix:px"), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LOWER_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_y/softness", PROPERTY_HINT_NONE), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LIMIT_SOFTNESS);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_y/restitution", PROPERTY_HINT_NONE), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_RESTITUTION);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_limit_y/damping", PROPERTY_HINT_NONE), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_DAMPING);

	ADD_GROUP("Linear Motor", "linear_motor_");
	// Linear X-axis
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "linear_motor_x/enabled"), "set_linear_flag_x", "get_linear_flag_x", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_MOTOR);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_motor_x/target_velocity", PROPERTY_HINT_NONE, "suffix:px/s"), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_TARGET_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_motor_x/force_limit", PROPERTY_HINT_NONE), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_FORCE_LIMIT);

	// Linear Y-axis
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "linear_motor_y/enabled"), "set_linear_flag_y", "get_linear_flag_y", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_MOTOR);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_motor_y/target_velocity", PROPERTY_HINT_NONE, "suffix:px/s"), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_TARGET_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_motor_y/force_limit", PROPERTY_HINT_NONE), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_FORCE_LIMIT);

	ADD_GROUP("Linear Spring", "linear_spring_");
	// Linear X-axis
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "linear_spring_x/enabled"), "set_linear_flag_x", "get_linear_flag_x", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_SPRING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_spring_x/stiffness", PROPERTY_HINT_NONE), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_STIFFNESS);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_spring_x/damping", PROPERTY_HINT_NONE), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_spring_x/equilibrium_point", PROPERTY_HINT_NONE, "suffix:px"), "set_linear_param_x", "get_linear_param_x", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_EQUILIBRIUM_POINT);

	// Linear Y-axis
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "linear_spring_y/enabled"), "set_linear_flag_y", "get_linear_flag_y", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_SPRING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_spring_y/stiffness", PROPERTY_HINT_NONE), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_STIFFNESS);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_spring_y/damping", PROPERTY_HINT_NONE), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "linear_spring_y/equilibrium_point", PROPERTY_HINT_NONE, "suffix:px"), "set_linear_param_y", "get_linear_param_y", PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_EQUILIBRIUM_POINT);

	ADD_GROUP("Angular Limit", "angular_limit_");
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "angular_limit/enabled"), "set_angular_flag", "get_angular_flag", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_limit/lower_angle", PROPERTY_HINT_NONE, "suffix:°"), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_LOWER_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_limit/upper_angle", PROPERTY_HINT_NONE, "suffix:°"), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_UPPER_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_limit/softness", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_LIMIT_SOFTNESS);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_limit/restitution", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_RESTITUTION);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_limit/damping", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_limit/force_limit", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_FORCE_LIMIT);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_limit/erp", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_ERP);

	ADD_GROUP("Angular Motor", "angular_motor_");
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "angular_motor/enabled"), "set_angular_flag", "get_angular_flag", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_MOTOR);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_motor/target_velocity", PROPERTY_HINT_NONE, "suffix:°/s"), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_MOTOR_TARGET_VELOCITY);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_motor/force_limit", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_MOTOR_FORCE_LIMIT);

	ADD_GROUP("Angular Spring", "angular_spring_");
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "angular_spring/enabled"), "set_angular_flag", "get_angular_flag", PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_SPRING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_spring/stiffness", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_SPRING_STIFFNESS);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_spring/damping", PROPERTY_HINT_NONE), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_SPRING_DAMPING);
	ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "angular_spring/equilibrium_point", PROPERTY_HINT_NONE, "suffix:°"), "set_angular_param", "get_angular_param", PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_SPRING_EQUILIBRIUM_POINT);
}

Generic3DOFJoint2D::Generic3DOFJoint2D() {

	// Linear x params
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_UPPER_LIMIT, 0.0);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LOWER_LIMIT, 0.0);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LIMIT_SOFTNESS, 0.5);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_RESTITUTION, 0.0);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_DAMPING, 1.0);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_TARGET_VELOCITY, 0);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_FORCE_LIMIT, 0);

	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_STIFFNESS, 20.0);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_DAMPING, 1.0);
	set_linear_param_x(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_EQUILIBRIUM_POINT, 0);

	// Linear x flags
	set_linear_flag_x(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_LIMIT, false);
	set_linear_flag_x(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_MOTOR, false);
	set_linear_flag_x(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_SPRING, false);

	// Linear y params
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_UPPER_LIMIT, 0.0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LOWER_LIMIT, 0.0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_LIMIT_SOFTNESS, 0.5);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_RESTITUTION, 0.0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_DAMPING, 1.0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_TARGET_VELOCITY, 0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_MOTOR_FORCE_LIMIT, 0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_STIFFNESS, 20.0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_DAMPING, 1.0);
	set_linear_param_y(PhysicsServer2D::G3DOFJointLinearAxisParam::G3DOF_JOINT_LINEAR_SPRING_EQUILIBRIUM_POINT, 0);

	// Linear y flags
	set_linear_flag_y(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_LIMIT, false);
	set_linear_flag_y(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_MOTOR, false);
	set_linear_flag_y(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_SPRING, false);

	// Angular params
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_LOWER_LIMIT, 0.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_UPPER_LIMIT, 0.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_LIMIT_SOFTNESS, 0.5);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_RESTITUTION, 0.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_DAMPING, 1.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_MOTOR_TARGET_VELOCITY, 0.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_MOTOR_FORCE_LIMIT, 0.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_FORCE_LIMIT, 0.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_ERP, 0.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_SPRING_STIFFNESS, 20.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_SPRING_DAMPING, 1.0);
	set_angular_param(PhysicsServer2D::G3DOFJointAngularAxisParam::G3DOF_JOINT_ANGULAR_SPRING_EQUILIBRIUM_POINT, 0.0);

	// Angular flags
	set_angular_flag(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_LIMIT, false);
	set_angular_flag(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_MOTOR, false);
	set_angular_flag(PhysicsServer2D::G3DOFJointAxisFlag::G3DOF_JOINT_FLAG_ENABLE_SPRING, false);
}
