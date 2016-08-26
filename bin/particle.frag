#version 330
#define PERIOD	1000
#define M_PI	3.14159

out vec4 outputColor;

uniform int timePassed;
uniform vec2 windowSize;

void main(){
	float distanceFromCenter = length(gl_FragCoord.xy - windowSize/2);

	outputColor = vec4(.04+.04*sin(2*M_PI*(distanceFromCenter/1000.0-timePassed/1000.0)), 0.02f, 0.06f, 0.4f);
}