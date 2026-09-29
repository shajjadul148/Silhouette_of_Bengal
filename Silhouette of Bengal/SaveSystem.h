#pragma once
/* =========================================================
   SaveSystem.h  --  File-based "database" for Silhouette of Bengal
   ---------------------------------------------------------
   Uses BOTH kinds of files (course requirement):

   1) BINARY file  "sob_save.dat"
        One profile PER PLAYER NAME (up to 20 players) plus the
        sound / music settings. Each profile keeps: unlocked
        level (= where LOAD GAME resumes), banked run score,
        best score per level, kills, deaths, games played and
        play time. Fixed-size struct with magic number, version
        and checksum (fwrite / fread).

   2) TEXT file    "sob_highscores.txt"
        Top-7 high score table, ONE line per player (that
        player's highest score), human readable (fprintf / fgets).

   How the menu uses it
     START GAME : always asks for a name (empty box). The name
                  is the player's profile; the run's score goes
                  on the score board under that name.
     LOAD GAME  : asks for a name. If a profile with that name
                  exists (not case sensitive) the game resumes
                  at that player's saved level.
     HIGH SCORE : shows the text table.

   Header-only. Include it in iMain.cpp AFTER iGraphics.h and
   globals.h.

   Safe on first run: if a file is missing, has the wrong size,
   wrong magic/version or a bad checksum, defaults are used and
   the game never crashes.
   ========================================================= */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "globals.h"

/* ---------- small portability wrappers (VS2013 secure CRT) ---------- */
#ifdef _MSC_VER
#define SV_SPRINTF sprintf_s
inline FILE* svOpen(const char* name, const char* mode)
{
	FILE* f = NULL;
	if (fopen_s(&f, name, mode) != 0) return NULL;
	return f;
}
inline void svLocalTime(time_t t, struct tm* out) { localtime_s(out, &t); }
#else
#define SV_SPRINTF snprintf
inline FILE* svOpen(const char* name, const char* mode) { return fopen(name, mode); }
inline void svLocalTime(time_t t, struct tm* out) { localtime_r(&t, out); }
#endif

/* =========================================================
   CONSTANTS
   ========================================================= */
#define SV_SAVE_FILE      "sob_save.dat"
#define SV_SCORE_FILE     "sob_highscores.txt"
#define SV_MAGIC          0x31424F53u      /* "SOB1" */
#define SV_VERSION        4
#define SV_MAX_LEVEL      4
#define SV_HS_MAX         7
#define SV_NAME_MAX       15
#define SV_MAX_PLAYERS    20

/* =========================================================
   1) BINARY SAVE  -  one profile per player + settings
   ========================================================= */
struct PlayerSave
{
	char name[SV_NAME_MAX + 1];        /* "" = empty slot               */
	int  highestLevelUnlocked;         /* 1..4  (LOAD GAME resumes here) */
	int  levelsCompleted;              /* how many level clears in total */
	int  bestScore[SV_MAX_LEVEL + 1];  /* index 1..4 used               */
	int  runScore;                     /* score banked from cleared levels
	                                      of the current run            */
	int  totalKills;
	int  totalDeaths;
	int  gamesPlayed;
	int  totalPlaySeconds;
};

struct SaveData
{
	unsigned int magic;
	int version;
	int soundOnFlag;
	int musicOnFlag;
	int playerCount;                   /* used slots, always packed at the front */
	int currentPlayer;                 /* index into players[], -1 = nobody      */
	PlayerSave players[SV_MAX_PLAYERS];
	unsigned int checksum;             /* must stay the LAST field      */
};

static SaveData   svData;
static PlayerSave svGuest;             /* used when no player is selected (never saved) */
static time_t     svSessionMark = 0;   /* last time play-time was added */
static int        svCarry = 0;         /* score carried from level 3 into level 4 */
static char       svMessage[64] = "";  /* last load / name error text    */

