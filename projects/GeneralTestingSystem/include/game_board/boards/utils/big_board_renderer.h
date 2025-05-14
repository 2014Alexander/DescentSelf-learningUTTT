// big_board_renderer.h
#pragma once
class RefereeBoard;
class BigBoard;

/**
* @brief Renderuje (rysuje) dużą planszę Ultimate Tic-Tac-Toe w konsoli z dodatkową informacją.
*
* @param bigBoard Obiekt BigBoard, reprezentujący stan gry.
*/
void drawBigBoard(BigBoard &bigBoard, RefereeBoard &refereeBoard);