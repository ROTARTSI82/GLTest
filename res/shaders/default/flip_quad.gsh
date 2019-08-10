#version 120

uniform bool doFlip;

void main() {
    for (int i = 0; i < 4; i++) {
        gl_Position = gl_PositionIn[i];
        EmitVertex();
    }
    //    if (doFlip) {
    //        for (int i = (gl_VerticesIn-1); i >= 0; i--) {
    //            gl_Position = gl_PositionIn[i];
    //            EmitVertex();
    //        }
    //    } else {
    //        for (int i = 0; i < gl_VerticesIn; i++) {
    //            gl_Position = gl_PositionIn[i];
    //            EmitVertex();
    //        }
    //    }

    EndPrimitive();
}
