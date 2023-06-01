#include "matrix.h"

float mat2_determinate(float* mat2s) {
    return (mat2s[0] * mat2s[3]) - (mat2s[1] * mat2s[2]);
}

float* mat3_sub(float* mat2d, float* mat3s, unsigned int i) {
    float* mat2 = mat2d ? mat2d : NEW(float, 4);

    int indices[4] = {4, 5, 7, 8};

    unsigned int c = i / 3;
    for (unsigned int x=0; x<c; x++) {
        indices[x*2+0] -= 3;
        indices[x*2+1] -= 3;
    }

    unsigned int r = i % 3;
    for (unsigned int x=0; x<r; x++) {
        indices[x*1+0] -= 1;
        indices[x*1+2] -= 1;
    }

    for (unsigned int x=0; x<4; x++) {
        mat2[x] = mat3s[indices[x]];
    }

    return mat2;
}

float* mat3_transpose(float* mat3d, float* mat3s) {
    float* mat3t = NEW(float, 9);

    for (unsigned i=0; i<3; i++)
        for (unsigned j=0; j<3; j++)
            mat3t[i*3+j] = mat3s[j*3+i];

    if (mat3d) {
        memcpy(mat3d, mat3t, 9*sizeof(float));
        free(mat3t);
    } else {
        mat3d = mat3t;
    }

    return mat3d;
}

float* mat3_inverse(float* mat3d, float* mat3s) {
    float* mat3m = NEW(float, 9);
    float  mat2[4];

    for (unsigned i=0; i<9; i++) {
        mat3_sub(mat2, mat3s, i);
        mat3m[i] = mat2_determinate(mat2);
    }

    float det = mat3s[0]*mat3m[0] - mat3s[1]*mat3m[1] + mat3s[2]*mat3m[2];

    // Cofactors
    for (unsigned i=0; i<9; i++)
        if (i % 2 == 1)
            mat3m[i] = -mat3m[i];

    // Adjugate
    mat3_transpose(mat3m, mat3m);

    // Multiply by 1/Determinant
    for (unsigned i=0; i<9; i++)
        mat3m[i] /= det;

    if (mat3d) {
        memcpy(mat3d, mat3m, 9*sizeof(float));
        free(mat3m);
    } else {
        mat3d = mat3m;
    }

    return mat3d;
}

float mat3_determinate(float* mat3s) {
    float vec[3];
    float mat2[4];

    for (unsigned i=0; i<3; i++) {
        mat3_sub(mat2, mat3s, i);
        vec[i] = mat2_determinate(mat2);
    }

    float det = mat3s[0]*vec[0] - mat3s[1]*vec[1] + mat3s[2]*vec[2];

    return det;
}

float* mat4_sub(float* mat3d, float* mat4s, unsigned int i) {
    float* mat3 = mat3d ? mat3d : NEW(float, 9);

    int indices[9] = {5, 6, 7, 9, 10, 11, 13, 14, 15};

    unsigned int c = i / 4;
    for (unsigned int x=0; x<c; x++) {
        indices[x*3+0] -= 4;
        indices[x*3+1] -= 4;
        indices[x*3+2] -= 4;
    }

    unsigned int r = i % 4;
    for (unsigned int x=0; x<r; x++) {
        indices[x*1+0] -= 1;
        indices[x*1+3] -= 1;
        indices[x*1+6] -= 1;
    }

    for (unsigned int x=0; x<9; x++) {
        mat3[x] = mat4s[indices[x]];
    }

    return mat3;
}

float* mat4_transpose(float* mat4d, float* mat4s) {
    float* mat4t = NEW(float, 16);

    for (unsigned i=0; i<4; i++)
        for (unsigned j=0; j<4; j++)
            mat4t[i*4+j] = mat4s[j*4+i];

    if (mat4d) {
        memcpy(mat4d, mat4t, 16*sizeof(float));
        free(mat4t);
    } else {
        mat4d = mat4t;
    }

    return mat4d;
}


