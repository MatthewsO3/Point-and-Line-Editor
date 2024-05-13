//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    :
// Neptun :
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram;
// vertex and fragment shaders
unsigned int vao;	   // virtual world on the GPU

float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix,
                          0, 1, 0, 0,    // row-major!
                          0, 0, 1, 0,
                          0, 0, 0, 1 };

class Object {
    unsigned int vao, vbo;
    std::vector<vec3> vtx;

public:
    Object() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    std::vector<vec3>& Vtx() {
        return vtx;
    }
    void updateGpu() {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vtx.size() * sizeof(vec3), &vtx[0], GL_DYNAMIC_DRAW);
    }
    void Draw(int type, vec3 color) {
        if(!vtx.empty()) {
            glBindVertexArray(vao);
            gpuProgram.setUniform(color, "color");
            glDrawArrays(type, 0, vtx.size());
        }
    }

};

class PointCollection{
    Object points;

public:
    void addPoint(vec3 p) {
        points.Vtx().push_back(p);
        printf("Point %f %f added\n", p.x,p.y);
    }
    bool not_in(vec3 p)
    {
        for(int i=0;i<points.Vtx().size();i++)
        {
            if(points.Vtx()[i].x == p.x && points.Vtx()[i].y == p.y)
            {
                return false;
            }
        }
        return true;
    }
    int size()
    {
        return points.Vtx().size();
    }
    void update(){
        points.updateGpu();
    }
    void Draw(vec3 color) {
        points.Draw(GL_POINTS, color);
    }
    vec3 distance_from_point(vec3 point1) {
        vec3 closest = vec3(0,0,0);
        for(int i = 0 ; i<points.Vtx().size(); i++)
        {
          if( sqrt(pow(points.Vtx()[i].x - point1.x, 2) + pow(points.Vtx()[i].y - point1.y, 2)) < 0.05) {
              closest = vec3(points.Vtx()[i].x, points.Vtx()[i].y, 1);
          }
        }
        return closest;
    }

};
PointCollection * collection;

class Line {
    vec3 point1, point2;
public:

    Line(vec3 p1, vec3 p2) : point1(p1), point2(p2){}

    static void translateLineToPoint(vec3& linePoint1, vec3& linePoint2, vec3 point) {


        // Egyenes egyik végpontjának és az adott pontnak a különbsége
        vec3 oldMidPoint = (linePoint1 + linePoint2) / 2.0f; // Az eredeti egyenes középpontja
        vec3 translation = point - oldMidPoint; // Az eltolási vektor

        linePoint1 =linePoint1 + translation; // Az egyenes elsõ végpontjának eltolása
        linePoint2 =linePoint2 + translation; //
    }

    vec3 getPoint1() const {
        return point1;
    }

    vec3 getPoint2() const {
        return point2;
    }

    static vec3 intersection(vec3 p1, vec3 p2, vec3 p3, vec3 p4){

        vec3 d1 = p2 - p1;
        vec3 d2 = p4 - p3;

        float det = d1.x * d2.y - d1.y * d2.x;

        if (det == 0) {
            return vec3(0, 0, 0); // Egyenesek párhuzamosak, nincs metszéspont
        }

        float t = ((p3.x - p1.x) * d2.y - (p3.y - p1.y) * d2.x) / det;
        float s = ((p1.y - p3.y) * d1.x - (p1.x - p3.x) * d1.y) / det;

        if (t >= 0 && t <= 1 && s >= 0 && s <= 1) {
            return vec3(p1.x + d1.x * t, p1.y + d1.y * t, 1);
        } else {
            return vec3(0, 0, 0); // A két szakasz nem metszi egymást
        }
    }


};
class LineCollection
        {
            Object lines;
public:
   Object &getLines()  {
        return lines;
    }

public:
    void line_calc(vec3 *p1, vec3 *p2)
    {
       float a,b,c;
       a = p2->y-p1->y;
       b = p1->x-p2->x;
       c = p2->x * p1->y - p1->x * p2->y;
       float x = -1.0f;
       float y = (-a*x-c)/b;
       *p1 = vec3(x,y,1);
        x = 1.0f;
        y= (-a*x-c)/b;
       *p2 = vec3(x,y,1);
    }
    void addLine(Line l, vec3 p1, vec3 p2)
    {
        lines.Vtx().push_back(l.getPoint1());
        lines.Vtx().push_back(l.getPoint2());
        printf("Line added between (%f, %f) and (%f,%f)\n", p1.x,p1.y,p2.x,p2.y);
        printf("\tImplicit: %.1f x + %.1f y + %.1f = 0\n",(p2.x-p1.x),(p2.y - p1.y), (-1 * p1.x)*p2.y + (-1 * p1.y)*p2.x);
        printf("\tParametetric: r(t) = <%.1f, %.1f> + <%.1f, %.1f>t\n", p1.x, p1.y, (p2.x - p1.x), (p2.y - p1.y));
    }
    void update(){
        lines.updateGpu();
    }
    void Draw(vec3 color) {
        lines.Draw(GL_LINES, color);
    }
    int near_line(vec3 p)
            {

                int nearestIndex = -1; // Az index, ahol a legközelebbi egyenes található

                for (int i = 0; i < lines.Vtx().size(); i += 2) { // Minden második pont egy egyenes kezdõpontja
                    vec3 linePoint1 = lines.Vtx()[i];
                    vec3 linePoint2 = lines.Vtx()[i + 1];

                    // Az egyenesre merõleges távolság kiszámítása a ponttól
                    float distanceToLine = distance(p, linePoint1, linePoint2);

                    // Ha ez a távolság a jelenlegi legkisebb távolságnál kisebb, akkor frissítjük
                    if (distanceToLine < 0.01) {
                        nearestIndex = i;
                    }
                }

                return nearestIndex;
            }
    float distance(vec3 p, vec3 linePoint1, vec3 linePoint2)
    {
        vec3 dir = linePoint2 - linePoint1; // Az egyenes irányvektora
        vec3 v = p - linePoint1; // A pont és az egyenes kezdõpontja közötti vektor

        // Az egyenesre merõleges távolság kiszámítása a ponttól
        float distanceToLine = sqrt(
                pow((dir.y * v.z - dir.z * v.y), 2) +
                pow((dir.z * v.x - dir.x * v.z), 2) +
                pow((dir.x * v.y - dir.y * v.x), 2)) /
                               sqrt(pow(dir.x, 2) + pow(dir.y, 2) + pow(dir.z, 2));

        return distanceToLine;
    }
    void setClicked(vec3 &p1,vec3 &p2, int i)
    {
        p1 = lines.Vtx()[i];
        p2 = lines.Vtx()[i+1];
    }
};
LineCollection *lines;

