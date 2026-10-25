#include <glut.h>
#include <iostream>
#include <string>
#include <cmath>

// Status Game
enum GameState { PLAYING, GAME_OVER };
GameState currentState = PLAYING;
std::string winnerText = "";

// Boundary Layar & Game (Sesuai koordinat GeoGebra)
const float X_MIN = -1.2f, X_MAX = 1.3f;
const float Y_MIN = -1.2f, Y_MAX = 1.2f;
const float FINISH_LINE_X = 1.05f; // Posisi garis finish

// Input Keyboard (Untuk smooth multi-key movement)
bool keys[256] = { false };
bool specialKeys[256] = { false };

// Struktur Objek Mobil Balap
struct Car {
    float x, y;
    float width, height;
    float speed;
    float r, g, b;             // Warna primer 
    float rDark, gDark, bDark; // Warna sekunder 
};

// Inisialisasi Mobil (Jalur Atas: Player 1 Biru/Cyan, Jalur Bawah: Player 2 Magenta/Pink)
Car car1 = { -0.85f, -0.40f, 0.42f, 0.14f, 0.02f, 0.10f, 0.65f, 1.00f, 0.05f, 0.35f, 0.70f };
Car car2 = { -0.85f, -0.92f, 0.42f, 0.14f, 0.02f, 1.00f, 0.15f, 0.50f, 0.65f, 0.05f, 0.30f };

// Fungsi Reset Game
void resetGame() {
    car1.x = -0.85f; car1.y = -0.40f;
    car2.x = -0.85f; car2.y = -0.92f;
    currentState = PLAYING;
    winnerText = "";
}

// Untuk buat teks makin tebal
void drawBoldStrokeText(float x, float y, const std::string& text, float scale, float lineWidth, bool center = false, int boldWeight = 5) {
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Hitung panjang teks dalam satuan stroke font GLUT
    float textWidth = (float)glutStrokeLength(GLUT_STROKE_ROMAN, (const unsigned char*)text.c_str()) * scale;
    float startX = center ? (x - textWidth / 2.0f) : x;

    glLineWidth(lineWidth);

    // Ini untuk menghasilkan ketebalan text
    float delta = scale * 0.14f;
    float offsets[][2] = {
        { 0.0f, 0.0f },
        { delta, 0.0f },
        { -delta, 0.0f },
        { 0.0f, delta },
        { 0.0f, -delta },
        { delta * 0.7f, delta * 0.7f },
        { -delta * 0.7f, delta * 0.7f },
        { delta * 0.7f, -delta * 0.7f },
        { -delta * 0.7f, -delta * 0.7f }
    };

    int totalPasses = (boldWeight > 9) ? 9 : ((boldWeight < 1) ? 1 : boldWeight);

    for (int p = 0; p < totalPasses; p++) {
        glPushMatrix();
        glTranslatef(startX + offsets[p][0], y + offsets[p][1], 0.0f);
        glScalef(scale, scale, 1.0f);
        for (char c : text) {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
        }
        glPopMatrix();
    }
}

// Helper function untuk bulan, roda dan lampu
void drawCircle(float cx, float cy, float r, int segments = 16) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * 3.14159265f * float(i) / float(segments);
        glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
    }
    glEnd();
}

// Helper function untuk bikin efek gelow"
void drawGlowCircle(float cx, float cy, float r, float red, float green, float blue, float alphaCenter, int segments = 16) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(red, green, blue, alphaCenter);
    glVertex2f(cx, cy);
    glColor4f(red, green, blue, 0.0f);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * 3.14159265f * float(i) / float(segments);
        glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
    }
    glEnd();
    glDisable(GL_BLEND);
}

