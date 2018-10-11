namespace FSGL {

static const GLchar* vertexShaderSource =
        "#version 100\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform mat4 viewMatrix;\n"
        "uniform mat4 modelMatrix;\n"

        "attribute vec4 vertex;\n"
        "attribute vec2 uvIn;\n"

        "varying vec2 uvOut;\n"
        "void main() {\n"
        "   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertex;\n"
        "   uvOut = uvIn;\n"
        "}\n";

static const GLchar* fragmentShaderSource =
        "#version 100\n"
	  "precision mediump int;\n"
	  "precision mediump float;\n"
	  "precision mediump sampler2D;\n"
	  "precision mediump samplerCube;\n"
        "varying mediump vec2 uvOut;\n"
        "uniform sampler2D texture;\n"
	  "uniform float brightness;\n"
        "void main() {\n"
	 "vec4 color = texture2D(texture, uvOut);\n"
	 "color.r = brightness * color.r;\n"
	 "color.g = brightness * color.g;\n"
	 "color.b = brightness * color.b;\n"
	 "if (color.a < 0.5) {\n"
	 "	discard;\n"
	 "}\n"
        "   gl_FragColor = color;"
        "}\n";

}