#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

#define cfromreal(re) (re)
#define cfromimag(im) ((im)*I)
#define mulcc(a, b) ((a)*(b))
#define addcc(a, b) ((a)+(b))
#define subcc(a, b) ((a)-(b))

#define N (1<<13)
#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])
#define MAX_FILEPATH_RECORDED   4096
#define MAX_FILEPATH_SIZE       2048
#define SAMPLE_RATE 4800

typedef struct {
    float in_raw[N];
    float in_win[N];
    float complex out_raw[N];
    float out_log[N];
    unsigned int frame_count;
} Track_r;

static Track_r *track = NULL;
static Music sound = {0};

static inline float amplitude(float complex z)
{
    float a = crealf(z);
    float b = cimagf(z);
    return logf(a*a + b*b);
}

// Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#Python
static void fft(float in[], size_t stride, float complex out[], size_t n)
{
    assert(n > 0);

    if (n == 1) {
        out[0] = cfromreal(in[0]);
        return;
    }

    fft(in, stride * 2, out, n/2);
    fft(in + stride, stride * 2, out + n/2, n/2);

    for (size_t k = 0; k < n / 2; ++k) {
        float t = (float)k / n;
        float complex v = mulcc(cexpf(cfromimag(-2*PI*t)), out[k + n/2]);
        float complex e = out[k];
        out[k]       = addcc(e, v);
        out[k + n/2] = subcc(e, v);
    }
}

static size_t fft_analyze(float dt)
{
    // Apply the Hann Window on the Input - https://en.wikipedia.org/wiki/Hann_function
    for (size_t i = 0; i < N; ++i) {
        float t = (float)i/(N - 1);
        float hann = 0.5 - 0.5*cosf(2*PI*t);
        track->in_win[i] = track->in_raw[i]*hann;
    }
    
    // FFT
    fft(track->in_win, 1, track->out_raw, N);

    // "Squash" into the Logarithmic Scale
    float step = 1.06;
    float lowf = 1.0f;
    size_t m = 0;
    float max_amp = 1.0f;
    for (float f = lowf; (size_t) f < N/2; f = ceilf(f*step)) {
        float f1 = ceilf(f*step);
        float a = 0.0f;
        for (size_t q = (size_t) f; q < N/2 && q < (size_t) f1; ++q) {
            float b = amplitude(track->out_raw[q]);
            if (b > a) a = b;
        }
        if (max_amp < a) max_amp = a;
        track->out_log[m++] = a;
    }
    
    return m;
}

static void fft_push(float frame)
{
    memmove(track->in_raw, track->in_raw + 1, (N - 1)*sizeof(track->in_raw[0]));
    track->in_raw[N-1] = frame;
}

static void callback(void *buffer_data, unsigned int frames)
{   
    // https://cdecl.org/?q=float+%28*fs%29%5B2%5D
    float (*fs)[2] = buffer_data;

    for (size_t i = 0; i < frames; ++i) {
        fft_push(fs[i][0]);
    }
}

void fft_render()
{
    int w = GetRenderWidth();
    int h = GetRenderHeight();

    float dt = 1.0f/ (float)SAMPLE_RATE;

    size_t t = fft_analyze(dt);
    float cell_width = (float)w / (float)(t) + 1;
    
    for (size_t i = 0; i < N/2; i++) {
        float value1 = track->out_log[i];
        float value2 = track->out_log[i + 1];

        float hue = (float)i / (float)t;
        Color color = ColorFromHSV(hue * 360, 0.75, 1.0);

        Vector2 start = {(float)i * cell_width, h - value1 * t/2};
        Vector2 end = {(float)(i + 1) * cell_width, h - value2 * t/2};

        DrawLineEx(start, end, 2.0f, color);
    }
}

static void fft_clean(void)
{
    memset(track->in_raw, 0, sizeof(track->in_raw));
    memset(track->out_raw, 0, sizeof(track->out_raw));
    memset(track->in_win, 0, sizeof(track->in_win));
    memset(track->out_log, 0, sizeof(track->out_log));
    
    UnloadMusicStream(sound);
    free(track);
}

static void load_sticker()
{
    
}

int main(void) {
    track = malloc(sizeof(Track_r));
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    
    InitWindow(1200, 800, "beatbox");
    InitAudioDevice();
    SetTargetFPS(60);

    int filePathCounter = 0;
    char *filePaths[MAX_FILEPATH_RECORDED] = {0};
    const char *filename;

    for (int i = 0; i < MAX_FILEPATH_RECORDED; i++) {
        filePaths[i] = (char *)calloc(MAX_FILEPATH_SIZE, 1);
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
        UpdateMusicStream(sound);

        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(sound)) {
                PauseMusicStream(sound);
            } else {
                ResumeMusicStream(sound);
            }
        }
        
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            for (int i = 0, offset = filePathCounter; i < (int)droppedFiles.count; i++) {
                if (filePathCounter < (MAX_FILEPATH_RECORDED - 1)) {
                    TextCopy(filePaths[offset + i], droppedFiles.paths[i]);
                    filePathCounter++;
                }
                filename = filePaths[i];

                sound = LoadMusicStream(filename);
                assert(sound.stream.sampleSize == 32);
                assert(sound.stream.channels == 2);

                PlayMusicStream(sound);
                SetMusicVolume(sound, 1.0f);
                AttachAudioStreamProcessor(sound.stream, callback);
            }
            UnloadDroppedFiles(droppedFiles);
        }

        fft_render();
                        
        ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
        EndDrawing();

    }
    fft_clean();
    CloseWindow();
    return 0;
}
