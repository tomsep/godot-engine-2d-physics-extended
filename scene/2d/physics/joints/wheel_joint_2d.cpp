/**************************************************************************/
/*  wheel_joint_2d.cpp                                                      */
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

#include "wheel_joint_2d.h"

#include "scene/2d/physics/physics_body_2d.h"

void WheelJoint2D::_notification(int p_what) {
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

void WheelJoint2D::_configure_joint(RID p_joint, PhysicsBody2D *body_a, PhysicsBody2D *body_b) {
	PhysicsServer2D::get_singleton()->joint_make_wheel_joint_2d(p_joint, get_global_position(), body_a->get_rid(), body_b ? body_b->get_rid() : RID());
	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_X_STIFFNESS, x_stiffness);
	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_X_DAMPING, x_damping);
	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_X_LOWER_LIMIT, x_lower_limit);
	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_X_UPPER_LIMIT, x_upper_limit);

	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_Y_STIFFNESS, y_stiffness);
	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_Y_DAMPING, y_damping);
	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_Y_LOWER_LIMIT, y_lower_limit);
	PhysicsServer2D::get_singleton()->wheel_joint_set_param(p_joint, PhysicsServer2D::WHEEL_JOINT_Y_UPPER_LIMIT, y_upper_limit);

	PhysicsServer2D::get_singleton()->wheel_joint_set_flag(p_joint, PhysicsServer2D::WHEEL_JOINT_FLAG_X_LIMITS_ENABLED, x_enable_limits);
	PhysicsServer2D::get_singleton()->wheel_joint_set_flag(p_joint, PhysicsServer2D::WHEEL_JOINT_FLAG_Y_LIMITS_ENABLED, y_enable_limits);
}

void WheelJoint2D::set_param(real_t &member_variable, real_t p_value, PhysicsServer2D::WheelJointParam param) {
    if (member_variable == p_value) {
        return;
    }
    member_variable = p_value;
    queue_redraw();
    if (is_configured()) {
        PhysicsServer2D::get_singleton()->wheel_joint_set_param(get_rid(), param, p_value);
    }
}

void WheelJoint2D::set_flag(bool &member_variable, bool p_enabled, PhysicsServer2D::WheelJointFlag param) {
    if (member_variable == p_enabled) {
        return;
    }
    member_variable = p_enabled;
    queue_redraw();
    if (is_configured()) {
        PhysicsServer2D::get_singleton()->wheel_joint_set_flag(get_rid(), param, p_enabled);
    }
}

void WheelJoint2D::set_x_stiffness(real_t p_value) {
    set_param(x_stiffness, p_value, PhysicsServer2D::WHEEL_JOINT_X_STIFFNESS);
}

real_t WheelJoint2D::get_x_stiffness() const {
    return x_stiffness;
}

void WheelJoint2D::set_x_damping(real_t p_value) {
    set_param(x_damping, p_value, PhysicsServer2D::WHEEL_JOINT_X_DAMPING);
}

real_t WheelJoint2D::get_x_damping() const {
    return x_damping;
}

void WheelJoint2D::set_x_lower_limit(real_t p_value) {
    set_param(x_lower_limit, p_value, PhysicsServer2D::WHEEL_JOINT_X_LOWER_LIMIT);
}

real_t WheelJoint2D::get_x_lower_limit() const {
    return x_lower_limit;
}

void WheelJoint2D::set_x_upper_limit(real_t p_value) {
    set_param(x_upper_limit, p_value, PhysicsServer2D::WHEEL_JOINT_X_UPPER_LIMIT);
}

real_t WheelJoint2D::get_x_upper_limit() const {
    return x_upper_limit;
}

void WheelJoint2D::set_y_stiffness(real_t p_value) {
    set_param(y_stiffness, p_value, PhysicsServer2D::WHEEL_JOINT_Y_STIFFNESS);
}

real_t WheelJoint2D::get_y_stiffness() const {
    return y_stiffness;
}

void WheelJoint2D::set_y_damping(real_t p_value) {
    set_param(y_damping, p_value, PhysicsServer2D::WHEEL_JOINT_Y_DAMPING);
}

real_t WheelJoint2D::get_y_damping() const {
    return y_damping;
}

void WheelJoint2D::set_y_lower_limit(real_t p_value) {
    set_param(y_lower_limit, p_value, PhysicsServer2D::WHEEL_JOINT_Y_LOWER_LIMIT);
}

real_t WheelJoint2D::get_y_lower_limit() const {
    return y_lower_limit;
}

