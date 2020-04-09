/** XRandR manipulator
 * Copyright (C) 2017 tarosuke<webmaster@tarosuke.net>
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

#include <toolbox/container/list.h>
#include <toolbox/string.h>



class XRandR{
	XRandR(const XRandR&);
	void operator=(const XRandR&);
public:
	class Monitor : public TB::List<Monitor>::Node{
		Monitor();
		Monitor(const Monitor&);
		void operator=(const Monitor&);
	public:
		Monitor(int, const char*); //引数は子プロセスへのパイプ
		void Enable(unsigned x, unsigned y);
		void Disable();
		const char* GetName(){ return monitorName; };
	private:
		static const unsigned char monitorNameHead[];
		char edid[128];
		const char* monitorName;
		TB::String connectionName;
	};

	//画面情報取得
	XRandR();

	//モニタ取得
	static Monitor* GetMonitor(const char* name);

private:
	static const char* const propScript;
	static bool GetLine(int, char*);
	static TB::List<Monitor> monitors;
};
