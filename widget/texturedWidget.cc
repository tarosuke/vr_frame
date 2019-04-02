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

#include "texturedWidget.h"

#include <math.h>

#include <toolbox/gl/gl.h>



namespace vr_core{

	/** 中身付きWidget
	 */
	TexturedWidget::TexturedWidget(
		Widget* parent,
		int x,
		int y,
		unsigned width,
		unsigned height,
		Format format,
		unsigned attributes) :
		BorderWidget(
			parent,
			x,
			y,
			width,
			height,
			attributes | Attribute(format)),
		TB::Texture(width, height, format){
		//TODO:テクスチャパラメタの設定
	}

	TexturedWidget::~TexturedWidget(){
		glDeleteTextures(1, &tid);
	}

	void TexturedWidget::Update(
		const void* buffer,
		int x,
		int y,
		unsigned width,
		unsigned height,
		Format format){

	}

	void TexturedWidget::DrawContent(){
		const Widget::Clip& vertexRect(GetClip());
		if(!vertexRect){
			//clipped out
			return;
		}

		const auto& size(Size());
		const Widget::Clip textureRect(
			vertexRect.left / size.X(),
			vertexRect.top / size.Y(),
			vertexRect.right / size.X(),
			vertexRect.bottom / size.Y());

		//内容描画
		Binder b(*this);

		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(textureRect.left, textureRect.top);
		glVertex2i(vertexRect.left, vertexRect.top);
		glTexCoord2f(textureRect.left, textureRect.bottom);
		glVertex2i(vertexRect.left, vertexRect.bottom);
		glTexCoord2f(textureRect.right, textureRect.top);
		glVertex2i(vertexRect.right, vertexRect.top);
		glTexCoord2f(textureRect.right, textureRect.bottom);
		glVertex2i(vertexRect.right, vertexRect.bottom);
		glEnd();
	}

	unsigned TexturedWidget::Attribute(Format format){
		return Texture::IsTransparent(format) ? hasContent | transparent : hasContent;
	}

}
