#pragma once

#include "globals.h"
#include "iGraphics.h"
#include <windows.h>
#include <string.h>




/* =========================================================
STORY STAGE
========================================================= */

enum StoryStage
{
	STORY_NONE,
	STORY_INTRO,
	STORY_LEVEL1,
	STORY_ENDING,

	/* Level 2 -- caption + narration plays twice: once before
	   the level starts (the outpost briefing) and once after
	   the level is finished (all troops defeated). */
	STORY_LEVEL2_START,
	STORY_LEVEL2_END,

	/* Level 3 -- same pattern: once before the fort raid
	   begins, once after Lord Wright is defeated. */
	STORY_LEVEL3_START,
	STORY_LEVEL3_END,

	/* Level 4 -- final fort confrontation. */
	STORY_LEVEL4_START,
	STORY_LEVEL4_END
};


/* =========================================================
STORY VARIABLES
========================================================= */

static StoryStage storyStage = STORY_NONE;

static bool narrationStarted = false;

static unsigned long long storyStartTime = 0;

static const unsigned long long VOICE_DELAY = 1000;


/* =========================================================
CAPTIONS
========================================================= */

static const char* STORY_CAPTION_1 =

"BENGAL, 1757.\n\n"

"The land waits beneath the shadow of a coming war.\n\n"

"British soldiers gather beyond the borders... but their movements are strongly precise. Routes once known only to the Nawab's trusted men are somehow reaching the enemy.\n"
"Then, one night, a trusted scout disappears while searching for the truth. No body is found. No message is delivered...\n"
"Only a blade and a map bearing his final trail.\n\n"

"That scout was Kazi Rahmat. And the trail now belongs to his son.\n"
"SHAMSHER KAZI";


static const char* STORY_CAPTION_2 =

"The map leads Shamsher to a forgotten path near Bengal's border.\n\n"
"His father had marked this place only once... and then never spoke of it again.\n"
"Five British knights now stand between him and the answers he seeks.\n"
"They are not guarding gold. They are not protecting the land.\n"
"They are waiting for something...or someone.\n\n"

"Shamsher must pass through them before the truth disappears within the day.";


static const char* STORY_CAPTION_3 =

"The path finally falls silent.\n\n"

"Among the fallen guards, Shamsher finds a sealed British message. It speaks of weapons crossing Bengal without resistance.\n"
"The letter mentions that the patrol routes had already been revealed.\n"
"Someone within the Nawab's own forces is secretly feeding the information to the enemy. His father's disappearance was not an accident.\n\n"

"The enemy Shamsher seeks may already be standing inside Bengal.";


/* =========================================================
LEVEL 2 CAPTIONS
========================================================= */

static const char* STORY_CAPTION_LEVEL2_START =

"The coded map leads Shamsher to a forgotten village outpost.\n\n"

"Once used by the Nawab's scouts, it now stands under British control.\n"
"The stolen patrol routes were being passed through this place.\n\n"

"But Shamsher discovers something worse.\n"
"The British are not acting alone.\n"
"A high-ranking Bengali commander has been secretly providing them with information.\n\n"

"Shamsher enters the outpost.\n"
"He must find the evidence before the traitor erases every trace.";


static const char* STORY_CAPTION_LEVEL2_END =

"The outpost falls silent.\n\n"

"Among the British records, Shamsher discovers payments, orders, and a partially burned list of names.\n"
"One name has been torn away.\n"
"But the truth is clear.\n"
"A trusted commander within the Nawab's forces has been working with the British.\n\n"

"Then Shamsher finds something unexpected.\n"
"A British report mentions Kazi Rahmat.\n"
"His father had reached this outpost before he disappeared.\n\n"

"Shamsher realizes the trail is no longer about stolen information.\n"
"It is about what happened to his father.\n"
"And the trail leads to the Fort Raid.";


/* =========================================================
LEVEL 3 CAPTIONS
========================================================= */

static const char* STORY_CAPTION_LEVEL3_START =

"The trail leads Shamsher to a heavily guarded British fort.\n\n"

"According to the documents found at the outpost, the fort holds the answers behind the stolen patrol routes and his father's disappearance.\n\n"

"Somewhere within its walls lies the evidence Shamsher needs. But the deeper he ventures, the more he realizes that the British are protecting something far more valuable than weapons.\n\n"

"At the heart of the fort awaits Lord Wright, a powerful British commander entrusted with protecting its secrets.\n\n"

"Shamsher must fight his way through the fort, confront Lord Wright, and uncover what the British are hiding.";


