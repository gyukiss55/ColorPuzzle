#pragma once

/*
Recognize.h
*/

#ifndef _RECOGNIZE_H_
#define _RECOGNIZE_H_

#include <Windows.h>
#include <array>
#include <vector>
#include <objidl.h>
#include <gdiplus.h>

using namespace Gdiplus;

#include "Defines.h"

#define REFPOINTNR 7

typedef std::array<int,2> Vector2D;
typedef std::array<int,2> Point2D;

typedef std::array<double ,2> PointD2D;

struct TubeGroup {
	int32_t top;
	int32_t bottom;
	std::vector<int32_t> xMainVector;
	TubeGroup () : top (0), bottom (0) {}
};

class FilterAdditive {
	double sum;
	int32_t num;
public:
	FilterAdditive() : sum(0.0), num(0) {}
	int32_t Add(int32_t val) { sum += val; ++num; return int32_t(sum / num); }
	int32_t Get() const { return int32_t(sum / num); }
	int32_t GetNum() const { return  num; }
};

class ReferencePoints {
	uint32_t status;
	std::array < Point2D, REFPOINTNR> refPoints;
public:
	ReferencePoints() : status(0) {}
	void Start() { status = 0; }
	void Add(const Point2D& p) 
	{
		if (status < REFPOINTNR)
			refPoints[status++] = p;
	};
	uint32_t GetStatus() const { return status; }
	const std::array < Point2D, REFPOINTNR>& GetRefPoints() const
	{
		return refPoints;
	}
};


class ShowRawStatus {
	bool status;
	int32_t rowSize;
public:

	ShowRawStatus() : status(false), rowSize(0) {}
	void Start(int32_t rawS) { rowSize = rawS; status = true; }
	bool State() const { return (rowSize > 0) && status; }
	int32_t RowSize() const { return rowSize;  }
};


class RecognizePuzzle {
	ReferencePoints refPoints;
	std::vector < Point2D> gridPoints;
	ShowRawStatus rowStatus;
	Rect imageRect;
	Rect imageRectCorrected;
public:
	RecognizePuzzle()  {}
	void Start();
	void DetectPuzzle(HWND hWnd, const Point2D& p);
	void DrawCross(HWND hWnd, HDC hDC, const COLORREF& col, const Point2D& p);
	int32_t CalcDiffSide(HDC hDC, const Point2D& p, int addX, int addY, Point2D& pSide);
	int32_t CalculateGridPoints(HWND hWnd, HDC hDC);
	int32_t DetectColorClass(HWND hWnd, HDC hDC, int tSize, int tNr);
	int32_t StartShowRowStatus(int32_t rowSize);
	int32_t ShowRawStatus(HWND hWnd, HDC hDC, const Point2D& p);
	void SetImageRect(const Rect& rect) { imageRect = rect; }
	bool ShowRawState() const {
		return rowStatus.State();
	}
	int GetDiff(HDC hDC, uint32_t y, std::vector<int32_t>& diffVector);
	int DetectTube(const std::vector<int32_t>& diffVector, uint32_t y, std::vector<int32_t>& tubeXpos, int32_t& tubeWidthPixel);
	bool IsEmpty(HDC hDC, const std::vector < Point2D>& points);
	bool GeneratePuzzle(HWND hWnd, int32_t tubeWidth, const std::vector < std::pair<std::vector<int32_t>, int32_t>>& tubeXYpos);
	int RecognizePicturePuzzle(HWND hWnd);
};

void SetImageRect(Rect& imageRect);

int32_t ColorDiff(COLORREF c1, COLORREF c2);

int InputReferencePointsMouse(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int InputReferencePointsKeyboard(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int ShowRawStatusMouse(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int ShowRowStatusKeyboard();

void RecognizePicturePuzzle(HWND hWnd);

#endif // _RECOGNIZE_H_