// 1.BG
void drawBackground() {
    // a. Langit malam yang bergradasi
    glBegin(GL_QUADS);
    glColor3f(0.04f, 0.03f, 0.09f); // Langit atas
    glVertex2f(X_MIN, Y_MAX);
    glVertex2f(X_MAX, Y_MAX);
    glColor3f(0.14f, 0.08f, 0.24f); // Horizon kota malam
    glVertex2f(X_MAX, 0.05f);
    glVertex2f(X_MIN, 0.05f);
    glEnd();

    // b. Glowingglowing bulan
    drawGlowCircle(-0.90f, 0.95f, 0.16f, 0.95f, 0.95f, 1.0f, 0.35f, 20);
    glColor3f(0.98f, 0.98f, 0.92f);
    drawCircle(-0.90f, 0.95f, 0.065f, 20);

    // Twinkle twinkle little star
    glColor3f(0.9f, 0.9f, 1.0f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glVertex2f(-1.10f, 1.05f);
    glVertex2f(-0.65f, 1.12f);
    glVertex2f(-0.35f, 0.98f);
    glVertex2f( 0.05f, 1.08f);
    glVertex2f( 0.40f, 1.15f);
    glVertex2f( 0.75f, 1.02f);
    glVertex2f( 1.15f, 1.10f);
    glVertex2f(-0.15f, 1.16f);
    glEnd();

    // c. Skyscraper
    glColor3f(0.10f, 0.07f, 0.20f);
    glRectf(-1.15f, 0.05f, -0.65f, 0.85f);
    glRectf(-0.45f, 0.05f,  0.05f, 0.95f);
    glRectf( 0.25f, 0.05f,  0.70f, 0.80f);
    glRectf( 0.85f, 0.05f,  1.35f, 0.75f);

    // d. Skyscraper agak depan 
    // Gedung 1
    glColor3f(0.18f, 0.10f, 0.35f);
    glRectf(-1.25f, 0.05f, -0.80f, 0.70f);
    // Gedung 2 (Tinggi Menjulang)
    glColor3f(0.24f, 0.12f, 0.44f);
    glRectf(-0.75f, 0.05f, -0.30f, 1.00f);
    // Antena Gedung 2 + Lampu Peringatan Pesawat Berkedip
    glColor3f(0.5f, 0.5f, 0.6f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.525f, 1.00f);
    glVertex2f(-0.525f, 1.14f);
    glEnd();
    int timeMs = glutGet(GLUT_ELAPSED_TIME);
    bool beaconOn = (timeMs / 450) % 2 == 0;
    if (beaconOn) {
        glColor3f(1.0f, 0.1f, 0.1f);
        drawCircle(-0.525f, 1.14f, 0.015f, 8);
        drawGlowCircle(-0.525f, 1.14f, 0.06f, 1.0f, 0.1f, 0.1f, 0.5f, 12);
    }

    // Gedung 3
    glColor3f(0.16f, 0.09f, 0.32f);
    glRectf(-0.25f, 0.05f,  0.20f, 0.65f);
    // Gedung 4 
    glColor3f(0.26f, 0.13f, 0.48f);
    glRectf( 0.15f, 0.05f,  0.55f, 0.90f);
    // Antena Gedung 4
    glBegin(GL_LINES);
    glVertex2f(0.35f, 0.90f);
    glVertex2f(0.35f, 1.05f);
    glEnd();
    if (beaconOn) {
        glColor3f(1.0f, 0.1f, 0.1f);
        drawCircle(0.35f, 1.05f, 0.015f, 8);
        drawGlowCircle(0.35f, 1.05f, 0.06f, 1.0f, 0.1f, 0.1f, 0.5f, 12);
    }

    // Gedung 5 & 6
    glColor3f(0.20f, 0.11f, 0.38f);
    glRectf( 0.50f, 0.05f,  0.90f, 0.72f);
    glColor3f(0.15f, 0.08f, 0.30f);
    glRectf( 0.85f, 0.05f,  1.30f, 0.85f);

    // e. Jendela di gedung glowing"
    // Jendela glowing kuning
    glColor3f(1.0f, 0.85f, 0.35f);
    glRectf(-0.70f, 0.85f, -0.62f, 0.89f);
    glRectf(-0.55f, 0.70f, -0.47f, 0.74f);
    glRectf(-0.70f, 0.55f, -0.62f, 0.59f);
    glRectf( 0.20f, 0.75f,  0.28f, 0.79f);
    glRectf( 0.38f, 0.60f,  0.46f, 0.64f);
    glRectf( 0.60f, 0.50f,  0.68f, 0.54f);
    glRectf( 0.72f, 0.35f,  0.80f, 0.39f);

    // Jendela glowing biru
    glColor3f(0.1f, 0.95f, 1.0f);
    glRectf(-1.18f, 0.50f, -1.10f, 0.54f);
    glRectf(-0.55f, 0.85f, -0.47f, 0.89f);
    glRectf(-0.18f, 0.50f, -0.10f, 0.54f);
    glRectf( 0.38f, 0.75f,  0.46f, 0.79f);
    glRectf( 0.95f, 0.65f,  1.03f, 0.69f);

    // Jendela Magenta / Pink Neon
    glColor3f(1.0f, 0.25f, 0.75f);
    glRectf(-0.70f, 0.70f, -0.62f, 0.74f);
    glRectf(-0.18f, 0.35f, -0.10f, 0.39f);
    glRectf( 0.20f, 0.60f,  0.28f, 0.64f);
    glRectf( 0.60f, 0.35f,  0.68f, 0.39f);
    glRectf( 1.10f, 0.50f,  1.18f, 0.54f);

    // f. Trotoar
    glColor3f(0.25f, 0.25f, 0.30f);
    glRectf(X_MIN, 0.02f, X_MAX, 0.08f);

    // detailing trotoar yang ada item" itu
    for (float kx = X_MIN; kx < X_MAX; kx += 0.20f) {
        if (int((kx + 2.0f) * 5.0f) % 2 == 0)
            glColor3f(0.95f, 0.75f, 0.05f); // ini kuning
        else
            glColor3f(0.12f, 0.12f, 0.14f); // ini itam
        glRectf(kx, 0.02f, kx + 0.10f, 0.04f);
    }

    // shadowing trotoar tipis"
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    glRectf(X_MIN, -0.03f, X_MAX, 0.02f);
    glDisable(GL_BLEND);

    // g. Jalan Raya (yang itam lebar)
    glColor3f(0.10f, 0.11f, 0.14f);
    glRectf(X_MIN, Y_MIN, X_MAX, 0.02f);

    // Garis tepi yang kuning atas dan bawah
    glColor3f(0.85f, 0.70f, 0.10f);
    glRectf(X_MIN, -0.06f, X_MAX, -0.04f);
    glRectf(X_MIN, -1.16f, X_MAX, -1.14f);

    // Garis putus" putih
    glColor3f(0.90f, 0.90f, 0.95f);
    for (float mx = -1.15f; mx < X_MAX; mx += 0.45f) {
        glRectf(mx, -0.60f, mx + 0.25f, -0.56f);
    }

    // h. Tiang lampu jalan
    float streetLightsX[] = { -0.85f, -0.25f, 0.35f };
    for (float lx : streetLightsX) {
        // ini tiangnya
        glColor3f(0.18f, 0.20f, 0.25f);
        glRectf(lx - 0.02f, 0.04f, lx + 0.02f, 0.56f);

        // ini yang agak melengkung itu
        glBegin(GL_POLYGON);
        glVertex2f(lx - 0.02f, 0.52f);
        glVertex2f(lx - 0.02f, 0.56f);
        glVertex2f(lx + 0.32f, 0.52f);
        glVertex2f(lx + 0.30f, 0.48f);
        glEnd();

        // ini tempat untuk taru lampu
        glColor3f(0.12f, 0.14f, 0.18f);
        glBegin(GL_POLYGON);
        glVertex2f(lx + 0.24f, 0.50f);
        glVertex2f(lx + 0.36f, 0.50f);
        glVertex2f(lx + 0.38f, 0.45f);
        glVertex2f(lx + 0.22f, 0.45f);
        glEnd();

        // ini bola lampu yang menyala
        glColor3f(1.0f, 0.96f, 0.60f);
        glRectf(lx + 0.25f, 0.44f, lx + 0.35f, 0.46f);

        // ini bulatan sinar sikit" dekat lampu
        drawGlowCircle(lx + 0.30f, 0.45f, 0.10f, 1.0f, 0.92f, 0.50f, 0.50f, 16);

        //ini buat blend kerucut cahaya
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Kerucut Cahaya 
        glBegin(GL_POLYGON);
        glColor4f(1.0f, 0.92f, 0.55f, 0.32f); // Terang di sumber kap lampu
        glVertex2f(lx + 0.25f, 0.44f);
        glVertex2f(lx + 0.35f, 0.44f);
        glColor4f(1.0f, 0.88f, 0.45f, 0.02f); // Memudar ke bawah kanan aspal
        glVertex2f(lx + 0.70f, -1.16f);
        glColor4f(1.0f, 0.88f, 0.45f, 0.02f); // Memudar ke bawah kiri aspal
        glVertex2f(lx - 0.10f, -1.16f);
        glEnd();

        // ini yang terang" di aspal
        glBegin(GL_POLYGON);
        glColor4f(1.0f, 0.92f, 0.55f, 0.16f); 
        glVertex2f(lx + 0.30f, -0.58f);
        glColor4f(1.0f, 0.88f, 0.45f, 0.0f);
        glVertex2f(lx + 0.65f, -0.58f);
        glVertex2f(lx + 0.52f, -1.14f);
        glVertex2f(lx + 0.08f, -1.14f);
        glVertex2f(lx - 0.05f, -0.58f);
        glEnd();

        glDisable(GL_BLEND);
    }

    // i. garis polkadot finish
    for (float fy = -1.16f; fy < 0.02f; fy += 0.14f) {
        // Kolom 1
        glColor3f(0.08f, 0.08f, 0.10f); glRectf(FINISH_LINE_X, fy, FINISH_LINE_X + 0.07f, fy + 0.07f);
        glColor3f(0.96f, 0.96f, 0.98f); glRectf(FINISH_LINE_X, fy + 0.07f, FINISH_LINE_X + 0.07f, fy + 0.14f);
        // Kolom 2
        glColor3f(0.96f, 0.96f, 0.98f); glRectf(FINISH_LINE_X + 0.07f, fy, FINISH_LINE_X + 0.14f, fy + 0.07f);
        glColor3f(0.08f, 0.08f, 0.10f); glRectf(FINISH_LINE_X + 0.07f, fy + 0.07f, FINISH_LINE_X + 0.14f, fy + 0.14f);
    }

    // j. ini tiang finish
    // Tiang kiri
    glColor3f(0.22f, 0.24f, 0.28f);
    glRectf(0.84f, 0.02f, 0.88f, 0.60f);
    // Tiang Kanan
    glRectf(1.22f, 0.02f, 1.26f, 0.60f);

    // palang atas
    glColor3f(0.15f, 0.16f, 0.20f);
    glRectf(0.82f, 0.58f, 1.28f, 0.62f);

    // ini buat tulisan FINISH
    glColor3f(0.06f, 0.06f, 0.10f);
    glRectf(0.82f, 0.35f, 1.28f, 0.58f);

    // ini untuk palang warna kuning itam selang seling
    for (float cx = 0.82f; cx < 1.28f; cx += 0.046f) {
        if (int((cx - 0.80f) * 22.0f) % 2 == 0)
            glColor3f(1.0f, 0.85f, 0.0f); // Kuning 
        else
            glColor3f(0.1f, 0.1f, 0.15f); // Gelap
        glRectf(cx, 0.55f, cx + 0.046f, 0.58f); // Strip 
        glRectf(cx, 0.35f, cx + 0.046f, 0.38f); // Strip 
    }

    // Border kuning kilat
    glColor3f(1.0f, 0.88f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0.82f, 0.35f);
    glVertex2f(1.28f, 0.35f);
    glVertex2f(1.28f, 0.58f);
    glVertex2f(0.82f, 0.58f);
    glEnd();

    // TEKS "FINISH" 
    glColor3f(1.0f, 0.92f, 0.05f); 
    drawBoldStrokeText(1.05f, 0.42f, "FINISH", 0.00065f, 4.5f, true, 6);

    // karna papan finishnya terang x, kita buat cahaya menyala ke bawah juga
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_POLYGON);
    glColor4f(1.0f, 0.92f, 0.5f, 0.25f);
    glVertex2f(0.95f, 0.35f);
    glVertex2f(1.15f, 0.35f);
    glColor4f(1.0f, 0.85f, 0.3f, 0.0f);
    glVertex2f(1.25f, -0.10f);
    glVertex2f(0.85f, -0.10f);
    glEnd();
    glDisable(GL_BLEND);
}