static const char* STORY_CAPTION_LEVEL3_END =

"The fort falls silent.\n\n"

"Lord Wright lies defeated, but before his final breath, he refuses to reveal the truth.\n\n"

"Deep within his quarters, Shamsher discovers sealed documents revealing secret meetings, stolen information, and orders passed between the British and someone within the Nawab's forces.\n\n"

"But the most disturbing discovery is a familiar name.\n"
"Kazi Rahmat.\n"
"His father had reached this fort before he disappeared.\n\n"

"One final document remains sealed. Its contents could reveal everything.\n"
"Shamsher takes it and prepares for the final confrontation.";


/* =========================================================
LEVEL 4 CAPTIONS
========================================================= */

static const char* STORY_CAPTION_LEVEL4_START =

"The sealed document finally reveals the truth.\n\n"

"Someone within the Nawab's own forces has been secretly feeding vital information to the British.\n\n"

"And the betrayer is none other than Aziz Ali, the Commander-in-Chief of the Nawab's army.\n\n"

"But before Shamsher can reach him, Lord Huron stands in his way.\n\n"

"Shamsher must defeat Lord Huron and push deeper into the fort.\n"
"Beyond him awaits Aziz Ali.\n\n"

"The final battle begins.";


static const char* STORY_CAPTION_LEVEL4_END =

"Both Lord Huron and Aziz Ali have fallen.\n\n"

"The traitor's betrayal has finally been exposed. Aziz Ali, the Nawab's Commander-in-Chief, had secretly helped the British weaken Bengal from within.\n\n"

"Among his records, Shamsher finds his father's final message.\n\n"

"Kazi Rahmat had discovered Aziz Ali's betrayal before he disappeared and left the trail for his son to follow.\n\n"

"Shamsher finally understands why his father vanished.\n\n"

"The trail is over. The truth remains. And Bengal stands on the edge of war.";

/* =========================================================
STOP ALL SOUND
========================================================= */

inline void stopAllAudio()
{
	PlaySound(
		NULL,
		NULL,
		SND_PURGE
		);
}


/* =========================================================
START STORY
========================================================= */

inline void storyStart(StoryStage stage)
{
	storyStage = stage;

	narrationStarted = false;

	storyStartTime = GetTickCount64();


	/* Stop current music */

	stopAllAudio();


	/* Change game state */

	if (stage == STORY_INTRO)
	{
		currentState = STORY_INTRO_STATE;
	}

	else if (stage == STORY_LEVEL1)
	{
		currentState = STORY_LEVEL1_STATE;
	}

	else if (stage == STORY_ENDING)
	{
		currentState = STORY_ENDING_STATE;
	}

	else if (stage == STORY_LEVEL2_START)
	{
		currentState = STORY_LEVEL2_START_STATE;
	}

	else if (stage == STORY_LEVEL2_END)
	{
		currentState = STORY_LEVEL2_END_STATE;
	}

	else if (stage == STORY_LEVEL3_START)
	{
		currentState = STORY_LEVEL3_START_STATE;
	}

	else if (stage == STORY_LEVEL3_END)
	{
		currentState = STORY_LEVEL3_END_STATE;
	}

	else if (stage == STORY_LEVEL4_START)
	{
		currentState = STORY_LEVEL4_START_STATE;
	}

	else if (stage == STORY_LEVEL4_END)
	{
		currentState = STORY_LEVEL4_END_STATE;
	}
}


/* =========================================================
START NARRATION
========================================================= */

