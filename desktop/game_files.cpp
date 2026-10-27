// The game on Linux (nfsmw-linux): its files or a disc image (.iso) in the data folder's game folder, a folder
// NFSMW_GAME_DIR names, or ~/Games/MostWanted.
#include <SDL3/SDL.h>
#include <SDL3/SDL_system.h>

#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>

#include "app/app.h"
#include "core/log.h"
#include "image_symbols.h"
#include "kernel/crypto.h"
#include "kernel/disc.h"

extern const char kImageSha1[];  // the xbe recompiled into this library, written by CMake

namespace app {

namespace {

namespace fs = std::filesystem;

// where the game's files or its .iso are looked for, in order
std::vector<fs::path> AssetFolders() {
    std::vector<fs::path> out;
    if (const char* d = std::getenv("NFSMW_GAME_DIR"); d && *d) out.emplace_back(d);
    out.push_back(ExeDir() / "game");
    out.push_back(ExeDir() / "assets");
    if (const char* h = std::getenv("HOME"); h && *h) {
        out.push_back(fs::path(h) / "Games" / "MostWanted");
        out.push_back(fs::path(h) / "Games" / "mostwanted");
        out.push_back(fs::path(h) / "Games" / "NFSMW");
        out.push_back(fs::path(h) / ".local" / "share" / "nfsmw-linux" / "game");
    }
    return out;
}

std::string Sha1(const std::vector<uint8_t>& data) {
    alignas(8) uint8_t ctx[128];
    uint8_t digest[20];
    crypto::ShaInit(ctx);
    crypto::ShaUpdate(ctx, data.data(), data.size());
    crypto::ShaFinal(ctx, digest);
    std::string hex;
    for (uint8_t b : digest) {
        hex += "0123456789abcdef"[b >> 4];
        hex += "0123456789abcdef"[b & 15];
    }
    return hex;
}

bool ReadHostFile(const fs::path& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    out.assign(std::istreambuf_iterator<char>(f), {});
    return true;
}

std::string Utf8(const wchar_t* w) {
    std::string s;
    for (; w && *w; w++) s += *w < 0x80 ? char(*w) : '?';
    return s;
}

// an image cut short still opens, but checks against size
void CheckImage(const kernel::Disc& disc, const fs::path& iso) {
    std::error_code ec;
    uint64_t size = fs::file_size(iso, ec);
    if (ec) {
        LOGE("game: %s: its size cannot be read: %s", iso.string().c_str(), ec.message().c_str());
        return;
    }
    uint64_t end = 0;
    size_t files = 0, past = 0;
    std::string first;
    std::vector<std::pair<const kernel::Disc::Entry*, std::string>> todo{{disc.Find(std::string_view()), ""}};
    while (!todo.empty()) {
        auto [dir, path] = todo.back();
        todo.pop_back();
        if (!dir) continue;
        for (const kernel::Disc::Entry& e : dir->children) {
            std::string name = path.empty() ? e.name : path + "\\" + e.name;
            if (e.dir) {
                todo.push_back({&e, name});
                continue;
            }
            files++;
            uint64_t file_end = e.offset + e.size;
            end = std::max(end, file_end);
            if (file_end > size && past++ == 0) first = name;
        }
    }
    LOGR("Game: image %llu bytes, %zu files up to byte %llu", (unsigned long long)size, files, (unsigned long long)end);
    if (past)
        LOGE("game: %s is cut short: %zu of its files end past its %llu bytes, %s the first; copy the image again",
             iso.string().c_str(), past, (unsigned long long)size, first.c_str());
}

// no game: player is told where it goes
void NoGame() {
    std::string where;
    for (const fs::path& p : AssetFolders()) where += "\n  " + p.string();
    LOGE("game: no game files (default.xbe with its folders, or the .iso) in any of:%s", where.c_str());
    std::string text = "The game files were not found.\n\nPut the .iso of your own copy of Need for Speed: Most Wanted (Black Edition), or "
                       "default.xbe with the NFS folder, in:\n  " + (ExeDir() / "game").string() +
                       "\n\nthen start the game again.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Need for Speed: Most Wanted", text.c_str(), nullptr);
}

fs::path AllowedFile() { return ExeDir() / "other_releases.txt"; }

bool AllowedRelease(const std::string& sha) {
    std::ifstream f(AllowedFile());
    for (std::string line; std::getline(f, line);)
        if (line == sha) return true;
    return false;
}

void AllowRelease(const std::string& sha) {
    std::ofstream f(AllowedFile(), std::ios::app);
    f << sha << "\n";
    if (!f) LOGW("game: %s cannot be written", AllowedFile().string().c_str());
}

bool AskOtherRelease(const fs::path& name, const std::string& sha) {
    if (AllowedRelease(sha)) {
        LOGR("Game: another release, started without asking as the player chose");
        return true;
    }
    std::string text = name.filename().string() +
                       " is a different release of the game than this build was made from, and will most likely "
                       "crash.\n\nStart it anyway?";
    const SDL_MessageBoxButtonData buttons[] = {
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "No"},
        {0, 1, "Yes"},
        {0, 2, "Yes, don't ask again"},
    };
    std::string title = Utf8(kGame.short_title);
    SDL_MessageBoxData box{SDL_MESSAGEBOX_WARNING, nullptr, title.c_str(), text.c_str(), 3, buttons, nullptr};
    int answer = 0;
    if (!SDL_ShowMessageBox(&box, &answer)) return false;
    if (answer == 2) AllowRelease(sha);
    return answer >= 1;
}

