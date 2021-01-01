/** quiTime
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
 *
 * --ouitimeコマンドライン引数で設定した時間が経ったら終了する
 */

#include <unistd.h>

#include <toolbox/thread/pthread.h>
#include <toolbox/prefs.h>
#include <toolbox/factory/factory.h>

#include <module.h>



namespace{

	class Qtime : core::Module, TB::PThread{
	private:
		static TB::Prefs<unsigned> qtime;
		static FACTORY<core::Module> factory;

		Qtime(){
			RaiseThread();
		};
		static core::Module* New(){
			new Qtime;
			return 0;
		};
		void ThreadBody(){
			sleep((unsigned)qtime);
			Quit();
		};
	};


	//コマンドラインオプション--quitime
	TB::Prefs<unsigned> Qtime::qtime("--quitime", 0, TB::CommonPrefs::nosave);

	FACTORY<core::Module> Qtime::factory(Qtime::New);
	template<> FACTORY<Qtime>* FACTORY<Qtime>::start(0);

}
