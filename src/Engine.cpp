﻿#include "Engine.h"

#ifdef _WIN32
#define NOMINMAX
#if defined(_MSC_VER) && !defined(__clang__)
#include <windows.h>
#pragma comment(lib, "user32.lib")
#endif
#endif
#include <cmath>
#include "Font.h"
#include "libavutil/pixfmt.h"

Engine Engine::engine_;

//std::atomic<int> Engine::working_;

Engine::Engine()
{
    //working_ = 1;
}

Engine::~Engine()
{
    //destroy();
}

void Engine::destroyTexture(BP_Texture* t)
{
    if (t) { SDL_DestroyTexture(t); }
}

void Engine::updateYUVTexture(BP_Texture* t, uint8_t* data0, int size0, uint8_t* data1, int size1, uint8_t* data2, int size2)
{
    SDL_UpdateYUVTexture(testTexture(t), nullptr, data0, size0, data1, size1, data2, size2);
}

BP_Texture* Engine::createTexture(int pix_fmt, int w, int h)
{
    return SDL_CreateTexture(renderer_, pix_fmt, SDL_TEXTUREACCESS_STREAMING, w, h);
}

void Engine::updateRGBATexture(BP_Texture* t, uint8_t* buffer, int pitch)
{
    SDL_UpdateTexture(testTexture(t), nullptr, buffer, pitch);
}

void Engine::renderCopy(BP_Texture* t, int x, int y, int w, int h, int inPresent)
{
    if (inPresent == 1)
    {
        x += rect_.x;
        y += rect_.y;
    }
    SDL_Rect r = { x, y, w, h };
    SDL_RenderCopy(renderer_, t, nullptr, &r);
}

void Engine::renderCopy(BP_Texture* t /*= nullptr*/)
{
    SDL_RenderCopyEx(renderer_, testTexture(t), nullptr, &rect_, rotation_, nullptr, SDL_FLIP_NONE);
}

void Engine::destroy()
{
    SDL_DestroyTexture(tex_);
    if (renderer_self_) { SDL_DestroyRenderer(renderer_); }
    if (window_mode_ == 0) { SDL_DestroyWindow(window_); }
}

void Engine::mixAudio(Uint8* dst, const Uint8* src, Uint32 len, int volume)
{
    SDL_MixAudioFormat(dst, src, audio_format_, len, volume);
}

