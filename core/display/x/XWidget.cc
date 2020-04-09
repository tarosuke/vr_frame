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
#if 0
#include <syslog.h>
#include <X11/extensions/Xcomposite.h>

#include <toolbox/factory/factory.h>

#include "XWidget.h"
#include "xDisplay.h"



namespace vr_core{



	typedef void (*t_glx_bind)(Display *, GLXDrawable, int , const int *);
	static t_glx_bind glXBindTexImageEXT = (t_glx_bind) glXGetProcAddress((const GLubyte *)"glXBindTexImageEXT");


	FACTORY<XDisplay::Module> XWidget::factory(Init);

	const XDisplay::Spec* XWidget::spec(0);


	/** XWidget全体の初期化
	 */
	XDisplay::Module* XWidget::Init(){
		// TODO：窓の奥行き問題が解決したら140行目をコメントアウトして続き
		return 0;

		assert(glXBindTexImageEXT);

		spec = &XDisplay::GetSpec();
		assert(spec);

		//XComposite設定
		if(!SetupXComposite(*spec)){
			return 0;
		}

		//金損窓分を同期
		OpenReadyWindows(*spec);

		return 0;
	}


	/** 既存窓分のXWidgetを生成
	 */
	void XWidget::OpenReadyWindows(const XDisplay::Spec& spec){
		//既存窓取得
		Window r;
		Window p;
		unsigned num;
		Window* wl;
		XQueryTree(spec.display, spec.root, &r, &p, &wl, &num);
		if(!!wl){
			Window* w(wl);
			for(unsigned n(0); n < num; n++, w++){
				if(*w != spec.wODMroot){
					XWindowAttributes attr;
					XGetWindowAttributes(spec.display, *w, &attr);
					if(attr.c_class == InputOutput &&
						attr.map_state == IsViewable){
						new XWidget(*w, attr);
					}
				}
			}
			XFree(wl);
		}
	}

	/** XCompositeの設定
	 */
	bool XWidget::SetupXComposite(const XDisplay::Spec& spec){
		int base;
		int error;
		if(!XCompositeQueryExtension(spec.display, &base, &error)){
			syslog(LOG_ERR, "XComposite is not avail");
			return false;
		}

		return true;
	}



	TexturedWidget::TWParam XWidget::MkParam(
		int wID,
		const XWindowAttributes& attr){
		assert(spec);

		TWParam param;

		param.x = attr.x;
		param.y = attr.y;
		param.width = attr.width;
		param.height = attr.height;

		return param;
	}



	/** 個別のXWidget
	 */
	XWidget::XWidget(int id, const XWindowAttributes& attr) :
		TexturedWidget(MkParam(id, attr)),
		wID(id){
	#if 1
		XCompositeRedirectWindow(
			(*spec).display, wID, CompositeRedirectManual);
	#endif

		//テクスチャの確保
		glGenTextures(1, &tID);
		assert(tID);
		Bind();
		//テクスチャのパラメタを設定
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//窓のpixmapをテクスチャに結びつけておく
		pID = XCompositeNameWindowPixmap((*spec).display, wID);
		assert(pID);

		// glxpixmapへ
	//	gID = glXCreatePixmap ((*spec).display, fbconfigs[i], pixmap, pixmapAttribs);
	}

	XWidget::~XWidget(){}

	void XWidget::Bind(){
		glBindTexture(GL_TEXTURE_2D, tID);
		glXBindTexImageEXT((*spec).display, gID, GLX_EXT_texture_from_pixmap, NULL);
	}

	void XWidget::Unbind(){
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	bool XWidget::OnEvent(const XEvent& e){
		return false;
	}

}
#endif

