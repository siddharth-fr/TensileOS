#ifndef TENSILE_SHELL_SHELL_H
#define TENSILE_SHELL_SHELL_H

#pragma once
// TensileOS terminal: line editor, history, tab completion, file commands, app launchers.
#include "games/games.h"

static bool g_leaveShell = false;

#define HIST_N 10
static char g_prompt[48] = "C:\\> ";
static int g_promptLen = 5;
#define MAX_LEN (SCR_COLS - g_promptLen - 1)

static char g_line[SCR_COLS];
static int g_len = 0, g_pos = 0, g_promptRow = 1;
static char g_hist[HIST_N][SCR_COLS];
static int g_histCount = 0, g_histIdx = 0;
static char g_saved[SCR_COLS];

static void redrawLine() {
  screen.lock();
  g_line[g_len] = 0;
  screen.fillRect(g_promptLen, g_promptRow, SCR_COLS - g_promptLen, 1, ' ', screen.attr);
  screen.text(g_promptLen, g_promptRow, g_line, screen.attr);
  screen.setCursor(g_promptLen + g_pos, g_promptRow);
  screen.unlock();
}

static void buildPrompt() {
  char d[PATH_LEN + 4]; fsDisp(g_cwd, d, sizeof d);
  if (strlen(d) > 36) snprintf(d, sizeof d, "C:\\...\\%s", baseName(g_cwd));
  snprintf(g_prompt, sizeof g_prompt, "%s> ", d);
  g_promptLen = strlen(g_prompt);
}

static void printPrompt(bool keepLine) {
  screen.lock();
  buildPrompt();
  if (screen.curX != 0) screen.print("\n");
  uint8_t a = screen.attr;
  screen.attr = 0x1E;
  screen.print(g_prompt);
  screen.attr = a;
  g_promptRow = screen.curY;
  if (!keepLine) { g_len = g_pos = 0; g_line[0] = 0; }
  redrawLine();
  screen.unlock();
  g_histIdx = g_histCount;
}

// ---------- commands ----------
typedef void (*CmdFn)(int, char**);
struct Cmd { const char* name; const char* help; CmdFn fn; };

#define DECL(n) static void n(int, char**)
DECL(cmd_help); DECL(cmd_ver); DECL(cmd_cls); DECL(cmd_echo); DECL(cmd_mem); DECL(cmd_uptime);
DECL(cmd_sysinfo); DECL(cmd_wifi); DECL(cmd_tasks); DECL(cmd_color); DECL(cmd_colors); DECL(cmd_demo);
DECL(cmd_reboot); DECL(cmd_dir); DECL(cmd_cd); DECL(cmd_pwd); DECL(cmd_mkdir); DECL(cmd_rmdir);
DECL(cmd_rm); DECL(cmd_cp); DECL(cmd_mv); DECL(cmd_cat); DECL(cmd_tree); DECL(cmd_df); DECL(cmd_touch);
DECL(cmd_edit); DECL(cmd_exit); DECL(cmd_snake); DECL(cmd_mines); DECL(cmd_tetris); DECL(cmd_calc);
DECL(cmd_bench); DECL(cmd_hwlab); DECL(cmd_paint); DECL(cmd_netmgr); DECL(cmd_browser);
DECL(cmd_beep); DECL(cmd_play); DECL(cmd_run); DECL(cmd_saver);

