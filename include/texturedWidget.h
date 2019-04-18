/** TexturedWidget
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
#pragma once

#include <toolbox/geometry/vector.h>
#include <toolbox/gl/texture.h>

#include "borderWidget.h"


namespace vr_core{

	/** 中身付きWidget
	 */
	class TexturedWidget : public BorderWidget, public TB::Texture{
		TexturedWidget();
	public:
		void Update(
			const void*,
			int x,
			int y,
			unsigned width,
			unsigned height,
			Format = BGRA);

	protected:

		TexturedWidget(
			Widget* parent,
			int x,
			int y,
			unsigned width,
			unsigned height,
			unsigned attributes = 0);

		void DrawContent() override;

		~TexturedWidget();

	private:
		static unsigned Attribute(Format);

		unsigned tid;
	};

}
