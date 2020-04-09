/** Shutdown
 * Copyright (C) 2019 tarosuke<webmaster@tarosuke.net>
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
#include <X11/Xlib.h>

#include <xmodule.h>
#include <toolbox/factory/factory.h>



namespace vr_core{

	class Shutdown : public XModule{
	public:
		static XModule* New();
	private:
		static FACTORY<XModule> factory;

		bool OnXKeyEvent(const XKeyEvent&) final;
	};

	FACTORY<XModule> Shutdown::factory(New);

	XModule* Shutdown::New(){
//		new Shutdown;
		return 0;
	}

	bool Shutdown::OnXKeyEvent(const XKeyEvent& e){
		return false;
	}

}
