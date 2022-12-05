/*
SolvePuzzle.cpp
*/

#include <map>
#include <objidl.h>
#include <gdiplus.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#include "SolvePuzzle.h"

#include "FileIO.h"

/*
static
*/

static int fromIndex = -1;

static PuzzleTask task0(4, 11, std::string(
	"PLNP"
	"LBZZ"
	"NBKB"
	"FNKL"
	"EPFZ"
	"NZPL"
	"FSSF"
	"KSEB"
	"SKEE"));

static PuzzleTask* taskPtr = nullptr;
static PuzzleTask* taskPtr2 = nullptr;

bool StaticLoadPuzzle(const std::wstring& filename)
{
	std::string stream;

	FileInput fileIO;
	fileIO.Open(filename.c_str());
	fileIO.Read(stream);
	fileIO.Close();
	if (taskPtr2->Read(stream))
		return true;

	return false;
}


bool StaticSavePuzzle(const std::wstring& filename)
{
	if (taskPtr2 == nullptr)
		return false;
	std::string stream;
	if (!taskPtr2->Write(stream)) {

		return false;
	}
	FileOutput fileIO;
	fileIO.Open(filename.c_str());
	fileIO.Write(stream);
	fileIO.Close();
	return true;
}


bool StaticSetPuzzle(int tSize, int tNr, const std::string& start, std::vector<COLORREF>* vectorOfcolor)
{
	delete taskPtr2;

	std::string s = start;
	taskPtr2 = new PuzzleTask(tSize, tNr, s, vectorOfcolor);

	taskPtr = taskPtr2;
	return taskPtr != nullptr;
}


bool StaticLoopPuzzle(HWND hWnd, HDC hDC)
{
	if (taskPtr == 0)
		return false;
	if (taskPtr->ValidState()) {
		taskPtr->ShowState(hWnd, hDC);
		taskPtr->Report();
		taskPtr->SolveNextStep();
		if (taskPtr->ValidState()) {
			taskPtr->ShowState(hWnd, hDC);
			taskPtr->Report();
			if (taskPtr->SolvedState()) {
				Graphics graphics(hDC);
				SolidBrush  brush(Color(255, 0, 0, 255));
				FontFamily  fontFamily(L"Times New Roman");
				Font        font(&fontFamily, 32, FontStyleRegular, UnitPixel);
				PointF      pointF(1110.0f, 660.0f);
				graphics.DrawString(L"       Solved!      ", -1, &font, pointF, &brush);

			}
			else {
				Graphics graphics(hDC);
				SolidBrush  brush(Color(255, 255, 255, 255));
				graphics.FillRectangle(&brush, 1100, 600, 300, 120);
			}
			return true;
		}
	}
	return false;
}

bool StaticShowPuzzle(HWND hWnd, HDC hDC)
{
	if (taskPtr == nullptr)
		return false;
	if (taskPtr->ValidState()) {
		taskPtr->ShowState(hWnd, hDC);
		taskPtr->Report();
		return true;
	}
	return false;
}

void StaticResetPuzzle(HWND hWnd, HDC hDC)
{
	if (taskPtr == nullptr)
		return;
	taskPtr->Reset();
	taskPtr->ShowState(hWnd, hDC);
	taskPtr->Report();
	return;
}


/*
TubeAction
*/

int TubeAction::mode (1);

TubeAction::TubeAction() : tubeSize(0), tubeNr(0), from(0), to(0)
{

}

TubeAction::TubeAction(int si, int nr, const std::string& st) : tubeSize(si), tubeNr(nr), from(0), to(0), actionState(st)
{
	if (!Valid())
		actionState.clear();
}

int TubeAction::GetFromIndex(int t) const
{
	for (int index = (t + 1) * tubeSize - 1; index >= t * tubeSize; --index) {
		if (actionState[index] == '0')
			continue;
		char ch = actionState[index];
		int index2 = tubeSize * t;
		if (index - index2 > tubeSize / 2) {
			for (; index2 < index; ++index2) {
				if (actionState[index2] != ch)
					return index;
			}
			return -1;
		}
		return index;
	}
	return -1;
}

int TubeAction::GetToIndex(int t) const
{
	int indexPrev = -1;
	for (int index = (t + 1) * tubeSize - 1; index >= t * tubeSize; --index) {
		if (actionState[index] == '0')
			indexPrev = index;
		else
			break;
	}
	return indexPrev;
}


