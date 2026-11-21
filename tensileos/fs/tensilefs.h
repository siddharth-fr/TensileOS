#ifndef TENSILE_FS_TENSILEFS_H
#define TENSILE_FS_TENSILEFS_H

#pragma once
// Flash filesystem (LittleFS) helpers: paths, listing, copy, recursive delete.
#include <FS.h>
#include <LittleFS.h>
#include "core/input.h"

#define PATH_LEN 96
static char g_cwd[PATH_LEN] = "/";
static bool g_fsOk = false;

struct FsEntry { char name[32]; bool dir; uint32_t size; };

static const char* baseName(const char* p) { const char* s = strrchr(p, '/'); return s ? s + 1 : p; }
static bool fsBegin() { g_fsOk = LittleFS.begin(true); return g_fsOk; }   // true = format on first boot

// Case-insensitive lookup (DOS habit): finds "NOTES" when you type "notes".
static bool fsFindCI(const char* parent, const char* name, char* out) {
  File d = LittleFS.open(parent, "r");
  if (!d || !d.isDirectory()) { d.close(); return false; }
  bool found = false;
  for (File f = d.openNextFile(); f && !found; f = d.openNextFile())
    if (!strcasecmp(baseName(f.name()), name)) { strncpy(out, baseName(f.name()), 31); out[31] = 0; found = true; }
  d.close();
  return found;
}

// Turn user input (relative, "..", either slash, any letter case) into a clean absolute path like "/docs/a.txt".
static bool fsResolve(const char* in, char* out, size_t n) {
  char tmp[PATH_LEN * 2];
  if (in[0] == '/' || in[0] == '\\') snprintf(tmp, sizeof tmp, "%s", in);
  else snprintf(tmp, sizeof tmp, "%s/%s", g_cwd, in);
  for (char* p = tmp; *p; p++) if (*p == '\\') *p = '/';
  const char* seg[24]; int ns = 0; char* sv = NULL;
  for (char* t = strtok_r(tmp, "/", &sv); t; t = strtok_r(NULL, "/", &sv)) {
    if (!strcmp(t, ".")) continue;
    if (!strcmp(t, "..")) { if (ns) ns--; continue; }
    if (ns >= 24) return false;
    seg[ns++] = t;
  }
  if (!ns) { snprintf(out, n, "/"); return true; }
  size_t len = 0; out[0] = 0;
  for (int i = 0; i < ns; i++) {
    char fixed[32]; const char* use = seg[i];
    char cand[PATH_LEN * 2]; snprintf(cand, sizeof cand, "%s/%s", out, seg[i]);
    if (!LittleFS.exists(cand) && fsFindCI(len ? out : "/", seg[i], fixed)) use = fixed;
    size_t sl = strlen(use);
    if (len + 1 + sl + 1 > n) return false;
    out[len++] = '/'; memcpy(out + len, use, sl); len += sl; out[len] = 0;
  }
  return true;
}
static void fsJoin(const char* dir, const char* name, char* out, size_t n) {
  snprintf(out, n, "%s%s%s", !strcmp(dir, "/") ? "" : dir, "/", name);
}
static bool fsExists(const char* p) { return !strcmp(p, "/") || LittleFS.exists(p); }
static bool fsIsDir(const char* p) {
  if (!strcmp(p, "/")) return true;
  if (!LittleFS.exists(p)) return false;
  File f = LittleFS.open(p, "r");
  bool d = f && f.isDirectory();
  f.close();
  return d;
}
static void fsDisp(const char* p, char* out, size_t n) {          // "/a/b" -> "C:\a\b"
  snprintf(out, n, "C:%s", !strcmp(p, "/") ? "\\" : p);
  for (char* q = out; *q; q++) if (*q == '/') *q = '\\';
}

static int fsCmp(const void* a, const void* b) {
  const FsEntry* x = (const FsEntry*)a; const FsEntry* y = (const FsEntry*)b;
  if (x->dir != y->dir) return x->dir ? -1 : 1;
  return strcasecmp(x->name, y->name);
}
// Fills out[] with up to max entries, folders first. Returns count, or -1 if not a folder.
static int fsList(const char* path, FsEntry* out, int max) {
  File d = LittleFS.open(path, "r");
  if (!d || !d.isDirectory()) { d.close(); return -1; }
  int n = 0;
  for (File f = d.openNextFile(); f; f = d.openNextFile()) {
    if (n < max) {
      strncpy(out[n].name, baseName(f.name()), 31); out[n].name[31] = 0;
      out[n].dir = f.isDirectory(); out[n].size = f.size(); n++;
    }
  }
  d.close();
  qsort(out, n, sizeof(FsEntry), fsCmp);
  return n;
}
static bool fsCopy(const char* src, const char* dst) {
  File a = LittleFS.open(src, "r");
  if (!a || a.isDirectory()) return false;
  File b = LittleFS.open(dst, "w");
  if (!b) { a.close(); return false; }
  uint8_t buf[256]; int n; bool ok = true;
  while ((n = a.read(buf, sizeof buf)) > 0) if (b.write(buf, n) != (size_t)n) { ok = false; break; }
  a.close(); b.close();
  return ok;
}
static bool fsRemoveAll(const char* path, int depth = 0) {
  if (!fsIsDir(path)) return LittleFS.remove(path);
  if (depth > 8) return false;
  for (;;) {
    char child[PATH_LEN]; child[0] = 0;
    File d = LittleFS.open(path, "r");
    File f = d.openNextFile();
    if (f) fsJoin(path, baseName(f.name()), child, sizeof child);
    f.close(); d.close();
    if (!child[0]) break;
    if (!fsRemoveAll(child, depth + 1)) return false;
  }
  return !strcmp(path, "/") ? true : LittleFS.rmdir(path);
}
static bool fsWriteText(const char* path, const char* text) {
  File f = LittleFS.open(path, "w");
  if (!f) return false;
  f.write((const uint8_t*)text, strlen(text));
  f.close();
  return true;
}
static void fsSeed() {                                  // first-boot sample files
  if (!g_fsOk || LittleFS.exists("/README.TXT")) return;
  fsWriteText("/README.TXT",
    "Welcome to TensileOS!\n\n"
    "This file lives in flash memory, so it\nsurvives power-off.\n\n"
    "Try this:\n"
    "  - Open Files on the desktop\n"
    "  - Double-click a file to edit it\n"
    "  - Ctrl+S saves, Ctrl+Q quits the editor\n"
    "  - Play Snake, Minesweeper or Tetris\n"
    "  - F10 opens the Terminal (dir, cd, mkdir, edit)\n");
  LittleFS.mkdir("/NOTES");
  fsWriteText("/NOTES/TODO.TXT", "My ESP32 OS to-do list:\n  [x] boot\n  [x] desktop\n  [ ] take over the world\n");
  fsWriteText("/AUTOEXEC.BAT", "REM TensileOS Startup Script\necho Welcome to TensileOS v0.2.0!\ncolor 1F\n");
}


#endif // TENSILE_FS_TENSILEFS_H
