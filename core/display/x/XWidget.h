/* captured X windows
 * Copyright (C) 2018 tarosuke<webmaster@tarosuke.net>
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
#if 0
#include <X11/Xlib.h>

#include <toolbox/gl/gl.h>

#include "xDisplay.h"
#include "../window/texturedWidget.h"



class XWidget : public TexturedWidget, public XDisplay::Module{



private:

	static FACTORY<XDisplay::Module> factory;
	static XDisplay::Module* Init();

	static const XDisplay::Spec* spec;

	const int wID;
	unsigned tID;
	unsigned pID;
	GLXPixmap gID;

	XWidget(int, const XWindowAttributes&);
	~XWidget();
	void Bind() final;
	void Unbind() final;
	bool OnEvent(const XEvent&) final;

	static bool SetupXComposite(const XDisplay::Spec&);
	static void OpenReadyWindows(const XDisplay::Spec&);

	static TexturedWidget::TWParam MkParam(int id, const XWindowAttributes&);
};
#endif
