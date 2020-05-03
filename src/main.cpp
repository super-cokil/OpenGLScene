#include <iostream>
#include <stack>
#include <cstdarg>

//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_image.h>

//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

//GML libraries
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "animation/AnimMoveAction.h"
#include "animation/AnimRotateAction.h"
#include "animation/Animation.h"

#include "buffer/complexvertexbuffer.h"
#include "buffer/indexbuffer.h"
#include "buffer/vertexbufferlayout.h"

#include "geometry/Cube.h"
#include "geometry/Square.h"
#include "geometry/ObjMesh.h"


#include "rendering/Camera.h"
#include "rendering/renderer.h"
#include "rendering/RenderedObject.h"

#include "light.h"
#include "material.h"
#include "Shader.h"
#include "texture.h"
#include "vertexarray.h"



Shader* setupShaders(const char* vertexPath, const char* fragmentPath, unsigned int count, ...) {
	va_list args;
	va_start(args, count);
	FILE* fragmentShader = fopen(fragmentPath, "r");
	FILE* vertexShader = fopen(vertexPath, "r");
	if(!fragmentShader || !vertexShader) {
		std::cerr << "[File Error] can't open shader" << std::endl;
		//exit(1);
	}

	auto shader = Shader::loadFromFiles(vertexShader, fragmentShader, count, args);
	va_end(args);
	fclose(fragmentShader);
	fclose(vertexShader);
// 	shader = Shader::loadFromStrings(
// 		"#version 330 core\nlayout(location = 0) in vec4 position;\nvoid main(){gl_Position = position;}",
// 		"#version 330 core\nlayout(location = 0) out vec4 color;\nvoid main(){color = vec4(1.0, 0.0, 0.0, 1.0);}");

	if (shader == 0 || shader == nullptr) {
		std::cerr << "[Shader Error] NULL" << std::endl;
		//exit(EXIT_FAILURE);
	}
	return shader;
}

