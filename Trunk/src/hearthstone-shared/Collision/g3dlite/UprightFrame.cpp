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

#include "UprightFrame.h"

namespace G3D {

UprightFrame::UprightFrame(const CoordinateFrame& cframe) {
	Vector3 look = cframe.lookVector();

	yaw = G3D::pi() + atan2(look.x, look.z);
	pitch = asin(look.y);

	translation = cframe.translation;
}

	
CoordinateFrame UprightFrame::toCoordinateFrame() const {
	CoordinateFrame cframe;

	Matrix3 P(Matrix3::fromAxisAngle(Vector3::unitX(), pitch));
	Matrix3 Y(Matrix3::fromAxisAngle(Vector3::unitY(), yaw));

	cframe.rotation = Y * P;
	cframe.translation = translation;

	return cframe;
}


UprightFrame UprightFrame::operator+(const UprightFrame& other) const {
	return UprightFrame(translation + other.translation, pitch + other.pitch, yaw + other.yaw);
}


UprightFrame UprightFrame::operator*(const float k) const {
	return UprightFrame(translation * k, pitch * k, yaw * k);
}


void UprightFrame::unwrapYaw(UprightFrame* a, int N) {
	// Use the first point to establish the wrapping convention
	for (int i = 1; i < N; ++i) {
		const float prev = a[i - 1].yaw;
		float& cur = a[i].yaw;

		// No two angles should be more than pi (i.e., 180-degrees) apart.  
		if (abs(cur - prev) > G3D::pi()) {
			// These angles must have wrapped at zero, causing them
			// to be interpolated the long way.

			// Find canonical [0, 2pi] versions of these numbers
			float p = wrap(prev, twoPi());
			float c = wrap(cur, twoPi());
			
			// Find the difference -pi < diff < pi between the current and previous values
			float diff = c - p;
			if (diff < -G3D::pi()) {
				diff += twoPi();
			} else if (diff > G3D::pi()) {
				diff -= twoPi();
			} 

			// Offset the current from the previous by the difference
			// between them.
			cur = prev + diff;
		}
	}
}

}