// Initialization, create an OpenGL context
    void onInitialization() {
        glViewport(0, 0, windowWidth, windowHeight);
        glGenVertexArrays(1, &vao);    // get 1 vao id
        glBindVertexArray(vao);        // make it active
        unsigned int vbo;        // vertex buffer object
        glGenBuffers(1, &vbo);    // Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        collection = new PointCollection();
        lines = new LineCollection();
       /* collection->addPoint(vec3(0.5,-0.5));
    collection->addPoint(vec3(-0.5,-0.5));
    collection->addPoint(vec3(-0.5,0.5));
    collection->addPoint(vec3(-0.17,0.4));*/

    collection->update();


    gpuProgram.create(vertexSource, fragmentSource, "outColor");
        glPointSize(10);
        glLineWidth(3);
}

// Window has become invalid: Redraw
void onDisplay()
{
        glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer
        int location = glGetUniformLocation(gpuProgram.getId(),"MVP");    // Get the GPU location of uniform variable MVP
        glUniformMatrix4fv(location, 1, GL_TRUE,&MVPtransf[0][0]);
        glBindVertexArray(vao);  // Draw call
        lines->Draw(vec3(0, 1, 1));
        collection->Draw(vec3(1, 0, 0));
        glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
bool key_pressed[4] = {false, false, false, false}; //p,l,m,i
void onKeyboard(unsigned char key, int pX, int pY)
{
        switch (key) {
            case 'p': {key_pressed[0] = true;
                for (int i = 1; i < 4; i++)
                {
                    key_pressed[i] = false;
                }
                break;}

            case 'l': {key_pressed[1] = true; 
                for (int i = 0; i < 4; i++)
                {
                    if(i!=1)
                    {
                        key_pressed[i] = false;
                    }
                }
                break;}

            case 'm':{key_pressed[2] = true;
                for (int i = 0; i < 4; i++)
                {
                    if (i != 2)
                    {
                        key_pressed[i] = false;
                    }
                }break;}

            case 'i':{key_pressed[3] = true;
                for (int i = 0; i < 4; i++)
                {
                    if (i != 3)
                    {
                        key_pressed[i] = false;
                    }
                }break;}
            default: break;
        }
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY)
{
    switch (key)
    {
        case 'p':  break;

        case 'l':  break;

        case 'm': break;

        case 'i': break;
        default:
            break;
    }
}
int nearestLineIndex=-1;
vec3 p1 = vec3(0,0,0), p2= vec3(0,0,0);
// Move mouse with key pressed
void onMouseMotion(int pX,int pY) {    // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    if (key_pressed[2]) {
        Line::translateLineToPoint(p1, p2, vec3(cX, cY, 1));
        if (nearestLineIndex != -1) {
            lines->line_calc(&p1, &p2);
            lines->getLines().Vtx()[nearestLineIndex] = p1;
            lines->getLines().Vtx()[nearestLineIndex + 1] = p2;
            lines->update();
            glutPostRedisplay();


        }
        //printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
    }
}

