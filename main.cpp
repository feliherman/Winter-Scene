#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include<windows.h>


#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types
#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include <iostream>



// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 lightDir1;
glm::vec3 lightColor1;
// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;


GLuint lightDirLoc;
GLuint lightColorLoc;

GLuint lightDirLoc1;
GLuint lightColorLoc1;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 10.0f, 20.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.8f;

GLboolean pressedKeys[1024];
bool depthPass = true;
// models
gps::Model3D teapot;
gps::Model3D snowMobil;
gps::Model3D snowMan;
gps::Model3D houseSnow;
gps::Model3D groundObject;
gps::Model3D tree;
gps::Model3D secondTree;
gps::Model3D fence;
gps::Model3D gate;
gps::Model3D cow;
gps::Model3D horse;
gps::Model3D dog;
gps::Model3D wolf;
gps::Model3D dogHouse;
gps::Model3D eagle;
gps::Model3D cart;
gps::Model3D meat;
gps::Model3D lightCube;
gps::Model3D snowflake;
gps::Model3D water;
gps::Model3D water2;
gps::Model3D water3;
gps::Model3D onlywater;

GLfloat angle=0.0f;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader groundShader;
gps::Shader lightShader;
gps::Shader skyboxShader;
gps::SkyBox mySkyBox;

std::vector<const GLchar*> faces;


bool initMouse = true;
float lastX = 400, lastY = 300;
const float sensitivity = 0.01f;

float angleY = 0.0f;
float yaw = 0, pitch = 0;

float angleWater = -10.0f;

const unsigned int SHADOW_WIDTH = 5120;
const unsigned int SHADOW_HEIGHT = 5120;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool wireFrame;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}


glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = -60.0f, far_plane = 60.0f;
    glm::mat4 lightProjection = glm::ortho(-150.0f, 150.0f, -50.0f, 150.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (initMouse)
    {
        lastX = xpos;
        lastY = ypos;
        initMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;


    yaw -= xoffset;
    pitch += yoffset;
    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    GLint viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_M]) {
        if (wireFrame) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        wireFrame = !wireFrame;

    }
}

void initOpenGLWindow() {
    myWindow.Create(1850, 980, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    snowMobil.LoadModel("models/snowmobil/snowMobil.obj");
    snowMan.LoadModel("models/snowMan/snowManSimple.obj");
    houseSnow.LoadModel("models/house/houseFarm.obj");
    groundObject.LoadModel("models/ground/ground.obj");
    tree.LoadModel("models/secondTree/tree.obj");
    fence.LoadModel("models/fence/fence.obj");
    gate.LoadModel("models/Gate/Gate.obj");
    cow.LoadModel("models/cow/cow.obj");
    horse.LoadModel("models/horse/horse.obj");
    dog.LoadModel("models/dog/dog.obj");
    wolf.LoadModel("models/wolf/wolf.obj");
    dogHouse.LoadModel("models/doghouse/doghouse.obj");
    eagle.LoadModel("models/eagle/eagle.obj");
    cart.LoadModel("models/cart/cart.obj");
    meat.LoadModel("models/meat/meat.obj");
    secondTree.LoadModel("models/secondTree/tree2.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    snowflake.LoadModel("models/snowflake/snowflake.obj");
    water.LoadModel("models/water/water.obj");
    water2.LoadModel("models/water/water2.obj");
    water3.LoadModel("models/water/water3.obj");
    onlywater.LoadModel("models/water/onlywater.obj");
}

void initShaders() {

	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depth.vert", "shaders/depth.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();
    
}


void initSkyShader() {

    faces.push_back("textures/skycube/skyrender0002.png");
    faces.push_back("textures/skycube/skyrender0004.png");
    faces.push_back("textures/skycube/skyrender005.png");
    faces.push_back("textures/skycube/skyrender0006.png");
    faces.push_back("textures/skycube/skyrender0001.png");
    faces.push_back("textures/skycube/skyrender0003.png");


    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();


}

void initGroundShader() {

    groundShader.loadShader("shaders/ground.vert","shaders/ground.frag");
    groundShader.useShaderProgram();

}


void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 200.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));



    //light2
    //set the light direction (direction towards the light)
    lightDir1 = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc1 = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir1");
    // send light dir to shader
    glUniform3fv(lightDirLoc1, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir1));

    //set light color
    lightColor1 = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc1 = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor1");
    // send light color to shader
    glUniform3fv(lightColorLoc1, 1, glm::value_ptr(lightColor1));


    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram,"lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

    
}