int Engine::openAudio(int& freq, int& channels, int& size, int minsize, AudioCallback f)
{
    SDL_AudioSpec want;
    SDL_zero(want);

    printf("\naudio freq/channels: stream %d/%d, ", freq, channels);
    if (channels <= 2) { channels = 2; }
    want.freq = freq;
    want.format = AUDIO_S16;
    want.channels = channels;
    want.samples = size;
    want.callback = mixAudioCallback;
    //want.userdata = this;
    want.silence = 0;

    audio_callback_ = f;
    //if (useMap())
    {
        want.samples = std::max(size, minsize);
    }

    audio_device_ = 0;
    int i = 10;
    while (audio_device_ == 0 && i > 0)
    {
        audio_device_ = SDL_OpenAudioDevice(NULL, 0, &want, &audio_spec_, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
        want.channels--;
        i--;
    }
    printf("device %d/%d\n", audio_spec_.freq, audio_spec_.channels);

    audio_format_ = audio_spec_.format;

    if (audio_device_)
    {
        SDL_PauseAudioDevice(audio_device_, 0);
    }
    else
    {
        printf("failed to open audio: %s\n", SDL_GetError());
    }

    freq = audio_spec_.freq;
    channels = audio_spec_.channels;

    return 0;
}

void Engine::mixAudioCallback(void* userdata, Uint8* stream, int len)
{
    SDL_memset(stream, 0, len);
    if (engine_.audio_callback_)
    {
        engine_.audio_callback_(stream, len);
    }
}

BP_Texture* Engine::createSquareTexture(int size)
{
    int d = size;
    auto square_s = SDL_CreateRGBSurface(0, d, d, 32, RMASK, GMASK, BMASK, AMASK);
    SDL_FillRect(square_s, nullptr, 0xffffffff);
    auto square = SDL_CreateTextureFromSurface(renderer_, square_s);
    SDL_SetTextureBlendMode(square, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(square, 128);
    SDL_FreeSurface(square_s);
    return square;
}

BP_Texture* Engine::createBallTexture(int size)
{
    int d = size;
    auto ball_s = SDL_CreateRGBSurface(0, d, d, 32, RMASK, GMASK, BMASK, AMASK);
    SDL_FillRect(ball_s, nullptr, 0);
    SDL_Rect r = { 0, 0, 1, 1 };
    auto& x = r.x;
    auto& y = r.y;
    double c = (d - 1) / 2.0;
    for (x = 0; x < d; x++)
    {
        for (y = 0; y < d; y++)
        {
            double ra = sqrt((x - c) * (x - c) + (y - c) * (y - c)) / c;
            double a0 = 0;
            if (ra > 1.05)
            {
                a0 = 255;
                //a0 = (ra - 1) * 255 * 4;
            }
            if (ra < 1)
            {
                a0 = (1 - ra) * 255 * 2;
            }
            uint8_t a = a0 > 255 ? 255 : a0;
            SDL_FillRect(ball_s, &r, SDL_MapRGBA(ball_s->format, 255, 255, 255, a));
        }
    }
    auto ball = SDL_CreateTextureFromSurface(renderer_, ball_s);
    SDL_SetTextureBlendMode(ball, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(ball, 128);
    SDL_FreeSurface(ball_s);
    return ball;
}

//注意：当字符串为空时，也会返回一个空字符串
std::vector<std::string> Engine::splitString(const std::string& s, const std::string& delim)
{
    std::vector<std::string> ret;
    size_t last = 0;
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos)
    {
        ret.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0)
    {
        ret.push_back(s.substr(last, index - last));
    }
    return ret;
}

void Engine::drawSubtitle(const std::string& fontname, const std::string& text, int size, int x, int y, uint8_t alpha, int align)
{
    if (alpha == 0)
    {
        return;
    }
    auto font = TTF_OpenFont(fontname.c_str(), size);
    if (!font) { return; }
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color colorb = { 0, 0, 0, 255 };
    auto ret = splitString(text, "\n");
    for (int i = 0; i < ret.size(); i++)
    {
        if (ret[i] == "")
        {
            continue;
        }
        TTF_SetFontOutline(font, 2);
        auto text_sb = TTF_RenderUTF8_Blended(font, ret[i].c_str(), colorb);
        TTF_SetFontOutline(font, 0);
        auto text_s = TTF_RenderUTF8_Blended(font, ret[i].c_str(), color);
        //SDL_SetTextureAlphaMod(text_t, alpha);
        SDL_Rect rectb = { 2, 2, 0, 0 };
        SDL_BlitSurface(text_s, NULL, text_sb, &rectb);

        auto text_t = SDL_CreateTextureFromSurface(renderer_, text_sb);

        SDL_FreeSurface(text_s);
        SDL_FreeSurface(text_sb);

        SDL_Rect rect;
        SDL_QueryTexture(text_t, nullptr, nullptr, &rect.w, &rect.h);
        rect.y = y + i * (size + 2);

        switch (align)
        {
        case BP_ALIGN_LEFT:
            rect.x = x;
            break;
        case BP_ALIGN_RIGHT:
            rect.x = x - rect.w;
            break;
        case BP_ALIGN_MIDDLE:
            rect.x = x - rect.w / 2;
            break;
        }

        SDL_RenderCopy(renderer_, text_t, nullptr, &rect);
        SDL_DestroyTexture(text_t);
    }
    TTF_CloseFont(font);
}

BP_Texture* Engine::createTextTexture(const std::string& fontname, const std::string& text, int size, BP_Color c)
{
    auto font = TTF_OpenFont(fontname.c_str(), size);
    if (!font) { return nullptr; }
    //SDL_Color c = { 255, 255, 255, 128 };
    auto text_s = TTF_RenderUTF8_Blended(font, text.c_str(), c);
    auto text_t = SDL_CreateTextureFromSurface(renderer_, text_s);
    SDL_FreeSurface(text_s);
    TTF_CloseFont(font);
    return text_t;
}

void Engine::drawText(const std::string& fontname, const std::string& text, int size, int x, int y, uint8_t alpha, int align)
{
    if (alpha <= 0 || size <= 0)
    {
        return;
    }

    int w = Font::getInstance()->getTextWidth(fontname, text, size);
    switch (align)
    {
    case BP_ALIGN_LEFT:
        break;
    case BP_ALIGN_RIGHT:
        x = x - w;
        break;
    case BP_ALIGN_MIDDLE:
        x = x - w / 2;
        break;
    }
    Font::getInstance()->draw(fontname, text, size, x, y, { 255, 255, 255, 255 }, alpha);
}

int Engine::init(void* handle /*= nullptr*/, int handle_type /*= 0*/)
{
    if (inited_) { return 0; }
    inited_ = true;
#ifndef _WINDLL
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        return -1;
    }
#endif
    window_mode_ = handle_type;
    if (handle)
    {
        if (handle_type == 0)
        {
            window_ = SDL_CreateWindowFrom(handle);
        }
        else
        {
            window_ = (BP_Window*)handle;
        }
    }
    else
    {
        window_ = SDL_CreateWindow("PotPlayer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, start_w_, start_h_, SDL_WINDOW_RESIZABLE);
    }
    //SDL_CreateWindowFrom()
#ifndef _WINDLL
    SDL_ShowWindow(window_);
    SDL_RaiseWindow(window_);
#endif
    renderer_ = SDL_GetRenderer(window_);
    printf("%s\n", SDL_GetError());
    if (renderer_ == nullptr)
    {
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE/*| SDL_RENDERER_PRESENTVSYNC*/);
        renderer_self_ = true;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    rect_ = { 0, 0, start_w_, start_h_ };
    logo_ = loadImage("logo.png");
    showLogo();
    renderPresent();
    TTF_Init();

#if defined(_MSC_VER) && !defined(__clang__)
    RECT r;
    SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&r, 0);
    int w = GetSystemMetrics(SM_CXEDGE);
    int h = GetSystemMetrics(SM_CYEDGE);
    min_x_ = r.left + w;
    min_y_ = r.top + h + GetSystemMetrics(SM_CYCAPTION);
    max_x_ = r.right - w;
    max_y_ = r.bottom - h;
#else
    SDL_Rect r;
    SDL_GetDisplayBounds(0, &r);
    min_x_ = r.x;
    min_y_ = r.y;
    max_x_ = r.w + r.x;
    max_y_ = r.h + r.y;
#endif
    printf("maximum width and height are: %d, %d\n", max_x_, max_y_);
    return 0;
}

int Engine::getWindowsWidth()
{
    int w;
    SDL_GetWindowSize(window_, &w, nullptr);
    return w;
}

int Engine::getWindowsHeight()
{
    int h;
    SDL_GetWindowSize(window_, nullptr, &h);
    return h;
}

bool Engine::isFullScreen()
{
    Uint32 state = SDL_GetWindowFlags(window_);
    full_screen_ = (state & SDL_WINDOW_FULLSCREEN) || (state & SDL_WINDOW_FULLSCREEN_DESKTOP);
    return full_screen_;
}

void Engine::toggleFullscreen()
{
    full_screen_ = !full_screen_;
    if (full_screen_)
    {
        SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else
    {
        SDL_SetWindowFullscreen(window_, 0);
    }
    SDL_RenderClear(renderer_);
}

BP_Texture* Engine::loadImage(const std::string& filename)
{
    return IMG_LoadTexture(renderer_, filename.c_str());
}

bool Engine::setKeepRatio(bool b)
{
    return keep_ratio_ = b;
}

void Engine::createMainTexture(int pix_fmt, int w, int h)
{
    tex_ = createTexture(pix_fmt, w, h);
    //_tex2 = createRGBATexture(w, h);
    setPresentPosition();
}

void Engine::setPresentPosition()
{
    if (!tex_)
    {
        return;
    }
    int w_dst = 0, h_dst = 0;
    int w_src = 0, h_src = 0;
    getWindowSize(w_dst, h_dst);
    SDL_QueryTexture(tex_, nullptr, nullptr, &w_src, &h_src);
    w_src *= ratio_x_;
    h_src *= ratio_y_;
    if (keep_ratio_)
    {
        if (w_src == 0 || h_src == 0) { return; }
        double ratio = std::min(1.0 * w_dst / w_src, 1.0 * h_dst / h_src);
        if (rotation_ == 90 || rotation_ == 270)
        {
            ratio = std::min(1.0 * w_dst / h_src, 1.0 * h_dst / w_src);
        }
        rect_.x = (w_dst - w_src * ratio) / 2;
        rect_.y = (h_dst - h_src * ratio) / 2;
        rect_.w = w_src * ratio;
        rect_.h = h_src * ratio;
    }
    else
    {
        //unfinshed
        rect_.x = 0;
        rect_.y = 0;
        rect_.w = w_dst;
        rect_.h = h_dst;
        if (rotation_ == 90 || rotation_ == 270)
        {
            rect_.x = (h_dst - w_dst) / 2;
            rect_.y = (w_dst - h_dst) / 2;
            rect_.w = h_dst;
            rect_.h = w_dst;
        }
    }
}

BP_Texture* Engine::transBitmapToTexture(const uint8_t* src, uint32_t color, int w, int h, int stride)
{
    auto s = SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
    SDL_FillRect(s, nullptr, color);
    auto p = (uint8_t*)s->pixels;
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            p[4 * (y * w + x)] = src[y * stride + x];
        }
    }
    auto t = SDL_CreateTextureFromSurface(renderer_, s);
    SDL_FreeSurface(s);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(t, 192);
    return t;
}