/* FNV-1a hash over every byte except the checksum field itself */
inline unsigned int svChecksum(const SaveData& d)
{
	const unsigned char* p = (const unsigned char*)&d;
	size_t n = sizeof(SaveData) - sizeof(unsigned int);
	unsigned int h = 2166136261u;
	for (size_t i = 0; i < n; i++)
	{
		h ^= p[i];
		h *= 16777619u;
	}
	return h;
}

inline void svClampInt(int& v, int lo, int hi)
{
	if (v < lo) v = lo;
	if (v > hi) v = hi;
}

inline void svInitProfile(PlayerSave& p, const char* name)
{
	memset(&p, 0, sizeof(PlayerSave));
	p.highestLevelUnlocked = 1;
	for (int i = 0; i < SV_NAME_MAX && name[i] != '\0'; i++) p.name[i] = name[i];
}

inline void saveSetDefaults()
{
	memset(&svData, 0, sizeof(SaveData));
	svData.magic = SV_MAGIC;
	svData.version = SV_VERSION;
	svData.soundOnFlag = 1;
	svData.musicOnFlag = 1;
	svData.playerCount = 0;
	svData.currentPlayer = -1;
	svInitProfile(svGuest, "");
}

/* the profile that all stats / progress go to */
inline PlayerSave& svCur()
{
	if (svData.currentPlayer >= 0 && svData.currentPlayer < svData.playerCount)
		return svData.players[svData.currentPlayer];
	return svGuest;
}

/* add the seconds played since the last flush */
inline void svFlushPlayTime()
{
	time_t now = time(NULL);
	if (svSessionMark != 0 && now > svSessionMark)
	{
		long add = (long)(now - svSessionMark);
		if (add > 0 && add < 86400)      /* ignore clock jumps */
			svCur().totalPlaySeconds += (int)add;
	}
	svSessionMark = now;
}

/* write the binary file; returns true on success */
inline bool saveWrite()
{
	svFlushPlayTime();
	svData.magic = SV_MAGIC;
	svData.version = SV_VERSION;
	svData.checksum = svChecksum(svData);

	FILE* f = svOpen(SV_SAVE_FILE, "wb");
	if (f == NULL) return false;
	size_t ok = fwrite(&svData, sizeof(SaveData), 1, f);
	fclose(f);
	return ok == 1;
}

/* read the binary file; on ANY problem fall back to defaults */
inline bool saveLoad()
{
	static SaveData tmp;                 /* static: the struct is fairly big */
	FILE* f = svOpen(SV_SAVE_FILE, "rb");
	if (f == NULL) { saveSetDefaults(); return false; }

	size_t got = fread(&tmp, sizeof(SaveData), 1, f);
	/* the file must be exactly one struct long */
	int extra = fgetc(f);
	fclose(f);

	if (got != 1 || extra != EOF ||
		tmp.magic != SV_MAGIC || tmp.version != SV_VERSION ||
		tmp.checksum != svChecksum(tmp))
	{
		saveSetDefaults();
		return false;
	}

	svData = tmp;
	svInitProfile(svGuest, "");
	svClampInt(svData.playerCount, 0, SV_MAX_PLAYERS);
	svClampInt(svData.currentPlayer, -1, svData.playerCount - 1);
	svData.soundOnFlag = (svData.soundOnFlag != 0);
	svData.musicOnFlag = (svData.musicOnFlag != 0);

	/* range checks, in case the values are still nonsense */
	for (int k = 0; k < svData.playerCount; k++)
	{
		PlayerSave& p = svData.players[k];
		p.name[SV_NAME_MAX] = '\0';
		svClampInt(p.highestLevelUnlocked, 1, SV_MAX_LEVEL);
		svClampInt(p.levelsCompleted, 0, 1000000);
		svClampInt(p.runScore, 0, 100000000);
		svClampInt(p.totalKills, 0, 100000000);
		svClampInt(p.totalDeaths, 0, 100000000);
		svClampInt(p.gamesPlayed, 0, 100000000);
		svClampInt(p.totalPlaySeconds, 0, 2000000000);
		for (int i = 0; i <= SV_MAX_LEVEL; i++)
			svClampInt(p.bestScore[i], 0, 100000000);
	}
	return true;
}

