/** EVDEVのラッパー
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

#include "evdev.h"

#include <ctype.h>
#include <syslog.h>



namespace vr_core{

	const char* Evdev::uidPatterns[] = { "event-kbd", "event-mouse", 0 };
	const Evdev::ModifiersBit Evdev::modifiersBits[] = {
			{ KEY_LEFTSHIFT, Widget::KeyEvent::shiftL },
			{ KEY_RIGHTSHIFT, Widget::KeyEvent::shiftR },
			{ KEY_LEFTCTRL, Widget::KeyEvent::ctrlL },
			{ KEY_RIGHTCTRL, Widget::KeyEvent::ctrlR },
			{ KEY_LEFTALT, Widget::KeyEvent::altL },
			{ KEY_RIGHTALT, Widget::KeyEvent::altR },
			{ 0 }
	};
	const char* Evdev::keyMap(0);
	const Evdev::CharMap* Evdev::charMap(&defaultCharMap); //0:normal, 1:shift 2:ctrl


	Widget::KeyEvent Evdev::GetKeyEvent(){
		Widget::KeyEvent ev = { type : Widget::KeyEvent::none };

		//キーコード取得
		const int kk(ReadKey());
		if(!kk){ return ev; }; //文字なし

		//キーコードだけ取り出す
		const unsigned char kCode((kk | (kk >> 8)) & 0xff);
		const bool space(!!(kk & 0xff00));

		//設定してあればkeyMap適用
		unsigned char k(keyMap ? keyMap[kCode] : kCode);

		//イベントタイプ設定
		ev.type = space ? Widget::KeyEvent::up : Widget::KeyEvent::down;

		//モディファイア設定
		ev.charCode = 0;
		ev.keyCode = k;
		for(const ModifiersBit* mb(modifiersBits); (*mb).keyCode; ++mb){
			if(k == (*mb).keyCode){
				//モディファイアキー一致
				if(space){
					//スペース
					modifiers &= ~(*mb).bit;
				}else{
					//マーク
					modifiers |= (*mb).bit;
				}
				ev.modifiers = modifiers;
				return ev;
			}
		}
		//CAPS LOCKの処理
		if(k == KEY_CAPSLOCK && !space){
			modifiers ^= Widget::KeyEvent::caps;
			return ev;
		}

		//通常文字
		ev.charCode = (*charMap).map
			[ev.keyCode]
			[!!(modifiers & Widget::KeyEvent::ctrl)]
			[!!(modifiers & Widget::KeyEvent::shift)];
		ev.modifiers = modifiers;
		if((modifiers & Widget::KeyEvent::caps) && isalpha(ev.charCode)){
			//CAPS LOCKが入っているのでUPPER/LOWERを交換
			ev.charCode ^= 0x20;
		}

		return ev;
	}


	/** 初期charMap
	 */
	const Evdev::CharMap Evdev::defaultCharMap = {{
		// { シフトなし ⇔ シフトあり }, ctrlなし ⇔ ctrlあり {  } },
		//00-0F
		{ {  }, {  } },
		{ { 0x18, 0x18 }, { 0x18 } }, //ESC
		{ { '1', '!' }, { 0 } },
		{ { '2', '@' }, { 0x03 } },
		{ { '3', '#' }, { 0 } },
		{ { '4', '$' }, { 0, 0x7b} },
		{ { '5', '%' }, { 0 } },
		{ { '6', '^' }, { 0x1e } },
		{ { '7', '&' }, { 0 } },
		{ { '8', '*' }, { 0 } },
		{ { '9', '(' }, { 0 } },
		{ { '0', ')' }, { 0 } },
		{ { '-', '_'  }, { 0x1f } },
		{ { '=', '+' }, { 0 } },
		{ { 0x08, 0x08 }, { 0x7f } }, //BS
		{ { 0x09, 0x0f }, { 0x94 } }, //TAB
		//10-1F
		{ { 'q', 'Q' }, { 0x11 } },
		{ { 'w', 'W' }, { 0x17 } },
		{ { 'e', 'E' }, { 0x05 } },
		{ { 'r', 'R' }, { 0x12 } },
		{ { 't', 'T' }, { 0x14 } },
		{ { 'y', 'Y' }, { 0x19 } },
		{ { 'u', 'U' }, { 0x15 } },
		{ { 'i', 'I' }, { 0x09 } },
		{ { 'o', 'O' }, { 0x1f } },
		{ { 'p', 'P' }, { 0x10 } },
		{ { '[', '{' }, { 0x1b } },
		{ { ']', '}' }, { 0x1d } },
		{ { 0x0d, 0x0d }, { 0x0a } }, //enter
		{ {  }, {  } }, //LEFT_CTRL
		{ { 'a', 'A' }, { 0x01 } },
		{ { 's', 'S' }, { 0x13 } },
		//20
		{ { 'd', 'D' }, { 0x04 } },
		{ { 'f', 'F' }, { 0x06 } },
		{ { 'g', 'G' }, { 0x07 } },
		{ { 'h', 'H'  }, { 0x08 } },
		{ { 'j', 'J' }, { 0x0a } },
		{ { 'k', 'K' }, { 0x0b } },
		{ { 'l', 'L' }, { 0x0c } },
		{ { ';', ':'  }, {  } },
		{ { '\'', '"' }, {  } },
		{ { 0x60, '~'  }, {  } },
		{ {  }, {  } }, //L_SHIFT
		{ { '\\', '|' }, { 0x1c } },
		{ { 'z', 'Z' }, { 0x1a } },
		{ { 'x', 'X' }, { 0x18 } },
		{ { 'c', 'C' }, { 0x03 } },
		{ { 'v', 'V' }, { 0x16 } },
		//30
		{ { 'b', 'B' }, { 0x02 } },
		{ { 'n', 'N' }, { 0x0e } },
		{ { 'm', 'M' }, { 0x0d } },
		{ { ',', '<' }, { } },
		{ { '.', '>' }, { } },
		{ { '/', '?' }, { } },
		{ {  }, {  } }, //R_SHIFT
		{ { 0x2a }, {  } }, //print screen?
		{ {  }, {  } }, //L_ALT
		{ {  }, {  } }, //R_ALT
		{ { ' ', ' ' }, { ' ', ' ' } },
		{ {  }, {  } }, //CAPS
		{ {  }, {  } }, //F1
		{ {  }, {  } }, //F2
		{ {  }, {  } }, //F3
		{ {  }, {  } }, //F4
		{ {  }, {  } }, //F5
		//40-4F
		{ {  }, {  } }, //F6
		{ {  }, {  } }, //F7
		{ {  }, {  } }, //F8
		{ {  }, {  } }, //F9
		{ {  }, {  } }, //F10
		{ {  }, {  } }, //NUM
		{ {  }, {  } }, //SCROLL LOCK
		{ { '7', '7' }, {  } }, //KP7
		{ { '8', '8' }, {  } }, //KP8
		{ { '9', '9' }, {  } }, //KP9
		{ { '-', '-' }, {  } }, //KP-
		{ { '4', '4' }, {  } }, //KP4
		{ { '5', '5' }, {  } }, //KP5
		{ { '6', '6' }, {  } }, //KP6
		{ { '+', '+' }, {  } }, //KP+
		{ { '3', '3' }, {  } }, //KP3
		//50-5F
		{ { '2', '2' }, {  } }, //KP2
		{ { '1', '1' }, {  } }, //KP1
		{ { '0', '0' }, {  } }, //KP0
		{ { '.', '.' }, {  } }, //KP.
		{ {  }, {  } }, //全角/半角
		{ {  }, {  } },
		{ {  }, {  } }, //F11
		{ {  }, {  } }, //F12
		{ {  }, {  } }, //SYSRO
		{ {  }, {  } }, //カタカナ
		{ {  }, {  } }, //ひらがな
		{ {  }, {  } }, //変換
		{ {  }, {  } }, //カタカナ/ひらがな
		{ {  }, {  } }, //無変換
		{ { ',', ',' }, {  } }, //KP,
		{ {  }, {  } }, //KP HOME
		//60-6f
		{ {  }, {  } }, //KP ENTER
		{ {  }, {  } }, //RIGHT CTRL
		{ { '/', '/' }, {  } }, //KP/
		//以下、対応する文字がないので略
	}};

}
