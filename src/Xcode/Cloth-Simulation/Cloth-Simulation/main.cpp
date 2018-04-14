#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#if __APPLE__
#include <OpenGL/gl.h>

#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif __linux__
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define BALL_POSITION_X 6
#define BALL_POSITION_Y -2
#define BALL_POSITION_Z 0
#define BALL_RADIUS 3
#define TRUE 1
#define FALSE 0

#define GRAVITY 0.0981
#define DAMPING 0.05
#define TIME_STEPSIZE 0.35

#define CLOTH_WIDTH 25
#define CLOTH_HEIGHT 20

#define PARTICLE_WIDTH 40 // must be >= 2 particles wide
#define PARTICLE_HEIGHT 40 // must be >= 2 particles heigh
#define MASS 1.0

#define ARRAY_SIZE PARTICLE_WIDTH*PARTICLE_HEIGHT
#define SPRING_COUNT 6 + 5*((PARTICLE_WIDTH - 2) + (PARTICLE_HEIGHT - 2)) + 4*(PARTICLE_WIDTH - 2) * (PARTICLE_HEIGHT - 2)
#define CONSTRAINT_ITERATIONS 20


// ************************************
// data structures

typedef struct
{
    float x;
    float y;
    float z;
} Vec3;

typedef struct
{
    int isMovable;
    float mass;
    Vec3 position;
    Vec3 previousPosition;
    Vec3 acceleration;
    Vec3 normal;
} Particle;

typedef struct
{
    int p1;//index for particle 1
    int p2;//index for particle 2
    float restDistance;
} Spring;


// ************************************
// global variables

int ball_time = 0;
int ball_direction = 1;
int ballAnimation = 1;
float ball_speed = 80.0;
float edge_correction = 0.3;
Particle particles[ARRAY_SIZE];
Spring springs[SPRING_COUNT];
Vec3 windForce = { 0.05, 0.01, 0.03 };
Vec3 ball_pos = { 0,0,0 };

// *************************************
// Vector methods

float magnitude(Vec3 v)
{
    return sqrt((v.x*v.x)+(v.y*v.y)+(v.z*v.z));
}

Vec3 normalize(Vec3 v)
{
    float m = sqrt((v.x*v.x)+(v.y*v.y)+(v.z*v.z));
    Vec3 vec = { v.x/m,
        v.y/m,
        v.z/m };
    return vec;
}

Vec3 cross(Vec3 i, Vec3 j)
{
    Vec3 v = {     (i.y * j.z) - (i.z * j.y),
        (i.z * j.x) - (i.x * j.z),
        (i.x * j.y) - (i.y * j.x) };
    return v;
}

float dot(Vec3 i, Vec3 j)
{
    return (i.x * j.x) + (i.y * j.y) + (i.z * j.z);
}

// **************************************
// Particle methods

Vec3 calcNormal(int i, int j, int k)
{
    // calculate normal force of triangle formed
    Vec3 v1 = {
        particles[j].position.x - particles[i].position.x,
        particles[j].position.y - particles[i].position.y,
        particles[j].position.z - particles[i].position.z
    };
    Vec3 v2 = {
        particles[k].position.x - particles[i].position.x,
        particles[k].position.y - particles[i].position.y,
        particles[k].position.z - particles[i].position.z
    };
    Vec3 normal = cross(v1,v2);
    return normal;
}

void initializeNormals()
{
    int i;
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        int y = i / PARTICLE_WIDTH + 1;
        
        // indexes for triangles
        int rightIndex = i + 1;
        int downIndex = i + PARTICLE_WIDTH;
        int downRightIndex = i + PARTICLE_WIDTH + 1;
        
        if (rightIndex < PARTICLE_WIDTH*y && downIndex < ARRAY_SIZE)
        {
            // triangle topleft, bottomleft, and topright
            Vec3 n1 = calcNormal(i, downIndex, rightIndex);
            n1 = normalize(n1);
            
            // triangle bottomright, bottomleft, topright
            Vec3 n2 = calcNormal(downRightIndex, downIndex, rightIndex);
            n2 = normalize(n2);
            
            particles[i].normal.x += n1.x;
            particles[i].normal.y += n1.y;
            particles[i].normal.z += n1.z;
            
            particles[downIndex].normal.x += n1.x + n2.x;
            particles[downIndex].normal.y += n1.y + n2.y;
            particles[downIndex].normal.z += n1.z + n2.z;
            
            particles[rightIndex].normal.x += n1.x + n2.x;
            particles[rightIndex].normal.y += n1.y + n2.y;
            particles[rightIndex].normal.z += n1.z + n2.z;
            
            particles[downRightIndex].normal.x += n2.x;
            particles[downRightIndex].normal.y += n2.y;
            particles[downRightIndex].normal.z += n2.z;
            
        }
    }
}

