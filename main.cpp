#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <math.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"

#define ClearOpenGLErrors() _check_gl_error(__FILE__,__LINE__)

/*void // ClearOpenGLErrors() {
	GLenum error = glGetError();
	if(error != GL_NO_ERROR) {
		std::cout << gluErrorString(error) << std::endl;
	}
}*/

void _check_gl_error(const char *file, int line) {
        GLenum err (glGetError());
 
        while(err!=GL_NO_ERROR) {
                std::string error;
 
                switch(err) {
                        case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
                        case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
                        case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
                        case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
                        case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
                }
 
                std::cerr << gluErrorString(err) <<" - "<<file<<":"<<line<<std::endl;
                err=glGetError();
        }
}

unsigned char *data1;
bool vertchangeup = false;
bool vertchangedown = false;
std::vector<unsigned char> normalsVec;
int VERTICES = 10;
std::vector<glm::vec3> vert;
std::vector<unsigned int> indices;
unsigned int EBO;
SDL_Window* mainwindow;
SDL_GLContext maincontext;
float deltatime = 0.0f;
float lastframe = 0.0f;
bool running = true;
glm::vec3 lightPos(0.0f, 40.0f, 0.0f);
Camera camera(glm::vec3(0.0, 20.0f, 30.0f));
bool wireframe = false;
int lastx = 1366/2, lasty = 768/2;
bool firstmouse = true;
bool useHeights = false;
int BINARYITER = 5;
const float TERRAIN_SIZE = 60.0f;

void renderQuad();
bool Init();

void InputProcess(SDL_Event event);

unsigned int createTerrain(const unsigned char* heightMap, int width);

unsigned int createAxes();

unsigned int createLamp();

void createNormalMap(const unsigned char* heightMap, int width);