bool TubeAction::Next(TubeAction& ta, int f, int t) const
{
	int i = GetFromIndex(f);
	if (i < 0)
		return false;
	if (f == t)
		return false;
	int j = GetToIndex(t);
	if (j < 0)
		return false;
	if ((j == t * tubeSize) || (actionState[i] == actionState[j - 1])) {
		ta.actionState = this->actionState;
		char ch = ta.actionState[j];
		ta.actionState[j] = ta.actionState[i];
		ta.actionState[i] = ch; /* ch == '0' */
		ta.from = f;
		ta.to = t;
		ta.tubeNr = tubeNr;
		ta.tubeSize = tubeSize;
		return true;
	}
	return false;
}


TubeAction  TubeAction::Next() const
{
	TubeAction ta;
	switch (mode) {
		case 0:
		{
			int toTmp = to + 1;
			for (int f = from; f < tubeNr; ++f) {
				for (int t = toTmp; t < tubeNr; ++t) {
					if (Next(ta, f, t))
						return ta;
				}
				toTmp = 0;
			}

		}
		break;
		case 1:
		{
			srand(time(NULL));
			for (int i = 0; i < 10000; ++i) {
				unsigned long r = 999999 + (unsigned long )((double)rand() / (double)RAND_MAX * (LONG_MAX - 999999 + 1));
				unsigned long f = r % tubeNr;
				unsigned long t = (r / tubeNr) % tubeNr;
				if (Next(ta, f, t))
					return ta;
				if (Next(ta, t, f))
					return ta;
			}

		}
		break;
	}

	return ta;
}


bool TubeAction::Solved() const
{
	return PuzzleTask::SolvedState(actionState, tubeSize);
}


bool TubeAction::Valid() const
{
	return PuzzleTask::ValidState(actionState, tubeSize);
}

void TubeAction::ShowAction(HWND hWnd, HDC hDC)
{

}


/*
PuzzleTask
*/

PuzzleTask::PuzzleTask() : tubeSize (0), tubeNr (0), puzzleStart ()
{

}


PuzzleTask::PuzzleTask(int s, int n, const std::string& puzzleSta, std::vector<COLORREF>* vectorOfcolorPtrIn) : tubeSize(s), tubeNr(n), puzzleStart(puzzleSta)
{
//	if (vectorOfcolorPtr != nullptr) {
//		delete vectorOfcolorPtr;
//	}
	if (vectorOfcolorPtrIn != nullptr) {
		std::unique_ptr < std::vector<COLORREF>> p = std::make_unique<std::vector<COLORREF>>(*vectorOfcolorPtrIn);
		vectorOfcolorPtr = std::move (p);

	}

}

PuzzleTask::~PuzzleTask()
{

}



bool PuzzleTask::SolveNextStep(int from, int to)
{
	bool ret = false;
	TubeAction  ta;
	if (puzzleState.size() == 0) {
		std::string s;
		if (!AppendEmptyTubes(puzzleStart, s))
			return false;
		puzzleState = s;

		ta = TubeAction(tubeSize, tubeNr, s);
	}
	else {
		//		if (actions.size() > 0)
		//			ta = actions.back();
		//		else
		ta = TubeAction(tubeSize, tubeNr, puzzleState);
	}

	TubeAction taNext;
	if (ta.Next(taNext, from, to)) {
		if (casheState.find(taNext.GetState()) == casheState.end()) {
			casheState[taNext.GetState()] = 1;
			actions.push_back(taNext);
			puzzleState = taNext.GetState();
			ret = true;
		}
	}

	return ret;
}