static const Cmd g_cmds[] = {
  {"help", "list commands (help <cmd> for one)", cmd_help},
  {"dir", "list files: dir [path]", cmd_dir}, {"ls", "same as dir", cmd_dir},
  {"cd", "change folder: cd <path> (cd .. goes up)", cmd_cd}, {"pwd", "show current folder", cmd_pwd},
  {"mkdir", "make a folder: mkdir <name>", cmd_mkdir}, {"rmdir", "remove an empty folder", cmd_rmdir},
  {"rm", "delete: rm [-r] <path>", cmd_rm}, {"del", "same as rm", cmd_rm},
  {"cp", "copy a file: cp <from> <to>", cmd_cp}, {"copy", "same as cp", cmd_cp},
  {"mv", "move/rename: mv <from> <to>", cmd_mv}, {"ren", "same as mv", cmd_mv},
  {"cat", "show a text file: cat <file>", cmd_cat}, {"type", "same as cat", cmd_cat},
  {"tree", "folder tree: tree [path]", cmd_tree}, {"df", "flash storage usage", cmd_df},
  {"touch", "create an empty file", cmd_touch}, {"edit", "text editor: edit <file>", cmd_edit},
  {"echo", "print text; echo hi > file writes it (>> appends)", cmd_echo},
  {"ver", "show TensileOS version", cmd_ver}, {"mem", "heap usage with a bar graph", cmd_mem},
  {"uptime", "time since boot", cmd_uptime}, {"sysinfo", "chip, CPU, flash, PSRAM details", cmd_sysinfo},
  {"wifi", "network status and address", cmd_wifi}, {"tasks", "RTOS task count, stack headroom", cmd_tasks},
  {"reboot", "restart the ESP32", cmd_reboot},
  {"cls", "clear the screen", cmd_cls}, {"clear", "same as cls", cmd_cls},
  {"color", "set colours, e.g. color 1F (bg,fg)", cmd_color}, {"colors", "show the 16-colour palette", cmd_colors},
  {"demo", "retro window demo", cmd_demo},
  {"snake", "play Snake", cmd_snake}, {"mines", "play Minesweeper", cmd_mines}, {"tetris", "play Tetris", cmd_tetris},
  {"calc", "calculator", cmd_calc},
  {"bench", "hardware benchmark suite", cmd_bench},
  {"hwlab", "hardware GPIO and ADC lab", cmd_hwlab},
  {"hw", "same as hwlab", cmd_hwlab},
  {"paint", "ANSI and CP437 pixel art studio", cmd_paint},
  {"netmgr", "Wi-Fi scanner and manager", cmd_netmgr},
  {"browser", "RetroWeb text HTML browser and search", cmd_browser},
  {"web", "same as browser", cmd_browser},
  {"beep", "play PC speaker tone: beep [freq] [ms]", cmd_beep},
  {"play", "play MML melody: play \"<notes>\"", cmd_play},
  {"run", "execute batch script: run <file.bat>", cmd_run},
  {"saver", "screensaver: saver [matrix|stars|box|off]", cmd_saver},
  {"desktop", "go back to the desktop", cmd_exit}, {"exit", "go back to the desktop", cmd_exit},
};
#define NCMDS (sizeof(g_cmds) / sizeof(g_cmds[0]))

static void cmd_help(int argc, char** argv) {
  if (argc > 1) {
    for (size_t i = 0; i < NCMDS; i++)
      if (!strcasecmp(argv[1], g_cmds[i].name)) { screen.printf("%s - %s\n", g_cmds[i].name, g_cmds[i].help); return; }
    screen.printf("No help for '%s'\n", argv[1]);
    return;
  }
  screen.print("Files : dir cd pwd mkdir rmdir rm cp mv cat tree df touch edit echo\n");
  screen.print("System: ver mem uptime sysinfo wifi tasks reboot\n");
  screen.print("Sound : beep play | Scripts: run <file.bat> | Screensaver: saver\n");
  screen.print("Screen: cls color colors demo\n");
  screen.print("Apps  : snake mines tetris calc bench hwlab paint netmgr browser (exit = desktop)\n");
  screen.print("help <cmd> explains one. Aliases: ls del copy ren type clear\n");
  screen.print("Keys  : Up/Down history, Tab complete, Ctrl+C cancel, Ctrl+L clear\n");
}
static void cmd_ver(int, char**) { screen.printf("%s version %s  (%s)\n", OS_NAME, OS_VERSION, ESP.getSdkVersion()); }
static void cmd_cls(int, char**) { screen.clear(); }
static void cmd_exit(int, char**) { g_leaveShell = true; }