template <typename T, typename I, typename O>
int MapInRange(T x, I in_min, I in_max, O out_min, O out_max)
{
	if(x < in_min) x = in_min;
	if(x > in_max) x = in_max;
	return (int)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
bool sucessoo = Init();
//Shader ourShader("shader.vs", "shader.fs");
//Shader ourTessShader("vertexShader.glsl", "tcShader.glsl", "teShader.glsl", "fragShader.glsl");
Shader ourTessShader("vertexShader.glsl", "test.tesc", "test.tese", "geometryShader.glsl", "fragShader.glsl");
//Shader ourTessShader("vertexShader.glsl", "fragShader.glsl");
Shader our2Shader("shader2.vs", "shader2.fs");

bool Init(){
	if (SDL_Init(SDL_INIT_VIDEO) < 0){
	    std::cout << "Failed to init SDL\n";
	    return false;
	}


	// Create our window centered at 512x512 resolution
	mainwindow = SDL_CreateWindow(
	    "Incrivel janela",
	    SDL_WINDOWPOS_CENTERED,
	    SDL_WINDOWPOS_CENTERED,
	    1366,
	    768,
	    SDL_WINDOW_OPENGL
	);


	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	maincontext = SDL_GL_CreateContext(mainwindow);
	if(maincontext == NULL){
		std::cout<<"failed to create context"<<std::endl;
		return false;
	}

	glewExperimental = GL_TRUE;

	GLenum status = glewInit();

	if(status != GLEW_OK){
		std::cout<<"failed to init glew"<<std::endl;
		return false;
	}

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	return true;
}

double getEntropy(int topLeftX, int topLeftY, int width, const int offset, const char* heightMap){
    std::vector<float> probs(256, 0);

    for(int i = topLeftY; i < topLeftY + offset; i++){
        for(int j = topLeftX; j < topLeftX + offset; j++){
            int curIndex = ((i * (width-1) + j) * 3);
            probs[heightMap[curIndex]]++;
        }
    }
    float entropy = 0;
    float entropySum = 0;
    int setSize = (offset * offset);
    for(int i = 0; i < 256; i++){
        if(probs[i] == 0) continue;
        entropySum += ((float(probs[i]/setSize)) * log2(float(probs[i]/setSize)));
        std::cout << entropySum << std::endl;
    }
    entropy = entropySum * -1;

    return entropy;
    
}

void InputProcess(SDL_Event event){

	const Uint8* keyboardSnapshot = SDL_GetKeyboardState(NULL);
	int x, y;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT)
			running = false;

		if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
		if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x){
			vertchangeup = true;

		}
		if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_z){
			vertchangedown = true;

		}

		if(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)){
			if(firstmouse == true){
				lastx = x;
        		lasty = y;
				firstmouse = false;
			}

			camera.ProcessMouseMovement(x - lastx, lasty - y, deltatime);
			lastx = x;
			lasty = y;
		} else {
			lastx = x;
        	lasty = y;
		}

		if(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_MIDDLE)){
			std::cout<<"teste mem"<<std::endl;
			if(firstmouse == true){
				lastx = x;
        		lasty = y;
				firstmouse = false;
			}

			camera.ProcessMouseScroll(lasty - y);
			lastx = x;
			lasty = y;
			} else {
				lastx = x;
				lasty = y;
			}

	}

	if(keyboardSnapshot[SDL_SCANCODE_W]) camera.ProcessKeyboard(FORWARD, deltatime);
	if(keyboardSnapshot[SDL_SCANCODE_S]) camera.ProcessKeyboard(BACKWARD, deltatime);
	if(keyboardSnapshot[SDL_SCANCODE_D]) camera.ProcessKeyboard(RIGHT, deltatime);
	if(keyboardSnapshot[SDL_SCANCODE_A]) camera.ProcessKeyboard(LEFT, deltatime);
	if(keyboardSnapshot[SDL_SCANCODE_T]) {
		ourTessShader.use();
		useHeights = true;
		ourTessShader.setBool("useHeights", useHeights);
	}
	if(keyboardSnapshot[SDL_SCANCODE_G]) {
		ourTessShader.use();
		useHeights = false;
		ourTessShader.setBool("useHeights", useHeights);
	}
	if(keyboardSnapshot[SDL_SCANCODE_KP_PLUS]){
		ourTessShader.use();
		BINARYITER++;
		std::cout<<"Binary shearch num Iterations = "<<BINARYITER<<std::endl;
		ourTessShader.setInt("binaryIter", BINARYITER);
	}
	if(keyboardSnapshot[SDL_SCANCODE_KP_MINUS]){
		ourTessShader.use();
		BINARYITER--;
		std::cout<<"Binary shearch num Iterations = "<<BINARYITER<<std::endl;
		ourTessShader.setInt("binaryIter", BINARYITER);
	}
	/*if(keyboardSnapshot[SDL_SCANCODE_W]) camera.ProcessMouseMovement(0, 10.0f, deltatime);
	if(keyboardSnapshot[SDL_SCANCODE_S]) camera.ProcessMouseMovement(0, -10.0f, deltatime);
	if(keyboardSnapshot[SDL_SCANCODE_D]) camera.ProcessMouseMovement(10.0f, 0, deltatime);
	if(keyboardSnapshot[SDL_SCANCODE_A]) camera.ProcessMouseMovement(-10.0f, 0, deltatime);*/
	if(keyboardSnapshot[SDL_SCANCODE_L]) lightPos.x += 1.0f * deltatime;
	if(keyboardSnapshot[SDL_SCANCODE_J]) lightPos.x	-= 1.0f * deltatime;
	if(keyboardSnapshot[SDL_SCANCODE_I]) lightPos.z	+= 1.0f * deltatime;
	if(keyboardSnapshot[SDL_SCANCODE_K]) lightPos.z	-= 1.0f * deltatime;
	if(keyboardSnapshot[SDL_SCANCODE_M]) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(keyboardSnapshot[SDL_SCANCODE_N]) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(keyboardSnapshot[SDL_SCANCODE_P]){
		ourTessShader.use();
		ourTessShader.setInt("parFlag", 1);
	}
	if(keyboardSnapshot[SDL_SCANCODE_B]){
		ourTessShader.use();
		ourTessShader.setInt("parFlag", 2);
	}
	if(keyboardSnapshot[SDL_SCANCODE_Q]){
		ourTessShader.use();
		ourTessShader.setInt("parFlag", 0);
	}


}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-TERRAIN_SIZE,  0.0f, TERRAIN_SIZE);
        glm::vec3 pos2(-TERRAIN_SIZE, 0.0f, -TERRAIN_SIZE);
        glm::vec3 pos3( TERRAIN_SIZE, 0.0f, -TERRAIN_SIZE);
        glm::vec3 pos4( TERRAIN_SIZE,  0.0f, TERRAIN_SIZE);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 1.0f, 0.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
				// // ClearOpenGLErrors();
        glGenBuffers(1, &quadVBO);
				// ClearOpenGLErrors();
        glBindVertexArray(quadVAO);
				// ClearOpenGLErrors();
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
				// ClearOpenGLErrors();
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
				// ClearOpenGLErrors();
        glEnableVertexAttribArray(0);
				// ClearOpenGLErrors();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
				// ClearOpenGLErrors();
        glEnableVertexAttribArray(1);
				// ClearOpenGLErrors();
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
				// ClearOpenGLErrors();
        glEnableVertexAttribArray(2);
				// ClearOpenGLErrors();
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
				// ClearOpenGLErrors();
        glEnableVertexAttribArray(3);
				// ClearOpenGLErrors();
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
				// ClearOpenGLErrors();
        glEnableVertexAttribArray(4);
				// ClearOpenGLErrors();
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
				// ClearOpenGLErrors();
    }
		// ClearOpenGLErrors();
		glPatchParameteri(GL_PATCH_VERTICES, 3);
    glBindVertexArray(quadVAO);
		// ClearOpenGLErrors();
    glDrawArrays(GL_PATCHES, 0, 6);
		// ClearOpenGLErrors();
    glBindVertexArray(0);
		// ClearOpenGLErrors();
}

