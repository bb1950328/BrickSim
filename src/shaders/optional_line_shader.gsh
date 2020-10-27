#version 330 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 bColor;

void main() {
    //todo look up how to convert these to screen coordinates (3D->2D)
    vec4 c1 = gl_in[0].gl_Position;//control point 1
    vec4 a = gl_in[1].gl_Position;//line point 1
    vec4 b = gl_in[2].gl_Position;//line point 2
    vec4 c2 = gl_in[3].gl_Position;//control point 2

    float determinant1 = (b.x - a.x) * (c1.y - a.y) - (b.y - a.y) * (c1.x - a.x);
    float determinant2 = (b.x - a.x) * (c2.y - a.y) - (b.y - a.y) * (c2.x - a.x);
    if (determinant1*determinant2>0) {//todo check if (determinant1>0&&determinant2>0)||(determinant1<0&&determinant2<0) is faster
        bColor = gs_in[0].color;
    } else {
        bColor = vec3(1.0, 0.0, 0.0);//todo remove this after debugging
    }
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}