// Mouse click event
vec3 clicked_point= vec3(0,0,0);
vec3 l1= vec3(0,0,0), l2= vec3(0,0,0);

vec3 searched = vec3(0,0,0);
void onMouse(int button, int state, int pX, int pY) // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
{// Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    char *buttonStat;
    switch (state) {
        case GLUT_DOWN: {
            buttonStat = "pressed";
            if (key_pressed[2]) {
                nearestLineIndex = lines->near_line(vec3(cX, cY, 1)); // legközelebbi egyenes indexének lekérése
                if (nearestLineIndex != -1) { // ha van legközelebbi egyenes
                    lines->setClicked(p1, p2,
                                      nearestLineIndex); // kiválasztott egyenes kezdõ- és végpontjainak beállítása
                    printf("Selected line  mozgatasra between (%f, %f) and (%f, %f)\n", p1.x, p1.y, p2.x, p2.y);

                }
            }
            break;
        }
            case GLUT_UP: {
                buttonStat = "released";
                if (key_pressed[2]) //m
                {
                    p1 = vec3(0, 0, 0);
                    p2 = vec3(0, 0, 0);
                    nearestLineIndex = -1;
                        /*Line::translateLineToPoint(p1, p2, vec3(cX, cY, 1));
                        if(nearestLineIndex != -1) {
                            lines->line_calc(&p1, &p2);
                            lines->getLines().Vtx()[nearestLineIndex] = p1;
                            lines->getLines().Vtx()[nearestLineIndex + 1] = p2;
                            lines->update();
                            p1 = vec3(0, 0, 0);
                            p2 = vec3(0, 0, 0);
                            nearestLineIndex = -1;
                        }*/
                }
                break;
            }
    }
    switch (button) {
            case GLUT_LEFT_BUTTON: {
               // printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);

                if (key_pressed[0] && state==GLUT_DOWN) { //p
                    if(collection->not_in(vec3(cX,cY,1))) {
                        collection->addPoint(vec3(cX, cY, 1));
                        collection->update();
                        glutPostRedisplay();
                    }
                }
                if (key_pressed[1] && state == GLUT_DOWN) { //l
                    if (clicked_point.x != 0 && clicked_point.y != 0) {
                        searched = collection->distance_from_point(vec3(cX, cY, 1));
                        if(searched.x !=0 && searched.y != 0)
                        {
                            lines->line_calc(&clicked_point,&searched);
                            lines->addLine(Line(clicked_point, searched), clicked_point, searched);
                            lines->update();
                            clicked_point = vec3(0, 0, 1);
                            searched = vec3(0,0,0);
                        }
                    } else {
                        searched = collection->distance_from_point(vec3(cX, cY, 1));
                        if (searched.x != 0 && searched.y != 0) {
                            clicked_point = searched;

                        }
                    }
                    glutPostRedisplay();
                }
                if (key_pressed[3] && state == GLUT_DOWN) // i
                {
                    if (p1.x != 0 && p1.y != 0) { // ellenõrizd, hogy már kiválasztottál-e egy egyenest
                        nearestLineIndex = lines->near_line(vec3(cX, cY, 1)); // legközelebbi egyenes indexének lekérése
                        if (nearestLineIndex != -1)
                        { // ha van legközelebbi egyenes
                            lines->setClicked(l1, l2, nearestLineIndex);
                            printf("Selected line between (%f, %f) and (%f, %f)\n", l1.x, l1.y, l2.x, l2.y);
                            vec3 p = Line::intersection(p1,p2,l1,l2);
                            if(p.x !=0 && p.y != 0 && p.x<=1 && p.x>=-1 && p.y<=1 && p.y>=-1) {
                                collection->addPoint(p);
                                collection->update();
                                p1 = vec3(0,0,0);
                                p2 = vec3(0,0,0);
                                l1 = vec3(0,0,0);
                                l2 = vec3(0,0,0);
                                p=vec3(0,0,0);
                                //nearestLineIndex = -1;
                                glutPostRedisplay();
                            }
                        }
                    } else { // ha még nem választottál ki egy egyenest sem
                        nearestLineIndex = lines->near_line(vec3(cX, cY, 1)); // legközelebbi egyenes indexének lekérése
                        if (nearestLineIndex != -1) { // ha van legközelebbi egyenes
                            lines->setClicked(p1, p2, nearestLineIndex); // kiválasztott egyenes kezdõ- és végpontjainak beállítása
                            printf("Selected first line between (%f, %f) and (%f, %f)\n", p1.x, p1.y, p2.x, p2.y);

                        }
                    }
                    glutPostRedisplay();
                }
                break;
            }

            case GLUT_MIDDLE_BUTTON:
                //printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
                break;

            case GLUT_RIGHT_BUTTON:
                //printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);


                break;
        }
    }

// Idle event indicating that some time elapsed: do animation here
    void onIdle() {
        long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
    }
