#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <pspkernel.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

PSP_MODULE_INFO("RetroGallery", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define W 480
#define H 272
#define MAX_IMG 200
#define DIR_PATH "ms0:/PICTURE/"

/* tombol PSP di SDL */
#define BTN_L 4
#define BTN_R 5
#define BTN_LEFT 7
#define BTN_RIGHT 9
#define BTN_START 11

static char files[MAX_IMG][256];
static int count = 0;

static int is_image(const char *n) {
    const char *e = strrchr(n, '.');
    if (!e) return 0;
    return !strcasecmp(e, ".png") || !strcasecmp(e, ".jpg") || !strcasecmp(e, ".jpeg");
}

static void scan(void) {
    DIR *d = opendir(DIR_PATH);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) && count < MAX_IMG)
        if (is_image(e->d_name)) snprintf(files[count++], 256, "%s%s", DIR_PATH, e->d_name);
    closedir(d);
}

static void rect(SDL_Renderer *r, int x, int y, int w, int h, int R, int G, int B) {
    SDL_Rect q = {x, y, w, h};
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_RenderFillRect(r, &q);
}

/* bingkai 3D ala Windows 95 */
static void bevel(SDL_Renderer *r, int x, int y, int w, int h) {
    rect(r, x, y, w, h, 192, 192, 192);
    rect(r, x, y, w, 2, 255, 255, 255);
    rect(r, x, y, 2, h, 255, 255, 255);
    rect(r, x, y + h - 2, w, 2, 64, 64, 64);
    rect(r, x + w - 2, y, 2, h, 64, 64, 64);
}

static void draw(SDL_Renderer *r, SDL_Texture *t, int idx) {
    /* latar teal + garis scanline */
    rect(r, 0, 0, W, H, 0, 96, 96);
    for (int y = 0; y < H; y += 4) rect(r, 0, y, W, 1, 0, 80, 80);

    /* jendela */
    bevel(r, 20, 14, 440, 244);
    rect(r, 24, 18, 432, 18, 0, 0, 128);          /* title bar biru */
    rect(r, 436, 21, 14, 12, 192, 192, 192);      /* kotak tombol */

    /* area gambar (inset) */
    rect(r, 30, 44, 420, 190, 64, 64, 64);
    rect(r, 32, 46, 416, 186, 0, 0, 0);

    if (t) {
        int tw, th;
        SDL_QueryTexture(t, NULL, NULL, &tw, &th);
        float s = 412.0f / tw;
        if (th * s > 182) s = 182.0f / th;
        int dw = tw * s, dh = th * s;
        SDL_Rect dst = {32 + (416 - dw) / 2, 46 + (186 - dh) / 2, dw, dh};
        SDL_RenderCopy(r, t, NULL, &dst);
    } else {
        rect(r, 200, 120, 80, 8, 255, 0, 0); /* penanda: tidak ada gambar */
    }

    /* progress bar (pengganti teks) */
    rect(r, 30, 240, 420, 12, 128, 128, 128);
    if (count > 0) rect(r, 32, 242, (416 * (idx + 1)) / count, 8, 0, 0, 128);

    SDL_RenderPresent(r);
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    SDL_Window *win = SDL_CreateWindow("Gallery", 0, 0, W, H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);
    SDL_JoystickOpen(0);

    scan();
    int idx = 0, run = 1, dirty = 1;
    SDL_Texture *tex = NULL;

    while (run) {
        if (dirty) {
            if (tex) SDL_DestroyTexture(tex);
            tex = count ? IMG_LoadTexture(ren, files[idx]) : NULL;
            draw(ren, tex, idx);
            dirty = 0;
        }
        SDL_Event e;
        if (SDL_WaitEvent(&e)) {
            if (e.type == SDL_QUIT) run = 0;
            if (e.type == SDL_JOYBUTTONDOWN && count) {
                int b = e.jbutton.button;
                if (b == BTN_RIGHT || b == BTN_R) { idx = (idx + 1) % count; dirty = 1; }
                if (b == BTN_LEFT || b == BTN_L)  { idx = (idx + count - 1) % count; dirty = 1; }
                if (b == BTN_START) run = 0;
            }
        }
    }
    if (tex) SDL_DestroyTexture(tex);
    IMG_Quit();
    SDL_Quit();
    sceKernelExitGame();
    return 0;
}
