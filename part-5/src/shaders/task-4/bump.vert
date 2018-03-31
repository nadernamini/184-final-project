attribute vec3 position;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec2 uv;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelMatrix;
uniform sampler2D textureDisplacement;
uniform vec2 textureDimension;

varying vec3 fPosition;
varying vec3 fNormal;

void main() {
    vec3 offset = position;

    // TODO: Compute displaced vertices
    float heightScaling = 0.8;

    // TODO: Compute displaced normals
    float normalScaling = 1.0;

    fPosition = vec3(0.0);
    fNormal = vec3(0.0);
    gl_Position = projectionMatrix * modelViewMatrix * vec4(offset, 1.0);
}