static void cmd_echo(int argc, char** argv) {
  int end = argc; const char* target = NULL; bool app = false;
  for (int i = 1; i + 1 < argc; i++)
    if (!strcmp(argv[i], ">") || !strcmp(argv[i], ">>")) { app = argv[i][1] == '>'; target = argv[i + 1]; end = i; break; }
  char text[SCR_COLS * 2]; text[0] = 0;
  for (int i = 1; i < end; i++) { strncat(text, argv[i], sizeof text - strlen(text) - 2); if (i + 1 < end) strcat(text, " "); }
  if (!target) { screen.print(text); screen.print("\n"); return; }
  char p[PATH_LEN];
  if (!fsResolve(target, p, sizeof p) || fsIsDir(p)) { screen.print("Bad file name\n"); return; }
  File f = LittleFS.open(p, app ? "a" : "w");
  if (!f) { screen.print("Cannot write file\n"); return; }
  f.write((const uint8_t*)text, strlen(text)); f.write((const uint8_t*)"\n", 1); f.close();
}

static void bar(uint32_t used, uint32_t total, int width) {
  int n = total ? (int)((uint64_t)used * width / total) : 0;
  screen.print("[");
  for (int i = 0; i < width; i++) screen.putc(i < n ? G_FULL : G_LIGHT);
  screen.print("]");
}
static void cmd_mem(int, char**) {
  uint32_t total = ESP.getHeapSize(), fr = ESP.getFreeHeap();
  screen.printf("Heap total : %6u bytes\n", (unsigned)total);
  screen.printf("Heap free  : %6u bytes\n", (unsigned)fr);
  screen.printf("Lowest free: %6u bytes\n", (unsigned)ESP.getMinFreeHeap());
  screen.printf("Largest blk: %6u bytes\n", (unsigned)ESP.getMaxAllocHeap());
  screen.printf("Screen     : %6u bytes\n", (unsigned)(sizeof(Cell) * SCR_CELLS));
  screen.print("Used ");
  bar(total - fr, total, 40);
  screen.printf(" %u%%\n", (unsigned)(total ? (uint64_t)(total - fr) * 100 / total : 0));
}
static void cmd_uptime(int, char**) {
  uint32_t s = millis() / 1000;
  screen.printf("Up %u days %02u:%02u:%02u\n", (unsigned)(s / 86400), (unsigned)((s / 3600) % 24), (unsigned)((s / 60) % 60), (unsigned)(s % 60));
}
static void cmd_sysinfo(int, char**) {
  screen.printf("Chip     : %s rev %u, %u core(s)\n", ESP.getChipModel(), (unsigned)ESP.getChipRevision(), (unsigned)ESP.getChipCores());
  screen.printf("CPU      : %u MHz\n", (unsigned)ESP.getCpuFreqMHz());
  screen.printf("Flash    : %u KB @ %u MHz\n", (unsigned)(ESP.getFlashChipSize() / 1024), (unsigned)(ESP.getFlashChipSpeed() / 1000000));
  screen.printf("PSRAM    : %u KB\n", (unsigned)(ESP.getPsramSize() / 1024));
  screen.printf("Firmware : %u KB used, %u KB free\n", (unsigned)(ESP.getSketchSize() / 1024), (unsigned)(ESP.getFreeSketchSpace() / 1024));
  screen.printf("SDK      : %s\n", ESP.getSdkVersion());
}
static void cmd_wifi(int, char**) {
  if (g_apMode) {
    screen.printf("Mode : Access point (own hotspot)\nSSID : %s\n", AP_SSID);
    screen.printf("IP   : %s\nUsers: %d connected\n", netIP().c_str(), (int)WiFi.softAPgetStationNum());
  } else {
    screen.printf("Mode : Station (home Wi-Fi)\nSSID : %s\n", WiFi.SSID().c_str());
    screen.printf("IP   : %s\nRSSI : %d dBm\n", netIP().c_str(), (int)WiFi.RSSI());
    screen.printf("Name : http://%s.local/\n", HOSTNAME);
  }
  screen.printf("Open : http://%s/   (%d viewer(s))\n", netIP().c_str(), wsClientCount());
}
static void cmd_tasks(int, char**) {
  screen.printf("RTOS tasks      : %u\n", (unsigned)uxTaskGetNumberOfTasks());
  screen.printf("OS stack free   : %u bytes\n", (unsigned)uxTaskGetStackHighWaterMark(NULL));
  screen.printf("OS runs on core : %d\n", (int)xPortGetCoreID());
}
static int hexv(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}
static void cmd_color(int argc, char** argv) {
  if (argc < 2 || strlen(argv[1]) != 2 || hexv(argv[1][0]) < 0 || hexv(argv[1][1]) < 0) {
    screen.printf("Usage: color BF   (B=background, F=foreground, hex 0-F)\nCurrent: %02X\n", screen.attr);
    return;
  }
  uint8_t a = (hexv(argv[1][0]) << 4) | hexv(argv[1][1]);
  if ((a >> 4) == (a & 15)) { screen.print("Foreground and background must differ\n"); return; }
  screen.recolor(a);
}
static void cmd_colors(int, char**) {
  uint8_t keep = screen.attr;
  for (int i = 0; i < 16; i++) { screen.attr = (i << 4) | (i < 8 ? 0x0F : 0x00); screen.printf(" %X ", i); }
  screen.attr = keep; screen.print("\n");
  for (int i = 0; i < 16; i++) { screen.attr = (i == 1 ? 0x70 : 0x00) | i; screen.print("Ab "); }
  screen.attr = keep; screen.print("\n");
}
static void cmd_demo(int, char**) {
  Cell* snap = (Cell*)malloc(sizeof(Cell) * SCR_CELLS);
  if (!snap) { screen.print("Out of memory\n"); return; }
  uint8_t sx = screen.curX, sy = screen.curY;
  screen.snapshot(snap);
  screen.showCursor(false);
  screen.fillRect(0, 1, SCR_COLS, SCR_ROWS - 1, G_MED, 0x19);
  drawFrame(14, 5, 52, 14, "TensileOS Demo", true, false);
  screen.text(17, 8, "Welcome to TensileOS!", 0x7F);
  screen.text(17, 10, "Panels, menus and buttons here are just", 0x70);
  screen.text(17, 11, "characters, the same trick DOS apps used.", 0x70);
  screen.text(17, 13, "Press any key.", 0x78);
  readKey(0);
  screen.restore(snap);
  screen.setCursor(sx, sy);
  screen.showCursor(true);
  free(snap);
  statusBar();
}
static void cmd_reboot(int, char**) { screen.print("Rebooting...\n"); delay(400); ESP.restart(); }