int Engine::showMessage(const std::string& content)
{
    const SDL_MessageBoxButtonData buttons[] =
    {
        { /* .flags, .buttonid, .text */        0, 0, "no" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "yes" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "cancel" },
    };
    const SDL_MessageBoxColorScheme colorScheme =
    {
        { /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 255,   0,   0 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            {   0, 255,   0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 255, 255,   0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            {   0,   0, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 255,   0, 255 }
        }
    };
    const SDL_MessageBoxData messageboxdata =
    {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        NULL, /* .window */
        "Pot Player", /* .title */
        content.c_str(), /* .message */
        SDL_arraysize(buttons), /* .numbuttons */
        buttons, /* .buttons */
        &colorScheme /* .colorScheme */
    };
    int buttonid;
    SDL_ShowMessageBox(&messageboxdata, &buttonid);
    return buttonid;
}

void Engine::setWindowSize(int w, int h)
{
    if (rotation_ == 90 || rotation_ == 270)
    {
        std::swap(w, h);
    }
    if (w <= 0 || h <= 0) { return; }
    win_w_ = std::min(max_x_ - min_x_, w);
    win_h_ = std::min(max_y_ - min_y_, h);
    double ratio;
    ratio = std::min(1.0 * win_w_ / w, 1.0 * win_h_ / h);
    win_w_ = w * ratio;
    win_h_ = h * ratio;
    if (!window_) { return; }

    SDL_SetWindowSize(window_, win_w_, win_h_);
    setPresentPosition();

    SDL_ShowWindow(window_);
    SDL_RaiseWindow(window_);
    SDL_GetWindowSize(window_, &win_w_, &win_h_);
    //resetWindowsPosition();
    //renderPresent();
}

void Engine::resetWindowsPosition()
{
    int x, y, w, h, x0, y0;
    SDL_GetWindowSize(window_, &w, &h);
    SDL_GetWindowPosition(window_, &x0, &y0);
    x = std::max(min_x_, x0);
    y = std::max(min_y_, y0);
    if (x + w > max_x_) { x = std::min(x, max_x_ - w); }
    if (y + h > max_y_) { y = std::min(y, max_y_ - h); }
    if (x != x0 || y != y0)
    {
        SDL_SetWindowPosition(window_, x, y);
    }
}

void Engine::setColor(BP_Texture* tex, BP_Color c, uint8_t alpha)
{
    SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(tex, alpha);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
}

void Engine::setWindowPosition(int x, int y)
{
    int w, h;
    SDL_GetWindowSize(window_, &w, &h);
    if (x == BP_WINDOWPOS_CENTERED) { x = min_x_ + (max_x_ - min_x_ - w) / 2; }
    if (y == BP_WINDOWPOS_CENTERED) { y = min_y_ + (max_y_ - min_y_ - h) / 2; }
    SDL_SetWindowPosition(window_, x, y);
}


