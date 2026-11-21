#ifndef TENSILE_NET_WEB_BROWSER_H
#define TENSILE_NET_WEB_BROWSER_H

#pragma once
// RetroWeb: Text-mode Web & HTML Browser with integrated Web & Local Search Engine.
// Supports HTTP/HTTPS pages (Lynx-style), local HTML/text files, and FrogFind/DuckDuckGo search.
#include "core/ui.h"
#include "net/net.h"
#include "fs/tensilefs.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <vector>
#include <string>

class RetroWeb : public App {
 public:
  struct WebLine {
    std::string text;
    uint8_t attr;
    int linkId; // 0 if none, >0 if link
  };

  struct WebLink {
    int id;
    std::string href;
  };

  std::vector<WebLine> lines;
  std::vector<WebLink> links;

  char currentUrl[128] = "home";
  char urlInput[128] = "";
  int urlLen = 0;
  bool editingUrl = false;
  int top = 0;
  int focusedLink = 0;
  bool loading = false;
  char statusMsg[64] = "Ready. Type a URL or ? search";
  char pageTitle[48] = "RetroWeb";

  RetroWeb(const char* start = NULL) {
    prefW = 62; prefH = 20;
    fixed = false;
    if (start && start[0]) navigate(start);
    else loadHome();
  }

  const char* title() override { return pageTitle[0] ? pageTitle : "RetroWeb"; }

  void loadHome() {
    lines.clear();
    links.clear();
    top = 0;
    focusedLink = 0;
    strncpy(pageTitle, "RetroWeb - Home", sizeof(pageTitle));
    strncpy(currentUrl, "home", sizeof(currentUrl));

    addLine("========================================================", 0x1E);
    addLine("            * RETROWEB & SEARCH PORTAL *               ", 0x1F);
    addLine("========================================================", 0x1E);
    addLine("", 0x70);
    addLine("Type '? query' in the URL bar to search the web!", 0x72);
    addLine("Example: '? vintage computers' or '? esp32'", 0x78);
    addLine("", 0x70);
    addLine("Quick Destinations (Click or type number):", 0x71);
    addLinkLine(" [1] FrogFind (Vintage Text Search Engine)", "http://frogfind.com/");
    addLinkLine(" [2] World Wide Web Project (CERN 1991)", "http://info.cern.ch/hypertext/WWW/TheProject.html");
    addLinkLine(" [3] Wikipedia Text Entry Point", "http://frogfind.com/?q=wikipedia");
    addLinkLine(" [4] TensileOS Local Docs (/README.TXT)", "file:///README.TXT");
    addLinkLine(" [5] TensileOS System Tasks (/NOTES/TODO.TXT)", "file:///NOTES/TODO.TXT");
    addLine("", 0x70);
    addLine("--------------------------------------------------------", 0x78);
    addLine("Shortcuts: Tab/Arrows: Move  Enter: Open  F: Find/URL", 0x78);

    snprintf(statusMsg, sizeof(statusMsg), "Home portal loaded. %d links", (int)links.size());
  }

  void addLine(const std::string& txt, uint8_t attr = 0x70, int linkId = 0) {
    WebLine wl = {txt, attr, linkId};
    lines.push_back(wl);
  }

  void addLinkLine(const std::string& label, const std::string& href) {
    int id = (int)links.size() + 1;
    WebLink lk = {id, href};
    links.push_back(lk);
    addLine(label, 0x1B, id);
  }

  void wrapAndAdd(const std::string& str, uint8_t attr = 0x70, int linkId = 0, int maxW = 58) {
    if (str.empty()) { addLine("", attr, linkId); return; }
    size_t start = 0;
    while (start < str.size()) {
      size_t len = str.size() - start;
      if ((int)len > maxW) {
        size_t end = start + maxW;
        while (end > start && str[end] != ' ') end--;
        if (end == start) end = start + maxW;
        addLine(str.substr(start, end - start), attr, linkId);
        start = (str[end] == ' ') ? end + 1 : end;
      } else {
        addLine(str.substr(start), attr, linkId);
        break;
      }
    }
  }

