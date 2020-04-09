/** EVDEVのmice
 * Copyright (C) 2016,2019 tarosuke<webmaster@tarosuke.net>
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

#include "mice.h"

#include <ctype.h>
#include <syslog.h>



namespace vr_core{

	const char* Mice::uidPatterns[] = { "event-mouse", 0 };
	Widget::MouseEvent Mice::GetEvent(){
		// 状態取得
		auto b(Buttons());
		for(unsigned n(0); n < numOfAxis; ++n){
			motion[n] = GetRel(n);
		}

		return Widget::MouseEvent({
			time: {},
			x : motion[0],
			y : motion[1],
			pressedButton: b.on,
			releasedButton: b.off,
			buttonState: b.state,
		});
	}

}
