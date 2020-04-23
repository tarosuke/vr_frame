/** CanvasWidget
 * Copyright (C) 2015,2016,2017,2018,2019 tarosuke<webmaster@tarosuke.net>
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
#include <canvasWidget.h>


#include <math.h>

#include <toolbox/image.h>



namespace core{

	/** 描画可能なWidget
	 */
	CanvasWidget::CanvasWidget(
		Widget* parent,
		int x,
		int y,
		unsigned width,
		unsigned height,
		Format format) :
		TexturedWidget(parent, x, y, width, height, format),
		Canvas(width, height){};

}
