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

typedef struct {
    float in_raw[N];
    float complex out_raw[N];
    unsigned int frame_count;
} Track_r;

Track_r *track = NULL;

// Hann window (Hanning window) - https://en.wikipedia.org/wiki/Hann_function
static inline void hanning(float data_buffer[])
{
    for (size_t i = 0; i < N; i++) {
        float t = (float)i/(N - 1);
        float hann = 0.5 - 0.5 * cosf(2*PI*t);
        data_buffer[i] *= hann;
    }
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
        out[k] = e + v;
        out[k + n/2] = e - v;
    }
}

static void fft_push(float *buffer, unsigned int frames)
{
    for (unsigned int i = 0; i < frames; i++) {
        track->in_raw[i] = buffer[i];
    }

    fft(track->in_raw, 1, track->out_raw, N);
}

void callback(void *buffer_data, unsigned int frames)
{
    float *buffer = (float*)buffer_data;
    fft_push(buffer, frames);
    track->frame_count = frames;
}

void fft_render(void)
{
    int w = GetRenderWidth();
    int h = GetRenderHeight();

    float scale = 5.0f;
    float cell_width = (float)w/track->frame_count;

    for (size_t i = 0; i < track->frame_count; i++) {
        complex double data = track->out_raw[i];
        double mag = cabs(data) * scale;
        float hue = (float)i/(float)track->frame_count;
        
        Color color = ColorFromHSV(hue * 360, 0.75, 1.0);
        DrawRectangle(i*cell_width + cell_width/2, h/2 - mag, cell_width, mag, color);
    }
}

static void fft_clean(void)
{
    memset(track->in_raw, 0, sizeof(track->in_raw));
    memset(track->out_raw, 0, sizeof(track->out_raw));
}

int main(void) {
    track = malloc(sizeof(Track_r));
   
    InitWindow(1200, 800, "beatbox");
    SetTargetFPS(30);

    InitAudioDevice();
    Music sound = LoadMusicStream("res/""");
    assert(sound.stream.sampleSize == 32);
    assert(sound.stream.channels == 2);

    PlayMusicStream(sound);
    SetMusicVolume(sound, 1.5f);
    AttachAudioStreamProcessor(sound.stream, callback);

    while (!WindowShouldClose()) {
        UpdateMusicStream(sound);
        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(sound)) {
                PauseMusicStream(sound);
            } else {
                ResumeMusicStream(sound);
            }
        }

        BeginDrawing();
        fft_render();
        ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
        EndDrawing();
    }

    fft_clean();
    CloseWindow();

    return 0;
}