void WheelJoint2D::set_y_upper_limit(real_t p_value) {
    set_param(y_upper_limit, p_value, PhysicsServer2D::WHEEL_JOINT_Y_UPPER_LIMIT);
}

real_t WheelJoint2D::get_y_upper_limit() const {
    return y_upper_limit;
}

void WheelJoint2D::set_x_enable_limits(bool p_enabled) {
    set_flag(x_enable_limits, p_enabled, PhysicsServer2D::WHEEL_JOINT_FLAG_X_LIMITS_ENABLED);
}

bool WheelJoint2D::get_x_enable_limits() const {
    return x_enable_limits;
}

void WheelJoint2D::set_y_enable_limits(bool p_enabled) {
    set_flag(y_enable_limits, p_enabled, PhysicsServer2D::WHEEL_JOINT_FLAG_Y_LIMITS_ENABLED);
}

bool WheelJoint2D::get_y_enable_limits() const {
    return y_enable_limits;
}
void WheelJoint2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_x_stiffness", "x_stiffness"), &WheelJoint2D::set_x_stiffness);
	ClassDB::bind_method(D_METHOD("get_x_stiffness"), &WheelJoint2D::get_x_stiffness);

	ClassDB::bind_method(D_METHOD("set_x_damping", "x_damping"), &WheelJoint2D::set_x_damping);
	ClassDB::bind_method(D_METHOD("get_x_damping"), &WheelJoint2D::get_x_damping);

	ClassDB::bind_method(D_METHOD("set_x_lower_limit", "x_lower_limit"), &WheelJoint2D::set_x_lower_limit);
	ClassDB::bind_method(D_METHOD("get_x_lower_limit"), &WheelJoint2D::get_x_lower_limit);

	ClassDB::bind_method(D_METHOD("set_x_upper_limit", "x_lower_limit"), &WheelJoint2D::set_x_upper_limit);
	ClassDB::bind_method(D_METHOD("get_x_upper_limit"), &WheelJoint2D::get_x_upper_limit);

	ClassDB::bind_method(D_METHOD("set_y_stiffness", "y_stiffness"), &WheelJoint2D::set_y_stiffness);
	ClassDB::bind_method(D_METHOD("get_y_stiffness"), &WheelJoint2D::get_y_stiffness);

	ClassDB::bind_method(D_METHOD("set_y_damping", "y_damping"), &WheelJoint2D::set_y_damping);
	ClassDB::bind_method(D_METHOD("get_y_damping"), &WheelJoint2D::get_y_damping);

	ClassDB::bind_method(D_METHOD("set_y_lower_limit", "y_lower_limit"), &WheelJoint2D::set_y_lower_limit);
	ClassDB::bind_method(D_METHOD("get_y_lower_limit"), &WheelJoint2D::get_y_lower_limit);

	ClassDB::bind_method(D_METHOD("set_y_upper_limit", "y_lower_limit"), &WheelJoint2D::set_y_upper_limit);
	ClassDB::bind_method(D_METHOD("get_y_upper_limit"), &WheelJoint2D::get_y_upper_limit);

	// Flags
	ClassDB::bind_method(D_METHOD("set_x_enable_limits", "x_enable_limits"), &WheelJoint2D::set_x_enable_limits);
	ClassDB::bind_method(D_METHOD("get_x_enable_limits"), &WheelJoint2D::get_x_enable_limits);

	ClassDB::bind_method(D_METHOD("set_y_enable_limits", "y_enable_limits"), &WheelJoint2D::set_y_enable_limits);
	ClassDB::bind_method(D_METHOD("get_y_enable_limits"), &WheelJoint2D::get_y_enable_limits);

	ADD_GROUP("X", "x_");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "x_stiffness"), "set_x_stiffness", "get_x_stiffness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "x_damping"), "set_x_damping", "get_x_damping");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "x_enable_limits"), "set_x_enable_limits", "get_x_enable_limits");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "x_lower_limit"), "set_x_lower_limit", "get_x_lower_limit");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "x_upper_limit"), "set_x_upper_limit", "get_x_upper_limit");

	ADD_GROUP("Y", "y_");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "y_stiffness"), "set_y_stiffness", "get_y_stiffness");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "y_damping"), "set_y_damping", "get_y_damping");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "y_enable_limits"), "set_y_enable_limits", "get_y_enable_limits");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "y_lower_limit"), "set_y_lower_limit", "get_y_lower_limit");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "y_upper_limit"), "set_y_upper_limit", "get_y_upper_limit");
}

WheelJoint2D::WheelJoint2D() {
}