inline void startNarration()
{
	if (narrationStarted)
		return;


	narrationStarted = true;


	if (storyStage == STORY_INTRO)
	{
		PlaySound(
			TEXT("Audios\\ShamsherKazi.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}


	else if (storyStage == STORY_LEVEL1)
	{
		PlaySound(
			TEXT("Audios\\StartingLevel1.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}


	else if (storyStage == STORY_ENDING)
	{
		PlaySound(
			TEXT("Audios\\EndingLevel1.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}


	else if (storyStage == STORY_LEVEL2_START)
	{
		PlaySound(
			TEXT("Audios\\Level2Audio1.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}


	else if (storyStage == STORY_LEVEL2_END)
	{
		PlaySound(
			TEXT("Audios\\Level2Audio2.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}


	else if (storyStage == STORY_LEVEL3_START)
	{
		PlaySound(
			TEXT("Audios\\Level3Audio1.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}


	else if (storyStage == STORY_LEVEL3_END)
	{
		PlaySound(
			TEXT("Audios\\Level3Audio2.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}

	else if (storyStage == STORY_LEVEL4_START)
	{
		PlaySound(
			TEXT("Audios\\Level4Audio1.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}

	else if (storyStage == STORY_LEVEL4_END)
	{
		PlaySound(
			TEXT("Audios\\Level4Audio2.wav"),
			NULL,
			SND_FILENAME |
			SND_ASYNC
			);
	}
}


/* =========================================================
STORY UPDATE

VOICE STARTS AFTER EXACTLY 1 SECOND
========================================================= */

inline void storyUpdate()
{
	if (
		currentState != STORY_INTRO_STATE &&
		currentState != STORY_LEVEL1_STATE &&
		currentState != STORY_ENDING_STATE &&
		currentState != STORY_LEVEL2_START_STATE &&
		currentState != STORY_LEVEL2_END_STATE &&
		currentState != STORY_LEVEL3_START_STATE &&
		currentState != STORY_LEVEL3_END_STATE &&
		currentState != STORY_LEVEL4_START_STATE &&
		currentState != STORY_LEVEL4_END_STATE
		)
	{
		return;
	}


	unsigned long long currentTime =
		GetTickCount64();


	/* Wait 1 second */

	if (
		!narrationStarted &&
		currentTime - storyStartTime >= VOICE_DELAY
		)
	{
		startNarration();
	}
}


/* =========================================================
CONTINUE STORY

ENTER KEY
========================================================= */

inline void storyContinue()
{
	/*
	Player can press ENTER only
	after narration has started.
	*/

	if (!narrationStarted)
		return;


	/* Stop narration */

	stopAllAudio();


	/* =========================
	INTRO FINISHED
	========================= */

	if (currentState == STORY_INTRO_STATE)
	{
		storyStart(
			STORY_LEVEL1
			);

		return;
	}


	/* =========================
	LEVEL INTRO FINISHED
	========================= */

	if (currentState == STORY_LEVEL1_STATE)
	{
		storyStage = STORY_NONE;

		currentState = PLAYING;


		/* Start Level Music */

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\level 1.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}

		return;
	}


	/* =========================
	ENDING STORY FINISHED
	========================= */

	if (currentState == STORY_ENDING_STATE)
	{
		storyStage = STORY_NONE;

		currentState =
			LEVEL_COMPLETE;


		/* Interface music */

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\interface.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}

		return;
	}


	/* =========================
	LEVEL 2 BRIEFING FINISHED
	(outpost briefing -> gameplay)
	========================= */

	if (currentState == STORY_LEVEL2_START_STATE)
	{
		storyStage = STORY_NONE;

		currentState = LEVEL2_PLAYING;


		/* Start Level 2 Music */

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\level 1.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}

		return;
	}


	/* =========================
	LEVEL 2 ENDING FINISHED
	(outpost cleared -> level complete)
	========================= */

	if (currentState == STORY_LEVEL2_END_STATE)
	{
		storyStage = STORY_NONE;

		currentState = LEVEL_COMPLETE;


		/* Interface music */

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\interface.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}

		return;
	}


	/* =========================
	LEVEL 3 BRIEFING FINISHED
	(fort approach -> gameplay)
	========================= */

	if (currentState == STORY_LEVEL3_START_STATE)
	{
		storyStage = STORY_NONE;

		currentState = LEVEL3_PLAYING;


		/* Start Level 3 Music */

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\level 1.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}

		return;
	}


	/* =========================
	LEVEL 3 ENDING FINISHED
	(Lord Wright defeated -> level complete)
	========================= */

	if (currentState == STORY_LEVEL3_END_STATE)
	{
		storyStage = STORY_NONE;

		currentState = LEVEL_COMPLETE;


		/* Interface music */

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\interface.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}

		return;
	}


	/* =========================
	LEVEL 4 BRIEFING FINISHED
	(final fort -> gameplay)
	========================= */

	if (currentState == STORY_LEVEL4_START_STATE)
	{
		storyStage = STORY_NONE;

		currentState = LEVEL4_PLAYING;

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\level 1.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}

		return;
	}


	/* =========================
	LEVEL 4 ENDING FINISHED
	(final truth revealed -> complete screen)
	========================= */

	if (currentState == STORY_LEVEL4_END_STATE)
	{
		storyStage = STORY_NONE;

		currentState = LEVEL_COMPLETE;

		if (musicOn)
		{
			PlaySound(
				TEXT("Images\\interface.wav"),
				NULL,
				SND_FILENAME |
				SND_ASYNC |
				SND_LOOP
				);
		}
	}
}


/* =========================================================
GEORGIA TEXT DRAWER
========================================================= */

inline void storyDrawGeorgiaText(
	const char* text,
	int x,
	int y,
	int width,
	int height
	)
{
	HDC screenDC =
		wglGetCurrentDC();


	if (!screenDC)
		return;


	BITMAPINFO bmi;

	ZeroMemory(
		&bmi,
		sizeof(bmi)
		);


	bmi.bmiHeader.biSize =
		sizeof(BITMAPINFOHEADER);

	bmi.bmiHeader.biWidth =
		width;

	bmi.bmiHeader.biHeight =
		height;

	bmi.bmiHeader.biPlanes =
		1;

	bmi.bmiHeader.biBitCount =
		32;

	bmi.bmiHeader.biCompression =
		BI_RGB;


	void* pixels =
		NULL;


	HBITMAP bitmap =
		CreateDIBSection(
		screenDC,
		&bmi,
		DIB_RGB_COLORS,
		&pixels,
		NULL,
		0
		);


	if (!bitmap || !pixels)
		return;


	HDC dc =
		CreateCompatibleDC(
		screenDC
		);


	HGDIOBJ oldBitmap =
		SelectObject(
		dc,
		bitmap
		);


	RECT rect =
	{
		0,
		0,
		width,
		height
	};


	HBRUSH blackBrush =
		CreateSolidBrush(
		RGB(0, 0, 0)
		);


	FillRect(
		dc,
		&rect,
		blackBrush
		);


	DeleteObject(
		blackBrush
		);


	HFONT font =
		CreateFontA(

		20,
		0,
		0,
		0,

		FW_NORMAL,

		FALSE,
		FALSE,
		FALSE,

		ANSI_CHARSET,

		OUT_DEFAULT_PRECIS,

		CLIP_DEFAULT_PRECIS,

		CLEARTYPE_QUALITY,

		FF_DONTCARE,

		"Georgia"
		);


	HGDIOBJ oldFont =
		SelectObject(
		dc,
		font
		);


	SetBkMode(
		dc,
		TRANSPARENT
		);


	SetTextColor(
		dc,
		RGB(
		255,
		220,
		70
		)
		);


	RECT textRect =
	{
		0,
		0,
		width,
		height
	};


	DrawTextA(

		dc,

		text,

		-1,

		&textRect,

		DT_LEFT |
		DT_TOP |
		DT_WORDBREAK |
		DT_NOPREFIX
		);


	SelectObject(
		dc,
		oldFont
		);


	DeleteObject(
		font
		);


	unsigned char* p =
		(unsigned char*)pixels;


	int pixelCount =
		width * height;


	for (
		int i = 0;
		i < pixelCount;
	i++
		)
	{
		unsigned char b =
			p[i * 4 + 0];

		unsigned char g =
			p[i * 4 + 1];

		unsigned char r =
			p[i * 4 + 2];


		unsigned char alpha =
			0;


		if (
			r > 3 ||
			g > 3 ||
			b > 3
			)
		{
			alpha =
				255;
		}


		p[i * 4 + 3] =
			alpha;
	}


	glPushAttrib(

		GL_ENABLE_BIT |
		GL_COLOR_BUFFER_BIT |
		GL_CURRENT_BIT
		);


	glDisable(
		GL_TEXTURE_2D
		);


	glEnable(
		GL_BLEND
		);


	glBlendFunc(

		GL_SRC_ALPHA,

		GL_ONE_MINUS_SRC_ALPHA
		);


	glRasterPos2i(
		x,
		y
		);


	glDrawPixels(

		width,

		height,

		GL_RGBA,

		GL_UNSIGNED_BYTE,

		pixels
		);


	glDisable(
		GL_BLEND
		);


	glPopAttrib();


	SelectObject(
		dc,
		oldBitmap
		);


	DeleteDC(
		dc
		);


	DeleteObject(
		bitmap
		);
}


/* =========================================================
DRAW STORY BOX
========================================================= */

inline void drawStoryBox(
	const char* caption,
	const char* title
	)
{
	/* Dark background */

	iSetColor(
		8,
		8,
		8
		);


	iFilledRectangle(

		0,

		0,

		SCREEN_WIDTH,

		SCREEN_HEIGHT
		);


	/* Story box -- widened/heightened slightly from the
	original so the longer Level 2 captions have room to
	breathe without crowding the frame. */

	const int boxX =
		70;

	const int boxY =
		95;

	const int boxW =
		SCREEN_WIDTH - 140;

	const int boxH =
		430;


	iSetColor(
		25,
		18,
		12
		);


	iFilledRectangle(

		boxX,

		boxY,

		boxW,

		boxH
		);


	/* Subtle vertical vignette -- a soft darker band along the
	very top and bottom of the parchment, giving the box a
	slight sense of depth instead of one flat fill. */

	iSetColor(
		15,
		10,
		6
		);

	iFilledRectangle(
		boxX,
		boxY,
		boxW,
		18
		);

	iFilledRectangle(
		boxX,
		boxY + boxH - 18,
		boxW,
		18
		);


	iSetColor(
		180,
		130,
		45
		);


	iRectangle(

		boxX,

		boxY,

		boxW,

		boxH
		);


	iSetColor(
		105,
		75,
		35
		);


	iRectangle(

		boxX + 6,

		boxY + 6,

		boxW - 12,

		boxH - 12
		);


	/* Small gold corner accents -- a subtle ornamental touch
	on all four corners of the inner frame. */

	iSetColor(
		200,
		155,
		70
		);

	const int corner = 14;

	iFilledRectangle(boxX + 6, boxY + 6, corner, 3);
	iFilledRectangle(boxX + 6, boxY + 6, 3, corner);

	iFilledRectangle(boxX + boxW - 6 - corner, boxY + 6, corner, 3);
	iFilledRectangle(boxX + boxW - 9, boxY + 6, 3, corner);

	iFilledRectangle(boxX + 6, boxY + boxH - 9, corner, 3);
	iFilledRectangle(boxX + 6, boxY + boxH - 6 - corner, 3, corner);

	iFilledRectangle(boxX + boxW - 6 - corner, boxY + boxH - 9, corner, 3);
	iFilledRectangle(boxX + boxW - 9, boxY + boxH - 6 - corner, 3, corner);


	/* Title */

	storyDrawGeorgiaText(

		title,

		boxX + 35,

		boxY + boxH - 55,

		boxW - 70,

		45
		);


	/* Thin divider between the title and the caption text,
	just below where the title sits. */

	iSetColor(
		120,
		85,
		40
		);

	iFilledRectangle(
		boxX + 35,
		boxY + boxH - 65,
		boxW - 70,
		2
		);


	/* Caption */

	storyDrawGeorgiaText(

		caption,

		boxX + 35,

		boxY + 40,

		boxW - 70,

		boxH - 115
		);


	iSetColor(
		190,
		170,
		120
		);


	if (!narrationStarted)
	{
		iText(

			400,

			70,

			"THE STORY IS BEGINNING...",

			GLUT_BITMAP_HELVETICA_18
			);
	}

	else
	{
		iText(

			395,

			70,

			"PRESS ENTER TO CONTINUE",

			GLUT_BITMAP_HELVETICA_18
			);
	}
}


/* =========================================================
DRAW CURRENT STORY
========================================================= */

inline void drawStory()
{
	if (
		currentState ==
		STORY_INTRO_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_1,

			"SHAMSHER KAZI"
			);
	}


	else if (
		currentState ==
		STORY_LEVEL1_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_2,

			"MISSION 1 : THE HIDDEN TRAIL"
			);
	}


	else if (
		currentState ==
		STORY_ENDING_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_3,

			"THE HIDDEN MESSAGE"
			);
	}


	else if (
		currentState ==
		STORY_LEVEL2_START_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_LEVEL2_START,

			"MISSION 2 : THE OUTPOST"
			);
	}


	else if (
		currentState ==
		STORY_LEVEL2_END_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_LEVEL2_END,

			"THE FATHER'S TRAIL"
			);
	}


	else if (
		currentState ==
		STORY_LEVEL3_START_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_LEVEL3_START,

			"MISSION 3 : THE FORT RAID"
			);
	}


	else if (
		currentState ==
		STORY_LEVEL3_END_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_LEVEL3_END,

			"THE SEALED DOCUMENT"
			);
	}


	else if (
		currentState ==
		STORY_LEVEL4_START_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_LEVEL4_START,

			"MISSION 4 : THE FINAL FORT"
			);
	}


	else if (
		currentState ==
		STORY_LEVEL4_END_STATE
		)
	{
		drawStoryBox(

			STORY_CAPTION_LEVEL4_END,

			"THE FINAL TRUTH"
			);
	}
}
