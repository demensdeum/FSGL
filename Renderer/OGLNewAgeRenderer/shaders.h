namespace FSGL {

static const GLchar* vertexShaderSource =
        "#version 100\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform mat4 viewMatrix;\n"
        "uniform mat4 modelMatrix;\n"

        "attribute vec4 vertex;\n"
        "attribute vec2 uvIn;\n"

        "attribute vec4 animationTransformRowOne;\n"
        "attribute vec4 animationTransformRowTwo;\n"
        "attribute vec4 animationTransformRowThree;\n"
        "attribute vec4 animationTransformRowFour;\n"

        "varying vec2 uvOut;\n"
        "void main() {\n"
        "   mat4 animationTransform = mat4(animationTransformRowOne, animationTransformRowTwo,animationTransformRowThree, animationTransformRowFour);\n"
        "   vec4 vertexPosition = vertex * animationTransform;\n"
        "   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPosition;\n"
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
	 "if (color.r == 0.0 && color.g == 0.0 && color.b == 1.0) {\n"
	 "	discard;"
	 "}\n"
	 "color.r = brightness * color.r;\n"
	 "color.g = brightness * color.g;\n"
	 "color.b = brightness * color.b;\n"
        "   gl_FragColor = color;"
        "}\n";

}