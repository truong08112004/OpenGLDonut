#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define M_PI 3.14159265358979323846

float rotationX = 0.0f;
float rotationY = 0.0f;
bool wireframe = false;

// Điều chỉnh các tham số kem để tạo hiệu ứng sóng sin chân thực hơn
float frostingAmplitude = 0.03f; // Giảm biên độ để tạo sóng nhỏ hơn
float frostingFrequency = 8.0f;  // Tăng tần số để tạo nhiều sóng hơn
float frostingThickness = 0.05f; // Tăng độ dày của lớp kem
float frostingOffset = 0.02f;

// Thêm biến toàn cục mới
float drippingFrequency = 10.0f; // Tăng tần số để có nhiều sóng nhỏ hơn
float drippingAmplitude = 0.1f;  // Giảm biên độ xuống rất nhỏ
float drippingOffset = 0.05f;    // Giảm offset để đường cắt thấp hơn

// Thêm nhiều màu sắc hơn cho cốm
const int NUM_SPRINKLE_COLORS = 8;
GLfloat sprinkleColors[NUM_SPRINKLE_COLORS][4] = {
    {1.0f, 0.2f, 0.2f, 1.0f}, // Đỏ
    {0.2f, 1.0f, 0.2f, 1.0f}, // Xanh lá
    {1.0f, 1.0f, 0.2f, 1.0f}, // Vàng
    {1.0f, 0.5f, 0.0f, 1.0f}, // Cam
    {1.0f, 0.4f, 0.8f, 1.0f}, // Hồng
    {0.4f, 0.4f, 1.0f, 1.0f}, // Xanh dương
    {0.8f, 0.4f, 1.0f, 1.0f}, // Tím
    {0.0f, 0.8f, 0.8f, 1.0f}  // Xanh ngọc
};

// hm tính độ cao của kem với nhiều lớp sóng sin
float calculateFrostingHeight(float angle, float theta)
{
    // Kết hợp nhiều hàm sin với tần số và pha khác nhau
    float height = frostingAmplitude * (sin(frostingFrequency * angle) +
                                        sin(frostingFrequency * 0.5f * angle + 2.0f) * 0.5f +
                                        sin(frostingFrequency * 0.25f * theta) * 0.25f +      // Thêm sóng theo theta
                                        sin(frostingFrequency * 2.0f * angle + theta) * 0.15f // Thêm sóng chéo
                                       );

    // Thêm sự giảm dần của độ cao theo góc theta
    float thetaFactor = cos(theta * 2.0f);
    height *= pow((thetaFactor + 1.0f) * 0.5f, 3.0f); // Tăng số mũ để làm mềm hơn ở viền

    return height + frostingOffset;
}

// Hàm tính độ muốt cho viên kem
float smoothstepEdge(float x)
{
    if (x < 0.0f)
        return 0.0f;
    if (x > 1.0f)
        return 1.0f;
    return x * x * (3 - 2 * x); // Hàm smoothstep để làm mềm các cạnh
}