  // Local LittleFS text file search when offline or requested
  void searchLocal(const char* query) {
    lines.clear();
    links.clear();
    top = 0;
    focusedLink = 0;
    snprintf(pageTitle, sizeof(pageTitle), "Search: %s", query);

    addLine("========================================================", 0x1E);
    std::string h = " Local File Search: \"" + std::string(query) + "\"";
    addLine(h, 0x1F);
    addLine("========================================================", 0x1E);
    addLine("", 0x70);

    // Search /README.TXT and /NOTES/TODO.TXT
    const char* filesToSearch[] = {"/README.TXT", "/NOTES/TODO.TXT", "/CONFIG/WIFI.DAT"};
    int foundCount = 0;

    for (int fi = 0; fi < 3; fi++) {
      const char* fn = filesToSearch[fi];
      if (!LittleFS.exists(fn)) continue;
      File f = LittleFS.open(fn, "r");
      if (!f) continue;

      bool headerPrinted = false;
      int lineNo = 1;
      while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() > 0 && strcasestr(line.c_str(), query) != NULL) {
          if (!headerPrinted) {
            std::string fhdr = "File: " + std::string(fn);
            addLine(fhdr, 0x71);
            headerPrinted = true;
          }
          char lbuf[16];
          snprintf(lbuf, sizeof(lbuf), "  Line %d: ", lineNo);
          std::string matchText = std::string(lbuf) + line.c_str();
          addLinkLine(matchText, std::string("file://") + fn);
          foundCount++;
        }
        lineNo++;
      }
      f.close();
    }

    if (foundCount == 0) {
      addLine("No matching text found in local storage files.", 0x74);
    } else {
      addLine("", 0x70);
      char fmsg[48];
      snprintf(fmsg, sizeof(fmsg), "Found %d matching occurrences.", foundCount);
      addLine(fmsg, 0x72);
    }
    snprintf(statusMsg, sizeof(statusMsg), "Search finished (%d matches)", foundCount);
  }

  // Load a local file from LittleFS
  void loadLocalFile(const char* path) {
    lines.clear();
    links.clear();
    top = 0;
    focusedLink = 0;
    snprintf(pageTitle, sizeof(pageTitle), "%s", baseName(path));

    if (!LittleFS.exists(path)) {
      addLine("Error 404: File Not Found", 0x74);
      addLine(std::string("Path: ") + path, 0x78);
      snprintf(statusMsg, sizeof(statusMsg), "File not found");
      return;
    }

    File f = LittleFS.open(path, "r");
    if (!f) {
      addLine("Error: Unable to open file", 0x74);
      return;
    }

    addLine(std::string("=== File: ") + path + " ===", 0x1F);
    addLine("", 0x70);

    while (f.available() && lines.size() < 250) {
      String l = f.readStringUntil('\n');
      l.replace("\r", "");
      wrapAndAdd(l.c_str(), 0x70);
    }
    f.close();
    snprintf(statusMsg, sizeof(statusMsg), "Loaded %u lines", (unsigned)lines.size());
  }

  // Decode basic HTML entities
  std::string decodeEntities(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); i++) {
      if (in[i] == '&') {
        if (in.compare(i, 4, "&lt;") == 0) { out.push_back('<'); i += 3; }
        else if (in.compare(i, 4, "&gt;") == 0) { out.push_back('>'); i += 3; }
        else if (in.compare(i, 5, "&amp;") == 0) { out.push_back('&'); i += 4; }
        else if (in.compare(i, 6, "&quot;") == 0) { out.push_back('"'); i += 5; }
        else if (in.compare(i, 6, "&apos;") == 0) { out.push_back('\''); i += 5; }
        else if (in.compare(i, 6, "&nbsp;") == 0) { out.push_back(' '); i += 5; }
        else out.push_back(in[i]);
      } else {
        out.push_back(in[i]);
      }
    }
    return out;
  }

  // Tokenizing HTML Parser: converts HTML stream into lines & links
  void parseAndRenderHtml(const String& html) {
    lines.clear();
    links.clear();
    top = 0;
    focusedLink = 0;

    std::string currentText;
    std::string currentHref;
    bool inTag = false;
    bool inLink = false;
    bool inScript = false;
    bool inStyle = false;
    bool isHeading = false;
    std::string tag;

    size_t i = 0;
    size_t n = html.length();

    while (i < n && lines.size() < 300) {
      char c = html[i++];
      if (c == '<') {
        inTag = true;
        tag.clear();
        continue;
      }
      if (inTag) {
        if (c == '>') {
          inTag = false;
          // Process tag name
          size_t sp = tag.find_first_of(" \t\r\n");
          std::string tagName = (sp == std::string::npos) ? tag : tag.substr(0, sp);
          for (char& tc : tagName) tc = tolower((unsigned char)tc);

          if (tagName == "script") inScript = true;
          else if (tagName == "/script") inScript = false;
          else if (tagName == "style") inStyle = true;
          else if (tagName == "/style") inStyle = false;
          else if (tagName == "title") {
            // Read title
            std::string t;
            while (i < n && html[i] != '<') t += html[i++];
            snprintf(pageTitle, sizeof(pageTitle), "RetroWeb - %s", t.substr(0, 30).c_str());
          } else if (tagName == "p" || tagName == "div" || tagName == "br" || tagName == "tr") {
            if (!currentText.empty()) {
              wrapAndAdd(decodeEntities(currentText), isHeading ? 0x1E : 0x70);
              currentText.clear();
            }
            if (tagName != "br") addLine("", 0x70);
          } else if (tagName == "h1" || tagName == "h2" || tagName == "h3") {
            if (!currentText.empty()) { wrapAndAdd(decodeEntities(currentText)); currentText.clear(); }
            isHeading = true;
            addLine("--------------------------------------------------------", 0x1E);
          } else if (tagName == "/h1" || tagName == "/h2" || tagName == "/h3") {
            if (!currentText.empty()) { wrapAndAdd(decodeEntities(currentText), 0x1E); currentText.clear(); }
            isHeading = false;
            addLine("--------------------------------------------------------", 0x1E);
          } else if (tagName == "li") {
            if (!currentText.empty()) { wrapAndAdd(decodeEntities(currentText)); currentText.clear(); }
            currentText = " * ";
          } else if (tagName == "a") {
            inLink = true;
            currentHref.clear();
            // Extract href
            size_t hpos = tag.find("href=");
            if (hpos != std::string::npos) {
              size_t vstart = hpos + 5;
              char quote = tag[vstart];
              if (quote == '"' || quote == '\'') {
                vstart++;
                size_t vend = tag.find(quote, vstart);
                if (vend != std::string::npos) currentHref = tag.substr(vstart, vend - vstart);
              } else {
                size_t vend = tag.find_first_of(" \t>", vstart);
                currentHref = tag.substr(vstart, vend - vstart);
              }
            }
          } else if (tagName == "/a") {
            if (inLink && !currentText.empty() && !currentHref.empty()) {
              int lid = (int)links.size() + 1;
              WebLink lk = {lid, currentHref};
              links.push_back(lk);
              char lnum[16];
              snprintf(lnum, sizeof(lnum), "[%d] ", lid);
              std::string linkLine = std::string(lnum) + decodeEntities(currentText);
              wrapAndAdd(linkLine, 0x1B, lid);
              currentText.clear();
            }
            inLink = false;
          }
        } else {
          tag += c;
        }
        continue;
      }

      if (inScript || inStyle) continue;

      if (c == '\n' || c == '\r' || c == '\t') c = ' ';
      if (c == ' ' && !currentText.empty() && currentText.back() == ' ') continue;
      currentText += c;
    }

    if (!currentText.empty()) {
      wrapAndAdd(decodeEntities(currentText), isHeading ? 0x1E : 0x70);
    }

    snprintf(statusMsg, sizeof(statusMsg), "Rendered %d lines, %d links", (int)lines.size(), (int)links.size());
  }

  // Fetch web page over HTTP/HTTPS
  void navigate(const char* target) {
    if (!target || !target[0]) return;
    strncpy(currentUrl, target, sizeof(currentUrl) - 1);
    top = 0;

    // 1. Check if Home requested
    if (strcasecmp(target, "home") == 0 || strcasecmp(target, "about:home") == 0) {
      loadHome();
      return;
    }

    // 2. Check if Search requested ('? query')
    if (target[0] == '?' || strncmp(target, "search ", 7) == 0) {
      const char* q = (target[0] == '?') ? target + 1 : target + 7;
      while (*q == ' ') q++;
      if (WiFi.status() == WL_CONNECTED) {
        // Online: Query FrogFind (fast text-only vintage search engine)
        char sUrl[140];
        String encodedQ = "";
        for (const char* p = q; *p; p++) {
          if (*p == ' ') encodedQ += '+';
          else if (isalnum((unsigned char)*p)) encodedQ += *p;
          else {
            char hex[4];
            snprintf(hex, sizeof(hex), "%%%02X", (unsigned char)*p);
            encodedQ += hex;
          }
        }
        snprintf(sUrl, sizeof(sUrl), "http://frogfind.com/?q=%s", encodedQ.c_str());
        navigate(sUrl);
        return;
      } else {
        // Offline: Search local LittleFS files
        searchLocal(q);
        return;
      }
    }

    // 3. Check if local file ('file:///path' or '/path')
    if (strncmp(target, "file://", 7) == 0) {
      loadLocalFile(target + 7);
      return;
    }
    if (target[0] == '/') {
      loadLocalFile(target);
      return;
    }

    // 4. Live Web Request over Wi-Fi
    if (WiFi.status() != WL_CONNECTED) {
      lines.clear();
      links.clear();
      snprintf(pageTitle, sizeof(pageTitle), "Network Offline");
      addLine("========================================================", 0x1E);
      addLine("           NETWORK ERROR: NOT CONNECTED TO WI-FI        ", 0x1F);
      addLine("========================================================", 0x1E);
      addLine("", 0x70);
      addLine("TensileOS is currently in Hotspot (AP) mode without", 0x78);
      addLine("external internet access.", 0x78);
      addLine("", 0x70);
      addLine("To browse the live web and use search engines:", 0x72);
      addLine("  1. Open 'Net Manager' on the desktop.", 0x70);
      addLine("  2. Select your home Wi-Fi and connect.", 0x70);
      addLine("  3. Reopen RetroWeb and surf the internet!", 0x70);
      addLine("", 0x70);
      addLinkLine(" [1] Return to RetroWeb Home Portal", "home");
      addLinkLine(" [2] Search Local Storage Files", "? ");
      snprintf(statusMsg, sizeof(statusMsg), "Wi-Fi not connected");
      return;
    }

    // Ensure URL has http:// or https://
    String fullUrl = target;
    if (!fullUrl.startsWith("http://") && !fullUrl.startsWith("https://")) {
      fullUrl = "http://" + fullUrl;
    }

    snprintf(statusMsg, sizeof(statusMsg), "Connecting to %s...", fullUrl.c_str());

    HTTPClient http;
    http.setTimeout(8000);
    http.setUserAgent("TensileOS-Web/1.0 (ESP32; 80x25 text)");

    bool isHttps = fullUrl.startsWith("https://");
    bool beginOk = false;

    if (isHttps) {
      WiFiClientSecure client;
      client.setInsecure(); // Skip certificate verification for speed & RAM savings
      beginOk = http.begin(client, fullUrl);
    } else {
      WiFiClient client;
      beginOk = http.begin(client, fullUrl);
    }

    if (!beginOk) {
      lines.clear();
      addLine("Error: Failed to initiate HTTP connection.", 0x74);
      snprintf(statusMsg, sizeof(statusMsg), "Connection failed");
      return;
    }

    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      http.end();
      parseAndRenderHtml(payload);
      snprintf(statusMsg, sizeof(statusMsg), "HTTP %d OK (%u bytes)", httpCode, (unsigned)payload.length());
    } else {
      lines.clear();
      char err[64];
      snprintf(err, sizeof(err), "HTTP Error: %s (code %d)", http.errorToString(httpCode).c_str(), httpCode);
      addLine(err, 0x74);
      addLine("Check that the URL is accessible and Wi-Fi is active.", 0x78);
      http.end();
      snprintf(statusMsg, sizeof(statusMsg), "HTTP request failed");
    }
  }

  void openSelectedLink() {
    if (focusedLink < 0 || focusedLink >= (int)links.size()) return;
    navigate(links[focusedLink].href.c_str());
  }

  void draw(View& v) override {
    v.fill(0x70);

    // Row 0: URL Bar
    v.fillRect(0, 0, v.w, 1, ' ', 0x1F);
    v.text(1, 0, "URL:", 0x1F);
    uint8_t uAttr = editingUrl ? 0x1E : 0x7F;
    v.fillRect(6, 0, v.w - 18, 1, ' ', uAttr);
    v.text(7, 0, editingUrl ? urlInput : currentUrl, uAttr);
    if (editingUrl) v.cursor(7 + urlLen, 0);

    // [Go] and [Home] buttons
    v.fillRect(v.w - 11, 0, 4, 1, ' ', 0x2F);
    v.text(v.w - 10, 0, "Go", 0x2F);
    v.fillRect(v.w - 6, 0, 5, 1, ' ', 0x3F);
    v.text(v.w - 6, 0, "Home", 0x3F);

    // Content area: rows 1 to v.h - 3
    int contentH = v.h - 3;
    for (int r = 0; r < contentH; r++) {
      int idx = top + r;
      if (idx >= (int)lines.size()) {
        v.fillRect(0, 1 + r, v.w, 1, ' ', 0x70);
        continue;
      }
      WebLine& wl = lines[idx];
      bool isFocused = (wl.linkId > 0 && wl.linkId == focusedLink + 1);
      uint8_t a = isFocused ? 0x1F : wl.attr;
      v.fillRect(0, 1 + r, v.w, 1, ' ', (a & 0xF0) ? a : (uint8_t)(0x70 | (a & 0x0F)));
      v.text(1, 1 + r, wl.text.c_str(), a);
    }

    // Scroll indicator / progress
    int maxTop = (int)lines.size() - contentH;
    if (maxTop > 0) {
      char sc[16];
      snprintf(sc, sizeof(sc), "[%d/%d]", top + 1, (int)lines.size());
      v.textr(v.w - 1, v.h - 2, sc, 0x78);
    }

    // Status bar (v.h - 2)
    v.fillRect(0, v.h - 2, v.w - 14, 1, ' ', 0x78);
    v.text(1, v.h - 2, statusMsg, 0x70);

    // Help Bar (v.h - 1)
    v.fillRect(0, v.h - 1, v.w, 1, ' ', 0x30);
    v.text(1, v.h - 1, "F: Address/Search  Tab/Arrows: Links  Enter: Open  Esc: Exit", 0x30);
  }

  void key(uint8_t k) override {
    if (editingUrl) {
      if (k == KEY_ENTER) {
        editingUrl = false;
        if (urlLen > 0) navigate(urlInput);
      } else if (k == KEY_ESC) {
        editingUrl = false;
      } else if ((k == KEY_BKSP || k == 127) && urlLen > 0) {
        urlInput[--urlLen] = 0;
      } else if (k >= 32 && k < 127 && urlLen < 120) {
        urlInput[urlLen++] = (char)k;
        urlInput[urlLen] = 0;
      }
      return;
    }

    if (k == 'f' || k == 'F' || k == '/') {
      editingUrl = true;
      strncpy(urlInput, currentUrl, sizeof(urlInput) - 1);
      urlLen = strlen(urlInput);
    } else if (k == 'h' || k == 'H') {
      loadHome();
    } else if (k == KEY_UP) {
      if (top > 0) top--;
    } else if (k == KEY_DOWN) {
      int maxTop = (int)lines.size() - (prefH - 3);
      if (top < maxTop) top++;
    } else if (k == KEY_PGUP) {
      top = (top >= 10) ? top - 10 : 0;
    } else if (k == KEY_PGDN) {
      int maxTop = (int)lines.size() - (prefH - 3);
      top = (top + 10 <= maxTop) ? top + 10 : (maxTop > 0 ? maxTop : 0);
    } else if (k == KEY_TAB) {
      if (!links.empty()) {
        focusedLink = (focusedLink + 1) % links.size();
        // Auto-scroll to focused link
        int targetId = focusedLink + 1;
        for (size_t i = 0; i < lines.size(); i++) {
          if (lines[i].linkId == targetId) {
            if ((int)i < top || (int)i >= top + (prefH - 3)) top = i;
            break;
          }
        }
      }
    } else if (k == KEY_ENTER) {
      openSelectedLink();
    } else if (k >= '1' && k <= '9') {
      int num = k - '0';
      if (num <= (int)links.size()) {
        focusedLink = num - 1;
        openSelectedLink();
      }
    } else if (k == KEY_ESC) {
      closeReq = true;
    }
  }

  void mouse(int mx, int my, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (!(edge & 1)) return;

    // Click on URL input field (row 0, x = 6..v.w - 12)
    if (my == 0 && mx >= 6 && mx < prefW - 12) {
      editingUrl = true;
      strncpy(urlInput, currentUrl, sizeof(urlInput) - 1);
      urlLen = strlen(urlInput);
      return;
    }

    // Click on [Go] button (row 0, x = v.w - 11..v.w - 7)
    if (my == 0 && mx >= prefW - 11 && mx < prefW - 7) {
      if (editingUrl && urlLen > 0) { editingUrl = false; navigate(urlInput); }
      else navigate(currentUrl);
      return;
    }

    // Click on [Home] button (row 0, x = v.w - 6..v.w - 1)
    if (my == 0 && mx >= prefW - 6) {
      editingUrl = false;
      loadHome();
      return;
    }

    // Click on content lines to activate links
    int contentH = prefH - 3;
    if (my >= 1 && my <= contentH) {
      int lineIdx = top + (my - 1);
      if (lineIdx < (int)lines.size() && lines[lineIdx].linkId > 0) {
        int lId = lines[lineIdx].linkId;
        focusedLink = lId - 1;
        openSelectedLink();
      }
    }
  }
};


#endif // TENSILE_NET_WEB_BROWSER_H
