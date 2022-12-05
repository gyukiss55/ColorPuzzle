/*
SolvePuzzle.h
*/
#pragma once


#ifndef _SOLVEPUZZLE_H_
#define _SOLVEPUZZLE_H_

#include <windows.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <memory>

#include "SmartPtr.h"


class TubeAction {
	static int mode;
	int tubeSize;
	int tubeNr;
	int from;
	int to;
	std::string actionState;
public:
	TubeAction();
	TubeAction(int si, int nr, const std::string& st);
	bool Next(TubeAction& ta, int f, int t) const;
	TubeAction Next() const;
	bool Solved() const;
	bool Valid() const;
	int GetFromIndex(int t) const;
	int GetToIndex(int t) const;
	int GetFrom() const { return from;  }
	int GetTo() const { return to; }
	std::string GetState() const { return actionState; }
	void ShowAction(HWND hWnd, HDC hDC);
};

class PuzzleTask {
	int tubeSize;
	int tubeNr;
	std::string puzzleStart;
	std::string puzzleState;
	std::list<TubeAction> actions;
	std::list<std::list<TubeAction>> solved;
	std::map<std::string, int> casheState;
	std::unique_ptr < std::vector<COLORREF>> vectorOfcolorPtr;
public:
	PuzzleTask();
	PuzzleTask(int s, int n, const std::string& puzzleSta, std::vector<COLORREF> * vectorOfcolorIn = nullptr);
	~PuzzleTask();
	bool SolveNextStep();
	bool SolveNextStep(int from, int to);
	bool Solve();
	int GetTubeSize() const { return tubeSize; }
	int GetTubeNr() const { return tubeNr; }
	void ShowState(HWND hWnd, HDC hDC);
	bool GetTubePosition(HWND hWnd, int& wTube, std::vector<int>& tubePosition) const;
	bool ValidState() const;
	void Report() const;
	void Reset();
	bool SolvedState ()const;

	bool Write(std::string& sta) const;
	static bool Read(const std::string& sta);

	static bool ManualSolvePuzzle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static bool ValidState(const std::string& sta, int tubeSize);
	static bool SolvedState(const std::string& sta, int tubeSize);
private:
	bool AppendEmptyTubes(const std::string& sta, std::string& newSta) const;
};

bool StaticLoopPuzzle(HWND hWnd, HDC hDC);
bool StaticShowPuzzle(HWND hWnd, HDC hDC);
void StaticResetPuzzle(HWND hWnd, HDC hDC);
bool StaticSetPuzzle(int tSize, int tNr, const std::string& start, std::vector<COLORREF>* vectorOfcolor = nullptr);
bool StaticLoadPuzzle(const std::wstring& filename);
bool StaticSavePuzzle(const std::wstring& filename);



#endif // _SOLVEPUZZLE_H_