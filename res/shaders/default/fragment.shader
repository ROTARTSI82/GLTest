uniform vec4 u_Tint;
uniform vec4 u_Mult;


void main()
{
	gl_FragColor = gl_Color * u_Mult + u_Tint;

}