// 2. ini untuk mobil dan detailing mobil
void drawCar(const Car& car, const std::string& label, float labelR, float labelG, float labelB) {
    float x = car.x;
    float y = car.y;
    float w = car.width;

    // a. bayangan di bawah mobil
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.01f, 0.01f, 0.03f, 0.55f);
    glBegin(GL_POLYGON);
    glVertex2f(x - 0.03f, y - 0.06f);
    glVertex2f(x + w + 0.03f, y - 0.06f);
    glVertex2f(x + w + 0.06f, y - 0.01f);
    glVertex2f(x - 0.05f, y - 0.01f);
    glEnd();

    // b. ini lampu neon di bawah mobil
    glColor4f(car.r, car.g, car.b, 0.38f);
    glBegin(GL_POLYGON);
    glVertex2f(x + 0.06f, y - 0.05f);
    glVertex2f(x + w - 0.06f, y - 0.05f);
    glVertex2f(x + w - 0.02f, y - 0.01f);
    glVertex2f(x + 0.02f, y - 0.01f);
    glEnd();
    glDisable(GL_BLEND);

    // c. ini lampu depan mobil yang silau
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Sinar Utama 
    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 0.98f, 0.80f, 0.40f); // Titik lampu terang
    glVertex2f(x + w, y + 0.035f);
    glColor4f(1.0f, 0.95f, 0.65f, 0.0f);  // Pudar ke kanan atas
    glVertex2f(x + w + 0.65f, y + 0.12f);
    glColor4f(1.0f, 0.90f, 0.50f, 0.0f);  // Pudar ke kanan bawah aspal
    glVertex2f(x + w + 0.75f, y - 0.08f);
    glEnd();

    // yang dekat mulut mobil putih" segitiga
    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 1.0f, 0.92f, 0.65f);
    glVertex2f(x + w, y + 0.035f);
    glColor4f(1.0f, 0.98f, 0.70f, 0.0f);
    glVertex2f(x + w + 0.30f, y + 0.07f);
    glColor4f(1.0f, 0.92f, 0.55f, 0.0f);
    glVertex2f(x + w + 0.35f, y - 0.03f);
    glEnd();

    // cahaya lampu belakang
    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 0.05f, 0.10f, 0.35f);
    glVertex2f(x, y + 0.055f);
    glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
    glVertex2f(x - 0.16f, y + 0.09f);
    glVertex2f(x - 0.16f, y + 0.02f);
    glEnd();
    glDisable(GL_BLEND);

    // d. roda mobil
    // Roda Belakang
    glColor3f(0.06f, 0.06f, 0.08f);
    drawCircle(x + 0.09f, y - 0.01f, 0.048f, 16);
    glColor3f(0.82f, 0.85f, 0.90f);
    drawCircle(x + 0.09f, y - 0.01f, 0.027f, 12);
    glColor3f(0.20f, 0.22f, 0.28f);
    drawCircle(x + 0.09f, y - 0.01f, 0.011f, 8);

    // Roda Depan
    glColor3f(0.06f, 0.06f, 0.08f);
    drawCircle(x + w - 0.09f, y - 0.01f, 0.048f, 16);
    glColor3f(0.82f, 0.85f, 0.90f);
    drawCircle(x + w - 0.09f, y - 0.01f, 0.027f, 12);
    glColor3f(0.20f, 0.22f, 0.28f);
    drawCircle(x + w - 0.09f, y - 0.01f, 0.011f, 8);

    // e. body mobil biar ganteng
    glColor3f(car.r, car.g, car.b);
    glBegin(GL_POLYGON);
    glVertex2f(x, y);                               // Bawah belakang
    glVertex2f(x + w, y);                           // Bawah depan 
    glVertex2f(x + w + 0.015f, y + 0.028f);         // Moncong bumper depan
    glVertex2f(x + w - 0.025f, y + 0.068f);         // Ujung kap mesin melandai
    glVertex2f(x + 0.29f, y + 0.078f);              // Pangkal kaca depan
    glVertex2f(x + 0.08f, y + 0.078f);              // Pangkal bagasi belakang
    glVertex2f(x, y + 0.065f);                      // Ujung bagasi belakang
    glEnd();

    // f. bagian atas mobil
    // Rangka Atap
    glColor3f(car.rDark, car.gDark, car.bDark);
    glBegin(GL_POLYGON);
    glVertex2f(x + 0.085f, y + 0.078f);
    glVertex2f(x + 0.130f, y + 0.145f);             // Kaca belakang miring
    glVertex2f(x + 0.235f, y + 0.145f);             // Garis atap atas
    glVertex2f(x + 0.295f, y + 0.078f);             // Kaca depan miring
    glEnd();

    // Kaca Samping Gelap Mewah 
    glColor3f(0.10f, 0.18f, 0.28f);
    glBegin(GL_POLYGON);
    glVertex2f(x + 0.105f, y + 0.082f);
    glVertex2f(x + 0.138f, y + 0.136f);
    glVertex2f(x + 0.228f, y + 0.136f);
    glVertex2f(x + 0.278f, y + 0.082f);
    glEnd();

    // Pilar Tengah Kaca 
    glColor3f(car.rDark, car.gDark, car.bDark);
    glRectf(x + 0.180f, y + 0.082f, x + 0.196f, y + 0.136f);

    // Pantulan Cahaya di Kaca 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 0.40f);
    glBegin(GL_POLYGON);
    glVertex2f(x + 0.145f, y + 0.085f);
    glVertex2f(x + 0.165f, y + 0.132f);
    glVertex2f(x + 0.185f, y + 0.132f);
    glVertex2f(x + 0.165f, y + 0.085f);
    glEnd();
    glDisable(GL_BLEND);

    // g. detailing badan mobil
    glColor3f(1.0f, 1.0f, 1.0f); // Putih kontras tajam
    glRectf(x + 0.04f, y + 0.032f, x + w - 0.04f, y + 0.046f);

    // Handle Pintu
    glColor3f(car.rDark * 0.6f, car.gDark * 0.6f, car.bDark * 0.6f);
    glRectf(x + 0.15f, y + 0.052f, x + 0.18f, y + 0.059f);

    // h. spoiler mobil
    // Tiang Spoiler
    glColor3f(0.12f, 0.12f, 0.16f);
    glRectf(x + 0.015f, y + 0.065f, x + 0.030f, y + 0.115f);
    // Bilah Sayap Aerofoil
    glColor3f(car.rDark, car.gDark, car.bDark);
    glBegin(GL_POLYGON);
    glVertex2f(x - 0.025f, y + 0.105f);
    glVertex2f(x + 0.055f, y + 0.105f);
    glVertex2f(x + 0.065f, y + 0.125f);
    glVertex2f(x - 0.015f, y + 0.125f);
    glEnd();

    // i. bola lampu depan dan belakang
    // Lampu Depan LED
    glColor3f(1.0f, 1.0f, 0.88f);
    glBegin(GL_POLYGON);
    glVertex2f(x + w - 0.020f, y + 0.028f);
    glVertex2f(x + w + 0.012f, y + 0.028f);
    glVertex2f(x + w - 0.010f, y + 0.052f);
    glVertex2f(x + w - 0.030f, y + 0.052f);
    glEnd();

    // Lampu Belakang LED Merah
    glColor3f(1.0f, 0.10f, 0.10f);
    glRectf(x - 0.005f, y + 0.046f, x + 0.012f, y + 0.066f);

    // j. label nama pemain 
    glColor3f(labelR, labelG, labelB);
    drawBoldStrokeText(x + w / 2.0f, y + 0.18f, label, 0.00030f, 2.2f, true, 3);
}

