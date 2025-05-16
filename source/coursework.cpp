#include <iostream>
#include <cmath>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

// Object struct
struct Object
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    float angle = 0.0f;
    std::string name;
};

// Function prototypes
void keyboardInput(GLFWwindow* window);
void mouseInput(GLFWwindow* window);
void checkCollision(Camera& camera, Object& obj1);

// Frame timers
float previousTime = 0.0f;  // time of previous iteration of the loop
float deltaTime = 0.0f;  // time elapsed since the previous frame

// Create camera object
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
//3D Camera bool
bool thirdCamera = false;
bool fourthCamera = false;

float playerYaw = 0.0f;



int main(void)
{
    // =========================================================================
    // Window creation - you shouldn't need to change this code
    // -------------------------------------------------------------------------
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(1024, 768, "Computer Graphics Coursework", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    // -------------------------------------------------------------------------
    // End of window creation
    // =========================================================================

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Use back face culling
    glEnable(GL_CULL_FACE);

    // Ensure we can capture keyboard inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Capture mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Compile shader program
    unsigned int shaderID, lightShaderID, skyboxShaderID;
    shaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    lightShaderID = LoadShaders("lightVertexShader.glsl", "lightFragmentShader.glsl");
    skyboxShaderID = LoadShaders("skyboxVertexShader.glsl", "skyboxFragmentShader.glsl");

    // Activate shader
    glUseProgram(shaderID);

    //Skybox
    
    float skyboxVertices[] =
    {
        //   Coordinates
        -1.0f, -1.0f,  1.0f,//        7--------6
         1.0f, -1.0f,  1.0f,//       /|       /|
         1.0f, -1.0f, -1.0f,//      4--------5 |
        -1.0f, -1.0f, -1.0f,//      | |      | |
        -1.0f,  1.0f,  1.0f,//      | 3------|-2
         1.0f,  1.0f,  1.0f,//      |/       |/
         1.0f,  1.0f, -1.0f,//      0--------1
        -1.0f,  1.0f, -1.0f
    };

    unsigned int skyboxIndices[] =
    {
        // Right
        1, 2, 6,
        6, 5, 1,
        // Left
        0, 4, 7,
        7, 3, 0,
        // Top
        4, 5, 6,
        6, 7, 4,
        // Bottom
        0, 3, 2,
        2, 1, 0,
        // Back
        0, 1, 5,
        5, 4, 0,
        // Front
        3, 7, 6,
        6, 2, 3
    };


    // Add light sources
    Light lightSources;
    lightSources.addPointLight("blueLight", glm::vec3(-5.07f, 0.4f, 0.22f),         // position
        glm::vec3(0.0f, 0.0f, 1.0f),         // colour
        1.0f, 0.1f, 0.02f);                  // attenuation

    lightSources.addPointLight("redLight", glm::vec3(-4.4f, 0.4f, 0.22f),        // position
        glm::vec3(1.0f, 0.0f, 0.0f),         // colour
        1.0f, 0.1f, 0.02f);                  // attenuation

    lightSources.addSpotLight("streetLight", glm::vec3(3.8f, 3.1f, -0.17f),          // position
        glm::vec3(0.0f, -1.0f, 0.0f),         // direction
        glm::vec3(1.0f, 1.0f, 0.0f),          // colour
        1.0f, 0.1f, 0.02f,                    // attenuation
        std::cos(Maths::radians(45.0f)));     // cos(phi)

    lightSources.addDirectionalLight("sun", glm::vec3(1.0f, -1.0f, 0.0f),  // direction
                                     glm::vec3(1.0f, 1.0f, 1.0f));  // colour


    // Load models
    Model teapot("../assets/teapot.obj");
    Model sphere("../assets/sphere.obj");
    Model cube("../assets/cube.obj");

    // Load the textures
    teapot.addTexture("../assets/blue.bmp", "diffuse");
    teapot.addTexture("../assets/diamond_normal.png", "normal");
    teapot.addTexture("../assets/neutral_specular.png", "specular");

    // Define teapot object lighting properties
    teapot.ka = 0.2f;
    teapot.kd = 0.7f;
    teapot.ks = 1.0f;
    teapot.Ns = 20.0f;

    // Teapot positions
    glm::vec3 teapotPositions[] = {
        glm::vec3(-4.0f,-0.2f, -5.0f),
        glm::vec3(-2.0f,-0.2f, -5.0f),
        glm::vec3(2.0f, -0.2f, -5.0f),
    };

    // Add teapots to objects vector
    std::vector<Object> objects;
    Object object;
    object.name = "teapot";
    for (unsigned int i = 0; i <= teapotPositions->length(); i++)
    {
        object.position = teapotPositions[i];
        object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
        object.scale = glm::vec3(0.25f, 0.25f, 0.25f);
        object.angle = Maths::radians(0.0f);
        objects.push_back(object);
    }
    object.name = "specialTeapot";
    object.position = glm::vec3(0.0f, -0.2f, -5.0f);
    object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    object.scale = glm::vec3(0.25f, 0.25f, 0.25f);
    object.angle = Maths::radians(40.0f);
    objects.push_back(object);

    // Load the textures
   cube.addTexture("../assets/crate.jpg", "diffuse");
   cube.addTexture("../assets/crate.jpg", "normal");
   cube.addTexture("../assets/crate.jpg", "specular");

    // Define cube object lighting properties
    cube.ka = 0.2f;
    cube.kd = 0.7f;
    cube.ks = 1.0f;
    cube.Ns = 20.0f;

    // cube positions
    glm::vec3 cratePositions[] = {
        glm::vec3(-4.0f, -0.6f, -5.0f),
        glm::vec3(-2.0f, -0.6f, -5.0f),
        glm::vec3(0.0f, -0.6f, -5.0f),
        glm::vec3(2.0f, -0.6f, -5.0f),
    };

    // Add cube to objects vector
    object.name = "crate";
    for (unsigned int i = 0; i <= cratePositions->length(); i++)
    {
        object.position = cratePositions[i];
        object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
        object.scale = glm::vec3(0.25f, 0.25f, 0.25f);
        object.angle = Maths::radians(0.0f);
        objects.push_back(object);
    }

    // Load a 2D plane model for the floor and add textures
    Model floor("../assets/plane.obj");
    floor.addTexture("../assets/stones_diffuse.png", "diffuse");
    floor.addTexture("../assets/stones_normal.png", "normal");
    floor.addTexture("../assets/stones_specular.png", "specular");

    // Define floor light properties
    floor.ka = 0.2f;
    floor.kd = 1.0f;
    floor.ks = 1.0f;
    floor.Ns = 20.0f;

    // Add floor model to objects vector
    object.position = glm::vec3(0.0f, -0.85f, 0.0f);
    object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    object.angle = 0.0f;
    object.name = "floor";
    objects.push_back(object);


    //WALLS

    glm::vec3 wallPositions[] = {
     glm::vec3(0.0f,  4.0f,  -8.0f),
     //glm::vec3(0.0f, 4.0f, 8.0f),
    };

    glm::vec3 wallRotations[] = {
     glm::vec3(1.0f,  0.0f,  0.0f),
     //glm::vec3(-1.0f, 0.0f, 0.0f),
    };
    //Load Wall
    Model wall("../assets/plane.obj");
    wall.addTexture("../assets/bricks_diffuse.png", "diffuse");
    wall.addTexture("../assets/bricks_normal.png", "normal");
    wall.addTexture("../assets/bricks_specular.png", "specular");

    //Wall properties
    wall.ka = 0.2f;
    wall.kd = 1.0f;
    wall.ks = 1.0f;
    wall.Ns = 20.0f;

    //Add wall model to objects vector

    object.name = "wall";
    for (unsigned int i = 0; i <= wallPositions->length(); i++)
    {
        object.position = wallPositions[i];
        object.rotation = wallRotations[i];
        object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        object.angle = Maths::radians(90.0f);
        objects.push_back(object);
    }

    glm::vec3 flippedWallPositions[] = {
    //glm::vec3(-8.0f, 4.0f, 0.0f),
    glm::vec3(8.0f, 4.0f, 0.0f)
    };

    glm::vec3 flippedWallRotations[] = {
     //glm::vec3(0.0f,  0.0f,  -1.0f),
     glm::vec3(0.0f, 0.0f, 1.0f)
    };
    //Load Wall
    Model flippedWall("../assets/plane.obj");
    flippedWall.addTexture("../assets/flipbricks_diffuse.png", "diffuse");
    flippedWall.addTexture("../assets/flipbricks_normal.png", "normal");
    flippedWall.addTexture("../assets/flipbricks_specular.png", "specular");

    //Wall properties
    flippedWall.ka = 0.2f;
    flippedWall.kd = 1.0f;
    flippedWall.ks = 1.0f;
    flippedWall.Ns = 20.0f;

    //Add wall model to objects vector

    object.name = "flippedWall";
    for (unsigned int i = 0; i <= flippedWallPositions->length(); i++)
    {
        object.position = flippedWallPositions[i];
        object.rotation = flippedWallRotations[i];
        object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        object.angle = Maths::radians(90.0f);
        objects.push_back(object);
    }

    Model player("../assets/suzanne.obj");
    player.addTexture("../assets/suzanne_diffuse.png", "diffuse");
    player.addTexture("../assets/suzanne_normal.png", "normal");

    //car properties
    player.ka = 0.2f;
    player.kd = 1.0f;
    player.ks = 1.0f;
    player.Ns = 20.0f;

    object.name = "player";
    object.position = camera.eye;
    object.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    object.scale = glm::vec3(0.25f, 0.25f, 0.25f);
    object.angle = 0.0f;
    objects.push_back(object);


    Model car("../assets/police_car.obj");
    car.addTexture("../assets/white.bmp", "diffuse");
    car.addTexture("../assets/neutral_specular.png", "specular");

    //car properties
    car.ka = 0.2f;
    car.kd = 1.0f;
    car.ks = 1.0f;
    car.Ns = 20.0f;

    //Add car model to objects vector
    object.position = glm::vec3(-5.0f, -0.7f, 0.0f);
    object.scale = glm::vec3(0.25f, 0.25f, 0.25f);
    object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    object.angle = 0.0f;
    object.name = "car";
    objects.push_back(object);
    


    Model streetlamp("../assets/streetlamp.obj");
    streetlamp.addTexture("../assets/grey.bmp", "diffuse");
    streetlamp.addTexture("../assets/bricks_normal.png", "normal");
    streetlamp.addTexture("../assets/bricks_specular.png", "specular");

    //streetlamp properties
    streetlamp.ka = 0.2f;
    streetlamp.kd = 1.0f;
    streetlamp.ks = 1.0f;
    streetlamp.Ns = 20.0f;

    //Add streetlamp model to objects vector
    object.position = glm::vec3(5.0f, -1.0f, 0.0f);
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    object.angle = Maths::radians(180.0f);
    object.name = "streetlamp";
    objects.push_back(object);
    object.name = "";
    object.position = glm::vec3(5.0f, 0.0f, 0.0f);
    object.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(object);
    object.position = glm::vec3(5.0f, 1.0f, 0.0f);
    objects.push_back(object);
    object.position = glm::vec3(5.0f, 2.0f, 0.0f);
    objects.push_back(object);
    object.position = glm::vec3(5.0f, 3.0f, 0.0f);
    objects.push_back(object);

    // Create VAO, VBO, and EBO for the skybox
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::string facesCubemap[6] =
    {
        "../assets/DaylightBox_Right.png",
        "../assets/DaylightBox_Left.png",
        "../assets/DaylightBox_Top.png",
        "../assets/DaylightBox_Bottom.png",
        "../assets/DaylightBox_Front.png",
        "../assets/DaylightBox_Back.png"
    };

    // Creates the cubemap texture
    unsigned int cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Cycles through all the textures and attaches them to the cubemap object
    for (unsigned int i = 0; i < 6; i++)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }
    float switchTime = 1.0f;
    float lightMoveAmount = 0.0f;
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> random(0.0f, 1.0f);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Update timer
        float time = glfwGetTime();
        deltaTime = time - previousTime;
        previousTime = time;

        // Get inputs

        keyboardInput(window);
        mouseInput(window);

        // Clear the window
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate view and projection matrices
        camera.target = camera.eye + camera.front;
        if (camera.jumping)
        {
            camera.eye.y = camera.jumpForce * sin(3.1416f * camera.jumpTime);
            camera.jumpTime -= deltaTime;

            if (camera.jumpTime <= 0)
            {
                camera.jumping = false;
            }
        }

        //Non-Quaternion
        //camera.calculateMatrices();
         
        //Quaternion 
        camera.quaternionCamera(thirdCamera, fourthCamera);


        // Activate shader
        glUseProgram(shaderID);

        // Send light source properties to the shader
        lightSources.toShader(shaderID, camera.view);

        // Loop through objects
        for (unsigned int i = 0; i < static_cast<unsigned int>(objects.size()); i++)
        {
            // Calculate model matrix
            glm::mat4 translate = Maths::translate(objects[i].position);
            glm::mat4 scale = Maths::scale(objects[i].scale);
            glm::mat4 rotate;
            if (objects[i].name == "specialTeapot")
            { 
                if (glm::distance(camera.eye, objects[i].position) <= 3)
                {
                    objects[i].angle = Maths::radians(50.0f);
                }
                else
                {
                    objects[i].angle = 0.0f;
                }
                if (glm::distance(camera.eye, objects[i].position) <= 2)
                {
                    objects[i].position.y += 0.05f * deltaTime;
                    if (objects[i].position.y >= 4)
                    {
                        objects[i].position.y = -0.2f;
                    }
                }
 
                rotate = Maths::rotate(objects[i].angle * glfwGetTime(), objects[i].rotation);
            }
            else if (objects[i].name == "player" && thirdCamera)
            {
                rotate = Maths::rotate(-camera.yaw, glm::vec3(0.0f, 1.0f, 0.0f)) * Maths::rotate(camera.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            }  
            else if (objects[i].name == "player" && fourthCamera)
            {
                rotate = Maths::rotate(-playerYaw, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else
            {
                rotate = Maths::rotate(objects[i].angle, objects[i].rotation);
            }

            if (objects[i].name == "streetlamp")
            {
                 if (glm::distance(camera.eye, objects[i].position) <= 5)
                 {
                     switchTime -= deltaTime;

                     if (switchTime <= 0)
                     {
                         lightSources.get("streetLight").colour = glm::vec3(random(gen), random(gen), random(gen));
                         switchTime = 1.0f;
                     }

                     lightSources.get("streetLight").direction = glm::vec3(lightMoveAmount, -1.0f, 0.0f);
                     if (lightMoveAmount < -2)
                     {
                         lightMoveAmount = 0.0f;
                     }
                     else
                     {
                         lightMoveAmount -= 0.1f * deltaTime;
                     }
                 }
            }

            if (objects[i].name == "car")
            {
                if (glm::distance(camera.eye, objects[i].position) <= 5)
                {
                    switchTime -= deltaTime;

                    if (switchTime <= 0)
                    {
                        glm::vec3 colour = lightSources.get("redLight").colour;
                        lightSources.get("redLight").colour = lightSources.get("blueLight").colour;
                        lightSources.get("blueLight").colour = colour;
                        switchTime = 1.0f;
                    }
                }
            }

            glm::mat4 model = translate * rotate * scale;

            // Send the MVP and MV matrices to the vertex shader
            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

            // Draw the model
            if (objects[i].name == "teapot")
                teapot.draw(shaderID);

            if (objects[i].name == "specialTeapot")
                teapot.draw(shaderID);

            if (objects[i].name == "floor")
                floor.draw(shaderID);

            if (objects[i].name == "wall")
                wall.draw(shaderID);

            if (objects[i].name == "flippedWall")
                flippedWall.draw(shaderID);

            if(objects[i].name == "car")
                car.draw(shaderID);

            if (objects[i].name == "streetlamp")
                streetlamp.draw(shaderID);

            if (objects[i].name == "crate")
            {
                cube.draw(shaderID);
            }
            
            if (objects[i].name == "player" && (thirdCamera || fourthCamera))
            {
                objects[i].position = camera.eye - glm::vec3(0.0f, 0.5f, 0.0f);
                player.draw(shaderID);
            }

            if (objects[i].name != "player")
            checkCollision(camera, objects[i]);
        }
        // Draw light sources
        lightSources.draw(lightShaderID, camera.view, camera.projection, sphere);

       
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShaderID);
  
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
        // The last row and column affect the translation of the skybox (which we don't want to affect)
        view = glm::mat4(glm::mat3(camera.view));
        projection = camera.calculatePerspective();
        glUniformMatrix4fv(glGetUniformLocation(skyboxShaderID, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShaderID, "projection"), 1, GL_FALSE, &projection[0][0]);
        
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    teapot.deleteBuffers();
    glDeleteProgram(shaderID);
    glDeleteProgram(skyboxShaderID);
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}

void keyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Move the camera using WSAD keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && fourthCamera)
    {
        camera.eye += 5.0f * deltaTime * glm::vec3(cos(playerYaw - 1.5), 0, sin(playerYaw - 1.5));
    }
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && fourthCamera)
    {
        camera.eye -= 5.0f * deltaTime * glm::vec3(cos(playerYaw - 1.5), 0, sin(playerYaw - 1.5));
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && fourthCamera)
    {
        playerYaw -= 2.0f *deltaTime;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.right;
        

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && fourthCamera)
    {
        playerYaw += 2.0f * deltaTime;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.right;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && camera.jumping == false)
    {
        camera.jumpTime = 1.0f;
        camera.jumping = true;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        thirdCamera = false;
        fourthCamera = false;
    }
        

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        thirdCamera = true;
        fourthCamera = false;
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        thirdCamera = false;
        fourthCamera = true;
    }
        

}