void initializeParticles()
{
    int i;
    int xPosOffset = (CLOTH_WIDTH / 2) ;
    
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        // create grid of particle positions
        //int y = i / PARTICLE_WIDTH; // sets row position
        //int x = i - (y*PARTICLE_WIDTH)-xPosOffset;
        //Vec3 pos = { x, y, 0 };
        
        float pw = PARTICLE_WIDTH;
        float ph = PARTICLE_HEIGHT;
        float cw = CLOTH_WIDTH;
        float ch = CLOTH_HEIGHT;
        
        int row = i / pw; // sets row position
        float x = ((cw/pw) * (i - (row*pw))) - xPosOffset;
        float y = ch/ph * row;
        
        Vec3 pos = { x, y, 0 };
        // set zero vector
        Vec3 z = { 0,0,0 };
        
        // set particle data
        particles[i].position = pos;
        particles[i].previousPosition = pos;
        particles[i].acceleration = z;
        particles[i].normal = z;
        particles[i].mass = MASS;
        particles[i].isMovable = 1;
        
        // immobilize corners
        if ((row+1)*pw == ARRAY_SIZE && (i + PARTICLE_WIDTH == ARRAY_SIZE || i + 1 == ARRAY_SIZE))
        {
            particles[i].isMovable = 0;
            particles[i-PARTICLE_WIDTH].isMovable = 0;
        }
    }
    initializeNormals();
}

void updateParticles()
{
    int i;
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        int row = i / PARTICLE_WIDTH; // sets row position
        if (particles[i].isMovable)
        {
            Vec3 temp = particles[i].position;
            
            particles[i].position.x += ((temp.x - particles[i].previousPosition.x) * (1.0 - DAMPING))
            + (particles[i].acceleration.x * TIME_STEPSIZE);
            particles[i].position.y += ((temp.y - particles[i].previousPosition.y) * (1.0 - DAMPING))
            + (particles[i].acceleration.y * TIME_STEPSIZE);
            particles[i].position.z += ((temp.z - particles[i].previousPosition.z) * (1.0 - DAMPING))
            + (particles[i].acceleration.z * TIME_STEPSIZE);
            
            particles[i].previousPosition = temp;
            
            //reset acceleration to 0
            Vec3 z = { 0, 0, 0 };
            particles[i].acceleration = z;
        }
    }
}

void addForce(Vec3 force, int i)
{
    particles[i].acceleration.x += force.x / particles[i].mass;
    particles[i].acceleration.y += force.y / particles[i].mass;
    particles[i].acceleration.z += force.z / particles[i].mass;
}

void addWindForceForTriangle(int i, int j, int k)
{
    // calc normal force of triangle formed
    Vec3 v1 = {
        particles[j].position.x - particles[i].position.x,
        particles[j].position.y - particles[i].position.y,
        particles[j].position.z - particles[i].position.z
    };
    Vec3 v2 = {
        particles[k].position.x - particles[i].position.x,
        particles[k].position.y - particles[i].position.y,
        particles[k].position.z - particles[i].position.z
    };
    Vec3 normal = cross(v1,v2);
    Vec3 normalized = normalize(normal);
    float d = dot(normalized, windForce);
    Vec3 force = {
        normal.x * d,
        normal.y * d,
        normal.z * d
    };
    
    // add force to particles acceleration
    addForce(force, i);
    addForce(force, j);
    addForce(force, k);
    
    
    //set normals
    //printf("i = %f, j = %f, k = %f\n", v1.x,v1.y,v1.z);
}