// Mencegah mobil keluar dari batas lintasan balapan
void keepInsideBounds(Car& car) {
    if (car.x < X_MIN) car.x = X_MIN;
    if (car.y < -1.10f) car.y = -1.10f; // Batas bawah aspal jalan raya
    if (car.y > -0.22f) car.y = -0.22f; // Batas atas aspal jalan raya
}

// Logika Pembaruan Posisi Mobil & Status Game
void updateGame() {
    if (currentState != PLAYING) return;

    // Kontrol Mobil 1: Tombol Panah Keyboard
    if (specialKeys[GLUT_KEY_RIGHT]) car1.x += car1.speed;
    if (specialKeys[GLUT_KEY_LEFT])  car1.x -= car1.speed;
    if (specialKeys[GLUT_KEY_UP])    car1.y += car1.speed;
    if (specialKeys[GLUT_KEY_DOWN])  car1.y -= car1.speed;

    // Kontrol Mobil 2: Tombol ASDW Keyboard (W: atas/maju, S: bawah/mundur, A: kiri, D: kanan)
    if (keys['d'] || keys['D']) car2.x += car2.speed;
    if (keys['a'] || keys['A']) car2.x -= car2.speed;
    if (keys['w'] || keys['W']) car2.y += car2.speed;
    if (keys['s'] || keys['S']) car2.y -= car2.speed;

    keepInsideBounds(car1);
    keepInsideBounds(car2);

    // Deteksi Kemenangan (Ujung depan mobil menyentuh garis finish)
    if (car1.x + car1.width >= FINISH_LINE_X) {
        winnerText = "Winner goes to Mobil 1!";
        currentState = GAME_OVER;
    }
    else if (car2.x + car2.width >= FINISH_LINE_X) {
        winnerText = "Winner goes to Mobil 2!";
        currentState = GAME_OVER;
    }

    glutPostRedisplay();
}