unsigned int createTerrain(const unsigned char* heightMap, int width){
	vert.clear();
	indices.clear();
	int nVerticesX, nVerticesY;
	nVerticesX = nVerticesY = VERTICES;

	float curZ = +TERRAIN_SIZE;
	for(int i=0; i<nVerticesX; i++){

		float curX = -TERRAIN_SIZE;

		for(int j=0; j<nVerticesY; j++){


			int heightMapX = MapInRange(curX, -TERRAIN_SIZE, TERRAIN_SIZE, 0, 1024);
			int heightMapZ = MapInRange(curZ, -TERRAIN_SIZE, TERRAIN_SIZE, 0, 1024);
			if(heightMapX > 1024){
				std::cout<<"Out of range = "<<heightMapX<<" "<<curX<<std::endl;
			}
			int vertexHeight = heightMap[((heightMapZ * width + heightMapX) * 3)];
			float curY = vertexHeight * 0.023;


			vert.push_back(glm::vec3(curX, curY, curZ));
			vert.push_back(glm::vec3(0.0f, 0.0f, 0.0f));


			curX+=29.8 / nVerticesX;
		}
		curZ-=29.8/nVerticesY;
	}


	for(int i=0; i < (nVerticesX - 1) * (nVerticesY); i+=1){
		if(vert[i * 2].z != vert[(i * 2) + 2].z) continue;
		//if((i+2)%150==0) continue;
		indices.push_back(i);
		indices.push_back(i+1);
		indices.push_back(i+(nVerticesX));
		indices.push_back(i+1);
		indices.push_back(i + 1 + nVerticesX);
		indices.push_back(i + nVerticesX);
	}
//	float nMin = 5465484, nMax = -50;
	for(unsigned int i=0; i<indices.size(); i += 3){
		glm::vec3 v1 = vert[indices[i+ 1] * 2] - vert[indices[i] * 2];
		glm::vec3 v2 = vert[indices[i+ 2] * 2] - vert[indices[i] * 2];
		//std::cout<<vert[indices[i+ 2] * 2].x<<" "<<vert[indices[i] * 2].x<<std::endl;

		glm::vec3 normal = glm::cross(v1, v2);

		vert[indices[i] * 2 + 1] += normal;
		vert[indices[i+1] * 2 + 1] += normal;
		vert[indices[i+2] * 2 + 1] += normal;

	}


	//for(int i=0; i< 900; i+=3) std::cout<<(unsigned int)normalsVec[i]<<" "<<(unsigned int)normalsVec[i+1]<<" "<<(unsigned int)normalsVec[i+2]<<std::endl;




	//------------------------------BUFFERS----------------------------------------//

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * vert.size() * sizeof(float), &vert[0], GL_STATIC_DRAW);


	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	//Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