float* mat4_inverse(float* mat4d, float* mat4s) {
    float* mat4m = NEW(float, 16);
    float  mat3[9];

    for (unsigned i=0; i<16; i++) {
        mat4_sub(mat3, mat4s, i);
        mat4m[i] = mat3_determinate(mat3);
    }

    float det = mat4s[0]*mat4m[0] - mat4s[1]*mat4m[1] + mat4s[2]*mat4m[2] - mat4s[3]*mat4m[3];

    // Cofactors
    for (unsigned i=0; i<16; i++) {
        if ((i / 4) % 2 == 1)
            if (i % 2 == 0)
                mat4m[i] = -mat4m[i];

        if ((i / 4) % 2 == 0)
            if (i % 2 == 1)
                mat4m[i] = -mat4m[i];
    }

    // Adjugate
    mat4_transpose(mat4m, mat4m);

    // Multiply by 1/Determinant
    for (unsigned i=0; i<16; i++)
        mat4m[i] /= det;

    if (mat4d) {
        memcpy(mat4d, mat4m, 16*sizeof(float));
        free(mat4m);
    } else {
        mat4d = mat4m;
    }

    return mat4d;
}

float mat4_determinate(float* mat4s) {
    float vec[4];
    float mat3[9];

    for (unsigned i=0; i<4; i++) {
        mat4_sub(mat3, mat4s, i);
        vec[i] = mat3_determinate(mat3);
    }

    float det = mat4s[0]*vec[0] - mat4s[1]*vec[1] + mat4s[2]*vec[2] - mat4s[3]*vec[3];

    return det;
}

float* mat4_identity(float* mat4d) {
    float* mat4i = mat4d ? mat4d : NEW(float, 16);

    for (unsigned i=0; i<16; i++)
        mat4i[i] = (i % 5 == 0) ? 1.0 : 0.0;

    return mat4i;
}

float* mat4_translate(float* mat4d, float* mat4s, float* vec3) {
    float* mat4t = mat4_identity(NULL);

    for (unsigned i=0; i<3; i++)
        mat4t[12+i] = vec3[i];

    if (mat4s)
        mat4_multiply(mat4t, mat4s, mat4t);

    if (mat4d) {
        memcpy(mat4d, mat4t, 16*sizeof(float));
        free(mat4t);
    } else {
        mat4d = mat4t;
    }

    return mat4d;
}

float* mat4_rotate(float* mat4d, float* mat4s, float radians, float* vec3) {
    float* mat4r = mat4_identity(NULL);

    float u = vec3[0];
    float v = vec3[1];
    float w = vec3[2];

    float vec3l = sqrt( pow(u,2) + pow(v,2) + pow(w,2) );
    if (vec3l != 1.0) {
        u /= vec3l;
        v /= vec3l;
        w /= vec3l;
    }

    mat4r[0] = pow(u, 2) + (pow(v, 2)+pow(w, 2))*cos(radians);
    mat4r[1] = (u * v * (1-cos(radians))) + (w*sin(radians));
    mat4r[2] = (u * w * (1-cos(radians))) - (v*sin(radians));
    mat4r[3] = 0;

    mat4r[4] = (u * v * (1-cos(radians))) - (w*sin(radians));
    mat4r[5] = pow(v, 2) + (pow(u, 2)+pow(w, 2))*cos(radians);
    mat4r[6] = (v * w * (1-cos(radians))) + (u*sin(radians));
    mat4r[7] = 0;

    mat4r[8] = (u * w * (1-cos(radians))) + (v*sin(radians));
    mat4r[9] = (v * w * (1-cos(radians))) - (u*sin(radians));
    mat4r[10] = pow(w, 2) + (pow(u, 2)+pow(v, 2))*cos(radians);
    mat4r[11] = 0;

    mat4r[12] = 0;
    mat4r[13] = 0;
    mat4r[14] = 0;
    mat4r[15] = 1;

    if (mat4s)
        mat4_multiply(mat4r, mat4s, mat4r);

    if (mat4d) {
        memcpy(mat4d, mat4r, 16*sizeof(float));
        free(mat4r);
    } else {
        mat4d = mat4r;
    }

    return mat4d;
}

