// This software is part of OpenMono, see http://developer.openmono.com
// Released under the MIT license, see LICENSE.txt
#include "app_controller.h"

using mono::geo::Point;
using mono::geo::Rect;
using mono::String;
using mono::TouchEvent;
using mono::ui::TextLabelView;
using mono::ui::View;

void TouchField::repaint ()
{
	// Clear background.
	painter.drawFillRect(viewRect,true);
	// Show box around touch area.
	painter.drawRect(viewRect);
	// Draw the game piece.
	switch (app->board[boardY][boardX])
	{
		case AppController::X:
		{
			painter.drawLine(Position(),Point(viewRect.X2(),viewRect.Y2()));
			painter.drawLine(Point(viewRect.X2(),viewRect.Y()),Point(viewRect.X(),viewRect.Y2()));
			break;
		}
		case AppController::O:
		{
			uint16_t radius = viewRect.Size().Width() / 2;
			painter.drawCircle(viewRect.X()+radius,viewRect.Y()+radius,radius);
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
	topLabel(Rect(0,10,View::DisplayWidth(),20),"Tic Tac Toe")
{
	topLabel.setAlignment(TextLabelView::ALIGN_CENTER);
}

void AppController::monoWakeFromReset ()
{
	startNewGame();
}

void AppController::monoWakeFromSleep ()
{
	mono::IApplicationContext::SoftwareReset();
	startNewGame();
}

void AppController::monoWillGotoSleep ()
{
}

void AppController::startNewGame ()
{
	// Clear the board.
	for (uint8_t x = 0; x < 3; ++x)
		for (uint8_t y = 0; y < 3; ++y)
			board[y][x] = _;
	// Setup touch fields.
	const uint8_t width = View::DisplayWidth();
	const uint8_t height = View::DisplayHeight();
	const uint8_t fieldSize = 50;
	const uint8_t fieldSeparation = 8;
	const uint8_t screenMargin = (width-(3*fieldSize+2*fieldSeparation))/2;
	uint8_t yOffset = height-width-(fieldSeparation-screenMargin);
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
	updateView();
	whosMove();
 	if (hasWinner())
 	{
		if (winner() == X) topLabel.setText("X wins!");
		else topLabel.setText("O wins!");
	}
	else if (nextToMove == _) topLabel.setText("Tie!");
	else if (nextToMove == X) topLabel.setText("X to move");
	else topLabel.setText("O to move");
	topLabel.show();
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
	else if (board[y][x] != _) return;
	else board[y][x] = nextToMove;
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