int Search(GameFiles& game) {
    GameFiles other;
    std::unique_ptr<kernel::Disc> other_disc;
    fs::path other_name;
    std::string other_sha;
    auto consider = [&](const fs::path& assets, const fs::path& name, std::vector<uint8_t> xbe,
                        std::unique_ptr<kernel::Disc> disc) {
        std::string sha = Sha1(xbe);
        bool match = sha == kImageSha1;
        LOGR("Game: %s in %s, sha1 %s%s", IMAGE_XBE, name.string().c_str(), sha.c_str(), match ? "" : ", another release");
        if (match) {
            game = {assets, std::move(xbe)};
            kernel::MountDisc(std::move(disc));
            return true;
        }
        if (other_name.empty()) {
            other = {assets, std::move(xbe)};
            other_disc = std::move(disc);
            other_name = name;
            other_sha = sha;
        }
        return false;
    };

    std::error_code ec;
    for (const fs::path& assets : AssetFolders()) {
        if (!fs::is_directory(assets, ec)) continue;
        std::vector<uint8_t> xbe;
        if (ReadHostFile(assets / IMAGE_XBE, xbe) && consider(assets, assets, std::move(xbe), nullptr)) return 1;
        std::vector<fs::path> isos;
        for (auto& e : fs::directory_iterator(assets, ec)) {
            std::string ext = e.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return char(std::tolower(c)); });
            if (ext == ".iso" && e.is_regular_file(ec)) isos.push_back(e.path());
        }
        std::sort(isos.begin(), isos.end());
        for (const fs::path& iso : isos) {
            auto disc = std::make_unique<kernel::Disc>();
            std::string error;
            if (!disc->Open(iso, error)) {
                LOGW("game: %s: %s", iso.string().c_str(), error.c_str());
                continue;
            }
            if (!disc->Load(IMAGE_XBE, xbe)) {
                LOGW("game: %s has no %s", iso.string().c_str(), IMAGE_XBE);
                continue;
            }
            CheckImage(*disc, iso);
            if (consider(assets, iso, std::move(xbe), std::move(disc))) return 1;
        }
    }

    if (other_name.empty()) return 0;
    if (!AskOtherRelease(other_name, other_sha)) return -1;
    game = std::move(other);
    kernel::MountDisc(std::move(other_disc));
    return 1;
}

}  // namespace

bool FindGame(GameFiles& game) {
    int found = Search(game);
    if (!found) NoGame();
    return found > 0;
}

}  // namespace app
