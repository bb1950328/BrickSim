#version 330 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 bColor;

void main() {
    vec2 c1 = (gl_in[0].gl_Position.xyz/gl_in[0].gl_Position.w).xy*0.5+0.5;//control point 1
    vec2 a = (gl_in[1].gl_Position.xyz/gl_in[1].gl_Position.w).xy*0.5+0.5;//line point 1
    vec2 b = (gl_in[2].gl_Position.xyz/gl_in[2].gl_Position.w).xy*0.5+0.5;//line point 2
    vec2 c2 = (gl_in[3].gl_Position.xyz/gl_in[3].gl_Position.w).xy*0.5+0.5;//control point 2

    float determinant1 = (b.x - a.x) * (c1.y - a.y) - (b.y - a.y) * (c1.x - a.x);
    float determinant2 = (b.x - a.x) * (c2.y - a.y) - (b.y - a.y) * (c2.x - a.x);
    if (determinant1*determinant2>0) { //todo check if (determinant1>0&&determinant2>0)||(determinant1<0&&determinant2<0) is faster
        bColor = gs_in[0].color;
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();
        EndPrimitive();
    }
}