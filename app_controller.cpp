// This software is part of OpenMono, see http://developer.openmono.com
// Released under the MIT license, see LICENSE.txt
#include "app_controller.h"

using mono::display::IDisplayController;
using mono::geo::Point;
using mono::geo::Rect;
using mono::IApplicationContext;
using mono::String;
using mono::Timer;
using mono::TouchEvent;
using mono::ui::TextLabelView;
using mono::ui::View;

namespace
{
	static const int dimBrightness = 50;
};

const mono::display::Color TouchField::crossColor = mono::display::GreenColor;
const mono::display::Color TouchField::circleColor = mono::display::BlueColor;

void TouchField::repaint ()
{
	// Clear background.
	painter.setBackgroundColor(StandardBackgroundColor);
	painter.drawFillRect(viewRect,true);
	// Show box around touch area.
	painter.setForegroundColor(StandardBorderColor);
	painter.drawRect(viewRect);
	// Draw the game piece.
	switch (app->board[boardY][boardX])
	{
		case AppController::X:
		{
			painter.setForegroundColor(crossColor);
			painter.drawLine(Position(),Point(viewRect.X2(),viewRect.Y2()));
			painter.drawLine(Point(viewRect.X2(),Position().Y()),Point(Position().X(),viewRect.Y2()));
			break;
		}
		case AppController::O:
		{
			painter.setForegroundColor(circleColor);
			uint16_t radius = (viewRect.Size().Width()-2) / 2;
			painter.drawCircle(Position().X()+radius+1,Position().Y()+radius+1,radius);
			break;
		}
		default:
			break;
	}
}

void TouchField::TouchBegin (TouchEvent & event)
{
	app->humanMoved(boardX,boardY);
}

AppController::AppController ()
:
	timer(1500),
	dimmer(30*1000,true),
	sleeper(30*1000,true),
	topLabel(Rect(0,15,176,20),"Tic Tac Toe")
{
	dimmer.setCallback<AppController>(this,&AppController::dim);
	sleeper.setCallback(IApplicationContext::EnterSleepMode);
	topLabel.setAlignment(TextLabelView::ALIGN_CENTER);
}

void AppController::monoWakeFromReset ()
{
	startNewGame();
}

void AppController::monoWakeFromSleep ()
{
	continueGame();
}

void AppController::monoWillGotoSleep ()
{
	timer.Stop();
	dimmer.Stop();
	sleeper.Stop();
}

void AppController::dim ()
{
	dimmer.Stop();
	IDisplayController * display = IApplicationContext::Instance->DisplayController;
	for (int i = display->Brightness(); i >= dimBrightness; --i)
	{
		display->setBrightness(i);
		wait_ms(2);
	}
	sleeper.Start();
}

void AppController::startNewGame ()
{
	// Clear the board.
	for (uint8_t x = 0; x < 3; ++x)
		for (uint8_t y = 0; y < 3; ++y)
			board[y][x] = _;
	// Setup touch fields.
	const uint8_t screenHeight = 220;
	const uint8_t screenWidth = 176;
	const uint8_t fieldSize = 50;
	const uint8_t fieldSeparation = 8;
	const uint8_t screenMargin = (screenWidth-(3*fieldSize+2*fieldSeparation))/2;
	uint8_t yOffset = screenHeight-screenWidth-(fieldSeparation-screenMargin);
	for (uint8_t y = 0; y < 3; ++y)
	{
		yOffset += fieldSeparation;
		uint8_t xOffset = screenMargin;
		for (uint8_t x = 0; x < 3; ++x)
		{
			// Give each touch field enough info to paint itself.
			TouchField & field = fields[y][x];
			field.app = this;
			field.boardX = x;
			field.boardY = y;
			// Tell the view & touch system where the field is on the screen.
			field.setRect(Rect(xOffset,yOffset,fieldSize,fieldSize));
			// Next x position.
			xOffset += fieldSize + fieldSeparation;
		}
		// Next y position.
		yOffset += fieldSize;
	}
	continueGame();
}

void AppController::continueGame ()
{
	sleeper.Stop();
	dimmer.Start();
	IApplicationContext::Instance->DisplayController->setBrightness(255);
	updateView();
	whosMove();
	if (hasWinner())
	{
		if (winner() == X) topLabel.setText("You win!");
		else topLabel.setText("Mono wins!");
		timer.setCallback<AppController>(this,&AppController::prepareNewGame);
		timer.Start();
	}
	else if (nextToMove == _)
	{
		topLabel.setText("Tie!");
		timer.setCallback<AppController>(this,&AppController::prepareNewGame);
		timer.Start();
	}
	else if (nextToMove == X)
	{
		topLabel.setText("Your move");
		topLabel.show();
	}
	else
	{
		topLabel.setText("Thinking...");
		timer.setCallback<AppController>(this,&AppController::autoMove);
		timer.Start();
	}
}

void AppController::updateView ()
{
	for (uint8_t y = 0; y < 3; ++y)
		for (uint8_t x = 0; x < 3; ++x)
			fields[y][x].show();
}

AppController::Token AppController::winner ()
{
	if (hasThreeInRow(X)) return X;
	if (hasThreeInRow(O)) return O;
	return _;
}

bool AppController::hasThreeInRow (Token token)
{
	for (uint8_t x = 0; x < 3; ++x)
		if (board[0][x] == token &&
			board[1][x] == token &&
			board[2][x] == token
		) return true;
	for (uint8_t y = 0; y < 3; ++y)
		if (board[y][0] == token &&
			board[y][1] == token &&
			board[y][2] == token
		) return true;
	if (board[0][0] == token &&
		board[1][1] == token &&
		board[2][2] == token
	) return true;
	if (board[0][2] == token &&
		board[1][1] == token &&
		board[2][0] == token
	) return true;
	return false;
}

bool AppController::hasWinner ()
{
	return winner() != _;
}

void AppController::humanMoved (uint8_t x, uint8_t y)
{
	if (nextToMove == _ || hasWinner()) return startNewGame();
	else if (nextToMove != X || board[y][x] != _) return;
	board[y][x] = X;
	continueGame();
}

void AppController::whosMove ()
{
	uint8_t xPieces = 0;
	uint8_t oPieces = 0;
	for (uint8_t y = 0; y < 3; ++y)
		for (uint8_t x = 0; x < 3; ++x)
			if (board[y][x] == X) ++xPieces;
			else if (board[y][x] == O) ++oPieces;
	if (xPieces + oPieces >= 9) nextToMove = _;
	else if (xPieces <= oPieces) nextToMove = X;
	else nextToMove = O;
}

void AppController::autoMove ()
{
	timer.Stop();
	// Play to win, if possible.
	for (uint8_t x = 0; x < 3; ++x)
		for (uint8_t y = 0; y < 3; ++y)
			if (board[y][x] == _)
			{
				board[y][x] = O;
				if (hasWinner()) return continueGame();
				else board[y][x] = _;
			}
	// Play to not loose.
	for (uint8_t x = 0; x < 3; ++x)
		for (uint8_t y = 0; y < 3; ++y)
			if (board[y][x] == _)
			{
				board[y][x] = X;
				if (hasWinner())
				{
					board[y][x] = O;
					return continueGame();
				}
				else board[y][x] = _;
			}
	// Play where free.
	for (uint8_t x = 0; x < 3; ++x)
		for (uint8_t y = 0; y < 3; ++y)
			if (board[y][x] == _)
			{
				board[y][x] = O;
				return continueGame();
			}
}

void AppController::prepareNewGame ()
{
	timer.Stop();
	topLabel.setText("Play again?");
}