/* generates a random number in the range [min, max] (inclusive). */
int generateRandom(int min, int max)
{
    int value = rand() % max;
    int length = max - min + 1;
    return min + (value % length);
}

void addWindForce()
{
    int i;
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        int y = i / PARTICLE_WIDTH + 1;
        
        // indexes for triangles
        int rightIndex = i + 1;
        int downIndex = i + PARTICLE_WIDTH;
        int downRightIndex = i + PARTICLE_WIDTH + 1;
        
        if (rightIndex < PARTICLE_WIDTH*y && downIndex < ARRAY_SIZE)
        {
            // triangle topleft, bottomleft, and topright
            addWindForceForTriangle(i, downIndex, rightIndex);
            
            // triangle bottomright, bottomleft, topright
            addWindForceForTriangle(downRightIndex, downIndex, rightIndex);
            
        }
    }
}

void addGravityForce()
{
    int i;
    Vec3 g = { 0,-GRAVITY * TIME_STEPSIZE,0 };
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        addForce(g,i);
    }
}

// **************************************
// Spring methods

void initializeSprings()
{
    int i;
    
    // default rest lengths of springs
    float restLengthX = particles[1].position.x - particles[0].position.x;
    float restLengthY = particles[PARTICLE_WIDTH].position.y - particles[0].position.y;
    float restLengthDiagonal = sqrt((restLengthX * restLengthX) + (restLengthY * restLengthY));
    
    printf("x = %f, y = %f\n",restLengthX,restLengthY);
    
    int j = 0;
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        int y = (i / PARTICLE_WIDTH) + 1;
        
        // next possible indexes
        int rightIndex = i + 1;
        int downIndex = i + PARTICLE_WIDTH;
        int rightDownIndex = i + PARTICLE_WIDTH + 1;
        
        //make all possible spring connections
        if (rightIndex < PARTICLE_WIDTH * y)
        {
            // connect top left to top right
            // i to rightIndex
            springs[j].p1 = i;
            springs[j].p2 = rightIndex;
            springs[j].restDistance = restLengthX;
            j++;
            if (rightDownIndex < ARRAY_SIZE)
            {
                // connect top left to bottom right
                // i to rightDownIndex
                springs[j].p1 = i;
                springs[j].p2 = rightDownIndex;
                springs[j].restDistance = restLengthDiagonal;
                j++;
            }
            
            if (downIndex < ARRAY_SIZE)
            {
                // connect bottom left to top right
                // downIndex to rightIndex
                springs[j].p1 = downIndex;
                springs[j].p2 = rightIndex;
                springs[j].restDistance = restLengthDiagonal;
                j++;
            }
        }
        if (downIndex < ARRAY_SIZE)
        {
            // connect top left to bottom left
            // i to downIndex
            springs[j].p1 = i;
            springs[j].p2 = downIndex;
            springs[j].restDistance = restLengthY;
            j++;
        }
    }
}

void constrainSprings()
{
    int i;
    int j;
    for(j=0; j<CONSTRAINT_ITERATIONS; j++) // iterate over all constraints several times
    {
        for (i = 0; i < SPRING_COUNT; i++)
        {
            int p1 = springs[i].p1;
            int p2 = springs[i].p2;
            
            Vec3 stretch = {
                particles[p2].position.x - particles[p1].position.x,
                particles[p2].position.y - particles[p1].position.y,
                particles[p2].position.z - particles[p1].position.z
            };
            
            float distance = magnitude(stretch);
            float d =  0.5 * (1 - (springs[i].restDistance / distance));
            
            Vec3 correction = {
                stretch.x * d,
                stretch.y * d,
                stretch.z * d
            };
            
            if (particles[p1].isMovable)
            {
                particles[p1].position.x += correction.x;
                particles[p1].position.y += correction.y;
                particles[p1].position.z += correction.z;
            }
            
            if (particles[p2].isMovable)
            {
                particles[p2].position.x -= correction.x;
                particles[p2].position.y -= correction.y;
                particles[p2].position.z -= correction.z;
            }
            
            //printf("Spring[%d] p1 = %d, p2 = %d, dist = %f, d = %f\n", i,p1,p2,distance, d);
            //printf("corr.x = %f, corr.y = %f, corr.z = %f\n\n", correction.x,correction.y,correction.z);
        }
    }
}

