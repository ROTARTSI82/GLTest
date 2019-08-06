uniform vec4 u_PMult;
uniform vec4 u_POffset;

void main()
{
	gl_Position = ftransform() * u_PMult + u_POffset;
}