// Fungsi Display Utama
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Gambar Latar Belakang (Langit malam, gedung pencakar langit, lampu jalan, jalan raya, garis finish)
    drawBackground();

    // Gambar Kedua Objek Mobil Balap
    drawCar(car1, "Mobil 1 (Panah)", 0.30f, 0.75f, 1.0f);
    drawCar(car2, "Mobil 2 (WASD)", 1.00f, 0.35f, 0.65f);

    // Tampilan Saat Pemenang Diumumkan 
    if (currentState == GAME_OVER) {
        // Kotak Overlay Transparan Gelap 
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.04f, 0.04f, 0.09f, 0.92f);
        glRectf(-0.85f, -0.28f, 0.85f, 0.28f);

        // Border Kotak kuning
        glColor4f(1.0f, 0.84f, 0.0f, 1.0f);
        glLineWidth(4.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.85f, -0.28f);
        glVertex2f( 0.85f, -0.28f);
        glVertex2f( 0.85f,  0.28f);
        glVertex2f(-0.85f,  0.28f);
        glEnd();

        // Garis Pembatas Horizontal Tipis antara Heading 1 dan Heading 2
        glColor4f(1.0f, 0.84f, 0.0f, 0.50f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(-0.65f, -0.04f);
        glVertex2f( 0.65f, -0.04f);
        glEnd();
        glDisable(GL_BLEND);

        // --- BARIS 1: HEADING 1 ---
        glColor3f(1.0f, 0.88f, 0.15f);
        drawBoldStrokeText(0.0f, 0.07f, winnerText, 0.00072f, 4.5f, true, 6);

        // --- BARIS 2: HEADING 2 ---
        glColor3f(0.92f, 0.95f, 1.0f);
        drawBoldStrokeText(0.0f, -0.16f, "Tekan 'R' untuk Restart", 0.00038f, 2.6f, true, 4);
    }

    glutSwapBuffers();
}

// Callback Input Keyboard Biasa (WASD & Restart 'R')
void keyboardDown(unsigned char key, int x, int y) {
    keys[key] = true;
    if (key == 'r' || key == 'R') {
        resetGame();
    }
}
void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

// Callback Input Tombol Panah Khusus (Mobil 1)
void specialDown(int key, int x, int y) {
    specialKeys[key] = true;
}
void specialUp(int key, int x, int y) {
    specialKeys[key] = false;
}

// Callback Reshape untuk Menjaga Resolusi & Proporsi Saat Full Screen / Maximize
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(X_MIN, X_MAX, Y_MIN, Y_MAX);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Timer Loop 60 FPS
void timer(int value) {
    updateGame();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(960, 540);
    glutInitWindowPosition(80, 80);
    glutCreateWindow("UTS Grafika Komputer - 2D Night City Racing");

    glClearColor(0.04f, 0.03f, 0.09f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(X_MIN, X_MAX, Y_MIN, Y_MAX); // Setup Proyeksi Ortho 2D Sesuai GeoGebra

    glutDisplayFunc(display);
    glutReshapeFunc(reshape); // Handler otomatis saat jendela di-maximize / full screen
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
