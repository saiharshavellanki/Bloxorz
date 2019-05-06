#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <FTGL/ftgl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;


struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID, fontProgramID, textureProgramID;;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

void display_end(char *str){
	Matrices.model = glm::mat4(1.0f);
	glm::vec3 fontColor = glm::vec3(0,0,0);
	glm::mat4 translateText = glm::translate(glm::vec3(400,400,0));
	float fontScaleValue = 40 ;
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	glm::mat4 MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(str);
	translateText = glm::translate(glm::vec3(0,-1,0));
	Matrices.model *= (translateText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("Press R to restart the level, Q to quit");
	translateText = glm::translate(glm::vec3(0,-1,0));
	Matrices.model *= (translateText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("Press B to go back to previous level");
}

void display_string(float x,float y,char *str,float fontScaleValue){
	glm::vec3 fontColor = glm::vec3(0,0,0);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(x,y,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	glm::mat4 MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(str);

}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;


typedef struct h
{
  float l,w,h,x1,y1,z1,x2,y2,z2,goal_x,goal_y,translatex,translatey,angley,anglex;
  glm::mat4 rotationmatrix[2];
}blocks;
blocks block;
int a[100][100],board_length,board_width,level=1,levelcount=0,gamestart=0,view=0;
int active_circle=0,active_rectangle=0,active_into=0,breakblock=0,presentblock=-1,leftmouse=0;
float right_angle,left_angle,up_angle,down_angle,zshift=4,namefall=0;
double start_time,current_time,game_over_time,viewtime,mouse_x,mouse_y,lgx,lgy;
void func(int a,float b,char *c,int x,int y);
void func1(int a,float b,char c[],int x,int y,float z);
//tiles block;
int num_of_tiles=00,rightmove=0,checkarrow=0,lives=3,presentlives=0,moves[]={0,8,18,28,11},present_moves[]={0,14,24,34,17};
int checkstate();
void changestateto(int state);
void addcoordinates(float x,float y,float x1,float y1);
void checkupordown(int p);
void rightkeypressed(float h1,float h2,float h3,float h4,float h5,float h6,float h7,float h8);
void upkeypressed(float h1,float h2,float h3,float h4,float h5,float h6,float h7,float h8);

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
      if(key==GLFW_KEY_RIGHT || key==GLFW_KEY_LEFT || key==GLFW_KEY_UP || key==GLFW_KEY_DOWN)
      checkarrow=0;
    }
    else if (action == GLFW_PRESS) {
      if(key==GLFW_KEY_RIGHT)
      {
				  system("mpg123 -vC sounds/1.mp3 &");
					present_moves[level]--;
          checkarrow=1;
          if(breakblock==0)
          {
          right_angle=45;
          rightkeypressed(2,4,-2,2,2,4,2,2);
          }
          else
          {
            if(presentblock!=-1)
            {
            right_angle=90;
            if(presentblock==0)
            block.x1+=2;
            else
            block.x2+=2;
            }
          }
      }
      else if(key==GLFW_KEY_LEFT)
      {
				system("mpg123 -vC sounds/1.mp3 &");
				present_moves[level]--;
        checkarrow=1;
        if(breakblock==0)
        {
        left_angle=45;
        rightkeypressed(-4,-2,-2,-2,-2,-2,-4,2);
        }
        else
        {
          if(presentblock!=-1)
          {
          left_angle=90;
          if(presentblock==0)
          block.x1-=2;
          else
          block.x2-=2;
          }
        }
      }
      else if(key==GLFW_KEY_UP)
      {
				system("mpg123 -vC sounds/1.mp3 &");
				present_moves[level]--;
        checkarrow=1;
        if(breakblock==0)
        {
        up_angle=45;
        upkeypressed(2,4,-2,4,2,2,2,2);
        }
        else
        {
          if(presentblock!=-1)
          {
          up_angle=90;
          if(presentblock==0)
          block.y1+=2;
          else
          block.y2+=2;
          }
        }
      }
      else if(key==GLFW_KEY_DOWN)
      {
			system("mpg123 -vC sounds/1.mp3 &");
			present_moves[level]--;
      checkarrow=1;
      if(breakblock==0)
      {
      down_angle=45;
      upkeypressed(-4,-2,-2,-2,-4,2,-2,-2);
      }
      else
      {
        if(presentblock!=-1)
        {
        down_angle=90;
        if(presentblock==0)
        block.y1-=2;
        else
        block.y2-=2;
        }
      }
      }
      else if(key==GLFW_KEY_B)
      {
        presentblock=1-presentblock;
      }
			else if(key==GLFW_KEY_ENTER)
			{
				gamestart=1;
			}
			else if(key==GLFW_KEY_Q)
			{
			  char level_str[30];
			  sprintf(level_str,"CONGRATS!! YOU COMPLETED %d LEVELS	",level-1);
			 	func(100,3,level_str,-15,0);
				gamestart=2;
			  //exit(0);
		  }
			else if(key==GLFW_KEY_V)
			{
				view=(view+1)%6;
				viewtime=glfwGetTime();
			}
//      cout<<block.x1<<" "<<block.y1<<" "<<block.x2<<" "<<block.y2<<endl;
      if(checkarrow==1)
      {
      if((int)block.x1==-12 && (int)block.y1==-2)
      {
        active_circle=1-active_circle;
      }
      else if((int)block.x2==-12 && (int)block.y2==-2)
      {
        active_circle=1-active_circle;
      }
      if((int)block.x1==0 && block.y1==0 && block.x2==0 && block.y2==0)
      active_into=1-active_into;
      //cout<<active_circle<<endl;
      }
     }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	if(action==GLFW_PRESS)
 {
	if(button==GLFW_MOUSE_BUTTON_LEFT)
	{
		leftmouse=1;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
		cout<<mouse_x<<" "<<mouse_y<<endl;
		if(fabs(736-mouse_x)+fabs(816-mouse_y)<50)
		{
			present_moves[level]--;
			checkarrow=1;
			if(breakblock==0)
			{
			left_angle=45;
			rightkeypressed(-4,-2,-2,-2,-2,-2,-4,2);
			}
			else
			{
				if(presentblock!=-1)
				{
				left_angle=90;
				if(presentblock==0)
				block.x1-=2;
				else
				block.x2-=2;
				}
			}
		}
		else if(fabs(798-mouse_x)+fabs(mouse_y-744)<50)
		{
			present_moves[level]--;
			checkarrow=1;
			if(breakblock==0)
			{
			up_angle=45;
			upkeypressed(2,4,-2,4,2,2,2,2);
			}
			else
			{
				if(presentblock!=-1)
				{
				up_angle=90;
				if(presentblock==0)
				block.y1+=2;
				else
				block.y2+=2;
				}
			}
		}
		else if(fabs(mouse_x-868)+fabs(mouse_y-800)<50)
		{
			present_moves[level]--;
			checkarrow=1;
			if(breakblock==0)
			{
			right_angle=45;
			rightkeypressed(2,4,-2,2,2,4,2,2);
			}
			else
			{
				if(presentblock!=-1)
				{
				right_angle=90;
				if(presentblock==0)
				block.x1+=2;
				else
				block.x2+=2;
				}
			}
		}
		else
		{
			present_moves[level]--;
      checkarrow=1;
      if(breakblock==0)
      {
      down_angle=45;
      upkeypressed(-4,-2,-2,-2,-4,2,-2,-2);
      }
      else
      {
        if(presentblock!=-1)
        {
        down_angle=90;
        if(presentblock==0)
        block.y1-=2;
        else
        block.y2-=2;
        }
      }
		}
		//
		//mouse_x=(2*mouse_x-1000)/50;
		// mouse_y=(2*mouse_y-1000)/50;
		//cout<<mouse_x<<" "<<mouse_y<<endl;
	}
 }
 else if(action==GLFW_RELEASE)
 {
	 if(button==GLFW_MOUSE_BUTTON_LEFT)
	 {
		 leftmouse=0;
	 }
 }
 if(view==3 && leftmouse==1)
        glfwGetCursorPos(window,&lgx,&lgy);
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
		// if(view==4)
    //  Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
		//  else
    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f,  0.1f, 500.0f);
}