// ---- file commands ----
static bool needArg(int argc, const char* usage) { if (argc >= 2) return true; screen.printf("Usage: %s\n", usage); return false; }
static bool pathArg(const char* in, char* out) {
  if (fsResolve(in, out, PATH_LEN)) return true;
  screen.print("Path too long or invalid\n"); return false;
}
static void fmtSize(uint32_t b, char* out, size_t n) {
  if (b < 10240) snprintf(out, n, "%u B", (unsigned)b); else if (b < 10485760) snprintf(out, n, "%u KB", (unsigned)(b / 1024)); else snprintf(out, n, "%u MB", (unsigned)(b / 1048576));
}

static void cmd_dir(int argc, char** argv) {
  char p[PATH_LEN]; if (!pathArg(argc > 1 ? argv[1] : ".", p)) return;
  if (!fsExists(p)) { screen.print("File not found\n"); return; }
  if (!fsIsDir(p)) { File f = LittleFS.open(p, "r"); screen.printf("%10u  %s\n", (unsigned)f.size(), baseName(p)); f.close(); return; }
  FsEntry* e = (FsEntry*)malloc(sizeof(FsEntry) * 64);
  if (!e) { screen.print("Out of memory\n"); return; }
  int n = fsList(p, e, 64);
  char d[PATH_LEN + 4]; fsDisp(p, d, sizeof d);
  screen.printf(" Directory of %s\n\n", d);
  int nf = 0, nd = 0; uint32_t tot = 0;
  for (int i = 0; i < n; i++) {
    if (e[i].dir) { screen.printf(" <DIR>      %s\n", e[i].name); nd++; }
    else { screen.printf("%10u  %s\n", (unsigned)e[i].size, e[i].name); nf++; tot += e[i].size; }
  }
  if (n == 64) screen.print(" (only the first 64 entries are shown)\n");
  screen.printf(" %d file(s) %u bytes, %d dir(s)\n", nf, (unsigned)tot, nd);
  free(e);
}
static void cmd_cd(int argc, char** argv) {
  if (argc < 2) { char d[PATH_LEN + 4]; fsDisp(g_cwd, d, sizeof d); screen.printf("%s\n", d); return; }
  char p[PATH_LEN]; if (!pathArg(argv[1], p)) return;
  if (!fsIsDir(p)) { screen.print("Folder not found\n"); return; }
  strcpy(g_cwd, p);
}
static void cmd_pwd(int, char**) { char d[PATH_LEN + 4]; fsDisp(g_cwd, d, sizeof d); screen.printf("%s\n", d); }
static void cmd_mkdir(int argc, char** argv) {
  if (!needArg(argc, "mkdir <name>")) return;
  char p[PATH_LEN]; if (!pathArg(argv[1], p)) return;
  if (fsExists(p)) { screen.print("Already exists\n"); return; }
  if (!LittleFS.mkdir(p)) screen.print("Could not create folder (does the parent exist?)\n");
}
static void cmd_rmdir(int argc, char** argv) {
  if (!needArg(argc, "rmdir <folder>")) return;
  char p[PATH_LEN]; if (!pathArg(argv[1], p)) return;
  if (!strcmp(p, "/")) { screen.print("Cannot remove the root\n"); return; }
  if (!fsIsDir(p)) { screen.print("Folder not found\n"); return; }
  if (!LittleFS.rmdir(p)) screen.print("Folder is not empty (use rm -r)\n");
  else {
    size_t pl = strlen(p);
    if (!strcmp(g_cwd, p) || (!strncmp(g_cwd, p, pl) && (g_cwd[pl] == '/' || !strcmp(p, "/")))) strcpy(g_cwd, "/");
  }
}
static void cmd_rm(int argc, char** argv) {
  bool rec = false; const char* t = NULL;
  for (int i = 1; i < argc; i++) { if (!strcmp(argv[i], "-r")) rec = true; else t = argv[i]; }
  if (!t) { screen.print("Usage: rm [-r] <file or folder>\n"); return; }
  char p[PATH_LEN]; if (!pathArg(t, p)) return;
  if (!strcmp(p, "/")) { screen.print("Refusing to delete the root\n"); return; }
  if (!fsExists(p)) { screen.print("File not found\n"); return; }
  if (fsIsDir(p) && !rec) { screen.print("That is a folder: use rm -r\n"); return; }
  if (!fsRemoveAll(p)) screen.print("Delete failed\n");
  if (!fsIsDir(g_cwd)) strcpy(g_cwd, "/");
}
static void cmd_cp(int argc, char** argv) {
  if (argc < 3) { screen.print("Usage: cp <from> <to>\n"); return; }
  char a[PATH_LEN], b[PATH_LEN]; if (!pathArg(argv[1], a) || !pathArg(argv[2], b)) return;
  if (!fsExists(a) || fsIsDir(a)) { screen.print("Source file not found\n"); return; }
  if (fsIsDir(b)) { char t[PATH_LEN]; fsJoin(b, baseName(a), t, sizeof t); strcpy(b, t); }
  if (!strcmp(a, b)) { screen.print("Source and target are the same\n"); return; }
  screen.print(fsCopy(a, b) ? "1 file copied\n" : "Copy failed\n");
}
static void cmd_mv(int argc, char** argv) {
  if (argc < 3) { screen.print("Usage: mv <from> <to>\n"); return; }
  char a[PATH_LEN], b[PATH_LEN]; if (!pathArg(argv[1], a) || !pathArg(argv[2], b)) return;
  if (!fsExists(a)) { screen.print("Source not found\n"); return; }
  if (fsIsDir(b)) { char t[PATH_LEN]; fsJoin(b, baseName(a), t, sizeof t); strcpy(b, t); }
  if (fsExists(b)) { screen.print("Target already exists\n"); return; }
  if (!LittleFS.rename(a, b)) screen.print("Move failed\n");
}
static void cmd_cat(int argc, char** argv) {
  if (!needArg(argc, "cat <file>")) return;
  char p[PATH_LEN]; if (!pathArg(argv[1], p)) return;
  if (!fsExists(p) || fsIsDir(p)) { screen.print("File not found\n"); return; }
  File f = LittleFS.open(p, "r");
  int shown = 0, c, lastc = '\n';
  while (shown < 1500 && (c = f.read()) >= 0) {
    if (c == '\r') continue;
    if (c == '\t') c = ' ';
    if (c >= 32 || c == '\n') { screen.putc((uint8_t)c); shown++; lastc = c; }
  }
  if (f.available()) { screen.print("\n[... file continues, use edit to read it all]"); lastc = 'x'; }
  f.close();
  if (lastc != '\n') screen.print("\n");
}
static void treeRec(const char* path, int depth, char* prefixBuf) {
  FsEntry* e = (FsEntry*)malloc(sizeof(FsEntry) * 40);
  if (!e) return;
  int n = fsList(path, e, 40);
  for (int i = 0; i < n; i++) {
    bool last = (i == n - 1);
    screen.printf("%s%c%c%c %s%s\n", prefixBuf, last ? 0xC0 : 0xC3, 0xC4, 0xC4, e[i].name, e[i].dir ? "\\" : "");
    if (e[i].dir && depth < 5) {
      char child[PATH_LEN]; fsJoin(path, e[i].name, child, sizeof child);
      size_t pl = strlen(prefixBuf);
      if (pl + 4 < 40) { prefixBuf[pl] = last ? ' ' : (char)0xB3; prefixBuf[pl + 1] = ' '; prefixBuf[pl + 2] = ' '; prefixBuf[pl + 3] = 0; }
      treeRec(child, depth + 1, prefixBuf);
      prefixBuf[pl] = 0;
    }
  }
  free(e);
}
static void cmd_tree(int argc, char** argv) {
  char p[PATH_LEN]; if (!pathArg(argc > 1 ? argv[1] : ".", p)) return;
  if (!fsIsDir(p)) { screen.print("Folder not found\n"); return; }
  char d[PATH_LEN + 4]; fsDisp(p, d, sizeof d); screen.printf("%s\n", d);
  char pre[40] = ""; treeRec(p, 0, pre);
}
static void cmd_df(int, char**) {
  uint32_t t = LittleFS.totalBytes(), u = LittleFS.usedBytes();
  char a[16], b[16], c[16]; fmtSize(t, a, sizeof a); fmtSize(u, b, sizeof b); fmtSize(t - u, c, sizeof c);
  screen.printf("Flash drive C: total %s, used %s, free %s\n", a, b, c);
  screen.print("Used "); bar(u, t, 40); screen.print("\n");
}
static void cmd_touch(int argc, char** argv) {
  if (!needArg(argc, "touch <file>")) return;
  char p[PATH_LEN]; if (!pathArg(argv[1], p)) return;
  if (fsIsDir(p)) { screen.print("That is a folder\n"); return; }
  if (!fsExists(p) && !fsWriteText(p, "")) screen.print("Could not create file\n");
}
static void cmd_edit(int argc, char** argv) {
  char p[PATH_LEN] = "";
  if (argc > 1 && !pathArg(argv[1], p)) return;
  if (p[0] && fsIsDir(p)) { screen.print("That is a folder\n"); return; }
  Editor* e = new Editor(p, true);
  runApp(e);
  delete e;
  screen.clear();
}
static void runGame(App* a) { runApp(a); delete a; screen.clear(); }
static void cmd_snake(int, char**)  { runGame(new Snake()); }
static void cmd_mines(int, char**)  { runGame(new Mines()); }
static void cmd_tetris(int, char**) { runGame(new Tetris()); }
static void cmd_calc(int, char**)   { runGame(new Calc()); }
static void cmd_bench(int, char**)  { runGame(new SysBench()); }
static void cmd_hwlab(int, char**)  { runGame(new HwLab()); }
static void cmd_paint(int, char**)  { runGame(new AnsiPaint()); }
static void cmd_netmgr(int, char**) { runGame(new NetManager()); }
static void cmd_browser(int argc, char** argv) { runGame(new RetroWeb(argc > 1 ? argv[1] : NULL)); }