int main(int argc, char* argv[]) {
	////////////////////////////////////////
	//SDL2 / OpenGL Context initialization :
	////////////////////////////////////////

	//Initialize SDL2
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
		ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
		return 0;
	}

	//init SDL_image
	if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		ERROR("Could not load SDL_image.\n");
		return EXIT_FAILURE;
	}

	//Create a Window
	SDL_Window* window = SDL_CreateWindow("OpenGL Scene",                           //Titre
	                                      SDL_WINDOWPOS_UNDEFINED,               //X Position
	                                      SDL_WINDOWPOS_UNDEFINED,               //Y Position
	                                      WIDTH, HEIGHT,                         //Resolution
	                                      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); //Flags (OpenGL + Show)

	//Initialize OpenGL Version (default version 3.0)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	//Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
	SDL_GLContext context = SDL_GL_CreateContext(window);

	//Tells GLEW to initialize the OpenGL function with this version
	glewExperimental = GL_TRUE;
	glewInit();
	SDL_GL_SetSwapInterval(1);



	std::cout << glGetString(GL_VERSION) << std::endl;
	{
		//Start using OpenGL to draw something on screen
		GLCall(glViewport(0, 0, WIDTH, HEIGHT)); //Draw on ALL the screen

		//The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
		GLCall(glClearColor(0.0, 0.0, 0.0, 1.0)); //Full Black

		GLCall(glEnable(GL_DEPTH_TEST)); //Active the depth test

		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCall(glEnable(GL_BLEND));

		Renderer renderer;
		RenderedObject root;
		Camera camera;

		glm::vec4 matColor(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 propert(0.5f, 0.5f, 0.5f, 50.0f);
		Material defaultMat(matColor, propert);

		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
		glm::vec4 lightPos(0.0f, 1.0f, 0.0f, 1.0f);
		Light sun = Light(lightPos, lightColor);

		auto defaultShader = setupShaders("resources/Shaders/Tex.vert", "resources/Shaders/Tex.frag", 2, "v_Position", "v_UV");

		auto lightShader = setupShaders("resources/Shaders/lightTex.vert", "resources/Shaders/lightTex.frag", 3, "v_Position", "v_UV", "v_Normal");

		Texture texture("resources/img/NonoreveLogo.png");
		Texture cubeTexture("resources/img/grass.png");

		
		// TODO include in geometry
// 		const unsigned int VERTICES = 4;
// 		const unsigned int TRIANGLES = 2;
// 		unsigned int indices[] = {
// 			0, 1, 2,
// 			2, 3, 0
// 		};
// 		IndexBuffer ib(indices, TRIANGLES * VP_TRIANGLE);

		VertexArray squareVA;
		Geometry* square = new Square();
		ComplexVertexBuffer squareVB = square->bufferFactory();
		squareVA.addBuffer(squareVB, square->bufferLayoutFactory());
		defaultShader->Bind();
		RenderedObject texSquare(squareVA, square, defaultMat, texture, root, defaultShader);
		squareVB.Unbind();
		defaultShader->Unbind();

		VertexArray cubeVA;
		Geometry* cube = new Cube();
		ComplexVertexBuffer cubeVB = cube->bufferFactory();
		cubeVA.addBuffer(cubeVB, cube->bufferLayoutFactory());
		lightShader->Bind();
		RenderedObject box(cubeVA, cube, defaultMat, cubeTexture, root, lightShader);
		cubeVB.Unbind();
		lightShader->Unbind();


		VertexArray fishVA;
		Geometry* fish = new ObjMesh("resources/Obj/fish.obj");
		ComplexVertexBuffer fishVB = fish->bufferFactory();
		fishVA.addBuffer(fishVB, fish->bufferLayoutFactory());
		lightShader->Bind();
		RenderedObject fishR(fishVA, fish, defaultMat, cubeTexture, root, lightShader);
		fishVB.Unbind();
		lightShader->Unbind();






		std::stack<glm::mat4> matrices;
		float currentTime = 0.0f;

		glm::vec3 scaling(5, 5, 5);

		texSquare.Move(glm::vec3(5, -5, -10));
		texSquare.SetScale(scaling);

		box.Move(glm::vec3(-5, 5, -10));
		box.SetScale(scaling);


		fishR.Move(glm::vec3(-2, 5, -10));
		fishR.SetScale(scaling);

		bool isOpened = true;
		//Main application loop
		while(isOpened) {
			//Time in ms telling us when this frame started. Useful for keeping a fix framerate
			uint32_t timeBegin = SDL_GetTicks();

			//Fetch the SDL events
			SDL_Event event;
			while(SDL_PollEvent(&event)) {
				switch(event.type) {
				case SDL_WINDOWEVENT:
					switch(event.window.event) {
					case SDL_WINDOWEVENT_CLOSE:
						isOpened = false;
						break;
					}
					break;
				default:
					camera.inputMove(event);
				}
			}
			
			camera.UpdateView();


			renderer.Clear();
			currentTime += TIME_PER_FRAME_MS;
			root.AfficherRecursif(matrices, currentTime, camera, sun);

			/*{
				glm::mat4 model = glm::translate(glm::mat4(1.0f), translationA);
				glm::mat4 mvp = camera.getProjectionM() * camera.getViewM() * model;
				//shader->SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);
				shader->SetUniformMat4f("u_MVP", mvp);

				renderer.Draw(va, ib, Shader(*shader));
				glDrawArrays(GL_TRIANGLES, 0, square->getNbVertices());
			}*/


			//Display on screen (swap the buffer on screen and the buffer you are drawing on)
			SDL_GL_SwapWindow(window);

			//Time in ms telling us when this frame ended. Useful for keeping a fix framerate
			uint32_t timeEnd = SDL_GetTicks();

			//We want FRAMERATE FPS
			if(timeEnd - timeBegin < TIME_PER_FRAME_MS)
				SDL_Delay(TIME_PER_FRAME_MS - (timeEnd - timeBegin));
		}
	} // inner scope to call all destructors before SDL_GL_DeleteContext
	//Free everything
	if(context != NULL)
		SDL_GL_DeleteContext(context);
	if(window != NULL)
		SDL_DestroyWindow(window);

	return 0;
}
