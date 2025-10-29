#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

const int WIDTH = 1200, HEIGHT = 800;
int currentMode = 1;

// -------- Common Structures --------
struct P { int x, y; };
struct C { int cx, cy, radius; float r, g, b; };
struct W { int xmin, ymin, xmax, ymax; };

vector<pair<P, P>> drawnLines;
vector<pair<P, P>> clippedLines;
vector<C> circleList;

P firstClick = {-1,-1};
P firstClip = {-1,-1};

int thickness = 1;
W clipBox = {250, 200, 850, 600};

// ========= BRESENHAM LINE =========
void drawPixel(int x, int y){
    glBegin(GL_POINTS);
    glVertex2i(x,y);
    glEnd();
}

void drawLineBasic(int x1,int y1,int x2,int y2){
    int dx = abs(x2-x1), dy = abs(y2-y1);
    int sx = (x1 < x2) ? 1:-1, sy = (y1 < y2) ? 1:-1;
    int err = dx - dy;
    while(true){
        drawPixel(x1,y1);
        if(x1==x2 && y1==y2) break;
        int e2 = err*2;
        if(e2 > -dy){ err -= dy; x1 += sx; }
        if(e2 < dx){ err += dx; y1 += sy; }
    }
}

void drawLineThick(int x1,int y1,int x2,int y2,int t){
    if(t <= 1) return drawLineBasic(x1,y1,x2,y2);
    double dx=x2-x1, dy=y2-y1;
    double len=sqrt(dx*dx+dy*dy);
    double offX = -dy/len*(t/2.0);
    double offY = dx/len*(t/2.0);
    for(int i=-t/2;i<=t/2;i++){
        drawLineBasic(x1+(int)(offX*i),y1+(int)(offY*i),x2+(int)(offX*i),y2+(int)(offY*i));
    }
}

void renderTask1(){
    glColor3f(1,1,1);
    glRasterPos2i(15,HEIGHT-25);
    string msg = "Mode 1: Bresenham Line | Thickness: " + to_string(thickness);
    for(char c:msg) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);

    glColor3f(0,1,0);
    for(auto &L: drawnLines)
        drawLineThick(L.first.x,L.first.y,L.second.x,L.second.y,thickness);

    if(firstClick.x!=-1){
        glColor3f(1,1,0);
        drawPixel(firstClick.x, firstClick.y);
    }
}

// ========= CIRCLE =========
void plotCircle(int cx,int cy,int x,int y,int t){
    for(int i=-t;i<=t;i++){
        drawPixel(cx+x+i,cy+y);
        drawPixel(cx-x+i,cy+y);
        drawPixel(cx+x+i,cy-y);
        drawPixel(cx-x+i,cy-y);
        drawPixel(cx+y+i,cy+x);
        drawPixel(cx-y+i,cy+x);
        drawPixel(cx+y+i,cy-x);
        drawPixel(cx-y+i,cy-x);
    }
}

void midCircle(int cx,int cy,int r,int t){
    int x=0,y=r,d=1-r;
    plotCircle(cx,cy,x,y,t);
    while(x<y){
        x++;
        if(d<0) d+=2*x+1;
        else{ y--; d+=2*(x-y)+1; }
        plotCircle(cx,cy,x,y,t);
    }
}

void makeCircles(){
    circleList.clear();
    int cx=WIDTH/2, cy=HEIGHT/2;
    for(int i=0;i<13;i++){
        C c; c.cx=cx; c.cy=cy; c.radius = 25*(i+1);
        float t = (float)i/12;
        c.r = 1 - t*0.4;
        c.g = 0.2 + t*0.5;
        c.b = 0.3 + t*0.6;
        circleList.push_back(c);
    }
}

void renderTask2(){
    glColor3f(1,1,1);
    glRasterPos2i(15,HEIGHT-25);
    string msg="Mode 2: Concentric Circles | Press SPACE to Regenerate";
    for(char c:msg) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);

    for(int i=circleList.size()-1;i>=0;i--){
        glColor3f(circleList[i].r,circleList[i].g,circleList[i].b);
        midCircle(circleList[i].cx,circleList[i].cy,circleList[i].radius, 1+i/4);
    }
}