// Hàm vẽ lớp kem
void drawFrosting()
{
    float R = 1.0f; // Bán kính vòng tròn lớn
    float r = 0.3f; // Bán kính đường tròn
    int mainSegments = 210;
    int tubeSegments = 140;

    // Điều chỉnh màu kem sang màu trắng kem tự nhiên hơn
    GLfloat frostingColor[] = {0.96f, 0.94f, 0.90f, 1.0f};
    GLfloat frostingSpecular[] = {0.6f, 0.6f, 0.6f, 1.0f}; // Giảm độ bóng
    GLfloat frostingShininess[] = {30.0f};                 // Giảm độ bóng để trông như kem thật

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, frostingColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, frostingSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, frostingShininess);

    for (int i = 0; i < mainSegments; i++)
    {
        float phi = 2.0f * M_PI * i / mainSegments;
        float nextPhi = 2.0f * M_PI * (i + 1) / mainSegments;

        // Tính đường cắt sin với nhiều tần số và pha khác nhau để tạo hiệu ứng mượt
        float cutHeight = drippingOffset - drippingAmplitude * (sin(drippingFrequency * phi) +
                                                                sin(drippingFrequency * 1.3f * phi + 0.2f) * 0.8f +
                                                                sin(drippingFrequency * 1.7f * phi + 0.5f) * 0.6f +
                                                                sin(drippingFrequency * 2.1f * phi + 0.8f) * 0.4f +
                                                                sin(drippingFrequency * 2.8f * phi + 1.0f) * 0.3f +
                                                                sin(drippingFrequency * 3.2f * phi + 1.3f) * 0.2f +
                                                                sin(drippingFrequency * 4.1f * phi + 1.6f) * 0.1f);

        // Vẽ kem chỉ đến độ cao được xác định bởi đường cắt
        for (int j = 0; j < tubeSegments / 2 + 2; j++)
        {
            float theta = 2.0f * M_PI * j / tubeSegments;
            float nextTheta = 2.0f * M_PI * (j + 1) / tubeSegments;

            float heightRatio = (float)j / (tubeSegments / 2);
            if (heightRatio > cutHeight)
            {
                // Vẽ khi đến nửa trên và thêm hiệu ứng mềm dần đến viên
                float edgeFactor = smoothstepEdge((sin(theta) + 1.0f) * 0.5f);
                if (edgeFactor > 0.01f)
                {
                    // Tính độ cao của kem với hiệu ứng mềm dần
                    float frostHeight = calculateFrostingHeight(phi, theta) * edgeFactor;
                    float nextFrostHeight = calculateFrostingHeight(nextPhi, theta) * edgeFactor;

                    glBegin(GL_QUADS);

                    // Điểm 1 (trên bề mặt donut)
                    float x1 = (R + r * cos(theta)) * cos(phi);
                    float y1 = r * sin(theta);
                    float z1 = (R + r * cos(theta)) * sin(phi);

                    // Điểm 2 (trên bề mặt kem)
                    float x2 = (R + (r + frostingThickness + frostHeight) * cos(theta)) * cos(phi);
                    float y2 = (r + frostingThickness * edgeFactor) * sin(theta);
                    float z2 = (R + (r + frostingThickness + frostHeight) * cos(theta)) * sin(phi);

                    // Điểm 3 (trên bề mặt kem tại góc tiếp theo)
                    float x3 = (R + (r + frostingThickness + nextFrostHeight) * cos(theta)) * cos(nextPhi);
                    float y3 = (r + frostingThickness * edgeFactor) * sin(theta);
                    float z3 = (R + (r + frostingThickness + nextFrostHeight) * cos(theta)) * sin(nextPhi);

                    // Điểm 4 (trên bề mặt donut tại góc tiếp theo)
                    float x4 = (R + r * cos(theta)) * cos(nextPhi);
                    float y4 = r * sin(theta);
                    float z4 = (R + r * cos(theta)) * sin(nextPhi);

                    // Tính normal vector với hiệu ứng mềm mịn
                    float nx = cos(phi) * cos(theta);
                    float ny = sin(theta);
                    float nz = sin(phi) * cos(theta);

                    // Điều chỉnh normal để tạo hiệu ứng ánh sáng mềm mịn
                    float normalLength = sqrt(nx * nx + ny * ny + nz * nz);
                    nx = nx / normalLength * 0.8f + 0.2f;
                    ny = ny / normalLength * 0.8f + 0.2f;
                    nz = nz / normalLength * 0.8f + 0.2f;

                    glNormal3f(nx, ny, nz);
                    glVertex3f(x1, y1, z1);
                    glVertex3f(x2, y2, z2);
                    glVertex3f(x3, y3, z3);
                    glVertex3f(x4, y4, z4);

                    glEnd();

                    // Vẽ mặt bên của kem với hiệu ứng mềm dần
                    if (j > 0 && edgeFactor > 0.1f)
                    {
                        glBegin(GL_QUADS);

                        float prevTheta = theta - 2.0f * M_PI / tubeSegments;
                        float prevEdgeFactor = smoothstepEdge((sin(prevTheta) + 1.0f) * 0.5f);

                        // Tính các điểm cho mặt bên với độ cao và độ mềm được điều chỉnh
                        float prevY = (r + frostingThickness * prevEdgeFactor) * sin(prevTheta);
                        float prevFrostHeight = calculateFrostingHeight(phi, prevTheta) * prevEdgeFactor;
                        float prevX = (R + (r + frostingThickness + prevFrostHeight) * cos(prevTheta)) * cos(phi);
                        float prevZ = (R + (r + frostingThickness + prevFrostHeight) * cos(prevTheta)) * sin(phi);

                        float nextPrevFrostHeight = calculateFrostingHeight(nextPhi, prevTheta) * prevEdgeFactor;
                        float nextPrevX = (R + (r + frostingThickness + nextPrevFrostHeight) * cos(prevTheta)) * cos(nextPhi);
                        float nextPrevZ = (R + (r + frostingThickness + nextPrevFrostHeight) * cos(prevTheta)) * sin(nextPhi);

                        // Tính normal vector cho mặt bên
                        float sideNx = (x3 - prevX);
                        float sideNy = (y3 - prevY);
                        float sideNz = (z3 - prevZ);
                        float sideLength = sqrt(sideNx * sideNx + sideNy * sideNy + sideNz * sideNz);

                        glNormal3f(sideNx / sideLength, sideNy / sideLength, sideNz / sideLength);
                        glVertex3f(x2, y2, z2);
                        glVertex3f(prevX, prevY, prevZ);
                        glVertex3f(nextPrevX, prevY, nextPrevZ);
                        glVertex3f(x3, y3, z3);

                        glEnd();
                    }
                }
            }
        }
    }
}