/* =========================================================
   2) TEXT HIGH SCORES  -  top 7, one line per player
   ---------------------------------------------------------
   File layout:
       SOB_HIGHSCORES 1
       NAME SCORE LEVEL YYYY-MM-DD
       ...
   ========================================================= */
struct HighScoreEntry
{
	char name[SV_NAME_MAX + 1];
	int  score;
	int  level;
	char date[11];
};

static HighScoreEntry hsTable[SV_HS_MAX];
static int hsCount = 0;

inline void svTodayString(char* out, int size)
{
	struct tm t;
	svLocalTime(time(NULL), &t);
	SV_SPRINTF(out, size, "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
}

/* names are stored without spaces so the line stays easy to parse */
inline void svCleanName(const char* in, char* out)
{
	int n = 0;
	for (int i = 0; in[i] != '\0' && n < SV_NAME_MAX; i++)
	{
		char c = in[i];
		if (c == ' ') c = '_';
		if (c > 32 && c < 127) out[n++] = c;
	}
	out[n] = '\0';
	if (n == 0) { memcpy(out, "PLAYER", 7); }
}

inline char svLower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

/* trimmed, case-insensitive name comparison */
inline bool svSameName(const char* a, const char* b)
{
	int a0 = 0, a1 = (int)strlen(a), b0 = 0, b1 = (int)strlen(b);
	while (a0 < a1 && a[a0] == ' ') a0++;
	while (a1 > a0 && a[a1 - 1] == ' ') a1--;
	while (b0 < b1 && b[b0] == ' ') b0++;
	while (b1 > b0 && b[b1 - 1] == ' ') b1--;
	if (a1 - a0 != b1 - b0) return false;
	for (int i = 0; i < a1 - a0; i++)
		if (svLower(a[a0 + i]) != svLower(b[b0 + i])) return false;
	return true;
}

inline bool hsWriteFile()
{
	FILE* f = svOpen(SV_SCORE_FILE, "w");
	if (f == NULL) return false;
	fprintf(f, "SOB_HIGHSCORES 1\n");
	for (int i = 0; i < hsCount; i++)
		fprintf(f, "%s %d %d %s\n", hsTable[i].name, hsTable[i].score,
			hsTable[i].level, hsTable[i].date);
	fclose(f);
	return true;
}

/* copy the next space-separated word from *p into out; returns false at end of line */
inline bool svNextWord(const char*& p, char* out, int outSize)
{
	while (*p == ' ' || *p == '\t') p++;
	if (*p == '\0' || *p == '\n' || *p == '\r') return false;
	int n = 0;
	while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r')
	{
		if (n < outSize - 1) out[n++] = *p;
		p++;
	}
	out[n] = '\0';
	return true;
}

inline void hsLoad()
{
	hsCount = 0;
	FILE* f = svOpen(SV_SCORE_FILE, "r");
	if (f == NULL) return;

	char line[128];
	if (fgets(line, sizeof(line), f) == NULL ||
		strncmp(line, "SOB_HIGHSCORES", 14) != 0)
	{
		fclose(f);
		return;                              /* not our file -> ignore */
	}

	while (hsCount < SV_HS_MAX && fgets(line, sizeof(line), f) != NULL)
	{
		const char* p = line;
		char w1[32], w2[32], w3[32], w4[32];
		if (!svNextWord(p, w1, sizeof(w1))) continue;
		if (!svNextWord(p, w2, sizeof(w2))) continue;
		if (!svNextWord(p, w3, sizeof(w3))) continue;
		if (!svNextWord(p, w4, sizeof(w4))) continue;

		int sc = atoi(w2), lv = atoi(w3);
		if (sc < 0 || sc > 100000000) continue;
		if (lv < 1 || lv > SV_MAX_LEVEL) continue;

		svCleanName(w1, hsTable[hsCount].name);
		hsTable[hsCount].score = sc;
		hsTable[hsCount].level = lv;
		memset(hsTable[hsCount].date, 0, sizeof(hsTable[hsCount].date));
		for (int k = 0; k < 10 && w4[k] != '\0'; k++) hsTable[hsCount].date[k] = w4[k];
		hsCount++;
	}
	fclose(f);

	/* keep the table sorted even if someone edited the file by hand */
	for (int i = 0; i < hsCount - 1; i++)
		for (int j = i + 1; j < hsCount; j++)
			if (hsTable[j].score > hsTable[i].score)
			{
				HighScoreEntry t = hsTable[i];
				hsTable[i] = hsTable[j];
				hsTable[j] = t;
			}
}

/* would this score make it into the table? (ignores the same-player rule) */
inline bool hsQualifies(int sc)
{
	if (sc <= 0) return false;
	if (hsCount < SV_HS_MAX) return true;
	return sc > hsTable[hsCount - 1].score;
}

/* Put a score on the board. A player only ever has ONE line: their
   highest score. A lower score by the same name is ignored, a higher
   one replaces the old line.
   returns the rank (0 = first place) or -1 if nothing changed */
inline int hsAdd(const char* name, int sc, int lvl)
{
	if (sc <= 0) return -1;

	char clean[SV_NAME_MAX + 1];
	svCleanName(name, clean);

	int existing = -1;
	for (int i = 0; i < hsCount; i++)
		if (svSameName(hsTable[i].name, clean)) { existing = i; break; }

	if (existing >= 0)
	{
		if (sc <= hsTable[existing].score) return -1;   /* not their best */
		for (int i = existing; i < hsCount - 1; i++)    /* drop the old line */
			hsTable[i] = hsTable[i + 1];
		hsCount--;
	}
	else if (!hsQualifies(sc))
		return -1;

	int pos = hsCount;                       /* ties go below older scores */
	for (int i = 0; i < hsCount; i++)
		if (sc > hsTable[i].score) { pos = i; break; }

	int last = (hsCount < SV_HS_MAX) ? hsCount : SV_HS_MAX - 1;
	for (int i = last; i > pos; i--)
		hsTable[i] = hsTable[i - 1];

	memcpy(hsTable[pos].name, clean, sizeof(hsTable[pos].name));
	hsTable[pos].score = sc;
	hsTable[pos].level = lvl;
	svTodayString(hsTable[pos].date, sizeof(hsTable[pos].date));
	if (hsCount < SV_HS_MAX) hsCount++;

	hsWriteFile();
	return pos;
}

inline void hsClear()
{
	hsCount = 0;
	hsWriteFile();
}

/* draw the leaderboard (call inside your draw function of a HIGHSCORE screen) */
inline void hsDraw(int x, int y)
{
	iSetColor(255, 255, 255);
	iText(x, y, "RANK   NAME              SCORE    LEVEL   DATE", GLUT_BITMAP_HELVETICA_18);
	if (hsCount == 0)
	{
		iText(x, y - 40, "No scores yet. Go fight!", GLUT_BITMAP_HELVETICA_18);
		return;
	}
	for (int i = 0; i < hsCount; i++)
	{
		char row[128];
		SV_SPRINTF(row, sizeof(row), "%d.       %-15s   %-7d  %-6d  %s",
			i + 1, hsTable[i].name, hsTable[i].score, hsTable[i].level, hsTable[i].date);
		iText(x, y - 40 - i * 30, row, GLUT_BITMAP_HELVETICA_18);
	}
}

/* =========================================================
   PLAYER NAME INPUT
   ---------------------------------------------------------
   nameInputReset()      when the name screen opens
   nameInputKey(key)     from iKeyboard; returns true when ENTER
                         is pressed with at least 1 character
   nameInputGet()        the typed name
   nameInputDraw(x, y)   draws "NAME: abc_"
   ========================================================= */
static char svNameBuf[SV_NAME_MAX + 1] = "";
static int  svNameLen = 0;

inline void nameInputReset() { svNameBuf[0] = '\0'; svNameLen = 0; }

inline bool nameInputKey(unsigned char key)
{
	if (key == 13)                                   /* ENTER */
	{
		for (int i = 0; i < svNameLen; i++)
			if (svNameBuf[i] != ' ') return true;   /* needs a real letter */
		return false;
	}
	if (key == 8)                                    /* BACKSPACE */
	{
		if (svNameLen > 0) svNameBuf[--svNameLen] = '\0';
		return false;
	}
	bool ok = (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') ||
		(key >= '0' && key <= '9') || key == '_' || key == ' ';
	if (ok && svNameLen < SV_NAME_MAX)
	{
		svNameBuf[svNameLen++] = (char)key;
		svNameBuf[svNameLen] = '\0';
	}
	return false;
}

inline const char* nameInputGet() { return svNameBuf; }

/* put an existing name into the text box */
inline void nameInputSet(const char* name)
{
	nameInputReset();
	for (int i = 0; name[i] != '\0' && svNameLen < SV_NAME_MAX; i++)
		svNameBuf[svNameLen++] = name[i];
	svNameBuf[svNameLen] = '\0';
}

inline void nameInputDraw(int x, int y)
{
	char line[64];
	SV_SPRINTF(line, sizeof(line), "NAME: %s_", svNameBuf);
	iSetColor(255, 255, 255);
	iText(x, y, line, GLUT_BITMAP_HELVETICA_18);
}

/* =========================================================
   PLAYER PROFILES  (Start Game / Load Game)
   ========================================================= */
inline int svFindPlayer(const char* name)
{
	for (int i = 0; i < svData.playerCount; i++)
		if (svSameName(svData.players[i].name, name)) return i;
	return -1;
}

inline bool saveHasPlayerName() { return svData.currentPlayer >= 0 && svData.currentPlayer < svData.playerCount; }
inline const char* saveGetPlayerName() { return saveHasPlayerName() ? svData.players[svData.currentPlayer].name : "PLAYER"; }
inline int  saveGetPlayerCount() { return svData.playerCount; }
inline const char* saveGetPlayerNameAt(int i) { return (i >= 0 && i < svData.playerCount) ? svData.players[i].name : ""; }

inline const char* saveGetMessage() { return svMessage; }
inline void saveClearMessage() { svMessage[0] = '\0'; }

/* START GAME: trims the name, selects the player's profile (creating it the
   first time), and writes sob_save.dat. An existing name keeps its progress. */
inline void saveSetPlayerName(const char* name)
{
	char trimmed[SV_NAME_MAX + 1];
	int a = 0, b = (int)strlen(name), n = 0;
	while (a < b && name[a] == ' ') a++;
	while (b > a && name[b - 1] == ' ') b--;
	for (int i = a; i < b && n < SV_NAME_MAX; i++) trimmed[n++] = name[i];
	trimmed[n] = '\0';
	if (n == 0) return;

	svFlushPlayTime();                                 /* time goes to the previous player */
	int idx = svFindPlayer(trimmed);
	if (idx < 0)
	{
		if (svData.playerCount < SV_MAX_PLAYERS)
			idx = svData.playerCount++;
		else                                           /* full: replace the least advanced player */
		{
			idx = 0;
			for (int i = 1; i < svData.playerCount; i++)
			{
				const PlayerSave& c = svData.players[i];
				const PlayerSave& w = svData.players[idx];
				if (c.highestLevelUnlocked < w.highestLevelUnlocked ||
					(c.highestLevelUnlocked == w.highestLevelUnlocked && c.levelsCompleted < w.levelsCompleted))
					idx = i;
			}
		}
		svInitProfile(svData.players[idx], trimmed);
	}
	svData.currentPlayer = idx;
	svCarry = 0;
	saveClearMessage();
	saveWrite();
}

/* LOAD GAME: select the profile with this name. Returns false (and sets
   saveGetMessage()) when nobody saved under that name. */
inline bool saveLoadPlayer(const char* name)
{
	int idx = svFindPlayer(name);
	if (idx < 0)
	{
		SV_SPRINTF(svMessage, sizeof(svMessage), "No saved game found for this name.");
		return false;
	}
	svFlushPlayTime();
	svData.currentPlayer = idx;
	svCarry = 0;
	saveClearMessage();
	saveWrite();
	return true;
}

/* small list of the saved players (for the LOAD GAME name screen) */
inline void saveDrawPlayerList(int x, int y)
{
	iSetColor(255, 235, 80);
	if (svData.playerCount == 0)
	{
		iText(x, y, "No saved players yet.", GLUT_BITMAP_HELVETICA_12);
		return;
	}
	iText(x, y, "Saved players:", GLUT_BITMAP_HELVETICA_12);
	iSetColor(255, 255, 255);
	for (int i = 0; i < svData.playerCount && i < 10; i++)
	{
		char row[64];
		SV_SPRINTF(row, sizeof(row), "%s  (level %d)",
			svData.players[i].name, svData.players[i].highestLevelUnlocked);
		iText(x, y - 18 - i * 16, row, GLUT_BITMAP_HELVETICA_12);
	}
}

/* =========================================================
   PUBLIC API  -  the functions the rest of the game calls
   ========================================================= */

/* call ONCE at startup (in main, before the music starts) */
inline void saveSystemInit()
{
	saveLoad();
	hsLoad();
	svSessionMark = time(NULL);
	soundOn = (svData.soundOnFlag != 0);      /* restore settings */
	musicOn = (svData.musicOnFlag != 0);
}

/* call after the sound / music toggle changes */
inline void saveSettings()
{
	svData.soundOnFlag = soundOn ? 1 : 0;
	svData.musicOnFlag = musicOn ? 1 : 0;
	saveWrite();
}

/* call when a new run starts (START GAME): banked run score goes back to 0 */
inline void saveOnGameStart()
{
	PlayerSave& p = svCur();
	p.gamesPlayed++;
	p.runScore = 0;
	svCarry = 0;
	saveWrite();
}

/* the score of the whole run so far, including the level being played.
   Level 3 -> 4 does not reset `score` in the game, so the carried part
   is not counted twice. */
inline int saveRunTotal(int lvl, int sc)
{
	int cur = sc;
	if (lvl == SV_MAX_LEVEL) cur = sc - svCarry;
	if (cur < 0) cur = 0;
	return svCur().runScore + cur;
}

/* put the finished run on the score board (one line per player: best score) */
inline void saveRecordRun(int lvl, int sc)
{
	hsAdd(saveGetPlayerName(), saveRunTotal(lvl, sc), lvl);
}

/* call when a level is cleared: banks the score, unlocks the next level
   (= the LOAD GAME checkpoint) and keeps the best score. Clearing the
   last level finishes the run and records it on the score board. */
inline void saveOnLevelComplete(int lvl, int sc)
{
	if (lvl < 1 || lvl > SV_MAX_LEVEL) return;
	PlayerSave& p = svCur();
	int total = saveRunTotal(lvl, sc);
	p.levelsCompleted++;
	if (sc > p.bestScore[lvl]) p.bestScore[lvl] = sc;
	if (lvl < SV_MAX_LEVEL && p.highestLevelUnlocked < lvl + 1)
		p.highestLevelUnlocked = lvl + 1;

	/* every cleared level goes on the score board right away, so the
	   score is kept even if the player never finishes the run */
	hsAdd(saveGetPlayerName(), total, lvl);

	if (lvl == SV_MAX_LEVEL)
	{
		p.runScore = 0;
		svCarry = 0;
	}
	else
	{
		p.runScore = total;
		svCarry = (lvl == 3) ? sc : 0;
	}
	saveWrite();
}

/* call when the player leaves a run early (pause -> main menu): the score
   so far goes on the score board. Progress (unlocked level, banked score)
   is kept so LOAD GAME can still resume. Not counted as a death. */
inline void saveOnQuitRun(int lvl, int sc)
{
	hsAdd(saveGetPlayerName(), saveRunTotal(lvl, sc), lvl);
	saveWrite();
}

/* call from the in-level SAVE & EXIT button: saves the level being played
   as the LOAD GAME checkpoint. It NEVER unlocks the next level (only
   saveOnLevelComplete does that) and is not counted as a level clear. */
inline void saveCurrentLevel(int lvl, int sc)
{
	if (lvl < 1 || lvl > SV_MAX_LEVEL) return;
	PlayerSave& p = svCur();
	if (p.highestLevelUnlocked < lvl) p.highestLevelUnlocked = lvl;
	hsAdd(saveGetPlayerName(), saveRunTotal(lvl, sc), lvl);
	saveWrite();
}

/* call when the hero dies: the run ends, its total goes on the score board.
   returns true if it changed the score board */
inline bool saveOnGameOver(int lvl, int sc)
{
	PlayerSave& p = svCur();
	p.totalDeaths++;
	int total = saveRunTotal(lvl, sc);
	int rank = hsAdd(saveGetPlayerName(), total, lvl);
	p.runScore = 0;
	svCarry = 0;
	saveWrite();
	return rank >= 0;
}

/* optional: call from the enemy death code (one line in enemy files) */
inline void saveStatAddKill()
{
	svCur().totalKills++;                     /* written at next save */
}

/* ---- queries ---- */
inline bool saveIsLevelUnlocked(int lvl)   { return lvl >= 1 && lvl <= svCur().highestLevelUnlocked; }
inline bool saveHasProgress()              { return svData.playerCount > 0; }
inline int  saveGetContinueLevel()         { return svCur().highestLevelUnlocked; }
inline int  saveGetBestScore(int lvl)      { return (lvl >= 1 && lvl <= SV_MAX_LEVEL) ? svCur().bestScore[lvl] : 0; }
inline int  saveGetTotalKills()            { return svCur().totalKills; }
inline int  saveGetTotalDeaths()           { return svCur().totalDeaths; }
inline int  saveGetGamesPlayed()           { return svCur().gamesPlayed; }
inline int  saveGetPlaySeconds()           { svFlushPlayTime(); return svCur().totalPlaySeconds; }

/* draw a small stats panel for the current player */
inline void saveDrawStats(int x, int y)
{
	char row[96];
	int s = saveGetPlaySeconds();
	const PlayerSave& p = svCur();
	iSetColor(255, 255, 255);
	SV_SPRINTF(row, sizeof(row), "Levels unlocked : %d / %d", p.highestLevelUnlocked, SV_MAX_LEVEL);
	iText(x, y, row, GLUT_BITMAP_HELVETICA_18);
	SV_SPRINTF(row, sizeof(row), "Games played    : %d", p.gamesPlayed);
	iText(x, y - 30, row, GLUT_BITMAP_HELVETICA_18);
	SV_SPRINTF(row, sizeof(row), "Enemies killed  : %d", p.totalKills);
	iText(x, y - 60, row, GLUT_BITMAP_HELVETICA_18);
	SV_SPRINTF(row, sizeof(row), "Deaths          : %d", p.totalDeaths);
	iText(x, y - 90, row, GLUT_BITMAP_HELVETICA_18);
	SV_SPRINTF(row, sizeof(row), "Play time       : %d min %d sec", s / 60, s % 60);
	iText(x, y - 120, row, GLUT_BITMAP_HELVETICA_18);
}

/* ---- reset the CURRENT player's progress ("Erase data"); name and settings stay ---- */
inline void saveResetProgress()
{
	PlayerSave& p = svCur();
	char keepName[SV_NAME_MAX + 1];
	memcpy(keepName, p.name, sizeof(keepName));
	svInitProfile(p, keepName);
	svCarry = 0;
	svSessionMark = time(NULL);
	saveWrite();
}
