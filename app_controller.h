// This software is part of OpenMono, see http://developer.openmono.com
// Released under the MIT license, see LICENSE.txt
#if !defined(__ttt_appcontroller_h)
#define __ttt_appcontroller_h
#include <mono.h>

class AppController;

class TouchField
:
	public mono::ui::ResponderView
{
	void TouchBegin (mono::TouchEvent & event);
	void repaint ();
public:
	AppController * app;
	uint8_t boardX;
	uint8_t boardY;
};

class AppController
:
	public mono::IApplication
{
public:
	AppController ();
	void monoWakeFromReset ();
	void monoWillGotoSleep ();
	void monoWakeFromSleep ();
	enum Token { _, X, O };
	Token board[3][3];
	void humanMoved (uint8_t x, uint8_t y);
private:
	mono::ui::TextLabelView topLabel;
	TouchField fields[3][3];
	Token nextToMove;
	bool hasThreeInRow (Token);
	void startNewGame ();
	void updateView ();
	void continueGame ();
	void whosMove ();
	Token winner ();
	bool hasWinner ();
};

#endif // __ttt_appcontroller_h