bool PuzzleTask::SolveNextStep()
{
	bool ret = false;
	TubeAction  ta;
	if (puzzleState.size() == 0) {
		std::string s;
		if (!AppendEmptyTubes(puzzleStart, s))
			return false;
		puzzleState = s;

		ta = TubeAction(tubeSize, tubeNr, s);
	}
	else {
//		if (actions.size() > 0)
//			ta = actions.back();
//		else
			ta = TubeAction(tubeSize, tubeNr, puzzleState);
	}

	TubeAction taNext;

	const int maxEmptyLoop = tubeNr * tubeNr;
	int emptyLoop = 0;
	if (ta.Solved()) {
		OutputDebugStringA("###################!!! Puzzle SOLVED !!!#################\n");
		return true;
	}
	for (taNext = ta.Next(); taNext.Valid(); taNext = ta.Next()) {
		if (casheState.find(taNext.GetState()) == casheState.end ()) {
			casheState[taNext.GetState()] = 1;
			actions.push_back(taNext);
			puzzleState = taNext.GetState();
			ret = true;
			break;
		}
		emptyLoop++;
		if (emptyLoop >= maxEmptyLoop) {
			taNext = TubeAction();
			break;
		}
		ta = taNext;
	}

	while (!taNext.Valid() && actions.size () > 0) {
		ta = actions.back();
		actions.pop_back();
		//casheState.erase(ta.GetState());
		emptyLoop = 0;
		for (taNext = ta.Next(); taNext.Valid(); taNext = ta.Next()) {
			if (casheState.find(taNext.GetState()) == casheState.end ()) {
				casheState[taNext.GetState()] = 1;
				actions.push_back(taNext);
				puzzleState = taNext.GetState();
				ret = true;
				break;
			}
			ta = taNext;
			emptyLoop++;
			if (emptyLoop >= maxEmptyLoop) {
				taNext = TubeAction();
				break;
			}
		}
	}
	if (actions.size() == 0 && casheState.size() > 0) {
		casheState.clear();
		puzzleState.clear();
	}
	return ret;
}


bool PuzzleTask::Solve()
{
	return false;
}


bool PuzzleTask::ValidState() const
{
	if (puzzleState.size() == 0) {
		if (this->tubeSize == 0 || this->tubeNr == 0 || this->puzzleStart.size () == 0)
			return false;
		return ValidState(puzzleStart, tubeSize);
	}
	return ValidState(puzzleState, tubeSize);

	return false;
}

bool PuzzleTask::AppendEmptyTubes(const std::string& sta, std::string& newSta) const
{
	newSta.clear();
	if (((tubeSize * tubeNr) - sta.size()) % tubeSize != 0)
		return false;
	if (sta.size() < tubeSize * tubeNr) {
		newSta = sta;
		for (int i = sta.size(); i < tubeSize * tubeNr; ++i) {
			newSta.append("0");
		}
		return true;
	}
	return false;
}

bool PuzzleTask::ValidState(const std::string& s, int tubeSize)
{
	if (s.size() == 0)
		return false;
	std::map<char, int> mapCharNr;
	for (auto ch : s) {
		auto it = mapCharNr.find(ch);
		if (it != mapCharNr.end())
			mapCharNr[ch]++;
		else
			mapCharNr[ch] = 1;
	}
	for (auto m : mapCharNr) {
		if ((m.first == '0') && ((m.second % tubeSize) != 0))
			return false;
		else if ((m.first != '0') && (m.second != tubeSize))
			return false;
	}

	return true;

}


bool PuzzleTask::SolvedState(const std::string& sta, int tubeSize)
{
	if (!ValidState(sta, tubeSize))
		return false;
	int i = 0;
	char chTube = 0;
	for (auto ch : sta) {
		if (i % tubeSize == 0) {
			chTube = ch;
		}
		else if (chTube != ch) {
			return false;
		}
		++i;
	}

	return true;

}


void PuzzleTask::Reset() 
{
	this->puzzleState.clear();
	this->actions.clear();
	this->casheState.clear();
}


bool PuzzleTask::SolvedState()const
{
	return SolvedState(this->puzzleState, this->tubeSize);
}


bool PuzzleTask::Write(std::string& sta) const
{
	if (!ValidState())
		return false;
	sta = "PuzzleV1.0-tubesize:" + std::to_string(this->tubeSize) + ",";
	sta += "tubenr:" + std::to_string(this->tubeNr) + ",";
	sta += "start:" + this->puzzleStart + "\r\n";
	if (this->vectorOfcolorPtr != nullptr) {
		sta += "colornr:" + std::to_string(this->vectorOfcolorPtr->size());
		for (int i = 0; i < this->vectorOfcolorPtr->size(); ++i) {
			sta += "," + std::to_string((*vectorOfcolorPtr)[i]);
		}
		sta += "\r\n";
	}
	return true;
}


