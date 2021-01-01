/** Scenery
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

#include <scenery.h>

#include <syslog.h>



namespace core{

	//コマンドラインオプション--scenery
	TB::Prefs<TB::String> Scenery::path("--scenery", "");

	FACTORY<Module> Scenery::factory(Scenery::New);
	template<> Scenery::Factory* Scenery::Factory::start(0);

	Scenery::Scenery(
		const Params& params,
		const TB::Image& image) :
		Model_C(params, image){}

	Module* Scenery::New(){
		try{
			//Imageに対応するSceteryをnewする
			Scenery* scenery(Factory::New(TB::Image(path)));
			if(scenery){
				syslog(
					LOG_INFO,
					"scenery file \"%s\" has loaded.",
					static_cast<const char*>(path));
				//登録
				(*scenery).RegisterExternals();
			}else{
				syslog(
					LOG_WARNING,
					"failed to load scenery file \"%s\".",
					static_cast<const char*>(path));
			}
		}
		catch(const char* m){
			syslog(LOG_ERR, "%s", m);
		}
		catch(...){
			syslog(LOG_ERR, "Unknown exception:" __FILE__ "(%d)", __LINE__);
		}
		return 0;
	}

}