static void batchRun(const char* path);

static void cmd_beep(int argc, char** argv) {
  int freq = (argc > 1) ? atoi(argv[1]) : 880;
  int dur = (argc > 2) ? atoi(argv[2]) : 150;
  if (freq <= 0) freq = 880;
  if (dur <= 0) dur = 150;
  soundTone(freq, dur);
}

static void cmd_play(int argc, char** argv) {
  if (argc < 2) {
    screen.print("Usage: play \"<mml notes>\"\nExample: play \"T160 O4 C D E F G A B > C\"\n");
    return;
  }
  char mml[128] = "";
  for (int i = 1; i < argc; i++) {
    if (i > 1 && strlen(mml) < sizeof(mml) - 2) strcat(mml, " ");
    strncat(mml, argv[i], sizeof(mml) - strlen(mml) - 1);
  }
  soundPlay(mml);
}

static void cmd_run(int argc, char** argv) {
  if (argc < 2) { screen.print("Usage: run <script.bat>\n"); return; }
  batchRun(argv[1]);
}

static void cmd_saver(int argc, char** argv) {
  if (argc > 1) {
    if (!strcasecmp(argv[1], "matrix")) launchScreensaver(SAVER_MATRIX);
    else if (!strcasecmp(argv[1], "stars") || !strcasecmp(argv[1], "starfield")) launchScreensaver(SAVER_STARFIELD);
    else if (!strcasecmp(argv[1], "box") || !strcasecmp(argv[1], "bounce")) launchScreensaver(SAVER_BOUNCE);
    else if (!strcasecmp(argv[1], "off")) { g_saverMode = SAVER_OFF; screen.print("Screensaver disabled.\n"); }
    else screen.printf("Unknown mode: %s (use matrix, stars, box, off)\n", argv[1]);
  } else {
    launchScreensaver(g_saverMode);
  }
}

