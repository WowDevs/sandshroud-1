/*
 * Sandshroud Hearthstone
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "platform.h"
#include <stdlib.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "g3dmath.h"
#include "../../format.h"

namespace G3D {

const Vector2& Vector2::one() { 
	static const Vector2 v(1, 1); return v; 
}


const Vector2& Vector2::zero() {
	static Vector2 v(0, 0);
	return v;
}

const Vector2& Vector2::unitX() {
	static Vector2 v(1, 0);
	return v;
}

const Vector2& Vector2::unitY() {
	static Vector2 v(0, 1);
	return v;
}

const Vector2& Vector2::inf() { 
	static Vector2 v((float)G3D::finf(), (float)G3D::finf());
	return v; 
}


const Vector2& Vector2::nan() { 
	static Vector2 v((float)G3D::fnan(), (float)G3D::fnan()); 
	return v; 
}


const Vector2& Vector2::minFinite() {
	static Vector2 v(-FLT_MAX, -FLT_MAX); 
	return v; 
}


const Vector2& Vector2::maxFinite() {
	static Vector2 v(FLT_MAX, FLT_MAX); 
	return v; 
}


size_t Vector2::hashCode() const {
	unsigned int xhash = (*(int*)(void*)(&x));
	unsigned int yhash = (*(int*)(void*)(&y));

	return xhash + (yhash * 37);
}

//----------------------------------------------------------------------------

Vector2 Vector2::random(G3D::Random& r) {
	Vector2 result;

	do {
		result = Vector2(r.uniform(-1, 1), r.uniform(-1, 1));

	} while (result.squaredLength() >= 1.0f);

	result.unitize();

	return result;
}


Vector2 Vector2::operator/ (float k) const {
	return *this * (1.0f / k);
}

Vector2& Vector2::operator/= (float k) {
	this->x /= k;
	this->y /= k;
	return *this;
}

//----------------------------------------------------------------------------
float Vector2::unitize (float fTolerance) {
	float fLength = length();

	if (fLength > fTolerance) {
		float fInvLength = 1.0f / fLength;
		x *= fInvLength;
		y *= fInvLength;
	} else {
		fLength = 0.0;
	}

	return fLength;
}

//----------------------------------------------------------------------------

std::string Vector2::toString() const {
	return format("(%g, %g)", x, y);
}

// 2-char swizzles

Vector2 Vector2::xx() const  { return Vector2	   (x, x); }
Vector2 Vector2::yx() const  { return Vector2	   (y, x); }
Vector2 Vector2::xy() const  { return Vector2	   (x, y); }
Vector2 Vector2::yy() const  { return Vector2	   (y, y); }

// 3-char swizzles

Vector3 Vector2::xxx() const  { return Vector3	   (x, x, x); }
Vector3 Vector2::yxx() const  { return Vector3	   (y, x, x); }
Vector3 Vector2::xyx() const  { return Vector3	   (x, y, x); }
Vector3 Vector2::yyx() const  { return Vector3	   (y, y, x); }
Vector3 Vector2::xxy() const  { return Vector3	   (x, x, y); }
Vector3 Vector2::yxy() const  { return Vector3	   (y, x, y); }
Vector3 Vector2::xyy() const  { return Vector3	   (x, y, y); }
Vector3 Vector2::yyy() const  { return Vector3	   (y, y, y); }

// 4-char swizzles

Vector4 Vector2::xxxx() const  { return Vector4	   (x, x, x, x); }
Vector4 Vector2::yxxx() const  { return Vector4	   (y, x, x, x); }
Vector4 Vector2::xyxx() const  { return Vector4	   (x, y, x, x); }
Vector4 Vector2::yyxx() const  { return Vector4	   (y, y, x, x); }
Vector4 Vector2::xxyx() const  { return Vector4	   (x, x, y, x); }
Vector4 Vector2::yxyx() const  { return Vector4	   (y, x, y, x); }
Vector4 Vector2::xyyx() const  { return Vector4	   (x, y, y, x); }
Vector4 Vector2::yyyx() const  { return Vector4	   (y, y, y, x); }
Vector4 Vector2::xxxy() const  { return Vector4	   (x, x, x, y); }
Vector4 Vector2::yxxy() const  { return Vector4	   (y, x, x, y); }
Vector4 Vector2::xyxy() const  { return Vector4	   (x, y, x, y); }
Vector4 Vector2::yyxy() const  { return Vector4	   (y, y, x, y); }
Vector4 Vector2::xxyy() const  { return Vector4	   (x, x, y, y); }
Vector4 Vector2::yxyy() const  { return Vector4	   (y, x, y, y); }
Vector4 Vector2::xyyy() const  { return Vector4	   (x, y, y, y); }
Vector4 Vector2::yyyy() const  { return Vector4	   (y, y, y, y); }



} // namespace