bool PuzzleTask::Read(const std::string& sta)
{
	int tSize = 0;
	int tNr = 0;
	std::string start;
	std::string tmp (sta);

	std::string item ("PuzzleV1.0-tubesize:");
	std::size_t pos = 0;
	pos = tmp.find(item);
	if (pos >= tmp.size())
		return false;
	tmp = tmp.substr(pos + item.size());
	std::size_t w = 0;
	tSize = std::stoi(tmp, &w);
	tmp = tmp.substr(w);

	item = "tubenr:";
	pos = tmp.find(item);
	if (pos >= tmp.size())
		return false;
	tmp = tmp.substr(pos + item.size());
	tNr = std::stoi(tmp, &w);
	tmp = tmp.substr(w);

	item = "start:";
	pos = tmp.find(item);
	if (pos >= tmp.size())
		return false;
	tmp = tmp.substr(pos + item.size());
	start = tmp;
	int endCH = start.find('\r');
	start = start.substr(0, endCH);

	item = "colornr:";
	pos = tmp.find(item);
	if (pos >= tmp.size())
		return false;
	tmp = tmp.substr(pos + item.size());
	int cNr = std::stoi(tmp, &w);
	tmp = tmp.substr(w);
	std::vector<COLORREF>* colorsPtr = nullptr;
	std::vector<COLORREF> colors;
	if (cNr > 0) {
		for (int i = 0; i < cNr; ++i) {
			item = ",";
			pos = tmp.find(item);
			if (pos >= tmp.size())
				return false;
			tmp = tmp.substr(pos + item.size());
			int color = std::stoi(tmp, &w);
			tmp = tmp.substr(w);
			colors.push_back(color);
		}
		colorsPtr = &colors;
	}

	PuzzleTask pt(tSize, tNr, start, colorsPtr);
	if (pt.ValidState()) {
		StaticSetPuzzle(tSize, tNr, start, colorsPtr);
		return true;
	}

	return false;
}


bool PuzzleTask::GetTubePosition(HWND hWnd, int& wTube, std::vector<int>& tubePosition) const
{
	tubePosition.clear();
	wTube = 0;
	if (tubeNr == 0 || tubeSize == 0)
		return false;

	RECT rect;
	GetClientRect(hWnd, &rect);
	wTube = rect.right * 2 / ((2 * tubeNr) + 2) / 3;
	int hTube = rect.bottom * 2 / 3 / 3;
	int hTubeCircle = hTube * 2 / tubeSize / 3;
	if (wTube > hTubeCircle) {
		wTube = hTubeCircle;
	}
	else {
		hTubeCircle = wTube;
		hTube = hTubeCircle * tubeSize;
	}
	int w = wTube;
	for (int i = 0; i < tubeNr; ++i) {
		int x = i * 4 * w / 3 + 3 * rect.right / 5;
		tubePosition.push_back(x);
	}
	return true;
}