void mouseInput(GLFWwindow* window)
{
    // Get mouse cursor position and reset to centre
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Update yaw and pitch angles
    camera.yaw += 0.005f * float(xPos - 1024 / 2);
    camera.pitch += 0.005f * float(768 / 2 - yPos);

    //camera.pitch = Maths::clamp(camera.pitch, Maths::radians(-89), Maths::radians(89));
    // Calculate camera vectors from the yaw and pitch angles
    camera.calculateCameraVectors();

}

void checkCollision(Camera& camera, Object& obj1)
{
    float offset = 0.1;
    float distance = 0.5f;
    if (obj1.name == "car")
        distance = 2.0f;
    if (glm::distance(camera.eye, obj1.position) < distance)
    {
        if (camera.eye.x < obj1.position.x)
        {
            camera.eye.x -= offset;
        }
        if (camera.eye.x > obj1.position.x)
        {
            camera.eye.x += offset;
        }
        if (camera.eye.z < obj1.position.z)
        {
            camera.eye.z -= offset;
        }
        if (camera.eye.z > obj1.position.z)
        {
            camera.eye.z += offset;
        }
    };


    if (camera.eye.x >= 7.7f )
    {
        camera.eye.x -= offset;
    }
    if (camera.eye.x <= -10.0f)
    {
        camera.eye.x += offset;
    }
    if (camera.eye.z >= 10.0f)
    {             
        camera.eye.z -= offset;
    }              
    if (camera.eye.z <= -7.7f)
    {              
        camera.eye.z += offset;
    }

};