void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO

    glGenFramebuffers(1, &shadowMapFBO);


    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}




void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}


void renderHouse(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    model1 = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f));
    model1 = glm::rotate(model1, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));


    // draw teapot
    houseSnow.Draw(shader);
}

void renderSnowMan(gps::Shader shader) {
    
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    
    model1 = glm::rotate(model1, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::translate(model1, glm::vec3(20.0f, 0.0f, 0.0f));
    
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    // draw teapot
    snowMan.Draw(shader);
}

void renderSnowManLeft(gps::Shader shader) {

    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);

    model1 = glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::translate(model1, glm::vec3(-20.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));


    // draw teapot
    snowMan.Draw(shader);
}


void renderSnowManShadows(gps::Shader shader) {

    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if(!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));


    // draw teapot
    snowMan.Draw(shader);
}


void drawSkyBox(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

   
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    mySkyBox.Draw(shader, view, projection);
}

void renderGround(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if(!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    // draw teapot
    groundObject.Draw(shader);
    
}

void renderTree1(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, -35.0f));
    model1 = glm::scale(model1, glm::vec3(2.0f));
    
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    tree.Draw(shader);

}


void renderTree2(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-28.0f, 0.0f, 45.0f));
    model1 = glm::scale(model1, glm::vec3(1.5f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    secondTree.Draw(shader);

}


void renderFence(gps::Shader shader,float posXTranslate, float posZTranslate) {
    // select active shader program
    shader.useShaderProgram();


    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(posXTranslate, 0.0f, posZTranslate));
    model1 = glm::scale(model1, glm::vec3(5.0f));


    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    fence.Draw(shader);
}


void renderFenceLeft(gps::Shader shader, float posXTranslate, float posZTranslate) {
    // select active shader program
    shader.useShaderProgram();


    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::translate(model1, glm::vec3(posXTranslate, 0.0f, posZTranslate));
    model1 = glm::scale(model1, glm::vec3(5.0f));


    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    fence.Draw(shader);
}

void renderFenceRight(gps::Shader shader, float posXTranslate, float posZTranslate) {
    // select active shader program
    shader.useShaderProgram();


    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::translate(model1, glm::vec3(posXTranslate, 0.0f, posZTranslate));
    model1 = glm::scale(model1, glm::vec3(5.0f));


    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    fence.Draw(shader);
}


void renderGate(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    

    model1 = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));
    model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 16.7f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    gate.Draw(shader);

}

void renderCow(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-21.0f, 0.0f, -35.0f));
    model1 = glm::scale(model1, glm::vec3(0.05f));
    model1 = glm::rotate(model1, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    cow.Draw(shader);

}


void renderCowLeft(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-35.0f, 0.0f, -30.0f));
    model1 = glm::scale(model1, glm::vec3(0.05f));
    model1 = glm::rotate(model1, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    cow.Draw(shader);

}



void renderHorse(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, -10.0f));
    model1 = glm::scale(model1, glm::vec3(0.04f));
    model1 = glm::rotate(model1, glm::radians(240.0f), glm::vec3(0.0f, 1.0f, 0.0f));


    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    horse.Draw(shader);

}