void createNormalMap(const unsigned char* heightMap, int width){
	int nVerticesX, nVerticesY;
	nVerticesX = nVerticesY = 1025;
	float curZ = +TERRAIN_SIZE;
	for(int i=0; i<nVerticesX; i++){

		float curX = -TERRAIN_SIZE;

		for(int j=0; j<nVerticesY; j++){


			int heightMapX = MapInRange(curX, -TERRAIN_SIZE, TERRAIN_SIZE, 0, 1024);
			int heightMapZ = MapInRange(curZ, -TERRAIN_SIZE, TERRAIN_SIZE, 0, 1024);
			if(heightMapX > 1024){
				std::cout<<"Out of range = "<<heightMapX<<" "<<curX<<std::endl;
			}
			int vertexHeight = heightMap[((heightMapZ * width + heightMapX) * 3)];
			float curY = vertexHeight * 0.023;


			vert.push_back(glm::vec3(curX, curY, curZ));
			vert.push_back(glm::vec3(0.0f, 0.0f, 0.0f));


			curX+=(TERRAIN_SIZE * 2) / nVerticesX;
		}
		curZ-=(TERRAIN_SIZE * 2) /nVerticesY;
	}
	std::cout<<"Create vertices"<<std::endl;


	for(int i=0; i < (nVerticesX - 1) * (nVerticesY); i+=1){
		if(vert[i * 2].z != vert[(i * 2) + 2].z) continue;
		//if((i+2)%150==0) continue;
		indices.push_back(i);
		indices.push_back(i+1);
		indices.push_back(i+(nVerticesX));
		indices.push_back(i+1);
		indices.push_back(i + 1 + nVerticesX);
		indices.push_back(i + nVerticesX);
	}
	float nMin = 5465484, nMax = -50;
	for(unsigned int i=0; i<indices.size(); i += 3){
		glm::vec3 v1 = vert[indices[i+ 1] * 2] - vert[indices[i] * 2];
		glm::vec3 v2 = vert[indices[i+ 2] * 2] - vert[indices[i] * 2];
		//std::cout<<vert[indices[i+ 2] * 2].x<<" "<<vert[indices[i] * 2].x<<std::endl;

		glm::vec3 normal = glm::cross(v1, v2);

		vert[indices[i] * 2 + 1] += normal;
		vert[indices[i+1] * 2 + 1] += normal;
		vert[indices[i+2] * 2 + 1] += normal;
		if(vert[indices[i] * 2 + 1].x > nMax) nMax = vert[indices[i] * 2 + 1].x;
		if(vert[indices[i] * 2 + 1].x < nMin) nMin = vert[indices[i] * 2 + 1].x;
		if(vert[indices[i] * 2 + 1].y > nMax) nMax = vert[indices[i] * 2 + 1].y;
		if(vert[indices[i] * 2 + 1].y < nMin) nMin = vert[indices[i] * 2 + 1].y;
		if(vert[indices[i] * 2 + 1].z > nMax) nMax = vert[indices[i] * 2 + 1].z;
		if(vert[indices[i] * 2 + 1].z < nMin) nMin = vert[indices[i] * 2 + 1].z;

		if(vert[indices[i+1] * 2 + 1].x > nMax) nMax = vert[indices[i+1] * 2 + 1].x;
		if(vert[indices[i+1] * 2 + 1].x < nMin) nMin = vert[indices[i+1] * 2 + 1].x;
		if(vert[indices[i+1] * 2 + 1].y > nMax) nMax = vert[indices[i+1] * 2 + 1].y;
		if(vert[indices[i+1] * 2 + 1].y < nMin) nMin = vert[indices[i+1] * 2 + 1].y;
		if(vert[indices[i+1] * 2 + 1].z > nMax) nMax = vert[indices[i+1] * 2 + 1].z;
		if(vert[indices[i+1] * 2 + 1].z < nMin) nMin = vert[indices[i+1] * 2 + 1].z;

		if(vert[indices[i+2] * 2 + 1].x > nMax) nMax = vert[indices[i+2] * 2 + 1].x;
		if(vert[indices[i+2] * 2 + 1].x < nMin) nMin = vert[indices[i+2] * 2 + 1].x;
		if(vert[indices[i+2] * 2 + 1].y > nMax) nMax = vert[indices[i+2] * 2 + 1].y;
		if(vert[indices[i+2] * 2 + 1].y < nMin) nMin = vert[indices[i+2] * 2 + 1].y;
		if(vert[indices[i+2] * 2 + 1].z > nMax) nMax = vert[indices[i+2] * 2 + 1].z;
		if(vert[indices[i+2] * 2 + 1].z < nMin) nMin = vert[indices[i+2] * 2 + 1].z;

	}

//	float min = 15455, max = -800;
	for(unsigned int i=1; i<vert.size(); i += 2){
		//std::cout<<vert[i].x<<" "<<vert[i].y<<" "<<vert[i].z<<std::endl;
		normalsVec.push_back((unsigned char)MapInRange(vert[i].x, nMin, nMax, 0, 255));
		normalsVec.push_back((unsigned char)MapInRange(vert[i].y, nMin, nMax, 0, 255));
		normalsVec.push_back((unsigned char)MapInRange(vert[i].z, nMin, nMax, 0, 255));


	}
}

