/** relative rotation VRHMD
 * Copyright (C) 2017,2019 tarosuke<webmaster@tarosuke.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#pragma once

#include <toolbox/complex/complex.h>

#include "vrhmd.h"



namespace vr_core{

	class VRHMD_Relative : public VRHMD{
		VRHMD_Relative();
		VRHMD_Relative(const VRHMD_Relative&);
		void operator=(const VRHMD_Relative&);
	public:
		virtual ~VRHMD_Relative(){};

	protected:
		VRHMD_Relative(int fd, const Profile& p) : VRHMD(fd, p){};

		//TODO:今はPOSEがしている回転の合成や補正をこのモジュールでやる
		void UpdatePose(const COMPLEX<4>& rot, const VECTOR<3>& acc, double dt){
			POSE::UpdatePose(rot, acc, dt);
		};
		void UpdateMagneticField(const VECTOR<3>& mag){
			POSE::UpdateMagneticField(mag);
		};

	private:

	};

}