// ========= LIANG-BARSKY =========
bool clipLine(double x1,double y1,double x2,double y2,double& nx1,double& ny1,double& nx2,double& ny2,W w){
    double dx=x2-x1, dy=y2-y1;
    double p[4]={-dx,dx,-dy,dy};
    double q[4]={x1-w.xmin,w.xmax-x1,y1-w.ymin,w.ymax-y1};
    double u1=0,u2=1;
    for(int i=0;i<4;i++){
        if(p[i]==0 && q[i]<0) return false;
        double t=q[i]/p[i];
        if(p[i]<0) u1=max(u1,t);
        else if(p[i]>0) u2=min(u2,t);
        if(u1>u2) return false;
    }
    nx1=x1+u1*dx; ny1=y1+u1*dy;
    nx2=x2+u2*dx; ny2=y2+u2*dy;
    return true;
}

void renderTask3(){
    glColor3f(1,1,1);
    glRasterPos2i(15,HEIGHT-25);
    string msg="Mode 3: Liang-Barsky Clipping | Arrow Keys Move Window";
    for(char c:msg) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);

    glColor3f(1,1,0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(clipBox.xmin,clipBox.ymin);
    glVertex2i(clipBox.xmax,clipBox.ymin);
    glVertex2i(clipBox.xmax,clipBox.ymax);
    glVertex2i(clipBox.xmin,clipBox.ymax);
    glEnd();

    for(auto &L: clippedLines){
        glColor3f(.5,.5,.5);
        drawLineBasic(L.first.x,L.first.y,L.second.x,L.second.y);

        double a,b,c,d;
        if(clipLine(L.first.x,L.first.y,L.second.x,L.second.y,a,b,c,d,clipBox)){
            glColor3f(0,1,0);
            drawLineThick((int)a,(int)b,(int)c,(int)d,3);
        }
    }
}

// -------- DISPLAY --------
void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    if(currentMode==1) renderTask1();
    else if(currentMode==2) renderTask2();
    else renderTask3();
    glFlush(); glutSwapBuffers();
}

// -------- INPUT --------
void mouse(int btn,int state,int x,int y){
    if(btn==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        y=HEIGHT-y;
        if(currentMode==1){
            if(firstClick.x==-1) firstClick={x,y};
            else{ drawnLines.push_back({firstClick,{x,y}}); firstClick={-1,-1}; }
        }
        else if(currentMode==3){
            if(firstClip.x==-1) firstClip={x,y};
            else{ clippedLines.push_back({firstClip,{x,y}}); firstClip={-1,-1}; }
        }
        glutPostRedisplay();
    }
}

void keyboard(unsigned char k,int,int){
    if(k=='1') currentMode=1;
    else if(k=='2'){ currentMode=2; if(circleList.empty()) makeCircles(); }
    else if(k=='3') currentMode=3;
    else if((k=='+'||k=='=') && thickness<20) thickness++;
    else if((k=='-'||k=='_') && thickness>1) thickness--;
    else if(k==' ') makeCircles();
    else if(k=='c'||k=='C'){ drawnLines.clear(); clippedLines.clear(); }
    else if(k==27) exit(0);
    glutPostRedisplay();
}

void specialKeys(int key,int,int){
    if(currentMode==3){
        if(key==GLUT_KEY_LEFT) clipBox.xmin--, clipBox.xmax--;
        if(key==GLUT_KEY_RIGHT) clipBox.xmin++, clipBox.xmax++;
        if(key==GLUT_KEY_UP) clipBox.ymin++, clipBox.ymax++;
        if(key==GLUT_KEY_DOWN) clipBox.ymin--, clipBox.ymax--;
        glutPostRedisplay();
    }
}

void init(){
    glClearColor(0,0,0,1);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0,WIDTH,0,HEIGHT);
}

int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(WIDTH,HEIGHT);
    glutCreateWindow("Graphics Assignment (Fixed Version)");
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMainLoop();
}