unsigned int createAxes(){
	float lines[] = {
	 	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	 	100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

	 	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	 	0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f,

	 	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

unsigned int createLamp(){
	float vertices[] = {
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

		-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,

		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,

		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

int main(int argc, char* args[]){
	std::cout<<"start"<<std::endl;

	ourTessShader.use();

	unsigned int VAO2 = createAxes();

	unsigned int lampVAO = createLamp();

	GLuint texture1, texture2, normalTexture;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	// set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps

    int width, height, nrChannels;
    //stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data1 = stbi_load("heightmap.jpg", &width, &height, &nrChannels, 0);

    if (data1 != NULL){
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
        glGenerateMipmap(GL_TEXTURE_2D);
		GLenum error = glGetError();
		if(error != GL_NO_ERROR) std::cout << gluErrorString(error)<<std::endl;
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
	createNormalMap(data1, 1025);
	unsigned int VAO = createTerrain(data1, 1025);


	unsigned char *data;
	 // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("ps_texture_1k.png", &width, &height, &nrChannels, 0);

    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
		GLenum error = glGetError();
		if(error != GL_NO_ERROR) std::cout << gluErrorString(error)<<std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	if (normalsVec.data() != NULL){
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1025, 1025, 0, GL_RGB, GL_UNSIGNED_BYTE, normalsVec.data());
        glGenerateMipmap(GL_TEXTURE_2D);
		GLenum error = glGetError();
		if(error != GL_NO_ERROR) std::cout << gluErrorString(error)<<std::endl;
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

	ourTessShader.use();
	glUniform1i(glGetUniformLocation(ourTessShader.ID, "texture1"), 0);
	ourTessShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	ourTessShader.setInt("texture2", 1);
	ourTessShader.setInt("normalTexture", 2);

	glEnable(GL_DEPTH_TEST);


	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)800/(float)600, 0.1f, 400.0f);
	unsigned int projectLoc = glGetUniformLocation(ourTessShader.ID, "projection");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));


	while(running){
		if(vertchangeup == true){
			VERTICES*=1.5;
			std::cout<<VERTICES<<std::endl;

			VAO = createTerrain(data1, 1025);
			vertchangeup = false;
		}
		if(vertchangedown == true){
			VERTICES/= 2;
			if(VERTICES <= 6)
				VERTICES = 6;
			VAO = createTerrain(data1, 1025);
			vertchangedown = false;
		}
		// ClearOpenGLErrors();
		SDL_Event event;
		float currentframe = (float)SDL_GetTicks()/100;
		deltatime = currentframe - lastframe;
		lastframe = currentframe;
		// ClearOpenGLErrors();

		glClearColor(0.2, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		// ClearOpenGLErrors();

		ourTessShader.use();
		// ClearOpenGLErrors();
		glBindVertexArray(VAO);
		glm::mat4 view;
		// ClearOpenGLErrors();

		view = camera.GetViewMatrix();
		unsigned int viewLoc = glGetUniformLocation(ourTessShader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));


		glm::mat4 model = glm::mat4(1.0);
		unsigned int modelLoc = glGetUniformLocation(ourTessShader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// ClearOpenGLErrors();

		ourTessShader.setVec3("lightPos", lightPos);
		// ClearOpenGLErrors();

		ourTessShader.setVec3("viewPos", camera.Position);
		// ClearOpenGLErrors();


		renderQuad();
		// ClearOpenGLErrors();
		our2Shader.use();
		// ClearOpenGLErrors();

		glBindVertexArray(VAO2);
		// ClearOpenGLErrors();

		projectLoc = glGetUniformLocation(our2Shader.ID, "projection");
		glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));
		// ClearOpenGLErrors();

		viewLoc = glGetUniformLocation(our2Shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// ClearOpenGLErrors();

		glm::mat4 model2 = glm::mat4(1.0);

		modelLoc = glGetUniformLocation(our2Shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		// ClearOpenGLErrors();


		glDrawArrays(GL_LINES, 0, 12);

		our2Shader.use();

		glBindVertexArray(lampVAO);

		projectLoc = glGetUniformLocation(our2Shader.ID, "projection");
		glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));

		viewLoc = glGetUniformLocation(our2Shader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		model2 = glm::mat4(1.0);
		model2 = glm::translate(model2, lightPos);
		model2 = glm::scale(model2, glm::vec3(1.0f));
		modelLoc = glGetUniformLocation(our2Shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));


    glPatchParameteri(GL_PATCH_VERTICES, 3);
		glDrawArrays(GL_PATCHES, 0, 12);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		SDL_GL_SwapWindow(mainwindow);

		InputProcess(event);
	}

	glDeleteVertexArrays(1, &VAO);


	SDL_Quit();

	return 0;

}