// *************************************
// Cloth methods

void drawPoint(Vec3 v)
{
    glVertex3f(v.x,v.y,v.z);
    //printf("Drawing point: (%f,%f,%f)\n", v.x,v.y,v.z);
    
}

void drawTriangle(int i, int j, int k)
{
    glNormal3f(particles[i].normal.x,particles[i].normal.y,particles[i].normal.z);
    glVertex3f(particles[i].position.x,particles[i].position.y,particles[i].position.z);
    
    glNormal3f(particles[j].normal.x,particles[j].normal.y,particles[j].normal.z);
    glVertex3f(particles[j].position.x,particles[j].position.y,particles[j].position.z);
    
    glNormal3f(particles[k].normal.x,particles[k].normal.y,particles[k].normal.z);
    glVertex3f(particles[k].position.x,particles[k].position.y,particles[k].position.z);
}

void drawCloth()
{
    //int i;
    //for (i = 0; i < SPRING_COUNT; i ++)
    //{
    //    drawPoint(particles[springs[i].p1].position);
    //    drawPoint(particles[springs[i].p2].position);
    //}
    
    glBegin(GL_TRIANGLES);
    
    
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        int y = i / PARTICLE_WIDTH + 1;
        
        // indexes for triangles
        int rightIndex = i + 1;
        int downIndex = i + PARTICLE_WIDTH;
        int downRightIndex = i + PARTICLE_WIDTH + 1;
        
        //set colors
        int pat = 0;
        if (y % 2 == 0) pat = 1;
        if (i % 2 == pat)
        {
            glColor3f(4.5,4.5,4.5);
        } else
        {
            glColor3f(1.0,3.5,1.5);
        }
        //border color
        if (y == 1 || (y+1)*PARTICLE_WIDTH >= ARRAY_SIZE || i == (y-1)*PARTICLE_WIDTH || i+1 == (y)*PARTICLE_WIDTH - 1)
        {
            glColor3f(0.0,0.0,0.0);
        }
        
        //draw valid triangles
        if (rightIndex < PARTICLE_WIDTH*y && downIndex < ARRAY_SIZE)
        {
            // triangle topleft, bottomleft, and topright
            drawTriangle(i, downIndex, rightIndex);
            
            // triangle bottomright, bottomleft, topright
            drawTriangle(downRightIndex, downIndex, rightIndex);
            
        }
    }
    
    glEnd();
}


void detectCollision()
{
    int i;
    float edge = BALL_RADIUS + edge_correction;
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        Vec3 v = {
            particles[i].position.x - ball_pos.x,
            particles[i].position.y - ball_pos.y,
            particles[i].position.z - ball_pos.z
        };
        Vec3 norm = normalize(v);
        float mag = magnitude(v);
        if (mag <= edge)
        {
            particles[i].position.x += norm.x * (edge - mag);
            particles[i].position.y += norm.y * (edge - mag);
            particles[i].position.z += norm.z * (edge - mag);
        }
    }
}


void resetCloth()
{
    
    // initialize cloth grid
    initializeParticles();
    
    //initialize springs on cloth grid
    initializeSprings();
    
    
    //initialize variables
    ball_time = 250;
    ball_direction = 1;
    ballAnimation = 1;
    ball_speed = 80.0;
    edge_correction = 0.3;
}


// *************************************
// Boiler-plate methods

void init (void)
{
    //resetCloth();
    
    glShadeModel (GL_SMOOTH);
    glClearColor (0.2f, 0.2f, 0.4f, 0.5f);
    glClearDepth (1.0f);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glEnable (GL_COLOR_MATERIAL);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT0);
    GLfloat lightPos[4] = {-1.0, 1.0, 0.5, 0.0};
    glLightfv (GL_LIGHT0, GL_POSITION, (GLfloat *) &lightPos);
    glEnable (GL_LIGHT1);
    GLfloat lightAmbient1[4] = {0.0, 0.0,  0.0, 0.0};
    GLfloat lightPos1[4]     = {1.0, 0.0, -0.2, 0.0};
    GLfloat lightDiffuse1[4] = {0.5, 0.5,  0.3, 0.0};
    glLightfv (GL_LIGHT1,GL_POSITION, (GLfloat *) &lightPos1);
    glLightfv (GL_LIGHT1,GL_AMBIENT, (GLfloat *) &lightAmbient1);
    glLightfv (GL_LIGHT1,GL_DIFFUSE, (GLfloat *) &lightDiffuse1);
    glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
}