// Hàm vẽ donut (giữ nguyên như c)
void drawDonut()
{
    float R = 1.0f;
    float r = 0.3f;
    int mainSegments = 210;
    int tubeSegments = 140;

    GLfloat materialColor[] = {0.8f, 0.4f, 0.2f, 1.0f};
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat materialShininess[] = {50.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

    for (int i = 0; i < mainSegments; i++)
    {
        float phi = 2.0f * M_PI * i / mainSegments;
        float nextPhi = 2.0f * M_PI * (i + 1) / mainSegments;

        for (int j = 0; j < tubeSegments; j++)
        {
            float theta = 2.0f * M_PI * j / tubeSegments;
            float nextTheta = 2.0f * M_PI * (j + 1) / tubeSegments;

            glBegin(GL_QUADS);

            float x1 = (R + r * cos(theta)) * cos(phi);
            float y1 = r * sin(theta);
            float z1 = (R + r * cos(theta)) * sin(phi);

            float x2 = (R + r * cos(nextTheta)) * cos(phi);
            float y2 = r * sin(nextTheta);
            float z2 = (R + r * cos(nextTheta)) * sin(phi);

            float x3 = (R + r * cos(nextTheta)) * cos(nextPhi);
            float y3 = r * sin(nextTheta);
            float z3 = (R + r * cos(nextTheta)) * sin(nextPhi);

            float x4 = (R + r * cos(theta)) * cos(nextPhi);
            float y4 = r * sin(theta);
            float z4 = (R + r * cos(theta)) * sin(nextPhi);

            float nx1 = cos(phi) * cos(theta);
            float ny1 = sin(theta);
            float nz1 = sin(phi) * cos(theta);

            glNormal3f(nx1, ny1, nz1);
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
            glVertex3f(x3, y3, z3);
            glVertex3f(x4, y4, z4);

            glEnd();
        }
    }
}

// Hàm vẽ một hạt cốm
void drawSprinkle(float x, float y, float z, float angleX, float angleY) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);

    // Kích thước của hạt cốm hình trụ
    float radius = 0.02f;    // Bán kính của trụ
    float height = 0.12f;    // Chiều cao của trụ
    int segments = 12;       // Số đoạn để tạo hình tròn

    // Chọn màu ngẫu nhiên từ mảng màu
    int colorIndex = rand() % NUM_SPRINKLE_COLORS;
    GLfloat *color = sprinkleColors[colorIndex];

    // Thiết lập material cho hạt cốm
    GLfloat sprinkleMaterial[] = {color[0], color[1], color[2], 1.0f};
    GLfloat sprinkleSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat sprinkleShininess[] = {50.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sprinkleMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, sprinkleSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, sprinkleShininess);

    // Vẽ thân hình trụ
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * M_PI * i) / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        
        // Normal vector cho điểm trên thân trụ
        glNormal3f(cos(angle), sin(angle), 0.0f);
        
        // Điểm trên và dưới của thân trụ
        glVertex3f(x, y, height);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    // Vẽ mặt trên của hình trụ
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, height);  // Tâm
    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * M_PI * i) / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex3f(x, y, height);
    }
    glEnd();

    // Vẽ mặt dưới của hình trụ
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);  // Tâm
    for (int i = segments; i >= 0; i--) {
        float angle = (2.0f * M_PI * i) / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    glPopMatrix();
}