float* mat4_multiply(float* mat4d, float* mat4a, float* mat4b) {
    float* mat4m = NEW(float, 16);

    for (unsigned i=0; i<4; i++)
        for (unsigned j=0; j<4; j++)
            mat4m[i*4+j] = mat4a[0+j]*mat4b[i*4+0] + mat4a[4+j]*mat4b[i*4+1] + mat4a[8+j]*mat4b[i*4+2]  + mat4a[12+j]*mat4b[i*4+3];

    if (mat4d) {
        memcpy(mat4d, mat4m, 16*sizeof(float));
        free(mat4m);
    } else {
        mat4d = mat4m;
    }

    return mat4d;
}

float* mat4_perspective(float* mat4d, double fov, double aspect, double near, double far) {
    float yc = 1 / tan(fov/2.0 * M_PI/180.0);
    float xc = yc / aspect;
    float zc = (near + far) / (near - far);
    float za = (2*far*near) / (near - far);

    float* mat4p = mat4_identity(mat4d);

    mat4p[0]  = xc;
    mat4p[5]  = yc;
    mat4p[10] = zc;
    mat4p[11] = -1;
    mat4p[14] = za;

    return mat4p;
}

float* mat4_orthographic(float* mat4d, double left, double right, double top, double bottom) {
    float xc = 2.0 / (right - left);
    float xt = -(2.0 * left + right - left) / (right - left);
    float yc = -2.0 / (bottom - top);
    float yt =  (2.0 * top + bottom - top) / (bottom - top);
    float zc = 1;
    float zt = 0;

    float* mat4p = mat4_identity(mat4d);

    mat4p[0]  = xc;
    mat4p[5]  = yc;
    mat4p[10] = zc;

    mat4p[12] = xt;
    mat4p[13] = yt;
    mat4p[14] = zt;

    return mat4p;
}

float* vec3_add(float* vec3d, float* vec3a, float* vec3b) {
    vec3d[0] = vec3a[0] + vec3b[0];
    vec3d[1] = vec3a[1] + vec3b[1];
    vec3d[2] = vec3a[2] + vec3b[2];

    return vec3d;
}

float* vec3_scale(float* vec3d, float* vec3s, float m) {
    memcpy(vec3d, vec3s, 3*sizeof(float));

    vec3d[0] = vec3d[0] * m;
    vec3d[1] = vec3d[1] * m;
    vec3d[2] = vec3d[2] * m;

    return vec3d;
}

float* vec3_transform(float* vec3d, float* mat4, float* vec3s) {
    float* vec3t  = NEW(float, 3);

    for (unsigned i=0; i<3; i++)
        vec3t[i] = vec3s[0]*mat4[i+0] + vec3s[1]*mat4[i+4] + vec3s[2]*mat4[i+8] + mat4[i+12];

    if (vec3d) {
        memcpy(vec3d, vec3t, 3*sizeof(float));
        free(vec3t);
    } else {
        vec3d = vec3t;
    }

    return vec3d;
}

float* vec3_normalize(float* vec3d, float* vec3s) {
    float* vec3n  = NEW(float, 3);
    memcpy(vec3n, vec3s, 3*sizeof(float));

    float length = sqrt(pow(vec3s[0], 2) + pow(vec3s[1], 2) + pow(vec3s[2], 2));

    for (int i=0; i<3; i++)
        vec3n[i] /= length;

    if (vec3d) {
        memcpy(vec3d, vec3n, 3*sizeof(float));
        free(vec3n);
    } else {
        vec3d = vec3n;
    }

    return vec3n;
}

float* vec4_transform(float* vec4d, float* mat4, float* vec4s) {
    float* vec4t  = NEW(float, 4);

    for (unsigned i=0; i<4; i++)
        vec4t[i] = vec4s[0]*mat4[i+0] + vec4s[1]*mat4[i+4] + vec4s[2]*mat4[i+8] + vec4s[3]*mat4[i+12];

    if (vec4d) {
        memcpy(vec4d, vec4t, 4*sizeof(float));
        free(vec4t);
    } else {
        vec4d = vec4t;
    }

    return vec4d;
}