void display (void)
{
    
    
    updateParticles();
    constrainSprings();
    addGravityForce();
    if (ball_time > 250) // let cloth have time to drop
    {
        addWindForce();
        detectCollision();
    }
    
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();
    glDisable (GL_LIGHTING);
    glBegin (GL_POLYGON);
    glColor3f (0.8f, 0.8f, 0.8f);
    glVertex3f (-200.0f, -100.0f, -100.0f);
    glVertex3f (200.0f, -100.0f, -100.0f);
    glColor3f (0.4f, 0.4f, 0.8f);
    glVertex3f (200.0f, 100.0f, -100.0f);
    glVertex3f (-200.0f, 100.0f, -100.0f);
    glEnd ();
    glEnable (GL_LIGHTING);
    glTranslatef (1, -7, -(CLOTH_HEIGHT + 5)); // move camera out and center on the cloth
    
    drawCloth();
    
    glPushMatrix ();
    
    if (ballAnimation) ball_time ++;
    float move = cos(ball_time/ball_speed)*20-3;
    
    ball_pos.x = -BALL_RADIUS/2 + 1;
    ball_pos.y = 5;
    ball_pos.z = BALL_POSITION_Z + move;
    
    glTranslatef ( ball_pos.x, ball_pos.y, ball_pos.z );
    glColor3f (1.0f, 1.0f, 0.0f);
    glutSolidSphere (BALL_RADIUS - 0.1, 50, 50); // draw the ball, but with a slightly lower radius, otherwise we could get ugly visual artifacts of rope penetrating the ball slightly
    
    glPopMatrix ();
    glutSwapBuffers();
    glutPostRedisplay();
    
    
    
}





void reshape (int w, int h)
{
    glViewport (0, 0, w, h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    if (h == 0)
    {
        gluPerspective (80, (float) w, 1.0, 5000.0);
    }
    else
    {
        gluPerspective (80, (float) w / (float) h, 1.0, 5000.0);
    }
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
}





void keyboard (unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:
            exit (0);
            break;
        case 32:
            break;
        case '=':
            break;
        case '-':
            break;
        case '>':
        case '.':
            if (ball_speed > 20.0) ball_speed -= 10.0;
            break;
        case '<':
        case ',':
            if (ball_speed < 120) ball_speed +=  10.0;
            break;
        case '1':
            if (edge_correction > 0.1) edge_correction -=  0.05;
            //printf("edge correction = %f\n", edge_correction);
            break;
        case '2':
            if (edge_correction < 1.0) edge_correction +=  0.05;
            //printf("edge correction = %f\n", edge_correction);
            break;
        case 'r':
            resetCloth();
            break;
        case 's':
            if (ballAnimation) ballAnimation = 0;
            else ballAnimation = 1;
            break;
        case 'f':
            glutFullScreen();
            break;
        case 'm':
            glutReshapeWindow (1280, 720 );
            break;
        default:
            break;
    }
}





void arrow_keys (int a_keys, int x, int y)
{
    switch(a_keys)
    {
        case GLUT_KEY_UP:
            windForce.z += 0.001;
            break;
        case GLUT_KEY_DOWN:
            windForce.z -= 0.001;
            break;
        case GLUT_KEY_LEFT:
            windForce.x -= 0.001;
            break;
        case GLUT_KEY_RIGHT:
            windForce.x += 0.001;
            break;
        default:
            break;
    }
}





int main (int argc, char *argv[])
{
  
    
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize (1280, 720 );
    glutCreateWindow ("Cloth simulator");
    init ();
    glutDisplayFunc (display);
    glutReshapeFunc (reshape);
    glutKeyboardFunc (keyboard);
    glutSpecialFunc (arrow_keys);
    resetCloth();
    glutMainLoop ();
    
}

