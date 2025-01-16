/**************************************************************************/
/*  pin_joint_2d.h                                                        */
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

#ifndef WHEEL_JOINT_2D_H
#define WHEEL_JOINT_2D_H

#include "scene/2d/physics/joints/joint_2d.h"
#include "servers/physics_server_2d.h"

class PhysicsBody2D;

class WheelJoint2D : public Joint2D {
	GDCLASS(WheelJoint2D, Joint2D);

	real_t x_stiffness = 20.0;
	real_t x_damping = 1.0;
	real_t x_lower_limit = 0.0;
	real_t x_upper_limit = 0.0;

	real_t y_stiffness = 20.0;
	real_t y_damping = 1.0;
	real_t y_lower_limit = 0.0;
	real_t y_upper_limit = 0.0;

	bool x_enable_limits = false;
	bool y_enable_limits = false;

protected:
	void _notification(int p_what);
	virtual void _configure_joint(RID p_joint, PhysicsBody2D *body_a, PhysicsBody2D *body_b) override;
	static void _bind_methods();

public:
	void set_param(real_t &member_variable, real_t p_value, PhysicsServer2D::WheelJointParam param);
	void set_flag(bool &member_variable, bool p_enabled, PhysicsServer2D::WheelJointFlag param);
	
	///////// Parameters

	void set_x_stiffness(real_t p_value);
	real_t get_x_stiffness() const;

	void set_x_lower_limit(real_t p_value);
	real_t get_x_lower_limit() const;

	void set_x_upper_limit(real_t p_value);
	real_t get_x_upper_limit() const;

	void set_x_damping(real_t p_value);
	real_t get_x_damping() const;

	void set_y_lower_limit(real_t p_value);
	real_t get_y_lower_limit() const;

	void set_y_upper_limit(real_t p_value);
	real_t get_y_upper_limit() const;

	void set_y_stiffness(real_t p_value);
	real_t get_y_stiffness() const;

	void set_y_damping(real_t p_value);
	real_t get_y_damping() const;

	///////// Flags

	void set_x_enable_limits(bool p_enabled);
	bool get_x_enable_limits() const;

	void set_y_enable_limits(bool p_enabled);
	bool get_y_enable_limits() const;

	WheelJoint2D();
};

#endif // WHEEL_JOINT_2D_H
