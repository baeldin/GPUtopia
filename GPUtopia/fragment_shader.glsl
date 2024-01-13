#version 330 core

uniform vec2 resolution = vec2(1000., 1000.);
uniform int max_iterations1 = 12;
uniform int max_iterations2 = 12;
float bailout = 1000000.;
uniform vec2 juliaSeed = vec2(0., 0.);
out vec4 fragColor;
int maxPasses = 128;

vec2 complexSquare(vec2 z) {
    return vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y);
}

vec2 complexMult(vec2 z1, vec2 z2) {
    return vec2(z1.x * z2.x - z1.y * z2.y, z1.x * z2.y + z1.y * z2.x);
}

vec2 complexDiv(vec2 z1, vec2 z2) {
    float div = z2.x * z2.x + z2.y * z2.y;
    return vec2(z1.x * z2.x + z1.y * z2.y, z1.y * z2.x - z1.x * z2.y) / div;
}

vec2 powCF(vec2 z, float f) {
    float angle = atan(z.y, z.x);
    float l = length(z);
    return vec2(cos(angle * f), sin(angle * f)) * pow(l, f);
}

vec2 expC(vec2 z) {
    return exp(z.x) * vec2(cos(z.y), sin(z.y));
}

vec2 sinC(vec2 z) {
    return vec2(sin(z.x)*cosh(z.y), cos(z.x) * sinh(z.y));
}

vec2 cosC(vec2 z) {
    return vec2(cos(z.x)*cosh(z.y), -sin(z.x) * sinh(z.y));
}

vec2 compAbs(vec2 z) { 
    return vec2(abs(z.x), abs(z.y)); 
}

uint lowbias32(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float halton(int i, int b) {
    float r = 0.;
    float f = 1;
    while (i > 0)
    {
        f = f / float(b);
        r = r + f * float(i % b);
        i = i/b;
    }
    return r;
}

float tent(float x) {
    if (x == 0.5) { return 0.; }
    float orig = x * 2 - 1;
    x = orig / sqrt(abs(orig));
    return x;
}

float wrap1(float u, float v) { 
    return (u + v < 1) ? u + v : u + v - 1;
}

const uint uint_max = 0xffffffffu;

float tofloat(uint x) {
    return float(x) / float(uint_max);
}

int mandelbrot(vec2 c) {
    vec2 z = c;
    int iterations = 0;

    for (int i = 0; i < max_iterations1; ++i) {
        z = powCF(compAbs(powCF(z, 3)), 0.5) + c;
        if (dot(z, z) > bailout) {
            break;
        }
        ++iterations;
    }

    return iterations;
}

float julia(vec2 z0, vec2 c) {
    vec2 z = z0;
    int iterations = 0;
    float sumConv = 0;
    float sumDiv = 0;
    for (int i = 0; i < max_iterations2; ++i) {
        vec2 z_old = z;
        z = powCF(compAbs(powCF(z, 4)), 0.3333333) + c;
        sumConv += exp(-1./length(z - z_old));
        sumDiv += exp(-length(z - z_old));
        if (dot(z, z) > bailout) {
            return sumDiv;
        }
        ++iterations;
    }

    return sumConv;
}

float julia2(vec2 z0, vec2 c) {
    vec2 z = z0;
    int iterations = 0;
    float sumConv = 0;
    float sumDiv = 0;
    int k = 0;
    int kmax = 250;
    int kstart = 500;
    int kend = 999999999;
    for (int i = 0; i < max_iterations2; ++i) {
        vec2 z_old = z;
        z = z + 0.1 * vec2(sin(10. * z.x), cos(10. * z.y));
        //z = powCF(compAbs(powCF(z, 4)), 0.3333333) + c;
        sumConv += exp(-1./length(z - z_old));
        sumDiv += exp(-length(z - z_old));
        if (dot(z, z) > bailout) {
            return sumDiv;
        }
        ++iterations;
    }

    return sumConv;
}
float clampZeroToOne(float x) {
    return (x < 0.) ? 0. : (x > 1.) ? 1. : x;
}

const float invGamma = 1. / 2.4;
float linear2srgb(float x) {
	return (clampZeroToOne(x) <= 0.0031308) ? clampZeroToOne(x) * 12.92 : 1.055 * pow(clampZeroToOne(x), invGamma) - 0.055;
}

vec4 linear2srgb(vec4 col) {
    return vec4(
        linear2srgb(col.x),
        linear2srgb(col.y),
        linear2srgb(col.z),
        col.w);
}

void main() {
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    int pixelIdx = int(gl_FragCoord.y * resolution.y + gl_FragCoord.x);
    for (int ii = 0; ii < maxPasses; ii++)
    {
        float xOffset = tent(wrap1(float(ii) / float(maxPasses), tofloat(lowbias32(uint(2 * pixelIdx    )))));
        float yOffset = tent(wrap1(halton(ii, 2),                tofloat(lowbias32(uint(2 * pixelIdx + 1)))));
        vec2 uv = (gl_FragCoord.xy + vec2(xOffset, yOffset)) / resolution;
        vec2 c = (uv - 0.5) * 2.0;
        vec2 seedUV = juliaSeed / resolution;
        vec2 seed = (seedUV - 0.5) * 2.0;
        //int iterations2 = mandelbrot(c);
        float index = julia(c, seed);
        fragColor.x += 0.5 + 0.5 * sin(0.4*index);
        fragColor.y += 0.5 + 0.5 * sin(0.4*index);
        fragColor.z += 0.5 + 0.5 * sin(0.4*index);
    }
    fragColor = linear2srgb(fragColor / float(maxPasses));
}