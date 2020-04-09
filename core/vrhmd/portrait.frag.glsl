uniform sampler2D frameBuffer;
uniform sampler2D de_distor;
uniform vec2 chromatic;
uniform vec2 center;
uniform float expandRatio;

void main(void){
	vec4 dg = texture(de_distor, gl_TexCoord[0]) / expandRatio;
	vec4 dr = dg * chromatic[0];
	vec4 db = dg * chromatic[1];
	dr[3] = dg[3] = db[3] = 1.0;

	vec4 c = vec4(
		0.5,
		center[0] + step(gl_TexCoord[0][1], 0.5) * center[1],
		0,
		0);
	dr += c;
	dg += c;
	db += c;

	vec4 r = textureProj(frameBuffer, dr);
	vec4 g = textureProj(frameBuffer, dg);
	vec4 b = textureProj(frameBuffer, db);

	g[2] = b[2];
	g[0] = r[0];

	gl_FragColor = g;
}