void renderDog(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::rotate(glm::mat4(1.0f), glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::translate(model1, glm::vec3(6.0f, 0.0f, 38.0f));
    model1 = glm::scale(model1, glm::vec3(0.6f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    dog.Draw(shader);

}

void renderWolf(gps::Shader shader,float deplasamentX, float deplasamentZ, float rotation) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(deplasamentX, 0.0f, deplasamentZ));
    model1 = glm::scale(model1, glm::vec3(0.05f));
    model1 = glm::rotate(model1, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    wolf.Draw(shader);

}

void renderDogHouse(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(37.0f, 0.0f, 26.0f));
    model1 = glm::scale(model1, glm::vec3(0.8f));
    model1 = glm::rotate(model1, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    dogHouse.Draw(shader);

}

void renderEagle(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader
    model1 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::translate(model1, glm::vec3(6.4f,10.32f, -0.15f));
    model1 = glm::scale(model1, glm::vec3(3.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    eagle.Draw(shader);

}


void renderCart(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);

    model1 = glm::rotate(glm::mat4(1.0f), glm::radians(70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::scale(model1, glm::vec3(1.5f));
    model1 = glm::translate(model1, glm::vec3(1.0f, 0.0f, -26.5f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    cart.Draw(shader);

}


void renderMeat(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 0.0f, 29.0f));
    model1 = glm::scale(model1, glm::vec3(0.3f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    meat.Draw(shader);

}

void renderSecondMeat(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(30.2f, 0.0f, 28.5f));
    model1 = glm::scale(model1, glm::vec3(0.3f));
    model1 = glm::rotate(model1, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    meat.Draw(shader);

}

void renderSnowflakes(gps::Shader shader , float translateX, float translateY, float translateZ) {
    // select active shader program
    shader.useShaderProgram();
    
    glm::mat4 model1 = glm::mat4(1.0f);
    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(translateX, translateY , translateZ));
    model1 = glm::scale(model1, glm::vec3(0.02f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

        // draw teapot
    snowflake.Draw(shader);
    
    }
    
void generateSnowflakes(gps::Shader shader) {

    
    for (int i = 0; i < 500; i++)
    {
        float translateX = (rand() % 180) - 90;
        float translateY = rand() % 30;
        float translateZ = (rand() % 180) - 90;
        renderSnowflakes(shader,translateX, translateY, translateZ);
        
    }
}

void renderWater(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-29.3f, 0.0f, -36.0f));
    model1 = glm::scale(model1, glm::vec3(0.05f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    water.Draw(shader);

}

void renderDogWater(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(33.8f, 0.0f, 24.0f));
    model1 = glm::scale(model1, glm::vec3(0.05f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    water2.Draw(shader);

}

void renderDogWaterMovement(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    //send teapot model matrix data to shader

    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(33.8f, 0.0f, 24.0f));
    model1 = glm::scale(model1, glm::vec3(0.05f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    water3.Draw(shader);

}

void renderDogOnlyWaterMovement(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    glm::mat4 model1 = glm::mat4(1.0f);
    angleWater += 0.001f;
    model1 = glm::translate(glm::mat4(1.0f), glm::vec3(33.8f, 0.0f, 24.0f));
    model1 = glm::scale(model1, glm::vec3(0.05f));
    model1 = glm::rotate(model1, glm::radians(angleWater), glm::vec3(1.0f, 0.0f, 0.0f));


    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model1));

    if (!depthPass)
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * model1))));

    onlywater.Draw(shader);

}

/*void generateAndRenderWater(gps::Shader shader) {

    

    for(int i = 0 ; i<100 ; i++)
    {
        
        angleWater += 0.01f;
        renderDogOnlyWaterMovement(shader, angleWater);
        
    }
    
    for (int i = 0; i < 100; i++)
    {

        angleWater -= 0.01f;
        renderDogOnlyWaterMovement(shader, angleWater);
       
    }
    
    
}*/


void renderObjects(gps::Shader shader, bool depthPass) {

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    renderGround(shader);
    //renderSnowManShadows(shader);
    renderHouse(shader);

    renderSnowMan(shader);
    renderSnowManLeft(shader);

    renderTree1(shader);
    renderTree2(shader);


    renderFence(shader, 41.0f, -49.0f);
    renderFence(shader, 30.0f, -49.0f);
    renderFence(shader, 19.0f, -49.0f);
    renderFence(shader, 8.0f, -49.0f);
    renderFence(shader, -3.0f, -49.0f);
    renderFence(shader, -14.0f, -49.0f);
    renderFence(shader, -25.0f, -49.0f);
    renderFence(shader, -36.0f, -49.0f);

    renderFenceLeft(shader, 45.0f, -43.0f);
    renderFenceLeft(shader, 34.0f, -43.0f);
    renderFenceLeft(shader, 23.0f, -43.0f);
    renderFenceLeft(shader, 12.0f, -43.0f);
    renderFenceLeft(shader, 1.0f, -43.0f);
    renderFenceLeft(shader, -10.0f, -43.0f);
    renderFenceLeft(shader, -21.0f, -43.0f);
    renderFenceLeft(shader, -32.0f, -43.0f);
    renderFenceLeft(shader, -43.0f, -43.0f);

    renderFenceRight(shader, 46.0f, -45.0f);
    renderFenceRight(shader, 35.0f, -45.0f);
    renderFenceRight(shader, 24.0f, -45.0f);
    renderFenceRight(shader, 13.0f, -45.0f);
    renderFenceRight(shader, 2.0f, -45.0f);
    renderFenceRight(shader, -9.0f, -45.0f);
    renderFenceRight(shader, -20.0f, -45.0f);
    renderFenceRight(shader, -31.0f, -45.0f);
    renderFenceRight(shader, -42.0f, -45.0f);


    renderFence(shader, 41.0f, 50.0f);
    renderFence(shader, 30.0f, 50.0f);
    renderFence(shader, 19.0f, 50.0f);
    renderFence(shader, 12.0f, 50.0f);
    renderFence(shader, -9.0f, 50.0f);
    renderFence(shader, -16.0f, 50.0f);
    renderFence(shader, -25.0f, 50.0f);
    renderFence(shader, -36.0f, 50.0f);


    renderFenceLeft(shader, 45.0f, -15.0f);
    renderFenceLeft(shader, 34.0f, -15.0f);
    renderFenceLeft(shader, 23.0f, -15.0f);
    renderFenceLeft(shader, 12.0f, -15.0f);
    renderFence(shader, -19.0f, -5.0f);
    renderFence(shader, -25.0f, -5.0f);
    renderFence(shader, -36.0f, -5.0f);




    renderGate(shader);

    renderCow(shader);
    renderCowLeft(shader);

    renderHorse(shader);

    renderDog(shader);

    renderWolf(shader, 60.0f, -40.0f, -45.0f);
    renderWolf(shader, 66.0f, -60.0f, -40.0f);
    renderWolf(shader, 68.0f, -48.0f, -25.0f);
    renderWolf(shader, 60.0f, -30.0f, -30.0f);
    renderWolf(shader, 55.0f, -50.0f, -20.0f);
    renderWolf(shader, 53.0f, -32.0f, -70.0f);


    renderDogHouse(shader);

    renderEagle(shader);

    renderCart(shader);

    renderMeat(shader);
    renderSecondMeat(shader);
    renderWater(shader);
    renderDogWater(shader);
    /*renderDogWaterMovement(shader);
    renderDogOnlyWaterMovement(shader);*/
    //generateAndRenderWater(shader);
    //renderSnowflakes(shader);
    generateSnowflakes(shader);

}
void renderScene() {

    depthPass = true;
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    depthPass = false;
    // final scene rendering pass (with shadows)

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    drawSkyBox(skyboxShader);
    //mySkyBox.Draw(skyboxShader, view, projection);
    renderObjects(myBasicShader, false);
    



}


void viewPresentation() {

        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        renderScene();
}


void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();

	initShaders();
    
    initSkyShader();
    initGroundShader();
    
    initUniforms();
    initFBO();

    setWindowCallbacks();

   


	glCheckError();
	// application loop

    while (myCamera.getPosition().z < glm::vec3(0.0f, 0.0f, 80.0f).z) {

        viewPresentation();
        glfwSwapBuffers(myWindow.getWindow());
        //printf("avem z ul ca fiind %f\n", myCamera.getPosition().z);
    }


	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		//glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