// ---------- parse & run ----------
static void execute(char* line) {
  char* argv[10]; int argc = 0; char* p = line;
  while (*p) {
    while (*p == ' ') p++;
    if (!*p || argc >= 9) break;
    if (*p == '"') { p++; argv[argc++] = p; while (*p && *p != '"') p++; }
    else           { argv[argc++] = p; while (*p && *p != ' ') p++; }
    if (*p) { *p = 0; p++; }
  }
  if (!argc) return;
  const char* ext = strrchr(argv[0], '.');
  if (ext && !strcasecmp(ext, ".bat")) { batchRun(argv[0]); return; }
  for (size_t i = 0; i < NCMDS; i++)
    if (!strcasecmp(argv[0], g_cmds[i].name)) { g_cmds[i].fn(argc, argv); return; }
  screen.printf("Bad command or file name: %s\n", argv[0]);
}

static void batchRun(const char* path) {
  char full[PATH_LEN];
  if (!pathArg(path, full)) return;
  if (!fsExists(full)) { screen.printf("Batch file not found: %s\n", path); return; }
  File f = LittleFS.open(full, "r");
  if (!f) { screen.printf("Cannot open: %s\n", path); return; }
  while (f.available()) {
    String l = f.readStringUntil('\n');
    l.trim();
    if (!l.length() || l[0] == '#' || !strncasecmp(l.c_str(), "REM", 3)) continue;
    if (!strncasecmp(l.c_str(), "DELAY ", 6)) {
      int ms = atoi(l.c_str() + 6);
      if (ms > 0) delay(ms);
      continue;
    }
    char buf[SCR_COLS];
    strncpy(buf, l.c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    execute(buf);
  }
  f.close();
}
static void histAdd(const char* s) {
  if (!*s) return;
  if (g_histCount && !strcmp(g_hist[g_histCount - 1], s)) return;
  if (g_histCount == HIST_N) { memmove(g_hist[0], g_hist[1], sizeof(g_hist[0]) * (HIST_N - 1)); g_histCount--; }
  strncpy(g_hist[g_histCount], s, SCR_COLS - 1); g_hist[g_histCount][SCR_COLS - 1] = 0;
  g_histCount++;
}
static void loadLine(const char* s) {
  strncpy(g_line, s, MAX_LEN); g_line[MAX_LEN] = 0;
  g_len = g_pos = strlen(g_line);
  redrawLine();
}
static void tabComplete() {
  g_line[g_len] = 0;
  if (strchr(g_line, ' ')) return;
  const char* m[NCMDS]; int n = 0; size_t L = strlen(g_line);
  for (size_t i = 0; i < NCMDS; i++) if (!strncasecmp(g_line, g_cmds[i].name, L)) m[n++] = g_cmds[i].name;
  if (!n) return;
  if (n == 1) {
    strcpy(g_line, m[0]); g_len = g_pos = strlen(m[0]);
    if (g_len < MAX_LEN) { g_line[g_len++] = ' '; g_pos = g_len; }
    redrawLine(); return;
  }
  size_t cp = strlen(m[0]);
  for (int i = 1; i < n; i++) while (strncasecmp(m[0], m[i], cp)) cp--;
  if (cp > L) { memcpy(g_line, m[0], cp); g_line[cp] = 0; g_len = g_pos = cp; redrawLine(); return; }
  screen.setCursor(0, g_promptRow); screen.print("\n");
  for (int i = 0; i < n; i++) { screen.print(m[i]); screen.print("  "); }
  screen.print("\n");
  printPrompt(true);
}
static void handleKey(uint8_t k) {
  switch (k) {
    case KEY_ENTER: case 10: {
      g_line[g_len] = 0;
      screen.setCursor(0, g_promptRow); screen.print("\n");
      char copy[SCR_COLS]; strcpy(copy, g_line);
      histAdd(copy); execute(copy);
      if (!g_leaveShell) printPrompt(false);
      break;
    }
    case KEY_BKSP: case 127:
      if (g_pos > 0) { memmove(g_line + g_pos - 1, g_line + g_pos, g_len - g_pos + 1); g_pos--; g_len--; redrawLine(); }
      break;
    case KEY_DEL:
      if (g_pos < g_len) { memmove(g_line + g_pos, g_line + g_pos + 1, g_len - g_pos); g_len--; redrawLine(); }
      break;
    case KEY_LEFT:  if (g_pos > 0)     { g_pos--; redrawLine(); } break;
    case KEY_RIGHT: if (g_pos < g_len) { g_pos++; redrawLine(); } break;
    case KEY_HOME: case 1: g_pos = 0;     redrawLine(); break;
    case KEY_END:  case 5: g_pos = g_len; redrawLine(); break;
    case KEY_UP:
      if (g_histIdx > 0) {
        if (g_histIdx == g_histCount) { g_line[g_len] = 0; strcpy(g_saved, g_line); }
        g_histIdx--; loadLine(g_hist[g_histIdx]);
      }
      break;
    case KEY_DOWN:
      if (g_histIdx < g_histCount) { g_histIdx++; loadLine(g_histIdx == g_histCount ? g_saved : g_hist[g_histIdx]); }
      break;
    case KEY_TAB: tabComplete(); break;
    case KEY_CTRL_U: g_len = g_pos = 0; redrawLine(); break;
    case KEY_CTRL_L: screen.clear(); printPrompt(false); break;
    case KEY_CTRL_C:
      screen.setCursor(g_promptLen + g_len, g_promptRow); screen.print("^C\n"); printPrompt(false);
      break;
    case KEY_F10: g_leaveShell = true; break;
    default:
      if (k >= 32 && k < 127 && g_len < MAX_LEN) {
        memmove(g_line + g_pos + 1, g_line + g_pos, g_len - g_pos + 1);
        g_line[g_pos++] = (char)k; g_len++;
        redrawLine();
      }
  }
}
static void banner() {
  screen.clear();
  screen.box(1, 2, 44, 5, 0x1B, true);
  screen.text(4, 3, OS_NAME "  v" OS_VERSION "  terminal", 0x1F);
  screen.text(4, 4, "A tiny text-mode OS for the ESP32", 0x17);
  screen.text(4, 5, "Type EXIT (or press F10) for the desktop.", 0x17);
  screen.setCursor(0, 8);
  screen.print("Type HELP for commands.\n\n");
}
static void shellRun() {
  g_leaveShell = false;
  g_barHint = "";
  banner();
  printPrompt(false);
  while (!g_leaveShell) handleKey(readKey(0));
}


#endif // TENSILE_SHELL_SHELL_H