VAO *tile,*brick[2],*circle,*rect[2],*triangle,*triangle1,*triangle2,*triangle3;
// Creates the triangle object used in this sample code

void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	triangle1 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	triangle2 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
	triangle3 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createtile(float length,float width,float height,int block,float c1,float c2,float c3,float c4,float c5,float c6,int num)
{
  GLfloat vertex_buffer_data[]=
  {
    //bottom
    -length,-width,-height,
    -length,-width,height,
    length,-width,height,

    length,-width,height,
    length,-width,-height,
    -length,-width,-height,
    //top
    length,width,height,
    -length,width,height,
    -length,width,-height,

    -length,width,-height,
    length,width,-height,
    length,width,height,
    //left
    -length,-width,height,
    -length,-width,-height,
    -length,width,-height,

    -length,width,-height,
    -length,width,height,
    -length,-width,height,
    //right
    length,width,height,
    length,width,-height,
    length,-width,-height,

    length,-width,-height,
    length,-width,height,
    length,width,height,
    //front
    -length,-width,height,
    length,-width,height,
    length,width,height,

    length,width,height,
    -length,width,height,
    -length,-width,height,
    //back
    -length,-width,-height,
    length,-width,-height,
    length,width,-height,

    length,width,-height,
    -length,width,-height,
    -length,-width,-height
  };
  GLfloat color_buffer_data[120];
  int i;
  int left[100]={0,1,5,12,13,17,24,29,30,35,7,8,9,14,15,16,28,34};
  //if(block==0)
  //{
  for(i=0;i<36;i++)
  {
  // cout<<vertex_buffer_data[3*i]<<" "<<vertex_buffer_data[3*i+1]<<endl;
  if(vertex_buffer_data[3*i]==-1 && (vertex_buffer_data[3*i+1]==-1))
  {
   color_buffer_data[3*i]=c1;
   color_buffer_data[3*i+1]=c2;
   color_buffer_data[3*i+2]=c3;
  }
  else
  {
    color_buffer_data[3*i]=c4;
    color_buffer_data[3*i+1]=c5;
    color_buffer_data[3*i+2]=c6;
  }
  if(num>=0 && vertex_buffer_data[3*i]==-1 && vertex_buffer_data[3*i+1]==1)
  {
     color_buffer_data[3*i]=c1;
     color_buffer_data[3*i+1]=c2;
     color_buffer_data[3*i+2]=c3;
  }
  }
  if(block==0)
  tile = create3DObject(GL_TRIANGLES,36, vertex_buffer_data, color_buffer_data, GL_FILL);
  else
  brick[num]=create3DObject(GL_TRIANGLES,36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createcircle(float r,float R,float G,float B,float x,float y)
{
  GLfloat vertex_buffer_data[5*9*360],color_buffer_data[5*9*360];
  int i;
  for(i=0;i<5*360;i++)
  {
    vertex_buffer_data[9*i]=x+r*cos((float)i/5*M_PI/180.0);
    vertex_buffer_data[9*i+1]=y+r*sin((float)i/5*M_PI/180.0);
    vertex_buffer_data[9*i+2]=0;
    vertex_buffer_data[9*i+3]=x+r*cos(((float)i/5+1)*M_PI/180.0);
    vertex_buffer_data[9*i+4]=y+r*sin(((float)i/5+1)*M_PI/180.0);
    vertex_buffer_data[9*i+5]=0;
    vertex_buffer_data[9*i+6]=x;
    vertex_buffer_data[9*i+7]=y;
    vertex_buffer_data[9*i+8]=0;
  }
  for(i=0;i<5*9*360;i+=3)
  {
    color_buffer_data[i]=R;
    color_buffer_data[i+1]=G;
    color_buffer_data[i+2]=B;
  }
 circle = create3DObject(GL_TRIANGLES, 5*1080, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float R,float G,float B,int i)
{
  // GL3 accepts only Triangles. Quads are not supported
  GLfloat vertex_buffer_data [] = {
    x1,y1,0.2, // vertex 1
    x2,y2,0.2, // vertex 2
    x3,y3,0.2, // vertex 3

    x3,y3,0.2, // vertex 3
    x4,y4,0.2, // vertex 4
    x1,y1,0.2  // vertex 1
  };

    GLfloat color_buffer_data [] = {
      R,G,B, // color 1
      R,G,B, // color 2
      R,G,B, // color 3

      R,G,B, // color 3
      R,G,B, // color 4
      R,G,B  // color 1
    };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rect[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  }
float camera_rotation_angle = 90;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void rightkeypressed(float h1,float h2,float h3,float h4,float h5,float h6,float h7,float h8)
{
  if(block.x1==block.x2 && block.y1==block.y2)
  {
    block.x1+=h1;
    block.x2+=h2;
    block.z2+=h3;
  }
  else
  {
    if(block.x1==block.x2)
    {
      block.x1+=h4;
      block.x2+=h5;
    }
    else
    {
      block.x1+=h6;
      block.x2+=h7;
      block.z2+=h8;
    }
  }
  //cout<<(4-block.y1)/2<<" "<<(12+block.x1)/2<<" "<<(4-block.y2)/2<<" "<<(12+block.x2)/2<<endl;
}
void upkeypressed(float h1,float h2,float h3,float h4,float h5,float h6,float h7,float h8)
{
  if(block.x1==block.x2 && block.y1==block.y2)
  {
    block.y1+=h1;
    block.y2+=h2;
    block.z2+=h3;
  }
  else
  {
    if(block.x1==block.x2)
    {
      block.y1+=h4;
      block.y2+=h5;
      block.z2+=h6;
    }
    else
    {
      block.y1+=h7;
      block.y2+=h8;
    }
  }
//  cout<<(4-block.y1)/2<<" "<<(12+block.x1)/2<<" "<<(4-block.y2)/2<<" "<<(12+block.x2)/2<<endl;
}
void drag(GLFWwindow* window)
{
  double lx1;
  double ly1;
  glfwGetCursorPos(window,&lx1,&ly1);
  if(view == 3 && leftmouse == 1)
  {
    camera_rotation_angle -= (lx1-lgx)/800;
  }
}
glm::vec3 getRGBfromHue (int hue)
{
  float intp;
  float fracp = modff(hue/60.0, &intp);
  float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

  if (hue < 60)
    return glm::vec3(1,x,0);
  else if (hue < 120)
    return glm::vec3(x,1,0);
  else if (hue < 180)
    return glm::vec3(0,1,x);
  else if (hue < 240)
    return glm::vec3(0,x,1);
  else if (hue < 300)
    return glm::vec3(x,0,1);
  else
    return glm::vec3(1,0,x);

}
int count=0,count1=0;
int draw (int presentlevel)
{
	presentlives=lives;
	if(present_moves[level]<0)
	{
		lives--;
	}
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  // glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // // Target - Where is the camera looking at.  Don't change unless you are sure!!
  // glm::vec3 target (0,0, 0);
  // // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  // glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
	  if(view==0)
    /*tower view*/Matrices.view = glm::lookAt(glm::vec3(-20,-20,15), glm::vec3(0,0,0), glm::vec3(1,1,8/3)); // Fixed camera for 2D (ortho) in XY plane
    else if(view==1)
    /*topview*/Matrices.view = glm::lookAt(glm::vec3(0,0,15), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    else if(view==2)
    /*backview*/Matrices.view =glm::lookAt(glm::vec3(20,20,15), glm::vec3(0,0,0), glm::vec3(1,1,8/3));
    else if(view==3)
    /*helicopter view*/Matrices.view =glm::lookAt(glm::vec3(-20*cos(camera_rotation_angle*M_PI/180),-20*sin(camera_rotation_angle*M_PI/180),15), glm::vec3(0,0,0), glm::vec3(1,1,8/3));
		else if(view==4)
		{
			/*block view*/
			Matrices.view =glm::lookAt(glm::vec3(block.x2+1,block.y2,block.z2+1), glm::vec3(block.x2+3,block.y2,block.z2), glm::vec3(0,0,1));
	  }
		else
		{
			/*follow cam view*/
			Matrices.view =glm::lookAt(glm::vec3(block.x2-6,block.y2,block.z2+5), glm::vec3(block.x2+3,block.y2,block.z2+2), glm::vec3(0,0,1));
		}
  //Matrices.view = glm::lookAt(glm::vec3(-20,-20,15), glm::vec3(0,0,0), glm::vec3(1,1,8/3)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model
//  glUseProgram(fontProgramID);
  // Load identity to model matrix
	if(gamestart==1)
	{
  int i,j;
  //cout<<block.xlength<<" "<<block.ylength<<endl;
  if(zshift>0)
  {
    zshift-=0.2;
  }
  for(i=0;i< 20;i++)
  {
    for(j=0;j< 20;j++)
    {
  if(a[i][j]!=0)
  {
  if(level==4 && i==5 && j==5)
  {
  createtile(1,1,0.2,0,0,0,0,0,0,0,-1);
  }
  else if(a[i][j]==1)
  createtile(1,1,0.2,0,0.87,0.87,0.87,0.627,0.627,0.627,-1);
  else
  createtile(1,1,0.2,0,1,0.5,0,1,0.7,0.4,-1);
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatetile = glm::translate (glm::vec3(2*j-(int)board_length-2,-2*i+(int)board_width-2,-zshift));        // glTranslatef
  glm::mat4 rotatetile = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatetile * rotatetile);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tile);
  }
  }
 }
 if(right_angle>0 && right_angle<=90)
 {
//cout<<right_angle<<endl;
 if(breakblock==0)
 {
 block.rotationmatrix[0]=glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[0];
 block.rotationmatrix[1]=glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[1];
 }
 if(presentblock==0)
 {
   block.rotationmatrix[0]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[0];
 }
 if(presentblock==1)
 block.rotationmatrix[1]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[1];
 right_angle+=45;
 }
 else
 right_angle=0;
 if(left_angle>0 && left_angle<=90)
 {
   if(breakblock==0)
   {
   block.rotationmatrix[0]=glm::rotate((float)(-45*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[0];
   block.rotationmatrix[1]=glm::rotate((float)(-45*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[1];
   }
   if(presentblock==0)
   {
     block.rotationmatrix[0]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[0];
   }
   if(presentblock==1)
   block.rotationmatrix[1]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[1];
   left_angle+=45;
 }
 else
 left_angle=0;
 if(up_angle>0 && up_angle<=90)
 {
  //cout<<up_angle<<endl;
  if(breakblock==0)
  {
  block.rotationmatrix[0]=glm::rotate((float)(-45*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[0];
  block.rotationmatrix[1]=glm::rotate((float)(-45*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[1];
  }
  if(presentblock==0)
  block.rotationmatrix[0]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[0];
  if(presentblock==1)
  block.rotationmatrix[1]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[1];
  up_angle+=45;
 }
 else
 up_angle=0;
 if(down_angle>0 && down_angle<=90)
 {
   if(breakblock==0)
   {
   block.rotationmatrix[0]=glm::rotate((float)(45*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[0];
   block.rotationmatrix[1]=glm::rotate((float)(45*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[1];
   }
   if(presentblock==0)
   block.rotationmatrix[0]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[0];
   if(presentblock==1)
   block.rotationmatrix[1]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[1];
   down_angle+=45;
 }
 else
 down_angle=0;
 int x1,y1,x2,y2,z1,z2;
 x1=(int)(board_width-2-block.y1)/2;
 y1=(int)(block.x1+board_length+2)/2;
 x2=(int)(board_width-2-block.y2)/2;
 y2=(int)(block.x2+board_length+2)/2;
 //cout<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
if(a[x1][y1]==2 && x1==x2 && y1==y2)
{
  a[x1][y1]=0;
}
if(block.z1<=-20 || block.z2<=-20)
{
	char level_str[30];
	sprintf(level_str,"CONGRATS!! YOU REACHED LEVEL %d",level);
	func(100,3,level_str,-15,0);
	gamestart=2;
  //exit(0);
}
if(count ==0 && a[x1][y1]==0 && a[x2][y2]==0)
{
	count=1;
  if(x1==x2 && y1==y2 && x1==block.goal_x && y1==block.goal_y)
  {
  presentlevel++;
  block.z1-=1;
  block.z2-=1;
	if(presentlevel==5)
	{
		gamestart=2;
	}
  }
  else
  {
	lives--;
	present_moves[level]=moves[level]+6;
  block.z1-=1;
  block.z2-=1;
  }
}
else if(count==0 && (a[x1][y1]==0 || a[x2][y2]==0))
{
  count=1;
  if(a[x2][y2]==0 && x2==block.goal_x && y2==block.goal_y)
  {
  }
  else if(a[x2][y2]==0)
  {
    if(x1==block.goal_x && y1==block.goal_y)
    {
    }
    else if(x1-1==x2)
    {
      block.rotationmatrix[0]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[0];
      block.rotationmatrix[1]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[1];
      block.y1+=2;
      block.z1-=2;
			lives--;
			present_moves[level]=moves[level]+6;
    }
    else
    {
      block.rotationmatrix[0]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[0];
      block.rotationmatrix[1]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[1];
      block.x1+=2;
      block.z1-=2;
			lives--;
			present_moves[level]=moves[level]+6;
    }
  }
  else if(a[x1][y1]==0 && x1==block.goal_x && y1==block.goal_y)
  {
  }
  else if(a[x1][y1]==0)
  {
    if(x2==block.goal_x && y2==block.goal_y)
    {
    }
    else if(y1==y2-1)
    {
        block.rotationmatrix[0]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[0];
        block.rotationmatrix[1]=glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[1];
        block.z1-=2;
        block.x2-=2;
				lives--;
				present_moves[level]=moves[level]+6;
    }
    else
    {
      block.rotationmatrix[0]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[0];
      block.rotationmatrix[1]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0)) * block.rotationmatrix[1];
      block.z1-=2;
      block.y2-=2;
			lives--;
			present_moves[level]=moves[level]+6;
    }
  }
}
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translatel = glm::translate(glm::vec3(block.translatex,block.translatey,0));
    glm::mat4 translateblock = glm::translate (glm::vec3(block.x1,block.y1,block.z1+zshift));        // glTranslatef
    // glm::mat4 rotateblockx = glm::rotate((float)(block.anglex*M_PI/180.0f), glm::vec3(1,0,0));
    // glm::mat4 rotateblocky = glm::rotate((float)(block.angley*M_PI/180.0f), glm::vec3(0,1,0));
    // block.rotationmatrix=rotateblocky*rotateblockx*block.rotationmatrix;
    glm::mat4 translateblock2 = glm::translate (glm::vec3(-1*block.translatex,-1*block.translatey,0));
    Matrices.model *= (translateblock*translateblock2*block.rotationmatrix[0]*translatel);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(brick[0]);

    Matrices.model = glm::mat4(1.0f);
    translatel = glm::translate(glm::vec3(block.translatex,block.translatey,0));
    translateblock = glm::translate (glm::vec3(block.x2,block.y2,block.z2+zshift));
    translateblock2 = glm::translate (glm::vec3(-1*block.translatex,-1*block.translatey,0));
    Matrices.model *= (translateblock*translateblock2*block.rotationmatrix[1]*translatel);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(brick[1]);
    if(level==2)
    {
      a[5][5]=active_circle;
      a[5][6]=active_circle;
      a[5][11]=active_into;
      a[5][12]=active_into;
      createcircle(1,0,0,0,-12,-2);
      Matrices.model = glm::mat4(1.0f);
      translatel = glm::translate(glm::vec3(0,0,0.2+zshift));
      Matrices.model *= (translatel);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

      // draw3DObject draws the VAO given to it using current MVP matrix
      draw3DObject(circle);

      createRectangle(-1,0.8,-0.8,1,1,-0.8,0.8,-1,0,0,0,0);
      createRectangle(-0.9,-1,1,0.9,0.9,1,-1,-0.9,0,0,0,1);
      for(i=0;i<2;i++)
      {
      Matrices.model = glm::mat4(1.0f);
      translatel = glm::translate(glm::vec3(0,0,0.2+zshift));
      Matrices.model *= (translatel);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(rect[i]);
      }
    }
    if(level==4)
    {
      if(x1==x2 && y1==y2 && x1==5 && y1==5)
      {
        breakblock++;
      }
      int a,b;
      if(breakblock==1)
      {
        presentblock=0;
        block.x1=4;
        block.y1=3;
        block.z1=1.2;
        block.x2=4;
        block.y2=-9;
        block.z2=1.2;
        block.rotationmatrix[1]=glm::mat4(1.0f);
        block.rotationmatrix[0]=glm::mat4(1.0f);
        //block.rotationmatrix[1]=block.rotationmatrix[0];
        breakblock++;
      }
      if(breakblock>1)
      {
        float t;
        if(fabs(x1-x2)==1 && y1==y2)
        {
          if(block.y1>block.y2)
          {
          t=block.y1;
          block.y1=block.y2;
          block.y2=t;
          }
          block.rotationmatrix[0]=glm::mat4(1.0f);
          block.rotationmatrix[1]=block.rotationmatrix[0];
          //
          breakblock=0;
          presentblock=-1;
        }
        else if(fabs(y1-y2)==1 && x1==x2)
        {
          block.rotationmatrix[0]=glm::mat4(1.0f);
          block.rotationmatrix[0]=glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)) * block.rotationmatrix[0];
          block.rotationmatrix[1]=block.rotationmatrix[0];
          if(block.x2<block.x1)
          {
           t=block.x1;
           block.x1=block.x2;
           block.x2=t;
          }
          breakblock=0;
          presentblock=-1;
        }
      }
    }
	int fontScale=20;
  float fontScaleValue = 2;
	glm::vec3 fontColor = getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);


 	char level_str[30];
	if(level<=4)
	sprintf(level_str,"LEVEL: %d",level);
	else
  sprintf(level_str,"LEVEL: %d",4);
	func(20,2,level_str,10,17);
	if(lives>0)
	sprintf(level_str,"LIVES: %d",lives);
	else
	sprintf(level_str,"LIVES: %d",0);
	func(20,2,level_str,10,15);
	if(presentlevel<5)
	{
	sprintf(level_str,"MOVES LEFT:%d",present_moves[level]);
	func(20,2,level_str,-19,15);
  }
  current_time=glfwGetTime();
	sprintf(level_str,"TIME:%.0lf",current_time-start_time);
	func(20,2,level_str,-2,17);
	if(current_time-viewtime<1)
	{
		if(view==0)
		strcpy(level_str,"TOWER VIEW");
		else if(view==1)
		strcpy(level_str,"TOP VIEW");
		else if(view==2)
		strcpy(level_str,"BACK VIEW");
		else if(view==3)
		strcpy(level_str,"HELICOPTER VIEW");
		else if(view==4)
		strcpy(level_str,"BLOCK VIEW");
		else
		strcpy(level_str,"FOLLOW CAM VIEW");
		func(100,3,level_str,-10,-10);
	}
	if(lives==0)
	{
		char level_str[30];
		sprintf(level_str,"CONGRATS!! YOU COMPLETED %d LEVELS",level-1);
		func(100,3,level_str,-15,0);
		gamestart=2;
	  //exit(0);
   }
	 if(view==4 || view==5)
 	 Matrices.projection = glm::perspective (90.0f, (GLfloat)1000 / (GLfloat) 1000, 0.1f, 500.0f);
	 else
	 Matrices.projection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f,  0.1f, 500.0f);
//down
	   Matrices.model = glm::mat4(1.0f);
glm::mat4 translateTriangle11 = glm::translate (glm::vec3(9.5,-15 ,0 )); // glTranslatef
                glm::mat4 rotateTriangle11 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
                glm::mat4 scaleTriangle11 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
                // rotate about vector (1,0,0)
                glm::mat4 triangleTransform11 = translateTriangle11* rotateTriangle11*scaleTriangle11;
                Matrices.model *= triangleTransform11;
                //MVP = VP * Matrices.model; // MVP = p * V * M
    MVP = Matrices.projection * Matrices.view * Matrices.model;

                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

                draw3DObject(triangle);

                Matrices.model = glm::mat4(1.0f);
//left
glm::mat4 translateTriangle12 = glm::translate (glm::vec3(10,-9 ,0 )); // glTranslatef
                glm::mat4 rotateTriangle12 = glm::rotate((float)(270*M_PI/180.0f), glm::vec3(0,0,1));
                glm::mat4 scaleTriangle12 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
                // rotate about vector (1,0,0)
                glm::mat4 triangleTransform12 = translateTriangle12* rotateTriangle12*scaleTriangle12;
                Matrices.model *= triangleTransform12;
                //MVP = VP * Matrices.model; // MVP = p * V * M
    MVP = Matrices.projection * Matrices.view * Matrices.model;

                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

                draw3DObject(triangle1);

                Matrices.model = glm::mat4(1.0f);
//up
glm::mat4 translateTriangle13 = glm::translate (glm::vec3(15,-10 ,0 )); // glTranslatef
                glm::mat4 rotateTriangle13 = glm::rotate((float)(180*M_PI/180.0f), glm::vec3(0,0,1));
                glm::mat4 scaleTriangle13 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
                // rotate about vector (1,0,0)
                glm::mat4 triangleTransform13 = translateTriangle13* rotateTriangle13*scaleTriangle13;
                Matrices.model *= triangleTransform13;
                //MVP = VP * Matrices.model; // MVP = p * V * M
    MVP = Matrices.projection * Matrices.view * Matrices.model;

                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

                draw3DObject(triangle2);

                Matrices.model = glm::mat4(1.0f);
//right
glm::mat4 translateTriangle14 = glm::translate (glm::vec3(15,-15 ,0 )); // glTranslatef
                glm::mat4 rotateTriangle14 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
                glm::mat4 scaleTriangle14 = glm::scale (glm::vec3(2, 2, 1)); // glTranslatef
                // rotate about vector (1,0,0)
                glm::mat4 triangleTransform14 = translateTriangle14* rotateTriangle14*scaleTriangle14;
                Matrices.model *= triangleTransform14;
                //MVP = VP * Matrices.model; // MVP = p * V * M
    MVP = Matrices.projection * Matrices.view * Matrices.model;

                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

                draw3DObject(triangle3);

  }
	else if(gamestart==0)
	{
		char c[30];
		strcpy(c,"BLOXORZ");
		func(100,7,c,-6,8);
		strcpy(c,"INSTRUCTIONS");
		func(100,5,c,-14,2);
		strcpy(c,"1) USE ARROW KEYS TO MOVE BLOCK");
		func(100,3,c,-14,-1);
	 	strcpy(c,"2) THERE ARE 4 LEVELS");
	  func(100,3,c,-14,-3);
	 	strcpy(c,"3) EACH LEVEL HAS 3 LIVES");
	  func(100,3,c,-14,-5);
	 	strcpy(c,"4) PRESS V TO CHANGE VIEW");
	  func(100,3,c,-14,-7);
	 	strcpy(c,"5) PRESS B TO CHANGE BLOCK IN LEVEL 4");
	  func(100,3,c,-14,-9);
	 	strcpy(c,"6) PRESS ENTER TO START THE GAME");
	  func(100,3,c,-14,-11);
		strcpy(c,"7) PRESS Q TO EXIT THE GAME");
	  func(100,3,c,-14,-13);
	}
	else
	{
		char level_str[30];
		sprintf(level_str,"CONGRATS!! YOU COMPLETED %d LEVELS",level-1);
	  func(100,3,level_str,-15,0);
	}
  return presentlevel;
}
void func(int a,float b,char c[],int x,int y)
{
	glm::mat4 MVP;
	int fontScale=a;
	float fontScaleValue =b;
	glm::vec3 fontColor = getRGBfromHue(fontScale);
	glUseProgram(fontProgramID);
	char level_str[30];
	strcpy(level_str,c);
	//strcpy(level_str,"BLOXORZ");
	//sprintf(level_str,"MOVES LEFT:%d",);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(x,y,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(level_str);
}
/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Blocks", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	// Create and compile our GLSL program from the shaders
  createtile(1,1,1,1,0.4,0.2,0,1,0.7,0.4,0);
  createtile(1,1,1,1,0.4,0.2,0,1,0.7,0.4,1);
	createTriangle();
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (1.0f, 1.0f,1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	const char* fontfile = "monaco.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
GL3Font.font->CharMap(ft_encoding_unicode);



    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
void initialiselevel()
{
	count=0;
  //cout<<level<<endl;
  int i,j;
  for(i=0;i<100;i++)
  {
    for(j=0;j<100;j++)
    {
      a[i][j]=0;
    }
  }

  block.rotationmatrix[0]= glm::mat4(1.0f);
  block.rotationmatrix[1]= glm::mat4(1.0f);
  zshift=4;
  left_angle=0;
  right_angle=0;
  up_angle=0;
  down_angle=0;
  block.l=1;
  block.w=1;
  block.h=1;
  if(level==1)
  {
  num_of_tiles=35;
  for(j=1;j<=3;j++)
  a[1][j]=1;
  for(j=1;j<=6;j++)
  a[2][j]=1;
  for(j=1;j<=9;j++)
  a[3][j]=1;
  for(j=2;j<=10;j++)
  a[4][j]=1;
  for(j=6;j<=10;j++)
  {
  if(j!=8)
  a[5][j]=1;
  }
  for(j=7;j<=9;j++)
  a[6][j]=1;
  block.x1=-8;
  block.y1=0;
  block.z1=1.2;
  block.x2=-8;
  block.y2=0;
  block.z2=3.2;
  board_length=10;
  board_width=6;
  block.goal_x=5;
  block.goal_y=8;
  block.anglex=0;
  block.angley=0;
  block.translatex=0;
  block.translatey=0;
}
if(level==2)
{
  for(i=2;i<=6;i++)
  {
    for(j=1;j<=4;j++)
    {
      a[i][j]=1;
    }
  }
  // a[5][5]=1;
  // a[5][6]=1;
  for(i=1;i<=6;i++)
  {
    for(j=7;j<=10;j++)
    {
      a[i][j]=1;
    }
  }
  // a[5][11]=1;
  // a[5][12]=1;
  for(i=1;i<=5;i++)
  {
    for(j=13;j<=15;j++)
    a[i][j]=1;
  }
  a[2][14]=0;
  block.x1=-14;
  block.y1=-6;
  block.z1=1.2;
  block.x2=-14;
  block.y2=-6;
  block.z2=3.2;
  board_length=16;
  board_width=6;
  block.goal_x=2;
  block.goal_y=14;
  block.anglex=0;
  block.angley=0;
  block.translatex=0;
  block.translatey=0;
  active_circle=0;
  active_into=0;
}
if(level==3)
{
  for(i=1;i<=2;i++)
  for(j=4;j<=10;j++)
  a[i][j]=2;
  for(j=1;j<=4;j++)
  a[3][j]=1;
  for(j=10;j<=12;j++)
  a[3][j]=1;
  for(i=4;i<=7;i++)
  {
    for(j=1;j<=3;j++)
    a[i][j]=1;
  }
  a[4][11]=1;a[4][12]=1;a[5][11]=1;a[5][12]=1;
  for(i=6;i<=9;i++)
  {
    for(j=6;j<=8;j++)
    a[i][j]=1;
  }
  a[8][7]=0;
  a[6][10]=1;a[6][9]=1;a[7][10]=1;a[7][9]=1;
  for(i=6;i<=9;i++)
  for(j=11;j<=14;j++)
  a[i][j]=1;
  for(i=6;i<=7;i++)
  {
    for(j=10;j<=14;j++)
    a[i][j]=2;
  }
  for(i=8;i<=9;i++)
  for(j=11;j<=14;j++)
  a[i][j]=2;
  a[8][13]=1;
  block.x1=-12;
  block.y1=-4;
  block.z1=1.2;
  block.x2=-12;
  block.y2=-4;
  block.z2=3.2;
  board_length=14;
  board_width=10;
  block.goal_x=8;
  block.goal_y=7;
  block.anglex=0;
  block.angley=0;
  block.translatex=0;
  block.translatey=0;
}
if(level==4)
{
  for(i=4;i<=6;i++)
  {
    for(j=1;j<=6;j++)
    {
      a[i][j]=1;
    }
  }
  // a[5][5]=1;
  // a[5][6]=1;
  for(i=1;i<=9;i++)
  {
    for(j=9;j<=11;j++)
    {
      a[i][j]=1;
    }
  }
  // a[5][11]=1;
  // a[5][12]=1;
  for(i=4;i<=6;i++)
  {
    for(j=12;j<=14;j++)
    a[i][j]=1;
  }
  a[5][13]=0;
  block.x1=-12;
  block.y1=-3;
  block.z1=1.2;
  block.x2=-12;
  block.y2=-3;
  block.z2=3.2;
  board_length=14;
  board_width=9;
  block.goal_x=5;
  block.goal_y=13;
  block.anglex=0;
  block.angley=0;
  block.translatex=0;
  block.translatey=0;
  active_circle=0;
  active_into=0;
}
}
int main (int argc, char** argv)
{
	int width = 1000;
	int height =1000,newlevel=0;

  GLFWwindow* window = initGLFW(width, height);
	initGL (window, width, height);

    double last_update_time = glfwGetTime();
		start_time=glfwGetTime();
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        if(newlevel==0)
        {
          newlevel=1;
					lives=3;
					present_moves[level]=moves[level]+6;
					system("mpg123 -vC sounds/2.mp3 &");
          initialiselevel();
        }
        if(newlevel!=level)
        {
        level=newlevel;
				lives=3;
				present_moves[level]=moves[level]+6;
				if(level<=4)
				system("mpg123 -vC sounds/2.mp3 &");
        initialiselevel();
        }
        // OpenGL Draw commands
        newlevel=draw(level);
				if(lives>0 && presentlives!=lives)
				{
					presentlives=lives;
					present_moves[level]=moves[level]+6;
					if(level<=4)
					system("mpg123 -vC sounds/2.mp3 &");
					initialiselevel();
				}
        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
				drag(window);
        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