// Thêm hàm vẽ tất cả các hạt cốm
void drawSprinkles()
{
    srand(123);

    float R = 1.0f;
    float r = 0.3f;
    int numSprinkles = 500;

    for (int i = 0; i < numSprinkles; i++)
    {
        float phi = ((float)rand() / RAND_MAX) * 2 * M_PI;
        float theta = ((float)rand() / RAND_MAX) * M_PI * 0.5f;

        if (sin(theta) > 0.3f)
        {
            // Giảm biên độ dao động của độ cao
            float heightOffset = 0.03f + ((float)rand() / RAND_MAX) * 0.01f; // Giảm độ dao động độ cao
            float inwardOffset = 0.2f;
            float adjustedR = R - inwardOffset;

            // Tính toán vị trí với độ cao ổn định hơn
            float x = (adjustedR + (r + frostingThickness + heightOffset) * cos(theta)) * cos(phi);
            float y = (r + frostingThickness + heightOffset) * sin(theta);
            float z = (adjustedR + (r + frostingThickness + heightOffset) * cos(theta)) * sin(phi);

            // Giảm góc nghiêng ngẫu nhiên
            float angleX = ((float)rand() / RAND_MAX - 0.5f) * 20.0f; // Giảm góc nghiêng
            float angleY = ((float)rand() / RAND_MAX) * 360.0f;

            drawSprinkle(x, y, z, angleX, angleY);
        }
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0.0f, 0.0f, 5.0f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);

    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    // Đảm bảo depth testing được bật
    glEnable(GL_DEPTH_TEST);

    if (wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    drawDonut();
    drawFrosting();
    drawSprinkles(); // Vẽ cốm sau cùng

    glutSwapBuffers();
}

// Các hàm còn lại giữ nguyên như cũ
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = {-5.0f, 5.0f, 5.0f, 1.0f}; // Điểm sáng từ góc trái cao lên
    GLfloat lightAmb[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiff[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightSpec[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        wireframe = !wireframe;
        break;
    case 27:
        exit(0);
        break;
    }
    glutPostRedisplay();
}

void mouseMotion(int x, int y)
{
    static int lastX = x;
    static int lastY = y;

    int deltaX = x - lastX;
    int deltaY = y - lastY;

    rotationY += deltaX * 0.5f;
    rotationX += deltaY * 0.5f;

    lastX = x;
    lastY = y;

    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Donut with Frosting");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(mouseMotion);

    printf("Huong dan su dung:\n");
    printf("- Keo chuot de xoay banh donut\n");
    printf("- Nhan W de chuyen doi giua che do khung day va to mau\n");
    printf("- Nhan ESC de thoat\n");

    glutMainLoop();
    return 0;
}
