#version 440 core                                                                           
                                                                                            
out vec4 FragColor;                                                                         
in vec4 fcolor;
                                                                                            
void main()                                                                                 
{ 
	FragColor = fcolor;
	//FragColor = vec4(0.0,drawCommands.vertexCount*10.0f/255.0f,0.0,1.0);
	//FragColor = vec4(0.0,100.0/255.0f,0.0,1.0);
}