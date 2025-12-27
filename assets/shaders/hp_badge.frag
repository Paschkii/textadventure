#version 120

uniform sampler2D texture;
uniform float diagonal;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec4 texColor = texture2D(texture, uv);

    if (texColor.a == 0.0) {
        discard;
    }

    vec4 colorA = vec4(1.0, 0.45, 0.45, 1.0);
    vec4 colorB = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 colorC = vec4(0.12, 0.12, 0.12, 1.0);
    float blend = clamp((uv.x + uv.y * diagonal) * 1.45 - 0.2, 0.0, 1.0);
    vec4 midpoint = mix(colorA, colorB, clamp(blend * 1.15, 0.0, 1.0));
    float fade = smoothstep(0.45, 0.85, blend);
    vec4 finalColor = mix(midpoint, colorC, fade);

    gl_FragColor = vec4(finalColor.rgb, texColor.a);
}
