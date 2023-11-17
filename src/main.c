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
static inline void hann_window(float *in, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        float multiplier = 0.5f * (1.0f - cosf(2.0f * PI * i / (n - 1)));
        in[i] *= multiplier;
    }
}

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

static void fft_push(float *buffer, unsigned int frames)
{
    track->frame_count = frames;
    memcpy(track->in_raw, buffer, track->frame_count * sizeof(float));

    hann_window(track->in_raw, track->frame_count);
    fft(track->in_raw, 1, track->out_raw, N);
}

static void callback(void *buffer_data, unsigned int frames)
{   
    float *buffer = (float*)buffer_data;
    fft_push(buffer, frames);
}

void fft_render(void)
{
    int w = GetRenderWidth();
    int h = GetRenderHeight();

    float scale = 2.0f;
    float cell_width = (float)w / track->frame_count;

    for (size_t i = 0; i < track->frame_count - 1; i++) {
        complex float data1 = track->out_raw[i];
        complex float data2 = track->out_raw[i + 1];

        float mag1 = cabs(data1);
        float mag2 = cabs(data2);
        float amp1 = amplitude(data1);
        float amp2 = amplitude(data2);

        float hue = (float)i / (float)track->frame_count;
        Color color = ColorFromHSV(hue * 360, 0.75, 1.0);

        Vector2 start = {i * cell_width, h / 2 - mag1 * amp1/scale};
        Vector2 end = {(i + 1) * cell_width, h / 2 - mag2 * amp2/scale};

        DrawLineEx(start, end, cell_width, color);
    }
}

static void fft_clean(void)
{
    memset(track->in_raw, 0, sizeof(track->in_raw));
    memset(track->out_raw, 0, sizeof(track->out_raw));
}

int main(void) {
    track = malloc(sizeof(Track_r));
    SetConfigFlags(FLAG_MSAA_4X_HINT); 
   
    InitWindow(1200, 800, "beatbox");
    SetTargetFPS(60);

    InitAudioDevice();
    Music sound = LoadMusicStream("res/Beatiful Love.mp3");
    assert(sound.stream.sampleSize == 32);
    assert(sound.stream.channels == 2);

    PlayMusicStream(sound);
    SetMusicVolume(sound, 1.0f);
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