void PuzzleTask::ShowState(HWND hWnd, HDC hDC)
{
	if (!ValidState())
		return;

	Graphics graphics(hDC);

	SolidBrush  brushBlue (Color(255, 0, 0, 132));
	SolidBrush  brushLBlue(Color(255, 0, 0, 255));
	SolidBrush  brushRed (Color(255, 132, 0, 0));
	SolidBrush  brushLRed(Color(255, 255, 0, 0));
	SolidBrush  brushGreen (Color(255, 0, 132, 0));
	SolidBrush  brushLGreen(Color(255, 0, 255, 0));
	SolidBrush  brushWhite (Color(255, 255, 255, 255));
	SolidBrush  brushBlack (Color(255, 0, 0, 0));
	SolidBrush  brushGray (Color(255, 128, 128, 128));
	SolidBrush  brushLGray (Color(255, 172, 172, 172));
	SolidBrush  brushYellow (Color(255, 132, 132, 0));
	SolidBrush  brushLYellow(Color(255, 255, 255, 0));
	SolidBrush  brushPink (Color(255, 132, 0, 132));
	SolidBrush  brushLPink(Color(255, 255, 0, 255));
	SolidBrush  brushCyan (Color(255, 0, 132, 132));
	SolidBrush  brushLCyan(Color(255, 0, 255, 255));
	SolidBrush  brushBraun (Color(255, 172, 132, 0));
	SolidBrush  brushPurple (Color(255, 172, 0, 172));
	SolidBrush  brushOrange (Color(255, 255, 172, 0));

	SolidBrush  brushAny(Color(255, 255, 172, 0));

	std::vector<Color> gdiColorVector;
	if (vectorOfcolorPtr != 0) {
		for (auto item : (*vectorOfcolorPtr)) {
			COLORREF col = item;
			gdiColorVector.push_back (Color(255, col & 0xff, (col >> 8) & 0xff, (col >> 16) & 0xff));
		}
	}

	Pen	penBlack(Color(255, 0, 0, 0));

	std::string s;
	if (puzzleState.size() == 0) {
		if (!AppendEmptyTubes(puzzleStart, s))
			return;
	}
	else {
		s = puzzleState;
	}


	std::vector<int> position;
	int wTube;

	if (!GetTubePosition(hWnd, wTube, position))
		return;
									
	int w = wTube;
	int h = wTube;
	for (int i = 0; i < s.size(); ++i) {
		int y = (tubeSize - (i % tubeSize) + 1) * h;
		int x = position[i / tubeSize];
		
		SolidBrush* brushPtr = nullptr;
		if (vectorOfcolorPtr == 0) {
			switch (s[i]) {
			case 'P':
				brushPtr = &brushLRed;
				break;
			case 'L':
				brushPtr = &brushPurple;
				break;
			case 'N':
				brushPtr = &brushOrange;
				break;
			case 'Z':
				brushPtr = &brushLGreen;
				break;
			case 'E':
				brushPtr = &brushWhite;
				break;
			case 'F':
				brushPtr = &brushBlack;
				break;
			case 'B':
				brushPtr = &brushBraun;
				break;
			case 'K':
				brushPtr = &brushLBlue;
				break;
			case 'S':
				brushPtr = &brushLYellow;
				break;
			}
		}
		else if ((s[i] >= 'A') && ((s[i] - 'A') < gdiColorVector.size())) {
				brushAny.SetColor(gdiColorVector[(s[i] - 'A')]);
				brushPtr = &brushAny;
		}

		if (brushPtr != nullptr) {
			graphics.FillRectangle(&brushWhite, x - 1, y - 1, w + 2, h + 2);
			graphics.FillEllipse(&brushBlack, x, y, w, h);
			graphics.FillEllipse(brushPtr, x+2, y+2, w-4, h-4);
		}
		else {
			graphics.FillRectangle(&brushWhite, x-1, y-1, w+2, h+2);
			graphics.DrawRectangle(&penBlack, x, y, w, h);
		}
	}
}

bool PuzzleTask::ManualSolvePuzzle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (taskPtr == nullptr)
		return false;
	switch (message) {
		case WM_LBUTTONDOWN:
		{
			WPARAM xPos = LOWORD(lParam);
			WPARAM yPos = HIWORD(lParam);
			int wTube = 0;
			std::vector<int>position;
			if (!taskPtr->GetTubePosition(hWnd, wTube, position))
				break;
			fromIndex = -1;
			for (int i = 0; i < taskPtr->GetTubeNr(); ++i) {
				if (xPos >= position[i] && xPos <= position[i] + wTube) {
					fromIndex = i;
					break;
				}
			}
		}
		case WM_LBUTTONUP:
		{
			if (fromIndex < 0)
				break;
			WPARAM xPos = LOWORD(lParam);
			WPARAM yPos = HIWORD(lParam);
			int wTube = 0;
			std::vector<int>position;
			if (!taskPtr->GetTubePosition(hWnd, wTube, position))
				break;
			int toIndex = -1;
			for (int i = 0; i < taskPtr->GetTubeNr(); ++i) {
				if (xPos >= position[i] && xPos <= position[i] + wTube) {
					toIndex = i;
					break;
				}
			}
			if ((toIndex < 0) || (fromIndex == toIndex))
				break;

			taskPtr->SolveNextStep(fromIndex,toIndex);

			fromIndex = -1;
			taskPtr->Report();

			HDC hDC = GetDC(hWnd);
			taskPtr->ShowState(hWnd, hDC);
			ReleaseDC(hWnd, hDC);
			break;
		}
	}
	return true;
}

void PuzzleTask::Report() const
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "PT-status: %s\n", puzzleState.c_str());
	OutputDebugStringA(buffer);
	sprintf_s(buffer, sizeof(buffer), "PT-actions nr:%ld\n", actions.size ());
	OutputDebugStringA(buffer);
	auto it = actions.begin();
	for (int i = 0; i < actions.size(); i++, it++) {
		sprintf_s(buffer, sizeof(buffer), "PT-actions %d: from:%d to:%d %s\n", i+1, it->GetFrom (), it->GetTo(), it->GetState().c_str ());
		OutputDebugStringA(buffer);
	}
}

