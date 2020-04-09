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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <ctype.h>

#include <toolbox/child.h>

#include "xrandr.h"



//接続画面情報を得るためのスクリプト(shを起動して投げつけて使う)
const char* const XRandR::propScript =
	"xrandr --prop | grep -B1 -A8 EDID: | grep -v '^--' | grep -v EDID; exit\n";

//画面リスト
TB::List<XRandR::Monitor> XRandR::monitors;

XRandR::XRandR(){
	const char* const xrrProp[] = { "sh",  0 };
	TB::PipedChild c((const char* const*)xrrProp);
	write(c.WriteFd(), propScript, strlen(propScript));

	//画面情報の読み込み
	for(char line[128]; GetLine(c.ReadFd(), line);){
		if(isspace(*line)){ continue; } //行頭がを廃都スペースなのはEDID

		//読めたので画面名抽出
		char* p(line);
		for(; *p && !isspace(*p); ++p);
		*p = 0;
		Monitor* const m(new Monitor(c.ReadFd(), line));
		if(m){
			monitors.Add(*m);
		}
	}
}

bool XRandR::GetLine(int fd, char* buff){
	char* p(buff);
	for(; 0 < read(fd, p, 1) && *p != '\n'; ++p);
	if(p != buff){
		*p = 0;
		return true;
	}
	*buff = 0;
	return false;
}


XRandR::Monitor* XRandR::GetMonitor(const char* name){
	for(TB::List<Monitor>::I i(monitors); ++i;){
		if(!strcmp(name, (*i).GetName())){
			return &*i;
		}
	}
	return 0;
}

/** EDIDの画面名ブロックにあるヘッダ
 */
const unsigned char XRandR::Monitor::monitorNameHead[] = {
	0x00, 0x00, 0x00, 0xfc, 0x00 };


XRandR::Monitor::Monitor(int fd, const char* name) :
	monitorName(0),
	connectionName(name){
	for(unsigned n(0); n < 8; ++n){
		char line[128];
		if(!GetLine(fd, line) || line[0] != '\t' || line[1] != '\t'){
			//EDIDの行が不正
			syslog(
				LOG_ERR,
				"Monitor:%s EDID read failed.",
				(const char*)connectionName);
			return;
		}
		//EDID行蓄積
		char* d(edid + 16*n);
		const char* s(line + 2);
		for(unsigned n(0); n < 16; ++n, s += 2, ++d){
			unsigned v;
			sscanf(s, "%02x", &v);
			*d = v;
		}
	}

	//画面名探索
	for(unsigned n(0); n < 4; ++n){
		char* p(edid + 54 + 18*n);
		if(!memcmp(monitorNameHead, p, sizeof(monitorNameHead))){
			//画面名ブロック発見
			p += sizeof(monitorNameHead);
			monitorName = p;
			for(; *p != 0x0a; ++p){
				if(&edid[128] <= p){
					syslog(
						LOG_ERR,
						"Monitor:%s EDID broken.",
						(const char*)connectionName);
					return;
				}
			}
			*p = 0;
			syslog(
				LOG_DEBUG,
				"Monitor:%s -> %s",
				(const char*)connectionName,
				monitorName);
			return;
		}
	}
	syslog(
		LOG_ERR,
		"Monitor:%s has no name.",
		(const char*)connectionName);
}

void XRandR::Monitor::Enable(unsigned x, unsigned y){
	char position[64];
	const char* args[] = {
		"xrandr",
		"--output", (const char*)connectionName,
		"--auto",
		"--pos", position,
		0 };
	sprintf(position, "%ux%u", x, y);
	TB::Child xrr(args);
	syslog(
		LOG_INFO,
		"enable %s->%s at %u, %u.",
		(const char*)connectionName,
		monitorName, x, y);
}

void XRandR::Monitor::Disable(){
	const char* args[] = {
		"xrandr",
		"--output", (const char*)connectionName,
		"--off",
		0};
	TB::Child xrr(args);
	syslog(
		LOG_INFO,
		"disable %s->%s.",
		(const char*)connectionName,
		monitorName);
}
